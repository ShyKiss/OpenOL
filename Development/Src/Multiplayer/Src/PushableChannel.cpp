#include "Multiplayer.h"
#include "PushableChannel.h"
#include "HeroChannel.h"    // PutI32 / ReadI32 helpers

// ============================================================================
// IndexPushables — fills CachedPushables, preserves LastSentPushDisplacement.
// Returns TRUE if any new pushable was discovered.
// ============================================================================

UBOOL AMultiplayerController::IndexPushables()
{
    TArray<AOLPushableObject*> NewPushables;
    for (FActorIterator It; It; ++It)
    {
        AOLPushableObject* P = Cast<AOLPushableObject>(*It);
        if (P && !P->bDeleteMe && !P->bPendingDelete)
            NewPushables.AddItem(P);
    }

    TArray<FLOAT> NewDisp;
    NewDisp.AddZeroed(NewPushables.Num());

    UBOOL bAnyNew = FALSE;
    for (INT i = 0; i < NewPushables.Num(); i++)
    {
        UBOOL bFound = FALSE;
        for (INT j = 0; j < CachedPushables.Num(); j++)
        {
            if (CachedPushables(j) == NewPushables(i))
            {
                NewDisp(i) = LastSentPushDisplacement(j);
                bFound = TRUE;
                break;
            }
        }
        if (!bFound)
            bAnyNew = TRUE;
    }

    CachedPushables          = NewPushables;
    LastSentPushDisplacement = NewDisp;
    bPushablesIndexed        = TRUE;
    return bAnyNew;
}

// ============================================================================
// ApplyPendingPushStates — flush states received before actors were loaded.
// ============================================================================

void AMultiplayerController::ApplyPendingPushStates()
{
    for (INT i = PendingPushStates.Num() - 1; i >= 0; i--)
    {
        AOLPushableObject* P = PushableChannel
            ? PushableChannel->FindPushableByKey(PendingPushStates(i).KeyX, PendingPushStates(i).KeyY, PendingPushStates(i).KeyZ)
            : NULL;
        if (P)
        {
            if (!P->bPlayerLocked)
            {
                P->SetNetDisplacement(PendingPushStates(i).Displacement);
                P->UpdateLinkedDoorState();
            }
            PendingPushStates.Remove(i, 1);
        }
    }
}

// ============================================================================
// FindPushableByKey
// ============================================================================

AOLPushableObject* UPushableChannel::FindPushableByKey(INT KeyX, INT KeyY, INT KeyZ)
{
    if (!ControllerOwner) return NULL;

    auto Search = [&]() -> AOLPushableObject*
    {
        for (INT i = 0; i < ControllerOwner->CachedPushables.Num(); i++)
        {
            AOLPushableObject* P = ControllerOwner->CachedPushables(i);
            if (P && (INT)P->Location.X == KeyX && (INT)P->Location.Y == KeyY && (INT)P->Location.Z == KeyZ)
                return P;
        }
        return NULL;
    };

    AOLPushableObject* P = Search();
    if (P) return P;
    ControllerOwner->IndexPushables();
    return Search();
}

// ============================================================================
// TickSend — send displacement of the locally active pushable every tick.
// ============================================================================

static void SendPushPacket(AOLPushableObject* P, FLOAT Disp, uint8_t bPushing)
{
    BYTE B[2 + sizeof(FPushStatePacket)];
    INT  N = 0;
    N = PutU8(B, N, CH_PUSH);
    N = PutU8(B, N, PUSH_STATE);
    FPushStatePacket Pkt;
    appMemzero(&Pkt, sizeof(Pkt));
    Pkt.KeyX      = (int32_t)P->Location.X;
    Pkt.KeyY      = (int32_t)P->Location.Y;
    Pkt.KeyZ      = (int32_t)P->Location.Z;
    Pkt.DispX1000 = (int32_t)appRound(Disp * 1000.0f);
    Pkt.Seq       = (uint32_t)++P->LocalPushSeq;
    Pkt.bPushing  = bPushing;
    appMemcpy(B + N, &Pkt, sizeof(Pkt));
    N += sizeof(Pkt);
    GMpConn.SendBinary(B, N);
}

