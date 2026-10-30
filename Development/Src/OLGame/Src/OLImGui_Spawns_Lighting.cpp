/*=============================================================================
    OLImGui_Spawns_Lighting.cpp — "Lighting" sub-tab in the Spawns hub.

    Allows placing movable lights (Point / Spot / Directional) in the world.
    Ghost is a real ALight actor whose LightComponent is updated live.
    Mouse wheel adjusts radius; Shift+wheel = yaw; Shift+Alt+wheel = pitch.
    LMB confirms (keeps light, tag=CustomSpawnedLight). RMB cancels (destroys).
=============================================================================*/
#include "OLImGui_Tabs.h"

// ---------------------------------------------------------------------------
// Light placement settings
// ---------------------------------------------------------------------------

enum ELightPlacementType { LPT_Point = 0, LPT_Spot, LPT_Directional, LPT_Sky, LPT_COUNT };
static const char* GLightTypeNames[] = { "Point", "Spot", "Directional", "Sky" };

static int   GLightType       = LPT_Point;
static float GLightBrightness = 1.5f;
static float GLightRadius     = 512.f;
static float GLightColor[3]   = { 1.f, 1.f, 1.f };
static float GLightInnerAngle = 20.f;
static float GLightOuterAngle = 45.f;
static bool  GLightDynShadows    = false;
static bool  GLightFollowCamera  = false;

// Set when any structural param (radius, cone, shadows) changes and a full reattach is needed.
static bool  GLightReattachPending = false;

static const FName GCustomLightTag(TEXT("CustomSpawnedLight"));

static void Action_BeginLightPlacement(); // forward declaration

// ---------------------------------------------------------------------------
// Apply current UI params to a spawned ALight.
// ---------------------------------------------------------------------------
static void ApplyParamsToLight(ALight* L)
{
    if (!L) return;
    ULightComponent* LC = L->LightComponent;
    if (!LC) return;

    LC->bForceDynamicLight = TRUE;  // prevent zone/portal culling
    // Never auto-disable this light for being far from the camera — the engine
    // default (2500 units) is well within normal level scale and would silently
    // turn off lights placed for distant/large-area use.
    LC->MaxDistFromCamForEnabled = 9999999.f;
    LC->Brightness         = GLightBrightness;
    LC->LightColor         = FColor(
        (BYTE)(Clamp(GLightColor[0], 0.f, 1.f) * 255.f),
        (BYTE)(Clamp(GLightColor[1], 0.f, 1.f) * 255.f),
        (BYTE)(Clamp(GLightColor[2], 0.f, 1.f) * 255.f));
    LC->CastDynamicShadows = GLightDynShadows ? 1 : 0;

    UPointLightComponent* PLC = Cast<UPointLightComponent>(LC);
    if (PLC) PLC->Radius = GLightRadius;

    USpotLightComponent* SLC = Cast<USpotLightComponent>(LC);
    if (SLC)
    {
        SLC->InnerConeAngle = GLightInnerAngle;
        SLC->OuterConeAngle = GLightOuterAngle;
    }

    if (GLightReattachPending)
    {
        LC->BeginDeferredReattach();
        GLightReattachPending = false;
    }
}

// ---------------------------------------------------------------------------
// Game-thread tick: sync PlacementRadius -> GLightRadius, apply params.
// ---------------------------------------------------------------------------
void OLSpawns_TickLightPlacement()
{
    AOLPlayerController* PC = Utils::GetOLPC();
    if (!PC || !PC->bPlacementMode) return;
    ALight* Ghost = Cast<ALight>(PC->PlacementGhost);
    if (!Ghost) return;

    // Sync radius: Alt+wheel writes PlacementRadius; mirror back into UI.
    if (PC->PlacementRadius > 0.f && PC->PlacementRadius != GLightRadius)
    {
        GLightRadius = PC->PlacementRadius;
        GLightReattachPending = true;
    }

    // Follow camera: snap light to camera position and orientation.
    if (GLightFollowCamera)
    {
        FVector  CamLoc;
        FRotator CamRot;
        PC->eventGetPlayerViewPoint(CamLoc, CamRot);
        Ghost->SetLocation(CamLoc);
        Ghost->SetRotation(CamRot);
    }

    ApplyParamsToLight(Ghost);
}

