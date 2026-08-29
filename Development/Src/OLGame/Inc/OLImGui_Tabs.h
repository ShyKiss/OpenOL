#pragma once
/*=============================================================================
    OLImGui_Tabs.h — Common includes and macro toolkit for OLImGui tab files.

    Every tab .cpp starts with:
        #include "OLImGui_Tabs.h"

    MACRO FAMILIES
    ──────────────
    OL_*           Raw string label (English literal, no localisation).
    OL_T_*         Localised label — LocKey_ resolved via OLLocale_T().
                   If the key is missing, the key path itself is shown.
    OL_L_*         Label-first layout: "Label  [widget]" on one row.
                   Widget ID is derived from the label via "##" suffix.
    OL_T_L_*       Localised + label-first combined.
    DBG_CMD        Fire a UE3 console command from a button.
    OL_TABLE_*     Multi-column button grid.
    OL_SEARCH      Full-width localised search box.
    OL_COMBO       Label + full-width dropdown, one-liner.
    OL_PREVIEW_*   Asset preview scene control (skeletal / static mesh).
    OL_ROW_SLIDER_WIDTH_N  Compute equal slider widths for N sliders on one row.
=============================================================================*/

#include "OLImGui_Locale.h"

#include "imgui_compat_msvc2012.h"
#include "imgui.h"

#include "OLGame.h"
#include "ImGuiShared.h"
#include "OLImGui_Bindings.h"
#include "OLImGui.h"
#include "OLUtilities.h"
#include "OLImGui_Utils.h"

// ---------------------------------------------------------------------------
// OLImGui_GetD3DTexture
//   Convert a UTexture2D* to an IDirect3DTexture9* for use with ImGui::Image.
//   Implemented in OLImGui_AssetPreview.cpp (already pulls D3D9Resources.h).
// ---------------------------------------------------------------------------
struct IDirect3DTexture9;
IDirect3DTexture9* OLImGui_GetD3DTexture(UTexture2D* Tex);

// ---------------------------------------------------------------------------
// OLImGui_EnqueueCall / OLImGui_EnqueuePostTickCall
//   Schedule a zero-argument function to run on the game thread.
//   ImGui runs on the render thread; any UObject or game-state mutation
//   MUST go through one of these.
//
//   EnqueueCall        — runs at the start of the next game tick.
//   EnqueuePostTickCall — runs after the tick completes (use for deferred
//                         actions that depend on the tick having finished,
//                         e.g. level transitions).
//
//   Example:
//       static void DoTeleport() { GetPC()->SetLocation(...); }
//       if (ImGui::Button("Teleport")) OLImGui_EnqueueCall(DoTeleport);
// ---------------------------------------------------------------------------
void OLImGui_EnqueueCall(OLImGuiCallFn Fn);
void OLImGui_EnqueuePostTickCall(OLImGuiCallFn Fn);

// Pending command slot — written on render thread, dispatched on game thread.
extern const char* GOLPendingCmd;
void OLImGui_ExecPendingCmd();

// ---------------------------------------------------------------------------
// DBG_CMD(Label_, Cmd_)
//   Button that fires a UE3 console command on the game thread.
//   Inside OL_TABLE_CELL the button stretches to fill the column.
//   Outside a table it uses its natural content width.
//
//   Example:
//       DBG_CMD("Show BSP", "Show BSP");
//       DBG_CMD("Stat FPS", "Stat FPS");
// ---------------------------------------------------------------------------
// Internal: apply binding RMB popup to the last drawn item.
// Id_ — unique action id string. Fn_ — OLImGuiCallFn for key-fire dispatch.
#define _OL_BIND_LAST_(Id_, Fn_) \
    OLImGui_Bindings_RegisterLast(Id_, Fn_)

#define DBG_CMD(Label_, Cmd_) \
    do { \
        if (ImGui::Button(Label_, _ol_in_table_cell ? ImVec2(-1.f,0.f) : ImVec2(0.f,0.f))) { \
            GOLPendingCmd = Cmd_; \
            OLImGui_EnqueueCall(OLImGui_ExecPendingCmd); \
        } \
    } while(0)

// DBG_CMD_B(Id_, Label_, Cmd_) — bindable variant of DBG_CMD.
// Key-fire executes the same console command via a registered static thunk.
#define DBG_CMD_B(Id_, Label_, Cmd_) \
    do { \
        struct _CmdThunk_ { static void Fire() { GOLPendingCmd = Cmd_; OLImGui_EnqueueCall(OLImGui_ExecPendingCmd); } }; \
        if (ImGui::Button(Label_, _ol_in_table_cell ? ImVec2(-1.f,0.f) : ImVec2(0.f,0.f))) _CmdThunk_::Fire(); \
        _OL_BIND_LAST_(Id_, _CmdThunk_::Fire); \
    } while(0)

