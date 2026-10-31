// Updater.cpp : Installs new releases from GitHub.
//
// Everything used here ships with both Windows and Wine/Proton. The Shell zip folder
// (IShellDispatch) must not be used: Wine has no handler for it, so extraction would
// always fail under Proton. Archives are unpacked by the inflate code below instead.
//
// The game folder is not touched until the archive has passed every check:
//   1. Download to <game>\OpenOL_Update, verify its size and SHA-256.
//   2. Extract to staged\, verify each file's CRC-32.
//   3. Create apply.marker, then for each file rename the original into backup\ and
//      the staged file into place. Renames work on a running exe, so the launcher can
//      replace itself. An error undoes the renames made so far.
//   4. Delete apply.marker. This is the commit point.
// If the process dies during step 3, the marker is still there on the next start and
// the originals are restored from backup\.

#include "stdafx.h"
#include "Updater.h"

#include <string>
#include <vector>

// ---- Version ---------------------------------------------------------------
#ifndef CURRENT_VERSION
#define CURRENT_VERSION   L"2.1.0"
#endif
#define GITHUB_REPO_PATH  L"ShyKiss/OpenOL"

#ifndef RELEASE_API_URL
#define RELEASE_API_URL   L"https://api.github.com/repos/" GITHUB_REPO_PATH L"/releases/latest"
#define DOWNLOAD_PREFIX   L"https://github.com/" GITHUB_REPO_PATH L"/releases/download/"
#endif
#define USER_AGENT        L"OutlastLauncher/" CURRENT_VERSION
#define UPDATE_TITLE      L"OpenOL Update"

// Must match the zip built by publish_production.sh. 2.1.0 launchers look for ".zip" too.
#define ASSET_NAME        "OpenOL.zip"

#define UPDATE_DIR        L"OpenOL_Update"
#define INSTALLED_TAG_REL L"Binaries\\Win64\\OpenOL\\installed_version.txt"

static const DWORD CHECK_WAIT_MS      = 10000; // The game must not wait longer than this for GitHub.
static const DWORD CONNECT_TIMEOUT_MS = 5000;
static const DWORD RECEIVE_TIMEOUT_MS = 30000;
static const int   DOWNLOAD_ATTEMPTS  = 3;

static const DWORD      MAX_JSON_SIZE   = 8u * 1024 * 1024;
static const ULONGLONG  MAX_ASSET_SIZE  = 2048ull * 1024 * 1024;
static const DWORD      MAX_ENTRY_SIZE  = 1024u * 1024 * 1024;
static const ULONGLONG  MAX_TOTAL_SIZE  = 8192ull * 1024 * 1024;

struct UpdateInfo
{
    WCHAR     tag[64];
    WCHAR     url[1024];
    BYTE      sha256[32];
    ULONGLONG size;
};

enum UpdateStage { STAGE_DOWNLOAD, STAGE_VERIFY, STAGE_EXTRACT, STAGE_APPLY };

static WCHAR         GError[1024];
static volatile LONG GCancel = 0;
static volatile LONG GStage  = STAGE_DOWNLOAD;
static HWND          GProgressWnd = NULL;

// ---- Helpers ---------------------------------------------------------------

static void SetError(LPCWSTR fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    _vsnwprintf(GError, ARRAYSIZE(GError) - 1, fmt, args);
    GError[ARRAYSIZE(GError) - 1] = L'\0';
    va_end(args);
}

static bool IsCancelled()
{
    if (!GCancel) return false;
    SetError(L"Cancelled.");
    return true;
}

static void ReportProgress(UpdateStage stage, ULONGLONG done, ULONGLONG total)
{
    InterlockedExchange(&GStage, stage);
    if (GProgressWnd)
        PostMessageW(GProgressWnd, WM_APP, (WPARAM)stage, total ? (LPARAM)(done * 100 / total) : 0);
}

// Joins path parts with '\'. Fails instead of truncating when the result does not fit.
static bool JoinPath(WCHAR out[MAX_PATH], LPCWSTR a, LPCWSTR b, LPCWSTR c = NULL, LPCWSTR d = NULL)
{
    LPCWSTR parts[4] = { a, b, c, d };
    size_t pos = 0;
    for (int i = 0; i < 4 && parts[i]; i++)
    {
        size_t len = wcslen(parts[i]);
        if (pos + len + 2 > MAX_PATH)
        {
            SetError(L"Path is too long: %s\\%s", a, b);
            return false;
        }
        if (i > 0 && pos > 0 && out[pos - 1] != L'\\')
            out[pos++] = L'\\';
        memcpy(out + pos, parts[i], len * sizeof(WCHAR));
        pos += len;
    }
    out[pos] = L'\0';
    return true;
}

static bool UpdatePath(WCHAR out[MAX_PATH], LPCWSTR gameDir, LPCWSTR sub, LPCWSTR rel = NULL)
{
    WCHAR dir[MAX_PATH];
    return JoinPath(dir, gameDir, UPDATE_DIR) && JoinPath(out, dir, sub, rel);
}

static bool PathExists(LPCWSTR path)
{
    return GetFileAttributesW(path) != INVALID_FILE_ATTRIBUTES;
}

static bool IsDirectory(LPCWSTR path)
{
    DWORD attrs = GetFileAttributesW(path);
    return attrs != INVALID_FILE_ATTRIBUTES && (attrs & FILE_ATTRIBUTE_DIRECTORY);
}

// Creates all missing directories leading to the file at |path|.
static bool CreateParentDirs(LPCWSTR path)
{
    WCHAR dir[MAX_PATH];
    wcsncpy(dir, path, MAX_PATH - 1);
    dir[MAX_PATH - 1] = L'\0';

    WCHAR* last = wcsrchr(dir, L'\\');
    if (!last) return true;
    *last = L'\0';

    for (WCHAR* p = dir + 1; *p; p++)
    {
        if (*p != L'\\') continue;
        *p = L'\0';
        CreateDirectoryW(dir, NULL); // Fails for drive roots and existing directories.
        *p = L'\\';
    }
    CreateDirectoryW(dir, NULL);
    if (IsDirectory(dir)) return true;

    SetError(L"Cannot create folder: %s (error %lu)", dir, GetLastError());
    return false;
}

// Antivirus and indexers hold files open for short periods, so sharing errors are retried.
static bool MoveWithRetry(LPCWSTR src, LPCWSTR dst, DWORD flags)
{
    DWORD err = 0;
    for (int i = 0; i < 15; i++)
    {
        if (MoveFileExW(src, dst, flags)) return true;
        err = GetLastError();
        if (err != ERROR_SHARING_VIOLATION && err != ERROR_ACCESS_DENIED && err != ERROR_LOCK_VIOLATION)
            break;
        Sleep(200);
    }
    SetError(L"Cannot move %s to %s (error %lu). Is the game still running?", src, dst, err);
    return false;
}

static void DeleteDirTree(LPCWSTR dir)
{
    WCHAR pattern[MAX_PATH];
    if (!JoinPath(pattern, dir, L"*")) return;

    WIN32_FIND_DATAW fd;
    HANDLE hFind = FindFirstFileW(pattern, &fd);
    if (hFind == INVALID_HANDLE_VALUE) return;

    do
    {
        if (wcscmp(fd.cFileName, L".") == 0 || wcscmp(fd.cFileName, L"..") == 0)
            continue;

        WCHAR path[MAX_PATH];
        if (!JoinPath(path, dir, fd.cFileName)) continue;

        if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
        {
            DeleteDirTree(path);
        }
        else
        {
            SetFileAttributesW(path, FILE_ATTRIBUTE_NORMAL);
            DeleteFileW(path);
        }
    }
    while (FindNextFileW(hFind, &fd));

    FindClose(hFind);
    RemoveDirectoryW(dir);
}

