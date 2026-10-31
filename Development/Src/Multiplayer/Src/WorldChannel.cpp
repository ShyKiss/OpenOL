#include "Multiplayer.h"
#include "HeroChannel.h"
#include "PushableChannel.h"
#include "WorldChannelPackets.h"
#include "OLUtilities.h"

// ============================================================================
// Internal helpers
// ============================================================================

static FORCEINLINE UBOOL CanWorldSend()
{
    return GMpConn.bIsConnected && GMpConn.bIsHandshaked;
}

static void SendWorldString(BYTE Channel, BYTE PktType, const FString& Str)
{
    // Encode as UTF-8; length byte is UTF-8 byte count (max 252 to stay under 255).
    BYTE Utf8[252];
    INT Utf8Len = TCHARToUTF8(Utf8, sizeof(Utf8), *Str);
    BYTE B[3 + 252];
    B[0] = Channel;
    B[1] = PktType;
    B[2] = (BYTE)Utf8Len;
    for (INT i = 0; i < Utf8Len; i++)
        B[3 + i] = Utf8[i];
    GMpConn.SendBinary(B, 3 + Utf8Len);
}

static void SendWorldIntString(BYTE Channel, BYTE PktType, INT Value, const FString& Str)
{
    BYTE Utf8[252];
    INT Utf8Len = TCHARToUTF8(Utf8, sizeof(Utf8), *Str);
    BYTE B[7 + 252];
    B[0] = Channel;
    B[1] = PktType;
    B[2] = (BYTE)(Value & 0xFF);
    B[3] = (BYTE)((Value >> 8) & 0xFF);
    B[4] = (BYTE)((Value >> 16) & 0xFF);
    B[5] = (BYTE)((Value >> 24) & 0xFF);
    B[6] = (BYTE)Utf8Len;
    for (INT i = 0; i < Utf8Len; i++)
        B[7 + i] = Utf8[i];
    GMpConn.SendBinary(B, 7 + Utf8Len);
}

// Decode [utf8Len(1)][UTF-8 bytes] from Data+Offset into OutStr. Returns false on underflow.
static UBOOL ReadWorldString(BYTE* Data, INT DataLen, INT Offset, FString& OutStr)
{
    if (Offset >= DataLen) return FALSE;
    INT Len = Data[Offset++];
    if (Offset + Len > DataLen) return FALSE;
    UTF8ToFString(Data + Offset, Len, OutStr);
    return TRUE;
}

// Fires all SeqEvent_RemoteEvent nodes whose EventName matches EventName.
static void TriggerRemoteKismetEvent(AMultiplayerController* Ctrl, FName EventName)
{
    USequence* GameSeq = GWorld->GetWorldInfo()->GetGameSequence();
    if (!GameSeq) return;

    TArray<USequenceObject*> AllEvents;
    GameSeq->FindSeqObjectsByClass(USeqEvent_RemoteEvent::StaticClass(), AllEvents, TRUE);
    for (INT i = 0; i < AllEvents.Num(); i++)
    {
        USeqEvent_RemoteEvent* Evt = Cast<USeqEvent_RemoteEvent>(AllEvents(i));
        if (Evt && Evt->EventName == EventName && Evt->bEnabled)
            Evt->CheckActivate(GWorld->GetWorldInfo(), NULL);
    }
}

// Fires Kismet pickup event for a pickable object found by path.
static AOLPickableObject* FindPickupNear(const FVector& Loc, FLOAT MaxDist = 200.f)
{
    FLOAT BestDist = MaxDist;
    AOLPickableObject* Best = NULL;
    for (FActorIterator It; It; ++It)
    {
        AOLPickableObject* P = Cast<AOLPickableObject>(*It);
        if (!P) continue;
        FLOAT D = FDist(P->Location, Loc);
        if (D < BestDist) { BestDist = D; Best = P; }
    }
    return Best;
}

static FVector ReadPickupLoc(BYTE* Data)
{
    INT IX = (INT)((DWORD)Data[0]|((DWORD)Data[1]<<8)|((DWORD)Data[2]<<16)|((DWORD)Data[3]<<24));
    INT IY = (INT)((DWORD)Data[4]|((DWORD)Data[5]<<8)|((DWORD)Data[6]<<16)|((DWORD)Data[7]<<24));
    INT IZ = (INT)((DWORD)Data[8]|((DWORD)Data[9]<<8)|((DWORD)Data[10]<<16)|((DWORD)Data[11]<<24));
    return FVector((FLOAT)IX, (FLOAT)IY, (FLOAT)IZ);
}

