/*=============================================================================
    MultiplayerLink.cpp — FMpConnection: persistent UDP connection singleton.
=============================================================================*/
// winsock2.h first — P2PBridge.h requires it.
#ifdef _WIN32
#  ifndef _WINDOWS_
#    define WIN32_LEAN_AND_MEAN
#    ifndef NOMINMAX
#      define NOMINMAX
#    endif
#    include <winsock2.h>
#  endif
#endif
#ifndef _WINSOCK2API_
#  define _WINSOCK2API_
#endif

#include "Multiplayer.h"
#include "HeroChannelPackets.h"
#include "ServerPackets.h"
#include "WorldChannelPackets.h"
#include "UnSocket.h"
#include "..\..\OnlineSubsystemSteamworks\Inc\OnlineSubsystemSteamworks.h"
#include "RelayThread.h"    // GRelayThread (for port) + P2PBridge.h (via RelayThread.h)
#include "MultiplayerHUD.h"

#if WITH_UE3_NETWORKING

IMPLEMENT_CLASS(AMultiplayerLink);

// ---------------------------------------------------------------------------
// Singleton
// ---------------------------------------------------------------------------

FMpConnection        GMpConn;
FMpConnectionTicker* GMpTicker = NULL;

// Global pointers — set each map load by AMultiplayerController::NativeInit.
AMultiplayerController* GMultiplayerController = NULL;
AOLHero*                GMultiplayerHero       = NULL;

FResolveInfo* GResolveInfo = NULL;

// Called from OnlineSubsystemSteamworks when Rich Presence join arrives at runtime (hot join).
// Mirrors the cold-launch path: sets bP2PColdLaunch so NativeInit calls ConnectP2P.
void GMpConn_SetPendingP2PJoin(QWORD HostSteamID, const FString& RoomCode)
{
    GMpConn.bP2PColdLaunch    = TRUE;
    GMpConn.P2PColdLaunchHost = FString::Printf(TEXT("%llu"), (unsigned long long)HostSteamID);
    GMpConn.P2PColdLaunchRoom = RoomCode.IsEmpty() ? TEXT("DEFAULT") : RoomCode;
    debugf(NAME_Log, TEXT("[MP] Hot P2P join queued: host=%llu room='%s'"),
        (unsigned long long)HostSteamID, *GMpConn.P2PColdLaunchRoom);
}

void AMultiplayerLink::NativeReloadConfig()
{
    GMpConn.bResolved = FALSE;
    GResolveInfo          = NULL;
    GMpConn.LoadConfig();
}


// ---------------------------------------------------------------------------
// FMpConnection — config
// ---------------------------------------------------------------------------

