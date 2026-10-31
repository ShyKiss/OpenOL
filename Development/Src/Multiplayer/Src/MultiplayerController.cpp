#include "Multiplayer.h"
#include "OLGameClasses.h"
#include "OpenOLGlobals.h"
#include "HeroChannel.h"
#include "OLUtilities.h"
#include "HeroChannelPackets.h"
#include "DoorChannel.h"
#include "EnemyChannel.h"
#include "EnemyChannelPackets.h"
#include "PushableChannel.h"
#include "WorldChannelPackets.h"
#include "ServerPackets.h"
#include "MultiplayerHUD.h"

extern FMpConnectionTicker* GMpTicker;


// ============================================================================
// World session
// ============================================================================

static void PutWorldStr(BYTE* B, INT& N, const FString& Str)
{
    const INT Len = Min(Str.Len(), 127);
    B[N++] = (BYTE)Len;
    for (INT i = 0; i < Len; i++)
        B[N++] = (BYTE)((*Str)[i] & 0x7F);
}

// Returns the offset after the string, or -1 if it does not fit.
static INT ReadWorldStr(const BYTE* Data, INT DataLen, INT Off, FString& Out)
{
    if (Off >= DataLen) return -1;
    const INT Len = Data[Off++];
    if (Off + Len > DataLen) return -1;
    Out = TEXT("");
    for (INT i = 0; i < Len; i++)
        Out += (TCHAR)(Data[Off + i] & 0x7F);
    return Off + Len;
}

// The persistent level identifies the world: after PrepareMapChange GWorld keeps its
// old package name, so only the checkpoint knows which map we are really on.
static FString PersistentLevelOf(const FString& CheckpointName)
{
    AOLCheckpoint* CP = CheckpointName.Len() > 0 ? Utils::GetCheckpointFromName(FName(*CheckpointName)) : NULL;
    if (CP && CP->PersistentLevelName != NAME_None)
        return CP->PersistentLevelName.ToString();
    return GWorld ? GWorld->GetOutermost()->GetName() : FString();
}

static FString CurrentCheckpointName()
{
    const FName CP = GWorld && GWorld->GetGameInfo() ? Utils::GetCurrentCheckpointName() : NAME_None;
    return CP != NAME_None ? CP.ToString() : FString();
}

static void SendSessionEnter(BYTE Flags, const FString& Checkpoint)
{
    BYTE B[5 + 2 * 128];
    INT N = 0;
    B[N++] = CH_SESS;
    B[N++] = SESS_ENTER;
    B[N++] = (BYTE)(GMpConn.WorldEpoch & 0xFF);
    B[N++] = (BYTE)(GMpConn.WorldEpoch >> 8);
    B[N++] = Flags;
    PutWorldStr(B, N, Checkpoint);
    PutWorldStr(B, N, PersistentLevelOf(Checkpoint));
    GMpConn.SendBinary(B, N);
    GMpConn.LastReportedCheckpoint = Checkpoint;
}

// Called from AOLPlayerController::StartNewGameAtCheckpoint.
void MpSession_OnStartAtCheckpoint(const FString& Checkpoint)
{
    // The connection outlives the multiplayer game: single player must not touch the room.
    if (!GMultiplayerController || !GMpConn.bIsConnected || !GMpConn.bIsHandshaked)
        return;
    if (GMpConn.TravelCheckpoint == Checkpoint)
    {
        // Following SESS_TRAVEL: the respawn reports SESS_ENTER, nothing is being replaced.
        GMpConn.TravelCheckpoint = TEXT("");
        return;
    }
    SendSessionEnter(SESS_ENTER_RESET, Checkpoint);
    GMpConn.bSkipNextEnter = TRUE;
}

// Called from UOLEngine::SaveCheckpoint.
void MpSession_OnCheckpointReached(FName Checkpoint)
{
    if (!GMultiplayerController || !GMpConn.bIsConnected || !GMpConn.bIsHandshaked || GMpConn.WorldEpoch == 0)
        return;
    const FString Name = Checkpoint.ToString();
    if (Name == GMpConn.LastReportedCheckpoint)
        return;
    GMpConn.LastReportedCheckpoint = Name;

    BYTE B[2 + 2 * 128];
    INT N = 0;
    B[N++] = CH_SESS;
    B[N++] = SESS_CHECKPOINT;
    PutWorldStr(B, N, Name);
    PutWorldStr(B, N, PersistentLevelOf(Name));
    GMpConn.SendBinary(B, N);
}