static void TriggerRemotePickupKismetEvent(AMultiplayerController* Ctrl, const FString& PickupPath)
{
    AOLPickableObject* Pickup = Cast<AOLPickableObject>(
        UObject::StaticFindObject(AOLPickableObject::StaticClass(), NULL, *PickupPath, FALSE));
    if (!Pickup) return;

    for (INT i = 0; i < Pickup->GeneratedEvents.Num(); i++)
    {
        UOLSeqEvent_Pickup* Evt = Cast<UOLSeqEvent_Pickup>(Pickup->GeneratedEvents(i));
        if (Evt && Evt->bEnabled)
            Evt->CheckActivate(Pickup, NULL);
    }
}

// Applies a remote touch trigger event by path.
// Returns true if any SeqAct_Interp reachable from Op is currently playing.
// Used to suppress duplicate remote triggers when multiple players fire the same event.
static UBOOL IsKismetChainPlaying(USequenceOp* Op, int Depth)
{
    if (!Op || Depth > 16) return FALSE;
    for (INT i = 0; i < Op->OutputLinks.Num(); i++)
    {
        for (INT j = 0; j < Op->OutputLinks(i).Links.Num(); j++)
        {
            USequenceOp* Next = Op->OutputLinks(i).Links(j).LinkedOp;
            if (!Next) continue;
            USeqAct_Interp* Interp = Cast<USeqAct_Interp>(Next);
            if (Interp && Interp->bIsPlaying)
                return TRUE;
            if (IsKismetChainPlaying(Next, Depth + 1))
                return TRUE;
        }
    }
    return FALSE;
}

// Fires the chain as an observer: visuals and scripted state, no gameplay side effects.
static void ApplyRemoteTrigger(AMultiplayerController* Ctrl, USeqEvent_Touch* TouchEvent)
{
    if (!TouchEvent->bEnabled) return;

    if (TouchEvent->MaxTriggerCount > 0 && TouchEvent->TriggerCount >= TouchEvent->MaxTriggerCount)
        return;

    // Repeatable triggers: the chain may still be playing from an earlier activation.
    if (TouchEvent->MaxTriggerCount == 0 && IsKismetChainPlaying(TouchEvent, 0))
        return;

    AOLPlayerController* OLPC = Cast<AOLPlayerController>(Ctrl);
    if (OLPC)
        OLPC->ForceActivateTouchEvent(TouchEvent);
}

static USeqEvent_Touch* FindTouchEvent(const FString& EventPath)
{
    return Cast<USeqEvent_Touch>(
        UObject::StaticFindObject(USeqEvent_Touch::StaticClass(), NULL, *EventPath, FALSE));
}

// The simulator holds triggers it cannot run yet back until its levels are in.
void SimNode_ApplyTrigger(AMultiplayerController* Ctrl, const FString& EventPath)
{
    USeqEvent_Touch* TouchEvent = FindTouchEvent(EventPath);
    if (TouchEvent)
        ApplyRemoteTrigger(Ctrl, TouchEvent);
    else
        debugf(NAME_Log, TEXT("[SimNode] Trigger not in this world: %s"), *EventPath);
}

// Set while activating an event the server has already accepted.
static UBOOL GApplyingTriggerCommit = FALSE;

