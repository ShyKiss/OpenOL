#include "Multiplayer.h"
#include "HeroChannel.h"
#include "DoorChannel.h"
#include "DoorChannelPackets.h"

// Per-session delta state — reset when DoorChannel is re-created
static AOLDoor*       GLastLockedDoor   = NULL;
static AOLHidingSpot* GLastActiveLocker = NULL;

// ============================================================================
// Helpers
// ============================================================================

static AOLDoor* FindIndexedDoor(AMultiplayerController* TC, INT X, INT Y, INT Z, INT& OutDoorIdx)
{
    if (!TC->bDoorsIndexed)
        TC->IndexDoors();

    for (INT i = 0; i < TC->CachedDoors.Num(); i++)
    {
        AOLDoor* D = Cast<AOLDoor>(TC->CachedDoors(i));
        if (D && appRound(D->Location.X) == X && appRound(D->Location.Y) == Y && appRound(D->Location.Z) == Z)
        {
            OutDoorIdx = i;
            return D;
        }
    }
    // Not found — re-index once and retry
    TC->IndexDoors();
    for (INT i = 0; i < TC->CachedDoors.Num(); i++)
    {
        AOLDoor* D = Cast<AOLDoor>(TC->CachedDoors(i));
        if (D && appRound(D->Location.X) == X && appRound(D->Location.Y) == Y && appRound(D->Location.Z) == Z)
        {
            OutDoorIdx = i;
            return D;
        }
    }
    OutDoorIdx = -1;
    return NULL;
}

static void StorePendingDoor(AMultiplayerController* TC, INT X, INT Y, INT Z, FLOAT Angle)
{
    for (INT i = 0; i < TC->PendingDoorStates.Num(); i++)
    {
        if (TC->PendingDoorStates(i).KeyX == X && TC->PendingDoorStates(i).KeyY == Y && TC->PendingDoorStates(i).KeyZ == Z)
        {
            TC->PendingDoorStates(i).Angle = Angle;
            return;
        }
    }
    FPendingDoorState S;
    S.KeyX = X; S.KeyY = Y; S.KeyZ = Z; S.Angle = Angle;
    TC->PendingDoorStates.AddItem(S);
}

// ============================================================================
// Binary send helpers
// ============================================================================

static void SendDoorState(INT X, INT Y, INT Z, FLOAT Angle, FLOAT Speed)
{
    BYTE B[DOOR_STATE_SIZE];
    INT N = 0;
    N = PutU8 (B, N, CH_DOOR);
    N = PutU8 (B, N, DOOR_STATE);
    N = PutI32(B, N, X);
    N = PutI32(B, N, Y);
    N = PutI32(B, N, Z);
    N = PutI32(B, N, appRound(Angle * 1000.0f));
    N = PutI32(B, N, appRound(Speed * 1000.0f));
    GMpConn.SendBinary(B, N);
}

// DOOR_INIT: same layout as DOOR_ANGLE but server only stores if no snapshot exists yet.
static void SendDoorInit(INT X, INT Y, INT Z, FLOAT Angle)
{
    BYTE B[DOOR_ANGLE_SIZE];
    INT N = 0;
    N = PutU8 (B, N, CH_DOOR);
    N = PutU8 (B, N, DOOR_INIT);
    N = PutI32(B, N, X);
    N = PutI32(B, N, Y);
    N = PutI32(B, N, Z);
    N = PutI32(B, N, appRound(Angle * 1000.0f));
    GMpConn.SendBinary(B, N);
}

static void SendDoorUnlock(INT X, INT Y, INT Z)
{
    BYTE B[DOOR_UNLOCK_SIZE];
    INT N = 0;
    N = PutU8 (B, N, CH_DOOR);
    N = PutU8 (B, N, DOOR_UNLOCK);
    N = PutI32(B, N, X);
    N = PutI32(B, N, Y);
    N = PutI32(B, N, Z);
    GMpConn.SendBinary(B, N);
}

// ============================================================================
// Free functions called from MultiplayerController
// ============================================================================

