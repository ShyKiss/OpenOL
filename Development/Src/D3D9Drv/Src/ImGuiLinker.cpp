/*=============================================================================
    OLImGui.cpp: ImGui D3D9/Win32 integration for Outlast.
    Owns: device init/shutdown, NewFrame, Render, device objects, toggle.
    UI content (tabs, snapshots, ticker) lives in OLGame/Src/OLImGui.cpp.
=============================================================================*/

// ImGui headers must come before UE3 (UE3 overrides operator new/delete).
#include "imgui_compat_msvc2012.h"
#include "imgui.h"
#include "imgui_impl_dx9.h"
#include "imgui_impl_win32.h"

// UE3 core (Engine.h via D3D9DrvPrivate.h).
#include "D3D9DrvPrivate.h"
#include "ImGuiLinker.h"
#include "NotoSans_Regular.h"

// Shared types and extern globals (GSnapFront/Back, GWriteQueue, etc.).
#include "ImGuiShared.h"
#include "OLImGui_Locale.h"

// ImGui_ImplWin32_WndProcHandler is hidden behind #if 0 in the header.
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

// OLGame — releases D3DPOOL_DEFAULT preview surfaces before device Reset.
extern void OLSpawns_ReleaseD3DObjects();

// ---------------------------------------------------------------------------
// Module-local state
// ---------------------------------------------------------------------------

static bool              GImGuiInitialized = false;
bool                     GImGuiShowDemoWindow = false; // extern in ImGuiShared.h
volatile int             GImGuiWantMouse = 0;          // extern in ImGuiShared.h

static IDirect3DDevice9* GPendingDevice = NULL;
static HWND              GPendingWnd    = NULL;

// Critical section guarding all ImGui context access.
// WndProc (game thread) and NewFrame/Render (render thread) must both hold
// this lock when touching the ImGui context to prevent corruption on
// fullscreen toggle (Alt+Enter) where both threads are active simultaneously.
static CRITICAL_SECTION  GImGuiCS;
static bool              GImGuiCSInited = false;

static void EnsureCS()
{
    if (!GImGuiCSInited)
    {
        InitializeCriticalSection(&GImGuiCS);
        GImGuiCSInited = true;
    }
}

// ---------------------------------------------------------------------------
// Blur helpers
//
// Strategy (no pixel shaders, fixed-function D3D9):
//   Pyramid downscale: full → 1/2 → 1/4 → 1/8 → 1/16 → back up to full.
//   Each StretchRect with D3DTEXF_LINEAR acts as a box blur pass.
//   5 levels down + 4 levels up = strong, smooth blur with no shader needed.
//
// All surfaces are render targets (required for StretchRect in D3D9).
// Textures are created once and recreated on device Reset.
// ---------------------------------------------------------------------------

#define BLUR_LEVELS 6

static IDirect3DTexture9* GBlurTex             = NULL; // final crop texture (window-sized)
static IDirect3DSurface9* GBlurSurf            = NULL; // surface of GBlurTex level 0
static IDirect3DSurface9* GBlurMip[BLUR_LEVELS] = {};  // pyramid: [0]=full, [1]=1/2, ..., [4]=1/16
static UINT GBlurBBW  = 0, GBlurBBH  = 0;
static UINT GBlurWinW = 0, GBlurWinH = 0;

static void OLBlur_Release()
{
    if (GBlurSurf) { GBlurSurf->Release(); GBlurSurf = NULL; }
    if (GBlurTex)  { GBlurTex->Release();  GBlurTex  = NULL; }
    for (int i = 0; i < BLUR_LEVELS; i++)
        if (GBlurMip[i]) { GBlurMip[i]->Release(); GBlurMip[i] = NULL; }
    GBlurBBW = GBlurBBH = GBlurWinW = GBlurWinH = 0;
}