// Called from USeqEvent_Touch::CheckTouchActivate. TRUE means the server decides: the
// event fires when WORLD_TRIGGER_FIRE comes back, so all players see one activation.
UBOOL MpWorld_InterceptTouch(USeqEvent_Touch* TouchEvent, AActor* Originator, AActor* Instigator)
{
    AMultiplayerController* Ctrl = GMultiplayerController;
    if (GApplyingTriggerCommit || !Ctrl || !Instigator || Instigator != GMultiplayerHero)
        return FALSE;
    if (!CanWorldSend() || GMpConn.WorldEpoch == 0 || !GMpConn.SyncMatinees)
        return FALSE;

    if (Originator)
    {
        const FString OriginatorPath = Originator->GetPathName().ToUpper();
        for (INT i = 0; i < Ctrl->TriggerActBlacklist.Num(); i++)
        {
            const FString& Filter = Ctrl->TriggerActBlacklist(i);
            if (Filter.Len() > 0 && OriginatorPath.InStr(Filter.ToUpper()) != -1)
                return FALSE;
        }
    }

    BYTE Utf8[252];
    const INT Utf8Len = TCHARToUTF8(Utf8, sizeof(Utf8), *TouchEvent->GetPathName());
    BYTE B[4 + 252];
    B[0] = CH_WORLD;
    B[1] = WORLD_TRIGGER_FIRE;
    B[2] = (BYTE)Clamp<INT>(TouchEvent->MaxTriggerCount, 0, 255);
    B[3] = (BYTE)Utf8Len;
    appMemcpy(B + 4, Utf8, Utf8Len);
    GMpConn.SendBinary(B, 4 + Utf8Len);
    return TRUE;
}

// ============================================================================
// Claims: CSAs and pickups wait for the server before anything happens locally
// ============================================================================

// The claim in flight, cleared by the answer or after a timeout if it never comes.
static UObject* GClaimPending      = NULL;
static DOUBLE   GClaimPendingUntil = 0.0;
// Set while re-running the interaction the server has just granted.
static UObject* GClaimGranted      = NULL;

// TRUE means the caller must not act on Object now.
static UBOOL ClaimOrWait(UObject* Object, BYTE Kind, INT MaxCount)
{
    if (GClaimGranted)
        return GClaimGranted != Object;
    if (!GMultiplayerController || !CanWorldSend() || GMpConn.WorldEpoch == 0)
        return FALSE;

    const DOUBLE Now = appSeconds();
    if (GClaimPending && Now < GClaimPendingUntil)
        return TRUE;

    BYTE Utf8[252];
    const INT Utf8Len = TCHARToUTF8(Utf8, sizeof(Utf8), *Object->GetPathName());
    BYTE B[5 + 252];
    B[0] = CH_WORLD;
    B[1] = WORLD_CLAIM;
    B[2] = Kind;
    B[3] = (BYTE)Clamp<INT>(MaxCount, 0, 255);
    B[4] = (BYTE)Utf8Len;
    appMemcpy(B + 5, Utf8, Utf8Len);
    GMpConn.SendBinary(B, 5 + Utf8Len);

    GClaimPending      = Object;
    GClaimPendingUntil = Now + 2.0;
    return TRUE;
}

// Called from AOLCSA::TryActivate just before it consumes anything.
UBOOL MpWorld_ClaimCSA(AOLCSA* CSA)
{
    if (!GMpConn.SyncMatinees)
        return FALSE;
    // Blacklisted CSAs stay local on every client.
    if (GMultiplayerController
        && GMultiplayerController->CSAActBlacklist.FindItemIndex(CSA->GetPathName()) != INDEX_NONE)
        return FALSE;
    return ClaimOrWait(CSA, CLAIM_CSA, CSA->MaxTriggerCount);
}

// Called from AOLHero::TryObjectPickup when the player presses use.
UBOOL MpWorld_ClaimPickup(AOLPickableObject* Pickup)
{
    if (!GMpConn.SyncPickups)
        return FALSE;
    return ClaimOrWait(Pickup, CLAIM_PICKUP, 1);
}

static void SendClaimRelease(BYTE Kind, UObject* Object, INT MaxCount)
{
    BYTE Utf8[252];
    const INT Utf8Len = TCHARToUTF8(Utf8, sizeof(Utf8), *Object->GetPathName());
    BYTE B[5 + 252];
    B[0] = CH_WORLD;
    B[1] = WORLD_CLAIM_RELEASE;
    B[2] = Kind;
    B[3] = (BYTE)Clamp<INT>(MaxCount, 0, 255);
    B[4] = (BYTE)Utf8Len;
    appMemcpy(B + 5, Utf8, Utf8Len);
    GMpConn.SendBinary(B, 5 + Utf8Len);
}

