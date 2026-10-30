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
// OLImGui_ToggleKeyCharFilter
//   InputText callback that discards the overlay toggle key character.
//   Pass as callback with ImGuiInputTextFlags_CallbackCharFilter to prevent
//   the toggle key char from appearing in any InputText field.
// ---------------------------------------------------------------------------
inline int OLImGui_ToggleKeyCharFilter(ImGuiInputTextCallbackData* Data)
{
    if (Data->EventFlag & ImGuiInputTextFlags_CallbackCharFilter)
    {
        ImGuiKey ToggleKey = (ImGuiKey)OLImGui_GetOverlayToggleImKey();
        if (ToggleKey != ImGuiKey_None &&
            (ImGui::IsKeyDown(ToggleKey) || ImGui::IsKeyReleased(ToggleKey)))
            return 1; // discard
    }
    return 0;
}

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

// Same, plus Desc_ (a plain const char* — already-resolved text) shown as the
// first line of the same tooltip, above "RMB to bind/rebind".
#define _OL_BIND_LAST_D_(Id_, Fn_, Desc_) \
    OLImGui_Bindings_RegisterLast(Id_, Fn_, Desc_)

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
void OLSpawns_DespawnTestActors();    // destroy all editor-spawned props/doors/lights/enemies (map change, player death)

typedef bool (*FPlacementConfirmFn)(AOLPlayerController*);
void OLSpawns_RegisterConfirmHandler(FPlacementConfirmFn Fn); // register a placement confirm handler
void OLSpawns_DispatchConfirm();      // call once per tick — routes ghost to the right handler
void OLImGui_TabSpawns_Enemies();     // enemies sub-tab (list / new / detail views)
void OLImGui_TabSpawns_Lighting();    // lighting sub-tab (place movable lights)
void OLSpawns_TickLightPlacement();   // sync light ghost params to PC each tick
void OLSpawns_TickLightConfirm();     // consume LMB confirm for light ghosts
void OLSpawns_TickLightList();        // rebuild CustomSpawnedLight list snapshot
void OLImGui_TabSpawns_Doors();       // doors sub-tab (spawn OLDoor with params)
void OLSpawns_TickDoors();            // rebuild CustomSpawnedDoor list + confirm placement
void OLImGui_TabSpawns_Props();       // props sub-tab (StaticMesh content browser)
void OLSpawns_ScanProps();            // advance incremental scan + rebuild placed list (always)
void OLSpawns_TickProps();            // load pending preview mesh + tick render (menu-open only)
void OLSpawns_ShutdownProps();        // release LRU cache and preview instance
void OLSpawns_Props_BeginScan();      // kick off package enumeration (call on menu open)
UStaticMesh* OLSpawns_Props_GetRandomMesh(); // random prop mesh for hub tile preview
void OLImGui_TabSpawns_Fog();         // fog sub-tab (edit the level's ExponentialHeightFog)
void OLSpawns_TickFog();              // load/apply fog params (call every tick, menu open or not)

struct FHubEnemyMesh
{
    USkeletalMesh* Mesh;
    const char*    AnimSetPkg;
    const char*    AnimSetObj;
    const char*    IdleAnim;
};
bool OLSpawns_Enemies_GetRandomMesh(FHubEnemyMesh& Out); // random enemy for hub tile preview
void OLSpawns_TickHubPreviews();      // tick enemy + door + prop hub tile previews
bool OLSpawns_IsHubVisible();         // true when hub grid is shown (no sub-tab open)
bool OLSpawns_IsEnemyTabActive();     // true when Enemies sub-tab is open
bool OLSpawns_IsFogTabActive();       // true when Fog sub-tab is open
void OLSpawns_ShutdownHubPreviews();  // shutdown hub tile preview instances (called from OLSpawns_ShutdownPreviews)

// OLImGui_InvalidateCaches
//   Call before PrepareMapChange or CollectGarbage.
//   Clears all raw UObject* caches not protected by AddToRoot/UPROPERTY
//   so the next tick reloads them instead of touching freed memory.
void OLImGui_InvalidateCaches();
void OLImGui_Scene_Invalidate(); // clear selected object + nav stack (call on map change)
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