void UPushableChannel::TickSend(FLOAT DeltaTime)
{
    if (!GMpConn.bIsConnected || !GMpConn.SyncInteractable)
        return;

    AOLHero* Hero = HeroPawn ? Cast<AOLHero>(HeroPawn) : NULL;
    AOLPushableObject* AP = Hero ? Hero->ActivePushable : NULL;

    if (!ControllerOwner->bPushablesIndexed)
        ControllerOwner->IndexPushables();

    // If hero is dead/absent: flush any pending stop-bursts, then reset state.
    if (!Hero)
    {
        for (INT i = 0; i < ControllerOwner->CachedPushables.Num(); i++)
        {
            AOLPushableObject* P = ControllerOwner->CachedPushables(i);
            if (!P) continue;
            // If we were pushing this object, send one immediate stop packet
            if (P->LocalPushSeq > 0 || P->PushStopRepeat > 0)
                SendPushPacket(P, P->CurrentDisplacement, 0);
            P->LocalPushSeq   = 0;
            P->PushStopRepeat = 0;
        }
        return;
    }

    for (INT i = 0; i < ControllerOwner->CachedPushables.Num(); i++)
    {
        AOLPushableObject* P = ControllerOwner->CachedPushables(i);
        if (!P) continue;

        FLOAT Disp = P->CurrentDisplacement;

        if (P == AP)
        {
            // Actively pushing — send on displacement change OR on first engagement (LocalPushSeq==0)
            UBOOL bDispChanged = Abs(Disp - ControllerOwner->LastSentPushDisplacement(i)) > 0.1f;
            if (bDispChanged || P->LocalPushSeq == 0)
            {
                ControllerOwner->LastSentPushDisplacement(i) = Disp;
                P->PushStopRepeat = 0;
                SendPushPacket(P, Disp, 1);
            }
        }
        else if (P->PushStopRepeat > 0)
        {
            // Released — send bPushing=0 redundantly; server will clear owner on first arrival
            SendPushPacket(P, Disp, 0);
            if (--P->PushStopRepeat == 0)
            {
                // Stop local sound/state now that burst is done
                P->StopMoving();
                P->LocalPushSeq = 0;
            }
        }
        else if (P->LocalPushSeq > 0 && P != AP)
        {
            // Was pushing but no longer active — trigger stop burst
            P->PushStopRepeat = 5;
            // LocalPushSeq reset happens after burst completes (in the PushStopRepeat branch above)
        }
    }
}

// ============================================================================
// BroadcastPushableStates — send current state of all pushed pushables.
// Called on REQUEST_STATE from a joining player.
// ============================================================================

void UPushableChannel::BroadcastPushableStates()
{
    if (!GMpConn.bIsConnected) return;
    if (!ControllerOwner->bPushablesIndexed)
        ControllerOwner->IndexPushables();

    for (INT i = 0; i < ControllerOwner->CachedPushables.Num(); i++)
    {
        AOLPushableObject* P = ControllerOwner->CachedPushables(i);
        if (!P) continue;

        // Skip pushables at rest — server only needs to know about non-zero positions.
        // PUSH_INIT is first-write-wins: server ignores it if a snapshot already exists,
        // so this broadcast won't overwrite state set by an active player.
        if (Abs(P->CurrentDisplacement) < 0.5f) continue;

        BYTE B[2 + sizeof(FPushStatePacket)];
        INT  N = 0;
        N = PutU8(B, N, CH_PUSH);
        N = PutU8(B, N, PUSH_INIT);  // first-write-wins; server stores only if no snapshot exists
        FPushStatePacket Pkt;
        appMemzero(&Pkt, sizeof(Pkt));
        Pkt.KeyX      = (INT)P->Location.X;
        Pkt.KeyY      = (INT)P->Location.Y;
        Pkt.KeyZ      = (INT)P->Location.Z;
        Pkt.DispX1000 = appRound(P->CurrentDisplacement * 1000.0f);
        Pkt.Seq       = 0;    // init snapshot has no sequence — receivers accept unconditionally
        Pkt.bPushing  = 0;    // always a position sync, never an active push signal
        appMemcpy(B + N, &Pkt, sizeof(Pkt));
        N += sizeof(Pkt);
        GMpConn.SendBinary(B, N);
    }
}

// ============================================================================
// OnBinaryPushState — called from AMultiplayerController::OnReceiveBinaryData
// ============================================================================

