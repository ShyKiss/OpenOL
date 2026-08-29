/*=============================================================================
    OLImGui_Bindings.cpp -- per-button key bindings for OpenOL ImGui actions.
=============================================================================*/

#include "OLImGui_Tabs.h"
#include "OLImGui_Bindings.h"
#include <stdio.h>
#include <string.h>

// ---------------------------------------------------------------------------
// Key table — built dynamically from Win32 GetKeyNameText.
// ---------------------------------------------------------------------------

// Declared in imgui_impl_win32.cpp but not exposed in the public header.
ImGuiKey ImGui_ImplWin32_KeyEventToImGuiKey(WPARAM wParam, LPARAM lParam);

struct FBindKey
{
    char     Name[64];
    int      VK;
    ImGuiKey ImKey;   // cached ImGuiKey for IsKeyDown()
};

static FBindKey GBindKeys[256];   // max 255 VK codes + "None"
static int      GBindKeyCount = 0;

static bool ShouldExcludeVK(int VK)
{
    // Mouse buttons
    if (VK >= VK_LBUTTON && VK <= VK_XBUTTON2) return true;
    // Modifiers (we keep them as separate concepts, not bindable)
    if (VK == VK_SHIFT || VK == VK_CONTROL || VK == VK_MENU) return true;
    if (VK == VK_LSHIFT || VK == VK_RSHIFT) return true;
    if (VK == VK_LCONTROL || VK == VK_RCONTROL) return true;
    if (VK == VK_LMENU || VK == VK_RMENU) return true;
    if (VK == VK_LWIN || VK == VK_RWIN) return true;
    // IME / packet / reserved
    if (VK == VK_PACKET) return true;
    if (VK >= 0xE0) return true;
    return false;
}

static void BuildKeyTable()
{
    if (GBindKeyCount > 0) return;

    // First entry: "None" = unbound
    GBindKeys[0].Name[0] = '\0';
    _snprintf(GBindKeys[0].Name, 64, "None");
    GBindKeys[0].VK    = 0;
    GBindKeys[0].ImKey = ImGuiKey_None;
    GBindKeyCount      = 1;

    for (int VK = 1; VK < 256; ++VK)
    {
        if (ShouldExcludeVK(VK)) continue;

        UINT SC = MapVirtualKey((UINT)VK, MAPVK_VK_TO_VSC);
        if (SC == 0) continue;

        // Extended key flag for keys like Insert, Delete, Home, End, arrows, numpad /
        LONG lParam = (LONG)(SC << 16);
        // Keys that need the extended flag to get the right name
        if (VK == VK_INSERT || VK == VK_DELETE || VK == VK_HOME || VK == VK_END ||
            VK == VK_PRIOR  || VK == VK_NEXT   ||
            VK == VK_LEFT   || VK == VK_RIGHT   || VK == VK_UP || VK == VK_DOWN ||
            VK == VK_NUMLOCK || VK == VK_DIVIDE || VK == VK_RCONTROL || VK == VK_RMENU)
        {
            lParam |= (1 << 24); // extended key bit
        }

        char Name[64] = {};
        if (GetKeyNameTextA(lParam, Name, sizeof(Name)) <= 0) continue;
        if (Name[0] == '\0') continue;

        FBindKey& K = GBindKeys[GBindKeyCount++];
        _snprintf(K.Name, 64, "%s", Name);
        K.VK    = VK;
        K.ImKey = ImGui_ImplWin32_KeyEventToImGuiKey((WPARAM)VK, lParam);

        if (GBindKeyCount >= 256) break;
    }
}

// ---------------------------------------------------------------------------
// Binding store — max 64 actions.
// ---------------------------------------------------------------------------
static const int MAX_BINDINGS = 64;

struct FBinding
{
    char ActionId[64];
    char Label[64];          // display label for the popup header
    int  VK;                 // 0 = unbound
    OLImGuiCallFn Fn;
};

static FBinding GBindings[MAX_BINDINGS];
static int      GBindingCount   = 0;
static bool     GBindingsLoaded = false;
static int      GOverlayToggleVK = VK_F6; // configurable, saved as overlay.toggle

// ---------------------------------------------------------------------------
// INI path helper — same base as settings.ini.
// ---------------------------------------------------------------------------
static void GetBindingsPath(char* Out, int Size)
{
    char ExeDir[512] = {};
    GetModuleFileNameA(NULL, ExeDir, sizeof(ExeDir));
    char* Slash = strrchr(ExeDir, '\\');
    if (Slash) Slash[1] = '\0';
    _snprintf(Out, Size, "%sOpenOL\\bindings.ini", ExeDir);
}

// ---------------------------------------------------------------------------
// Load / Save
// ---------------------------------------------------------------------------
static int FindVKByName(const char* Name)
{
    for (int k = 0; k < GBindKeyCount; ++k)
        if (_stricmp(GBindKeys[k].Name, Name) == 0)
            return GBindKeys[k].VK;
    return 0;
}