static bool OLBlur_EnsureResources(IDirect3DDevice9* Dev, UINT bbW, UINT bbH, UINT winW, UINT winH)
{
    bool ok = GBlurBBW == bbW && GBlurBBH == bbH && GBlurWinW == winW && GBlurWinH == winH && GBlurTex;
    for (int i = 0; i < BLUR_LEVELS && ok; i++) ok = GBlurMip[i] != NULL;
    if (ok) return true;

    OLBlur_Release();

    // Pyramid levels: full, 1/2, 1/4, 1/8, 1/16
    UINT w = bbW, h = bbH;
    for (int i = 0; i < BLUR_LEVELS; i++)
    {
        if (FAILED(Dev->CreateRenderTarget(w, h, D3DFMT_A8R8G8B8, D3DMULTISAMPLE_NONE, 0, FALSE, &GBlurMip[i], NULL)))
        {
            OLBlur_Release();
            return false;
        }
        w = (w > 1) ? w / 2 : 1;
        h = (h > 1) ? h / 2 : 1;
    }

    // Final window-sized crop texture
    if (FAILED(Dev->CreateTexture(winW, winH, 1, D3DUSAGE_RENDERTARGET, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &GBlurTex, NULL)))
    {
        OLBlur_Release();
        return false;
    }
    if (FAILED(GBlurTex->GetSurfaceLevel(0, &GBlurSurf)))
    {
        OLBlur_Release();
        return false;
    }

    GBlurBBW = bbW; GBlurBBH = bbH;
    GBlurWinW = winW; GBlurWinH = winH;
    return true;
}

// Capture and blur the region behind the ImGui window.
// Returns the D3D texture to pass to ImGui as ImTextureID, or NULL on failure.
static IDirect3DTexture9* OLBlur_Capture(IDirect3DDevice9* Dev, int wx, int wy, int ww, int wh)
{
    if (ww <= 0 || wh <= 0)
        return NULL;

    // Clamp window rect to backbuffer — abort if fully outside.
    IDirect3DSurface9* pBB = NULL;
    if (FAILED(Dev->GetBackBuffer(0, 0, D3DBACKBUFFER_TYPE_MONO, &pBB)))
        return NULL;
    D3DSURFACE_DESC bbDesc;
    pBB->GetDesc(&bbDesc);
    UINT bbW = bbDesc.Width, bbH = bbDesc.Height;

    // If window is fully outside the screen, skip.
    if (wx >= (int)bbW || wy >= (int)bbH || wx + ww <= 0 || wy + wh <= 0)
    {
        pBB->Release();
        return NULL;
    }

    if (!OLBlur_EnsureResources(Dev, bbW, bbH, (UINT)ww, (UINT)wh))
    {
        pBB->Release();
        return NULL;
    }

    // 1. Copy backbuffer → level 0 (full res).
    HRESULT hr = Dev->StretchRect(pBB, NULL, GBlurMip[0], NULL, D3DTEXF_LINEAR);
    pBB->Release();
    if (FAILED(hr))
        return NULL;

    // 2. Downscale pyramid: [0]→[1]→[2]→[3]→[4]
    for (int i = 0; i < BLUR_LEVELS - 1; i++)
        if (FAILED(Dev->StretchRect(GBlurMip[i], NULL, GBlurMip[i + 1], NULL, D3DTEXF_LINEAR)))
            return NULL;

    // 3. Extra ping-pong passes at the two lowest levels to strengthen blur.
    for (int p = 0; p < 12; p++)
    {
        int a = (BLUR_LEVELS - 1), b = (BLUR_LEVELS - 2);
        if (FAILED(Dev->StretchRect(GBlurMip[a], NULL, GBlurMip[b], NULL, D3DTEXF_LINEAR))) return NULL;
        if (FAILED(Dev->StretchRect(GBlurMip[b], NULL, GBlurMip[a], NULL, D3DTEXF_LINEAR))) return NULL;
        if (FAILED(Dev->StretchRect(GBlurMip[a], NULL, GBlurMip[b > 0 ? b-1 : b], NULL, D3DTEXF_LINEAR))) return NULL;
        if (FAILED(Dev->StretchRect(GBlurMip[b > 0 ? b-1 : b], NULL, GBlurMip[a], NULL, D3DTEXF_LINEAR))) return NULL;
    }

    // 4. Upscale pyramid: [4]→[3]→[2]→[1]→[0]
    for (int i = BLUR_LEVELS - 1; i > 0; i--)
        if (FAILED(Dev->StretchRect(GBlurMip[i], NULL, GBlurMip[i - 1], NULL, D3DTEXF_LINEAR)))
            return NULL;

    // 5. Crop visible part of window from blurred full-res into the output texture.
    //    srcRect — clamped to backbuffer; dstRect — offset into the output texture
    //    so the visible portion lands at the correct position.
    int clampL = (wx < 0)          ? 0        : wx;
    int clampT = (wy < 0)          ? 0        : wy;
    int clampR = (wx + ww > (int)bbW) ? (int)bbW : wx + ww;
    int clampB = (wy + wh > (int)bbH) ? (int)bbH : wy + wh;

    if (clampR <= clampL || clampB <= clampT)
        return NULL;

    RECT srcRect, dstRect;
    srcRect.left   = clampL; srcRect.top    = clampT;
    srcRect.right  = clampR; srcRect.bottom = clampB;

    // dstRect: position within the output texture (same size as window).
    dstRect.left   = clampL - wx; dstRect.top    = clampT - wy;
    dstRect.right  = dstRect.left + (clampR - clampL);
    dstRect.bottom = dstRect.top  + (clampB - clampT);

    if (FAILED(Dev->StretchRect(GBlurMip[0], &srcRect, GBlurSurf, &dstRect, D3DTEXF_LINEAR)))
        return NULL;

    return GBlurTex;
}

