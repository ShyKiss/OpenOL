// OutlastLauncher.cpp : Defines the entry point for the application.

#include "stdafx.h"
#include "OutlastLauncher.h"

// ---- Version ---------------------------------------------------------------
#define CURRENT_VERSION   L"2.1.0"
#define GITHUB_OWNER      L"ShyKiss"
#define GITHUB_REPO       L"OpenOL"

// ---- Helpers ---------------------------------------------------------------

// Show a message box with the given text.
static void ShowMsg(LPCWSTR title, LPCWSTR text, UINT type = MB_OK | MB_ICONINFORMATION)
{
    MessageBoxW(NULL, text, title, type);
}

// Perform an HTTPS GET and store the response in |out| (null-terminated).
// Returns number of bytes read, or 0 on failure.
static DWORD HttpGet(LPCWSTR host, LPCWSTR path, char* out, DWORD outSize)
{
    HINTERNET hInet = InternetOpenW(L"OutlastLauncher/1.0",
        INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);
    if (!hInet) return 0;

    HINTERNET hConn = InternetConnectW(hInet, host, INTERNET_DEFAULT_HTTPS_PORT,
        NULL, NULL, INTERNET_SERVICE_HTTP, 0, 0);
    if (!hConn) { InternetCloseHandle(hInet); return 0; }

    DWORD flags = INTERNET_FLAG_SECURE | INTERNET_FLAG_RELOAD | INTERNET_FLAG_NO_CACHE_WRITE;
    HINTERNET hReq = HttpOpenRequestW(hConn, L"GET", path, NULL, NULL, NULL, flags, 0);
    if (!hReq) { InternetCloseHandle(hConn); InternetCloseHandle(hInet); return 0; }

    if (!HttpSendRequestW(hReq, NULL, 0, NULL, 0))
    {
        InternetCloseHandle(hReq); InternetCloseHandle(hConn); InternetCloseHandle(hInet);
        return 0;
    }

    DWORD total = 0, read = 0;
    while (total < outSize - 1 &&
           InternetReadFile(hReq, out + total, outSize - 1 - total, &read) && read > 0)
        total += read;
    out[total] = '\0';

    InternetCloseHandle(hReq);
    InternetCloseHandle(hConn);
    InternetCloseHandle(hInet);
    return total;
}

// Download a file from |url| (https) to |destPath|. Returns TRUE on success.
static BOOL DownloadFile(LPCWSTR host, LPCWSTR path, LPCWSTR destPath)
{
    HINTERNET hInet = InternetOpenW(L"OutlastLauncher/1.0",
        INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);
    if (!hInet) return FALSE;

    HINTERNET hConn = InternetConnectW(hInet, host, INTERNET_DEFAULT_HTTPS_PORT,
        NULL, NULL, INTERNET_SERVICE_HTTP, 0, 0);
    if (!hConn) { InternetCloseHandle(hInet); return FALSE; }

    DWORD flags = INTERNET_FLAG_SECURE | INTERNET_FLAG_RELOAD | INTERNET_FLAG_NO_CACHE_WRITE;
    HINTERNET hReq = HttpOpenRequestW(hConn, L"GET", path, NULL, NULL, NULL, flags, 0);
    if (!hReq) { InternetCloseHandle(hConn); InternetCloseHandle(hInet); return FALSE; }

    if (!HttpSendRequestW(hReq, NULL, 0, NULL, 0))
    {
        InternetCloseHandle(hReq); InternetCloseHandle(hConn); InternetCloseHandle(hInet);
        return FALSE;
    }

    HANDLE hFile = CreateFileW(destPath, GENERIC_WRITE, 0, NULL,
        CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE)
    {
        InternetCloseHandle(hReq); InternetCloseHandle(hConn); InternetCloseHandle(hInet);
        return FALSE;
    }

    char buf[65536];
    DWORD read, written;
    BOOL ok = TRUE;
    while (InternetReadFile(hReq, buf, sizeof(buf), &read) && read > 0)
    {
        if (!WriteFile(hFile, buf, read, &written, NULL)) { ok = FALSE; break; }
    }

    CloseHandle(hFile);
    InternetCloseHandle(hReq);
    InternetCloseHandle(hConn);
    InternetCloseHandle(hInet);
    return ok;
}