static void RunGrantedClaim(BYTE Kind, UObject* Object, UBOOL bCatchUp)
{
    AOLHero* Hero = GMultiplayerHero;
    UBOOL bStarted = FALSE;
    GClaimGranted = Object;
    if (Hero)
        bStarted = Kind == CLAIM_CSA ? Hero->TryCSA(TRUE) : Hero->TryObjectPickup(TRUE);
    GClaimGranted = NULL;

    // A catch-up used nothing on the server, so there is nothing to give back.
    if (!bStarted && !bCatchUp)
        SendClaimRelease(Kind, Object, Kind == CLAIM_CSA ? ((AOLCSA*)Object)->MaxTriggerCount : 1);
}

static UObject* FindClaimObject(BYTE Kind, const FString& Path)
{
    UClass* Class = Kind == CLAIM_CSA ? AOLCSA::StaticClass() : AOLPickableObject::StaticClass();
    return UObject::StaticFindObject(Class, NULL, *Path, FALSE);
}

// ============================================================================
// Send — pickup state
// ============================================================================

void UWorldChannel::SendPickupState(INT CurSMT)
{
    // Implemented in HeroChannel::SendSpecialMoveType where LastSentSpecialMove and HeroPawn are available.
}

// ============================================================================
// Send — pickup Kismet event
// ============================================================================

void UWorldChannel::SendPickupKismet(AOLPickableObject* Pickup)
{
    if (!CanWorldSend()) return;
    if (!GMpConn.SyncPickups || !GMpConn.SyncMatinees) return;
    if (!Pickup) return;

    SendWorldString(CH_WORLD, WORLD_PICKUP_KISMET, Pickup->GetPathName());
}

// ============================================================================
// ============================================================================
// Send — inventory item consumed
// ============================================================================

// Set while applying a consume from the network: it fires the same event as a local one.
static UBOOL GApplyingRemoteConsume = FALSE;

void UWorldChannel::SendItemConsume(FName ItemName)
{
    if (!CanWorldSend() || GApplyingRemoteConsume) return;
    if (!GMpConn.SyncInteractable) return;

    SendWorldString(CH_WORLD, WORLD_ITEM_CONSUME, ItemName.ToString());
}

// ============================================================================
// Send — active matinee state (position + playrate for all playing SeqAct_Interps)
// ============================================================================

void UWorldChannel::SendMatineeState()
{
    if (!CanWorldSend() || !GMpConn.SyncMatinees) return;
    if (!GWorld || !GWorld->GetGameSequence()) return;

    TArray<USequenceObject*> AllMatinees;
    GWorld->GetGameSequence()->FindSeqObjectsByClass(USeqAct_Interp::StaticClass(), AllMatinees, TRUE);

    // Build packet: [CH_WORLD][WORLD_MATINEE_STATE][count(1)] { [pathLen(1)][path ASCII][position f32][playRate f32] }
    BYTE B[OL_MAX_MSG - 4];  // SendBinary adds the 4-byte player_id
    INT  N = 0;
    B[N++] = CH_WORLD;
    B[N++] = WORLD_MATINEE_STATE;
    INT CountOffset = N++;  // placeholder for count
    BYTE Count = 0;

    for (INT i = 0; i < AllMatinees.Num() && Count < 32; i++)
    {
        USeqAct_Interp* Interp = Cast<USeqAct_Interp>(AllMatinees(i));
        if (!Interp || !Interp->bIsPlaying) continue;

        FString Path = Interp->GetPathName();
        INT PathLen = Min(Path.Len(), 127);
        if (N + 1 + PathLen + 4 + 4 > (INT)sizeof(B)) break;

        B[N++] = (BYTE)PathLen;
        for (INT c = 0; c < PathLen; c++)
            B[N++] = (BYTE)((*Path)[c] & 0x7F);

        FLOAT Pos      = Interp->Position;
        FLOAT PlayRate = Interp->PlayRate;
        appMemcpy(B + N, &Pos,      4); N += 4;
        appMemcpy(B + N, &PlayRate, 4); N += 4;
        Count++;
    }

    // Count 0 tells the server to forget the previous state.
    B[CountOffset] = Count;
    GMpConn.SendBinary(B, N);
}

// ============================================================================
// Send — recording marker
// ============================================================================

void UWorldChannel::SendRecordingMarker(AOLRecordingMarker* Marker)
{
    if (!Marker || !CanWorldSend()) return;
    if (!GMpConn.SyncPickups) return;

    SendWorldString(CH_WORLD, WORLD_RECORDING, Marker->GetPathName());
}

