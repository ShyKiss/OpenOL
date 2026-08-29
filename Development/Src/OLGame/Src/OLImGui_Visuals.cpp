/*=============================================================================
    OLImGui_Visuals.cpp — "Visuals" tab: show flags, stat overlays, rendering toggles.
=============================================================================*/
#include "OLImGui_Tabs.h"
#include "OLCmd_Player.h"

void OLImGui_TabVisuals()
{
    OL_T_SECTION("visuals.section.inspector");
    OL_TABLE_BEGIN("##inspector", 3, true, 0)
        OL_TABLE_CELL OL_T_TOGGLE("visuals.inspector.freecam", GOLPlayerState.bFreeCamInspector, OLCmd_ToggleFreeCamInspector);
    OL_TABLE_END

    OL_T_SECTION("visuals.section.stats");
    OL_TABLE_BEGIN("##stats", 3, true, 0)
        OL_TABLE_CELL DBG_CMD(OLLocale_T("visuals.stats.fps"),    "Stat FPS");
        OL_TABLE_CELL DBG_CMD(OLLocale_T("visuals.stats.unit"),   "Stat UNIT");
        OL_TABLE_CELL DBG_CMD(OLLocale_T("visuals.stats.levels"), "Stat LEVELS");
    OL_TABLE_END

    OL_T_SECTION("visuals.section.showflags");
    OL_TABLE_BEGIN("##showflags", 3, true, 0)
        OL_TABLE_CELL DBG_CMD(OLLocale_T("visuals.flag.bsp"),             "Show BSP");
        OL_TABLE_CELL DBG_CMD(OLLocale_T("visuals.flag.staticmeshes"),    "Show STATICMESHES");
        OL_TABLE_CELL DBG_CMD(OLLocale_T("visuals.flag.skeletalmeshes"),  "Show SKELETALMESHES");
        OL_TABLE_CELL DBG_CMD(OLLocale_T("visuals.flag.meshedges"),       "Show MESHEDGES");
        OL_TABLE_CELL DBG_CMD(OLLocale_T("visuals.flag.paths"),           "Show PATHS");
        OL_TABLE_CELL DBG_CMD(OLLocale_T("visuals.flag.bounds"),          "Show BOUNDS");
        OL_TABLE_CELL DBG_CMD(OLLocale_T("visuals.flag.collision"),       "Show COLLISION");
        OL_TABLE_CELL DBG_CMD(OLLocale_T("visuals.flag.volumes"),         "Show VOLUMES");
        OL_TABLE_CELL DBG_CMD(OLLocale_T("visuals.flag.fog"),             "Show FOG");
        OL_TABLE_CELL DBG_CMD(OLLocale_T("visuals.flag.postprocess"),     "Show POSTPROCESS");
        OL_TABLE_CELL DBG_CMD(OLLocale_T("visuals.flag.lightfuncs"),      "Show LIGHTFUNCTIONS");
        OL_TABLE_CELL DBG_CMD(OLLocale_T("visuals.flag.zeroextent"),      "Show ZEROEXTENT");
        OL_TABLE_CELL DBG_CMD(OLLocale_T("visuals.flag.levelcoloration"), "Show LEVELCOLORATION");
        OL_TABLE_CELL DBG_CMD(OLLocale_T("visuals.flag.miplevels"),       "SHOWMIPLEVELS");
        OL_TABLE_CELL DBG_CMD(OLLocale_T("visuals.flag.aidebug"),         "ShowDebug OLAI");
    OL_TABLE_END

    OL_T_SECTION("visuals.section.rendering");
    OL_TABLE_BEGIN("##rendering", 2, true, 0)
        OL_TABLE_CELL DBG_CMD(OLLocale_T("visuals.rendering.freeze"),       "FREEZERENDERING");
        OL_TABLE_CELL DBG_CMD(OLLocale_T("visuals.rendering.freezestream"), "FREEZESTREAMING");
    OL_TABLE_END

    OL_T_SECTION("visuals.section.framerate");
    OL_TABLE_BEGIN("##Framerate", 2, false, 0)
        if (GEngine)
        {
            float FPSLimit = GEngine->MaxSmoothedFrameRate;
            OL_TABLE_CELL
            {
                bool _changed = false;
                { float _sc = OLLocale_FitScaleLabel("visuals.framerate.max");
                  ImGui::SetWindowFontScale(_sc);
                  ImGui::TextUnformatted(OLLocale_T("visuals.framerate.max"));
                  ImGui::SetWindowFontScale(1.f);
                  ImGui::SameLine();
                  ImGui::SetNextItemWidth(-1.f);
                  _changed = ImGui::SliderFloat(OL_L_ID_, &FPSLimit, 5.f, 500.f);
                }
                if (_changed)
                    GEngine->MaxSmoothedFrameRate = FPSLimit;
            }
        }
    OL_TABLE_END
}
