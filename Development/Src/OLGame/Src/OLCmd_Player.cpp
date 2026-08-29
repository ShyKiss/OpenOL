/*=============================================================================
    OLCmd_Player.cpp — Player cheat commands + render-thread state snapshot.

    Functions that have complex UC-side logic (camera mode switches, pause,
    GhostPawn native call) are dispatched via ProcessEvent so the existing
    UnrealScript code runs unchanged. Simple flag flips are done directly.

    All functions run on the game thread (dispatched via OLImGui_EnqueueCall).
=============================================================================*/
#include "OLGame.h"
#include "OLUtilities.h"
#include "OLCmd_Player.h"

// ---------------------------------------------------------------------------
// State snapshot
// ---------------------------------------------------------------------------

volatile FOLPlayerState GOLPlayerState = {};

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------

// Returns the cheat manager cast to UOLCheatManager, or NULL.
static UOLCheatManager* GetCM()
{
    AOLPlayerController* PC = Utils::GetOLPC();
    if (!PC) return NULL;
    return Cast<UOLCheatManager>(PC->CheatManager);
}

// Call a zero-arg exec function on the cheat manager by name.
static void CallCMExec(const TCHAR* FnName)
{
    UOLCheatManager* CM = GetCM();
    if (!CM) return;
    UFunction* Fn = CM->FindFunction(FName(FnName));
    if (Fn) CM->ProcessEvent(Fn, NULL);
}

// ---------------------------------------------------------------------------
// State snapshot
// ---------------------------------------------------------------------------

void OLCmd_Player_UpdateSnapshot()
{
    AOLPlayerController* PC = Utils::GetOLPC();
    if (!PC) return;

    GOLPlayerState.bGodMode  = !!PC->bGodMode;
    GOLPlayerState.bFreeCam  = !!PC->bDebugFreeCam;
    GOLPlayerState.bFixedCam = !!PC->bDebugFixedCam;
    GOLPlayerState.bBunnyHop = !!PC->bBunnyHop;
    GOLPlayerState.bLimping               = PC->HeroPawn ? !!PC->HeroPawn->bLimping              : false;
    GOLPlayerState.bHobbling              = PC->HeroPawn ? !!PC->HeroPawn->bHobbling             : false;
    GOLPlayerState.HobblingIntensity      = PC->HeroPawn ? PC->HeroPawn->HobblingIntensity       : 0.f;
    GOLPlayerState.TargetHobblingIntensity= PC->HeroPawn ? PC->HeroPawn->TargetHobblingIntensity : 0.f;
    UOLCheatManager* CM = GetCM();
    GOLPlayerState.bGhost    = !!PC->bDebugGhost;
    GOLPlayerState.bFreeCamInspector        = CM ? !!CM->bFreeCamInspector        : false;
    GOLPlayerState.bUnlimitedBatteries      = CM ? !!CM->bUnlimitedBatteries      : false;
    GOLPlayerState.MaxHealth                = PC->HeroPawn ? PC->HeroPawn->HealthMax : 100;
    GOLPlayerState.bCheckpointListAvailable = AOLCheckpointList::GetCheckpointList() != NULL;
    UOLEngine* Eng = Cast<UOLEngine>(GEngine);
    GOLPlayerState.bSmoothCamera = Eng ? !!Eng->bSmoothCamera : false;
    GOLPlayerState.FreeCamFOV        = PC->DebugFreeCamFOV;
    GOLPlayerState.FreeCamSpeed      = PC->DebugFreeCamSpeed;
    GOLPlayerState.DefaultFOV        = PC->HeroPawn ? PC->HeroPawn->DefaultFOV       : 90.f;
    GOLPlayerState.RunningFOV        = PC->HeroPawn ? PC->HeroPawn->RunningFOV       : 90.f;
    GOLPlayerState.CamcorderMinFOV   = PC->HeroPawn ? PC->HeroPawn->CamcorderMinFOV  : 55.f;
    GOLPlayerState.CamcorderMaxFOV   = PC->HeroPawn ? PC->HeroPawn->CamcorderMaxFOV  : 75.f;
    GOLPlayerState.CamcorderNVMaxFOV = PC->HeroPawn ? PC->HeroPawn->CamcorderNVMaxFOV: 55.f;
}

// ---------------------------------------------------------------------------
// God Mode — toggle bGodMode on APlayerController (Engine base class).
// ---------------------------------------------------------------------------

void OLCmd_ToggleGodMode()
{
    AOLPlayerController* PC = Utils::GetOLPC();
    if (!PC) return;
    PC->bGodMode = !PC->bGodMode;
}

// ---------------------------------------------------------------------------
// Freecam — delegate to UC (handles pause, camera mode switch, ghost reset).
// ---------------------------------------------------------------------------

void OLCmd_ToggleFreecam()
{
    // ToggleFreeCamNoPause keeps the game running — better for debug use.
    CallCMExec(TEXT("ToggleFreeCamNoPause"));
}

// ---------------------------------------------------------------------------
// Fixed Cam — delegate to UC.
// ---------------------------------------------------------------------------

