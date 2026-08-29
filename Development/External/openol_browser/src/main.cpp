/*
 * openol_browser -- offscreen CEF browser for OLGame.
 *
 * Usage: openol_browser.exe <shm_name> <input_pipe> <cmd_pipe> <width> <height> [url]
 *
 * Renders pages offscreen -> shared memory (double-buffered BGRA).
 * Reads input events from input_pipe, commands from cmd_pipe.
 * Compiled with v110 (MSVC 2012) -- no C++11 threads/atomics, uses WinAPI.
 */

#include <windows.h>
#include <shellapi.h>
#include <string>
#include <cstdio>
#include <cstring>

#include "include/cef_app.h"
#include "include/cef_client.h"
#include "include/cef_render_handler.h"
#include "include/cef_browser.h"
#include "include/cef_life_span_handler.h"
#include "include/cef_load_handler.h"
#include "include/cef_display_handler.h"
#include "include/wrapper/cef_helpers.h"

#include "ol_browser_ipc.h"

// ---------------------------------------------------------------------------
// Globals
// ---------------------------------------------------------------------------

static HANDLE               g_ShmHandle    = NULL;
static void*                g_ShmBase      = NULL;
static OLBrowserFrameHeader* g_Header      = NULL;
static BYTE*                g_PixelBuf[2]  = { NULL, NULL };

static HANDLE               g_InputPipe    = INVALID_HANDLE_VALUE;
static HANDLE               g_CmdPipe      = INVALID_HANDLE_VALUE;

static CefRefPtr<CefBrowser> g_Browser;
static volatile LONG        g_Running      = 1;

static int g_Width  = 1280;
static int g_Height = 720;

// ---------------------------------------------------------------------------
// Pixel buffer helpers
// ---------------------------------------------------------------------------

static BYTE* GetWriteBuffer()
{
    LONG writeIdx = g_Header->WriteIdx ^ 1;
    return g_PixelBuf[writeIdx];
}

static void PublishFrame()
{
    LONG writeIdx = g_Header->WriteIdx ^ 1;
    InterlockedExchange(&g_Header->WriteIdx, writeIdx);
    InterlockedExchangeAdd(&g_Header->FrameSeq, 1L);
}

// ---------------------------------------------------------------------------
// CefRenderHandler
// ---------------------------------------------------------------------------

class OLRenderHandler : public CefRenderHandler
{
public:
    void GetViewRect(CefRefPtr<CefBrowser> browser, CefRect& rect)
    {
        rect.Set(0, 0, g_Width, g_Height);
    }

    void OnPaint(CefRefPtr<CefBrowser> browser,
                 PaintElementType type,
                 const RectList& dirtyRects,
                 const void* buffer,
                 int width, int height)
    {
        if (type != PET_VIEW || !g_Header) return;
        if (width != g_Width || height != g_Height) return;

        BYTE* dst = GetWriteBuffer();
        memcpy(dst, buffer, (size_t)width * height * 4);
        PublishFrame();
    }

    IMPLEMENT_REFCOUNTING(OLRenderHandler);
};

// ---------------------------------------------------------------------------
// CefDisplayHandler
// ---------------------------------------------------------------------------

class OLDisplayHandler : public CefDisplayHandler
{
public:
    void OnTitleChange(CefRefPtr<CefBrowser> browser, const CefString& title)
    {
        if (!g_Header) return;
        std::string t = title.ToString();
        strncpy(g_Header->PageTitle, t.c_str(), sizeof(g_Header->PageTitle) - 1);
        g_Header->PageTitle[sizeof(g_Header->PageTitle) - 1] = '\0';
    }

    void OnAddressChange(CefRefPtr<CefBrowser> browser,
                         CefRefPtr<CefFrame> frame,
                         const CefString& url)
    {
        if (!g_Header || !frame->IsMain()) return;
        std::string u = url.ToString();
        strncpy(g_Header->PageURL, u.c_str(), sizeof(g_Header->PageURL) - 1);
        g_Header->PageURL[sizeof(g_Header->PageURL) - 1] = '\0';
    }