void PushableChannel_OnBinaryPushState(UPushableChannel* Ch, INT SenderID, BYTE* Data, INT DataLen)
{
    if (!Ch || !Ch->ControllerOwner) return;
    if (DataLen < (INT)sizeof(FPushStatePacket)) return;
    if (!GMpConn.SyncInteractable) return;

    const FPushStatePacket* Pkt = (const FPushStatePacket*)Data;
    INT      KeyX     = (INT)Pkt->KeyX;
    INT      KeyY     = (INT)Pkt->KeyY;
    INT      KeyZ     = (INT)Pkt->KeyZ;
    FLOAT    Disp     = Pkt->DispX1000 / 1000.0f;
    UINT     Seq      = Pkt->Seq;
    UBOOL    bPushing = Pkt->bPushing != 0;

    AMultiplayerController* Ctrl = Ch->ControllerOwner;

    if (!Ctrl->bPushablesIndexed)
        Ctrl->IndexPushables();

    AOLPushableObject* P = Ch->FindPushableByKey(KeyX, KeyY, KeyZ);
    if (!P)
    {
        // Actor not loaded yet — queue only active pushes (stop packets are irrelevant)
        if (!bPushing) return;
        for (INT i = 0; i < Ctrl->PendingPushStates.Num(); i++)
        {
            if (Ctrl->PendingPushStates(i).KeyX == KeyX
                && Ctrl->PendingPushStates(i).KeyY == KeyY
                && Ctrl->PendingPushStates(i).KeyZ == KeyZ)
            {
                Ctrl->PendingPushStates(i).Displacement = Disp;
                return;
            }
        }
        FPendingPushState S; S.KeyX = KeyX; S.KeyY = KeyY; S.KeyZ = KeyZ; S.Displacement = Disp;
        Ctrl->PendingPushStates.AddItem(S);
        return;
    }

    // Drop out-of-order packets (sequence wrap handled via signed comparison).
    // Stop packets (bPushing=0) always pass — never drop them or the looping sound gets stuck.
    if (bPushing && Seq != 0 && (INT)(Seq - (UINT)P->RemotePushSeq) <= 0)
        return;
    P->RemotePushSeq = (INT)Seq;

    // If we own this pushable locally, ignore remote active pushes but still process stops
    // so bNetLocked is cleared and the looping sound stops if the remote released it.
    if (P->bPlayerLocked && bPushing) return;

    if (!bPushing)
    {
        // Remote player released — always stop movement regardless of bNetLocked.
        // StopMoving must be called even if bNetLocked is false (e.g. stop packet
        // arrived before or without a matching start packet due to fast in/out).
        if (P->bNetLocked)
        {
            P->bNetLocked    = FALSE;
            P->RemotePushSeq = 0;
            P->NetStopPushing();
            P->PostAkEvent(P->SndStopPushing);
        }
        P->StopMoving();
        P->SetNetDisplacement(Disp);
        P->UpdateLinkedDoorState();
        return;
    }

    // Remote player actively pushing
    if (!P->bNetLocked)
    {
        P->bNetLocked = TRUE;
        P->NetStartPushing();
        P->PostAkEvent(P->SndStartPushing);
        P->bPushActive = TRUE;
    }
    P->NetPushLastTime = GWorld ? GWorld->GetTimeSeconds() : 0.0f;
    P->SetNetDisplacement(Disp);
    P->UpdateLinkedDoorState();
}

// ============================================================================
// OnBinaryPushDenied — server rejected our PUSH_STATE (another player owns it).
// Force-stop local push animation immediately.
// ============================================================================

void PushableChannel_OnBinaryPushDenied(UPushableChannel* Ch, BYTE* Data, INT DataLen)
{
    if (!Ch || !Ch->ControllerOwner) return;
    if (DataLen < 12) return; // [KeyX(4)][KeyY(4)][KeyZ(4)]

    INT KeyX = (INT)(Data[0] | (Data[1] << 8) | (Data[2] << 16) | (Data[3] << 24));
    INT KeyY = (INT)(Data[4] | (Data[5] << 8) | (Data[6] << 16) | (Data[7] << 24));
    INT KeyZ = (INT)(Data[8] | (Data[9] << 8) | (Data[10] << 16) | (Data[11] << 24));

    AOLPushableObject* P = Ch->FindPushableByKey(KeyX, KeyY, KeyZ);
    if (!P) return;

    // Cancel outgoing burst and stop local animation
    P->PushStopRepeat = 0;
    P->LocalPushSeq   = 0;

    AOLHero* Hero = Ch->HeroPawn ? Cast<AOLHero>(Ch->HeroPawn) : NULL;
    if (Hero && Hero->ActivePushable == P)
        Hero->StopPushing(); // releases bPlayerLocked + clears ActivePushable
}