// OL_ACTION_BD(Id_, Label_, Fn_, Desc_) — bindable + hover description.
#define OL_ACTION_BD(Id_, Label_, Fn_, Desc_) \
    do { if (ImGui::Button(Label_, _ol_in_table_cell ? ImVec2(-1.f,0.f) : ImVec2(0.f,0.f))) OLImGui_EnqueueCall(Fn_); \
         _OL_BIND_LAST_D_(Id_, Fn_, Desc_); } while(0)

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

// OL_TOGGLE_BD(Id_, Label_, State_, Fn_, Desc_) — bindable + hover description.
#define OL_TOGGLE_BD(Id_, Label_, State_, Fn_, Desc_) \
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
        _OL_BIND_LAST_D_(Id_, Fn_, Desc_); \
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

// OL_T_ACTION_BD(Id_, LocKey_, Fn_, DescKey_) — DescKey_ is a locale key
// resolved via OLLocale_T(), shown as the first line of the bind tooltip.
#define OL_T_ACTION_BD(Id_, LocKey_, Fn_, DescKey_) \
    OL_T_SCALE_BEGIN_(LocKey_) OL_ACTION_BD(Id_, OLLocale_T(LocKey_), Fn_, OLLocale_T(DescKey_)); OL_T_SCALE_END_

#define OL_T_TOGGLE(LocKey_, State_, Fn_) \
    OL_T_SCALE_BEGIN_(LocKey_) OL_TOGGLE(OLLocale_T(LocKey_), State_, Fn_); OL_T_SCALE_END_

#define OL_T_TOGGLE_B(Id_, LocKey_, State_, Fn_) \
    OL_T_SCALE_BEGIN_(LocKey_) OL_TOGGLE_B(Id_, OLLocale_T(LocKey_), State_, Fn_); OL_T_SCALE_END_

// OL_T_TOGGLE_BD(Id_, LocKey_, State_, Fn_, DescKey_) — DescKey_ is a locale
// key resolved via OLLocale_T(), shown as the first line of the bind tooltip.
#define OL_T_TOGGLE_BD(Id_, LocKey_, State_, Fn_, DescKey_) \
    OL_T_SCALE_BEGIN_(LocKey_) OL_TOGGLE_BD(Id_, OLLocale_T(LocKey_), State_, Fn_, OLLocale_T(DescKey_)); OL_T_SCALE_END_

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

#define OL_T_L_COLOR3(LocKey_, Ptr_) \
    OL_T_SCALE_LABEL_BEGIN_(LocKey_) OL_L_COLOR3(OLLocale_T(LocKey_), Ptr_); OL_T_SCALE_END_