// Internal render-thread counterpart of OLImGui_Bindings_DrawPopup.
// Called every frame from OLImGui_BuildUI after DrawPopup triggers OpenPopup.
void OLImGui_Bindings_RenderPopup();

// ---------------------------------------------------------------------------
// Tab entry points — implemented in their respective .cpp files.
// ---------------------------------------------------------------------------
void MpHud_DrawOverlay();

void OLImGui_TabActions();
void OLImGui_TabVisuals();
void OLImGui_TabPlayer();
void OLImGui_TabSpawns();
void OLSpawns_TickPreviews();         // tick main + hover preview instances (game thread)
void OLSpawns_TickPlacement();        // consume placement confirm — must be called every tick, menu open or not
void OLSpawns_DrawPlacementOverlay(); // draw ghost ellipse overlay — call every frame during placement mode
void OLSpawns_InvalidateMeshCache();  // internal — prefer OLImGui_InvalidateCaches()
void OLSpawns_ShutdownPreviews();     // shut down ALL preview instances; called on map change
void OLSpawns_ReleaseD3DObjects();    // release D3DPOOL_DEFAULT surfaces before device Reset
void OLSpawns_BackToHub();            // return from any sub-tab to the Spawns hub grid
void OLImGui_TabSpawns_Enemies();     // enemies sub-tab (list / new / detail views)

// OLImGui_InvalidateCaches
//   Call before PrepareMapChange or CollectGarbage.
//   Clears all raw UObject* caches not protected by AddToRoot/UPROPERTY
//   so the next tick reloads them instead of touching freed memory.
void OLImGui_InvalidateCaches();
void OLImGui_TabScene();
void OLImGui_TabRelay();
void OLImGui_TabSystem();
bool OLImGui_CheckpointsAvailable();
void OLImGui_TabCheckpoints();
void OLImGui_Checkpoints_Tick();      // call from game-thread ticker

// ---------------------------------------------------------------------------
// Asset preview — OLImGui_AssetPreview.cpp / OLImGui_AssetPreview.h
// ---------------------------------------------------------------------------
#include "OLImGui_AssetPreview.h"

extern void* volatile GOLPreviewPendingAttachMesh;
extern int   volatile GOLPreviewPendingAttachSlot;
extern char           GOLPreviewPendingAttachSocket[256];
void OLPreview_ApplyAttach();

// OL_PREVIEW_SKELETAL(MeshPtr_)
//   Load a USkeletalMesh into the main preview actor (slot 0).
//   MeshPtr_ — USkeletalMesh* (may be NULL to clear).
//   Enqueues OLPreview_ApplySkeletal on the game thread.
//
//   Example:
//       OL_PREVIEW_SKELETAL(MyMesh);
#define OL_PREVIEW_SKELETAL(MeshPtr_) \
    do { GOLPreviewPendingSkeletal = (MeshPtr_); OLImGui_EnqueueCall(OLPreview_ApplySkeletal); } while(0)

// OL_PREVIEW_STATIC(MeshPtr_)
//   Load a UStaticMesh into the preview actor.
//   MeshPtr_ — UStaticMesh* (may be NULL to clear).
//
//   Example:
//       OL_PREVIEW_STATIC(MyStaticMesh);
#define OL_PREVIEW_STATIC(MeshPtr_) \
    do { GOLPreviewPendingStatic = (MeshPtr_); OLImGui_EnqueueCall(OLPreview_ApplyStatic); } while(0)

// OL_PREVIEW_CLEAR()
//   Remove the current mesh from the preview actor.
#define OL_PREVIEW_CLEAR() \
    do { GOLPreviewPendingSkeletal = NULL; OLImGui_EnqueueCall(OLPreview_ApplySkeletal); } while(0)

// OL_PREVIEW_ATTACH(Slot_, MeshPtr_, SocketName_)
//   Attach a skeletal mesh to a named socket on the main mesh.
//   Slot_       — attachment slot index 0..3.
//   MeshPtr_    — USkeletalMesh*.
//   SocketName_ — narrow string literal or const char*.
//
//   Example:
//       OL_PREVIEW_ATTACH(0, HeadMesh, "HeadSocket");
#define OL_PREVIEW_ATTACH(Slot_, MeshPtr_, SocketName_) \
    do { \
        GOLPreviewPendingAttachSlot = (Slot_); \
        GOLPreviewPendingAttachMesh = (MeshPtr_); \
        _snprintf(GOLPreviewPendingAttachSocket, 256, "%s", (SocketName_)); \
        OLImGui_EnqueueCall(OLPreview_ApplyAttach); \
    } while(0)