// Lists all files below |root\rel| as paths relative to |root|.
static bool ListFiles(LPCWSTR root, const std::wstring& rel, std::vector<std::wstring>& out)
{
    WCHAR dir[MAX_PATH], pattern[MAX_PATH];
    if (!JoinPath(dir, root, rel.c_str()) || !JoinPath(pattern, dir, L"*")) return false;

    WIN32_FIND_DATAW fd;
    HANDLE hFind = FindFirstFileW(pattern, &fd);
    if (hFind == INVALID_HANDLE_VALUE) return true;

    bool ok = true;
    do
    {
        if (wcscmp(fd.cFileName, L".") == 0 || wcscmp(fd.cFileName, L"..") == 0)
            continue;

        std::wstring child = rel.empty() ? std::wstring(fd.cFileName) : rel + L"\\" + fd.cFileName;
        if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
            ok = ListFiles(root, child, out) && ok;
        else
            out.push_back(child);
    }
    while (FindNextFileW(hFind, &fd));

    FindClose(hFind);
    return ok;
}

// Writes |data| to a new file. The data is flushed so that a power loss after the
// rename into place cannot leave an empty game file behind.
static bool WriteNewFile(LPCWSTR path, const BYTE* data, DWORD size)
{
    HANDLE hFile = CreateFileW(path, GENERIC_WRITE, 0, NULL, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE)
    {
        SetError(L"Cannot create %s (error %lu)", path, GetLastError());
        return false;
    }

    bool ok = true;
    DWORD offset = 0, err = 0;
    while (ok && offset < size)
    {
        DWORD chunk = size - offset < 1024u * 1024 ? size - offset : 1024u * 1024, written = 0;
        ok = WriteFile(hFile, data + offset, chunk, &written, NULL) && written == chunk;
        offset += written;
    }
    if (ok) ok = FlushFileBuffers(hFile) != FALSE;
    if (!ok) err = GetLastError();

    CloseHandle(hFile);
    if (!ok)
    {
        DeleteFileW(path);
        SetError(L"Cannot write %s (error %lu)", path, err);
    }
    return ok;
}

// ---- HTTP ------------------------------------------------------------------

static HINTERNET OpenSession()
{
    HINTERNET hInet = InternetOpenW(USER_AGENT, INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);
    if (!hInet)
    {
        SetError(L"Cannot initialize WinINet (error %lu)", GetLastError());
        return NULL;
    }
    DWORD connect = CONNECT_TIMEOUT_MS, receive = RECEIVE_TIMEOUT_MS;
    InternetSetOptionW(hInet, INTERNET_OPTION_CONNECT_TIMEOUT, &connect, sizeof(connect));
    InternetSetOptionW(hInet, INTERNET_OPTION_SEND_TIMEOUT,    &receive, sizeof(receive));
    InternetSetOptionW(hInet, INTERNET_OPTION_RECEIVE_TIMEOUT, &receive, sizeof(receive));
    return hInet;
}

// Opens |url|, following redirects. Anything but a final 200 is an error, because
// error pages must not be mistaken for the requested content.
static HINTERNET OpenUrl(HINTERNET hInet, LPCWSTR url, LPCWSTR headers)
{
    const DWORD flags = INTERNET_FLAG_RELOAD | INTERNET_FLAG_NO_CACHE_WRITE |
                        INTERNET_FLAG_NO_UI | INTERNET_FLAG_NO_COOKIES;

    HINTERNET hReq = InternetOpenUrlW(hInet, url, headers, headers ? (DWORD)-1 : 0, flags, 0);
    if (!hReq)
    {
        SetError(L"Cannot connect to the server (error %lu)", GetLastError());
        return NULL;
    }

    DWORD status = 0, len = sizeof(status);
    if (!HttpQueryInfoW(hReq, HTTP_QUERY_STATUS_CODE | HTTP_QUERY_FLAG_NUMBER, &status, &len, NULL)
        || status != 200)
    {
        SetError(L"Server returned HTTP status %lu", status);
        InternetCloseHandle(hReq);
        return NULL;
    }
    return hReq;
}

static bool HttpGetText(LPCWSTR url, LPCWSTR headers, std::vector<char>& out)
{
    HINTERNET hInet = OpenSession();
    if (!hInet) return false;

    bool ok = false;
    HINTERNET hReq = OpenUrl(hInet, url, headers);
    if (hReq)
    {
        char buf[16384];
        DWORD read = 0;
        while ((ok = InternetReadFile(hReq, buf, sizeof(buf), &read) != FALSE) && read > 0)
        {
            if (out.size() + read > MAX_JSON_SIZE)
            {
                SetError(L"Server response is too large.");
                ok = false;
                break;
            }
            out.insert(out.end(), buf, buf + read);
        }
        if (!ok && !GError[0])
            SetError(L"Connection lost (error %lu)", GetLastError());
        InternetCloseHandle(hReq);
    }
    InternetCloseHandle(hInet);
    return ok;
}

// ---- JSON ------------------------------------------------------------------
// Just enough to walk the release object. Values are skipped by structure, so text
// inside the release notes can never be mistaken for a field.

static const char* JsonSkipWs(const char* p, const char* end)
{
    while (p && p < end && (*p == ' ' || *p == '\t' || *p == '\r' || *p == '\n')) p++;
    return p;
}

// |p| is at the opening quote; returns the position after the closing quote.
static const char* JsonSkipString(const char* p, const char* end)
{
    for (p++; p < end; p++)
    {
        if (*p == '\\') { p++; continue; }
        if (*p == '"') return p + 1;
    }
    return NULL;
}

static const char* JsonSkipValue(const char* p, const char* end)
{
    p = JsonSkipWs(p, end);
    if (!p || p >= end) return NULL;
    if (*p == '"') return JsonSkipString(p, end);

    if (*p == '{' || *p == '[')
    {
        int depth = 0;
        while (p < end)
        {
            if (*p == '"')
            {
                p = JsonSkipString(p, end);
                if (!p) return NULL;
                continue;
            }
            if (*p == '{' || *p == '[') depth++;
            if (*p == '}' || *p == ']') depth--;
            p++;
            if (depth == 0) return p;
        }
        return NULL;
    }

    while (p < end && *p != ',' && *p != '}' && *p != ']' && *p != ' ' && *p != '\r' && *p != '\n') p++;
    return p;
}

// Returns the value of |key| in the object starting at |obj|, or NULL. Nested objects are not searched.
static const char* JsonMember(const char* obj, const char* end, const char* key)
{
    const char* p = JsonSkipWs(obj, end);
    if (!p || p >= end || *p != '{') return NULL;
    p++;

    size_t keyLen = strlen(key);
    for (;;)
    {
        p = JsonSkipWs(p, end);
        if (!p || p >= end || *p != '"') return NULL;

        const char* nameStart = p + 1;
        p = JsonSkipString(p, end);
        if (!p) return NULL;
        bool match = (size_t)(p - 1 - nameStart) == keyLen && memcmp(nameStart, key, keyLen) == 0;

        p = JsonSkipWs(p, end);
        if (!p || p >= end || *p != ':') return NULL;
        p = JsonSkipWs(p + 1, end);
        if (match) return p;

        p = JsonSkipWs(JsonSkipValue(p, end), end);
        if (!p || p >= end || *p != ',') return NULL;
        p++;
    }
}