#define OL_T_L_COLOR4(LocKey_, Ptr_) \
    OL_T_SCALE_LABEL_BEGIN_(LocKey_) OL_L_COLOR4(OLLocale_T(LocKey_), Ptr_); OL_T_SCALE_END_

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
     ImGui::InputTextWithHint(Id_, OLLocale_T(HintKey_), Buf_, BufSz_, \
         ImGuiInputTextFlags_CallbackCharFilter, OLImGui_ToggleKeyCharFilter))

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
// ROW ACTION MENU  (OL_ROW_MENU_*)
// "..." button in a table cell that opens a popup of row actions (Teleport,
// Delete, etc). Each item copies the row's identifying name into a
// caller-supplied buffer before dispatching its handler to the game thread,
// since the handler runs later and needs to know which row it applies to.
//
//   OL_ROW_MENU_BEGIN(PopupId_)
//     Draws the "..." button (fills the table cell) and opens/begins the
//     popup. PopupId_ must be unique per row (e.g. a snprintf'd "##menu%d").
//     Wrap the item macros below in `if (OL_ROW_MENU_BEGIN(...)) { ... }`.
//
//   OL_ROW_MENU_ITEM(LocKey_, DestBuf_, DestBufSz_, Name_, Fn_)
//     Normal-colored menu item. On click: snprintf's Name_ into DestBuf_
//     (size DestBufSz_), then OLImGui_EnqueueCall(Fn_).
//
//   OL_ROW_MENU_ITEM_DANGER(LocKey_, DestBuf_, DestBufSz_, Name_, Fn_)
//     Same, but rendered in red (for destructive actions like Delete).
//
//   OL_ROW_MENU_SEPARATOR
//     Thin wrapper over ImGui::Separator() — use between normal and danger items.
//
//   OL_ROW_MENU_END
//     Closes the popup opened by OL_ROW_MENU_BEGIN. Always call this exactly
//     once, matching the `if` block that guarded the item macros.
//
//   Example:
//       char PopupId[32];
//       _snprintf(PopupId, sizeof(PopupId), "##dmenu%d", i);
//       if (OL_ROW_MENU_BEGIN(PopupId))
//       {
//           OL_ROW_MENU_ITEM("spawns.action.teleport", GPendingDoorAction, sizeof(GPendingDoorAction), De.Name, Action_TeleportDoor);
//           OL_ROW_MENU_SEPARATOR;
//           OL_ROW_MENU_ITEM_DANGER("spawns.action.delete", GPendingDoorAction, sizeof(GPendingDoorAction), De.Name, Action_DeleteDoor);
//           OL_ROW_MENU_END;
//       }
// ---------------------------------------------------------------------------
#define OL_ROW_MENU_BEGIN(PopupId_) \
    ((ImGui::Button("...", ImVec2(-1.f, 0.f)) ? (ImGui::OpenPopup(PopupId_), 0) : 0), ImGui::BeginPopup(PopupId_))

#define OL_ROW_MENU_ITEM(LocKey_, DestBuf_, DestBufSz_, Name_, Fn_) \
    do { if (ImGui::MenuItem(OLLocale_T(LocKey_))) { \
        _snprintf((DestBuf_), (DestBufSz_), "%s", (Name_)); \
        OLImGui_EnqueueCall(Fn_); \
    } } while(0)

#define OL_ROW_MENU_ITEM_DANGER(LocKey_, DestBuf_, DestBufSz_, Name_, Fn_) \
    do { \
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.f, 0.35f, 0.35f, 1.f)); \
        OL_ROW_MENU_ITEM(LocKey_, DestBuf_, DestBufSz_, Name_, Fn_); \
        ImGui::PopStyleColor(); \
    } while(0)

#define OL_ROW_MENU_SEPARATOR ImGui::Separator()

#define OL_ROW_MENU_END ImGui::EndPopup()

