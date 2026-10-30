/*=============================================================================
    OLImGui_Spawns_Fog.cpp — "Fog" sub-tab in the Spawns hub.

    Editor for the level's single ExponentialHeightFog actor. Unlike Lighting
    (many lights, placement ghost, list view), fog is a per-level singleton:
    this tab finds the existing fog actor (spawning one if the level has none)
    and edits its ExponentialHeightFogComponent parameters directly. No ghost
    placement, no list, no delete.

    ExponentialHeightFog (the actor) is a pure UnrealScript class — extends
    Info without a `native` modifier — so UnrealBuildTool never generates a
    C++ AExponentialHeightFog type for it. Only its component,
    UExponentialHeightFogComponent (native(FogVolume), declared in
    EngineFogVolumeClasses.h), has a native C++ mirror. So the actor itself is
    handled as a generic AActor* found/spawned by UClass name, and its fog
    component is located via AActor::AllComponents rather than a typed field
    access — the same pattern OLImGui_Spawns_Props.cpp uses for
    DynamicSMActor_Spawnable.
=============================================================================*/
#include "OLImGui_Tabs.h"
#include "EngineFogVolumeClasses.h"

// ---------------------------------------------------------------------------
// UI-side mirror of ExponentialHeightFogComponent parameters.
// ---------------------------------------------------------------------------

static bool  GFogEnabled                  = true;
static float GFogDensity                  = 0.02f;
static float GFogHeightFalloff            = 0.2f;
static float GFogMaxOpacity               = 1.f;
static float GFogStartDistance            = 0.f;
static float GFogLightTerminatorAngle     = 45.f;
static float GFogOppositeLightBrightness  = 0.2f;
static float GFogOppositeLightColor[3]    = { 177.f/255.f, 208.f/255.f, 255.f/255.f };
static float GFogLightInscatteringBrightness = 1.f;
static float GFogLightInscatteringColor[3]   = { 245.f/255.f, 212.f/255.f, 41.f/255.f };

// Set whenever the UI changes a value; consumed on the next tick to push
// params into the live component (mirrors GLightReattachPending in Lighting).
static bool GFogParamsDirty = false;

// Set once params have been read back from an existing/newly-spawned fog
// actor, so the UI doesn't overwrite it with defaults before that happens.
static bool GFogParamsLoaded = false;

static const FName GCustomFogTag(TEXT("CustomSpawnedFog"));

// ---------------------------------------------------------------------------
// Find the ExponentialHeightFogComponent on an actor via its component list
// (the actor's script-only class has no native C++ type to cast to).
// ---------------------------------------------------------------------------
static UExponentialHeightFogComponent* GetFogComponent(AActor* A)
{
    if (!A) return NULL;
    for (INT i = 0; i < A->AllComponents.Num(); ++i)
    {
        UExponentialHeightFogComponent* C = Cast<UExponentialHeightFogComponent>(A->AllComponents(i));
        if (C) return C;
    }
    return NULL;
}

// ---------------------------------------------------------------------------
// Find the level's fog actor, spawning one (tagged) if none exists yet.
// ---------------------------------------------------------------------------
static AActor* FindOrSpawnFog()
{
    if (!GWorld || !GWorld->PersistentLevel) return NULL;

    ULevel* PL = GWorld->PersistentLevel;
    for (INT i = 0; i < PL->Actors.Num(); ++i)
    {
        AActor* A = PL->Actors(i);
        if (!A || A->IsPendingKill()) continue;
        if (GetFogComponent(A)) return A;
    }

    UClass* FogClass = FindObject<UClass>(ANY_PACKAGE, TEXT("ExponentialHeightFog"));
    if (!FogClass) return NULL;

    AActor* NewFog = GWorld->SpawnActor(FogClass, NAME_None, FVector(0,0,0), FRotator(0,0,0),
        NULL, TRUE, FALSE, NULL, NULL, TRUE);
    if (!NewFog) return NULL;

    NewFog->bNoDelete = FALSE;
    NewFog->Tag = GCustomFogTag;
    return NewFog;
}