void FMpConnection::LoadConfig()
{
    // Read exclusively from OLGame.OLNetworkConfig — single source of truth.
    const FString IniFile = appGameConfigDir() + TEXT("OLMultiplayer.ini");
    GConfig->LoadFile(*IniFile);
    const TCHAR* Ini  = *IniFile;
    const TCHAR* Sect = TEXT("OLGame.OLNetworkConfig");

    FString Tmp;
    if (GConfig->GetString(Sect, TEXT("IP"),       Tmp, Ini)) IP       = Tmp;
    if (GConfig->GetString(Sect, TEXT("UdpPort"),  Tmp, Ini)) UdpPort  = Tmp;
    if (GConfig->GetString(Sect, TEXT("UserName"), Tmp, Ini)) Username = Tmp;
    if (GConfig->GetString(Sect, TEXT("RoomCode"), Tmp, Ini)) RoomCode = Tmp;
    if (GConfig->GetString(Sect, TEXT("Password"), Tmp, Ini)) Password = Tmp;

    UBOOL B = TRUE;
    if (GConfig->GetBool(Sect, TEXT("SyncInteractable"), B, Ini)) SyncInteractable = B; B = TRUE;
    if (GConfig->GetBool(Sect, TEXT("SyncEnemies"),      B, Ini)) SyncEnemies      = B; B = TRUE;
    if (GConfig->GetBool(Sect, TEXT("SyncMatinees"),     B, Ini)) SyncMatinees     = B; B = TRUE;
    if (GConfig->GetBool(Sect, TEXT("SyncPickups"),      B, Ini)) SyncPickups      = B; B = FALSE;
    if (GConfig->GetBool(Sect, TEXT("SpeedrunMode"),     B, Ini)) SpeedrunMode     = B;

    if (IP.IsEmpty())       IP       = TEXT("127.0.0.1");
    if (UdpPort.IsEmpty())  UdpPort  = TEXT("7777");
    if (Username.IsEmpty()) Username = TEXT("Player");
    if (RoomCode.IsEmpty()) RoomCode = TEXT("DEFAULT");

    // One-time cmdline check: -openol_host <steamID> -openol_room <room> (Steam cold-launch).
    // If present, store for NativeInit to call ConnectP2P instead of normal Connect().
    static bool bCmdLineChecked = false;
    if (!bCmdLineChecked)
    {
        bCmdLineChecked = true;

        auto ParseSpaceArg = [](const TCHAR* CL, const TCHAR* Flag, TCHAR* Out, INT Max)
        {
            const TCHAR* Found = appStrstr(CL, Flag);
            if (!Found) return;
            Found += appStrlen(Flag);
            while (*Found == TEXT(' ')) Found++;
            INT i = 0;
            while (*Found && *Found != TEXT(' ') && i < Max - 1)
                Out[i++] = *Found++;
            Out[i] = 0;
        };

        TCHAR CmdHost[64] = {0};
        TCHAR CmdRoom[64] = {0};
        const TCHAR* CL = appCmdLine();

        if (!Parse(CL, TEXT("openol_host="), CmdHost, ARRAY_COUNT(CmdHost)))
            ParseSpaceArg(CL, TEXT("-openol_host "), CmdHost, ARRAY_COUNT(CmdHost));
        if (!Parse(CL, TEXT("openol_room="), CmdRoom, ARRAY_COUNT(CmdRoom)))
            ParseSpaceArg(CL, TEXT("-openol_room "), CmdRoom, ARRAY_COUNT(CmdRoom));

        if (CmdHost[0])
        {
            bP2PColdLaunch    = TRUE;
            P2PColdLaunchHost = FString(CmdHost);
            P2PColdLaunchRoom = CmdRoom[0] ? FString(CmdRoom) : TEXT("DEFAULT");
            debugf(NAME_Log, TEXT("MpConn: Cold-launch P2P detected: host=%s room=%s"),
                *P2PColdLaunchHost, *P2PColdLaunchRoom);
        }
    }
}

// ---------------------------------------------------------------------------
// FMpConnection::Connect — idempotent; starts DNS resolve on first call.
// ---------------------------------------------------------------------------

void FMpConnection::Connect()
{
    bCancelled = FALSE;  // explicit Connect() clears cancel state

    FString OldIP       = IP;
    FString OldUdpPort  = UdpPort;
    FString OldRoomCode = RoomCode;
    FString OldPassword = Password;

    // In P2P or host mode the caller already set IP/UdpPort/RoomCode/Password —
    // skip LoadConfig() so it doesn't overwrite them from the ini file.
    if (!bP2PMode && !bHostMode)
        LoadConfig();

    // If connection params changed, drop the existing connection and re-resolve.
    if (bResolved && (IP != OldIP || UdpPort != OldUdpPort || RoomCode != OldRoomCode || Password != OldPassword))
    {
        Disconnect();
        bResolved     = FALSE;
        bIsConnected  = FALSE;
        bIsHandshaked = FALSE;
        HelloTimer    = 0.f;
        if (GResolveInfo) { delete GResolveInfo; GResolveInfo = NULL; }
    }

    if (bResolved || GResolveInfo)
        return;

    // GetHostByName handles both dotted IPs (returns cached immediately)
    // and hostnames (async). We poll IsComplete() in Tick.
    debugf(NAME_Log, TEXT("MpConn: Resolving '%s':%s room='%s' P2P=%d"), *IP, *UdpPort, *RoomCode, (INT)bP2PMode);
    GResolveInfo = GSocketSubsystem->GetHostByName(TCHAR_TO_ANSI(*IP));
}