// ===========================================================================
// ROW MULTI-SELECT  (FOLRowMultiSelect, OL_ROW_MULTISELECT_*, OL_ROW_SELECTABLE, OL_ROW_MULTIMENU_*)
//
// Thin wrapper around ImGui's own BeginMultiSelect()/EndMultiSelect() API
// (imgui.h, ImGuiSelectionBasicStorage) for Ctrl/Shift-click multi-select
// over a table's rows, plus a right-click context menu for bulk actions
// (Delete, etc) over the current selection. One FOLRowMultiSelect instance
// per list/table. Selection is keyed by row INDEX into the current
// filtered/visible list (ImGuiSelectionBasicStorage's default adapter), not
// by name — indices only need to stay valid for the current frame's list,
// which every Spawns tab already rebuilds each frame from its snapshot.
//
// We do NOT hand-roll io.KeyCtrl/io.KeyShift checks after Selectable() like
// an earlier version of this macro did: io.KeyCtrl/io.KeyShift are legacy
// bools only refreshed once at the top of NewFrame(), and reading them here
// produced stale/incorrect results (see [[project_imgui_keymod_bug]] in
// memory). BeginMultiSelect() reads modifiers itself, internally, at the
// correct point in the frame — so routing everything through it sidesteps
// the whole class of bug instead of chasing it further.
//
//   static FOLRowMultiSelect GSel;
//
//   OL_ROW_MULTISELECT_BEGIN(Sel_, ItemsCount_)
//     Call once, right after ImGui::BeginTable()/TableHeadersRow() and
//     before the ImGuiListClipper loop. ItemsCount_ is the number of rows
//     in the CURRENT filtered/visible list (e.g. FilteredCount). Declares a
//     local `_ms_io` used by OL_ROW_MULTISELECT_INCLUDE_RANGE_SRC below.
//
//   OL_ROW_MULTISELECT_INCLUDE_RANGE_SRC(Clipper_)
//     Call once, right after ImGuiListClipper::Begin() and before the
//     Step() loop. Ensures the Shift-range anchor row is never clipped out
//     (required whenever a clipper is used with multi-select).
//
//   OL_ROW_SELECTABLE(Sel_, DisplayLabel_, RowIdx_)
//     Draws DisplayLabel_ as a Selectable bound to selection index RowIdx_
//     (this row's position in the CURRENT filtered/visible list — pass the
//     loop index, not a stable id). Must be called between
//     OL_ROW_MULTISELECT_BEGIN and OL_ROW_MULTISELECT_END.
//
//   OL_ROW_MULTISELECT_END(Sel_)
//     Call once, right after the clipper loop ends (Clipper.End()) and
//     before ImGui::EndTable().
//
//   OL_ROW_MULTIMENU_BEGIN(Sel_, PopupId_)
//     Right-click-anywhere-in-table opens a popup for bulk actions over the
//     current selection. Put this once after OL_ROW_MULTISELECT_END (not
//     per row) — it opens on ImGui::IsMouseClicked(1) while hovering the
//     table. Wrap the item macros below in `if (OL_ROW_MULTIMENU_BEGIN(...)) { ... }`.
//
//   OL_ROW_MULTIMENU_ITEM_DANGER(LocKey_, Sel_, KeyAtRowVar_, KeyAtRowExpr_, DestBuf_, DestBufSz_, Fn_)
//     Red menu item that queues Fn_ once per selected row via
//     OLImGui_EnqueueCall — same as every other Spawns action macro.
//     KeyAtRowExpr_ is an expression (using KeyAtRowVar_ as the row index)
//     that evaluates to the `const char*` key of a row — e.g.
//     `Snap.Entries[GFilteredIdx[KeyAtRowVar_]].Name` — since selection
//     indices must be resolved back to stable names before Fn_ runs (by
//     which point this frame's filtered list may already be gone). Fn_
//     itself only reads DestBuf_ when it actually runs (after this frame's
//     ImGui build finishes), so all queued calls must be given a chance to
//     read their own DestBuf_ snapshot before the next one overwrites it;
//     this copies the key into a per-selection scratch slot and gives each
//     queued call its own copy via a tiny per-item shim, so calling Fn_
//     directly here (which would destroy the actor mid-frame, while ImGui/
//     the renderer may still be touching it this same frame) is never done.
//     Clears the selection after queuing.
//
//   Example (GFilteredIdx maps the visible row index to the entry index in
//   Snap.Entries):
//       if (ImGui::BeginTable(...))
//       {
//           ImGui::TableHeadersRow();
//           OL_ROW_MULTISELECT_BEGIN(GSel, FilteredCount);
//
//           ImGuiListClipper Clipper;
//           Clipper.Begin(FilteredCount);
//           OL_ROW_MULTISELECT_INCLUDE_RANGE_SRC(Clipper);
//           while (Clipper.Step())
//           {
//               for (int ci = Clipper.DisplayStart; ci < Clipper.DisplayEnd; ++ci)
//               {
//                   const FLightEntry& En = Snap.Entries[GFilteredIdx[ci]];
//                   ImGui::TableNextRow();
//                   ImGui::TableSetColumnIndex(0);
//                   OL_ROW_SELECTABLE(GSel, En.Name, ci);
//                   ...
//               }
//           }
//           Clipper.End();
//           OL_ROW_MULTISELECT_END(GSel);
//
//           if (OL_ROW_MULTIMENU_BEGIN(GSel, "##bulkmenu"))
//           {
//               OL_ROW_MULTIMENU_ITEM_DANGER("spawns.action.delete", GSel, _r, Snap.Entries[GFilteredIdx[_r]].Name, GPendingDeleteLight, sizeof(GPendingDeleteLight), Action_DeleteLight);
//               OL_ROW_MENU_END;
//           }
//           ImGui::EndTable();
//       }
// ---------------------------------------------------------------------------
struct FOLRowMultiSelect
{
    ImGuiSelectionBasicStorage Storage;
};

