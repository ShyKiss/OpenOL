// OutlastLauncher.cpp : Defines the entry point for the application.

#include "stdafx.h"
#include "OutlastLauncher.h"
#include "Updater.h"

// ---- Entry point -----------------------------------------------------------

int _tmain(int argc, _TCHAR* argv[])
{
    WCHAR openolHost[64]  = {};   // SteamID of host
    WCHAR openolRoom[64]  = {};   // Room code (default "DEFAULT")

    for (int i = 1; i < argc; i++)
    {
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

    // ---- Check for updates --------------------------------------------------
    if (Updater_Run(gameDir, exePath))
        return 0; // The updated launcher has taken over.

    // Locate OLGame.exe — try next to the launcher first, then relative paths for other layouts
    WCHAR olGameExe[MAX_PATH];
    PathCombineW(olGameExe, gameDir, L"OLGame.exe");
    if (GetFileAttributesW(olGameExe) == INVALID_FILE_ATTRIBUTES)
        PathCombineW(olGameExe, gameDir, L"Binaries\\Win64\\OLGame.exe");
    if (GetFileAttributesW(olGameExe) == INVALID_FILE_ATTRIBUTES)
        PathCombineW(olGameExe, gameDir, L"..\\..\\Binaries\\Win64\\OLGame.exe");

    // ---- Collect extra args from Steam launch options ----------------------
    // Any argv tokens not consumed by our own parsing are forwarded verbatim.
    // Known tokens we consume: +openol <host> <room>, bare connect strings.
    // Everything else (e.g. -dx11, -windowed, custom user flags) passes through.
    WCHAR extraArgs[1024] = {};
    for (int i = 1; i < argc; i++)
    {
        // Skip tokens we already handled.
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