// OL_PREVIEW_DETACH(Slot_)
//   Remove the attachment in slot Slot_ (0..3).
//
//   Example:
//       OL_PREVIEW_DETACH(0);
#define OL_PREVIEW_DETACH(Slot_) \
    do { \
        GOLPreviewPendingAttachSlot    = (Slot_); \
        GOLPreviewPendingAttachMesh    = NULL; \
        GOLPreviewPendingAttachSocket[0] = '\0'; \
        OLImGui_EnqueueCall(OLPreview_ApplyAttach); \
    } while(0)

// OL_PREVIEW_WIDGET(W_, H_)
//   Draw the preview render target as an image of size W_ × H_ pixels,
//   preceded by a "Preview" separator.
//   Shows "No preview" placeholder when no texture is available yet.
//
//   Example:
//       OL_PREVIEW_WIDGET(256, 256);
#define OL_PREVIEW_WIDGET(W_, H_) \
    do { \
        ImGui::SeparatorText("Preview"); \
        void* _tid = OLPreview_GetImTextureID(); \
        if (_tid) { \
            ImGui::Image((ImTextureID)_tid, ImVec2((float)(W_), (float)(H_)), ImVec2(0,0), ImVec2(1,1)); \
        } \
        else { \
            ImGui::Dummy(ImVec2((float)(W_), (float)(H_))); \
            ImVec2 _dmin = ImGui::GetItemRectMin(), _dmax = ImGui::GetItemRectMax(); \
            ImGui::GetWindowDrawList()->AddRectFilled(_dmin, _dmax, IM_COL32(30,30,30,200)); \
            ImGui::GetWindowDrawList()->AddText( \
                ImVec2(_dmin.x + 8.f, _dmin.y + (float)(H_) * 0.5f - 8.f), \
                IM_COL32(120,120,120,255), "No preview"); \
        } \
    } while(0)

// ===========================================================================
// WIDGET SHORTHANDS
// ===========================================================================

// ---------------------------------------------------------------------------
// OL_SECTION(Label_)
//   Spacing + horizontal separator with a centred label. Use to open a group
//   of related controls. Label_ is a raw English string.
//
//   OL_T_SECTION(LocKey_)
//   Localised variant. LocKey_ is a locale key, e.g. "visuals.section.stats".
//
//   Example:
//       OL_SECTION("Debug");
//       OL_T_SECTION("visuals.section.stats");
// ---------------------------------------------------------------------------
#define OL_SECTION(Label_) \
    do { ImGui::Spacing(); ImGui::SeparatorText(Label_); } while(0)

// ---------------------------------------------------------------------------
// OL_ACTION(Label_, Fn_)
//   Button that dispatches Fn_ to the game thread when clicked.
//   Inside OL_TABLE_CELL the button fills the column width.
//   Fn_ must be a static void(void) function.
//
//   OL_T_ACTION(LocKey_, Fn_)
//   Localised variant — font is scaled down if the translated text is wider
//   than its English reference.
//
//   Example:
//       static void DoReset() { GMyVar = 0; }
//       OL_ACTION("Reset", DoReset);
//       OL_T_ACTION("actions.reset", DoReset);
// ---------------------------------------------------------------------------
#define OL_ACTION(Label_, Fn_) \
    do { if (ImGui::Button(Label_, _ol_in_table_cell ? ImVec2(-1.f,0.f) : ImVec2(0.f,0.f))) OLImGui_EnqueueCall(Fn_); } while(0)

// OL_ACTION_B(Id_, Label_, Fn_) — bindable variant.
#define OL_ACTION_B(Id_, Label_, Fn_) \
    do { if (ImGui::Button(Label_, _ol_in_table_cell ? ImVec2(-1.f,0.f) : ImVec2(0.f,0.f))) OLImGui_EnqueueCall(Fn_); \
         _OL_BIND_LAST_(Id_, Fn_); } while(0)