    IMPLEMENT_REFCOUNTING(OLDisplayHandler);
};

// ---------------------------------------------------------------------------
// CefLoadHandler
// ---------------------------------------------------------------------------

class OLLoadHandler : public CefLoadHandler
{
public:
    void OnLoadStart(CefRefPtr<CefBrowser> browser,
                     CefRefPtr<CefFrame> frame,
                     TransitionType transition_type)
    {
        if (g_Header && frame->IsMain())
            InterlockedExchange(&g_Header->PageLoading, 1L);
    }

    void OnLoadEnd(CefRefPtr<CefBrowser> browser,
                   CefRefPtr<CefFrame> frame,
                   int httpStatusCode)
    {
        if (g_Header && frame->IsMain())
            InterlockedExchange(&g_Header->PageLoading, 0L);
    }

    IMPLEMENT_REFCOUNTING(OLLoadHandler);
};

// ---------------------------------------------------------------------------
// CefLifeSpanHandler
// ---------------------------------------------------------------------------

class OLLifeSpanHandler : public CefLifeSpanHandler
{
public:
    void OnAfterCreated(CefRefPtr<CefBrowser> browser)
    {
        g_Browser = browser;
        if (g_Header)
            InterlockedExchange(&g_Header->BrowserReady, 1L);
    }

    void OnBeforeClose(CefRefPtr<CefBrowser> browser)
    {
        g_Browser = nullptr;
        InterlockedExchange(&g_Running, 0L);
    }

    IMPLEMENT_REFCOUNTING(OLLifeSpanHandler);
};

// ---------------------------------------------------------------------------
// CefClient
// ---------------------------------------------------------------------------

class OLClient : public CefClient
{
public:
    OLClient()
    :   m_RenderHandler(new OLRenderHandler())
    ,   m_DisplayHandler(new OLDisplayHandler())
    ,   m_LoadHandler(new OLLoadHandler())
    ,   m_LifeSpanHandler(new OLLifeSpanHandler())
    {}

    CefRefPtr<CefRenderHandler>   GetRenderHandler()   { return m_RenderHandler; }
    CefRefPtr<CefDisplayHandler>  GetDisplayHandler()  { return m_DisplayHandler; }
    CefRefPtr<CefLoadHandler>     GetLoadHandler()     { return m_LoadHandler; }
    CefRefPtr<CefLifeSpanHandler> GetLifeSpanHandler() { return m_LifeSpanHandler; }

private:
    CefRefPtr<OLRenderHandler>   m_RenderHandler;
    CefRefPtr<OLDisplayHandler>  m_DisplayHandler;
    CefRefPtr<OLLoadHandler>     m_LoadHandler;
    CefRefPtr<OLLifeSpanHandler> m_LifeSpanHandler;

    IMPLEMENT_REFCOUNTING(OLClient);
};

// ---------------------------------------------------------------------------
// Input pipe reader thread (WinAPI)
// ---------------------------------------------------------------------------