// ---------------------------------------------------------------------------
// FMpConnection::SendBinary
// ---------------------------------------------------------------------------

void FMpConnection::SendBinary(BYTE* Data, INT Count)
{
    // Wire format: [channel(1)][type(1)][player_id LE4][payload...]
    // Data layout: [channel(1)][type(1)][payload...]
    // SendBinary inserts player_id between byte[1] and byte[2].
    if (Count < 2 || Count + 4 > OL_MAX_MSG || !bIsConnected)
        return;

    BYTE Out[OL_MAX_MSG];
    Out[0] = Data[0];                        // channel
    Out[1] = Data[1];                        // type
    Out[2] = (BYTE)(LocalPlayerID);
    Out[3] = (BYTE)(LocalPlayerID >>  8);
    Out[4] = (BYTE)(LocalPlayerID >> 16);
    Out[5] = (BYTE)(LocalPlayerID >> 24);
    appMemcpy(Out + 6, Data + 2, Count - 2); // payload

    ol_conn_send(&Conn, ol_lane_for(Out[0], Out[1]), Out, Count + 4);
}

void FMpConnection::TransportDeliver(void* User, int Lane, uint8_t* Msg, int Len)
{
    ((FMpConnection*)User)->HandleMessage(Msg, Len);
}

void FMpConnection::TransportSend(void* User, const uint8_t* Datagram, int Len)
{
    FMpConnection* Self = (FMpConnection*)User;
    Self->SendTo(Self->ServerAddr, (BYTE*)Datagram, Len);
}

// ---------------------------------------------------------------------------
// FMpConnection::Disconnect — send DISCONNECT packet, reset state.
// ---------------------------------------------------------------------------

void FMpConnection::Disconnect()
{
    if (!bResolved || !bIsConnected)
        return;

    BYTE B[2] = { CH_WORLD, WORLD_DISCONNECT };
    SendBinary(B, 2);
    ol_conn_flush(&Conn, appSeconds(), TransportSend, this);

    bIsConnected  = FALSE;
    bIsHandshaked = FALSE;
    bP2PMode      = FALSE;
}

// ---------------------------------------------------------------------------
// FMpConnection::CancelConnect — abort an in-progress connect without sending DISCONNECT.
// ---------------------------------------------------------------------------

void FMpConnection::CancelConnect()
{
    // Abort pending DNS resolve.
    if (GResolveInfo)
    {
        delete GResolveInfo;
        GResolveInfo = NULL;
    }

    // Close the UDP socket so BindPort won't fail on the next Connect().
    if (SocketData.Socket)
    {
        SocketData.Socket->Close();
        GSocketSubsystem->DestroySocket(SocketData.Socket);
        SocketData.Socket = NULL;
    }

    // Reset handshake state so the next Connect() starts fresh.
    bResolved     = FALSE;
    bIsConnected  = FALSE;
    bIsHandshaked = FALSE;
    bCancelled    = TRUE;   // suppress auto-reconnect in Tick until Connect() is called again
    HelloAttempt  = 0;
}

// Plain C wrapper — callable from OLGame without including Multiplayer.h.
void MpConn_CancelConnect()
{
    GMpConn.CancelConnect();
}

// ---------------------------------------------------------------------------
// FMpConnection::SendText — legacy path for SendToServer(string).
// ---------------------------------------------------------------------------

void FMpConnection::SendText(const FString& Msg)
{
    if (!bResolved || !bIsConnected)
        return;
    FString Line = FString::Printf(TEXT("%d,%s\n"), LocalPlayerID, *Msg);
    FTCHARToANSI Conv(*Line);
    SendTo(ServerAddr, (BYTE*)(ANSICHAR*)Conv, Conv.Length());
}

// ---------------------------------------------------------------------------
// FMpConnection::OnReceivedData — FUdpLink callback (game thread via Poll).
// ---------------------------------------------------------------------------