static const char* FindNameByVK(int VK)
{
    for (int k = 0; k < GBindKeyCount; ++k)
        if (GBindKeys[k].VK == VK)
            return GBindKeys[k].Name;
    return NULL;
}

void OLImGui_Bindings_Load()
{
    BuildKeyTable();
    char Path[512];
    GetBindingsPath(Path, sizeof(Path));

    FILE* F = fopen(Path, "r");
    if (!F) return;

    char Line[256];
    while (fgets(Line, sizeof(Line), F))
    {
        char* NL = strchr(Line, '\n');
        if (NL) *NL = '\0';

        char* Eq = strchr(Line, '=');
        if (!Eq) continue;
        *Eq = '\0';
        const char* Key = Line;
        const char* Val = Eq + 1;

        // Overlay toggle key
        if (_stricmp(Key, "overlay.toggle") == 0)
        {
            int VK = FindVKByName(Val);
            if (VK != 0) GOverlayToggleVK = VK;
            continue;
        }

        // Regular action bindings
        for (int i = 0; i < GBindingCount; ++i)
        {
            if (_stricmp(GBindings[i].ActionId, Key) == 0)
            {
                GBindings[i].VK = FindVKByName(Val);
                break;
            }
        }
    }
    fclose(F);
}

int OLImGui_GetOverlayToggleVK()
{
    BuildKeyTable();
    return GOverlayToggleVK;
}

void OLImGui_Bindings_Save()
{
    char Path[512];
    GetBindingsPath(Path, sizeof(Path));

    FILE* F = fopen(Path, "w");
    if (!F) return;

    // Overlay toggle
    const char* ToggleName = FindNameByVK(GOverlayToggleVK);
    if (ToggleName)
        fprintf(F, "overlay.toggle=%s\n", ToggleName);

    // Action bindings
    for (int i = 0; i < GBindingCount; ++i)
    {
        if (GBindings[i].VK == 0) continue;
        const char* KeyName = FindNameByVK(GBindings[i].VK);
        if (KeyName)
            fprintf(F, "%s=%s\n", GBindings[i].ActionId, KeyName);
    }
    fclose(F);
}

// ---------------------------------------------------------------------------
// Fire
// ---------------------------------------------------------------------------
void OLImGui_Bindings_FireKey(int VK)
{
    if (VK == 0) return;
    for (int i = 0; i < GBindingCount; ++i)
    {
        if (GBindings[i].VK == VK && GBindings[i].Fn)
            OLImGui_EnqueueCall(GBindings[i].Fn);
    }
}

// ---------------------------------------------------------------------------
// Popup state
// ---------------------------------------------------------------------------
static int  GPopupBindingIdx = -1;   // index into GBindings being edited
static int  GPopupKeyIdx     = 0;    // selected index in GBindKeys combo
static bool GPopupOpen       = false;