// ---------------------------------------------------------------------------
// OL_TOGGLE(Label_, State_, Fn_)
//   Button that shows a white glow border when State_ is true, plain otherwise.
//   Clicking dispatches Fn_ to the game thread (the function should flip
//   whatever bool drives State_).
//
//   OL_T_TOGGLE(LocKey_, State_, Fn_)
//   Localised variant.
//
//   Example:
//       OL_TOGGLE("Free cam", GOLPlayerState.bFreeCam, ToggleFreeCam);
//       OL_T_TOGGLE("visuals.inspector.freecam", GOLPlayerState.bFreeCam, ToggleFreeCam);
// ---------------------------------------------------------------------------
#define OL_TOGGLE(Label_, State_, Fn_) \
    do { \
        const bool _on = (bool)(State_); \
        if (_on) { \
            ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 2.f); \
            ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(1.f, 1.f, 1.f, 1.f)); \
        } \
        if (ImGui::Button(Label_, _ol_in_table_cell ? ImVec2(-1.f,0.f) : ImVec2(0.f,0.f))) OLImGui_EnqueueCall(Fn_); \
        if (_on) { \
            ImGui::PopStyleColor(); \
            ImGui::PopStyleVar(); \
            const ImVec2 _rmin = ImGui::GetItemRectMin(); \
            const ImVec2 _rmax = ImGui::GetItemRectMax(); \
            ImDrawList* _dl = ImGui::GetWindowDrawList(); \
            _dl->AddRect(ImVec2(_rmin.x-2.f,_rmin.y-2.f), ImVec2(_rmax.x+2.f,_rmax.y+2.f), IM_COL32(255,255,255,60), 3.f, 0, 1.f); \
            _dl->AddRect(ImVec2(_rmin.x-4.f,_rmin.y-4.f), ImVec2(_rmax.x+4.f,_rmax.y+4.f), IM_COL32(255,255,255,25), 4.f, 0, 1.f); \
            _dl->AddRect(ImVec2(_rmin.x-6.f,_rmin.y-6.f), ImVec2(_rmax.x+6.f,_rmax.y+6.f), IM_COL32(255,255,255,10), 5.f, 0, 1.f); \
        } \
    } while(0)

// OL_TOGGLE_B(Id_, Label_, State_, Fn_) — bindable variant.
#define OL_TOGGLE_B(Id_, Label_, State_, Fn_) \
    do { \
        const bool _on = (bool)(State_); \
        if (_on) { \
            ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 2.f); \
            ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(1.f, 1.f, 1.f, 1.f)); \
        } \
        if (ImGui::Button(Label_, _ol_in_table_cell ? ImVec2(-1.f,0.f) : ImVec2(0.f,0.f))) OLImGui_EnqueueCall(Fn_); \
        if (_on) { \
            ImGui::PopStyleColor(); \
            ImGui::PopStyleVar(); \
            const ImVec2 _rmin = ImGui::GetItemRectMin(); \
            const ImVec2 _rmax = ImGui::GetItemRectMax(); \
            ImDrawList* _dl = ImGui::GetWindowDrawList(); \
            _dl->AddRect(ImVec2(_rmin.x-2.f,_rmin.y-2.f), ImVec2(_rmax.x+2.f,_rmax.y+2.f), IM_COL32(255,255,255,60), 3.f, 0, 1.f); \
            _dl->AddRect(ImVec2(_rmin.x-4.f,_rmin.y-4.f), ImVec2(_rmax.x+4.f,_rmax.y+4.f), IM_COL32(255,255,255,25), 4.f, 0, 1.f); \
            _dl->AddRect(ImVec2(_rmin.x-6.f,_rmin.y-6.f), ImVec2(_rmax.x+6.f,_rmax.y+6.f), IM_COL32(255,255,255,10), 5.f, 0, 1.f); \
        } \
        _OL_BIND_LAST_(Id_, Fn_); \
    } while(0)

// ---------------------------------------------------------------------------
// OL_BOOL(Label_, BoolPtr_, Fn_)
//   Checkbox bound to *BoolPtr_. Dispatches Fn_ on change.
//   The checkbox updates the bool immediately on the render thread;
//   Fn_ is used for any game-thread side-effect (can be a no-op lambda).
//
//   OL_T_BOOL(LocKey_, BoolPtr_, Fn_)
//   Localised variant.
//
//   Example:
//       static bool bWire = false;
//       static void ApplyWire() { ... }
//       OL_BOOL("Wireframe", &bWire, ApplyWire);
// ---------------------------------------------------------------------------
#define OL_BOOL(Label_, BoolPtr_, Fn_) \
    do { if (ImGui::Checkbox(Label_, BoolPtr_)) OLImGui_EnqueueCall(Fn_); } while(0)

// ---------------------------------------------------------------------------
// OL_DRAG_INT(Label_, Ptr_, Speed_)
// OL_DRAG_F (Label_, Ptr_, Speed_)
//   DragInt / DragFloat with a label rendered by ImGui (left of the widget).
//   The value is written directly — dispatch manually if a game-thread callback
//   is needed. Speed_ is pixels-per-unit drag sensitivity.
//
//   Example:
//       static int GDamage = 10;
//       OL_DRAG_INT("Damage", &GDamage, 1);
//       OL_DRAG_F("Speed",   &GSpeed,  0.1f);
// ---------------------------------------------------------------------------
#define OL_DRAG_INT(Label_, Ptr_, Speed_) \
    ImGui::DragInt(Label_, Ptr_, (float)(Speed_))

