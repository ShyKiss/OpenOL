#pragma once
/*=============================================================================
    OLImGui_Bindings.h -- per-button key bindings for OpenOL ImGui actions.

    Usage (in any tab .cpp that includes OLImGui_Tabs.h):

        static void DoKill() { ... }
        OL_BINDABLE("player.kill", "Kill", DoKill);

    The button fires DoKill when clicked. Right-clicking it opens a small popup
    where the user picks a key; the binding is saved to OpenOL/bindings.ini and
    fires DoKill whenever that key is pressed (overlay closed).

    Call OLImGui_Bindings_FireKey(VK) from WinViewport WM_KEYDOWN when the
    overlay is NOT open.
=============================================================================*/

// Load bindings from OpenOL/bindings.ini (call once at startup).
void OLImGui_Bindings_Load();

// Called after ImGui::Button to attach a binding to the last drawn item.
// Registers the action (first call), shows RMB popup, shows key hint in tooltip.
// Fn_ is used for key-fire dispatch; may be NULL for DBG_CMD_B (no key-fire).
void OLImGui_Bindings_RegisterLast(const char* ActionId, OLImGuiCallFn Fn);

// Save all bindings to OpenOL/bindings.ini.
void OLImGui_Bindings_Save();

// Called from WinViewport WM_KEYDOWN when the overlay is closed.
// Fires the action bound to this VK code (if any).
void OLImGui_Bindings_FireKey(int VK);

// Key table accessors — for building combos in UI.
int         OLImGui_Bindings_KeyCount();
const char* OLImGui_Bindings_KeyName(int Idx);
int         OLImGui_Bindings_KeyVK(int Idx);
int         OLImGui_Bindings_FindKeyIdx(int VK); // returns 0 (None) if not found

// Overlay toggle key.
void OLImGui_Bindings_SetOverlayToggle(int VK);

// Draw the bind popup if one is pending. Call once per frame inside OLImGui_BuildUI.
void OLImGui_Bindings_DrawPopup();