// Copies a string value. The fields read here never contain escapes, so one is an error.
static bool JsonGetString(const char* value, const char* end, char* out, size_t outLen)
{
    if (!value || value >= end || *value != '"') return false;
    const char* close = JsonSkipString(value, end);
    if (!close) return false;

    size_t len = (size_t)(close - 1 - (value + 1));
    if (len >= outLen || memchr(value + 1, '\\', len)) return false;
    memcpy(out, value + 1, len);
    out[len] = '\0';
    return true;
}

static bool JsonMemberString(const char* obj, const char* end, const char* key, char* out, size_t outLen)
{
    return JsonGetString(JsonMember(obj, end, key), end, out, outLen);
}

static int HexDigit(char c)
{
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    return -1;
}

// Fills |info| from a GitHub "release" object.
static bool ParseRelease(const char* json, const char* end, UpdateInfo* info)
{
    ZeroMemory(info, sizeof(*info));

    char tag[64];
    if (!JsonMemberString(json, end, "tag_name", tag, sizeof(tag)))
    {
        SetError(L"Release has no tag.");
        return false;
    }
    MultiByteToWideChar(CP_UTF8, 0, tag, -1, info->tag, ARRAYSIZE(info->tag));

    const char* p = JsonSkipWs(JsonMember(json, end, "assets"), end);
    if (!p || p >= end || *p != '[')
    {
        SetError(L"Release has no asset list.");
        return false;
    }

    for (p++; ; p++)
    {
        p = JsonSkipWs(p, end);
        if (!p || p >= end || *p != '{') break;

        char name[128], state[32], digest[80], url[1024], size[32];
        if (JsonMemberString(p, end, "name", name, sizeof(name)) && strcmp(name, ASSET_NAME) == 0)
        {
            if (!JsonMemberString(p, end, "state", state, sizeof(state)) || strcmp(state, "uploaded") != 0
                || !JsonMemberString(p, end, "browser_download_url", url, sizeof(url))
                || !JsonMemberString(p, end, "digest", digest, sizeof(digest))
                || strlen(digest) != 7 + 64 || strncmp(digest, "sha256:", 7) != 0)
            {
                SetError(L"Release asset is incomplete or has no SHA-256 digest.");
                return false;
            }

            for (int i = 0; i < 32; i++)
            {
                int hi = HexDigit(digest[7 + i * 2]), lo = HexDigit(digest[8 + i * 2]);
                if (hi < 0 || lo < 0)
                {
                    SetError(L"Release asset has a malformed digest.");
                    return false;
                }
                info->sha256[i] = (BYTE)(hi << 4 | lo);
            }

            const char* sizeValue = JsonMember(p, end, "size");
            size_t sizeLen = 0;
            while (sizeValue && sizeValue + sizeLen < end && sizeLen < sizeof(size) - 1
                   && sizeValue[sizeLen] >= '0' && sizeValue[sizeLen] <= '9')
            {
                size[sizeLen] = sizeValue[sizeLen];
                sizeLen++;
            }
            size[sizeLen] = '\0';
            info->size = _strtoui64(size, NULL, 10);
            if (info->size == 0 || info->size > MAX_ASSET_SIZE)
            {
                SetError(L"Release asset has an invalid size.");
                return false;
            }

            MultiByteToWideChar(CP_UTF8, 0, url, -1, info->url, ARRAYSIZE(info->url));
            if (_wcsnicmp(info->url, DOWNLOAD_PREFIX, wcslen(DOWNLOAD_PREFIX)) != 0)
            {
                SetError(L"Release asset has an unexpected URL: %s", info->url);
                return false;
            }
            return true;
        }

        p = JsonSkipWs(JsonSkipValue(p, end), end);
        if (!p || p >= end || *p != ',') break;
    }

    SetError(L"Release has no " TEXT(ASSET_NAME) L" asset.");
    return false;
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

// ---- Download --------------------------------------------------------------

// One download attempt, hashing while writing. Sets |retry| if another attempt could help.
static bool DownloadOnce(HINTERNET hInet, const UpdateInfo& info, LPCWSTR destPath, bool& retry)
{
    retry = true;

    HCRYPTPROV hProv = 0;
    HCRYPTHASH hHash = 0;
    if (!CryptAcquireContextW(&hProv, NULL, NULL, PROV_RSA_AES, CRYPT_VERIFYCONTEXT))
    {
        SetError(L"Cannot initialize CryptoAPI (error %lu)", GetLastError());
        retry = false;
        return false;
    }
    if (!CryptCreateHash(hProv, CALG_SHA_256, 0, 0, &hHash))
    {
        SetError(L"SHA-256 is not available (error %lu)", GetLastError());
        CryptReleaseContext(hProv, 0);
        retry = false;
        return false;
    }

    bool ok = false;
    HANDLE hFile = INVALID_HANDLE_VALUE;
    HINTERNET hReq = OpenUrl(hInet, info.url, NULL);
    if (hReq)
    {
        hFile = CreateFileW(destPath, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
        if (hFile == INVALID_HANDLE_VALUE)
        {
            SetError(L"Cannot create %s (error %lu)", destPath, GetLastError());
            retry = false;
        }
    }

    if (hFile != INVALID_HANDLE_VALUE)
    {
        static char buf[65536];
        ULONGLONG total = 0;
        DWORD read = 0;
        ok = true;
        for (;;)
        {
            if (IsCancelled()) { ok = false; retry = false; break; }
            if (!InternetReadFile(hReq, buf, sizeof(buf), &read))
            {
                SetError(L"Connection lost (error %lu)", GetLastError());
                ok = false;
                break;
            }
            if (read == 0) break;

            DWORD written = 0;
            if (!WriteFile(hFile, buf, read, &written, NULL) || written != read)
            {
                SetError(L"Cannot write %s (error %lu)", destPath, GetLastError());
                ok = false;
                retry = false;
                break;
            }
            CryptHashData(hHash, (const BYTE*)buf, read, 0);
            total += read;
            ReportProgress(STAGE_DOWNLOAD, total, info.size);
        }
        CloseHandle(hFile);

        if (ok && total != info.size)
        {
            SetError(L"Incomplete download: got %I64u of %I64u bytes.", total, info.size);
            ok = false;
        }
    }
    if (hReq) InternetCloseHandle(hReq);

    if (ok)
    {
        BYTE digest[32];
        DWORD digestLen = sizeof(digest);
        if (!CryptGetHashParam(hHash, HP_HASHVAL, digest, &digestLen, 0)
            || digestLen != sizeof(digest) || memcmp(digest, info.sha256, sizeof(digest)) != 0)
        {
            SetError(L"Downloaded file failed the SHA-256 check.");
            ok = false;
        }
    }

    CryptDestroyHash(hHash);
    CryptReleaseContext(hProv, 0);
    return ok;
}

static bool DownloadVerified(const UpdateInfo& info, LPCWSTR destPath)
{
    HINTERNET hInet = OpenSession();
    if (!hInet) return false;

    bool ok = false, retry = true;
    for (int attempt = 0; attempt < DOWNLOAD_ATTEMPTS && !ok && retry; attempt++)
    {
        if (attempt > 0) Sleep(1000);
        ok = DownloadOnce(hInet, info, destPath, retry);
    }
    InternetCloseHandle(hInet);
    return ok;
}

// ---- Inflate (RFC 1951) ----------------------------------------------------

struct InflateState
{
    const BYTE* in;
    size_t      inLen, inPos;
    BYTE*       out;
    size_t      outLen, outPos;
    DWORD       bitBuf;
    int         bitCnt;
    bool        bad;
};

struct Huffman
{
    short count[16];
    short symbol[288];
};

static DWORD InflateBits(InflateState& s, int need)
{
    DWORD val = s.bitBuf;
    while (s.bitCnt < need)
    {
        if (s.inPos >= s.inLen) { s.bad = true; return 0; }
        val |= (DWORD)s.in[s.inPos++] << s.bitCnt;
        s.bitCnt += 8;
    }
    s.bitBuf = val >> need;
    s.bitCnt -= need;
    return val & ((1u << need) - 1);
}

// Returns the next symbol, or -1 if the code is invalid or the input ran out.
static int InflateDecode(InflateState& s, const Huffman& h)
{
    int code = 0, first = 0, index = 0;
    DWORD bitBuf = s.bitBuf;
    int bitCnt = s.bitCnt;

    for (int len = 1; len <= 15; len++)
    {
        if (bitCnt == 0)
        {
            if (s.inPos >= s.inLen) { s.bad = true; return -1; }
            bitBuf = s.in[s.inPos++];
            bitCnt = 8;
        }
        code |= bitBuf & 1;
        bitBuf >>= 1;
        bitCnt--;

        int count = h.count[len];
        if (code - count < first)
        {
            s.bitBuf = bitBuf;
            s.bitCnt = bitCnt;
            return h.symbol[index + (code - first)];
        }
        index += count;
        first += count;
        first <<= 1;
        code <<= 1;
    }
    s.bad = true;
    return -1;
}

// Returns 0 for a complete code, >0 if incomplete, <0 if over-subscribed.
static int InflateBuild(Huffman& h, const short* lengths, int n)
{
    ZeroMemory(h.count, sizeof(h.count));
    for (int i = 0; i < n; i++) h.count[lengths[i]]++;
    if (h.count[0] == n) return 0;

    int left = 1;
    for (int len = 1; len <= 15; len++)
    {
        left *= 2;
        left -= h.count[len];
        if (left < 0) return left;
    }

    short offs[16];
    offs[1] = 0;
    for (int len = 1; len < 15; len++) offs[len + 1] = offs[len] + h.count[len];
    for (int i = 0; i < n; i++)
        if (lengths[i] != 0) h.symbol[offs[lengths[i]]++] = (short)i;
    return left;
}

static bool InflateCodes(InflateState& s, const Huffman& lenCode, const Huffman& distCode)
{
    static const short lens[29] = { 3, 4, 5, 6, 7, 8, 9, 10, 11, 13, 15, 17, 19, 23, 27, 31,
        35, 43, 51, 59, 67, 83, 99, 115, 131, 163, 195, 227, 258 };
    static const short lext[29] = { 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 2, 2, 2, 2,
        3, 3, 3, 3, 4, 4, 4, 4, 5, 5, 5, 5, 0 };
    static const short dists[30] = { 1, 2, 3, 4, 5, 7, 9, 13, 17, 25, 33, 49, 65, 97, 129, 193,
        257, 385, 513, 769, 1025, 1537, 2049, 3073, 4097, 6145, 8193, 12289, 16385, 24577 };
    static const short dext[30] = { 0, 0, 0, 0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6,
        7, 7, 8, 8, 9, 9, 10, 10, 11, 11, 12, 12, 13, 13 };

    for (;;)
    {
        int sym = InflateDecode(s, lenCode);
        if (sym < 0) return false;

        if (sym < 256)
        {
            if (s.outPos >= s.outLen) return false;
            s.out[s.outPos++] = (BYTE)sym;
        }
        else if (sym == 256)
        {
            return true;
        }
        else
        {
            sym -= 257;
            if (sym >= 29) return false;
            size_t len = lens[sym] + InflateBits(s, lext[sym]);

            sym = InflateDecode(s, distCode);
            if (sym < 0 || sym >= 30) return false;
            size_t dist = dists[sym] + InflateBits(s, dext[sym]);

            if (s.bad || dist > s.outPos || len > s.outLen - s.outPos) return false;

            // Source and destination may overlap: this must copy byte by byte.
            BYTE* to = s.out + s.outPos;
            const BYTE* from = to - dist;
            for (size_t i = 0; i < len; i++) to[i] = from[i];
            s.outPos += len;
        }
    }
}

static bool InflateStored(InflateState& s)
{
    s.bitBuf = 0;
    s.bitCnt = 0;
    if (s.inLen - s.inPos < 4) return false;

    size_t len  = s.in[s.inPos] | (s.in[s.inPos + 1] << 8);
    size_t nlen = s.in[s.inPos + 2] | (s.in[s.inPos + 3] << 8);
    s.inPos += 4;
    if (len != (~nlen & 0xFFFF) || len > s.inLen - s.inPos || len > s.outLen - s.outPos)
        return false;

    memcpy(s.out + s.outPos, s.in + s.inPos, len);
    s.inPos  += len;
    s.outPos += len;
    return true;
}

static bool InflateFixed(InflateState& s)
{
    static Huffman lenCode, distCode;
    static bool built = false;
    if (!built)
    {
        short lengths[288 + 30];
        int i = 0;
        for (; i < 144; i++) lengths[i] = 8;
        for (; i < 256; i++) lengths[i] = 9;
        for (; i < 280; i++) lengths[i] = 7;
        for (; i < 288; i++) lengths[i] = 8;
        InflateBuild(lenCode, lengths, 288);
        for (i = 0; i < 30; i++) lengths[i] = 5;
        InflateBuild(distCode, lengths, 30);
        built = true;
    }
    return InflateCodes(s, lenCode, distCode);
}

static bool InflateDynamic(InflateState& s)
{
    static const short order[19] = { 16, 17, 18, 0, 8, 7, 9, 6, 10, 5, 11, 4, 12, 3, 13, 2, 14, 1, 15 };

    int nlen  = InflateBits(s, 5) + 257;
    int ndist = InflateBits(s, 5) + 1;
    int ncode = InflateBits(s, 4) + 4;
    if (s.bad || nlen > 286 || ndist > 30) return false;

    short lengths[320];
    int index = 0;
    for (; index < ncode; index++) lengths[order[index]] = (short)InflateBits(s, 3);
    for (; index < 19; index++)    lengths[order[index]] = 0;
    if (s.bad) return false;

    static Huffman lenCode, distCode;
    if (InflateBuild(lenCode, lengths, 19) != 0) return false;

    index = 0;
    while (index < nlen + ndist)
    {
        int sym = InflateDecode(s, lenCode);
        if (sym < 0) return false;

        if (sym < 16)
        {
            lengths[index++] = (short)sym;
        }
        else
        {
            short prev = 0;
            int repeat;
            if (sym == 16)
            {
                if (index == 0) return false;
                prev = lengths[index - 1];
                repeat = 3 + InflateBits(s, 2);
            }
            else if (sym == 17) repeat = 3 + InflateBits(s, 3);
            else                repeat = 11 + InflateBits(s, 7);

            if (s.bad || index + repeat > nlen + ndist) return false;
            while (repeat--) lengths[index++] = prev;
        }
    }
    if (lengths[256] == 0) return false;

    int err = InflateBuild(lenCode, lengths, nlen);
    if (err < 0 || (err > 0 && nlen - lenCode.count[0] != 1)) return false;

    err = InflateBuild(distCode, lengths + nlen, ndist);
    if (err < 0 || (err > 0 && ndist - distCode.count[0] != 1)) return false;

    return InflateCodes(s, lenCode, distCode);
}

static bool Inflate(const BYTE* in, size_t inLen, BYTE* out, size_t outLen)
{
    InflateState s;
    ZeroMemory(&s, sizeof(s));
    s.in = in;     s.inLen = inLen;
    s.out = out;   s.outLen = outLen;

    DWORD last;
    do
    {
        last = InflateBits(s, 1);
        DWORD type = InflateBits(s, 2);
        if (s.bad) return false;

        bool ok;
        if (type == 0)      ok = InflateStored(s);
        else if (type == 1) ok = InflateFixed(s);
        else if (type == 2) ok = InflateDynamic(s);
        else                ok = false;
        if (!ok || s.bad) return false;
    }
    while (!last);

    return s.outPos == outLen;
}

// ---- CRC-32 ----------------------------------------------------------------

static DWORD Crc32(const BYTE* data, size_t size)
{
    static DWORD table[256];
    static bool built = false;
    if (!built)
    {
        for (DWORD i = 0; i < 256; i++)
        {
            DWORD c = i;
            for (int k = 0; k < 8; k++) c = (c & 1) ? 0xEDB88320u ^ (c >> 1) : c >> 1;
            table[i] = c;
        }
        built = true;
    }

    DWORD crc = 0xFFFFFFFFu;
    for (size_t i = 0; i < size; i++) crc = table[(crc ^ data[i]) & 0xFF] ^ (crc >> 8);
    return crc ^ 0xFFFFFFFFu;
}

// ---- Zip -------------------------------------------------------------------

static DWORD ReadLE16(const BYTE* p) { return p[0] | (p[1] << 8); }
static DWORD ReadLE32(const BYTE* p) { return p[0] | (p[1] << 8) | (p[2] << 16) | ((DWORD)p[3] << 24); }

struct ZipEntry
{
    WCHAR       name[MAX_PATH];
    const BYTE* data;
    DWORD       packedSize, size, crc, method;
    bool        isDir;
};

struct ZipCursor
{
    const BYTE* zip;
    size_t      zipSize;
    size_t      pos;
    DWORD       remaining;
};

// Accepts only plain relative paths inside the game folder, and converts them to
// backslashes. Archive contents must never be able to write anywhere else.
static bool SanitizeEntryName(WCHAR* name)
{
    if (!name[0]) return false;
    for (WCHAR* p = name; *p; p++)
    {
        if (*p == L'/') *p = L'\\';
        if (*p < 32 || wcschr(L":*?\"<>|", *p)) return false;
    }

    static const WCHAR* reserved[] = { L"CON", L"PRN", L"AUX", L"NUL" };
    for (WCHAR* part = name; ; )
    {
        WCHAR* sep = wcschr(part, L'\\');
        size_t len = sep ? (size_t)(sep - part) : wcslen(part);

        if (len == 0 || part[len - 1] == L'.' || part[len - 1] == L' ') return false; // Also rejects "." and "..".

        size_t stem = 0;
        while (stem < len && part[stem] != L'.') stem++;
        for (int i = 0; i < ARRAYSIZE(reserved); i++)
            if (stem == 3 && _wcsnicmp(part, reserved[i], 3) == 0) return false;
        if (stem == 4 && (_wcsnicmp(part, L"COM", 3) == 0 || _wcsnicmp(part, L"LPT", 3) == 0)
            && part[3] >= L'0' && part[3] <= L'9') return false;

        if (!sep) break;
        part = sep + 1;
    }

    // The archive must not be able to reach into the update's own working files.
    size_t dirLen = wcslen(UPDATE_DIR);
    if (_wcsnicmp(name, UPDATE_DIR, dirLen) == 0 && (name[dirLen] == L'\\' || name[dirLen] == L'\0'))
        return false;
    return true;
}

// Locates the central directory. Only single-disk archives without ZIP64 are valid.
static bool ZipOpen(const BYTE* zip, size_t zipSize, ZipCursor* cur)
{
    if (zipSize < 22) { SetError(L"Archive is too small."); return false; }

    size_t lowest = zipSize > 22 + 65535 ? zipSize - 22 - 65535 : 0;
    for (size_t pos = zipSize - 22; ; pos--)
    {
        if (ReadLE32(zip + pos) == 0x06054B50 && pos + 22 + ReadLE16(zip + pos + 20) == zipSize)
        {
            DWORD count  = ReadLE16(zip + pos + 10);
            DWORD cdSize = ReadLE32(zip + pos + 12);
            DWORD cdOff  = ReadLE32(zip + pos + 16);
            if (ReadLE16(zip + pos + 4) != 0 || ReadLE16(zip + pos + 6) != 0
                || count != ReadLE16(zip + pos + 8) || count == 0xFFFF || cdOff == 0xFFFFFFFFu
                || (ULONGLONG)cdOff + cdSize > pos)
            {
                SetError(L"Archive format is not supported.");
                return false;
            }
            cur->zip = zip;
            cur->zipSize = zipSize;
            cur->pos = cdOff;
            cur->remaining = count;
            return true;
        }
        if (pos == lowest) break;
    }
    SetError(L"Archive is damaged: end of central directory not found.");
    return false;
}

// Reads and validates the next entry. Returns false on a malformed one.
static bool ZipNext(ZipCursor* cur, ZipEntry* e)
{
    const BYTE* zip = cur->zip;
    size_t pos = cur->pos;

    if (cur->zipSize < 46 || pos > cur->zipSize - 46 || ReadLE32(zip + pos) != 0x02014B50)
    {
        SetError(L"Archive is damaged: bad central directory.");
        return false;
    }

    DWORD madeBy   = ReadLE16(zip + pos + 4);
    DWORD flags    = ReadLE16(zip + pos + 8);
    e->method      = ReadLE16(zip + pos + 10);
    e->crc         = ReadLE32(zip + pos + 16);
    e->packedSize  = ReadLE32(zip + pos + 20);
    e->size        = ReadLE32(zip + pos + 24);
    DWORD nameLen  = ReadLE16(zip + pos + 28);
    DWORD extraLen = ReadLE16(zip + pos + 30);
    DWORD noteLen  = ReadLE16(zip + pos + 32);
    DWORD attrs    = ReadLE32(zip + pos + 38);
    DWORD localOff = ReadLE32(zip + pos + 42);

    size_t next = pos + 46 + nameLen + extraLen + noteLen;
    if (next > cur->zipSize)
    {
        SetError(L"Archive is damaged: bad central directory.");
        return false;
    }

    int wlen = nameLen == 0 ? 0 : MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS,
        (const char*)zip + pos + 46, nameLen, e->name, MAX_PATH - 1);
    e->name[wlen] = L'\0';

    e->isDir = wlen > 0 && (e->name[wlen - 1] == L'/' || e->name[wlen - 1] == L'\\');
    if (e->isDir) e->name[wlen - 1] = L'\0';

    if (wlen == 0 || !SanitizeEntryName(e->name))
    {
        SetError(L"Archive contains an unsafe file name: %s", e->name);
        return false;
    }

    // Unix symlinks are the other way to escape the target folder.
    if ((madeBy >> 8) == 3 && ((attrs >> 16) & 0170000) == 0120000)
    {
        SetError(L"Archive contains a symbolic link: %s", e->name);
        return false;
    }
    if (flags & 1)
    {
        SetError(L"Archive entry is encrypted: %s", e->name);
        return false;
    }
    if (e->size > MAX_ENTRY_SIZE)
    {
        SetError(L"Archive entry is too large: %s", e->name);
        return false;
    }
    if (e->method != 0 && e->method != 8)
    {
        SetError(L"Archive uses unsupported compression method %lu: %s", e->method, e->name);
        return false;
    }
    if (e->method == 0 && e->packedSize != e->size)
    {
        SetError(L"Archive is damaged: %s", e->name);
        return false;
    }

    // The local header has its own name/extra lengths, which may differ from the central ones.
    if ((ULONGLONG)localOff + 30 > cur->zipSize || ReadLE32(zip + localOff) != 0x04034B50)
    {
        SetError(L"Archive is damaged: bad local header for %s", e->name);
        return false;
    }
    ULONGLONG dataOff = (ULONGLONG)localOff + 30 + ReadLE16(zip + localOff + 26) + ReadLE16(zip + localOff + 28);
    if (dataOff + e->packedSize > cur->zipSize)
    {
        SetError(L"Archive is damaged: %s is cut off.", e->name);
        return false;
    }
    e->data = zip + dataOff;

    cur->pos = next;
    cur->remaining--;
    return true;
}

// Unpacks one entry to |destPath|, verifying its CRC-32.
static bool ZipExtractEntry(const ZipEntry& e, LPCWSTR destPath)
{
    BYTE* out = NULL;
    if (e.size > 0)
    {
        out = (BYTE*)VirtualAlloc(NULL, e.size, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
        if (!out)
        {
            SetError(L"Not enough memory to unpack %s", e.name);
            return false;
        }
    }

    bool ok;
    if (e.method == 0)
    {
        if (e.size > 0) memcpy(out, e.data, e.size);
        ok = true;
    }
    else
    {
        // An empty file may still have a final empty block, which needs no output buffer.
        static BYTE empty;
        ok = Inflate(e.data, e.packedSize, out ? out : &empty, e.size);
    }

    if (!ok)
        SetError(L"Archive is damaged: cannot unpack %s", e.name);
    else if (Crc32(out, e.size) != e.crc)
    {
        SetError(L"Archive is damaged: CRC mismatch in %s", e.name);
        ok = false;
    }

    if (ok) ok = CreateParentDirs(destPath) && WriteNewFile(destPath, out, e.size);

    if (out) VirtualFree(out, 0, MEM_RELEASE);
    return ok;
}

// Validates the whole archive first, then extracts every file into |gameDir\...\staged|.
static bool ZipExtractAll(const BYTE* zip, size_t zipSize, LPCWSTR gameDir)
{
    ZipEntry e;
    ZipCursor cur;

    if (!ZipOpen(zip, zipSize, &cur)) return false;
    const ZipCursor first = cur;

    ULONGLONG total = 0;
    bool hasLauncher = false, hasGame = false;
    while (cur.remaining > 0)
    {
        if (!ZipNext(&cur, &e)) return false;

        WCHAR probe[MAX_PATH];
        if (!UpdatePath(probe, gameDir, L"staged", e.name)) return false;

        total += e.size;
        if (total > MAX_TOTAL_SIZE)
        {
            SetError(L"Archive is too large when unpacked.");
            return false;
        }
        if (_wcsicmp(e.name, L"OutlastLauncher.exe") == 0)        hasLauncher = true;
        if (_wcsicmp(e.name, L"Binaries\\Win64\\OLGame.exe") == 0) hasGame = true;
    }

    if (!hasLauncher || !hasGame)
    {
        SetError(L"Archive is not an OpenOL release: required files are missing.");
        return false;
    }

    ULONGLONG done = 0;
    cur = first;
    while (cur.remaining > 0)
    {
        if (IsCancelled() || !ZipNext(&cur, &e)) return false;
        if (e.isDir) continue;

        WCHAR dest[MAX_PATH];
        if (!UpdatePath(dest, gameDir, L"staged", e.name) || !ZipExtractEntry(e, dest))
            return false;

        done += e.size;
        ReportProgress(STAGE_EXTRACT, done, total);
    }
    return true;
}

static bool ZipExtractFile(LPCWSTR zipPath, LPCWSTR gameDir)
{
    HANDLE hFile = CreateFileW(zipPath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE)
    {
        SetError(L"Cannot open %s (error %lu)", zipPath, GetLastError());
        return false;
    }

    LARGE_INTEGER size;
    HANDLE hMap = NULL;
    const BYTE* view = NULL;
    if (GetFileSizeEx(hFile, &size) && size.QuadPart > 0 && (ULONGLONG)size.QuadPart <= MAX_ASSET_SIZE)
    {
        hMap = CreateFileMappingW(hFile, NULL, PAGE_READONLY, 0, 0, NULL);
        if (hMap) view = (const BYTE*)MapViewOfFile(hMap, FILE_MAP_READ, 0, 0, 0);
    }

    bool ok = false;
    if (view)
        ok = ZipExtractAll(view, (size_t)size.QuadPart, gameDir);
    else
        SetError(L"Cannot read %s (error %lu)", zipPath, GetLastError());

    if (view) UnmapViewOfFile(view);
    if (hMap) CloseHandle(hMap);
    CloseHandle(hFile);
    return ok;
}

// ---- Install transaction ---------------------------------------------------

// Puts the original back (if there was one), or removes the new file. A new file at
// |dst| may be the running launcher, which cannot be deleted but can be renamed away.
static bool RestoreOriginal(LPCWSTR gameDir, const std::wstring& rel, bool hadOriginal)
{
    static LONG trashCounter = 0;

    WCHAR dst[MAX_PATH], bak[MAX_PATH];
    if (!JoinPath(dst, gameDir, rel.c_str()) || !UpdatePath(bak, gameDir, L"backup", rel.c_str()))
        return false;

    if (PathExists(dst))
    {
        WCHAR name[32], trash[MAX_PATH];
        _snwprintf(name, ARRAYSIZE(name) - 1, L"%ld", InterlockedIncrement(&trashCounter));
        name[ARRAYSIZE(name) - 1] = L'\0';
        if (!UpdatePath(trash, gameDir, L"trash", name) || !CreateParentDirs(trash)
            || !MoveWithRetry(dst, trash, MOVEFILE_REPLACE_EXISTING))
            return false;
    }
    return !hadOriginal || MoveWithRetry(bak, dst, 0);
}

static bool WriteApplyMarker(LPCWSTR gameDir)
{
    WCHAR marker[MAX_PATH];
    if (!UpdatePath(marker, gameDir, L"apply.marker")) return false;
    DeleteFileW(marker);
    return WriteNewFile(marker, (const BYTE*)"1", 1);
}

// Replaces the game files with the staged ones, or leaves them as they were.
static bool ApplyStaged(LPCWSTR gameDir)
{
    struct Applied { std::wstring rel; bool hadOriginal; };

    WCHAR stagedDir[MAX_PATH];
    if (!UpdatePath(stagedDir, gameDir, L"staged")) return false;

    std::vector<std::wstring> files;
    if (!ListFiles(stagedDir, std::wstring(), files) || files.empty())
    {
        SetError(L"No staged files to install.");
        return false;
    }

    if (!WriteApplyMarker(gameDir)) return false;

    std::vector<Applied> applied;
    applied.reserve(files.size());
    bool ok = true;

    for (size_t i = 0; i < files.size() && ok; i++)
    {
        const std::wstring& rel = files[i];
        ReportProgress(STAGE_APPLY, i, files.size());

        WCHAR src[MAX_PATH], dst[MAX_PATH], bak[MAX_PATH];
        ok = JoinPath(src, stagedDir, rel.c_str()) && JoinPath(dst, gameDir, rel.c_str())
            && UpdatePath(bak, gameDir, L"backup", rel.c_str());
        if (!ok) break;

        if (IsDirectory(dst))
        {
            SetError(L"A folder is in the way of a file: %s", dst);
            ok = false;
            break;
        }

        Applied entry;
        entry.rel = rel;
        entry.hadOriginal = PathExists(dst);

        if (entry.hadOriginal)
        {
            if (!CreateParentDirs(bak) || !MoveWithRetry(dst, bak, 0)) { ok = false; break; }
            applied.push_back(entry); // From here on, undoing this file is required.
            ok = CreateParentDirs(dst) && MoveWithRetry(src, dst, 0);
        }
        else
        {
            ok = CreateParentDirs(dst) && MoveWithRetry(src, dst, 0);
            if (ok) applied.push_back(entry);
        }
    }

    if (ok)
    {
        WCHAR marker[MAX_PATH];
        if (UpdatePath(marker, gameDir, L"apply.marker") && DeleteFileW(marker))
            return true;
        SetError(L"Cannot finish the update (error %lu)", GetLastError());
    }

    // Undo in reverse order, keeping the first error for the user.
    WCHAR firstError[ARRAYSIZE(GError)];
    wcscpy(firstError, GError);

    bool restored = true;
    for (size_t i = applied.size(); i-- > 0; )
        restored = RestoreOriginal(gameDir, applied[i].rel, applied[i].hadOriginal) && restored;

    if (restored)
    {
        WCHAR marker[MAX_PATH];
        if (UpdatePath(marker, gameDir, L"apply.marker")) DeleteFileW(marker);
        SetError(L"%s", firstError);
    }
    else
    {
        // The marker stays, so the next start tries again.
        SetError(L"%s\n\nSome game files could not be restored. They will be restored the next "
                 L"time the launcher starts.", firstError);
    }
    return false;
}

// Restores the originals if a previous install was interrupted. Returns false if that failed.
// Files the release added stay in place: nothing refers to them, and the next update overwrites them.
static bool RecoverInterruptedInstall(LPCWSTR gameDir)
{
    WCHAR marker[MAX_PATH], backupDir[MAX_PATH];
    if (!UpdatePath(marker, gameDir, L"apply.marker") || !UpdatePath(backupDir, gameDir, L"backup"))
        return false;
    if (!PathExists(marker)) return true;

    std::vector<std::wstring> files;
    bool ok = ListFiles(backupDir, std::wstring(), files);
    for (size_t i = 0; i < files.size(); i++)
        ok = RestoreOriginal(gameDir, files[i], true) && ok;

    return ok && DeleteFileW(marker);
}

// ---- Install ---------------------------------------------------------------

static bool WriteInstalledTag(LPCWSTR gameDir, LPCWSTR tag)
{
    WCHAR path[MAX_PATH];
    if (!UpdatePath(path, gameDir, L"staged", INSTALLED_TAG_REL)) return false;

    char text[64];
    int len = WideCharToMultiByte(CP_UTF8, 0, tag, -1, text, sizeof(text), NULL, NULL);
    if (len <= 0) return false;
    DeleteFileW(path);
    return CreateParentDirs(path) && WriteNewFile(path, (const BYTE*)text, (DWORD)len - 1);
}

static bool ReadInstalledTag(LPCWSTR gameDir, WCHAR* tag, int tagLen)
{
    WCHAR path[MAX_PATH];
    if (!JoinPath(path, gameDir, INSTALLED_TAG_REL)) return false;

    HANDLE hFile = CreateFileW(path, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE) return false;

    char text[64];
    DWORD read = 0;
    BOOL ok = ReadFile(hFile, text, sizeof(text) - 1, &read, NULL);
    CloseHandle(hFile);
    if (!ok) return false;

    text[read] = '\0';
    return MultiByteToWideChar(CP_UTF8, 0, text, -1, tag, tagLen) > 0;
}

static bool InstallRelease(LPCWSTR gameDir, const UpdateInfo& info)
{
    WCHAR zipPath[MAX_PATH];
    if (!UpdatePath(zipPath, gameDir, L"OpenOL.zip")) return false;

    if (!DownloadVerified(info, zipPath)) return false;
    if (IsCancelled()) return false;

    ReportProgress(STAGE_VERIFY, 0, 0);
    if (!ZipExtractFile(zipPath, gameDir)) return false;

    // Staged like any other file, so it is installed and undone together with them.
    if (!WriteInstalledTag(gameDir, info.tag))
    {
        if (!GError[0]) SetError(L"Cannot record the installed version.");
        return false;
    }

    if (IsCancelled()) return false;
    return ApplyStaged(gameDir);
}

// ---- Progress window -------------------------------------------------------

static LRESULT CALLBACK ProgressWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
    case WM_APP:
        {
            WCHAR text[128];
            switch ((UpdateStage)wParam)
            {
            case STAGE_DOWNLOAD: _snwprintf(text, ARRAYSIZE(text), L"Downloading update... %d%%", (int)lParam); break;
            case STAGE_VERIFY:   wcscpy(text, L"Checking archive..."); break;
            case STAGE_EXTRACT:  _snwprintf(text, ARRAYSIZE(text), L"Unpacking... %d%%", (int)lParam); break;
            default:             wcscpy(text, L"Installing, please do not close..."); break;
            }
            text[ARRAYSIZE(text) - 1] = L'\0';
            if (!GCancel) SetDlgItemTextW(hWnd, 1, text);
        }
        return 0;

    case WM_CLOSE:
        // Installing cannot be cancelled: it is short, and interrupting it is what the rollback is for.
        if (GStage != STAGE_APPLY)
        {
            InterlockedExchange(&GCancel, 1);
            SetDlgItemTextW(hWnd, 1, L"Cancelling...");
        }
        return 0;
    }
    return DefWindowProcW(hWnd, msg, wParam, lParam);
}