#define OL_DRAG_F(Label_, Ptr_, Speed_) \
    ImGui::DragFloat(Label_, Ptr_, Speed_)

// ---------------------------------------------------------------------------
// OL_SLIDER_F(Label_, Ptr_, Min_, Max_)
// OL_SLIDER_I(Label_, Ptr_, Min_, Max_)
//   SliderFloat / SliderInt. Label rendered left of the widget by ImGui.
//
//   Example:
//       OL_SLIDER_F("FOV", &GFOV, 60.f, 120.f);
//       OL_SLIDER_I("Count", &GCount, 1, 10);
// ---------------------------------------------------------------------------
#define OL_SLIDER_F(Label_, Ptr_, Min_, Max_) \
    ImGui::SliderFloat(Label_, Ptr_, Min_, Max_)

#define OL_SLIDER_I(Label_, Ptr_, Min_, Max_) \
    ImGui::SliderInt(Label_, Ptr_, Min_, Max_)

// ---------------------------------------------------------------------------
// OL_COLOR3(Label_, Ptr_)
// OL_COLOR4(Label_, Ptr_)
//   ColorEdit3 / ColorEdit4. Ptr_ is float[3] or float[4].
//
//   Example:
//       static float FogColor[3] = {0.5f, 0.5f, 0.5f};
//       OL_COLOR3("Fog color", FogColor);
// ---------------------------------------------------------------------------
#define OL_COLOR3(Label_, Ptr_) \
    ImGui::ColorEdit3(Label_, Ptr_)

#define OL_COLOR4(Label_, Ptr_) \
    ImGui::ColorEdit4(Label_, Ptr_)

// ===========================================================================
// LOCALISED VARIANTS (OL_T_*)
// Font is scaled down automatically when the translated text exceeds the
// English reference width, then reset to 1.f after the widget.
// Do NOT nest OL_T_* macros inside each other.
// ===========================================================================

// Internal scale helpers — not for direct use.
#define OL_T_SCALE_BEGIN_(LocKey_) \
    { float _ol_t_sc_ = OLLocale_FitScale(LocKey_); ImGui::SetWindowFontScale(_ol_t_sc_);
#define OL_T_SCALE_END_ \
    ImGui::SetWindowFontScale(1.f); }