// Same steps as the checkpoint tab: cross-map travel goes through PrepareMapChange,
// PlayerTick starts the checkpoint once its actor exists.
static UBOOL ApplyTravel()
{
    AOLGame* Game = GWorld ? Cast<AOLGame>(GWorld->GetGameInfo()) : NULL;
    if (!Game)
        return FALSE;

    GMpConn.bTravelPending = FALSE;
    const FString Checkpoint = GMpConn.TravelCheckpoint;
    const FString Persist    = GMpConn.TravelPersist;
    debugf(NAME_Log, TEXT("[MP] Travel: cp='%s' persist='%s' epoch=%d"), *Checkpoint, *Persist, (INT)GMpConn.WorldEpoch);

    if (Checkpoint.Len() == 0)
    {
        if (GWorld->GetOutermost()->GetName() != Persist)
            GEngine->Exec(*FString::Printf(TEXT("open %s?game=Multiplayer.MultiplayerGame"), *Persist));
        else if (GMultiplayerHero)
            SendSessionEnter(0, Checkpoint);
        return TRUE;
    }
    // A player may stay where it is, but the simulator's enemies belong to the old world.
    if (!GIsSimNode && Checkpoint == CurrentCheckpointName() && Persist == PersistentLevelOf(Checkpoint))
    {
        GMpConn.TravelCheckpoint = TEXT("");
        SendSessionEnter(0, Checkpoint);
        return TRUE;
    }

    Game->PendingCheckpointName = FName(*Checkpoint);
    for (FActorIterator It; It; ++It)
    {
        AOLCheckpoint* CP = Cast<AOLCheckpoint>(*It);
        if (CP && !CP->IsPendingKill() && CP->CheckpointName == Game->PendingCheckpointName)
            return TRUE;
    }
    UGameEngine* GameEngine = Cast<UGameEngine>(GEngine);
    if (GameEngine)
    {
        TArray<FName> LevelNames;
        LevelNames.AddItem(FName(*Persist));
        GameEngine->PrepareMapChange(LevelNames);
        GameEngine->bShouldCommitPendingMapChange = TRUE;
    }
    return TRUE;
}

static void HandleSessionPacket(AMultiplayerController* Controller, BYTE Type, BYTE* Payload, INT PayloadLen)
{
    switch (Type)
    {
    case SESS_WORLD:
    case SESS_TRAVEL:
    {
        if (PayloadLen < 2) return;
        FString Checkpoint, Persist;
        INT Off = ReadWorldStr(Payload, PayloadLen, 2, Checkpoint);
        if (Off < 0 || ReadWorldStr(Payload, PayloadLen, Off, Persist) < 0) return;
        GMpConn.WorldEpoch = (WORD)(Payload[0] | (Payload[1] << 8));
        if (Type == SESS_WORLD)
        {
            debugf(NAME_Log, TEXT("[MP] In world: cp='%s' persist='%s' epoch=%d"), *Checkpoint, *Persist, (INT)GMpConn.WorldEpoch);
            GMpConn.bInBaseline = TRUE;
            break;
        }
        extern void DoorChannel_ResetSession();
        extern void EnemyChannel_ResetSession();
        DoorChannel_ResetSession();
        EnemyChannel_ResetSession();
        GMpConn.TravelCheckpoint = Checkpoint;
        GMpConn.TravelPersist    = Persist;
        GMpConn.bTravelPending   = TRUE;
        ApplyTravel();
        break;
    }
    case SESS_BASELINE_END:
        debugf(NAME_Log, TEXT("[MP] Baseline received"));
        GMpConn.bInBaseline = FALSE;
        break;
    case SESS_SIM_OWNER:
        if (PayloadLen >= 4)
        {
            extern void EnemyChannel_OnSimOwnerChanged(AMultiplayerController*, INT);
            EnemyChannel_OnSimOwnerChanged(Controller,
                (INT)((DWORD)Payload[0] | ((DWORD)Payload[1]<<8) | ((DWORD)Payload[2]<<16) | ((DWORD)Payload[3]<<24)));
        }
        break;
    case SESS_PLAYER_LEFT:
        if (PayloadLen >= 4)
            Controller->eventOnPlayerDisconnected((INT)((DWORD)Payload[0] | ((DWORD)Payload[1]<<8) | ((DWORD)Payload[2]<<16) | ((DWORD)Payload[3]<<24)));
        break;
    default: break;
    }
}