// ---------------------------------------------------------------------------
// Game-thread tick: consume LMB confirm for light ghosts.
// Detaches ghost (keeps it in world with tag), starts a new one.
// ---------------------------------------------------------------------------
static bool ConfirmLightPlacement(AOLPlayerController* PC)
{
    ALight* Ghost = Cast<ALight>(PC->PlacementGhost);
    if (!Ghost) return false;

    PC->bPlacementConfirmPending = FALSE;
    Ghost->Tag = GCustomLightTag;
    PC->eventDetachPlacementMode();
    Action_BeginLightPlacement();
    return true;
}

void OLSpawns_TickLightConfirm()
{
    OLSpawns_RegisterConfirmHandler(ConfirmLightPlacement);
}

// ---------------------------------------------------------------------------
// Game-thread: spawn ghost light.
// ---------------------------------------------------------------------------
static void Action_BeginLightPlacement()
{
    AOLPlayerController* PC = Utils::GetOLPC();
    if (!PC) return;

    const char* ClassName = "PointLightMovable";
    if (GLightType == LPT_Spot)
        ClassName = "SpotLightMovable";
    else if (GLightType == LPT_Directional)
        ClassName = "DirectionalLightToggleable";
    else if (GLightType == LPT_Sky)
        ClassName = "SkyLightToggleable";

    UClass* LightClass = FindObject<UClass>(ANY_PACKAGE, ANSI_TO_TCHAR(ClassName));
    if (!LightClass) return;

    FVector SpawnLoc; FRotator SpawnRot;
    PC->eventGetPlayerViewPoint(SpawnLoc, SpawnRot);
    SpawnLoc += FRotationMatrix(SpawnRot).GetAxis(0) * 256.f;

    AActor* Ghost = GWorld->SpawnActor(LightClass, NAME_None, SpawnLoc, SpawnRot,
        NULL, TRUE, FALSE, NULL, NULL, TRUE);
    if (!Ghost) return;

    Ghost->bNoDelete = FALSE;

    ALight* LightGhost = Cast<ALight>(Ghost);
    if (LightGhost && LightGhost->LightComponent)
        LightGhost->LightComponent->LightingChannels.SetAllChannels();
    GLightReattachPending = true; // force full reattach on first apply
    ApplyParamsToLight(LightGhost);

    // Only Point/Spot use radius wheel; Directional/Sky use yaw/pitch only.
    PC->PlacementRadius = (GLightType == LPT_Point || GLightType == LPT_Spot) ? GLightRadius : 0.f;
    PC->eventBeginPlacementMode(Ghost);
}

// ---------------------------------------------------------------------------
// Game-thread: cancel active light ghost on type change.
// ---------------------------------------------------------------------------
static void Action_CancelAndRestartLightPlacement()
{
    AOLPlayerController* PC = Utils::GetOLPC();
    if (!PC) return;
    if (PC->bPlacementMode && Cast<ALight>(PC->PlacementGhost))
        PC->eventEndPlacementMode();
    Action_BeginLightPlacement();
}

// ---------------------------------------------------------------------------
// Light list snapshot — double-buffer, game thread writes, render reads.
// ---------------------------------------------------------------------------

struct FLightEntry
{
    char  Name[64];
    char  TypeStr[16]; // "Point", "Spot", "Dir"
    float LocX, LocY, LocZ;
};

static const int MAX_LIGHT_SNAP = 256;
struct FLightListSnap { FLightEntry Entries[MAX_LIGHT_SNAP]; int Count; };