// Extract a simple JSON string value for |key| from |json|.
// Looks for: "key":"value" — fills buf (wchar). Returns TRUE on success.
static BOOL JsonGetString(const char* json, const char* key, WCHAR* buf, int bufLen)
{
    // Build search pattern: "key":"
    char pattern[128];
    _snprintf(pattern, sizeof(pattern), "\"%s\":\"", key);

    const char* p = strstr(json, pattern);
    if (!p) return FALSE;
    p += strlen(pattern);

    const char* end = strchr(p, '"');
    if (!end) return FALSE;

    int len = (int)(end - p);
    MultiByteToWideChar(CP_UTF8, 0, p, len, buf, bufLen);
    buf[min(len, bufLen - 1)] = L'\0';
    return TRUE;
}

// Recursively copy all files from srcDir to dstDir.
static void CopyDirContents(LPCWSTR srcDir, LPCWSTR dstDir)
{
    WCHAR pattern[MAX_PATH];
    PathCombineW(pattern, srcDir, L"*");

    WIN32_FIND_DATAW fd;
    HANDLE hFind = FindFirstFileW(pattern, &fd);
    if (hFind == INVALID_HANDLE_VALUE) return;

    do
    {
        if (wcscmp(fd.cFileName, L".") == 0 || wcscmp(fd.cFileName, L"..") == 0)
            continue;

        WCHAR src[MAX_PATH], dst[MAX_PATH];
        PathCombineW(src, srcDir, fd.cFileName);
        PathCombineW(dst, dstDir, fd.cFileName);

        if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
        {
            CreateDirectoryW(dst, NULL);
            CopyDirContents(src, dst);
        }
        else
        {
            CopyFileW(src, dst, FALSE);
        }
    }
    while (FindNextFileW(hFind, &fd));

    FindClose(hFind);
}

// Recursively delete a directory.
static void DeleteDir(LPCWSTR dir)
{
    WCHAR pattern[MAX_PATH];
    PathCombineW(pattern, dir, L"*");

    WIN32_FIND_DATAW fd;
    HANDLE hFind = FindFirstFileW(pattern, &fd);
    if (hFind == INVALID_HANDLE_VALUE) return;

    do
    {
        if (wcscmp(fd.cFileName, L".") == 0 || wcscmp(fd.cFileName, L"..") == 0)
            continue;

        WCHAR path[MAX_PATH];
        PathCombineW(path, dir, fd.cFileName);

        if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
            DeleteDir(path);
        else
            DeleteFileW(path);
    }
    while (FindNextFileW(hFind, &fd));

    FindClose(hFind);
    RemoveDirectoryW(dir);
}

// Unzip |zipPath| into |destDir| using the built-in Windows Shell.
// Extract a .7z archive using 7z.exe from PATH or Program Files.
static BOOL Extract7z(LPCWSTR archivePath, LPCWSTR destDir)
{
    // Try to find 7z.exe
    WCHAR sevenZip[MAX_PATH] = L"7z.exe"; // try PATH first

    WCHAR cmdLine[MAX_PATH * 3];
    _snwprintf(cmdLine, _countof(cmdLine), L"\"%s\" x \"%s\" -o\"%s\" -y", sevenZip, archivePath, destDir);

    STARTUPINFOW si = {}; si.cb = sizeof(si);
    PROCESS_INFORMATION pi = {};
    if (!CreateProcessW(NULL, cmdLine, NULL, NULL, FALSE, CREATE_NO_WINDOW, NULL, NULL, &si, &pi))
    {
        // Try common install paths
        static const WCHAR* paths[] = {
            L"C:\\Program Files\\7-Zip\\7z.exe",
            L"C:\\Program Files (x86)\\7-Zip\\7z.exe",
        };
        for (int i = 0; i < 2; i++)
        {
            _snwprintf(cmdLine, _countof(cmdLine), L"\"%s\" x \"%s\" -o\"%s\" -y", paths[i], archivePath, destDir);
            if (CreateProcessW(NULL, cmdLine, NULL, NULL, FALSE, CREATE_NO_WINDOW, NULL, NULL, &si, &pi))
                goto wait;
        }
        return FALSE;
    }
wait:
    WaitForSingleObject(pi.hProcess, 60000);
    DWORD exitCode = 1;
    GetExitCodeProcess(pi.hProcess, &exitCode);
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
    return exitCode == 0;
}

