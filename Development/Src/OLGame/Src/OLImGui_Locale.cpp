/*=============================================================================
    OLImGui_Locale.cpp — OpenOL localisation system implementation.

    Reads from:
        <exe_dir>/OpenOL/settings.ini
        <exe_dir>/OpenOL/locales/*.ini
=============================================================================*/

#include "OLGame.h"
#include "OLImGui_Locale.h"

// ---------------------------------------------------------------------------
// Helpers — pure C, no UE3 allocators
// ---------------------------------------------------------------------------

static void StrTrimRight(char* s)
{
    int n = (int)strlen(s);
    while (n > 0 && (s[n-1] == ' ' || s[n-1] == '\t' || s[n-1] == '\r' || s[n-1] == '\n'))
        s[--n] = '\0';
}

static void StrTrimLeft(char** p)
{
    while (**p == ' ' || **p == '\t') ++(*p);
}

// Case-insensitive strcmp for short strings.
static bool StrEqI(const char* a, const char* b)
{
    for (; *a && *b; ++a, ++b)
        if (tolower((unsigned char)*a) != tolower((unsigned char)*b)) return false;
    return *a == *b;
}

static void SafeCopy(char* dst, const char* src, int dstSize)
{
    int i = 0;
    for (; i < dstSize - 1 && src[i]; ++i) dst[i] = src[i];
    dst[i] = '\0';
}

// ---------------------------------------------------------------------------
// State
// ---------------------------------------------------------------------------

static FOLLocaleTable  GTables[OL_LOCALE_MAX_LANGS];
static int             GNumTables   = 0;
static int             GActiveIdx   = -1;

// Buffer for settings INI path and locales dir.
static char GSettingsPath [512] = {};
static char GLocalesDir   [512] = {};

// ---------------------------------------------------------------------------
// Build file paths from appBaseDir()
// ---------------------------------------------------------------------------

static void BuildPaths()
{
    // appBaseDir() returns the directory containing OLGame.exe, e.g.
    // "S:\common\Outlast\Binaries\Win64\" (Windows path, TCHAR).
    FString Base = appBaseDir();
    // Narrow conversion — ASCII path is safe for Win32 file ops.
    FString SettingsFStr = Base + TEXT("OpenOL\\settings.ini");
    FString LocalesDFStr = Base + TEXT("OpenOL\\locales\\");

    const TCHAR* S = *SettingsFStr;
    const TCHAR* D = *LocalesDFStr;
    int i = 0;
    for (; i < 511 && S[i]; ++i) GSettingsPath[i] = (char)S[i];
    GSettingsPath[i] = '\0';
    i = 0;
    for (; i < 511 && D[i]; ++i) GLocalesDir[i] = (char)D[i];
    GLocalesDir[i] = '\0';
}

// ---------------------------------------------------------------------------
// Parse one INI file into a locale table.
// Returns number of key=value pairs parsed (excluding special locale.* keys).
// ---------------------------------------------------------------------------

static void ParseLocaleFile(const char* path, FOLLocaleTable& Out)
{
    Out.Count = 0;

    FILE* f = fopen(path, "r");
    if (!f) return;

    char Line[512];
    while (fgets(Line, sizeof(Line), f))
    {
        char* p = Line;
        StrTrimLeft(&p);
        if (!*p || *p == '#' || *p == ';') continue; // comment / blank

        // Split on first '='
        char* eq = strchr(p, '=');
        if (!eq) continue;

        *eq = '\0';
        char* key = p;
        char* val = eq + 1;

        StrTrimRight(key);
        StrTrimLeft(&val);
        StrTrimRight(val);

        if (!key[0] || !val[0]) continue;

        // Special keys
        if (StrEqI(key, "locale.display_name"))
        {
            SafeCopy(Out.DisplayName, val, sizeof(Out.DisplayName));
            continue;
        }
        if (StrEqI(key, "locale.code"))
        {
            SafeCopy(Out.Code, val, sizeof(Out.Code));
            continue;
        }

        // Regular translation entry
        if (Out.Count >= OL_LOCALE_MAX_KEYS) continue;
        FOLLocaleEntry& E = Out.Entries[Out.Count++];
        SafeCopy(E.Key,   key, OL_LOCALE_KEY_LEN);
        SafeCopy(E.Value, val, OL_LOCALE_VAL_LEN);
    }

    fclose(f);
}

