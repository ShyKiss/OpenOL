/*=============================================================================
    OLImGui_LoadingOverlay.cpp -- global "loading in progress" debug overlay.
=============================================================================*/
#include "OLImGui_Tabs.h"
#include "OLImGui_LoadingOverlay.h"

#ifndef UCONST_LOADING_MOVIE
#define UCONST_LOADING_MOVIE TEXT("LoadingScreen")
#endif

// ---------------------------------------------------------------------------
// Current stage — a short, human-readable line naming what's happening right
// now (package being loaded, checkpoint being applied, player spawning...).
// Written from a few call sites below and from OLPlayerController.cpp at the
// points that actually know what's going on; read every tick into the
// render-thread snapshot. Game thread only, so a plain buffer is fine.
// ---------------------------------------------------------------------------
static char GLoadingStage[128] = {};

void OLLoadingOverlay_SetStage(const TCHAR* Stage)
{
    if (!Stage || !*Stage)
    {
        GLoadingStage[0] = '\0';
        return;
    }
    appStrncpyANSI(GLoadingStage, TCHAR_TO_ANSI(Stage), ARRAY_COUNT(GLoadingStage));
}

// ---------------------------------------------------------------------------
// PreLoadMap / PostLoadMap observer — the only two points that reliably name
// the package being loaded for a *plain* level load. Seamless travel and
// checkpoint travel set more specific stage text themselves (see
// StartTravelToCheckpoint/TravelComplete/OnLevelBecomingVisible in
// OLPlayerController.cpp), which this simply won't override while they're
// in charge (they call OLLoadingOverlay_SetStage right before/after too).
// ---------------------------------------------------------------------------
class FOLLoadingStageObserver : public FCallbackEventObserver
{
public:
    virtual void Send(ECallbackEventType InType)
    {
        if (InType == CALLBACK_PreLoadMap)
        {
            FString MapName = GEngine ? GEngine->TransitionDescription : FString();
            if (MapName.Len() == 0) MapName = TEXT("level");
            _snprintf(GLoadingStage, ARRAY_COUNT(GLoadingStage), "Loading package: %s", TCHAR_TO_ANSI(*MapName));
        }
        else if (InType == CALLBACK_PostLoadMap)
        {
            GLoadingStage[0] = '\0';
        }
    }
};
static FOLLoadingStageObserver GLoadingStageObserver;
static bool GLoadingStageObserverRegistered = false;

// ---------------------------------------------------------------------------
// Snapshot — written on game thread, read on render thread.
// ---------------------------------------------------------------------------
struct FLoadingOverlaySnap
{
    bool  bLoading;
    char  Stage[128];
    float ElapsedSeconds;
};
static volatile FLoadingOverlaySnap GLoadingSnap = {};

void OLLoadingOverlay_UpdateSnapshot()
{
    if (!GLoadingStageObserverRegistered)
    {
        GLoadingStageObserverRegistered = true;
        GCallbackEvent->Register(CALLBACK_PreLoadMap,  &GLoadingStageObserver);
        GCallbackEvent->Register(CALLBACK_PostLoadMap, &GLoadingStageObserver);
    }

    AOLPlayerController* PC = Utils::GetOLPC();

    const UBOOL bTravelling = PC && PC->bTravellingToCheckpoint;
    const UBOOL bMoviePlaying = GFullScreenMovie && GFullScreenMovie->GameThreadIsMoviePlaying(UCONST_LOADING_MOVIE);
    const bool  bLoading = !!bTravelling || !!bMoviePlaying;

    static DOUBLE LoadStartTime = 0.0;
    if (bLoading && LoadStartTime == 0.0)
        LoadStartTime = appSeconds();
    else if (!bLoading)
        LoadStartTime = 0.0;

    GLoadingSnap.bLoading       = bLoading;
    GLoadingSnap.ElapsedSeconds = bLoading ? (float)(appSeconds() - LoadStartTime) : 0.f;
    appMemcpy((void*)GLoadingSnap.Stage, GLoadingStage, sizeof(GLoadingStage));
}

void OLLoadingOverlay_Draw()
{
    const FLoadingOverlaySnap& S = *(const FLoadingOverlaySnap*)&GLoadingSnap;
    if (!S.bLoading) return;

    const char* Stage = S.Stage[0] ? S.Stage : OLLocale_T("loading.stage.level");

    // Bottom-center, close to the screen edge — the loading movie behind it
    // typically fills the whole screen, so staying low keeps this out of its way.
    ImGuiIO& IO = ImGui::GetIO();
    ImGui::SetNextWindowPos(ImVec2(IO.DisplaySize.x * 0.5f, IO.DisplaySize.y - 24.f), ImGuiCond_Always, ImVec2(0.5f, 1.f));
    ImGui::SetNextWindowBgAlpha(0.55f);
    ImGui::Begin("##loading_overlay", NULL,
        ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_NoNav |
        ImGuiWindowFlags_NoMove | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoFocusOnAppearing);

    ImGui::Text("%s  (%.1fs)", Stage, S.ElapsedSeconds);

    // Indeterminate bar: no real load percentage is available for the general
    // case (only per-package via GetAsyncLoadPercentage, which needs a specific
    // package name we don't always know here), so animate a moving segment.
    ImGui::PushStyleColor(ImGuiCol_PlotHistogram, ImVec4(0.35f, 0.65f, 1.f, 1.f));
    ImGui::ProgressBar(-1.0f * (float)ImGui::GetTime(), ImVec2(220.f, 0.f), "");
    ImGui::PopStyleColor();

    ImGui::End();
}
