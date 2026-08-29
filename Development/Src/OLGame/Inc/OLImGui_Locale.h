#pragma once
/*=============================================================================
    OLImGui_Locale.h — OpenOL localisation system.

    Layout on disk (relative to OLGame.exe):
        OpenOL/
            OpenOLSettings.ini       -- global settings (active_locale = en, …)
            OpenOLLocales/
                en.ini               -- English strings
                ru.ini               -- Russian
                ...

    INI format (no sections, dead-simple):
        # comment
        ; comment
        key = value

    Special keys understood inside locale files:
        locale.display_name = English   -- shown in the Language combo
        locale.code         = en        -- overrides filename if present

    Usage:
        const char* s = OLLocale_T("spawns.enemies.title");
        OLLocale_SetActive(idx);
        OLLocale_Reload();
=============================================================================*/

#define OL_LOCALE_MAX_LANGS  16
#define OL_LOCALE_MAX_KEYS   1024
#define OL_LOCALE_KEY_LEN    64
#define OL_LOCALE_VAL_LEN    256

struct FOLLocaleEntry
{
    char Key  [OL_LOCALE_KEY_LEN];
    char Value[OL_LOCALE_VAL_LEN];
};

struct FOLLocaleTable
{
    char           Code       [16];   // "en", "ru", …
    char           DisplayName[64];   // shown in the Language combo
    FOLLocaleEntry Entries    [OL_LOCALE_MAX_KEYS];
    int            Count;
};

// --- lifecycle --------------------------------------------------------------
void OLLocale_Init();    // call once during OLImGui startup (game thread)
void OLLocale_Reload();  // re-read all files from disk

// --- query ------------------------------------------------------------------
int         OLLocale_NumLocales();
const char* OLLocale_Code       (int i);   // "en"
const char* OLLocale_DisplayName(int i);   // "English"
int         OLLocale_ActiveIndex();        // -1 if nothing loaded
const char* OLLocale_ActiveCode ();        // "" if nothing loaded

// --- switch -----------------------------------------------------------------
void OLLocale_SetActive      (int i);
void OLLocale_SetActiveByCode(const char* code);

// --- translate --------------------------------------------------------------
// Returns localised value for key, or key itself if not found.
const char* OLLocale_T(const char* key);

// Returns the English (reference) string for key — used for width comparison.
// Falls back to OLLocale_T() if no English locale is loaded.
const char* OLLocale_TRef(const char* key);

// ---------------------------------------------------------------------------
// OLLocale_FitScale — compute font scale so that text fits within availW.
//
// If the text is narrower than availW, returns 1.0f (no change).
// If it's wider, returns (availW / textWidth), clamped to OL_LOCALE_MIN_SCALE.
//
// availW = 0 means "use ImGui::GetContentRegionAvail().x" (current cell/window).
//
// Usage:
//   float _s = OLLocale_FitScaleStr(str, avail);
//   ImGui::SetWindowFontScale(_s);
//   ImGui::Button(str);
//   ImGui::SetWindowFontScale(1.f);
// ---------------------------------------------------------------------------

#define OL_LOCALE_MIN_SCALE 0.70f   // never shrink below 70 %

#ifndef __cplusplus
#  error OLImGui_Locale.h requires C++
#endif

#include "imgui.h"

// Fit a known string into availW pixels. Pass availW=0 to auto-read from ImGui.
inline float OLLocale_FitScaleStr(const char* text, float availW = 0.f)
{
    if (!text || !text[0]) return 1.f;
    float w = (availW > 0.f) ? availW : ImGui::GetContentRegionAvail().x;
    if (w < 1.f) return 1.f;
    // Subtract frame padding (left + right) — the button's inner text area is smaller.
    float pad = ImGui::GetStyle().FramePadding.x * 2.f;
    float inner = w - pad;
    float textW = ImGui::CalcTextSize(text).x;
    if (textW <= inner) return 1.f;
    float scale = inner / textW;
    if (scale < OL_LOCALE_MIN_SCALE) scale = OL_LOCALE_MIN_SCALE;
    return scale;
}

// Convenience: resolve key then fit into availW (0 = auto from ImGui).
inline float OLLocale_FitScale(const char* key, float availW = 0.f)
{
    return OLLocale_FitScaleStr(OLLocale_T(key), availW);
}

// For label-first widgets (OL_T_L_*): the label sits next to a widget,
// so we compare label width against the English reference label width —
// not against the whole row. Returns scale so label stays ≤ en label width.
inline float OLLocale_FitScaleLabel(const char* key)
{
    const char* local = OLLocale_T(key);
    const char* ref   = OLLocale_TRef(key);
    if (local == ref) return 1.f; // same pointer → en or key not found
    float localW = ImGui::CalcTextSize(local).x;
    float refW   = ImGui::CalcTextSize(ref).x;
    if (localW <= refW || refW < 1.f) return 1.f;
    float scale = refW / localW;
    if (scale < OL_LOCALE_MIN_SCALE) scale = OL_LOCALE_MIN_SCALE;
    return scale;
}