// ---------------------------------------------------------------------------
// Read OpenOLSettings.ini — returns value for a given key, or "" if absent.
// ---------------------------------------------------------------------------

static void ReadSettingsKey(const char* wantKey, char* outVal, int outSize)
{
    outVal[0] = '\0';
    FILE* f = fopen(GSettingsPath, "r");
    if (!f) return;

    char Line[256];
    while (fgets(Line, sizeof(Line), f))
    {
        char* p = Line;
        StrTrimLeft(&p);
        if (!*p || *p == '#' || *p == ';') continue;

        char* eq = strchr(p, '=');
        if (!eq) continue;
        *eq = '\0';
        char* key = p;
        char* val = eq + 1;
        StrTrimRight(key);
        StrTrimLeft(&val);
        StrTrimRight(val);

        if (StrEqI(key, wantKey))
        {
            SafeCopy(outVal, val, outSize);
            break;
        }
    }
    fclose(f);
}

// ---------------------------------------------------------------------------
// Write (or overwrite) a single key in OpenOLSettings.ini.
// Reads the whole file, updates the key, rewrites.
// ---------------------------------------------------------------------------

static void WriteSettingsKey(const char* wantKey, const char* newVal)
{
    // Read existing lines.
    char Lines[64][256];
    int  NLines = 0;
    bool Found  = false;

    FILE* f = fopen(GSettingsPath, "r");
    if (f)
    {
        char Buf[256];
        while (NLines < 63 && fgets(Buf, sizeof(Buf), f))
        {
            SafeCopy(Lines[NLines++], Buf, 256);

            // Check if this line contains the key we want to update.
            char Tmp[256];
            SafeCopy(Tmp, Buf, 256);
            char* p = Tmp; StrTrimLeft(&p);
            if (*p == '#' || *p == ';') continue;
            char* eq = strchr(p, '=');
            if (!eq) continue;
            *eq = '\0'; char* k = p; StrTrimRight(k);
            if (StrEqI(k, wantKey))
            {
                // Replace this line.
                _snprintf(Lines[NLines-1], 256, "%s = %s\n", wantKey, newVal);
                Found = true;
            }
        }
        fclose(f);
    }

    if (!Found)
    {
        // Append the key.
        _snprintf(Lines[NLines++], 256, "%s = %s\n", wantKey, newVal);
    }

    FILE* w = fopen(GSettingsPath, "w");
    if (!w) return;
    for (int i = 0; i < NLines; ++i) fputs(Lines[i], w);
    fclose(w);
}

// ---------------------------------------------------------------------------
// Scan OpenOLLocales/ for *.ini files and load them.
// ---------------------------------------------------------------------------

static void LoadAllLocales()
{
    GNumTables = 0;

    // Use Win32 FindFirstFile / FindNextFile via UE3's GFileManager.
    FString Pattern = FString(ANSI_TO_TCHAR(GLocalesDir)) + TEXT("*.ini");
    TArray<FString> Files;
    GFileManager->FindFiles(Files, *Pattern, TRUE, FALSE);

    for (int fi = 0; fi < Files.Num() && GNumTables < OL_LOCALE_MAX_LANGS; ++fi)
    {
        // Build full path.
        FString FullFStr = FString(ANSI_TO_TCHAR(GLocalesDir)) + Files(fi);
        char FullPath[512] = {};
        const TCHAR* FP = *FullFStr;
        int i = 0;
        for (; i < 511 && FP[i]; ++i) FullPath[i] = (char)FP[i];
        FullPath[i] = '\0';

        FOLLocaleTable& T = GTables[GNumTables];
        memset(&T, 0, sizeof(T));

        // Derive code from filename (strip .ini extension).
        // Files(fi) is a bare filename like "en.ini" — strip extension manually.
        const TCHAR* FN = *Files(fi);
        int ci = 0;
        while (ci < 15 && FN[ci] && FN[ci] != TEXT('.'))
            T.Code[ci++] = (char)FN[ci];  // ASCII locale codes are safe
        T.Code[ci] = '\0';

        // Default display name = code, will be overridden by locale.display_name key.
        SafeCopy(T.DisplayName, T.Code, sizeof(T.DisplayName));

        ParseLocaleFile(FullPath, T);

        // If file didn't provide a display name, use code.
        if (!T.DisplayName[0]) SafeCopy(T.DisplayName, T.Code, sizeof(T.DisplayName));

        ++GNumTables;
    }
}

