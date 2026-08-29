/*=============================================================================
    OLImGui_Player.cpp — "Player" tab UI.
=============================================================================*/
#include "OLImGui_Tabs.h"
#include "OLCmd_Player.h"

static volatile int   GPendingMaxHealth = -1;
static volatile float GPendingHobblingCurrent = -1.f;
static volatile float GPendingHobblingTarget  = -1.f;
static volatile float GPendingFreeCamFOV      = -1.f;
static volatile float GPendingFreeCamSpeed    = -1.f;
static volatile float GPendingDefaultFOV      = -1.f;
static volatile float GPendingRunningFOV      = -1.f;
static volatile float GPendingCamcorderMinFOV = -1.f;
static volatile float GPendingCamcorderMaxFOV = -1.f;
static volatile float GPendingCamcorderNVFOV  = -1.f;

static void ApplyMaxHealth_GameThread()
{
    int v = GPendingMaxHealth;
    GPendingMaxHealth = -1;
    if (v > 0) OLCmd_SetMaxHealth(v);
}

static void ApplyHobblingIntensity_GameThread()
{
    float cur = GPendingHobblingCurrent;
    float tgt = GPendingHobblingTarget;
    GPendingHobblingCurrent = GPendingHobblingTarget = -1.f;
    OLCmd_SetHobblingIntensity(cur, tgt);
}

static void ApplyFreeCamFOV_GameThread()      { float v = GPendingFreeCamFOV;      GPendingFreeCamFOV      = -1.f; OLCmd_SetFreeCamFOV(v);       }
static void ApplyFreeCamSpeed_GameThread()    { float v = GPendingFreeCamSpeed;    GPendingFreeCamSpeed    = -1.f; OLCmd_SetFreeCamSpeed(v);     }
static void ApplyDefaultFOV_GameThread()      { float v = GPendingDefaultFOV;      GPendingDefaultFOV      = -1.f; OLCmd_SetDefaultFOV(v);       }
static void ApplyRunningFOV_GameThread()      { float v = GPendingRunningFOV;      GPendingRunningFOV      = -1.f; OLCmd_SetRunningFOV(v);       }
static void ApplyCamcorderMinFOV_GameThread() { float v = GPendingCamcorderMinFOV; GPendingCamcorderMinFOV = -1.f; OLCmd_SetCamcorderMinFOV(v);  }
static void ApplyCamcorderMaxFOV_GameThread() { float v = GPendingCamcorderMaxFOV; GPendingCamcorderMaxFOV = -1.f; OLCmd_SetCamcorderMaxFOV(v);  }
static void ApplyCamcorderNVFOV_GameThread()  { float v = GPendingCamcorderNVFOV;  GPendingCamcorderNVFOV  = -1.f; OLCmd_SetCamcorderNVMaxFOV(v); }