#define OL_T_SCALE_LABEL_BEGIN_(LocKey_) \
    { float _ol_t_sc_ = OLLocale_FitScaleLabel(LocKey_); ImGui::SetWindowFontScale(_ol_t_sc_);

#define OL_T_ACTION(LocKey_, Fn_) \
    OL_T_SCALE_BEGIN_(LocKey_) OL_ACTION(OLLocale_T(LocKey_), Fn_); OL_T_SCALE_END_

#define OL_T_ACTION_B(Id_, LocKey_, Fn_) \
    OL_T_SCALE_BEGIN_(LocKey_) OL_ACTION_B(Id_, OLLocale_T(LocKey_), Fn_); OL_T_SCALE_END_

#define OL_T_TOGGLE(LocKey_, State_, Fn_) \
    OL_T_SCALE_BEGIN_(LocKey_) OL_TOGGLE(OLLocale_T(LocKey_), State_, Fn_); OL_T_SCALE_END_

#define OL_T_TOGGLE_B(Id_, LocKey_, State_, Fn_) \
    OL_T_SCALE_BEGIN_(LocKey_) OL_TOGGLE_B(Id_, OLLocale_T(LocKey_), State_, Fn_); OL_T_SCALE_END_

#define OL_T_BOOL(LocKey_, BoolPtr_, Fn_) \
    OL_T_SCALE_BEGIN_(LocKey_) OL_BOOL(OLLocale_T(LocKey_), BoolPtr_, Fn_); OL_T_SCALE_END_

#define OL_T_SECTION(LocKey_) \
    OL_T_SCALE_BEGIN_(LocKey_) OL_SECTION(OLLocale_T(LocKey_)); OL_T_SCALE_END_

// ---------------------------------------------------------------------------
// OL_T_L_* — localised label-first variants.
// Same as OL_L_* but LocKey_ is resolved through OLLocale_T().
// Font scale is applied to the label; widget chrome renders at normal scale.
//
//   Example:
//       OL_T_L_BOOL("player.godmode", &bGod, ApplyGod);
//       OL_T_L_DRAG_F("player.speed", &GSpeed, 0.5f);
//       OL_T_L_SLIDER_F("player.fov", &GFOV, 60.f, 120.f);
// ---------------------------------------------------------------------------
#define OL_T_L_BOOL(LocKey_, BoolPtr_, Fn_) \
    OL_T_SCALE_LABEL_BEGIN_(LocKey_) OL_L_BOOL(OLLocale_T(LocKey_), BoolPtr_, Fn_); OL_T_SCALE_END_

#define OL_T_L_DRAG_INT(LocKey_, Ptr_, Speed_) \
    OL_T_SCALE_LABEL_BEGIN_(LocKey_) OL_L_DRAG_INT(OLLocale_T(LocKey_), Ptr_, Speed_); OL_T_SCALE_END_

#define OL_T_L_DRAG_F(LocKey_, Ptr_, Speed_) \
    OL_T_SCALE_LABEL_BEGIN_(LocKey_) OL_L_DRAG_F(OLLocale_T(LocKey_), Ptr_, Speed_); OL_T_SCALE_END_

#define OL_T_L_SLIDER_F(LocKey_, Ptr_, Min_, Max_) \
    OL_T_SCALE_LABEL_BEGIN_(LocKey_) OL_L_SLIDER_F(OLLocale_T(LocKey_), Ptr_, Min_, Max_); OL_T_SCALE_END_

#define OL_T_L_SLIDER_I(LocKey_, Ptr_, Min_, Max_) \
    OL_T_SCALE_LABEL_BEGIN_(LocKey_) OL_L_SLIDER_I(OLLocale_T(LocKey_), Ptr_, Min_, Max_); OL_T_SCALE_END_

// ===========================================================================
// SEARCH BOX
// ===========================================================================

// ---------------------------------------------------------------------------
// OL_SEARCH(Id_, HintKey_, Buf_, BufSz_)
//   Full-width search input with a localised placeholder hint.
//   Equivalent to SetNextItemWidth(-1) + InputTextWithHint.
//   Returns bool (true when the text changed), like InputTextWithHint.
//
//   Id_      — unique ImGui widget id, e.g. "##myfilter"
//   HintKey_ — locale key for the placeholder, e.g. "spawns.enemies.search"
//   Buf_     — char buffer to write into
//   BufSz_   — sizeof(Buf_)
//
//   Example:
//       static char Filter[128] = "";
//       if (OL_SEARCH("##myfilter", "section.search", Filter, sizeof(Filter)))
//           RebuildList();
//       for (int i = 0; i < N; ++i)
//           if (OLImGui_MatchFilter(Items[i], Filter))
//               ImGui::Selectable(Items[i], false);
// ---------------------------------------------------------------------------
#define OL_SEARCH(Id_, HintKey_, Buf_, BufSz_) \
    (ImGui::SetNextItemWidth(-1.f), \
     ImGui::InputTextWithHint(Id_, OLLocale_T(HintKey_), Buf_, BufSz_))

// ===========================================================================
// COMBO (DROPDOWN)
// ===========================================================================

// ---------------------------------------------------------------------------
// OL_COMBO(Label_, Items_, Count_, CurrentIdx_)
//   "Label  [dropdown]" on one row. Dropdown fills remaining width.
//   Returns bool (true when selection changed), like ImGui::Combo.
//
//   Label_      — raw string rendered left of the widget
//   Items_      — const char* array of display names
//   Count_      — number of items in Items_
//   CurrentIdx_ — int& holding the selected index
//
//   Example:
//       static const char* Modes[] = { "Easy", "Normal", "Hard" };
//       static int GMode = 0;
//       if (OL_COMBO("Difficulty", Modes, 3, GMode))
//           OLImGui_EnqueueCall(ApplyDifficulty);
// ---------------------------------------------------------------------------
#define OL_COMBO(Label_, Items_, Count_, CurrentIdx_) \
    (ImGui::TextUnformatted(Label_), ImGui::SameLine(), \
     ImGui::SetNextItemWidth(-1.f), \
     ImGui::Combo("##" Label_, &(CurrentIdx_), (Items_), (Count_)))

// ===========================================================================
// TABLE GRID (OL_TABLE_*)
// ===========================================================================

// ---------------------------------------------------------------------------
// OL_TABLE_BEGIN(Id_, Cols_, Split_, Col_)
// OL_TABLE_CELL
// OL_TABLE_END
//
//   Multi-column grid where each cell contains one widget (DBG_CMD / OL_ACTION /
//   OL_TOGGLE). Widgets inside OL_TABLE_CELL automatically stretch to fill
//   their column (_ol_in_table_cell is set to true).
//
//   Id_    — unique ImGui table id string, e.g. "##stats"
//   Cols_  — number of equal-width columns
//   Split_ — true:  items flow left-to-right, wrapping to the next row
//                   (Col_ is ignored; use for grids of buttons)
//           — false: every OL_TABLE_CELL opens a new row and places the item
//                   in column Col_ (use for a fixed-column label/value layout)
//   Col_   — column index used when Split_ = false (0-based)
//
//   Example (3-column button grid):
//       OL_TABLE_BEGIN("##stats", 3, true, 0)
//           OL_TABLE_CELL  DBG_CMD("Stat FPS",    "Stat FPS");
//           OL_TABLE_CELL  DBG_CMD("Stat UNIT",   "Stat UNIT");
//           OL_TABLE_CELL  DBG_CMD("Stat LEVELS", "Stat LEVELS");
//       OL_TABLE_END
//
//   Example (single-column list, each row in column 0):
//       OL_TABLE_BEGIN("##flags", 1, false, 0)
//           OL_TABLE_CELL  OL_TOGGLE("Wireframe", bWire, ToggleWire);
//           OL_TABLE_CELL  OL_TOGGLE("Collision", bColl, ToggleColl);
//       OL_TABLE_END
// ---------------------------------------------------------------------------
static bool _ol_table_split   = true;
static int  _ol_table_col     = 0;
static bool _ol_in_table_cell = false;

#define OL_TABLE_BEGIN(Id_, Cols_, Split_, Col_) \
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(4.f, 4.f)); \
    _ol_table_split = (bool)(Split_); \
    _ol_table_col   = (int)(Col_); \
    _ol_in_table_cell = false; \
    if (ImGui::BeginTable(Id_, Cols_, ImGuiTableFlags_SizingStretchSame)) {

#define OL_TABLE_CELL \
    _ol_in_table_cell = true; \
    if (_ol_table_split) { \
        ImGui::TableNextColumn(); \
    } else { \
        ImGui::TableNextRow(); \
        ImGui::TableSetColumnIndex(_ol_table_col); \
    }

#define OL_TABLE_END \
        ImGui::EndTable(); \
    } \
    _ol_in_table_cell = false; \
    ImGui::PopStyleVar();