// Debug vectors for runtime tuning — written by render thread, read by game thread.
volatile float GDebugVec0[3] = { 0.f,  0.f, 8.f };
volatile float GDebugVec1[3] = { 0.f, -5.f, 0.f };


// ---------------------------------------------------------------------------
// Init / Shutdown
// ---------------------------------------------------------------------------

static void OLImGui_InitInternal()
{
    if (GImGuiInitialized || !GPendingDevice || !GPendingWnd)
        return;

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& IO = ImGui::GetIO();
    IO.ConfigFlags |= ImGuiConfigFlags_NoMouseCursorChange;
    ImGui::StyleColorsDark();

    // Dark-grey theme — no blue tints.
    {
        ImGuiStyle& St = ImGui::GetStyle();

        // Geometry
        St.WindowRounding    = 4.f;
        St.FrameRounding     = 3.f;
        St.ScrollbarRounding = 3.f;
        St.GrabRounding      = 3.f;
        St.TabRounding       = 3.f;
        St.WindowBorderSize  = 1.f;
        St.FrameBorderSize   = 0.f;
        St.SeparatorTextAlign = ImVec2(0.5f, 0.5f);
        St.ItemSpacing       = ImVec2(8.f, 5.f);
        St.FramePadding      = ImVec2(6.f, 4.f);
        St.ScrollbarSize     = 12.f;

        ImVec4* C = St.Colors;
        // Gradient concept: title bar is the lightest point (~0.22),
        // window body is mid-dark (~0.07), child panels are near-black (~0.04).
        // Interactive elements (buttons, frames) sit at 0.16-0.20 so they
        // read clearly against the body without washing it out.

        // Backgrounds
        C[ImGuiCol_WindowBg]             = ImVec4(0.07f, 0.07f, 0.07f, 0.96f);
        C[ImGuiCol_ChildBg]              = ImVec4(0.04f, 0.04f, 0.04f, 1.00f);
        C[ImGuiCol_PopupBg]              = ImVec4(0.09f, 0.09f, 0.09f, 0.97f);
        // Borders
        C[ImGuiCol_Border]               = ImVec4(0.32f, 0.32f, 0.32f, 0.50f);
        C[ImGuiCol_BorderShadow]         = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
        // Title bar — lightest element, creates top-to-bottom gradient illusion
        C[ImGuiCol_TitleBg]              = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
        C[ImGuiCol_TitleBgActive]        = ImVec4(0.22f, 0.22f, 0.22f, 1.00f);
        C[ImGuiCol_TitleBgCollapsed]     = ImVec4(0.08f, 0.08f, 0.08f, 0.90f);
        // Menubar
        C[ImGuiCol_MenuBarBg]            = ImVec4(0.10f, 0.10f, 0.10f, 1.00f);
        // Frame (input, checkbox, slider bg)
        C[ImGuiCol_FrameBg]              = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
        C[ImGuiCol_FrameBgHovered]       = ImVec4(0.22f, 0.22f, 0.22f, 1.00f);
        C[ImGuiCol_FrameBgActive]        = ImVec4(0.28f, 0.28f, 0.28f, 1.00f);
        // Buttons
        C[ImGuiCol_Button]               = ImVec4(0.18f, 0.18f, 0.18f, 1.00f);
        C[ImGuiCol_ButtonHovered]        = ImVec4(0.30f, 0.30f, 0.30f, 1.00f);
        C[ImGuiCol_ButtonActive]         = ImVec4(0.42f, 0.42f, 0.42f, 1.00f);
        // Headers (collapsing, selectable, tree)
        C[ImGuiCol_Header]               = ImVec4(0.18f, 0.18f, 0.18f, 1.00f);
        C[ImGuiCol_HeaderHovered]        = ImVec4(0.28f, 0.28f, 0.28f, 1.00f);
        C[ImGuiCol_HeaderActive]         = ImVec4(0.38f, 0.38f, 0.38f, 1.00f);
        // Scrollbar
        C[ImGuiCol_ScrollbarBg]          = ImVec4(0.03f, 0.03f, 0.03f, 1.00f);
        C[ImGuiCol_ScrollbarGrab]        = ImVec4(0.26f, 0.26f, 0.26f, 1.00f);
        C[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.38f, 0.38f, 0.38f, 1.00f);
        C[ImGuiCol_ScrollbarGrabActive]  = ImVec4(0.52f, 0.52f, 0.52f, 1.00f);
        // Slider / drag grab
        C[ImGuiCol_SliderGrab]           = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
        C[ImGuiCol_SliderGrabActive]     = ImVec4(0.72f, 0.72f, 0.72f, 1.00f);
        // Check mark
        C[ImGuiCol_CheckMark]            = ImVec4(0.88f, 0.88f, 0.88f, 1.00f);
        // Separator
        C[ImGuiCol_Separator]            = ImVec4(0.26f, 0.26f, 0.26f, 1.00f);
        C[ImGuiCol_SeparatorHovered]     = ImVec4(0.46f, 0.46f, 0.46f, 1.00f);
        C[ImGuiCol_SeparatorActive]      = ImVec4(0.62f, 0.62f, 0.62f, 1.00f);
        // Resize grip
        C[ImGuiCol_ResizeGrip]           = ImVec4(0.22f, 0.22f, 0.22f, 0.55f);
        C[ImGuiCol_ResizeGripHovered]    = ImVec4(0.40f, 0.40f, 0.40f, 0.80f);
        C[ImGuiCol_ResizeGripActive]     = ImVec4(0.58f, 0.58f, 0.58f, 1.00f);
        // Tabs — slightly lighter than body so the bar reads as a distinct band
        C[ImGuiCol_Tab]                  = ImVec4(0.11f, 0.11f, 0.11f, 1.00f);
        C[ImGuiCol_TabHovered]           = ImVec4(0.28f, 0.28f, 0.28f, 1.00f);
        C[ImGuiCol_TabActive]            = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);
        C[ImGuiCol_TabUnfocused]         = ImVec4(0.07f, 0.07f, 0.07f, 1.00f);
        C[ImGuiCol_TabUnfocusedActive]   = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
        // Text
        C[ImGuiCol_Text]                 = ImVec4(0.92f, 0.92f, 0.92f, 1.00f);
        C[ImGuiCol_TextDisabled]         = ImVec4(0.42f, 0.42f, 0.42f, 1.00f);
        // Misc
        C[ImGuiCol_ModalWindowDimBg]     = ImVec4(0.00f, 0.00f, 0.00f, 0.60f);
        C[ImGuiCol_NavHighlight]         = ImVec4(0.55f, 0.55f, 0.55f, 1.00f);
    }

    // Load NotoSans with broad Unicode coverage (Cyrillic, Greek, Latin Extended).
    static const ImWchar GlyphRanges[] =
    {
        0x0020, 0x00FF,  // Latin Basic + Latin-1 Supplement
        0x0100, 0x017F,  // Latin Extended-A
        0x0370, 0x03FF,  // Greek
        0x0400, 0x04FF,  // Cyrillic
        0,
    };
    IO.Fonts->AddFontFromMemoryCompressedTTF(
        NotoSans_Regular_compressed_data,
        (int)NotoSans_Regular_compressed_size,
        20.f, NULL, GlyphRanges);

    ImGui_ImplWin32_Init(GPendingWnd);
    ImGui_ImplDX9_Init(GPendingDevice);
    GImGuiInitialized = true;
}