static FLightListSnap      GLightBuf[2];
static volatile int        GLightReadIdx  = -1;
static int                 GLightWriteIdx = 0;

static void RebuildLightList()
{
    if (!GWorld || !GWorld->PersistentLevel) return;
    ULevel* PL = GWorld->PersistentLevel;

    FLightListSnap Tmp;
    Tmp.Count = 0;

    for (INT ai = 0; ai < PL->Actors.Num(); ++ai)
    {
        AActor* A = PL->Actors(ai);
        if (!A || A->IsPendingKill() || A->Tag != GCustomLightTag) continue;
        ALight* L = Cast<ALight>(A);
        if (!L) continue;
        if (Tmp.Count >= MAX_LIGHT_SNAP) break;

        FLightEntry& En = Tmp.Entries[Tmp.Count++];
        En.LocX = L->Location.X;
        En.LocY = L->Location.Y;
        En.LocZ = L->Location.Z;

        const FString FName = L->GetFName().ToString();
        const TCHAR* N = *FName;
        int ni = 0;
        while (ni < 63 && N[ni]) { En.Name[ni] = (char)N[ni]; ++ni; }
        En.Name[ni] = '\0';

        const char* TypeName = Cast<ASpotLight>(L) ? "Spot"
                             : Cast<APointLight>(L) ? "Point"
                             : Cast<ASkyLight>(L)   ? "Sky"
                             : "Dir";
        int ti = 0;
        while (ti < 15 && TypeName[ti]) { En.TypeStr[ti] = TypeName[ti]; ++ti; }
        En.TypeStr[ti] = '\0';
    }

    GLightBuf[GLightWriteIdx] = Tmp;
    GLightReadIdx  = GLightWriteIdx;
    GLightWriteIdx = 1 - GLightWriteIdx;
}

// ---------------------------------------------------------------------------
// Game-thread: delete light by name.
// ---------------------------------------------------------------------------
static char GPendingDeleteLight[64] = {};

static void Action_DeleteLight()
{
    if (!GWorld || !GWorld->PersistentLevel) return;
    FName Target(ANSI_TO_TCHAR(GPendingDeleteLight));
    ULevel* PL = GWorld->PersistentLevel;
    for (INT i = 0; i < PL->Actors.Num(); ++i)
    {
        AActor* A = PL->Actors(i);
        if (!A || A->IsPendingKill()) continue;
        ALight* L = Cast<ALight>(A);
        if (L && L->Tag == GCustomLightTag && L->GetFName() == Target)
        {
            L->bNoDelete = FALSE;
            GWorld->DestroyActor(L);
            return;
        }
    }
}

// ---------------------------------------------------------------------------
// Views
// ---------------------------------------------------------------------------

enum ELightView { LV_List, LV_New };
static ELightView GLightView = LV_List;