static DWORD WINAPI InputThreadProc(LPVOID)
{
    ConnectNamedPipe(g_InputPipe, NULL);

    while (g_Running)
    {
        OLBrowserInputEvent ev;
        DWORD bytesRead = 0;
        BOOL ok = ReadFile(g_InputPipe, &ev, sizeof(ev), &bytesRead, NULL);
        if (!ok || bytesRead != sizeof(ev)) break;

        if (!g_Browser) continue;
        CefRefPtr<CefBrowserHost> host = g_Browser->GetHost();

        switch (ev.Type)
        {
        case OL_INPUT_MOUSE_MOVE:
        {
            CefMouseEvent me;
            me.x = ev.X; me.y = ev.Y; me.modifiers = ev.Modifiers;
            host->SendMouseMoveEvent(me, false);
            break;
        }
        case OL_INPUT_MOUSE_DOWN:
        case OL_INPUT_MOUSE_UP:
        {
            CefMouseEvent me;
            me.x = ev.X; me.y = ev.Y; me.modifiers = ev.Modifiers;
            CefBrowserHost::MouseButtonType btn =
                ev.Button == OL_MOUSE_RIGHT  ? MBT_RIGHT  :
                ev.Button == OL_MOUSE_MIDDLE ? MBT_MIDDLE : MBT_LEFT;
            host->SendMouseClickEvent(me, btn, ev.Type == OL_INPUT_MOUSE_UP, 1);
            break;
        }
        case OL_INPUT_MOUSE_SCROLL:
        {
            CefMouseEvent me;
            me.x = ev.X; me.y = ev.Y; me.modifiers = ev.Modifiers;
            host->SendMouseWheelEvent(me, ev.DeltaX, ev.DeltaY);
            break;
        }
        case OL_INPUT_KEY_DOWN:
        case OL_INPUT_KEY_UP:
        {
            CefKeyEvent ke;
            ke.type = (ev.Type == OL_INPUT_KEY_DOWN) ? KEYEVENT_RAWKEYDOWN : KEYEVENT_KEYUP;
            ke.windows_key_code = (int)ev.WinKeyCode;
            ke.native_key_code  = (int)ev.WinKeyCode;
            ke.modifiers        = ev.Modifiers;
            ke.is_system_key    = false;
            host->SendKeyEvent(ke);
            break;
        }
        case OL_INPUT_KEY_CHAR:
        {
            CefKeyEvent ke;
            ke.type             = KEYEVENT_CHAR;
            ke.windows_key_code = (int)ev.Char;
            ke.native_key_code  = (int)ev.Char;
            ke.character        = (char16_t)ev.Char;
            ke.modifiers        = ev.Modifiers;
            ke.is_system_key    = false;
            host->SendKeyEvent(ke);
            break;
        }
        }
    }
    return 0;
}

// ---------------------------------------------------------------------------
// Command pipe reader thread (WinAPI)
// ---------------------------------------------------------------------------

static DWORD WINAPI CmdThreadProc(LPVOID)
{
    ConnectNamedPipe(g_CmdPipe, NULL);

    while (g_Running)
    {
        OLBrowserCmd cmd;
        DWORD bytesRead = 0;
        BOOL ok = ReadFile(g_CmdPipe, &cmd, sizeof(cmd), &bytesRead, NULL);
        if (!ok || bytesRead != sizeof(cmd)) break;

        if (!g_Browser) continue;

        switch (cmd.Type)
        {
        case OL_CMD_NAVIGATE:
            g_Browser->GetMainFrame()->LoadURL(cmd.URL);
            break;
        case OL_CMD_RELOAD:
            g_Browser->Reload();
            break;
        case OL_CMD_BACK:
            if (g_Browser->CanGoBack()) g_Browser->GoBack();
            break;
        case OL_CMD_FORWARD:
            if (g_Browser->CanGoForward()) g_Browser->GoForward();
            break;
        case OL_CMD_RESIZE:
            if (cmd.Width > 0 && cmd.Height > 0)
            {
                g_Width  = cmd.Width;
                g_Height = cmd.Height;
                InterlockedExchange(&g_Header->Width,  (LONG)g_Width);
                InterlockedExchange(&g_Header->Height, (LONG)g_Height);
                g_Browser->GetHost()->WasResized();
            }
            break;
        case OL_CMD_CLOSE:
            g_Browser->GetHost()->CloseBrowser(true);
            break;
        }
    }
    return 0;
}

// ---------------------------------------------------------------------------
// Shared memory setup
// ---------------------------------------------------------------------------