// ---------------------------------------------------------------------------
// Bulk-delete driver behind OL_ROW_MULTIMENU_ITEM_DANGER — not used directly.
//
// Runs one Fn_() call per game tick (via OLImGui_EnqueueCall, re-queuing
// itself until the batch drains), each with its own copy of the key already
// written into DestBuf_ before that tick's Fn_ runs. This matters for two
// reasons: (1) Fn_ (e.g. Action_DeleteEnemy) is only ever called through
// OLImGui_EnqueueCall elsewhere in the codebase, since calling it directly
// from inside an ImGui::MenuItem callback would destroy the actor mid-frame,
// while ImGui/the renderer may still be touching it this same frame — that
// was the actual cause of the heap corruption crash multi-select delete hit
// on its first try, calling Fn_ synchronously in a loop; (2) deleting several
// actors within the same tick, one after another, while something else may
// still hold a raw pointer into the array being mutated (e.g. an in-progress
// FActorIterator elsewhere this tick) is avoided by spreading it over ticks.
// ---------------------------------------------------------------------------
struct FOLBulkDeleteState
{
    TArray<FString> Names;
    int    Next;
    char*  DestBuf;
    int    DestBufSz;
    OLImGuiCallFn Fn;
};
static FOLBulkDeleteState _OLBulkDelete;

static void _OLBulkDelete_Step()
{
    if (_OLBulkDelete.Next >= _OLBulkDelete.Names.Num()) return;
    _snprintf(_OLBulkDelete.DestBuf, _OLBulkDelete.DestBufSz, "%s", TCHAR_TO_ANSI(*_OLBulkDelete.Names(_OLBulkDelete.Next)));
    ++_OLBulkDelete.Next;
    _OLBulkDelete.Fn();
    if (_OLBulkDelete.Next < _OLBulkDelete.Names.Num())
        OLImGui_EnqueueCall(_OLBulkDelete_Step);
}

// Starts (or extends) a bulk-delete batch. Only one batch driver exists, so
// if a batch is already draining this queues behind it rather than
// interleaving — fine in practice since a user can't open two bulk-delete
// menus in the same frame. Names is filled by the OL_ROW_MULTIMENU_ITEM_DANGER
// macro (it alone knows how to resolve a selection index back to a name).
static void _OLBulkDelete_Start(const TArray<FString>& SrcNames, char* DestBuf, int DestBufSz, OLImGuiCallFn Fn)
{
    const bool bWasIdle = (_OLBulkDelete.Next >= _OLBulkDelete.Names.Num());
    _OLBulkDelete.Names     = SrcNames;
    _OLBulkDelete.Next      = 0;
    _OLBulkDelete.DestBuf   = DestBuf;
    _OLBulkDelete.DestBufSz = DestBufSz;
    _OLBulkDelete.Fn        = Fn;
    if (bWasIdle && _OLBulkDelete.Names.Num() > 0)
        OLImGui_EnqueueCall(_OLBulkDelete_Step);
}

#define OL_ROW_MULTISELECT_BEGIN(Sel_, ItemsCount_) \
    ImGuiMultiSelectIO* _ms_io = ImGui::BeginMultiSelect(ImGuiMultiSelectFlags_ClearOnEscape | ImGuiMultiSelectFlags_ClearOnClickVoid | ImGuiMultiSelectFlags_BoxSelect1d, (Sel_).Storage.Size, (ItemsCount_)); \
    (Sel_).Storage.ApplyRequests(_ms_io)

#define OL_ROW_MULTISELECT_INCLUDE_RANGE_SRC(Clipper_) \
    do { if (_ms_io->RangeSrcItem != -1) (Clipper_).IncludeItemByIndex((int)_ms_io->RangeSrcItem); } while(0)