void DoorChannel_OnDoorDeny(UDoorChannel* DC, INT X, INT Y, INT Z)
{
    AMultiplayerController* TC = DC->ControllerOwner;
    if (!TC) return;

    INT DoorIdx = -1;
    AOLDoor* D = FindIndexedDoor(TC, X, Y, Z, DoorIdx);

    if (D)
        D->bNetDrivenMove = TRUE;

    // If we were holding this door, release it immediately
    if (GLastLockedDoor == D)
    {
        DoorChannel_SendUnlock(DC, D);
        GLastLockedDoor = NULL;
    }

    // Cancel local hero's door interaction (CancelSpecialMove is protected in AOLHero, public in AOLPawn)
    if (GMultiplayerHero)
        static_cast<AOLPawn*>(GMultiplayerHero)->CancelSpecialMove();
}

void DoorChannel_SendUnlock(UDoorChannel* DC, AOLDoor* D)
{
    if (!D || !GMpConn.bIsConnected) return;

    INT X = appRound(D->Location.X);
    INT Y = appRound(D->Location.Y);
    INT Z = appRound(D->Location.Z);

    SendDoorState(X, Y, Z,
        D->GetOpenAngle(),
        GMultiplayerHero ? 0.0f : 0.0f);  // speed 0 on release

    SendDoorUnlock(X, Y, Z);
}

// ============================================================================
// UC → C++ implementations
// ============================================================================

// --- TickSend ---

void UDoorChannel::TickSend(FLOAT DeltaTime)
{
    if (!ControllerOwner || !GMpConn.bIsConnected || !HeroPawn)
        return;

    // ---- Door angle send ----
    if (!ControllerOwner->bDoorsIndexed)
        ControllerOwner->IndexDoors();
    if (GMpConn.SyncInteractable)
    {
        for (INT i = 0; i < ControllerOwner->CachedDoors.Num(); i++)
        {
            AOLDoor* D = Cast<AOLDoor>(ControllerOwner->CachedDoors(i));
            if (!D || D->bNetDrivenMove || D->bRemoteKismetDriven)
                continue;
            if (D->DoorState != DS_PlayerInteracting || D->DoorUser != HeroPawn)
                continue;

            FLOAT Angle = D->GetOpenAngle();
            if (Abs(Angle - ControllerOwner->LastSentDoorAngle(i)) > 0.5f)
            {
                FLOAT AngleSpeed = (DeltaTime > 0.0f) ? (Abs(Angle - ControllerOwner->LastSentDoorAngle(i)) / DeltaTime) : 0.0f;
                ControllerOwner->LastSentDoorAngle(i) = Angle;
                SendDoorState(
                    appRound(D->Location.X), appRound(D->Location.Y), appRound(D->Location.Z),
                    Angle, AngleSpeed);
            }
        }
    }

    // ---- Door lock/unlock tracking ----
    {
        AOLDoor* D = GMpConn.SyncInteractable ? HeroPawn->ActiveDoor : NULL;
        UBOOL bInteracting = D && (D->DoorState == DS_PlayerInteracting || (INT)HeroPawn->SpecialMove == 28);
        if (bInteracting && D != GLastLockedDoor)
        {
            if (GLastLockedDoor)
                DoorChannel_SendUnlock(this, GLastLockedDoor);

            D->bNetDrivenMove = FALSE;

            INT  X = appRound(D->Location.X);
            INT  Y = appRound(D->Location.Y);
            INT  Z = appRound(D->Location.Z);

            BYTE B[DOOR_LOCK_SIZE];
            INT  N = 0;
            N = PutU8 (B, N, CH_DOOR);
            N = PutU8 (B, N, DOOR_LOCK);
            N = PutI32(B, N, X);
            N = PutI32(B, N, Y);
            N = PutI32(B, N, Z);
            N = PutU8 (B, N, (BYTE)HeroPawn->DoorOpeningType);
            N = PutI32(B, N, appRound(HeroPawn->LastGrabPos.X * 10.0f));
            N = PutI32(B, N, appRound(HeroPawn->LastGrabPos.Y * 10.0f));
            N = PutI32(B, N, appRound(HeroPawn->LastGrabPos.Z * 10.0f));
            N = PutI32(B, N, appRound(HeroPawn->LastGrabDir.X * 10000.0f));
            N = PutI32(B, N, appRound(HeroPawn->LastGrabDir.Y * 10000.0f));
            N = PutI32(B, N, appRound(HeroPawn->LastGrabDir.Z * 10000.0f));
            N = PutU8 (B, N, (BYTE)HeroPawn->DoorPartialOpenType);
            N = PutU8 (B, N, (BYTE)HeroPawn->DoorClosingType);
            N = PutU8 (B, N, HeroPawn->bQuietDoorInteraction ? 1 : 0);
            GMpConn.SendBinary(B, N);

            // Reset delta so first angle packet goes out immediately
            INT DoorIdx = -1;
            FindIndexedDoor(ControllerOwner, X, Y, Z, DoorIdx);
            if (DoorIdx != -1 && DoorIdx < ControllerOwner->LastSentDoorAngle.Num())
                ControllerOwner->LastSentDoorAngle(DoorIdx) = -999.0f;

            GLastLockedDoor = D;
        }
        else if (!bInteracting && GLastLockedDoor)
        {
            DoorChannel_SendUnlock(this, GLastLockedDoor);
            GLastLockedDoor = NULL;
        }
    }

    // ---- Locker state send ----
    {
        if (HeroPawn->ActiveLocker != GLastActiveLocker)
        {
            if (GLastActiveLocker && GLastActiveLocker->AssociatedDoor)
            {
                // Remote side needs angle=0 when we leave the locker
                AOLDoor* LD = GLastActiveLocker->AssociatedDoor;
                SendDoorState(
                    appRound(LD->Location.X), appRound(LD->Location.Y), appRound(LD->Location.Z),
                    0.0f, 0.0f);
            }
            GLastActiveLocker = HeroPawn->ActiveLocker;
        }
    }
}