// ============================================================================
// OnPawnTouchedTrigger — local player touched a trigger; send to remote.
// ============================================================================

void UWorldChannel::OnPawnTouchedTrigger(AActor* TriggerActor)
{
    // Requests now go out from MpWorld_InterceptTouch. Still called from OLPawn.uc.
}

// ============================================================================
// Receive — nick / disconnect
// ============================================================================

// Update persistent nick registry in GMpConn.
static void UpdateKnownPlayer(INT PlayerID, const FString& Nick)
{
    for (INT i = 0; i < GMpConn.KnownPlayers.Num(); i++)
        if (GMpConn.KnownPlayers(i).PlayerID == PlayerID)
        {
            GMpConn.KnownPlayers(i).Username = Nick;
            return;
        }
    FMpConnection::FRemoteNick Entry;
    Entry.PlayerID = PlayerID;
    Entry.Username = Nick;
    GMpConn.KnownPlayers.AddItem(Entry);
}

static void RemoveKnownPlayer(INT PlayerID)
{
    for (INT i = GMpConn.KnownPlayers.Num() - 1; i >= 0; i--)
        if (GMpConn.KnownPlayers(i).PlayerID == PlayerID)
            GMpConn.KnownPlayers.Remove(i, 1);
}

static FString GetKnownNick(INT PlayerID)
{
    for (INT i = 0; i < GMpConn.KnownPlayers.Num(); i++)
        if (GMpConn.KnownPlayers(i).PlayerID == PlayerID)
            return GMpConn.KnownPlayers(i).Username;

    // Fallback: check RemotePlayers on the active controller.
    if (GMultiplayerController)
    {
        INT Idx = GMultiplayerController->FindRemoteIndex(PlayerID);
        if (Idx >= 0 && GMultiplayerController->RemotePlayers(Idx))
        {
            const FString& Nick = GMultiplayerController->RemotePlayers(Idx)->PlayerNick;
            if (Nick.Len() > 0)
                return Nick;
        }
    }
    return FString::Printf(TEXT("Player%d"), PlayerID);
}

void UWorldChannel::OnBinaryNick(INT SenderID, const FString& Nick)
{
    if (SenderID == GMpConn.LocalPlayerID) return;

    UpdateKnownPlayer(SenderID, Nick);

    INT Idx = ControllerOwner->FindRemoteIndex(SenderID);
    if (Idx < 0)
    {
        ControllerOwner->RegisterRemotePlayer(SenderID, Nick);

        // Respond with our nick
        if (CanWorldSend())
        {
            FString OurNick = GMpConn.Username.Len() > 0
                ? FString(GMpConn.Username) : FString(TEXT("Player"));
            SendWorldString(CH_HERO, HERO_NICK, OurNick);
        }
    }
    else
    {
        ControllerOwner->RemotePlayers(Idx)->PlayerNick = Nick;
    }
}

void UWorldChannel::OnDisconnected(const TArray<FString>& Parts, INT SenderID)
{
    FString Name = GetKnownNick(SenderID);
    RemoveKnownPlayer(SenderID);

    HUD_AddNotification(Cast<AMultiplayerHUD>(ControllerOwner->myHUD), Name + TEXT(" disconnected"));

    ControllerOwner->eventOnPlayerDisconnected(SenderID);
}

// ============================================================================
// Receive — binary world packets
// ============================================================================