// ---------------------------------------------------------------------------
// "New light" panel
// ---------------------------------------------------------------------------
static void DrawLightNew()
{
    if (ImGui::Button(OLLocale_T("common.back")))
    {
        GLightView = LV_List;
        OLImGui_EnqueueCall([]() {
            AOLPlayerController* PC = Utils::GetOLPC();
            if (PC && PC->bPlacementMode && Cast<ALight>(PC->PlacementGhost))
                PC->eventEndPlacementMode();
        });
        return;
    }

    OL_T_SECTION("spawns.lighting.section");

    // Type combo — no OL_T macro for combos, use raw with locale key
    ImGui::TextUnformatted(OLLocale_T("spawns.lighting.type")); ImGui::SameLine();
    ImGui::SetNextItemWidth(-1.f);
    const int PrevType = GLightType;
    ImGui::Combo("##ltype", &GLightType, GLightTypeNames, LPT_COUNT);
    if (GLightType != PrevType)
        OLImGui_EnqueueCall(Action_CancelAndRestartLightPlacement);

    ImGui::Spacing();

    { const float PrevBri = GLightBrightness;
      OL_T_L_DRAG_F("spawns.lighting.brightness", &GLightBrightness, 0.05f);
      if (GLightBrightness < 0.f) GLightBrightness = 0.f;
      if (GLightBrightness != PrevBri) GLightReattachPending = true; }

    if (GLightType == LPT_Point || GLightType == LPT_Spot)
    {
        const float PrevRadius = GLightRadius;
        OL_T_L_DRAG_F("spawns.lighting.radius", &GLightRadius, 4.f);
        if (GLightRadius < 16.f) GLightRadius = 16.f;
        if (GLightRadius != PrevRadius) GLightReattachPending = true;
    }

    { float PrevCol[3] = { GLightColor[0], GLightColor[1], GLightColor[2] };
      OL_T_L_COLOR3("spawns.lighting.color", GLightColor);
      if (GLightColor[0] != PrevCol[0] || GLightColor[1] != PrevCol[1] || GLightColor[2] != PrevCol[2])
          GLightReattachPending = true; }

    if (GLightType == LPT_Spot)
    {
        ImGui::Spacing();
        OL_T_SECTION("spawns.lighting.section.cone");
        const char* sInner = OLLocale_T("spawns.lighting.inner");
        const char* sOuter = OLLocale_T("spawns.lighting.outer");
        float W = OL_ROW_SLIDER_WIDTH_2(sInner, sOuter);
        const float PrevInner = GLightInnerAngle, PrevOuter = GLightOuterAngle;
        OL_T_L_SLIDER_F_N("spawns.lighting.inner", &GLightInnerAngle, 0.f, 80.f, W);
        ImGui::SameLine();
        OL_T_L_SLIDER_F_N("spawns.lighting.outer", &GLightOuterAngle, 1.f, 89.f, W);
        if (GLightInnerAngle >= GLightOuterAngle)
            GLightInnerAngle = GLightOuterAngle - 1.f;
        if (GLightInnerAngle != PrevInner || GLightOuterAngle != PrevOuter)
            GLightReattachPending = true;
    }

    ImGui::Spacing();
    const bool PrevShadows = GLightDynShadows;
    OL_T_L_BOOL("spawns.lighting.dyn_shadows", &GLightDynShadows, [](){});
    if (GLightDynShadows != PrevShadows) GLightReattachPending = true;

    OL_T_L_BOOL("spawns.lighting.follow_camera", &GLightFollowCamera, [](){});

    ImGui::Spacing();
    if (!GLightFollowCamera)
    {
        ImGui::TextDisabled(GLightType == LPT_Point || GLightType == LPT_Spot
            ? OLLocale_T("spawns.lighting.hint.radius")
            : OLLocale_T("spawns.lighting.hint.no_radius"));
    }
    ImGui::TextDisabled(OLLocale_T("spawns.lighting.hint.place"));

    ImGui::Spacing();

    if (ImGui::Button(OLLocale_T("spawns.lighting.spawn_btn"), ImVec2(-1.f, 0.f)))
        OLImGui_EnqueueCall(Action_BeginLightPlacement);
}

// ---------------------------------------------------------------------------
// Light list view
// ---------------------------------------------------------------------------
static char GLightFilter[128] = {};
static FOLRowMultiSelect GLightSel;