// ---------------------------------------------------------------------------
// Push UI params into the live fog component.
// ---------------------------------------------------------------------------
static void ApplyParamsToFog(AActor* Fog)
{
    UExponentialHeightFogComponent* C = GetFogComponent(Fog);
    if (!C) return;

    C->FogDensity                  = GFogDensity;
    C->FogHeightFalloff            = GFogHeightFalloff;
    C->FogMaxOpacity               = GFogMaxOpacity;
    C->StartDistance               = GFogStartDistance;
    C->LightTerminatorAngle        = GFogLightTerminatorAngle;
    C->OppositeLightBrightness     = GFogOppositeLightBrightness;
    C->OppositeLightColor          = FColor(
        (BYTE)(Clamp(GFogOppositeLightColor[0], 0.f, 1.f) * 255.f),
        (BYTE)(Clamp(GFogOppositeLightColor[1], 0.f, 1.f) * 255.f),
        (BYTE)(Clamp(GFogOppositeLightColor[2], 0.f, 1.f) * 255.f));
    C->LightInscatteringBrightness = GFogLightInscatteringBrightness;
    C->LightInscatteringColor      = FColor(
        (BYTE)(Clamp(GFogLightInscatteringColor[0], 0.f, 1.f) * 255.f),
        (BYTE)(Clamp(GFogLightInscatteringColor[1], 0.f, 1.f) * 255.f),
        (BYTE)(Clamp(GFogLightInscatteringColor[2], 0.f, 1.f) * 255.f));

    C->SetEnabled(GFogEnabled ? TRUE : FALSE);
    C->BeginDeferredReattach();
}

// ---------------------------------------------------------------------------
// Read the live fog component into the UI mirror (called once on first tick
// the tab is open, so the sliders start at the level's actual values).
// ---------------------------------------------------------------------------
static void LoadParamsFromFog(AActor* Fog)
{
    UExponentialHeightFogComponent* C = GetFogComponent(Fog);
    if (!C) return;

    GFogEnabled                  = C->bEnabled != 0;
    GFogDensity                  = C->FogDensity;
    GFogHeightFalloff            = C->FogHeightFalloff;
    GFogMaxOpacity                = C->FogMaxOpacity;
    GFogStartDistance             = C->StartDistance;
    GFogLightTerminatorAngle      = C->LightTerminatorAngle;
    GFogOppositeLightBrightness   = C->OppositeLightBrightness;
    GFogOppositeLightColor[0]     = C->OppositeLightColor.R / 255.f;
    GFogOppositeLightColor[1]     = C->OppositeLightColor.G / 255.f;
    GFogOppositeLightColor[2]     = C->OppositeLightColor.B / 255.f;
    GFogLightInscatteringBrightness = C->LightInscatteringBrightness;
    GFogLightInscatteringColor[0] = C->LightInscatteringColor.R / 255.f;
    GFogLightInscatteringColor[1] = C->LightInscatteringColor.G / 255.f;
    GFogLightInscatteringColor[2] = C->LightInscatteringColor.B / 255.f;

    GFogParamsLoaded = true;
}

// ---------------------------------------------------------------------------
// Game-thread tick: load params on first open, apply on change.
// ---------------------------------------------------------------------------
void OLSpawns_TickFog()
{
    if (!OLSpawns_IsFogTabActive()) return;

    AActor* Fog = FindOrSpawnFog();
    if (!Fog) return;

    if (!GFogParamsLoaded)
    {
        LoadParamsFromFog(Fog);
        return; // don't also apply this tick — UI just got its values from Fog
    }

    if (GFogParamsDirty)
    {
        ApplyParamsToFog(Fog);
        GFogParamsDirty = false;
    }
}

// Called when the Fog tab is closed (back to hub) — forces a fresh read next open,
// in case the level's fog actor changed underneath us (e.g. map reload).
static void ResetFogParamsLoaded()
{
    GFogParamsLoaded = false;
}