static HWND CreateProgressWindow()
{
    WNDCLASSW wc = {};
    wc.lpfnWndProc   = ProgressWndProc;
    wc.hInstance     = GetModuleHandleW(NULL);
    wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
    wc.lpszClassName = L"OpenOLUpdateProgress";
    RegisterClassW(&wc);

    const int width = 420, height = 110;
    HWND hWnd = CreateWindowExW(WS_EX_TOPMOST, wc.lpszClassName, UPDATE_TITLE, WS_CAPTION | WS_SYSMENU,
        (GetSystemMetrics(SM_CXSCREEN) - width) / 2, (GetSystemMetrics(SM_CYSCREEN) - height) / 2,
        width, height, NULL, NULL, wc.hInstance, NULL);
    if (!hWnd) return NULL;

    HWND hText = CreateWindowExW(0, L"STATIC", L"Connecting...", WS_CHILD | WS_VISIBLE | SS_CENTER,
        10, 26, width - 30, 24, hWnd, (HMENU)1, wc.hInstance, NULL);
    SendMessageW(hText, WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT), TRUE);

    ShowWindow(hWnd, SW_SHOW);
    return hWnd;
}

struct InstallJob
{
    LPCWSTR    gameDir;
    UpdateInfo info;
    bool       ok;
};