void OLImGui_Init(IDirect3DDevice9* Device, HWND Wnd)
{
    GPendingDevice = Device;
    GPendingWnd    = Wnd;
    OLLocale_Init();         // load locale files before any tab is drawn
    OLImGui_EnsureTicker(); // allocate snapshot buffers + register FTickableObject
}

void OLImGui_Shutdown()
{
    if (!GImGuiInitialized)
        return;
    OLBlur_Release();
    ImGui_ImplDX9_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();
    GImGuiInitialized = false;
}

// ---------------------------------------------------------------------------
// Per-frame
// ---------------------------------------------------------------------------

void OLImGui_NewFrame()
{
    if (!GImGuiInitialized)
        OLImGui_InitInternal();
    if (!GImGuiInitialized)
        return;

    // When OLGame requests mouse capture (e.g. Connecting banner), enable cursor rendering
    // even if the debug window is closed.
    if (!GImGuiShowDemoWindow && GImGuiWantMouse)
        ImGui::GetIO().ConfigFlags &= ~ImGuiConfigFlags_NoMouseCursorChange;

    ImGui_ImplDX9_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();
}

void OLImGui_Render()
{
    if (!GImGuiInitialized)
        return;

    // Delegate all UI building to OLGame.
    OLImGui_BuildUI();

    // Draw blurred background behind the main window.
    // Window rect is written by OLImGui_BuildUI via OLImGui_SetWindowRect().
    // When the menu is hidden, release blur resources immediately so they don't
    // block D3D device Reset (D3DPOOL_DEFAULT resources must be freed before Reset).
    if (!GImGuiShowDemoWindow)
    {
        OLBlur_Release();
    }
    if (GImGuiShowDemoWindow && GPendingDevice && GOLWinW > 0 && GOLWinH > 0)
    {
        IDirect3DTexture9* BlurTex = OLBlur_Capture(GPendingDevice, GOLWinX, GOLWinY, GOLWinW, GOLWinH);
        if (BlurTex)
        {
            // Draw blur texture over the full window rect.
            // ImGui clips draw calls to the display, so offscreen parts are dropped automatically.
            // The texture was filled with dstRect offset, so UVs map 1:1 to window space.
            ImDrawList* DL = ImGui::GetBackgroundDrawList();
            DL->AddImage(
                (ImTextureID)BlurTex,
                ImVec2((float)GOLWinX,             (float)GOLWinY),
                ImVec2((float)(GOLWinX + GOLWinW), (float)(GOLWinY + GOLWinH)));
        }
    }

    ImGui::Render();
    ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
}

