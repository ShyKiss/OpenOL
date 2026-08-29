#include "Multiplayer.h"
#include "MultiplayerHUD.h"
#include "OLUtilities.h"

// AMultiplayerHUD::AddNotification — native implementation.
// Appends a timed notification to the HUD's Notifications array.
void AMultiplayerHUD::AddNotification(const FString& Msg)
{
    FLOAT Now = (GWorld && GWorld->GetWorldInfo()) ? GWorld->GetWorldInfo()->TimeSeconds : 0.0f;
    FNotificationEntry E;
    E.Text       = Msg;
    E.ExpireTime = Now + 5.0f; // NOTIF_DURATION
    Notifications.AddItem(E);
}

void HUD_AddNotification(AMultiplayerHUD* HUD, const FString& Msg)
{
    if (HUD)
        HUD->AddNotification(Msg);
}

// ---------------------------------------------------------------------------
// MpHud_UpdateSnapshot — fills GMpHudBack each game tick.
// Called from FOLImGuiTicker::Tick (OLImGui.cpp) which can't include Multiplayer.h.
// ---------------------------------------------------------------------------

// Convert TCHAR to UTF-8 and write into a char buffer (ImGui snapshot fields).
static void TCHARToMP(char* Dst, int Len, const TCHAR* Src)
{
    INT Written = TCHARToUTF8((BYTE*)Dst, Len, Src);
    Dst[Written] = '\0';
}

void MpHud_UpdateSnapshot()
{
    AMultiplayerController* TC = GMultiplayerController;

    FMpHudSnapshot& S = GMpHudBack;
    appMemzero(&S, sizeof(S));

    if (!TC)
        return;

    FLOAT Now = (GWorld && GWorld->GetWorldInfo()) ? GWorld->GetWorldInfo()->TimeSeconds : 0.f;

    // --- Server / ping ---
    TCHARToMP(S.ServerName, MP_HUD_SERVER_LEN, *TC->ServerName);
    S.OnlineCount = TC->OnlineCount;
    S.TimeSeconds = Now;

    UBOOL bPingStale = (TC->LastPongTime > 0.f && Now - TC->LastPongTime > 10.f)
                    || (TC->CurrentPingMs <= 0.f);
    S.bPingStale = !!bPingStale;
    S.PingMs     = TC->CurrentPingMs;

    if (bPingStale)         S.PingBars = 0;
    else if (S.PingMs <= 200.f) S.PingBars = 4;
    else if (S.PingMs <= 500.f) S.PingBars = 2;
    else                        S.PingBars = 1;

    // --- Self ---
    FString SelfNick = TC->NativeGetUsername();
    FLOAT SelfHealth = 1.f;
    if (TC->Pawn)
        SelfHealth = Clamp<FLOAT>(FLOAT(TC->Pawn->Health) / 100.f, 0.f, 1.f);

    if (S.PlayerCount < MP_HUD_MAX_PLAYERS)
    {
        FMpHudPlayer& P = S.Players[S.PlayerCount++];
        TCHARToMP(P.Nick, MP_HUD_NICK_LEN, *SelfNick);
        P.HealthRatio  = SelfHealth;
        P.bIsSelf      = true;
        P.ScreenX      = -1.f;
        P.ScreenY      = -1.f;
        P.NametagScale = 0.f;
    }

    // --- Remote players ---
    for (INT i = 0; i < TC->RemotePlayers.Num() && S.PlayerCount < MP_HUD_MAX_PLAYERS; ++i)
    {
        URemotePlayer* RP = TC->RemotePlayers(i);
        if (!RP) continue;

        FMpHudPlayer& P = S.Players[S.PlayerCount++];
        const FString& Nick = RP->PlayerNick.Len() > 0 ? RP->PlayerNick : RP->Nick;
        TCHARToMP(P.Nick, MP_HUD_NICK_LEN, *Nick);
        P.HealthRatio  = Clamp<FLOAT>(FLOAT(RP->LastRemoteHealth) / 100.f, 0.f, 1.f);
        P.bIsSelf      = false;
        P.ScreenX      = -1.f;
        P.ScreenY      = -1.f;
        P.NametagScale = 0.f;
    }

    // --- Notifications ---
    AMultiplayerHUD* HUD = Cast<AMultiplayerHUD>(TC->myHUD);
    if (HUD)
    {
        for (INT i = 0; i < HUD->Notifications.Num() && S.NotifCount < MP_HUD_MAX_NOTIF; ++i)
        {
            const FNotificationEntry& N = HUD->Notifications(i);
            FLOAT Remain = N.ExpireTime - Now;
            if (Remain <= 0.f) continue;

            FMpHudNotif& Out = S.Notifs[S.NotifCount++];
            TCHARToMP(Out.Text, MP_HUD_NOTIF_LEN, *N.Text);
            Out.bIsDisconnect = (N.Text.InStr(TEXT("disconnected")) != INDEX_NONE);

            FLOAT Age = 5.f - Remain; // NOTIF_DURATION = 5
            Out.Alpha = 1.f;
            if (Age < 0.25f)        Out.Alpha = Clamp<FLOAT>(Age / 0.25f, 0.f, 1.f);
            else if (Remain < 0.6f) Out.Alpha = Clamp<FLOAT>(Remain / 0.6f, 0.f, 1.f);
        }
    }

    // Screen size from viewport
    if (GEngine && GEngine->GameViewport && GEngine->GameViewport->Viewport)
    {
        S.ScreenW = (FLOAT)GEngine->GameViewport->Viewport->GetSizeX();
        S.ScreenH = (FLOAT)GEngine->GameViewport->Viewport->GetSizeY();
    }

    // Connection status — shown in HUD while connecting so the user knows what's happening.
    extern FMpConnection GMpConn;
    extern FResolveInfo* GResolveInfo;
    if (GResolveInfo)
        S.ConnStatus = MCS_Resolving;
    else if (!GMpConn.bResolved)
        S.ConnStatus = MCS_Disconnected;
    else if (!GMpConn.bIsConnected)
        S.ConnStatus = MCS_Connecting;
    else
        S.ConnStatus = MCS_Connected;

    S.bValid = true;

    // Swap to front (render thread reads GMpHudFront).
    Exchange(GMpHudFront, GMpHudBack);
    GMpHudReady = 1;
}
