/*=============================================================================
    OLImGui_MultiplayerHUD.cpp — MP HUD overlay drawn via BackgroundDrawList.
    Always rendered regardless of debug window state.
=============================================================================*/
#include "OLImGui_Tabs.h"

// ---------------------------------------------------------------------------
// Drawing helpers (local)
// ---------------------------------------------------------------------------

static void MpHud_DrawAccentPanel(ImDrawList* DL, float X, float Y, float W, float H, ImU32 AccentCol)
{
    const float AccentW = 3.f;
    DL->AddRectFilled(ImVec2(X, Y),   ImVec2(X+W, Y+H), IM_COL32(12,12,14,150));
    DL->AddLine(ImVec2(X, Y),         ImVec2(X+W, Y),   IM_COL32(255,255,255,16));
    DL->AddLine(ImVec2(X, Y+H),       ImVec2(X+W, Y+H), IM_COL32(255,255,255,16));
    DL->AddRectFilled(ImVec2(X, Y),   ImVec2(X+AccentW, Y+H), AccentCol);
}

static void MpHud_DrawSignalBars(ImDrawList* DL, float X, float Y, ImU32 Col, int Filled)
{
    const float BW = 3.f, Gap = 2.f;
    for (int i = 0; i < 4; ++i)
    {
        float BH = 4.f + i * 3.f;
        float BX = X + i * (BW + Gap);
        float BY = Y + (13.f - BH);
        ImU32 C  = (i < Filled) ? Col : IM_COL32(
            (Col>>0)&0xFF, (Col>>8)&0xFF, (Col>>16)&0xFF, 45);
        DL->AddRectFilled(ImVec2(BX, BY), ImVec2(BX+BW, BY+BH), C);
    }
}

static ImU32 MpHud_PingColor(const FMpHudSnapshot& S)
{
    if (S.bPingStale || S.PingBars == 0) return IM_COL32(160,160,160,255);
    if (S.PingBars == 4)                 return IM_COL32(100,220,100,255);
    if (S.PingBars == 2)                 return IM_COL32(230,140, 60,255);
    return                                      IM_COL32(220, 80, 80,255);
}

// ---------------------------------------------------------------------------
// Game-thread callbacks (dispatched via OLImGui_EnqueueCall)
// ---------------------------------------------------------------------------

// Defined in Multiplayer/Src/MultiplayerLink.cpp — plain C wrapper, no Multiplayer.h needed.
extern void MpConn_CancelConnect();

static void MpHud_CancelConnect()
{
    MpConn_CancelConnect();
}

// ---------------------------------------------------------------------------
// Public entry point — called each frame from OLImGui_BuildUI
// ---------------------------------------------------------------------------