void FMpConnection::OnReceivedData(FIpAddr SrcAddr, BYTE* Data, INT Count)
{
    if (Count <= 0)
        return;

    if (Data[0] == OL_MAGIC)
    {
        ol_conn_on_datagram(&Conn, appSeconds(), Data, Count, TransportDeliver, this);
        return;
    }

    if (bIsConnected)
        return;

    // Outside a session the server only takes part in the handshake.
    if (Data[0] == OL_HS_CHALLENGE && Count == OL_HS_CHALLENGE_BYTES)
    {
        if (!CheckServerKey(ol_hs_challenge_key(Data)))
            return;
        // The simulator proves the server's secret instead of a room password.
        BYTE Auth[OL_HS_AUTH_MAX];
        const INT Len = ol_hs_client_auth(&Hs, Data, Count, TCHAR_TO_ANSI(*RoomCode), TCHAR_TO_ANSI(*Password),
            GIsSimNode ? OL_ROLE_SIM : OL_ROLE_PLAYER, bHasSessionToken ? SessionToken : NULL, Auth);
        if (Len > 0)
            SendTo(ServerAddr, Auth, Len);
    }
    else if (Data[0] == OL_HS_RESULT)
    {
        BYTE Msg[OL_HS_RESULT_MAX];
        const INT Len = ol_hs_client_result(&Hs, Data, Count, Msg);
        if (Len > 0)
            HandleMessage(Msg, Len);
    }
}

// ---------------------------------------------------------------------------
// Server key pinning
// ---------------------------------------------------------------------------

static const TCHAR* GKnownServersSection = TEXT("OLGame.OLKnownServers");

// What the pin belongs to; empty means nothing on this path can be intercepted.
static FString ServerKeyId(const FMpConnection& Conn)
{
    if (GP2PBridgeClient.IsRunning())
        return FString::Printf(TEXT("steam:%llu"), (unsigned long long)Conn.HostSteamID);
    if (Conn.IP == TEXT("127.0.0.1") || Conn.IP == TEXT("localhost"))
        return FString();
    return FString::Printf(TEXT("%s:%s"), *Conn.IP, *Conn.UdpPort);
}

static FString KeyToHex(const BYTE* Key)
{
    FString Out;
    for (INT i = 0; i < 32; i++)
        Out += FString::Printf(TEXT("%02x"), Key[i]);
    return Out;
}

UBOOL FMpConnection::CheckServerKey(const BYTE* Key)
{
    PendingServerKey = KeyToHex(Key);

    FString Expected = ConfiguredServerKey;
    const FString Id = ServerKeyId(*this);
    const FString IniFile = appGameConfigDir() + TEXT("OLMultiplayer.ini");
    if (Expected.Len() == 0 && Id.Len() > 0)
        GConfig->GetString(GKnownServersSection, *Id, Expected, *IniFile);

    if (Expected.Len() == 0 || Expected == PendingServerKey)
        return TRUE;

    // Keep trying: a forged challenge must not be able to end the real connection.
    if (!bServerKeyWarned)
    {
        bServerKeyWarned = TRUE;
        debugf(NAME_Warning, TEXT("MpConn: Server key changed for %s: expected %s, got %s. Not connecting. If the server was reinstalled, remove its line from [%s] in OLMultiplayer.ini"),
            *Id, *Expected, *PendingServerKey, GKnownServersSection);
        if (GMultiplayerController)
            HUD_AddNotification(Cast<AMultiplayerHUD>(GMultiplayerController->myHUD),
                FString::Printf(TEXT("Server key changed, not connecting to %s. See the log."), *IP));
    }
    return FALSE;
}

void FMpConnection::RememberServerKey()
{
    const FString Id = ServerKeyId(*this);
    if (GIsSimNode || Id.Len() == 0 || PendingServerKey.Len() == 0)
        return;
    const FString IniFile = appGameConfigDir() + TEXT("OLMultiplayer.ini");
    FString Known;
    if (GConfig->GetString(GKnownServersSection, *Id, Known, *IniFile) && Known == PendingServerKey)
        return;
    debugf(NAME_Log, TEXT("MpConn: First connection to %s, remembering its key %s"), *Id, *PendingServerKey);
    GConfig->SetString(GKnownServersSection, *Id, *PendingServerKey, *IniFile);
    GConfig->Flush(FALSE, *IniFile);
}