static BOOL UnzipShell(LPCWSTR zipPath, LPCWSTR destDir)
{
    IShellDispatch* pShell = NULL;
    CoInitialize(NULL);

    if (FAILED(CoCreateInstance(CLSID_Shell, NULL, CLSCTX_INPROC_SERVER,
        IID_IShellDispatch, (void**)&pShell)))
    {
        CoUninitialize();
        return FALSE;
    }

    VARIANT vZip, vDest, vOpts;
    VariantInit(&vZip);  VariantInit(&vDest);  VariantInit(&vOpts);

    vZip.vt  = VT_BSTR; vZip.bstrVal  = SysAllocString(zipPath);
    vDest.vt = VT_BSTR; vDest.bstrVal = SysAllocString(destDir);
    vOpts.vt = VT_I4;   vOpts.lVal    = FOF_SILENT | FOF_NOCONFIRMATION | FOF_NOERRORUI;

    Folder* pZipFolder = NULL;
    Folder* pDestFolder = NULL;

    pShell->NameSpace(vZip,  &pZipFolder);
    pShell->NameSpace(vDest, &pDestFolder);

    BOOL ok = FALSE;
    if (pZipFolder && pDestFolder)
    {
        FolderItems* pItems = NULL;
        pZipFolder->Items(&pItems);
        if (pItems)
        {
            VARIANT vItems;
            VariantInit(&vItems);
            vItems.vt       = VT_DISPATCH;
            vItems.pdispVal = pItems;

            VARIANT vEmpty; VariantInit(&vEmpty);
            pDestFolder->CopyHere(vItems, vOpts);

            // Shell CopyHere is async — wait for files to appear
            Sleep(3000);
            pItems->Release();
            ok = TRUE;
        }
        if (pDestFolder) pDestFolder->Release();
        if (pZipFolder)  pZipFolder->Release();
    }

    SysFreeString(vZip.bstrVal);
    SysFreeString(vDest.bstrVal);
    pShell->Release();
    CoUninitialize();
    return ok;
}

// ---- Version comparison ----------------------------------------------------

// Parse "1.2.3" or "v1.2.3" into major/minor/patch integers.
static void ParseVersion(const WCHAR* s, int* major, int* minor, int* patch)
{
    if (*s == L'v' || *s == L'V') s++;
    *major = *minor = *patch = 0;
    swscanf(s, L"%d.%d.%d", major, minor, patch);
}

// Returns TRUE if |a| is strictly newer than |b|.
static BOOL VersionNewer(const WCHAR* a, const WCHAR* b)
{
    int ma, na, pa, mb, nb, pb;
    ParseVersion(a, &ma, &na, &pa);
    ParseVersion(b, &mb, &nb, &pb);
    if (ma != mb) return ma > mb;
    if (na != nb) return na > nb;
    return pa > pb;
}

// ---- Check for updates -----------------------------------------------------

// Queries GitHub Releases API and returns the latest tag in |latestTag|.
// Returns TRUE if a newer version is available.
static BOOL CheckForUpdate(WCHAR* latestTag, int tagBufLen, WCHAR* zipUrl, int urlBufLen)
{
    WCHAR apiPath[256];
    _snwprintf(apiPath, 256, L"/repos/%s/%s/releases/latest", GITHUB_OWNER, GITHUB_REPO);

    char response[65536] = {};
    if (!HttpGet(L"api.github.com", apiPath, response, sizeof(response)))
        return FALSE;

    WCHAR tag[64] = {};
    if (!JsonGetString(response, "tag_name", tag, 64))
        return FALSE;

    wcsncpy(latestTag, tag, tagBufLen);

    // Find zip asset URL (browser_download_url ending in .zip)
    const char* p = response;
    while ((p = strstr(p, "browser_download_url")) != NULL)
    {
        p += strlen("browser_download_url") + 3; // skip ":"
        const char* end = strchr(p, '"');
        if (!end) break;

        char urlA[1024] = {};
        int len = (int)(end - p);
        strncpy(urlA, p, min(len, (int)sizeof(urlA) - 1));

        if (strstr(urlA, ".zip") || strstr(urlA, ".7z"))
        {
            MultiByteToWideChar(CP_UTF8, 0, urlA, -1, zipUrl, urlBufLen);
            break;
        }
        p = end + 1;
    }

    // Only report update if remote is strictly newer and we have a download URL
    return VersionNewer(tag, CURRENT_VERSION) && zipUrl[0] != L'\0';
}