void AMultiplayerController::NativeInit()
{
    debugf(TEXT("[MP] NativeInit"));
    GMultiplayerController            = this;
    GMultiplayerHero                  = NULL;
    GHeroChannelTicker.Channel        = HeroChannel;
    GHeroChannelReceiveTicker.Channel = HeroChannel;

    if (!GMpTicker)
    {
        GMpTicker = new FMpConnectionTicker();
        atexit([]() { GMpConn.Disconnect(); });
    }

    // LoadConfig() runs once and checks cmdline for -openol_host (Steam cold-launch).
    // Do it before Connect() so we can take the P2P branch if needed.
    // Skip if bP2PMode or bHostMode — caller already set the connection params.
    if (!GMpConn.bP2PMode && !GMpConn.bHostMode)
        GMpConn.LoadConfig();

    // Override username from Steam — Steamworks is guaranteed alive by NativeInit time,
    // whereas LoadConfig() may run before Steam is ready (GSteamFriends == NULL then).
    {
        char SteamName[128] = {0};
        GetSteamPersonaName(SteamName, sizeof(SteamName));
        debugf(NAME_Log, TEXT("[MP] Steam persona: '%s'"), UTF8_TO_TCHAR(SteamName));
        if (SteamName[0])
            GMpConn.Username = FString(UTF8_TO_TCHAR(SteamName));
        debugf(NAME_Log, TEXT("[MP] Username set to: '%s'"), *GMpConn.Username);
    }

    // Cold-launch P2P join: skip DNS connect, go straight to P2P bridge.
    if (GMpConn.bP2PColdLaunch)
    {
        GMpConn.bP2PColdLaunch = FALSE;
        FString HostID   = GMpConn.P2PColdLaunchHost;
        FString RoomCode = GMpConn.P2PColdLaunchRoom;
        WORD    Port     = (WORD)appAtoi(*GMpConn.UdpPort);
        debugf(NAME_Log, TEXT("[MP] P2P cold-launch: host=%s port=%d room='%s'"), *HostID, (int)Port, *RoomCode);
        QWORD SteamID = (QWORD)appAtoi64(*HostID);
        if (SteamID != 0)
            GMpConn.ConnectP2P(SteamID, Port, RoomCode, TEXT(""));
        return;
    }

    GMpConn.Connect();

    if (GMpConn.bIsConnected)
    {
        ServerName  = GMpConn.ServerName;
        OnlineCount = GMpConn.OnlineCount;

        for (INT i = 0; i < GMpConn.KnownPlayers.Num(); i++)
        {
            const FMpConnection::FRemoteNick& KP = GMpConn.KnownPlayers(i);
            if (KP.PlayerID != GMpConn.LocalPlayerID)
                RegisterRemotePlayer(KP.PlayerID, KP.Username);
        }
    }
}

void AMultiplayerController::NativeDestroyed()
{
    if (GMpConn.bIsConnected)
    {
        BYTE B[2] = { CH_SESS, SESS_LEAVE };
        GMpConn.SendBinary(B, 2);
    }
    GMpConn.WorldEpoch = 0;
    extern void DoorChannel_ResetSession();
    extern void EnemyChannel_ResetSession();
    DoorChannel_ResetSession();
    EnemyChannel_ResetSession();
    extern APlayerController* GHeadlessController;
    if (GHeadlessController == this)
        GHeadlessController = NULL;
    GMultiplayerController            = NULL;
    GMultiplayerHero                  = NULL;
    GHeroChannelTicker.Channel        = NULL;
    GHeroChannelReceiveTicker.Channel = NULL;
    HeroChannel     = NULL;
    DoorChannel     = NULL;
    PushableChannel = NULL;
    EnemyChannel    = NULL;
    WorldChannel    = NULL;
}

void AMultiplayerController::NativeSetHero(AOLHero* Hero)
{
    GMultiplayerHero = Hero;
    if (Hero && GIsSimNode)
        SimNode_OnHeroSpawned(Hero);
    if (!Hero || !IsConnected())
        return;
    if (GMpConn.bTravelPending && ApplyTravel())
        return;
    if (GMpConn.bSkipNextEnter)
        GMpConn.bSkipNextEnter = FALSE;
    else
        SendSessionEnter(0, CurrentCheckpointName());
}

