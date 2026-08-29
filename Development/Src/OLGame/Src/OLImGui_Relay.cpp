/*=============================================================================
    OLImGui_Relay.cpp — "Relay" tab: relay server controls + log output.
=============================================================================*/
#include "OLImGui_Tabs.h"
#include "OpenOLGlobals.h"

#ifndef _WINSOCK2API_
#  define _WINSOCK2API_
#endif
#include "RelayThread.h"

void OLImGui_TabRelay()
{
    static char SRelayPort[8]     = "7777";
    static char SRelayName[64]    = "My Server";
    static char SRelayDbPath[256] = "relay_db.json";
    static char SRelayLog[4096]   = {};
    static int  SRelayLogLen      = 0;

    // Drain new log lines from relay thread.
    {
        char Line[HISTORY_MSG_LEN];
        while (GRelayThread.PopLogLine(Line))
        {
            int LineLen = 0;
            while (Line[LineLen] && LineLen < HISTORY_MSG_LEN - 1) ++LineLen;
            if (SRelayLogLen + LineLen + 2 > (int)sizeof(SRelayLog))
            {
                int Half = (int)sizeof(SRelayLog) / 2;
                memmove(SRelayLog, SRelayLog + Half, SRelayLogLen - Half);
                SRelayLogLen -= Half;
            }
            memcpy(SRelayLog + SRelayLogLen, Line, LineLen);
            SRelayLogLen += LineLen;
            SRelayLog[SRelayLogLen++] = '\n';
            SRelayLog[SRelayLogLen]   = '\0';
        }
    }

    bool bRunning = GRelayThread.IsRunning() != 0;

    if (bRunning)
    {
        ImGui::TextColored(ImVec4(0.2f, 1.0f, 0.2f, 1.0f), "Status: RUNNING  port %s", SRelayPort);
        if (ImGui::Button(OLLocale_T("relay.stop")))
        {
            GRelayThread.StopRelay();
            SRelayLogLen = 0; SRelayLog[0] = '\0';
            ClearRelayRichPresence();
        }
    }
    else
    {
        ImGui::TextColored(ImVec4(1.0f, 0.5f, 0.2f, 1.0f), "Status: stopped");
        ImGui::SetNextItemWidth(80.0f);
        ImGui::InputText(OLLocale_T("relay.port"),    SRelayPort,   sizeof(SRelayPort));
        ImGui::SetNextItemWidth(200.0f);
        ImGui::InputText(OLLocale_T("relay.name"),    SRelayName,   sizeof(SRelayName));
        ImGui::SetNextItemWidth(300.0f);
        ImGui::InputText(OLLocale_T("relay.db_path"), SRelayDbPath, sizeof(SRelayDbPath));
        if (ImGui::Button(OLLocale_T("relay.start")))
        {
            int Port = 0;
            for (int ci = 0; SRelayPort[ci]; ++ci)
                Port = Port * 10 + (SRelayPort[ci] - '0');
            GRelayThread.StartRelay((WORD)Port, SRelayName, SRelayDbPath);
            SetRelayRichPresence("DEFAULT");
        }
    }

    ImGui::Separator();
    ImGui::BeginChild("##relaylog", ImVec2(0, 0), false, ImGuiWindowFlags_HorizontalScrollbar);
    ImGui::TextUnformatted(SRelayLog, SRelayLog + SRelayLogLen);
    if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY())
        ImGui::SetScrollHereY(1.0f);
    ImGui::EndChild();
}