void FMpConnection::HandleMessage(BYTE* Data, INT Count)
{
    if (Count <= 0)
        return;

    if (GWorld && GWorld->GetWorldInfo())
        LastReceivedTime = GWorld->GetWorldInfo()->TimeSeconds;

    // All packets are binary.
    // Server→client (SRV): [type >= 0xE0][sender=0 LE4][payload...]   (5+ bytes)
    // Client→client (game): [channel <= 0x05][type(1)][sender_id LE4][payload...]  (6+ bytes)
    if (Count >= 1)
    {
        BYTE FirstByte = Data[0];

        // --- Server-originated packets (0xE0..0xFF) ---
        if (FirstByte >= 0xE0)
        {
            if (Count < 5) return;
            BYTE  PktType  = FirstByte;
            // SenderID field is 0 (pad) for server packets; payload starts at Data[5]
            BYTE* Payload    = Data + 5;
            INT   PayloadLen = Count - 5;

            // SRV_READY (0xE0) — parse server name/player-id into GMpConn.
            if (PktType == SRV_READY && PayloadLen >= 4)
            {
                INT PID = (INT)((DWORD)Payload[0] | ((DWORD)Payload[1]<<8) |
                                ((DWORD)Payload[2]<<16) | ((DWORD)Payload[3]<<24));
                LocalPlayerID = PID;

                if (PayloadLen >= 5)
                {
                    BYTE NLen   = Payload[4];
                    INT  NBytes = Min((INT)NLen, PayloadLen - 5);
                    TCHAR TmpName[256] = {0};
                    for (INT i = 0; i < NBytes && i < 255; i++)
                        TmpName[i] = (TCHAR)Payload[5 + i];
                    ServerName = FString(TmpName);

                    INT TokenOffset = 5 + NBytes;
                    if (PayloadLen >= TokenOffset + 32)
                    {
                        appMemcpy(SessionToken, Payload + TokenOffset, 32);
                        bHasSessionToken = TRUE;
                    }
                    if (PayloadLen >= TokenOffset + 32 + 8)
                    {
                        QWORD ConnId = 0;
                        appMemcpy(&ConnId, Payload + TokenOffset + 32, 8);
                        if (ConnId != Conn.conn_id)
                        {
                            ol_conn_reset(&Conn, ConnId);
                            ol_conn_set_keys(&Conn, Hs.c2s_key, Hs.s2c_key);
                        }
                    }
                }
                else
                {
                    ServerName = TEXT("Server");
                }

                if (!bIsConnected)
                {
                    bIsConnected = TRUE;
                    HelloAttempt = 0;
                    bServerKeyWarned = FALSE;
                    RememberServerKey();
                    debugf(NAME_Log, TEXT("MpConn: Connected! PlayerID=%d server='%s' addr=%s:%s room='%s' P2P=%d"),
                        LocalPlayerID, *ServerName, *IP, *UdpPort, *RoomCode, (INT)bP2PMode);
                    if (GMultiplayerController)
                        GMultiplayerController->OnConnected();
                }
                if (GMultiplayerController)
                {
                    GMultiplayerController->ServerName  = ServerName;
                    GMultiplayerController->OnlineCount = OnlineCount;
                }
            }

            if (PktType == SRV_HELLO_FAIL && PayloadLen >= 1)
            {
                TCHAR Reason[256] = {0};
                const INT ReasonLen = Min((INT)Payload[0], Min(PayloadLen - 1, 255));
                for (INT i = 0; i < ReasonLen; i++)
                    Reason[i] = (TCHAR)Payload[1 + i];
                debugf(NAME_Log, TEXT("MpConn: Server refused the connection: %s"), Reason);
                if (GMultiplayerController && HelloAttempt <= 1)
                    HUD_AddNotification(Cast<AMultiplayerHUD>(GMultiplayerController->myHUD),
                        FString::Printf(TEXT("Server refused the connection: %s"), Reason));
            }

            // SRV_ONLINE_COUNT — update GMpConn.
            if (PktType == SRV_ONLINE_COUNT && PayloadLen >= 4)
            {
                INT Cnt = (INT)((DWORD)Payload[0] | ((DWORD)Payload[1]<<8) |
                                ((DWORD)Payload[2]<<16) | ((DWORD)Payload[3]<<24));
                OnlineCount = Max(Cnt, 1);
            }

            if (GMultiplayerController)
                GMultiplayerController->OnReceiveBinaryData(PktType, 0, Payload, PayloadLen);
            return;
        }

        // --- Client-originated game packets: [channel(1)][type(1)][sender_id LE4][payload...] ---
        if (FirstByte <= OL_CHANNEL_MAX)
        {
            if (Count < 6) return;
            BYTE  Channel    = FirstByte;
            BYTE  PktType    = Data[1];
            INT   SenderID   = (INT)((DWORD)Data[2] | ((DWORD)Data[3]<<8) |
                                     ((DWORD)Data[4]<<16) | ((DWORD)Data[5]<<24));
            BYTE* Payload    = Data + 6;
            INT   PayloadLen = Count - 6;

            // CH_SRV: PING echo
            if (Channel == CH_SRV && PktType == SRV_PING)
            {
                if (PayloadLen >= 4 && GMultiplayerController)
                {
                    DWORD SentMs = (DWORD)Payload[0] | ((DWORD)Payload[1] << 8)
                                 | ((DWORD)Payload[2] << 16) | ((DWORD)Payload[3] << 24);
                    DWORD NowMs  = (DWORD)(appSeconds() * 1000.0);
                    FLOAT RTT    = (FLOAT)(NowMs - SentMs);
                    if (RTT > 0.f && RTT < 10000.f)
                    {
                        GMultiplayerController->CurrentPingMs = RTT;
                        AWorldInfo* WI = GWorld ? GWorld->GetWorldInfo() : NULL;
                        GMultiplayerController->LastPongTime = WI ? WI->TimeSeconds : 0.f;
                    }
                }
                return;
            }

            // OnReceiveBinaryData expects: PktType=Channel, Data=[type(1)][payload...]
            // Wire has pid at Data[2..5], so build [type][payload] by skipping pid.
            if (GMultiplayerController && PayloadLen >= 0)
            {
                BYTE Buf[4096];
                Buf[0] = PktType;
                if (PayloadLen > 0)
                    appMemcpy(Buf + 1, Payload, PayloadLen);
                GMultiplayerController->OnReceiveBinaryData(Channel, SenderID, Buf, 1 + PayloadLen);
            }
            return;
        }
    }

}