UBOOL AMultiplayerController::IsConnected()
{
    return GMpConn.bIsConnected && GMpConn.bIsHandshaked;
}

UBOOL AMultiplayerController::IsReady()
{
    return GMultiplayerHero != NULL && GMpConn.bIsConnected && GMpConn.bIsHandshaked;
}

FString AMultiplayerController::NativeGetUsername()
{
    return GMpConn.Username.Len() > 0 ? FString(GMpConn.Username) : FString(TEXT("Player"));
}


INT AMultiplayerController::FindRemoteIndex(INT PlayerID)
{
    for (INT i = 0; i < RemotePlayers.Num(); i++)
        if (RemotePlayers(i) && RemotePlayers(i)->PlayerID == PlayerID)
            return i;
    return -1;
}

// ============================================================================
// Connection events
// ============================================================================

void AMultiplayerController::OnConnected()
{
    GMpConn.bIsHandshaked = TRUE;
    if (HeroChannel)
    {
        HeroChannel->LastSentSpecialMove   = -1;
        HeroChannel->LastSentMeshPreset    = -1;
        HeroChannel->LastSentCinematicAnim = TEXT("");
        HeroChannel->bSendingJumpGroundZ   = FALSE;
    }
    GMpConn.PingTimer = 0.5f;

    // Send binary HELLO: Data=[CH_SRV][SRV_HELLO][nick_len(1)][nick ASCII...]
    // SendBinary inserts player_id after byte[1], so wire is:
    //   [CH_SRV][SRV_HELLO][player_id LE4][nick_len(1)][nick ASCII...]
    FString Nick = GMpConn.Username.Len() > 0
        ? FString(GMpConn.Username)
        : FString(TEXT("Player"));
    INT NickLen = Min(Nick.Len(), 32);

    BYTE B[3 + 32];
    INT N = 0;
    N = PutU8(B, N, CH_SRV);
    N = PutU8(B, N, SRV_HELLO);
    N = PutU8(B, N, (BYTE)NickLen);
    for (INT i = 0; i < NickLen; i++)
        B[N++] = (BYTE)((*Nick)[i] & 0x7F);
    GMpConn.SendBinary(B, N);

    GMpConn.WorldEpoch = 0;
    GMpConn.SimOwnerID = 0;
    GMpConn.bSkipNextEnter = FALSE;
    // The simulator goes to the world the server names rather than reporting its own.
    if (GMultiplayerHero && !GIsSimNode)
        SendSessionEnter(0, CurrentCheckpointName());
}

void AMultiplayerController::OnDisconnected()
{
    GMpConn.bIsHandshaked = FALSE;
    GMpConn.OnlineCount = 0;
    GMpConn.KnownPlayers.Empty();
    CurrentPingMs = 0.f;
    LastPongTime  = 0.f;
    // Clean up all remote players
    for (INT i = RemotePlayers.Num() - 1; i >= 0; i--)
    {
        if (RemotePlayers(i))
            eventOnPlayerDisconnected(RemotePlayers(i)->PlayerID);
    }

    HUD_AddNotification(Cast<AMultiplayerHUD>(myHUD),
        FString::Printf(TEXT("[%s] Disconnected. Reconnecting..."), *GMpConn.ServerName));
}

void AMultiplayerController::NativeRemoveRemotePlayer(INT PlayerID)
{
    INT Idx = FindRemoteIndex(PlayerID);
    if (Idx == -1)
        return;

    URemotePlayer* P = RemotePlayers(Idx);

    // Release any door this player was holding.
    INT LockedIdx = P->LockedDoorIdx;
    if (LockedIdx != -1 && LockedIdx < CachedDoors.Num())
    {
        AOLDoor* D = Cast<AOLDoor>(CachedDoors(LockedIdx));
        if (D && D->DoorUser == NULL &&
            (D->DoorState == DS_PlayerInteracting ||
             D->DoorState == DS_Opening ||
             D->DoorState == DS_Closing))
            D->DoorState = DS_Idle;
        if (LockedIdx < RemoteDoorLockExpiry.Num())
            RemoteDoorLockExpiry(LockedIdx) = 0.f;
    }

    // Destroy the dummy hero pawn.
    if (P->DummyPlayer)
    {
        GWorld->DestroyActor(P->DummyPlayer);
        P->DummyPlayer = NULL;
    }

    // Destroy all dummy enemies that belonged to this player and remove from the list.
    for (INT j = RemoteEnemies.Num() - 1; j >= 0; j--)
    {
        if (RemoteEnemies(j).OwnerID == PlayerID)
        {
            if (RemoteEnemies(j).DummyEnemy)
                GWorld->DestroyActor(RemoteEnemies(j).DummyEnemy);
            RemoteEnemies.Remove(j, 1);
        }
    }

    RemotePlayers.Remove(Idx, 1);
}