void OLImGui_Bindings_DrawPopup()
{
    if (!GPopupOpen) return;

    ImGui::OpenPopup("##bind_popup");
    GPopupOpen = false;

    // Center on screen
    ImVec2 Center = ImVec2(ImGui::GetIO().DisplaySize.x * 0.5f,
                           ImGui::GetIO().DisplaySize.y * 0.5f);
    ImGui::SetNextWindowPos(Center, ImGuiCond_Always, ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowSize(ImVec2(260.f, 0.f), ImGuiCond_Always);
}

// Must be called every frame after DrawPopup triggers OpenPopup.
static void ConfirmBinding(FBinding& B)
{
    B.VK = GBindKeys[GPopupKeyIdx].VK;
    OLImGui_Bindings_Save();
    ImGui::CloseCurrentPopup();
    GPopupBindingIdx = -1;
}

static void DrawPopupModal()
{
    if (GPopupBindingIdx < 0 || GPopupBindingIdx >= GBindingCount)
    {
        ImGui::CloseCurrentPopup();
        return;
    }

    FBinding& B = GBindings[GPopupBindingIdx];

    ImGui::Text("Bind: %s", B.Label);
    ImGui::Separator();
    ImGui::TextDisabled("Press any key...");
    ImGui::Spacing();

    // Current selection display
    const char* CurName = (GPopupKeyIdx > 0 && GPopupKeyIdx < GBindKeyCount)
                          ? GBindKeys[GPopupKeyIdx].Name : "None";
    ImGui::Text("Selected: %s", CurName);
    ImGui::Separator();

    // Detect any key press via GetAsyncKeyState (bypasses ImGui focus entirely).
    // Low bit (0x0001) = key went down since last GetAsyncKeyState call for this VK.
    for (int k = 1; k < GBindKeyCount; ++k)
    {
        if (GetAsyncKeyState(GBindKeys[k].VK) & 0x0001)
        {
            GPopupKeyIdx = k;
            ConfirmBinding(B);
            return;
        }
    }

    ImGui::Spacing();

    float BtnW = (ImGui::GetContentRegionAvail().x - ImGui::GetStyle().ItemSpacing.x) * 0.5f;
    if (ImGui::Button("Confirm", ImVec2(BtnW, 0.f)))
        ConfirmBinding(B);
    ImGui::SameLine();
    if (ImGui::Button("Clear", ImVec2(BtnW, 0.f)))
    {
        B.VK = 0;
        OLImGui_Bindings_Save();
        ImGui::CloseCurrentPopup();
        GPopupBindingIdx = -1;
    }
}

// Called every frame from OLImGui_BuildUI.
// Note: OLImGui_Bindings_DrawPopup() triggers OpenPopup; this actually renders it.
void OLImGui_Bindings_RenderPopup()
{
    ImVec2 Center = ImVec2(ImGui::GetIO().DisplaySize.x * 0.5f,
                           ImGui::GetIO().DisplaySize.y * 0.5f);
    ImGui::SetNextWindowPos(Center, ImGuiCond_Always, ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowSize(ImVec2(260.f, 0.f), ImGuiCond_Always);

    if (ImGui::BeginPopup("##bind_popup"))
    {
        DrawPopupModal();
        ImGui::EndPopup();
    }
}

// ---------------------------------------------------------------------------
// RegisterLast — attach binding to the last drawn ImGui item.
// Call immediately after ImGui::Button (or OL_ACTION etc.)
// ---------------------------------------------------------------------------
static int FindOrRegister(const char* ActionId, OLImGuiCallFn Fn)
{
    BuildKeyTable();

    for (int i = 0; i < GBindingCount; ++i)
        if (_stricmp(GBindings[i].ActionId, ActionId) == 0)
            return i;

    if (GBindingCount >= MAX_BINDINGS)
        return -1;

    if (!GBindingsLoaded)
    {
        GBindingsLoaded = true;
        OLImGui_Bindings_Load();
    }

    int Idx = GBindingCount++;
    _snprintf(GBindings[Idx].ActionId, 64, "%s", ActionId);
    _snprintf(GBindings[Idx].Label,    64, "%s", ActionId); // label filled on first hover
    GBindings[Idx].VK = 0;
    GBindings[Idx].Fn = Fn;

    // Load saved VK for this action
    char Path[512]; GetBindingsPath(Path, sizeof(Path));
    FILE* F = fopen(Path, "r");
    if (F) {
        char Line[256];
        while (fgets(Line, sizeof(Line), F)) {
            char* NL = strchr(Line, '\n'); if (NL) *NL = '\0';
            char* Eq = strchr(Line, '=');  if (!Eq) continue;
            *Eq = '\0';
            if (_stricmp(Line, ActionId) == 0) {
                for (int k = 0; k < GBindKeyCount; ++k)
                    if (_stricmp(GBindKeys[k].Name, Eq+1) == 0)
                        { GBindings[Idx].VK = GBindKeys[k].VK; break; }
                break;
            }
        }
        fclose(F);
    }
    return Idx;
}

void OLImGui_Bindings_RegisterLast(const char* ActionId, OLImGuiCallFn Fn)
{
    int Idx = FindOrRegister(ActionId, Fn);
    if (Idx < 0) return;

    // Update Fn in case it changed (e.g. DBG_CMD_B thunk address)
    if (Fn) GBindings[Idx].Fn = Fn;

    // Show key hint as tooltip when hovered
    if (ImGui::IsItemHovered())
    {
        if (GBindings[Idx].VK != 0)
        {
            const char* KeyName = "?";
            for (int k = 0; k < GBindKeyCount; ++k)
                if (GBindKeys[k].VK == GBindings[Idx].VK)
                    { KeyName = GBindKeys[k].Name; break; }
            char Tip[64];
            _snprintf(Tip, sizeof(Tip), "[%s]  RMB to rebind", KeyName);
            ImGui::SetTooltip("%s", Tip);
        }
        else
        {
            ImGui::SetTooltip("RMB to bind a key");
        }

        // Open popup on RMB
        if (ImGui::IsMouseClicked(ImGuiMouseButton_Right))
        {
            GPopupBindingIdx = Idx;
            GPopupOpen       = true;
            GPopupKeyIdx     = 0;
            for (int k = 0; k < GBindKeyCount; ++k)
                if (GBindKeys[k].VK == GBindings[Idx].VK)
                    { GPopupKeyIdx = k; break; }
        }
    }
}

// ---------------------------------------------------------------------------
// Key table accessors
// ---------------------------------------------------------------------------
int OLImGui_Bindings_KeyCount()             { BuildKeyTable(); return GBindKeyCount; }
const char* OLImGui_Bindings_KeyName(int i) { BuildKeyTable(); return (i >= 0 && i < GBindKeyCount) ? GBindKeys[i].Name : ""; }
int OLImGui_Bindings_KeyVK(int i)           { BuildKeyTable(); return (i >= 0 && i < GBindKeyCount) ? GBindKeys[i].VK   : 0;  }

int OLImGui_Bindings_FindKeyIdx(int VK)
{
    BuildKeyTable();
    for (int k = 0; k < GBindKeyCount; ++k)
        if (GBindKeys[k].VK == VK) return k;
    return 0;
}

void OLImGui_Bindings_SetOverlayToggle(int VK)
{
    GOverlayToggleVK = VK;
    OLImGui_Bindings_Save();
}