static DWORD WINAPI InstallThread(LPVOID param)
{
    InstallJob* job = (InstallJob*)param;
    job->ok = InstallRelease(job->gameDir, job->info);
    return 0;
}

// Runs the install on a worker thread while this thread keeps the window responsive.
static bool RunInstallWithProgress(LPCWSTR gameDir, const UpdateInfo& info)
{
    static InstallJob job;
    job.gameDir = gameDir;
    job.info    = info;
    job.ok      = false;

    GProgressWnd = CreateProgressWindow();

    HANDLE hThread = CreateThread(NULL, 0, InstallThread, &job, 0, NULL);
    if (!hThread)
    {
        SetError(L"Cannot start the update (error %lu)", GetLastError());
    }
    else
    {
        while (MsgWaitForMultipleObjects(1, &hThread, FALSE, INFINITE, QS_ALLINPUT) == WAIT_OBJECT_0 + 1)
        {
            MSG msg;
            while (PeekMessageW(&msg, NULL, 0, 0, PM_REMOVE))
            {
                TranslateMessage(&msg);
                DispatchMessageW(&msg);
            }
        }
        CloseHandle(hThread);
    }

    if (GProgressWnd) DestroyWindow(GProgressWnd);
    GProgressWnd = NULL;
    return job.ok;
}