void AMultiplayerController::NativeDestroyRemoteEnemies()
{
    // Destroy all remote enemy dummy actors and clear the array.
    for (INT i = 0; i < RemoteEnemies.Num(); i++)
    {
        if (RemoteEnemies(i).DummyEnemy)
            GWorld->DestroyActor(RemoteEnemies(i).DummyEnemy);
    }
    RemoteEnemies.Empty();
}

// ============================================================================
// Text packet routing
// ============================================================================

void AMultiplayerController::OnReceiveData(const FString& Data)
{
    // All packets are binary now; text path is unused.
    (void)Data;
}

// ============================================================================
// Binary packet routing
// ============================================================================

// Called from MultiplayerLink with:
//   For SRV packets (0xE0+): PktType=type, SenderID=0, Data=payload, DataLen=payloadLen
//   For game packets:        PktType=channel, SenderID=sender, Data=[type(1)][payload...], DataLen=1+payloadLen
void AMultiplayerController::OnReceiveBinaryData(BYTE PktType, INT SenderID, BYTE* Data, INT DataLen)
{
    // --- Server→client packets (PktType >= 0xE0) ---
    if (PktType >= 0xE0)
    {
        switch (PktType)
        {
        case SRV_READY:
            ServerName  = GMpConn.ServerName;
            OnlineCount = GMpConn.OnlineCount;
            break;
        case SRV_ONLINE_COUNT:
            if (DataLen >= 4)
            {
                INT Count = (INT)((DWORD)Data[0] | ((DWORD)Data[1]<<8) | ((DWORD)Data[2]<<16) | ((DWORD)Data[3]<<24));
                GMpConn.OnlineCount = Max(Count, 1);
                OnlineCount = GMpConn.OnlineCount;
            }
            break;
        case SRV_HELLO_FAIL:
            break;
        case SRV_DISCONNECT:
            if (DataLen >= 4)
            {
                INT PID = (INT)((DWORD)Data[0] | ((DWORD)Data[1]<<8) | ((DWORD)Data[2]<<16) | ((DWORD)Data[3]<<24));
                if (WorldChannel)
                {
                    TArray<FString> Parts;
                    Parts.AddItem(FString::Printf(TEXT("%d"), PID));
                    Parts.AddItem(TEXT("DISCONNECT"));
                    WorldChannel->OnDisconnected(Parts, PID);
                }
            }
            break;
        case SRV_DOOR_DENY:
            if (DataLen >= 2)
            {
                BYTE KLen = Data[0];
                INT  KBytes = Min((INT)KLen, DataLen - 1);
                TCHAR TmpKey[256] = {0};
                for (INT i = 0; i < KBytes && i < 255; i++)
                    TmpKey[i] = (TCHAR)Data[1 + i];
                if (DoorChannel)
                {
                    TArray<FString> KParts;
                    FString(TmpKey).ParseIntoArray(&KParts, TEXT(","), TRUE);
                    if (KParts.Num() >= 3)
                        DoorChannel->OnDoorDeny(appAtoi(*KParts(0)), appAtoi(*KParts(1)), appAtoi(*KParts(2)));
                }
            }
            break;
        default: break;
        }
        return;
    }

    // --- Game packets: PktType=channel, Data[0]=type, Data[1..]=payload ---
    if (DataLen < 1) return;
    BYTE Channel = PktType;
    BYTE Type    = Data[0];
    BYTE* Payload    = Data + 1;
    INT   PayloadLen = DataLen - 1;

    switch (Channel)
    {
    case CH_HERO:
        switch (Type)
        {
        case HERO_STATE:          HeroChannel->OnBinaryLoc(SenderID, Payload, PayloadLen);            break;
        case HERO_NICK:
        {
            if (WorldChannel)
            {
                FString NickStr;
                // Payload: [utf8Len(1)][UTF-8 bytes]
                if (PayloadLen >= 1)
                {
                    INT Utf8Len = Min((INT)Payload[0], PayloadLen - 1);
                    UTF8ToFString(Payload + 1, Utf8Len, NickStr);
                }
                WorldChannel->OnBinaryNick(SenderID, NickStr);
            }
            break;
        }
        case HERO_HEAD_ROT:       HeroChannel->OnBinaryHeadRot(SenderID, Payload, PayloadLen);        break;
        case HERO_MESH_PRESET:    HeroChannel->OnBinaryMesh(SenderID, Payload, PayloadLen);           break;
        case HERO_CINEMATIC_ANIM: HeroChannel->OnBinaryCinematicAnim(SenderID, Payload, PayloadLen);  break;
        case HERO_SMT_TYPE:       HeroChannel->OnBinarySmtType(SenderID, Payload, PayloadLen);        break;
        case HERO_PLAYER_EVENT:   HeroChannel->OnBinaryPlayerEvent(SenderID, Payload, PayloadLen);    break;
        case HERO_PLAYER_LIFECYCLE: HeroChannel->OnBinaryPlayerLifecycle(SenderID, Payload, PayloadLen); break;
        default: break;
        }
        break;

    case CH_DOOR:
        if (DoorChannel) DoorChannel->OnBinaryPacket(SenderID, Type, Payload, PayloadLen);
        break;

    case CH_ENEMY:
        if (Type == ENEMY_LOC)
        {
            if (EnemyChannel) EnemyChannel->OnBinaryLoc(SenderID, Payload, PayloadLen);
        }
        else
        {
            if (EnemyChannel) EnemyChannel->OnBinaryPacket(SenderID, Type, Payload, PayloadLen);
        }
        break;

    case CH_PUSH:
        if (Type == PUSH_STATE)
            PushableChannel_OnBinaryPushState(PushableChannel, SenderID, Payload, PayloadLen);
        else if (Type == PUSH_DENIED)
            PushableChannel_OnBinaryPushDenied(PushableChannel, Payload, PayloadLen);
        break;

    case CH_WORLD:
        if (WorldChannel) WorldChannel->OnBinaryWorldPacket(SenderID, Type, Payload, PayloadLen);
        break;

    case CH_SESS:
        HandleSessionPacket(this, Type, Payload, PayloadLen);
        break;

    default: break;
    }
}