// ===========================================================================
// LABEL-FIRST LAYOUT  (OL_L_*)
// "Label  [widget]" on a single row. The widget ID is derived from the label
// via a "##" suffix, so Label_ must be a unique string literal in its scope.
// For runtime strings use OL_L_DRAG_INT / OL_L_DRAG_F (they use __LINE__).
// ===========================================================================

// ---------------------------------------------------------------------------
// OL_L_BOOL(Label_, BoolPtr_, Fn_)
//   "Label  [checkbox]". Dispatches Fn_ on change.
//
//   Example:
//       static bool bGod = false;
//       OL_L_BOOL("God mode", &bGod, ApplyGod);
// ---------------------------------------------------------------------------
#define OL_L_BOOL(Label_, BoolPtr_, Fn_) \
    do { \
        ImGui::TextUnformatted(Label_); ImGui::SameLine(); \
        if (ImGui::Checkbox("##" Label_, BoolPtr_)) OLImGui_EnqueueCall(Fn_); \
    } while(0)

// Internal: unique widget IDs derived from source line to avoid collisions
// when the same macro is used multiple times in one scope with runtime labels.
#define OL_L_DETAIL2_(x) #x
#define OL_L_DETAIL_(x)  OL_L_DETAIL2_(x)
#define OL_L_ID_         "##_l" OL_L_DETAIL_(__LINE__)

// ---------------------------------------------------------------------------
// OL_L_DRAG_INT(Label_, Ptr_, Speed_)
// OL_L_DRAG_F (Label_, Ptr_, Speed_)
//   "Label  [drag widget]". Widget fills remaining row width.
//   Label_ may be a runtime string (ID uses __LINE__).
//
//   Example:
//       OL_L_DRAG_INT("Damage",  &GDamage, 1);
//       OL_L_DRAG_F  ("Speed",   &GSpeed,  0.1f);
// ---------------------------------------------------------------------------
#define OL_L_DRAG_INT(Label_, Ptr_, Speed_) \
    (ImGui::TextUnformatted(Label_), ImGui::SameLine(), \
     ImGui::DragInt(OL_L_ID_, Ptr_, (float)(Speed_)))

#define OL_L_DRAG_F(Label_, Ptr_, Speed_) \
    (ImGui::TextUnformatted(Label_), ImGui::SameLine(), \
     ImGui::DragFloat(OL_L_ID_, Ptr_, Speed_))