static void DrawLightList()
{
    if (ImGui::Button(OLLocale_T("common.back")))
    {
        OLSpawns_BackToHub();
        return;
    }
    ImGui::SameLine();
    OL_SEARCH("##lfilter", "spawns.lighting.search", GLightFilter, sizeof(GLightFilter));

    ImGui::Spacing();

    const int ri = GLightReadIdx;
    if (ri < 0)
    {
        ImGui::TextDisabled(OLLocale_T("common.loading"));
    }
    else
    {
        const FLightListSnap& Snap = GLightBuf[ri];

        static int GFilteredLight[MAX_LIGHT_SNAP];
        int FilteredCount = 0;
        for (int i = 0; i < Snap.Count; ++i)
            if (OLImGui_MatchFilter(Snap.Entries[i].Name, GLightFilter))
                GFilteredLight[FilteredCount++] = i;

        const float PlusH = ImGui::GetFrameHeightWithSpacing() + ImGui::GetStyle().ItemSpacing.y;
        ImGui::BeginChild("##lightlist", ImVec2(0, -PlusH), false);

        if (ImGui::BeginTable("##ltbl", 3,
            ImGuiTableFlags_BordersInnerV | ImGuiTableFlags_SizingStretchProp |
            ImGuiTableFlags_RowBg | ImGuiTableFlags_ScrollY))
        {
            ImGui::TableSetupColumn(OLLocale_T("spawns.lighting.col.name"),    ImGuiTableColumnFlags_WidthStretch, 3.f);
            ImGui::TableSetupColumn(OLLocale_T("spawns.lighting.col.type"),    ImGuiTableColumnFlags_WidthStretch, 1.f);
            ImGui::TableSetupColumn(OLLocale_T("spawns.lighting.col.actions"), ImGuiTableColumnFlags_WidthFixed,  50.f);
            ImGui::TableSetupScrollFreeze(0, 1);
            ImGui::TableHeadersRow();

            OL_ROW_MULTISELECT_BEGIN(GLightSel, FilteredCount);

            ImGuiListClipper Clipper;
            Clipper.Begin(FilteredCount);
            OL_ROW_MULTISELECT_INCLUDE_RANGE_SRC(Clipper);
            while (Clipper.Step())
            {
                for (int ci = Clipper.DisplayStart; ci < Clipper.DisplayEnd; ++ci)
                {
                    const int i = GFilteredLight[ci];
                    const FLightEntry& En = Snap.Entries[i];
                    ImGui::TableNextRow();

                    ImGui::TableSetColumnIndex(0);
                    ImGui::PushID(i);
                    OL_ROW_SELECTABLE(GLightSel, En.Name, ci);
                    ImGui::PopID();

                    ImGui::TableSetColumnIndex(1);
                    ImGui::TextUnformatted(En.TypeStr);

                    ImGui::TableSetColumnIndex(2);
                    ImGui::PushID(i + 10000);
                    char PopupId[32];
                    _snprintf(PopupId, sizeof(PopupId), "##lmenu%d", i);
                    if (OL_ROW_MENU_BEGIN(PopupId))
                    {
                        OL_ROW_MENU_ITEM_DANGER("spawns.action.delete", GPendingDeleteLight, sizeof(GPendingDeleteLight), En.Name, Action_DeleteLight);
                        OL_ROW_MENU_END;
                    }
                    ImGui::PopID();
                }
            }
            Clipper.End();
            OL_ROW_MULTISELECT_END(GLightSel);

            if (OL_ROW_MULTIMENU_BEGIN(GLightSel, "##lbulkmenu"))
            {
                OL_ROW_MULTIMENU_ITEM_DANGER("spawns.action.delete", GLightSel, _r, Snap.Entries[GFilteredLight[_r]].Name, GPendingDeleteLight, sizeof(GPendingDeleteLight), Action_DeleteLight);
                OL_ROW_MENU_END;
            }
            ImGui::EndTable();
        }

        ImGui::EndChild();
    }

    if (ImGui::Button("+", ImVec2(-1.f, 0.f)))
        GLightView = LV_New;
}

// ---------------------------------------------------------------------------
// Game-thread tick: rebuild list snapshot.
// ---------------------------------------------------------------------------
void OLSpawns_TickLightList()
{
    RebuildLightList();
}

// ---------------------------------------------------------------------------
// Tab entry point
// ---------------------------------------------------------------------------
void OLImGui_TabSpawns_Lighting()
{
    switch (GLightView)
    {
    case LV_New:  DrawLightNew();  break;
    default:      DrawLightList(); break;
    }
}