INT AMultiplayerController::RegisterRemotePlayer(INT PlayerID, const FString& Nick)
{
    URemotePlayer* P = ConstructObject<URemotePlayer>(URemotePlayer::StaticClass(), this);
    P->PlayerID         = PlayerID;
    P->LockedDoorIdx    = -1;
    P->LastRemoteHealth = 100;
    P->DummyPlayer      = SpawnDummy(this);
    AOLHero* Dummy = Cast<AOLHero>(P->DummyPlayer);
    if (Dummy) Dummy->DummyOwnerID = PlayerID;

    FString NickStr = Nick.Len() > 0 ? Nick : FString::Printf(TEXT("Player%d"), PlayerID);
    P->PlayerNick = NickStr;

    RemotePlayers.AddItem(P);


    return RemotePlayers.Num() - 1;
}

UBOOL AMultiplayerController::IndexDoors()
{
    TArray<AActor*> NewDoors;
    for (FActorIterator It; It; ++It)
    {
        AOLDoor* D = Cast<AOLDoor>(*It);
        if (D && !D->bDeleteMe && !D->bPendingDelete)
            NewDoors.AddItem(D);
    }

    TArray<FLOAT> NewAngles, NewExpiry;
    NewAngles.AddZeroed(NewDoors.Num());
    NewExpiry.AddZeroed(NewDoors.Num());

    UBOOL bAnyNew = FALSE;
    for (INT i = 0; i < NewDoors.Num(); i++)
    {
        UBOOL bFound = FALSE;
        for (INT j = 0; j < CachedDoors.Num(); j++)
        {
            if (CachedDoors(j) == NewDoors(i))
            {
                NewAngles(i) = LastSentDoorAngle(j);
                NewExpiry(i) = RemoteDoorLockExpiry(j);
                bFound = TRUE;
                break;
            }
        }
        if (!bFound)
        {
            NewAngles(i) = -9999.0f;
            bAnyNew = TRUE;
        }
    }

    CachedDoors          = NewDoors;
    LastSentDoorAngle    = NewAngles;
    RemoteDoorLockExpiry = NewExpiry;
    bDoorsIndexed        = TRUE;
    return bAnyNew;
}

// ============================================================================
// Remote player management — previously in UC
// ============================================================================