// ---------------------------------------------------------------------------
// FMpConnection::Tick — polls socket, runs HELLO retries, heartbeat check.
// ---------------------------------------------------------------------------

void FMpConnection::Tick(FLOAT DeltaTime)
{
    // Check async DNS resolve.
    if (GResolveInfo)
    {
        if (GResolveInfo->IsComplete())
        {
            if (GResolveInfo->GetErrorCode() != SE_NO_ERROR)
            {
                // DNS failed — notify and retry next connect attempt
                debugf(NAME_Log, TEXT("MpConn: DNS resolve failed for '%s' (err=%d)"),
                    *IP, (INT)GResolveInfo->GetErrorCode());
                delete GResolveInfo;
                GResolveInfo = NULL;
                if (GMultiplayerController)
                    GMultiplayerController->OnDisconnected();
                return;
            }
            FInternetIpAddr Resolved = GResolveInfo->GetResolvedAddress();
            delete GResolveInfo;
            GResolveInfo = NULL;

            // FIpAddr has a constructor from FInternetIpAddr (Core.h).
            ServerAddr      = FIpAddr(Resolved);
            ServerAddr.Port = appAtoi(*UdpPort);
            bResolved = TRUE;
            // Global FMpConnection is constructed before GSocketSubsystem is ready,
            // so the socket must be created here, after subsystem init.
            if (SocketData.Socket == NULL && GSocketSubsystem)
            {
                SocketData.Socket = GSocketSubsystem->CreateDGramSocket(TEXT("MpConn"), TRUE);
                if (SocketData.Socket)
                {
                    SocketData.Socket->SetReuseAddr();
                    SocketData.Socket->SetNonBlocking();
                    SocketData.Socket->SetRecvErr();
                }
            }
            BindPort(0);
        }
        // Else: still resolving, wait.
        return;
    }

    // Poll socket for incoming datagrams.
    if (bResolved)
        Poll();

    if (!bResolved)
    {
        // Don't auto-reconnect after CancelConnect() — wait for an explicit Connect() call.
        if (!GResolveInfo && !bCancelled)
            Connect();
        return;
    }

    // HELLO retry loop — send until connected.
    if (!bIsConnected)
    {
        HelloTimer -= DeltaTime;
        if (HelloTimer <= 0.f)
        {
            HelloTimer = 2.0f;
            HelloAttempt++;
            debugf(NAME_Log, TEXT("MpConn: HELLO #%d -> %s:%s room='%s' P2P=%d token=%d"),
                HelloAttempt, *IP, *UdpPort, *RoomCode, (INT)bP2PMode, (INT)bHasSessionToken);
            BYTE Hello[OL_HS_HELLO_BYTES];
            ol_hs_client_hello(Hello);
            SendTo(ServerAddr, Hello, OL_HS_HELLO_BYTES);
        }
        return;
    }

    // PING — send every 5 seconds while handshaked.
    if (bIsHandshaked)
    {
        PingTimer -= DeltaTime;
        if (PingTimer <= 0.f)
        {
            PingTimer = 5.f;
            DWORD NowMs = (DWORD)(appSeconds() * 1000.0);
            BYTE B[6];
            B[0] = CH_SRV;
            B[1] = SRV_PING;
            B[2] = (BYTE)(NowMs);
            B[3] = (BYTE)(NowMs >> 8);
            B[4] = (BYTE)(NowMs >> 16);
            B[5] = (BYTE)(NowMs >> 24);
            SendBinary(B, 6);
        }
    }

    // Heartbeat check — disconnect if server went silent.
    HeartbeatTimer -= DeltaTime;
    if (HeartbeatTimer <= 0.f)
    {
        HeartbeatTimer = 5.0f;
        AWorldInfo* WI = GWorld ? GWorld->GetWorldInfo() : NULL;
        FLOAT Now = WI ? WI->TimeSeconds : 0.f;
        if (Now - LastReceivedTime > 30.0f)
        {
            bIsConnected  = FALSE;
            bIsHandshaked = FALSE;
            HelloAttempt  = 0;
            HelloTimer    = 0.f;
            if (GMultiplayerController)
                GMultiplayerController->OnDisconnected();
            return;
        }
    }

    ol_conn_flush(&Conn, appSeconds(), TransportSend, this);
}