// ---------------------------------------------------------------------------
// OL_L_SLIDER_F(Label_, Ptr_, Min_, Max_)
// OL_L_SLIDER_I(Label_, Ptr_, Min_, Max_)
//   "Label  [slider]". Slider stretches to fill the remaining row width.
//   For multiple sliders on one row use OL_L_SLIDER_F_N with OL_ROW_SLIDER_WIDTH_N.
//
//   Example:
//       OL_L_SLIDER_F("FOV",   &GFOV,   60.f, 120.f);
//       OL_L_SLIDER_I("Count", &GCount, 1,    10);
// ---------------------------------------------------------------------------
#define OL_L_SLIDER_F(Label_, Ptr_, Min_, Max_) \
    (ImGui::TextUnformatted(Label_), ImGui::SameLine(), \
     ImGui::SetNextItemWidth(-1.f), \
     ImGui::SliderFloat(OL_L_ID_, Ptr_, Min_, Max_))

#define OL_L_SLIDER_I(Label_, Ptr_, Min_, Max_) \
    (ImGui::TextUnformatted(Label_), ImGui::SameLine(), \
     ImGui::SetNextItemWidth(-1.f), \
     ImGui::SliderInt(OL_L_ID_, Ptr_, Min_, Max_))

// ---------------------------------------------------------------------------
// OL_L_COLOR3 / OL_L_COLOR4
//   "Label  [color picker]".
//
//   Example:
//       static float Fog[3] = {0.5f,0.5f,0.5f};
//       OL_L_COLOR3("Fog", Fog);
// ---------------------------------------------------------------------------
#define OL_L_COLOR3(Label_, Ptr_) \
    (ImGui::TextUnformatted(Label_), ImGui::SameLine(), \
     ImGui::ColorEdit3("##" Label_, Ptr_))

#define OL_L_COLOR4(Label_, Ptr_) \
    (ImGui::TextUnformatted(Label_), ImGui::SameLine(), \
     ImGui::ColorEdit4("##" Label_, Ptr_))

// ===========================================================================
// MULTI-SLIDER ROW  (OL_ROW_SLIDER_WIDTH_N + OL_L_SLIDER_*_N)
// ===========================================================================

// ---------------------------------------------------------------------------
// OL_ROW_SLIDER_WIDTH_1/2/3(labels...)
//   Compute the pixel width each slider should get when N sliders with their
//   labels share one row. Call once before the first slider, pass the result
//   as SliderW_ to each OL_L_SLIDER_F_N / OL_L_SLIDER_I_N.
//   Item spacing between sliders is subtracted automatically.
//
// OL_L_SLIDER_F_N(Label_, Ptr_, Min_, Max_, SliderW_)
// OL_L_SLIDER_I_N(Label_, Ptr_, Min_, Max_, SliderW_)
//   Same as OL_L_SLIDER_F/I but uses an explicit width instead of -1.
//   Separate items with ImGui::SameLine().
//
//   Example (two sliders on one row):
//       float W = OL_ROW_SLIDER_WIDTH_2("Min FPS", "Max FPS");
//       OL_L_SLIDER_F_N("Min FPS", &mn, 5.f, 120.f, W);
//       ImGui::SameLine();
//       OL_L_SLIDER_F_N("Max FPS", &mx, 5.f, 500.f, W);
// ---------------------------------------------------------------------------
#define OL_LABEL_W(Label_) \
    (ImGui::CalcTextSize(Label_).x + ImGui::GetStyle().ItemSpacing.x)

#define OL_ROW_SLIDER_WIDTH_1(L1_) \
    ((ImGui::GetContentRegionAvail().x - OL_LABEL_W(L1_)) / 1.f)

#define OL_ROW_SLIDER_WIDTH_2(L1_, L2_) \
    ((ImGui::GetContentRegionAvail().x - OL_LABEL_W(L1_) - OL_LABEL_W(L2_) - ImGui::GetStyle().ItemSpacing.x) / 2.f)

#define OL_ROW_SLIDER_WIDTH_3(L1_, L2_, L3_) \
    ((ImGui::GetContentRegionAvail().x - OL_LABEL_W(L1_) - OL_LABEL_W(L2_) - OL_LABEL_W(L3_) - ImGui::GetStyle().ItemSpacing.x * 2.f) / 3.f)

#define OL_L_SLIDER_F_N(Label_, Ptr_, Min_, Max_, SliderW_) \
    (ImGui::TextUnformatted(Label_), ImGui::SameLine(), \
     ImGui::SetNextItemWidth(SliderW_), \
     ImGui::SliderFloat("##" Label_, Ptr_, Min_, Max_))

#define OL_L_SLIDER_I_N(Label_, Ptr_, Min_, Max_, SliderW_) \
    (ImGui::TextUnformatted(Label_), ImGui::SameLine(), \
     ImGui::SetNextItemWidth(SliderW_), \
     ImGui::SliderInt("##" Label_, Ptr_, Min_, Max_))