void AMultiplayerController::RemoveRemotePlayer(INT PlayerID)
{
    INT Idx = FindRemoteIndex(PlayerID);
    if (Idx < 0)
        return;

    URemotePlayer* P = RemotePlayers(Idx);
    INT LockedIdx = P->LockedDoorIdx;
    if (LockedIdx >= 0 && LockedIdx < CachedDoors.Num())
    {
        AOLDoor* D = Cast<AOLDoor>(CachedDoors(LockedIdx));
        if (D && !D->DoorUser &&
            (D->DoorState == DS_PlayerInteracting || D->DoorState == DS_Opening || D->DoorState == DS_Closing))
            D->DoorState = DS_Idle;
    }

    if (P->DummyPlayer && GWorld)
        GWorld->DestroyActor(P->DummyPlayer);

    for (INT j = RemoteEnemies.Num() - 1; j >= 0; j--)
    {
        if (RemoteEnemies(j).OwnerID == P->PlayerID)
        {
            if (RemoteEnemies(j).DummyEnemy && GWorld)
                GWorld->DestroyActor(RemoteEnemies(j).DummyEnemy);
            RemoteEnemies.Remove(j, 1);
        }
    }

    RemotePlayers.Remove(Idx, 1);
}

void AMultiplayerController::DestroyRemoteEnemies()
{
    if (GWorld)
        for (INT i = 0; i < RemoteEnemies.Num(); i++)
            if (RemoteEnemies(i).DummyEnemy)
                GWorld->DestroyActor(RemoteEnemies(i).DummyEnemy);
    RemoteEnemies.Empty();
}

// ============================================================================
// Game code callbacks
// ============================================================================

void AMultiplayerController::NotifyDummyPlayerHit(AOLHero* DummyTarget, FLOAT Damage, FLOAT KnockbackPower, FVector HitDir)
{
    if (DummyTarget && HeroChannel)
        HeroChannel->SendPlayerHit(DummyTarget->DummyOwnerID, Damage, KnockbackPower, HitDir);
}

void AMultiplayerController::NotifyDummyPlayerGrab(INT TargetPlayerID, FVector GrabTargetLoc, FVector CharDir, UBOOL bCrouched, INT EnemyTypeInt, FLOAT BlendAlpha, UBOOL bLeftAnim, INT GrabType)
{
    if (HeroChannel)
        HeroChannel->SendPlayerGrab(TargetPlayerID, GrabTargetLoc, CharDir, bCrouched, EnemyTypeInt, BlendAlpha, bLeftAnim, GrabType);
}

void AMultiplayerController::NotifyDummyPlayerThrow(INT TargetPlayerID, FLOAT ThrowRotation)
{
    if (HeroChannel)
        HeroChannel->SendPlayerThrow(TargetPlayerID, ThrowRotation);
}

void AMultiplayerController::NotifyDummyPlayerKill(INT TargetPlayerID, INT EnemyTypeInt, INT WeaponType, UBOOL bBackAnim, UBOOL bLeftAnim, FLOAT BlendAlpha, FVector AnimStart, FVector CharDir, INT KillType, INT VictimYaw)
{
    if (HeroChannel)
        HeroChannel->SendPlayerKill(TargetPlayerID, EnemyTypeInt, WeaponType, bBackAnim, bLeftAnim, BlendAlpha, AnimStart, CharDir, KillType, VictimYaw);
}

void AMultiplayerController::NotifyDummyEnemySMT(AOLEnemyPawn* Enemy, INT SMTType, INT Param1, INT Param2)
{
    if (Enemy && EnemyChannel)
        EnemyChannel->SendSMTDirect(Enemy, SMTType, Param1, Param2);
}

void AMultiplayerController::NativeNotifyEnemyDoorOpen(AOLEnemyPawn* Enemy, AOLDoor* D, FLOAT Speed, FLOAT Angle)
{
    debugf(TEXT("### NativeNotifyEnemyDoorOpen: Enemy=%s Door=%s Speed=%.1f Angle=%.1f Chan=%s"),
        Enemy ? *Enemy->GetName() : TEXT("NULL"),
        D ? *D->GetName() : TEXT("NULL"),
        Speed, Angle,
        EnemyChannel ? TEXT("OK") : TEXT("NULL"));
    if (Enemy && D && EnemyChannel)
        EnemyChannel->SendEnemyDoorOpen(Enemy, D, Speed, Angle);
}