#define OL_ROW_MULTISELECT_END(Sel_) \
    do { _ms_io = ImGui::EndMultiSelect(); (Sel_).Storage.ApplyRequests(_ms_io); } while(0)

#define OL_ROW_SELECTABLE(Sel_, DisplayLabel_, RowIdx_) \
    do { \
        const int _rowIdx = (RowIdx_); \
        const bool _rowSelected = (Sel_).Storage.Contains((ImGuiID)_rowIdx); \
        ImGui::SetNextItemSelectionUserData((ImGuiSelectionUserData)_rowIdx); \
        ImGui::Selectable((DisplayLabel_), _rowSelected, ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowOverlap); \
    } while(0)

#define OL_ROW_MULTIMENU_BEGIN(Sel_, PopupId_) \
    ((ImGui::IsMouseClicked(1) && ImGui::IsWindowHovered(ImGuiHoveredFlags_AllowWhenBlockedByPopup) && (Sel_).Storage.Size > 0) \
        ? (ImGui::OpenPopup(PopupId_), 0) : 0), ImGui::BeginPopup(PopupId_)

// KeyAtRowVar_/KeyAtRowExpr_: resolves a selected row index (bound as
// KeyAtRowVar_) to its stable `const char*` name, e.g.
// `Snap.Entries[GFilteredIdx[KeyAtRowVar_]].Name` — needed because the
// selection stores indices into the CURRENT filtered list, which won't
// exist anymore by the time the queued delete actually runs.
#define OL_ROW_MULTIMENU_ITEM_DANGER(LocKey_, Sel_, KeyAtRowVar_, KeyAtRowExpr_, DestBuf_, DestBufSz_, Fn_) \
    do { \
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.f, 0.35f, 0.35f, 1.f)); \
        if (ImGui::MenuItem(OLLocale_T(LocKey_))) { \
            TArray<FString> _bulkNames; \
            void* _it = NULL; \
            ImGuiID _id; \
            while ((Sel_).Storage.GetNextSelectedItem(&_it, &_id)) { \
                const int KeyAtRowVar_ = (int)_id; \
                _bulkNames.AddItem(FString((KeyAtRowExpr_))); \
            } \
            _OLBulkDelete_Start(_bulkNames, (DestBuf_), (DestBufSz_), (Fn_)); \
            (Sel_).Storage.Clear(); \
        } \
        ImGui::PopStyleColor(); \
    } while(0)

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
        if (ImGui::Checkbox(OL_L_ID_, BoolPtr_)) OLImGui_EnqueueCall(Fn_); \
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
     ImGui::SetNextItemWidth(-1.f), ImGui::ColorEdit3(OL_L_ID_, Ptr_))

#define OL_L_COLOR4(Label_, Ptr_) \
    (ImGui::TextUnformatted(Label_), ImGui::SameLine(), \
     ImGui::SetNextItemWidth(-1.f), ImGui::ColorEdit4(OL_L_ID_, Ptr_))

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
     ImGui::SliderFloat(OL_L_ID_, Ptr_, Min_, Max_))

#define OL_L_SLIDER_I_N(Label_, Ptr_, Min_, Max_, SliderW_) \
    (ImGui::TextUnformatted(Label_), ImGui::SameLine(), \
     ImGui::SetNextItemWidth(SliderW_), \
     ImGui::SliderInt(OL_L_ID_, Ptr_, Min_, Max_))

#define OL_T_L_SLIDER_F_N(LocKey_, Ptr_, Min_, Max_, SliderW_) \
    OL_T_SCALE_LABEL_BEGIN_(LocKey_) OL_L_SLIDER_F_N(OLLocale_T(LocKey_), Ptr_, Min_, Max_, SliderW_); OL_T_SCALE_END_

#define OL_T_L_SLIDER_I_N(LocKey_, Ptr_, Min_, Max_, SliderW_) \
    OL_T_SCALE_LABEL_BEGIN_(LocKey_) OL_L_SLIDER_I_N(OLLocale_T(LocKey_), Ptr_, Min_, Max_, SliderW_); OL_T_SCALE_END_