// ---------------------------------------------------------------------------
// FMpConnection::ConnectP2P — connect via Steam P2P loopback bridge.
//
// Starts GP2PBridge in CLIENT mode: it listens on 127.0.0.1:(RelayPort+1)
// and tunnels all traffic to/from HostSteamID over Steam P2P.
// FMpConnection is pointed at that loopback port and proceeds normally.
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
// GMpConn_ConnectP2P — thin wrapper callable from OLGame without Multiplayer headers
// ---------------------------------------------------------------------------

void GMpConn_ConnectP2P(QWORD HostSteamID, WORD RelayPort,
    const FString& RoomCode, const FString& Password)
{
    GMpConn.ConnectP2P(HostSteamID, RelayPort, RoomCode, Password);
}

// GMpConn_ConnectLocal — local (non-P2P) connect to 127.0.0.1, used when hosting.
void GMpConn_ConnectLocal(WORD Port)
{
    GMpConn.IP       = TEXT("127.0.0.1");
    GMpConn.UdpPort  = FString::Printf(TEXT("%d"), (int)Port);
    GMpConn.RoomCode = TEXT("DEFAULT");
    GMpConn.bP2PMode  = FALSE;
    GMpConn.bHostMode = TRUE;
    GMpConn.Connect();
}

// Leaving to the main menu ends the multiplayer session completely: otherwise the
// next map would reconnect and the server would send the player back to the world.
void GMpConn_LeaveSession()
{
    if (GP2PBridgeClient.IsRunning())
        GP2PBridgeClient.StopBridge();

    GMpConn.Disconnect();
    GMpConn.CancelConnect();

    GMpConn.bHostMode        = FALSE;
    GMpConn.bP2PMode         = FALSE;
    GMpConn.bP2PColdLaunch   = FALSE;
    GMpConn.bHasSessionToken = FALSE;
    GMpConn.WorldEpoch       = 0;
    GMpConn.SimOwnerID       = 0;
    GMpConn.bTravelPending   = FALSE;
    GMpConn.bSkipNextEnter   = FALSE;
    GMpConn.TravelCheckpoint = TEXT("");
    GMpConn.TravelPersist    = TEXT("");
    GMpConn.LastReportedCheckpoint = TEXT("");
    GMpConn.OnlineCount      = 0;
    GMpConn.KnownPlayers.Empty();

    if (GRelayThread.IsRunning())
        GRelayThread.StopRelay();
}