// ---- Update mode -----------------------------------------------------------

static void RunUpdate(LPCWSTR gameDir, LPCWSTR zipUrl)
{
    // Parse host and path from full URL (https://github.com/...)
    // URL format: https://objects.githubusercontent.com/...
    WCHAR host[256] = {}, path[1024] = {};
    const WCHAR* p = zipUrl;

    // Skip "https://"
    if (wcsncmp(p, L"https://", 8) == 0) p += 8;
    else if (wcsncmp(p, L"http://",  7) == 0) p += 7;

    const WCHAR* slash = wcschr(p, L'/');
    if (slash)
    {
        wcsncpy(host, p, (int)(slash - p));
        wcsncpy(path, slash, 1024);
    }

    // Download archive to temp
    BOOL bIs7z = (wcsstr(zipUrl, L".7z") != NULL);
    WCHAR tempDir[MAX_PATH], zipPath[MAX_PATH], extractDir[MAX_PATH];
    GetTempPathW(MAX_PATH, tempDir);
    PathCombineW(zipPath,    tempDir, bIs7z ? L"OpenOL_update.7z" : L"OpenOL_update.zip");
    PathCombineW(extractDir, tempDir, L"OpenOL_extract");

    // Clean previous temp
    DeleteFileW(zipPath);
    DeleteDir(extractDir);
    CreateDirectoryW(extractDir, NULL);

    if (!DownloadFile(host, path, zipPath))
    {
        ShowMsg(L"OpenOL Update", L"Failed to download update.", MB_OK | MB_ICONERROR);
        return;
    }

    BOOL extractOk = bIs7z ? Extract7z(zipPath, extractDir) : UnzipShell(zipPath, extractDir);
    if (!extractOk)
    {
        ShowMsg(L"OpenOL Update", L"Failed to extract update.\n7-Zip must be installed for .7z archives.", MB_OK | MB_ICONERROR);
        return;
    }

    // Copy extracted files over game directory
    CopyDirContents(extractDir, gameDir);

    // Cleanup
    DeleteFileW(zipPath);
    DeleteDir(extractDir);

    ShowMsg(L"OpenOL Update", L"Installation complete!\nNow you can launch the game.");
}

// ---- Entry point -----------------------------------------------------------