void UWorldChannel::OnBinaryWorldPacket(INT SenderID, BYTE PktType, BYTE* Data, INT DataLen)
{
    FString Str;
    switch (PktType)
    {
    case WORLD_TRIGGER_FIRE:
        if (DataLen >= 3 && ReadWorldString(Data, DataLen, 1, Str))
        {
            USeqEvent_Touch* TouchEvent = FindTouchEvent(Str);
            if (GIsSimNode && SimNode_DeferTrigger(TouchEvent, Str)) break;
            if (!TouchEvent) break;
            if (SenderID != GMpConn.LocalPlayerID)
            {
                ApplyRemoteTrigger(ControllerOwner, TouchEvent);
                break;
            }
            // Our own request was accepted: this is the real activation.
            if (GMultiplayerHero)
            {
                GApplyingTriggerCommit = TRUE;
                TouchEvent->CheckTouchActivate(TouchEvent->Originator, GMultiplayerHero, FALSE);
                GApplyingTriggerCommit = FALSE;
            }
        }
        break;

    case WORLD_TRIGGER_DENIED:
        if (ReadWorldString(Data, DataLen, 0, Str))
        {
            // The commit of a winning player arrives before this, so a local count below
            // the limit means our Kismet was rolled back (checkpoint reload, late join).
            // For this client it is the first run: activate fully, only here.
            USeqEvent_Touch* TouchEvent = FindTouchEvent(Str);
            if (TouchEvent && GMultiplayerHero && TouchEvent->MaxTriggerCount > 0
                && TouchEvent->TriggerCount < TouchEvent->MaxTriggerCount)
            {
                GApplyingTriggerCommit = TRUE;
                TouchEvent->CheckTouchActivate(TouchEvent->Originator, GMultiplayerHero, FALSE);
                GApplyingTriggerCommit = FALSE;
            }
        }
        break;

    case WORLD_CLAIM:
        if (DataLen >= 4 && ReadWorldString(Data, DataLen, 2, Str))
        {
            const BYTE Kind = Data[0];
            UObject* Object = FindClaimObject(Kind, Str);
            if (SenderID == GMpConn.LocalPlayerID)
            {
                GClaimPending = NULL;
                if (Object) RunGrantedClaim(Kind, Object, FALSE);
                break;
            }
            // Someone else got it. A CSA must look used here before our own refusal arrives,
            // or the refusal would read as "our Kismet was rolled back".
            AOLCSA* CSA = Kind == CLAIM_CSA ? Cast<AOLCSA>(Object) : NULL;
            if (CSA)
                CSA->TriggerCount = Max(CSA->TriggerCount, (INT)Data[DataLen - 1]);
        }
        break;

    case WORLD_CLAIM_RELEASE:
        if (DataLen >= 4 && SenderID != GMpConn.LocalPlayerID && ReadWorldString(Data, DataLen, 2, Str))
        {
            AOLCSA* CSA = Data[0] == CLAIM_CSA ? Cast<AOLCSA>(FindClaimObject(CLAIM_CSA, Str)) : NULL;
            if (CSA)
                CSA->TriggerCount = (INT)Data[DataLen - 1];
        }
        break;

    case WORLD_CLAIM_DENIED:
        if (DataLen >= 2 && ReadWorldString(Data, DataLen, 1, Str))
        {
            GClaimPending = NULL;
            // Same catch-up as triggers: a local count below the limit means this client
            // was rolled back, and for it the CSA is a first run. Pickups stay refused:
            // the snapshot already hid the object and restored its item.
            AOLCSA* CSA = Data[0] == CLAIM_CSA ? Cast<AOLCSA>(FindClaimObject(CLAIM_CSA, Str)) : NULL;
            if (CSA && CSA->MaxTriggerCount > 0 && CSA->TriggerCount < CSA->MaxTriggerCount)
                RunGrantedClaim(CLAIM_CSA, CSA, TRUE);
        }
        break;

    case WORLD_ITEM_CONSUME:
        if (ReadWorldString(Data, DataLen, 0, Str) && GMpConn.SyncInteractable)
        {
            AOLHero* Hero = Cast<AOLHero>(ControllerOwner->Pawn);
            if (Hero && Hero->OLPC && Hero->OLPC->InventoryManager)
            {
                GApplyingRemoteConsume = TRUE;
                Hero->OLPC->InventoryManager->ConsumeItem(FName(*Str));
                GApplyingRemoteConsume = FALSE;
            }
        }
        break;

    case WORLD_PICKUP_KISMET:
        if (ReadWorldString(Data, DataLen, 0, Str) && GMpConn.SyncPickups && GMpConn.SyncMatinees)
            TriggerRemotePickupKismetEvent(ControllerOwner, Str);
        break;

    case WORLD_RECORDING:
        if (ReadWorldString(Data, DataLen, 0, Str))
        {
            AOLRecordingMarker* Marker = Cast<AOLRecordingMarker>(
                UObject::StaticFindObject(AOLRecordingMarker::StaticClass(), NULL, *Str, FALSE));
            if (Marker && !Marker->bRecorded)
            {
                AOLPlayerController* OLPC = Cast<AOLPlayerController>(ControllerOwner);
                if (OLPC) OLPC->NativeApplyRemoteRecording(Marker);
            }
        }
        break;

    case WORLD_PICKUP_START:
        if (DataLen >= 12 && GMpConn.SyncPickups)
        {
            // Store pickup location on the remote player slot for ATTACH lookup
            INT Idx = ControllerOwner->FindRemoteIndex(SenderID);
            if (Idx != -1)
                ControllerOwner->RemotePlayers(Idx)->LastRemotePickupLoc = ReadPickupLoc(Data);
        }
        break;

    case WORLD_PICKUP_ATTACH:
        if (DataLen >= 12 && GMpConn.SyncPickups)
        {
            INT Idx = ControllerOwner->FindRemoteIndex(SenderID);
            if (Idx != -1)
            {
                AOLHero* Dummy = Cast<AOLHero>(ControllerOwner->RemotePlayers(Idx)->DummyPlayer);
                AOLPickableObject* Best = FindPickupNear(ReadPickupLoc(Data));
                if (Dummy && Best)
                {
                    Dummy->AttachPickupMeshToDummyHand(Best);
                    TriggerRemotePickupKismetEvent(ControllerOwner, Best->GetPathName());
                }
            }
        }
        break;

    case WORLD_MATINEE_STATE:
        if (DataLen >= 1 && GMpConn.bInBaseline && GMpConn.SyncMatinees && GWorld && GWorld->GetGameSequence())
        {
            INT o = 0;
            BYTE Count = Data[o++];
            TArray<USequenceObject*> AllMatinees;
            GWorld->GetGameSequence()->FindSeqObjectsByClass(USeqAct_Interp::StaticClass(), AllMatinees, TRUE);

            for (BYTE ci = 0; ci < Count && o < DataLen; ci++)
            {
                INT PathLen = Data[o++];
                if (o + PathLen + 8 > DataLen) break;

                FString Path(TEXT(""));
                for (INT c = 0; c < PathLen; c++)
                    Path += (TCHAR)(Data[o + c] & 0x7F);
                o += PathLen;

                FLOAT Pos, PlayRate;
                appMemcpy(&Pos,      Data + o, 4); o += 4;
                appMemcpy(&PlayRate, Data + o, 4); o += 4;

                // Find matching SeqAct_Interp by path suffix (full PathName may vary by level)
                for (INT mi = 0; mi < AllMatinees.Num(); mi++)
                {
                    USeqAct_Interp* Interp = Cast<USeqAct_Interp>(AllMatinees(mi));
                    if (!Interp) continue;
                    if (Interp->GetPathName() != Path) continue;

                    // Apply position only if not locally playing (avoid fighting local matinees)
                    if (!Interp->bIsPlaying)
                    {
                        Interp->ForceStartPosition = Pos;
                        Interp->bForceStartPos     = TRUE;
                        Interp->PlayRate           = PlayRate;
                    }
                    break;
                }
            }
        }
        break;

    case WORLD_PICKUP_STATE:
        if (DataLen >= 12 && GMpConn.SyncPickups)
        {
            AOLPickableObject* Best = FindPickupNear(ReadPickupLoc(Data));
            if (Best && !Best->bUsed)
            {
                Best->bUsed = TRUE;
                if (Best->PickupMesh)
                    Best->PickupMesh->SetHiddenGame(TRUE);

                AOLPlayerController* OLPC = Cast<AOLPlayerController>(ControllerOwner);
                if (OLPC && OLPC->InventoryManager)
                {
                    AOLCollectiblePickup* Doc = Cast<AOLCollectiblePickup>(Best);
                    AOLGameplayItemPickup* Item = Doc ? NULL : Cast<AOLGameplayItemPickup>(Best);
                    if (Doc)
                    {
                        OLPC->InventoryManager->AddCollectible(Doc->CollectibleName);
                        if (OLPC->myHUD)
                            Cast<AOLHUD>(OLPC->myHUD)->SetLatestDocument(Doc->CollectibleName);
                    }
                    else if (Item)
                    {
                        OLPC->InventoryManager->AddUniqueItem(Item->ItemName);
                    }
                }
            }
        }
        break;

    default: break;
    }
}
