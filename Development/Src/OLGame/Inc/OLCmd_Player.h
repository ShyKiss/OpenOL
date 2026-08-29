#pragma once
// Player cheat commands — implemented in OLCmd_Player.cpp.
// All functions run on the game thread.

void OLCmd_ToggleGodMode();
void OLCmd_ToggleFreecam();
void OLCmd_ToggleFixedCam();
void OLCmd_ToggleGhost();
void OLCmd_ToggleBunnyHop();
void OLCmd_ToggleLimp();
void OLCmd_ToggleHobble();
void OLCmd_SetHobblingIntensity(float Current, float Target);
void OLCmd_ToggleSmoothCamera();
void OLCmd_SetFreeCamFOV(float FOV);
void OLCmd_SetFreeCamSpeed(float Speed);
void OLCmd_SetDefaultFOV(float FOV);
void OLCmd_SetRunningFOV(float FOV);
void OLCmd_SetCamcorderMinFOV(float FOV);
void OLCmd_SetCamcorderMaxFOV(float FOV);
void OLCmd_SetCamcorderNVMaxFOV(float FOV);
void OLCmd_ToggleFreeCamInspector();
void OLCmd_ToggleUnlimitedBatteries();
void OLCmd_KillPlayer();
void OLCmd_SetMaxHealth(int Value);

// State snapshot — written on game thread each tick, read on render thread.
// Use these in OL_TOGGLE(Label_, OLPlayerState.bXxx, Fn_).
struct FOLPlayerState
{
    bool bGodMode;
    bool bFreeCam;
    bool bFixedCam;
    bool bGhost;
    bool bBunnyHop;
    bool  bLimping;
    bool  bHobbling;
    float HobblingIntensity;
    float TargetHobblingIntensity;
    bool  bFreeCamInspector;
    bool  bUnlimitedBatteries;
    int   MaxHealth;   // AOLHero::HealthMax (APawn base)
    bool  bCheckpointListAvailable;
    bool  bSmoothCamera;
    float FreeCamFOV;
    float FreeCamSpeed;
    float DefaultFOV;
    float RunningFOV;
    float CamcorderMinFOV;
    float CamcorderMaxFOV;
    float CamcorderNVMaxFOV;
};
extern volatile FOLPlayerState GOLPlayerState;

// Call once per tick from OLSpawns_TickPreviews or a dedicated ticker.
void OLCmd_Player_UpdateSnapshot();