// ---- Update check ----------------------------------------------------------

struct CheckJob
{
    UpdateInfo info;
    bool       found;
};

static DWORD WINAPI CheckThread(LPVOID param)
{
    CheckJob* job = (CheckJob*)param;

    std::vector<char> response;
    if (!HttpGetText(RELEASE_API_URL, L"Accept: application/vnd.github+json\r\n", response) || response.empty())
        return 0;

    job->found = ParseRelease(&response[0], &response[0] + response.size(), &job->info);
    return 0;
}

// Asks GitHub for the latest release, without letting a bad network delay the game
// for long. The job is leaked on timeout: the thread may still use it, and the
// launcher is about to exit anyway.
static bool FetchLatestRelease(UpdateInfo* info)
{
    CheckJob* job = new CheckJob();
    HANDLE hThread = CreateThread(NULL, 0, CheckThread, job, 0, NULL);
    if (!hThread)
    {
        delete job;
        return false;
    }

    bool finished = WaitForSingleObject(hThread, CHECK_WAIT_MS) == WAIT_OBJECT_0;
    CloseHandle(hThread);
    if (!finished)
    {
        fwprintf(stderr, L"OutlastLauncher: update check timed out\n");
        return false;
    }

    bool found = job->found;
    if (found)
        *info = job->info;
    else
        fwprintf(stderr, L"OutlastLauncher: update check failed: %s\n", GError);
    delete job;
    return found;
}