void MpHud_DrawOverlay()
{
    if (!GMpHudFront.bValid)
        return;

    const FMpHudSnapshot& S  = GMpHudFront;
    ImDrawList*           DL = ImGui::GetBackgroundDrawList();
    ImGuiIO&              IO = ImGui::GetIO();

    const float SW  = IO.DisplaySize.x;
    const float SH  = IO.DisplaySize.y;
    const float Pad = 8.f;

    // --- Connecting / Resolving status banner ---
    if (S.ConnStatus == MCS_Resolving || S.ConnStatus == MCS_Connecting)
    {
        // Request mouse capture so the Cancel button is clickable even with debug window closed.
        GImGuiWantMouse = 1;

        const char* StatusKey = (S.ConnStatus == MCS_Resolving) ? "mp.conn.resolving" : "mp.conn.connecting";
        const char* StatusBuf = OLLocale_T(StatusKey);

        ImFont* Font     = ImGui::GetFont();
        float   FontSize = SH * 0.033f;

        // Accent color per state.
        ImU32 AccentCol = (S.ConnStatus == MCS_Resolving) ? IM_COL32(180,160,80,255)
                                                           : IM_COL32(80,160,220,255);

        // Measure status text and Cancel button to size the panel.
        const char* CancelLabel = OLLocale_T("mp.conn.cancel");
        ImVec2 StatusSz = Font->CalcTextSizeA(FontSize, FLT_MAX, 0.f, StatusBuf);
        float  BaseFontSize = ImGui::GetFontSize();
        ImVec2 BtnSz    = Font->CalcTextSizeA(BaseFontSize, FLT_MAX, 0.f, CancelLabel);
        float  BW       = (StatusSz.x > BtnSz.x + Pad * 4.f ? StatusSz.x : BtnSz.x + Pad * 4.f) + Pad * 3.f;
        float  BH       = StatusSz.y + Pad * 1.5f + BtnSz.y + Pad * 3.f;
        float  BX       = SW * 0.5f - BW * 0.5f;
        float  BY       = SH * 0.5f - BH * 0.5f;

        // Dim the entire screen.
        DL->AddRectFilled(ImVec2(0, 0), ImVec2(SW, SH), IM_COL32(0, 0, 0, 160));

        // Panel: dark fill + thin top/bottom accent lines, no left bar.
        DL->AddRectFilled(ImVec2(BX, BY), ImVec2(BX+BW, BY+BH), IM_COL32(12,12,14,200));
        DL->AddLine(ImVec2(BX, BY),       ImVec2(BX+BW, BY),     AccentCol);
        DL->AddLine(ImVec2(BX, BY+BH),    ImVec2(BX+BW, BY+BH),  AccentCol);

        // Status text — centered, with jitter animation.
        float TextX = BX + BW * 0.5f - StatusSz.x * 0.5f;
        float TextY = BY + Pad * 0.75f;
        OLImGui_AddJitterText(DL, Font, FontSize, ImVec2(TextX, TextY),
            IM_COL32(235,235,235,255), StatusBuf, S.TimeSeconds, 0.5f);

        // Cancel button (drawn via ImGui so it receives clicks).
        float BtnRowY = BY + StatusSz.y + Pad * 2.f;
        float BtnX    = BX + BW * 0.5f - (BtnSz.x + Pad * 4.f) * 0.5f;
        ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_Always);
        ImGui::SetNextWindowSize(ImVec2(SW, SH), ImGuiCond_Always);
        ImGui::SetNextWindowBgAlpha(0.f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
        if (ImGui::Begin("##conn_overlay", NULL,
                ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
                ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoNav |
                ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBringToFrontOnFocus))
        {
            ImGui::SetCursorPos(ImVec2(BtnX, BtnRowY));
            ImGui::PushStyleColor(ImGuiCol_Button,        IM_COL32(160, 50, 50, 200));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, IM_COL32(210, 70, 70, 230));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive,  IM_COL32(240, 90, 90, 255));
            if (ImGui::Button(CancelLabel, ImVec2(BtnSz.x + Pad * 4.f, BtnSz.y + Pad)))
            {
                // Dispatch to game thread — CancelConnect must not be called from render thread.
                OLImGui_EnqueueCall(MpHud_CancelConnect);
            }
            ImGui::PopStyleColor(3);
        }
        ImGui::End();
        ImGui::PopStyleVar(2);

        return; // don't draw the normal HUD while not yet connected
    }

    // No longer connecting — release mouse capture and hide cursor if main window is closed.
    GImGuiWantMouse = 0;
    if (!GImGuiShowDemoWindow)
    {
        ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_NoMouseCursorChange;
        ::SetCursor(NULL);
    }

    ImU32       PingCol  = MpHud_PingColor(S);
    ImFont*     Font     = ImGui::GetFont();
    const float FontSize = ImGui::GetFontSize();

    // --- Server / ping panel (top-left) ---
    char PingBuf[48];
    if (S.bPingStale)
        _snprintf(PingBuf, sizeof(PingBuf), "Ping: ...");
    else
        _snprintf(PingBuf, sizeof(PingBuf), "Ping: %d ms", (int)S.PingMs);

    bool bShowServer = S.ServerName[0] != '\0';
    char ServerBuf[MP_HUD_SERVER_LEN + 32];
    if (bShowServer)
        _snprintf(ServerBuf, sizeof(ServerBuf), "%s: %d %s",
            S.ServerName, S.OnlineCount, S.OnlineCount == 1 ? "player" : "players");

    float PingW   = Font->CalcTextSizeA(FontSize, FLT_MAX, 0.f, PingBuf).x;
    float ServerW = bShowServer ? Font->CalcTextSizeA(FontSize, FLT_MAX, 0.f, ServerBuf).x : 0.f;
    float PanelW  = (ServerW > PingW ? ServerW : PingW) + 24.f + Pad * 2.f;
    float RowH    = FontSize + 4.f;
    float PanelH  = RowH * (bShowServer ? 2.f : 1.f) + Pad;

    float PX = Pad, PY = Pad;
    MpHud_DrawAccentPanel(DL, PX, PY, PanelW, PanelH, PingCol);

    float RowY = PY + Pad * 0.5f;
    if (bShowServer)
    {
        // Tilt animation: ease-in-out via sin, -2..0..2 degrees, period ~3.5s
        float T       = sinf(S.TimeSeconds * (2.f * 3.14159f / 3.5f));
        float TiltRad = T * 2.f * (3.14159f / 180.f);

        ImVec2 SrvSize = Font->CalcTextSizeA(FontSize, FLT_MAX, 0.f, ServerBuf);
        ImVec2 SrvPos  = ImVec2(PX + Pad * 1.5f, RowY);

        OLImGui_AddTiltedText(DL, Font, FontSize, SrvPos,
            IM_COL32(190,210,255,255), ServerBuf, TiltRad);
        RowY += RowH;
    }

    MpHud_DrawSignalBars(DL, PX + Pad * 1.5f, RowY + (RowH - 13.f) * 0.5f, PingCol, S.PingBars);
    DL->AddText(Font, FontSize, ImVec2(PX + Pad * 1.5f + 24.f, RowY), PingCol, PingBuf);

    float CurY = PY + PanelH + Pad * 0.5f;

    // --- Nick list ---
    for (int i = 0; i < S.PlayerCount; ++i)
    {
        const FMpHudPlayer& P = S.Players[i];
        char NickBuf[MP_HUD_NICK_LEN + 8];
        if (P.bIsSelf)
            _snprintf(NickBuf, sizeof(NickBuf), "%s (You)", P.Nick);
        else
            _snprintf(NickBuf, sizeof(NickBuf), "%s", P.Nick);

        ImVec2 Sz = Font->CalcTextSizeA(FontSize, FLT_MAX, 0.f, NickBuf);
        float  NW = Sz.x + Pad * 2.f;
        float  NH = Sz.y + Pad;

        byte  R = 255, G = (byte)(255.f * P.HealthRatio), B = (byte)(255.f * P.HealthRatio);
        ImU32 HealthCol = IM_COL32(R, G, B, 230);
        ImU32 AccentCol = P.bIsSelf ? IM_COL32(100,180,255,255) : HealthCol;

        MpHud_DrawAccentPanel(DL, Pad, CurY, NW, NH, AccentCol);
        DL->AddText(Font, FontSize, ImVec2(Pad * 2.f, CurY + Pad * 0.5f), HealthCol, NickBuf);
        CurY += NH + 2.f;
    }

    // --- Notifications (bottom-left) ---
    if (S.NotifCount > 0)
    {
        const float AccentW = 3.f;
        float NY = SH - Pad - S.NotifCount * (FontSize + Pad + 2.f);
        for (int i = 0; i < S.NotifCount; ++i)
        {
            const FMpHudNotif& N = S.Notifs[i];
            byte  A   = (byte)(255.f * N.Alpha);
            byte  FA  = (byte)(150.f * N.Alpha);
            byte  LA  = (byte)(16.f  * N.Alpha);
            ImU32 AccentCol = N.bIsDisconnect ? IM_COL32(220,90,90,255) : IM_COL32(120,220,140,255);
            ImVec2 Sz = Font->CalcTextSizeA(FontSize, FLT_MAX, 0.f, N.Text);
            float  NW = Sz.x + Pad * 2.f;
            float  NH = Sz.y + Pad;

            DL->AddRectFilled(ImVec2(Pad, NY),    ImVec2(Pad+NW, NY+NH), IM_COL32(12,12,14,FA));
            DL->AddLine(ImVec2(Pad, NY),           ImVec2(Pad+NW, NY),    IM_COL32(255,255,255,LA));
            DL->AddLine(ImVec2(Pad, NY+NH),        ImVec2(Pad+NW, NY+NH), IM_COL32(255,255,255,LA));
            DL->AddRectFilled(ImVec2(Pad, NY),     ImVec2(Pad+AccentW, NY+NH), AccentCol);
            DL->AddText(Font, FontSize, ImVec2(Pad * 1.5f + 4.f, NY + Pad * 0.5f),
                IM_COL32(235,235,235,A), N.Text);
            NY += NH + 2.f;
        }
    }
}