// --- OnLocalDoorOpen / OnLocalDoorClose ---

void UDoorChannel::OnLocalDoorOpen(AOLDoor* D)
{
    if (!D || !GMpConn.bIsConnected) return;
    BYTE B[DOOR_OPEN_SIZE];
    INT  N = 0;
    N = PutU8 (B, N, CH_DOOR);
    N = PutU8 (B, N, DOOR_OPEN);
    N = PutI32(B, N, appRound(D->Location.X));
    N = PutI32(B, N, appRound(D->Location.Y));
    N = PutI32(B, N, appRound(D->Location.Z));
    GMpConn.SendBinary(B, N);
}

void UDoorChannel::OnLocalDoorClose(AOLDoor* D)
{
    if (!D || !GMpConn.bIsConnected) return;
    BYTE B[DOOR_CLOSE_SIZE];
    INT  N = 0;
    N = PutU8 (B, N, CH_DOOR);
    N = PutU8 (B, N, DOOR_CLOSE);
    N = PutI32(B, N, appRound(D->Location.X));
    N = PutI32(B, N, appRound(D->Location.Y));
    N = PutI32(B, N, appRound(D->Location.Z));
    GMpConn.SendBinary(B, N);
}

// --- Receive (binary) ---
// All On* functions are now called from OnReceiveBinaryData in MultiplayerController.
void UDoorChannel::OnDoorDeny(INT X, INT Y, INT Z)
{
    DoorChannel_OnDoorDeny(this, X, Y, Z);
}

// --- Binary receive dispatch ---