void OLCmd_ToggleFixedCam()
{
    CallCMExec(TEXT("ToggleFixedCam"));
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------
// Ghost (fully ghost / bDebugFullyGhost) — UC-only flag in Picker; we don't
// have it natively, so expose the same Ghost() path for now.
// TODO: add bDebugFullyGhost native field if needed separately.
// ---------------------------------------------------------------------------

void OLCmd_ToggleGhost()
{
    CallCMExec(TEXT("Ghost"));
}

// ---------------------------------------------------------------------------
// Bunny Hop — native bBunnyHop on AOLPlayerController; skips eventLanded.
// ---------------------------------------------------------------------------

void OLCmd_ToggleBunnyHop()
{
    AOLPlayerController* PC = Utils::GetOLPC();
    if (!PC) return;
    PC->bBunnyHop = !PC->bBunnyHop;
}

// ---------------------------------------------------------------------------
// Limp — toggle bLimping on AOLHero.
// ---------------------------------------------------------------------------

void OLCmd_ToggleLimp()
{
    AOLPlayerController* PC = Utils::GetOLPC();
    if (!PC || !PC->HeroPawn) return;
    PC->HeroPawn->bLimping = !PC->HeroPawn->bLimping;
}

// ---------------------------------------------------------------------------
// Hobble — toggle bHobbling on AOLHero.
// ---------------------------------------------------------------------------

void OLCmd_ToggleHobble()
{
    AOLPlayerController* PC = Utils::GetOLPC();
    if (!PC || !PC->HeroPawn) return;
    PC->HeroPawn->bHobbling = !PC->HeroPawn->bHobbling;
    // Set a default intensity when enabling so the effect is visible.
    if (PC->HeroPawn->bHobbling && PC->HeroPawn->HobblingIntensity == 0.f)
        PC->HeroPawn->HobblingIntensity = 0.5f;
}

void OLCmd_SetHobblingIntensity(float Current, float Target)
{
    AOLPlayerController* PC = Utils::GetOLPC();
    if (!PC || !PC->HeroPawn) return;
    PC->HeroPawn->HobblingIntensity       = Current;
    PC->HeroPawn->TargetHobblingIntensity = Target;
}

void OLCmd_ToggleSmoothCamera()
{
    UOLEngine* Eng = Cast<UOLEngine>(GEngine);
    if (Eng) Eng->SetSmoothCamera(!Eng->bSmoothCamera);
}

void OLCmd_SetFreeCamFOV(float FOV)
{
    AOLPlayerController* PC = Utils::GetOLPC();
    if (PC) PC->DebugFreeCamFOV = FOV;
}

void OLCmd_SetFreeCamSpeed(float Speed)
{
    AOLPlayerController* PC = Utils::GetOLPC();
    if (PC) PC->DebugFreeCamSpeed = Speed;
}

void OLCmd_SetDefaultFOV(float FOV)
{
    AOLPlayerController* PC = Utils::GetOLPC();
    if (PC && PC->HeroPawn) PC->HeroPawn->DefaultFOV = FOV;
}

void OLCmd_SetRunningFOV(float FOV)
{
    AOLPlayerController* PC = Utils::GetOLPC();
    if (PC && PC->HeroPawn) PC->HeroPawn->RunningFOV = FOV;
}

void OLCmd_SetCamcorderMinFOV(float FOV)
{
    AOLPlayerController* PC = Utils::GetOLPC();
    if (PC && PC->HeroPawn) PC->HeroPawn->CamcorderMinFOV = FOV;
}

void OLCmd_SetCamcorderMaxFOV(float FOV)
{
    AOLPlayerController* PC = Utils::GetOLPC();
    if (PC && PC->HeroPawn) PC->HeroPawn->CamcorderMaxFOV = FOV;
}

void OLCmd_SetCamcorderNVMaxFOV(float FOV)
{
    AOLPlayerController* PC = Utils::GetOLPC();
    if (PC && PC->HeroPawn) PC->HeroPawn->CamcorderNVMaxFOV = FOV;
}

void OLCmd_ToggleFreeCamInspector()
{
    CallCMExec(TEXT("ToggleFreeCamInspector"));
}

// ---------------------------------------------------------------------------
// Unlimited Batteries — toggle bUnlimitedBatteries on UOLCheatManager.
// ---------------------------------------------------------------------------

void OLCmd_ToggleUnlimitedBatteries()
{
    CallCMExec(TEXT("ToggleUnlimitedBatteries"));
}

// ---------------------------------------------------------------------------
// Kill Player — deal enough damage to kill the hero instantly.
// ---------------------------------------------------------------------------

void OLCmd_KillPlayer()
{
    AOLPlayerController* PC = Utils::GetOLPC();
    if (!PC || !PC->HeroPawn) return;
    PC->HeroPawn->NativeTakeDamage(PC->HeroPawn->Health + 1, NULL,
        PC->HeroPawn->Location, NULL);
}

// ---------------------------------------------------------------------------
// SetMaxHealth — change HealthMax (and clamp current Health).
// ---------------------------------------------------------------------------

void OLCmd_SetMaxHealth(int Value)
{
    AOLPlayerController* PC = Utils::GetOLPC();
    if (!PC || !PC->HeroPawn || Value <= 0) return;
    PC->HeroPawn->HealthMax = Value;
    if (PC->HeroPawn->Health > Value)
        PC->HeroPawn->Health = Value;
}