UBOOL GMpConn_IsConnected()
{
    return GMpConn.bIsConnected;
}

void FMpConnection::ConnectP2P(QWORD InHostSteamID, WORD InRelayPort,
    const FString& InRoomCode, const FString& InPassword)
{
    bP2PMode    = TRUE;
    HostSteamID = InHostSteamID;
    RoomCode    = InRoomCode;
    Password    = InPassword;

    debugf(NAME_Log, TEXT("[P2P] ConnectP2P host=%llu port=%d room=%s"),
        (unsigned long long)InHostSteamID, (int)InRelayPort, *InRoomCode);

    // Self-connect fallback: if the host SteamID is our own, connect directly
    // to the local relay over UDP (Steam P2P doesn't deliver packets to self).
    UBOOL bSelf = FALSE;
    if (GSteamUser)
    {
        QWORD MySteamID = GSteamUser->GetSteamID().ConvertToUint64();
        bSelf = (MySteamID == InHostSteamID);
    }

    if (bSelf)
    {
        // Direct loopback to the embedded relay — no P2P bridge needed.
        debugf(NAME_Log, TEXT("[P2P] self-connect -> 127.0.0.1:%d"), (int)InRelayPort);
        IP      = TEXT("127.0.0.1");
        UdpPort = FString::Printf(TEXT("%d"), (int)InRelayPort);
    }
    else
    {
        if (GP2PBridgeClient.IsRunning())
            GP2PBridgeClient.StopBridge();

        // Start the client bridge (separate from the server bridge GP2PBridge).
        UBOOL bOk = GP2PBridgeClient.StartClient(InRelayPort, InHostSteamID);
        debugf(NAME_Log, TEXT("[P2P] StartClient port=%d result=%d"), (int)InRelayPort, (int)bOk);

        // Point FMpConnection at the bridge's loopback listen port.
        WORD ListenPort = GP2PBridgeClient.GetClientListenPort();
        debugf(NAME_Log, TEXT("[P2P] FMpConnection -> 127.0.0.1:%d"), (int)ListenPort);
        IP      = TEXT("127.0.0.1");
        UdpPort = FString::Printf(TEXT("%d"), (int)ListenPort);
    }

    // Reset and reconnect.
    bResolved    = FALSE;
    bIsConnected = FALSE;
    bCancelled   = FALSE;
    HelloAttempt = 0;
    HelloTimer   = 0.f;
    if (GResolveInfo) { delete GResolveInfo; GResolveInfo = NULL; }

    Connect();
}

#endif // WITH_UE3_NETWORKING