void OLImGui_TabPlayer()
{
    const FOLPlayerState& S = *(const FOLPlayerState*)&GOLPlayerState;

    OL_T_SECTION("player.section.toggles");
    OL_TABLE_BEGIN("##player_btns", 3, true, 0)
        OL_TABLE_CELL  OL_T_TOGGLE_B("player.ghost",    "player.toggle.ghost",         S.bGhost,              OLCmd_ToggleGhost);
        OL_TABLE_CELL  OL_T_TOGGLE_B("player.bhop",     "player.toggle.bunnyhop",      S.bBunnyHop,           OLCmd_ToggleBunnyHop);
        OL_TABLE_CELL  OL_T_TOGGLE_B("player.unl_bat",  "player.toggle.unlimited_bat", S.bUnlimitedBatteries, OLCmd_ToggleUnlimitedBatteries);
    OL_TABLE_END

    OL_T_SECTION("player.section.hobble");
    {
        OL_TABLE_BEGIN("##hobble_btns", 2, true, 0)
            OL_TABLE_CELL  OL_T_TOGGLE_B("player.limp",   "player.toggle.limp",   S.bLimping,  OLCmd_ToggleLimp);
            OL_TABLE_CELL  OL_T_TOGGLE_B("player.hobble", "player.toggle.hobble", S.bHobbling, OLCmd_ToggleHobble);
        OL_TABLE_END

        static float sCurrent = 0.f;
        static float sTarget  = 0.f;
        static bool  sSliderActive = false;
        if (!sSliderActive)
        {
            sCurrent = S.HobblingIntensity;
            sTarget  = S.TargetHobblingIntensity;
        }

        bool changed = false;
        OL_T_L_SLIDER_F("player.hobble.current", &sCurrent, 0.f, 1.f); changed |= ImGui::IsItemEdited();
        OL_T_L_SLIDER_F("player.hobble.target",  &sTarget,  0.f, 1.f); changed |= ImGui::IsItemEdited();
        if (changed)
        {
            sSliderActive           = true;
            GPendingHobblingCurrent = sCurrent;
            GPendingHobblingTarget  = sTarget;
            OLImGui_EnqueueCall(ApplyHobblingIntensity_GameThread);
        }
        if (!ImGui::IsAnyItemActive())
            sSliderActive = false;
    }

    OL_T_SECTION("player.section.camera");
    {
        OL_TABLE_BEGIN("##cam_btns", 3, true, 0)
            OL_TABLE_CELL  OL_T_TOGGLE_B("player.freecam",    "player.toggle.freecam",       S.bFreeCam,      OLCmd_ToggleFreecam);
            OL_TABLE_CELL  OL_T_TOGGLE_B("player.fixedcam",   "player.toggle.fixedcam",      S.bFixedCam,     OLCmd_ToggleFixedCam);
            OL_TABLE_CELL  OL_T_TOGGLE_B("player.cam.smooth", "player.toggle.smooth_camera", S.bSmoothCamera, OLCmd_ToggleSmoothCamera);
        OL_TABLE_END

        // FreeCam FOV + Speed
        static float sFreeCamFOV   = 90.f;
        static float sFreeCamSpeed = 0.004f;
        static bool  sFreeCamActive = false;
        if (!sFreeCamActive) { sFreeCamFOV = S.FreeCamFOV; sFreeCamSpeed = S.FreeCamSpeed; }
        OL_T_L_SLIDER_F("player.cam.freecam_fov",   &sFreeCamFOV,   10.f,   170.f);
        if (ImGui::IsItemEdited()) { sFreeCamActive = true; GPendingFreeCamFOV   = sFreeCamFOV;   OLImGui_EnqueueCall(ApplyFreeCamFOV_GameThread);   }
        OL_T_L_SLIDER_F("player.cam.freecam_speed", &sFreeCamSpeed, 0.0001f, 0.05f);
        if (ImGui::IsItemEdited()) { sFreeCamActive = true; GPendingFreeCamSpeed = sFreeCamSpeed; OLImGui_EnqueueCall(ApplyFreeCamSpeed_GameThread); }
        if (!ImGui::IsAnyItemActive()) sFreeCamActive = false;

        // Collapsible FOV settings
        if (ImGui::TreeNode(OLLocale_T("player.cam.fov_settings")))
        {
            static float sDefaultFOV      = 80.f;
            static float sRunningFOV      = 90.f;
            static float sCamcorderMinFOV = 55.f;
            static float sCamcorderMaxFOV = 75.f;
            static float sCamcorderNVFOV  = 55.f;
            static bool  sFovActive = false;
            if (!sFovActive)
            {
                sDefaultFOV      = S.DefaultFOV;
                sRunningFOV      = S.RunningFOV;
                sCamcorderMinFOV = S.CamcorderMinFOV;
                sCamcorderMaxFOV = S.CamcorderMaxFOV;
                sCamcorderNVFOV  = S.CamcorderNVMaxFOV;
            }
            OL_T_L_SLIDER_F("player.cam.default_fov",       &sDefaultFOV,      40.f, 170.f);
            if (ImGui::IsItemEdited()) { sFovActive = true; GPendingDefaultFOV      = sDefaultFOV;      OLImGui_EnqueueCall(ApplyDefaultFOV_GameThread);      }
            OL_T_L_SLIDER_F("player.cam.running_fov",       &sRunningFOV,      40.f, 170.f);
            if (ImGui::IsItemEdited()) { sFovActive = true; GPendingRunningFOV      = sRunningFOV;      OLImGui_EnqueueCall(ApplyRunningFOV_GameThread);      }
            OL_T_L_SLIDER_F("player.cam.camcorder_min_fov", &sCamcorderMinFOV, 10.f, 120.f);
            if (ImGui::IsItemEdited()) { sFovActive = true; GPendingCamcorderMinFOV = sCamcorderMinFOV; OLImGui_EnqueueCall(ApplyCamcorderMinFOV_GameThread); }
            OL_T_L_SLIDER_F("player.cam.camcorder_max_fov", &sCamcorderMaxFOV, 10.f, 120.f);
            if (ImGui::IsItemEdited()) { sFovActive = true; GPendingCamcorderMaxFOV = sCamcorderMaxFOV; OLImGui_EnqueueCall(ApplyCamcorderMaxFOV_GameThread); }
            OL_T_L_SLIDER_F("player.cam.camcorder_nv_fov",  &sCamcorderNVFOV,  10.f, 120.f);
            if (ImGui::IsItemEdited()) { sFovActive = true; GPendingCamcorderNVFOV  = sCamcorderNVFOV;  OLImGui_EnqueueCall(ApplyCamcorderNVFOV_GameThread);  }
            if (!ImGui::IsAnyItemActive()) sFovActive = false;
            ImGui::TreePop();
        }
    }

    OL_T_SECTION("player.section.health");
    {
        OL_TABLE_BEGIN("##health_btns", 1, true, 0)
            OL_TABLE_CELL  OL_T_TOGGLE_B("player.god", "player.toggle.god", S.bGodMode, OLCmd_ToggleGodMode);
        OL_TABLE_END

        static int  sMaxHealth    = 100;
        static bool sSliderActive = false;
        if (!sSliderActive)
            sMaxHealth = S.MaxHealth > 0 ? S.MaxHealth : 100;

        OL_T_L_SLIDER_I("player.health.max", &sMaxHealth, 1, 500);
        if (ImGui::IsItemEdited())
        {
            sSliderActive     = true;
            GPendingMaxHealth = sMaxHealth;
            OLImGui_EnqueueCall(ApplyMaxHealth_GameThread);
        }
        if (!ImGui::IsAnyItemActive())
            sSliderActive = false;

        ImGui::SameLine();
        if (ImGui::Button(OLLocale_T("player.health.kill")))
            OLImGui_EnqueueCall(OLCmd_KillPlayer);
    }
}