void AMultiplayerController::NativeNotifyEnemyDoorDone(AOLEnemyPawn* Enemy, AOLDoor* D, FLOAT CloseSpeed)
{
    debugf(TEXT("### NativeNotifyEnemyDoorDone: Enemy=%s Door=%s Speed=%.1f Chan=%s"),
        Enemy ? *Enemy->GetName() : TEXT("NULL"),
        D ? *D->GetName() : TEXT("NULL"),
        CloseSpeed,
        EnemyChannel ? TEXT("OK") : TEXT("NULL"));
    if (Enemy && D && EnemyChannel)
        EnemyChannel->SendEnemyDoorDone(Enemy, D, CloseSpeed);
}

void AMultiplayerController::NotifyEnemyDoorBash(AOLEnemyPawn* Enemy, AOLDoor* D, UBOOL bReversed)
{
    debugf(TEXT("### NotifyEnemyDoorBash: Enemy=%s Door=%s EnemyChannel=%s"),
        Enemy ? *Enemy->GetName() : TEXT("NULL"),
        D ? *D->GetName() : TEXT("NULL"),
        EnemyChannel ? TEXT("OK") : TEXT("NULL"));
    if (Enemy && D && EnemyChannel)
        EnemyChannel->SendEnemyDoorBash(Enemy, D, bReversed);
}

void AMultiplayerController::NotifyEnemyDoorBreak(AOLEnemyPawn* Enemy, AOLDoor* D, UBOOL bReversed)
{
    debugf(TEXT("### NotifyEnemyDoorBreak: Enemy=%s Door=%s EnemyChannel=%s"),
        Enemy ? *Enemy->GetName() : TEXT("NULL"),
        D ? *D->GetName() : TEXT("NULL"),
        EnemyChannel ? TEXT("OK") : TEXT("NULL"));
    if (Enemy && D && EnemyChannel)
        EnemyChannel->SendEnemyDoorBreak(Enemy, D, bReversed);
}

void AMultiplayerController::NotifyPawnTouchedTrigger(AActor* TriggerActor)
{
    if (WorldChannel)
        WorldChannel->OnPawnTouchedTrigger(TriggerActor);
}

void AMultiplayerController::OnInventoryItemConsumed(FName ItemName)
{
    if (WorldChannel)
        WorldChannel->SendItemConsume(ItemName);
}

void AMultiplayerController::OnPickupKismetEvent(AOLPickableObject* Pickup)
{
    if (WorldChannel)
        WorldChannel->SendPickupKismet(Pickup);
}

void AMultiplayerController::OnLocalDoorOpen(AOLDoor* D)
{
    if (DoorChannel)
        DoorChannel->OnLocalDoorOpen(D);
}

void AMultiplayerController::OnLocalDoorClose(AOLDoor* D)
{
    if (DoorChannel)
        DoorChannel->OnLocalDoorClose(D);
}

void AMultiplayerController::OnRecordingMarkerCompleted(AOLRecordingMarker* Marker)
{
    if (WorldChannel)
        WorldChannel->SendRecordingMarker(Marker);
}

void AMultiplayerController::OnToggleCinematicMode(USeqAct_ToggleCinematicMode* Action)
{
    if (Role < ROLE_Authority) return;
    UBOOL bNewCinematicMode;
    if      (Action->InputLinks(0).bHasImpulse) bNewCinematicMode = TRUE;
    else if (Action->InputLinks(1).bHasImpulse) bNewCinematicMode = FALSE;
    else                                         bNewCinematicMode = !bCinematicMode;
    // bObserverOnly means the matinee only affects observers — skip without mutating the flag,
    // so the real OLPlayerController still sees the original value when it processes the action.
    if (Action->bObserverOnly) return;
    eventSetCinematicMode(Action, bNewCinematicMode, Action->bHidePlayer, Action->bHideHUD,
        Action->bDisableMovement, Action->bDisableTurning, Action->bDisableInput);
}

void AMultiplayerController::InterpolationStarted(USeqAct_Interp* InterpAction, UInterpGroupInst* GroupInst)
{
    if (WorldChannel && GMpConn.bIsConnected)
        WorldChannel->SendMatineeState();
}

void AMultiplayerController::InterpolationFinished(USeqAct_Interp* InterpAction)
{
    if (WorldChannel && GMpConn.bIsConnected)
        WorldChannel->SendMatineeState();
}