void UDoorChannel::OnBinaryPacket(INT SenderID, BYTE PktType, BYTE* Data, INT DataLen)
{
    AMultiplayerController* TC = ControllerOwner;
    if (!TC) return;

    switch (PktType)
    {
    case DOOR_LOCK:
    {
        if (DataLen < (INT)sizeof(FDoorLockPacket)) return;
        const FDoorLockPacket* P = (const FDoorLockPacket*)Data;
        INT DoorIdx = -1;
        AOLDoor* D = FindIndexedDoor(TC, P->X, P->Y, P->Z, DoorIdx);
        if (DoorIdx == -1) return;

        if (GMpConn.SyncInteractable && D && D->DoorUser == NULL
            && (D->DoorState == DS_Idle || D->DoorState == DS_Opening || D->DoorState == DS_Closing))
        {
            D->bNetDrivenMove = TRUE;
            D->NetReplicateInteractStart();
        }

        INT Idx = TC->FindRemoteIndex(SenderID);
        if (Idx != -1)
        {
            URemotePlayer* RP       = TC->RemotePlayers(Idx);
            RP->LockedDoorIdx                = DoorIdx;
            RP->LastRemoteDoorOpeningType    = P->OpeningType;
            RP->LastRemoteDoorPartialOpenType= P->PartialOpenType;
            RP->LastRemoteDoorClosingType    = P->ClosingType;
            RP->bLastRemoteDoorQuiet         = P->bQuiet != 0;

            FVector HandlePos(P->GrabPosX / 10.0f, P->GrabPosY / 10.0f, P->GrabPosZ / 10.0f);
            FVector HandleDir(P->GrabDirX / 10000.0f, P->GrabDirY / 10000.0f, P->GrabDirZ / 10000.0f);
            RP->LastRemoteDoorHandlePos = HandlePos;

            AOLHero* Dummy = Cast<AOLHero>(RP->DummyPlayer);
            if (Dummy && !HandlePos.IsNearlyZero())
            {
                RP->LastReceivedLoc = Dummy->Location;
                Dummy->SetDummyDoorOpeningType(P->OpeningType);
                Dummy->SetDummyActiveDoor(D);
                Dummy->StartSpecialMove((ESpecialMoveType)28, HandlePos, HandleDir);
            }
        }
        if (DoorIdx != -1)
            TC->RemoteDoorLockExpiry(DoorIdx) = 0.0f;
        break;
    }
    case DOOR_UNLOCK:
    {
        if (DataLen < (INT)sizeof(FDoorUnlockPacket)) return;
        const FDoorUnlockPacket* P = (const FDoorUnlockPacket*)Data;
        INT DoorIdx = -1;
        AOLDoor* D = FindIndexedDoor(TC, P->X, P->Y, P->Z, DoorIdx);

        if (DoorIdx != -1)
        {
            if (D) D->bNetDrivenMove = FALSE;
            FLOAT Now = (GWorld && GWorld->GetWorldInfo()) ? GWorld->GetWorldInfo()->TimeSeconds : 0.0f;
            TC->RemoteDoorLockExpiry(DoorIdx) = Now + 0.5f;
        }
        INT Idx = TC->FindRemoteIndex(SenderID);
        if (Idx != -1)
        {
            TC->RemotePlayers(Idx)->LockedDoorIdx = -1;
            AOLHero* Dummy = Cast<AOLHero>(TC->RemotePlayers(Idx)->DummyPlayer);
            if (Dummy) Dummy->SetDummyActiveDoor(NULL);
        }
        break;
    }
    case DOOR_STATE:
    {
        if (DataLen < (INT)sizeof(FDoorStatePacket)) return;
        const FDoorStatePacket* P = (const FDoorStatePacket*)Data;
        FLOAT Angle = P->AngleX1000 / 1000.0f;
        FLOAT Speed = P->SpeedX1000 / 1000.0f;
        INT DoorIdx = -1;
        AOLDoor* D = FindIndexedDoor(TC, P->X, P->Y, P->Z, DoorIdx);

        if (GMpConn.SyncInteractable && D && D->MaxOpenAngle > 0 && D->DoorState == DS_PlayerInteracting)
            D->SetNetInteractiveAngle(Angle, Speed);

        INT RemoteIdx = TC->FindRemoteIndex(SenderID);
        if (RemoteIdx != -1)
        {
            AOLHero* Dummy = Cast<AOLHero>(TC->RemotePlayers(RemoteIdx)->DummyPlayer);
            if (Dummy && D && D->MaxOpenAngle > 0)
                Dummy->SetDoorAnimRatio(Angle / D->MaxOpenAngle,
                    TC->RemotePlayers(RemoteIdx)->LastRemoteDoorOpeningType);
        }
        break;
    }
    case DOOR_OPEN:
    {
        if (DataLen < (INT)sizeof(FDoorOpenPacket) || !GMpConn.SyncInteractable) return;
        const FDoorOpenPacket* P = (const FDoorOpenPacket*)Data;
        INT DoorIdx = -1;
        AOLDoor* D = FindIndexedDoor(TC, P->X, P->Y, P->Z, DoorIdx);
        if (!D) return;

        AOLHero* DummyUser = Cast<AOLHero>(D->DoorUser);
        if ((D->DoorUser == NULL || (DummyUser && DummyUser->bIsDummyPawn))
            && (D->DoorState == DS_Idle || D->DoorState == DS_PlayerInteracting))
        {
            D->bNetDrivenMove = TRUE;
            D->NetReplicateOpen(D->OpeningSpeed);
        }
        break;
    }
    case DOOR_CLOSE:
    {
        if (DataLen < (INT)sizeof(FDoorClosePacket) || !GMpConn.SyncInteractable) return;
        const FDoorClosePacket* P = (const FDoorClosePacket*)Data;
        INT DoorIdx = -1;
        AOLDoor* D = FindIndexedDoor(TC, P->X, P->Y, P->Z, DoorIdx);
        if (!D) return;

        AOLHero* DummyUser = Cast<AOLHero>(D->DoorUser);
        if ((D->DoorUser == NULL || (DummyUser && DummyUser->bIsDummyPawn))
            && (D->DoorState == DS_Idle || D->DoorState == DS_PlayerInteracting))
        {
            D->bNetDrivenMove = TRUE;
            D->NetReplicateClose(D->ClosingSpeed);
        }
        break;
    }
    case DOOR_ANGLE:
    {
        if (DataLen < (INT)sizeof(FDoorAnglePacket)) return;
        const FDoorAnglePacket* P = (const FDoorAnglePacket*)Data;
        FLOAT Angle = P->AngleX1000 / 1000.0f;
        INT DoorIdx = -1;
        AOLDoor* D = FindIndexedDoor(TC, P->X, P->Y, P->Z, DoorIdx);

        if (DoorIdx != -1)
        {
            if (GMpConn.SyncInteractable && D && D->MaxOpenAngle > 0 && D->DoorUser == NULL)
                D->SetNetTargetOpenRatio(Angle / D->MaxOpenAngle);
        }
        else
        {
            StorePendingDoor(TC, P->X, P->Y, P->Z, Angle);
        }
        break;
    }
    case DOOR_PARAMS:
    {
        if (DataLen < (INT)sizeof(FDoorParamsPacket)) return;
        const FDoorParamsPacket* P = (const FDoorParamsPacket*)Data;
        INT Idx = TC->FindRemoteIndex(SenderID);
        if (Idx != -1)
        {
            URemotePlayer* RP            = TC->RemotePlayers(Idx);
            RP->LastRemoteDoorOpeningType     = P->OpeningType;
            RP->LastRemoteDoorPartialOpenType = P->PartialOpenType;
            RP->LastRemoteDoorClosingType     = P->ClosingType;
            RP->bLastRemoteDoorQuiet          = P->bQuiet != 0;
        }
        break;
    }
    default:
        break;
    }
}

// --- BroadcastDoorStates ---

void UDoorChannel::BroadcastDoorStates(const FString& LevelFilter)
{
    AMultiplayerController* TC = ControllerOwner;
    // HeroPawn may be null on level load (called from OnLevelBecameVisible before pawn spawns)
    if (!TC || !GMpConn.bIsConnected) return;

    if (!TC->bDoorsIndexed)
        TC->IndexDoors();

    for (INT i = 0; i < TC->CachedDoors.Num(); i++)
    {
        AOLDoor* D = Cast<AOLDoor>(TC->CachedDoors(i));
        if (!D) continue;
        if (LevelFilter.Len() > 0 && FString(D->GetOutermost()->GetName()) != LevelFilter)
            continue;

        FLOAT Angle = D->GetOpenAngle();

        // Send DOOR_INIT for every door — server stores only if no snapshot exists yet
        // (first-write-wins). Never relay to other clients, so their state is not disturbed.
        // The joining client receives correct state via REQUEST_DOORS after this broadcast.
        SendDoorInit(appRound(D->Location.X), appRound(D->Location.Y), appRound(D->Location.Z), Angle);
    }
}