static bool SetupSharedMemory(const char* shmName)
{
    g_ShmHandle = CreateFileMappingA(INVALID_HANDLE_VALUE, NULL,
        PAGE_READWRITE, 0, (DWORD)OL_BROWSER_SHM_SIZE, shmName);
    if (!g_ShmHandle) return false;

    g_ShmBase = MapViewOfFile(g_ShmHandle, FILE_MAP_ALL_ACCESS, 0, 0, OL_BROWSER_SHM_SIZE);
    if (!g_ShmBase) return false;

    memset(g_ShmBase, 0, OL_BROWSER_SHM_SIZE);
    g_Header = (OLBrowserFrameHeader*)g_ShmBase;
    g_Header->Width  = g_Width;
    g_Header->Height = g_Height;

    size_t frameBytes = (size_t)OL_BROWSER_MAX_WIDTH * OL_BROWSER_MAX_HEIGHT * 4;
    BYTE* pixBase = (BYTE*)g_ShmBase + sizeof(OLBrowserFrameHeader);
    g_PixelBuf[0] = pixBase;
    g_PixelBuf[1] = pixBase + frameBytes;

    return true;
}

// ---------------------------------------------------------------------------
// CefApp
// ---------------------------------------------------------------------------

class OLApp : public CefApp, public CefBrowserProcessHandler
{
public:
    CefRefPtr<CefBrowserProcessHandler> GetBrowserProcessHandler() { return this; }

    void OnBeforeCommandLineProcessing(const CefString& process_type,
                                       CefRefPtr<CefCommandLine> cmd)
    {
        // Disable GPU rendering -- offscreen doesn't need it, avoids D3D issues under Wine/Proton.
        // Software rasterizer is kept enabled so the software video decode path works
        // (needed for H.264/AAC on YouTube/Twitch when a proprietary-codecs libffmpeg.dll is present).
        cmd->AppendSwitch("disable-gpu");
        cmd->AppendSwitch("disable-gpu-compositing");
        // No sandbox under Wine/Proton
        cmd->AppendSwitch("no-sandbox");
        // Enable autoplay so video streams start without user gesture requirement
        cmd->AppendSwitchWithValue("autoplay-policy", "no-user-gesture-required");
    }

    IMPLEMENT_REFCOUNTING(OLApp);
};