// ---------------------------------------------------------------------------
// Overlay toggle
// ---------------------------------------------------------------------------

void OLImGui_ToggleOverlay()
{
    GImGuiShowDemoWindow = !GImGuiShowDemoWindow;

    if (!GImGuiInitialized)
        return;

    ImGuiIO& IO = ImGui::GetIO();
    if (GImGuiShowDemoWindow)
    {
        IO.ConfigFlags &= ~ImGuiConfigFlags_NoMouseCursorChange;
    }
    else
    {
        IO.ConfigFlags |= ImGuiConfigFlags_NoMouseCursorChange;
        ::SetCursor(NULL);

        // Clear stale detail snapshot.
        GDetailRequest = NULL;
        GDetailReady   = 0;
        if (GDetailFront) { GDetailFront->ActorPtr = NULL; GDetailFront->Groups.Empty(); }
        if (GDetailBack)  { GDetailBack->ActorPtr  = NULL; GDetailBack->Groups.Empty();  }
    }
}

bool OLImGui_IsOverlayVisible()
{
    return GImGuiShowDemoWindow || (GImGuiWantMouse != 0);
}

// ---------------------------------------------------------------------------
// Device object management (called around D3D9 Reset)
// ---------------------------------------------------------------------------

int OLImGui_GetLiveResourceCount()
{
    int count = 0;
    // Blur resources (D3DPOOL_DEFAULT)
    if (GBlurSurf) count++;
    if (GBlurTex)  count++;
    for (int i = 0; i < BLUR_LEVELS; i++)
        if (GBlurMip[i]) count++;
    // ImGui DX9 backend resources (pVB, pIB, FontTexture)
    if (GImGuiInitialized)
        count += ImGui_ImplDX9_GetLiveResourceCount();
    return count;
}

void OLImGui_InvalidateDeviceObjects()
{
    // Called from game thread after render thread is already stopped (device reset path).
    // No CS needed — render thread is not running at this point.
    if (!GImGuiInitialized) return;
    OLBlur_Release();
    // Release D3DPOOL_DEFAULT preview surfaces (StagingB/W) from the asset preview system.
    OLSpawns_ReleaseD3DObjects();
    // Full Shutdown+Init instead of just InvalidateDeviceObjects — guarantees that
    // every D3D object the backend holds
    // is released before Reset.
    ImGui_ImplDX9_Shutdown();
    ImGui_ImplDX9_Init(GPendingDevice);
}

void OLImGui_CreateDeviceObjects()
{
    // Called from game thread after device reset, before render thread restarts.
    // No CS needed.
    if (!GImGuiInitialized) return;
    ImGui_ImplDX9_CreateDeviceObjects();
}

// ---------------------------------------------------------------------------
// Win32 message forwarding
// ---------------------------------------------------------------------------

LRESULT OLImGui_WndProcHandler(HWND Wnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
    return ImGui_ImplWin32_WndProcHandler(Wnd, Msg, wParam, lParam);
}
