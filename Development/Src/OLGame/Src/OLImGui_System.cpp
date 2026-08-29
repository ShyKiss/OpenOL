/*=============================================================================
    OLImGui_System.cpp — "System" tab: OpenOL global settings.

    Sections:
        Language — locale combo + reload button
=============================================================================*/

#include "OLImGui_Tabs.h"

// ---------------------------------------------------------------------------
// Language section
// ---------------------------------------------------------------------------

static void DrawLanguageSection()
{
    OL_T_SECTION("system.section.language");

    const int N = OLLocale_NumLocales();
    if (N == 0)
    {
        ImGui::TextDisabled("No locale files found in OpenOL/locales/");
        ImGui::TextDisabled("Create en.ini, ru.ini, etc. with locale.display_name = ...");
        return;
    }

    // Build items array for Combo (pointers into stable GTables storage).
    static const char* Items[OL_LOCALE_MAX_LANGS];
    for (int i = 0; i < N; ++i)
        Items[i] = OLLocale_DisplayName(i);

    int Cur = OLLocale_ActiveIndex();
    if (Cur < 0) Cur = 0;

    ImGui::SetNextItemWidth(-1.f);
    if (ImGui::Combo("##lang", &Cur, Items, N))
        OLLocale_SetActive(Cur);   // persists to settings.ini

    ImGui::Spacing();
    if (ImGui::Button(OLLocale_T("system.reload_locale"), ImVec2(-1.f, 0.f)))
        OLLocale_Reload();
}

// ---------------------------------------------------------------------------
// Keybindings section — overlay toggle key
// ---------------------------------------------------------------------------

static bool GWaitingForToggleKey = false;

static void DrawKeybindingsSection()
{
    OL_SECTION("Keybindings");

    int CurVK = OLImGui_GetOverlayToggleVK();
    int CurIdx = OLImGui_Bindings_FindKeyIdx(CurVK);
    const char* CurName = OLImGui_Bindings_KeyName(CurIdx);

    ImGui::TextUnformatted("Menu toggle key");
    ImGui::SameLine();

    if (GWaitingForToggleKey)
    {
        ImGui::TextDisabled("Press any key...");

        const int N = OLImGui_Bindings_KeyCount();
        for (int k = 1; k < N; ++k)
        {
            if (GetAsyncKeyState(OLImGui_Bindings_KeyVK(k)) & 0x0001)
            {
                OLImGui_Bindings_SetOverlayToggle(OLImGui_Bindings_KeyVK(k));
                GWaitingForToggleKey = false;
                break;
            }
        }
    }
    else
    {
        ImGui::Text("[%s]", CurName);
        ImGui::SameLine();
        if (ImGui::SmallButton("Change##toggle"))
            GWaitingForToggleKey = true;
    }
}

// ---------------------------------------------------------------------------
// Tab entry point
// ---------------------------------------------------------------------------

void OLImGui_TabSystem()
{
    DrawLanguageSection();
    DrawKeybindingsSection();
}