// ---------------------------------------------------------------------------
// Draw
// ---------------------------------------------------------------------------
static void DrawFog()
{
    if (ImGui::Button(OLLocale_T("common.back")))
    {
        ResetFogParamsLoaded();
        OLSpawns_BackToHub();
        return;
    }

    ImGui::Spacing();

    if (!GFogParamsLoaded)
    {
        ImGui::TextDisabled(OLLocale_T("common.loading"));
        return;
    }

    OL_T_SECTION("spawns.fog.section");

    { const bool Prev = GFogEnabled;
      OL_T_L_BOOL("spawns.fog.enabled", &GFogEnabled, [](){});
      if (GFogEnabled != Prev) GFogParamsDirty = true; }

    ImGui::Spacing();

    { const float Prev = GFogDensity;
      OL_T_L_DRAG_F("spawns.fog.density", &GFogDensity, 0.001f);
      if (GFogDensity < 0.f) GFogDensity = 0.f;
      if (GFogDensity != Prev) GFogParamsDirty = true; }

    { const float Prev = GFogHeightFalloff;
      OL_T_L_DRAG_F("spawns.fog.height_falloff", &GFogHeightFalloff, 0.001f);
      if (GFogHeightFalloff < 0.f) GFogHeightFalloff = 0.f;
      if (GFogHeightFalloff != Prev) GFogParamsDirty = true; }

    { const float Prev = GFogMaxOpacity;
      OL_T_L_SLIDER_F("spawns.fog.max_opacity", &GFogMaxOpacity, 0.f, 1.f);
      if (GFogMaxOpacity != Prev) GFogParamsDirty = true; }

    { const float Prev = GFogStartDistance;
      OL_T_L_DRAG_F("spawns.fog.start_distance", &GFogStartDistance, 4.f);
      if (GFogStartDistance < 0.f) GFogStartDistance = 0.f;
      if (GFogStartDistance != Prev) GFogParamsDirty = true; }

    ImGui::Spacing();
    OL_T_SECTION("spawns.fog.section.lighting");

    { const float Prev = GFogLightTerminatorAngle;
      OL_T_L_SLIDER_F("spawns.fog.terminator_angle", &GFogLightTerminatorAngle, 0.f, 180.f);
      if (GFogLightTerminatorAngle != Prev) GFogParamsDirty = true; }

    { const float Prev = GFogLightInscatteringBrightness;
      OL_T_L_DRAG_F("spawns.fog.inscattering_brightness", &GFogLightInscatteringBrightness, 0.01f);
      if (GFogLightInscatteringBrightness < 0.f) GFogLightInscatteringBrightness = 0.f;
      if (GFogLightInscatteringBrightness != Prev) GFogParamsDirty = true; }

    { float Prev[3] = { GFogLightInscatteringColor[0], GFogLightInscatteringColor[1], GFogLightInscatteringColor[2] };
      OL_T_L_COLOR3("spawns.fog.inscattering_color", GFogLightInscatteringColor);
      if (GFogLightInscatteringColor[0] != Prev[0] || GFogLightInscatteringColor[1] != Prev[1] || GFogLightInscatteringColor[2] != Prev[2])
          GFogParamsDirty = true; }

    { const float Prev = GFogOppositeLightBrightness;
      OL_T_L_DRAG_F("spawns.fog.opposite_brightness", &GFogOppositeLightBrightness, 0.01f);
      if (GFogOppositeLightBrightness < 0.f) GFogOppositeLightBrightness = 0.f;
      if (GFogOppositeLightBrightness != Prev) GFogParamsDirty = true; }

    { float Prev[3] = { GFogOppositeLightColor[0], GFogOppositeLightColor[1], GFogOppositeLightColor[2] };
      OL_T_L_COLOR3("spawns.fog.opposite_color", GFogOppositeLightColor);
      if (GFogOppositeLightColor[0] != Prev[0] || GFogOppositeLightColor[1] != Prev[1] || GFogOppositeLightColor[2] != Prev[2])
          GFogParamsDirty = true; }
}

// ---------------------------------------------------------------------------
// Tab entry point
// ---------------------------------------------------------------------------
void OLImGui_TabSpawns_Fog()
{
    DrawFog();
}