// ---------------------------------------------------------------------------
// Public API
// ---------------------------------------------------------------------------

void OLLocale_Init()
{
    BuildPaths();
    OLLocale_Reload();
}

void OLLocale_Reload()
{
    LoadAllLocales();

    // Restore previously saved active locale.
    char SavedCode[16] = {};
    ReadSettingsKey("active_locale", SavedCode, sizeof(SavedCode));

    GActiveIdx = -1;
    if (SavedCode[0])
    {
        for (int i = 0; i < GNumTables; ++i)
        {
            if (StrEqI(GTables[i].Code, SavedCode))
            {
                GActiveIdx = i;
                break;
            }
        }
    }
    // If no saved locale or not found, default to first available.
    if (GActiveIdx < 0 && GNumTables > 0)
        GActiveIdx = 0;
}

int OLLocale_NumLocales()
{
    return GNumTables;
}

const char* OLLocale_Code(int i)
{
    if (i < 0 || i >= GNumTables) return "";
    return GTables[i].Code;
}

const char* OLLocale_DisplayName(int i)
{
    if (i < 0 || i >= GNumTables) return "";
    return GTables[i].DisplayName;
}

int OLLocale_ActiveIndex()
{
    return GActiveIdx;
}

const char* OLLocale_ActiveCode()
{
    if (GActiveIdx < 0 || GActiveIdx >= GNumTables) return "";
    return GTables[GActiveIdx].Code;
}

void OLLocale_SetActive(int i)
{
    if (i < 0 || i >= GNumTables) return;
    GActiveIdx = i;
    WriteSettingsKey("active_locale", GTables[i].Code);
}

void OLLocale_SetActiveByCode(const char* code)
{
    for (int i = 0; i < GNumTables; ++i)
    {
        if (StrEqI(GTables[i].Code, code))
        {
            OLLocale_SetActive(i);
            return;
        }
    }
}

const char* OLLocale_T(const char* key)
{
    if (GActiveIdx < 0 || GActiveIdx >= GNumTables) return key;
    const FOLLocaleTable& T = GTables[GActiveIdx];
    for (int i = 0; i < T.Count; ++i)
        if (strcmp(T.Entries[i].Key, key) == 0)
            return T.Entries[i].Value;
    return key; // fallback: show the key itself
}

// Find a string in a specific locale table by code.
static const char* FindInTable(const char* code, const char* key)
{
    for (int t = 0; t < GNumTables; ++t)
    {
        if (!StrEqI(GTables[t].Code, code)) continue;
        const FOLLocaleTable& T = GTables[t];
        for (int i = 0; i < T.Count; ++i)
            if (strcmp(T.Entries[i].Key, key) == 0)
                return T.Entries[i].Value;
        return NULL; // table found but key missing
    }
    return NULL; // table not found
}

// Returns the English (reference) string for a key, or NULL if not available.
// Used to measure the "reference width" for font scaling.
const char* OLLocale_TRef(const char* key)
{
    const char* s = FindInTable("en", key);
    if (s) return s;
    // If no en locale, return active translation so scale = 1.0
    return OLLocale_T(key);
}