int _tmain(int argc, _TCHAR* argv[])
{
    bool bUpdateMode      = false;
    WCHAR openolHost[64]  = {};   // SteamID of host
    WCHAR openolRoom[64]  = {};   // Room code (default "DEFAULT")

    for (int i = 1; i < argc; i++)
    {
        if (_tcsicmp(argv[i], TEXT("-updaterequired")) == 0)
            bUpdateMode = true;

        // "+openol <hostSteamID> <room>" — our own format passed between launcher instances.
        if (_tcsicmp(argv[i], TEXT("+openol")) == 0)
        {
            if (i + 1 < argc) { wcsncpy(openolHost, argv[i + 1], 63); i++; }
            if (i + 1 < argc) { wcsncpy(openolRoom, argv[i + 1], 63); i++; }
        }

        // Steam Rich Presence cold launch: passes connect string verbatim as a bare argument.
        // Our connect string format: "<hostSteamID>/<room>"  e.g. "76561198012345678/DEFAULT"
        // If it contains '/' and the left part is numeric — it's our P2P connect string.
        if (argv[i][0] != L'-' && argv[i][0] != L'+' && !openolHost[0])
        {
            WCHAR* slash = wcschr(argv[i], L'/');
            if (slash)
            {
                *slash = L'\0';
                // Left part = SteamID (numeric), right part = room
                bool bNumeric = true;
                for (int j = 0; argv[i][j]; j++)
                    if (argv[i][j] < L'0' || argv[i][j] > L'9') { bNumeric = false; break; }
                if (bNumeric && argv[i][0])
                {
                    wcsncpy(openolHost, argv[i],   63);
                    wcsncpy(openolRoom, slash + 1, 63);
                }
                *slash = L'/'; // restore
            }
        }
    }

    // Default room if host found but room not specified
    if (openolHost[0] && !openolRoom[0])
        wcsncpy(openolRoom, L"DEFAULT", 63);

    // Get directory of this executable
    WCHAR exePath[MAX_PATH], gameDir[MAX_PATH];
    GetModuleFileNameW(NULL, exePath, MAX_PATH);
    wcsncpy(gameDir, exePath, MAX_PATH);
    PathRemoveFileSpecW(gameDir);

    // ---- Update mode: download & install ------------------------------------
    if (bUpdateMode)
    {
        WCHAR zipUrl[1024] = {};
        WCHAR installDir[MAX_PATH] = {};

        for (int i = 1; i < argc - 1; i++)
        {
            if (_tcsicmp(argv[i], TEXT("-updaterequired")) == 0)
                wcsncpy(zipUrl, argv[i + 1], 1024);
            else if (_tcsicmp(argv[i], TEXT("-gamedir")) == 0)
                wcsncpy(installDir, argv[i + 1], MAX_PATH);
        }

        if (zipUrl[0] == L'\0')
        {
            ShowMsg(L"OpenOL Update", L"No download URL provided.", MB_OK | MB_ICONERROR);
            return 1;
        }

        // Use explicitly passed game dir; fall back to our own dir (direct run case)
        RunUpdate(installDir[0] ? installDir : gameDir, zipUrl);
        return 0;
    }

    // ---- Normal mode: check for updates ------------------------------------
    WCHAR latestTag[64] = {}, zipUrl[1024] = {};
    if (CheckForUpdate(latestTag, 64, zipUrl, 1024))
    {
        WCHAR msg[256];
        _snwprintf(msg, 256,
            L"A new version of OpenOL is available!\n\n"
            L"Current: %s\nLatest:  %s\n\n"
            L"Download and install now?",
            CURRENT_VERSION, latestTag);

        int result = MessageBoxW(NULL, msg, L"OpenOL Update",
            MB_YESNO | MB_ICONQUESTION);

        if (result == IDYES)
        {
            // Copy self to %TEMP%\OpenOL\ and launch with -updaterequired
            WCHAR tempDir[MAX_PATH], tempExe[MAX_PATH];
            GetTempPathW(MAX_PATH, tempDir);
            PathAppendW(tempDir, L"OpenOL");
            CreateDirectoryW(tempDir, NULL);
            PathCombineW(tempExe, tempDir, L"OutlastLauncher.exe");

            CopyFileW(exePath, tempExe, FALSE);

            // Build command line: OutlastLauncher.exe -updaterequired "<zipUrl>" -gamedir "<gameDir>"
            // Pass through +openol <hostSteamID> if present so cold-launch join survives the update flow.
            WCHAR cmdLine[2048];
            if (openolHost[0])
                _snwprintf(cmdLine, 2048, L"\"%s\" -updaterequired \"%s\" -gamedir \"%s\" +openol \"%s\" \"%s\"", tempExe, zipUrl, gameDir, openolHost, openolRoom);
            else
                _snwprintf(cmdLine, 2048, L"\"%s\" -updaterequired \"%s\" -gamedir \"%s\"", tempExe, zipUrl, gameDir);

            STARTUPINFOW si = {};
            si.cb = sizeof(si);
            PROCESS_INFORMATION pi = {};
            CreateProcessW(NULL, cmdLine, NULL, NULL, FALSE, 0, NULL, gameDir, &si, &pi);
            CloseHandle(pi.hProcess);
            CloseHandle(pi.hThread);

            return 0; // Exit current launcher
        }
    }

    // Locate OLGame.exe — try next to the launcher first, then relative paths for other layouts
    WCHAR olGameExe[MAX_PATH];
    PathCombineW(olGameExe, gameDir, L"OLGame.exe");
    if (GetFileAttributesW(olGameExe) == INVALID_FILE_ATTRIBUTES)
        PathCombineW(olGameExe, gameDir, L"Binaries\\Win64\\OLGame.exe");
    if (GetFileAttributesW(olGameExe) == INVALID_FILE_ATTRIBUTES)
        PathCombineW(olGameExe, gameDir, L"..\\..\\Binaries\\Win64\\OLGame.exe");

    // ---- Collect extra args from Steam launch options ----------------------
    // Any argv tokens not consumed by our own parsing are forwarded verbatim.
    // Known tokens we consume: -updaterequired, +openol <host> <room>, bare connect strings.
    // Everything else (e.g. -dx11, -windowed, custom user flags) passes through.
    WCHAR extraArgs[1024] = {};
    for (int i = 1; i < argc; i++)
    {
        // Skip tokens we already handled.
        if (_tcsicmp(argv[i], TEXT("-updaterequired")) == 0) { i++; continue; }
        if (_tcsicmp(argv[i], TEXT("-gamedir"))        == 0) { i++; continue; }
        if (_tcsicmp(argv[i], TEXT("+openol"))         == 0) { i += 2; continue; }

        // Skip bare connect strings (numeric/room already parsed above).
        if (argv[i][0] != L'-' && argv[i][0] != L'+')
        {
            WCHAR* sl = wcschr(argv[i], L'/');
            if (sl)
            {
                *sl = L'\0';
                bool bNum = true;
                for (int j = 0; argv[i][j]; j++)
                    if (argv[i][j] < L'0' || argv[i][j] > L'9') { bNum = false; break; }
                *sl = L'/';
                if (bNum && argv[i][0]) continue; // already consumed as connect string
            }
        }

        // Append to extra args.
        wcsncat(extraArgs, L" ", ARRAYSIZE(extraArgs) - wcslen(extraArgs) - 1);
        wcsncat(extraArgs, argv[i], ARRAYSIZE(extraArgs) - wcslen(extraArgs) - 1);
    }

    // ---- Build command line ------------------------------------------------
    // When lpApplicationName is set, CreateProcessW still uses lpCommandLine as-is for
    // GetCommandLine() inside the child — argv[0] is the first whitespace-delimited token.
    // We must include the exe path (quoted) as argv[0] so UE3's cmdline parser sees the
    // map URL as argv[1], not argv[0] (which it skips as the process name).
    WCHAR gameCmdLine[MAX_PATH * 4];
    if (openolHost[0])
    {
        // Cold-launch P2P join: exe as argv[0], map URL as argv[1] (UE3 positional), then switches.
        _snwprintf(gameCmdLine, ARRAYSIZE(gameCmdLine),
            L"\"%s\" DLC_Intro_Persistent?game=Multiplayer.MultiplayerGame"
            L" -installed -nohomedir -seekfreeloadingpcconsole"
            L" -openol_host %s -openol_room %s%s",
            olGameExe, openolHost, openolRoom[0] ? openolRoom : L"DEFAULT", extraArgs);
    }
    else
    {
        _snwprintf(gameCmdLine, ARRAYSIZE(gameCmdLine),
            L"\"%s\" -installed -nohomedir -seekfreeloadingpcconsole%s",
            olGameExe, extraArgs);
    }

    PROCESS_INFORMATION processInformation = {};
    STARTUPINFOW startupInfo = {};
    startupInfo.cb = sizeof(startupInfo);

    BOOL bOk = CreateProcessW(olGameExe, gameCmdLine,
        NULL, NULL, FALSE, 0, NULL, NULL, &startupInfo, &processInformation);

    if (!bOk)
    {
        DWORD errCode = GetLastError();
        fprintf(stderr, "OutlastLauncher: CreateProcess failed with error code: 0x%08x", errCode);
        return errCode;
    }

    CloseHandle(processInformation.hProcess);
    CloseHandle(processInformation.hThread);

    return 0;
}