// ---- Entry point -----------------------------------------------------------

// Updates only make sense in an installed game. A launcher started from the build
// output or from the source tree must never overlay a release onto that folder.
static bool IsUpdatableInstall(LPCWSTR gameDir)
{
    WCHAR olgame[MAX_PATH], binaries[MAX_PATH], git[MAX_PATH];
    return JoinPath(olgame, gameDir, L"OLGame") && IsDirectory(olgame)
        && JoinPath(binaries, gameDir, L"Binaries\\Win64") && IsDirectory(binaries)
        && JoinPath(git, gameDir, L".git") && !PathExists(git);
}

bool Updater_Run(LPCWSTR gameDir, LPCWSTR exePath)
{
    if (!IsUpdatableInstall(gameDir)) return false;

    // Only one launcher may touch the update folder. Held until this process exits, so
    // a restarted launcher waits here until the old one is gone.
    HANDLE hMutex = CreateMutexW(NULL, FALSE, L"Local\\OpenOLLauncherUpdate");
    if (!hMutex) return false;
    DWORD wait = WaitForSingleObject(hMutex, 5000);
    if (wait != WAIT_OBJECT_0 && wait != WAIT_ABANDONED) return false;

    WCHAR updateDir[MAX_PATH];
    if (!JoinPath(updateDir, gameDir, UPDATE_DIR)) return false;

    if (!RecoverInterruptedInstall(gameDir))
    {
        WCHAR msg[1536];
        _snwprintf(msg, ARRAYSIZE(msg) - 1,
            L"A previous update was interrupted and the game files could not be restored:\n\n%s\n\n"
            L"Close the game if it is running and start the launcher again, or verify the game files in Steam.",
            GError);
        msg[ARRAYSIZE(msg) - 1] = L'\0';
        MessageBoxW(NULL, msg, UPDATE_TITLE, MB_OK | MB_ICONERROR);
        return false;
    }
    DeleteDirTree(updateDir);

    UpdateInfo info;
    if (!FetchLatestRelease(&info) || !VersionNewer(info.tag, CURRENT_VERSION)) return false;

    // Stops a release whose launcher still has the old CURRENT_VERSION from being offered forever.
    WCHAR installedTag[64];
    if (ReadInstalledTag(gameDir, installedTag, ARRAYSIZE(installedTag)) && _wcsicmp(installedTag, info.tag) == 0)
        return false;

    WCHAR msg[256];
    _snwprintf(msg, ARRAYSIZE(msg) - 1,
        L"A new version of OpenOL is available!\n\n"
        L"Current: %s\nLatest:  %s\n\n"
        L"Download and install now?",
        CURRENT_VERSION, info.tag);
    msg[ARRAYSIZE(msg) - 1] = L'\0';
    if (MessageBoxW(NULL, msg, UPDATE_TITLE, MB_YESNO | MB_ICONQUESTION) != IDYES)
        return false;

    GError[0] = L'\0';
    if (!CreateDirectoryW(updateDir, NULL) && GetLastError() != ERROR_ALREADY_EXISTS)
    {
        SetError(L"No write access to the game folder (error %lu):\n%s\n\n"
                 L"Run the launcher as administrator, or install the update manually.",
                 GetLastError(), gameDir);
    }
    else if (RunInstallWithProgress(gameDir, info))
    {
        DeleteDirTree(updateDir); // The old launcher in backup\ stays until the next start.

        // Start the new launcher with the same arguments, so a Steam join survives the update.
        std::wstring cmd = std::wstring(L"\"") + exePath + L"\" " + PathGetArgsW(GetCommandLineW());
        std::vector<WCHAR> cmdLine(cmd.begin(), cmd.end());
        cmdLine.push_back(L'\0');

        STARTUPINFOW si = {};
        si.cb = sizeof(si);
        PROCESS_INFORMATION pi = {};
        if (CreateProcessW(exePath, &cmdLine[0], NULL, NULL, FALSE, 0, NULL, gameDir, &si, &pi))
        {
            CloseHandle(pi.hProcess);
            CloseHandle(pi.hThread);
            return true;
        }

        _snwprintf(msg, ARRAYSIZE(msg) - 1, L"Update installed, but the new launcher did not start (error %lu).",
            GetLastError());
        msg[ARRAYSIZE(msg) - 1] = L'\0';
        MessageBoxW(NULL, msg, UPDATE_TITLE, MB_OK | MB_ICONWARNING);
        return false;
    }

    bool cancelled = GCancel != 0;
    DeleteDirTree(updateDir);

    if (!cancelled)
    {
        WCHAR text[ARRAYSIZE(GError) + 128];
        _snwprintf(text, ARRAYSIZE(text) - 1, L"Update failed:\n\n%s\n\nThe game will start with the current version.", GError);
        text[ARRAYSIZE(text) - 1] = L'\0';
        MessageBoxW(NULL, text, UPDATE_TITLE, MB_OK | MB_ICONERROR);
    }
    return false;
}