// ---------------------------------------------------------------------------
// WinMain
// ---------------------------------------------------------------------------

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR lpCmdLine, int)
{
    // CEF subprocess check (must be before anything else)
    CefMainArgs mainArgs(hInstance);
    {
        CefRefPtr<OLApp> app(new OLApp());
        int exitCode = CefExecuteProcess(mainArgs, app, NULL);
        if (exitCode >= 0) return exitCode;
    }

    // Parse args: <shm_name> <input_pipe> <cmd_pipe> <width> <height> [url]
    int    argc = 0;
    LPWSTR* argv = CommandLineToArgvW(GetCommandLineW(), &argc);
    if (!argv || argc < 6)
    {
        MessageBoxA(NULL,
            "Usage: openol_browser.exe <shm> <input_pipe> <cmd_pipe> <w> <h> [url]",
            "openol_browser", MB_OK | MB_ICONERROR);
        return 1;
    }

    // Convert wide args to UTF-8
    auto W2A = [](LPWSTR w) -> std::string {
        int n = WideCharToMultiByte(CP_UTF8, 0, w, -1, NULL, 0, NULL, NULL);
        std::string s(n, '\0');
        WideCharToMultiByte(CP_UTF8, 0, w, -1, &s[0], n, NULL, NULL);
        if (!s.empty() && s[s.size()-1] == '\0') s.resize(s.size()-1);
        return s;
    };

    std::string shmName   = W2A(argv[1]);
    std::string inputPipe = W2A(argv[2]);
    std::string cmdPipe   = W2A(argv[3]);
    g_Width               = _wtoi(argv[4]);
    g_Height              = _wtoi(argv[5]);
    std::string startURL  = (argc > 6) ? W2A(argv[6]) : std::string("about:blank");
    LocalFree(argv);

    // Shared memory
    if (!SetupSharedMemory(shmName.c_str()))
    {
        MessageBoxA(NULL, "Failed to create shared memory", "openol_browser", MB_OK | MB_ICONERROR);
        return 1;
    }

    // Input named pipe (server side -- game connects as client)
    g_InputPipe = CreateNamedPipeA(inputPipe.c_str(),
        PIPE_ACCESS_INBOUND,
        PIPE_TYPE_BYTE | PIPE_READMODE_BYTE | PIPE_WAIT,
        1, 0, sizeof(OLBrowserInputEvent) * 64, 0, NULL);

    // Cmd named pipe
    g_CmdPipe = CreateNamedPipeA(cmdPipe.c_str(),
        PIPE_ACCESS_INBOUND,
        PIPE_TYPE_BYTE | PIPE_READMODE_BYTE | PIPE_WAIT,
        1, 0, sizeof(OLBrowserCmd) * 16, 0, NULL);

    // Start pipe threads (WinAPI, compatible with v110)
    CloseHandle(CreateThread(NULL, 0, InputThreadProc, NULL, 0, NULL));
    CloseHandle(CreateThread(NULL, 0, CmdThreadProc,   NULL, 0, NULL));

    // CEF settings
    CefSettings settings;
    settings.windowless_rendering_enabled = true;
    settings.no_sandbox                   = true;
    settings.multi_threaded_message_loop  = false;
    CefString(&settings.log_file).FromASCII("openol_browser.log");
    settings.log_severity = LOGSEVERITY_WARNING;

    // Use a per-instance cache dir so multiple instances don't collide on the
    // singleton lock that CEF places in root_cache_path.
    // shmName is already unique per AOLBrowser actor (contains the actor pointer).
    {
        // Get the directory of the running exe
        char exeDir[MAX_PATH] = {};
        GetModuleFileNameA(NULL, exeDir, MAX_PATH);
        char* last = strrchr(exeDir, '\\');
        if (last) *(last + 1) = '\0';

        std::string cachePath = std::string(exeDir) + "cache\\" + shmName;
        CefString(&settings.root_cache_path).FromASCII(cachePath.c_str());
        CefString(&settings.cache_path).FromASCII(cachePath.c_str());
    }

    CefRefPtr<OLApp> app(new OLApp());
    if (!CefInitialize(mainArgs, settings, app, NULL))
    {
        MessageBoxA(NULL, "CefInitialize failed", "openol_browser", MB_OK | MB_ICONERROR);
        return 1;
    }

    // Create offscreen browser
    CefWindowInfo windowInfo;
    windowInfo.SetAsWindowless(NULL);

    CefBrowserSettings browserSettings;
    browserSettings.windowless_frame_rate = 30;

    CefRefPtr<OLClient> client(new OLClient());
    CefBrowserHost::CreateBrowser(windowInfo, client, startURL, browserSettings, nullptr, nullptr);

    // Message loop
    while (g_Running)
        CefDoMessageLoopWork();

    CefShutdown();

    if (g_ShmBase)  UnmapViewOfFile(g_ShmBase);
    if (g_ShmHandle) CloseHandle(g_ShmHandle);
    if (g_InputPipe != INVALID_HANDLE_VALUE) CloseHandle(g_InputPipe);
    if (g_CmdPipe   != INVALID_HANDLE_VALUE) CloseHandle(g_CmdPipe);

    // Remove per-instance cache dir (best-effort, ignore errors)
    {
        char exeDir[MAX_PATH] = {};
        GetModuleFileNameA(NULL, exeDir, MAX_PATH);
        char* last = strrchr(exeDir, '\\');
        if (last) *(last + 1) = '\0';

        std::string cachePath = std::string(exeDir) + "cache\\" + shmName;
        // Recursively delete: use cmd /c rd /s /q
        std::string cmd = "cmd /c rd /s /q \"" + cachePath + "\"";
        STARTUPINFOA si = {}; si.cb = sizeof(si);
        PROCESS_INFORMATION pi = {};
        if (CreateProcessA(NULL, &cmd[0], NULL, NULL, FALSE,
                CREATE_NO_WINDOW, NULL, NULL, &si, &pi))
        {
            WaitForSingleObject(pi.hProcess, 5000);
            CloseHandle(pi.hProcess);
            CloseHandle(pi.hThread);
        }
    }

    return 0;
}
