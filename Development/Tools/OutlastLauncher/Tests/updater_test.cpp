// updater_test.cpp : Command-line access to the updater internals for test_updater.py.
// Each command prints "OK" or "FAIL: <error>" and returns 0 or 1.

#include "../stdafx.h"
#include "../Updater.cpp"

static int Finish(bool ok)
{
    if (ok) wprintf(L"OK\n");
    else    wprintf(L"FAIL: %s\n", GError);
    return ok ? 0 : 1;
}

static bool ParseHex(LPCWSTR hex, BYTE out[32])
{
    if (wcslen(hex) != 64) return false;
    for (int i = 0; i < 32; i++)
    {
        int hi = HexDigit((char)hex[i * 2]), lo = HexDigit((char)hex[i * 2 + 1]);
        if (hi < 0 || lo < 0) return false;
        out[i] = (BYTE)(hi << 4 | lo);
    }
    return true;
}

int wmain(int argc, WCHAR* argv[])
{
    if (argc < 3) return 2;
    LPCWSTR cmd = argv[1];

    // extract <zip> <gameDir>
    if (wcscmp(cmd, L"extract") == 0 && argc == 4)
    {
        WCHAR dir[MAX_PATH];
        UpdatePath(dir, argv[3], L"x");
        CreateParentDirs(dir);
        return Finish(ZipExtractFile(argv[2], argv[3]));
    }

    // download <url> <size> <sha256> <dest>
    if (wcscmp(cmd, L"download") == 0 && argc == 6)
    {
        static UpdateInfo info;
        wcsncpy(info.url, argv[2], ARRAYSIZE(info.url) - 1);
        info.size = _wcstoui64(argv[3], NULL, 10);
        if (!ParseHex(argv[4], info.sha256)) return 2;
        return Finish(DownloadVerified(info, argv[5]));
    }

    // apply <gameDir> [locked file]: the file, if given, is held open for the whole install.
    if (wcscmp(cmd, L"apply") == 0 && argc >= 3)
    {
        HANDLE hLock = INVALID_HANDLE_VALUE;
        if (argc == 4)
        {
            hLock = CreateFileW(argv[3], GENERIC_READ, 0, NULL, OPEN_EXISTING, 0, NULL);
            if (hLock == INVALID_HANDLE_VALUE) return 2;
        }
        bool ok = ApplyStaged(argv[2]);
        if (hLock != INVALID_HANDLE_VALUE) CloseHandle(hLock);
        return Finish(ok);
    }

    // apply-self <gameDir>: replaces the running test exe, then restores it through the rollback path.
    if (wcscmp(cmd, L"apply-self") == 0 && argc == 3)
    {
        if (!ApplyStaged(argv[2])) return Finish(false);
        return Finish(true);
    }

    // recover <gameDir>
    if (wcscmp(cmd, L"recover") == 0 && argc == 3)
        return Finish(RecoverInterruptedInstall(argv[2]));

    // parse <json file>: prints the fields found.
    if (wcscmp(cmd, L"parse") == 0 && argc == 3)
    {
        FILE* f = _wfopen(argv[2], L"rb");
        if (!f) return 2;
        std::vector<char> json(MAX_JSON_SIZE);
        size_t len = fread(&json[0], 1, json.size(), f);
        fclose(f);

        static UpdateInfo info;
        if (!ParseRelease(&json[0], &json[0] + len, &info)) return Finish(false);
        wprintf(L"%s %I64u %s ", info.tag, info.size, info.url);
        for (int i = 0; i < 32; i++) wprintf(L"%02x", info.sha256[i]);
        wprintf(L"\n");
        return Finish(true);
    }

    // parse-list <json file>: prints the release that would be offered to a release candidate.
    if (wcscmp(cmd, L"parse-list") == 0 && argc == 3)
    {
        FILE* f = _wfopen(argv[2], L"rb");
        if (!f) return 2;
        std::vector<char> json(MAX_JSON_SIZE);
        size_t len = fread(&json[0], 1, json.size(), f);
        fclose(f);

        static UpdateInfo info;
        if (!ParseReleaseList(&json[0], &json[0] + len, &info)) return Finish(false);
        wprintf(L"%s\n", info.tag);
        return Finish(true);
    }

    // newer <a> <b>
    if (wcscmp(cmd, L"newer") == 0 && argc == 4)
    {
        GError[0] = L'\0';
        return Finish(VersionNewer(argv[2], argv[3]) != FALSE);
    }

    return 2;
}
