#include "Multiplayer.h"
#include "HeroChannel.h"
#include "HeroChannelPackets.h"
#include "WorldChannelPackets.h"
#include "MultiplayerHUD.h"
#include "OLGameClasses.h"
#include "EngineAnimClasses.h"
#include "UDKBaseAnimationClasses.h"
#include "OLGameAnimClasses.h"
#include "OLUtilities.h"

// ============================================================================
// FindCornerMarkerNear — find closest AOLCornerMarker within radius to Pos.
// Checks CachedCorners first (fast path); falls back to FActorIterator if cache is empty.
// ============================================================================

static AOLCornerMarker* FindCornerMarkerNear(AOLHero* Dummy, const FVector& Pos)
{
    AOLCornerMarker* best = NULL;
    FLOAT bestDistSq = Square(200.f);

    for (INT i = 0; i < Dummy->CachedCorners.Num(); i++)
    {
        AOLCornerMarker* m = Dummy->CachedCorners(i);
        if (!m) continue;
        FLOAT dSq = (m->Location - Pos).SizeSquared();
        if (dSq < bestDistSq) { bestDistSq = dSq; best = m; }
    }
    if (best) return best;

    // Cache not yet populated — scan world actors directly.
    bestDistSq = Square(200.f);
    for (FActorIterator It; It; ++It)
    {
        AOLCornerMarker* m = Cast<AOLCornerMarker>(*It);
        if (!m) continue;
        FLOAT dSq = (m->Location - Pos).SizeSquared();
        if (dSq < bestDistSq) { bestDistSq = dSq; best = m; }
    }
    return best;
}

// ============================================================================
// ============================================================================
// Send: build binary hero state packet
// ============================================================================

void BuildStatePacket(AOLHero* Hero, UBOOL bSendingJumpGroundZ, FLOAT JumpGroundZ, BYTE* B, INT& N)
{
    const FLOAT LocZ = bSendingJumpGroundZ ? JumpGroundZ : Hero->Location.Z;

    N = PutU8 (B, 0,  CH_HERO);
    N = PutU8 (B, N,  HERO_STATE);
    N = PutF32(B, N,  Hero->Location.X);
    N = PutF32(B, N,  Hero->Location.Y);
    N = PutF32(B, N,  LocZ);
    N = PutU16(B, N,  Hero->Rotation.Pitch & 0xFFFF);
    N = PutU16(B, N,  Hero->Rotation.Yaw   & 0xFFFF);
    N = PutI16(B, N,  (INT)Hero->Velocity.X);
    N = PutI16(B, N,  (INT)Hero->Velocity.Y);
    N = PutI16(B, N,  (INT)Hero->Velocity.Z);
    N = PutU8 (B, N,  (INT)Hero->bIsCrouched);
    N = PutU8 (B, N,  Hero->CamcorderState);
    N = PutU8 (B, N,  Hero->SpecialMove);
    N = PutU8 (B, N,  Hero->LocomotionMode);
    N = PutI16(B, N,  (INT)(Hero->LadderAnimNode->SmoothedDelta          * 1000.f));
    N = PutI16(B, N,  (INT)(Hero->GetLedgeHangSignedDelta()              * 1000.f));
    N = PutI16(B, N,  (INT)(Hero->GetLedgeWalkSignedDelta()              * 1000.f));
    N = PutI16(B, N,  Hero->Health);
    N = PutU8 (B, N,  (INT)Hero->bHeatShielding);
    N = PutI16(B, N,  (INT)(Hero->HeatDistance * 10.f));
    N = PutI16(B, N,  (INT)(Hero->GetSqueezeSignedDelta()                * 1000.f));
    N = PutU8 (B, N,  (INT)Hero->bHobbling);
    N = PutI16(B, N,  (INT)(Hero->HobblingIntensity                      * 1000.f));
    N = PutI16(B, N,  (INT)(Hero->TargetHobblingIntensity                * 1000.f));
    N = PutU8 (B, N,  (INT)Hero->bLimping);
    N = PutI16(B, N,  (INT)(Hero->CurrentLean                            * 1000.f));
    N = PutI16(B, N,  (INT)(Hero->GetPeekingRatio()                      * 1000.f));
    N = PutI16(B, N,  (INT)(Hero->CornerPeek.IKStrength                  * 1000.f));
    N = PutU8 (B, N,  (INT)(Hero->BodySetup == HBS_CamcorderVisible || Hero->CamcorderState == CCS_Active));
    N = PutU8 (B, N,  (INT)Hero->bLeftAnim);
    N = PutI16(B, N,  Hero->EyeRotation.Yaw - Hero->Rotation.Yaw);
    N = PutU8 (B, N,  (INT)Hero->IsInDarkness());
    N = PutU8 (B, N,  (INT)Hero->bIsGhost);
    N = PutU8 (B, N,  (INT)Hero->bParrying);
    N = PutI16(B, N,  (INT)(Hero->ParryingAnimNode ? Hero->ParryingAnimNode->EnemyDistance : -1.f));
    N = PutI16(B, N,  (INT)(Hero->ParryingAnimNode ? Hero->ParryingAnimNode->EnemyRelYaw   :  0.f));

    // Footstep surface: 0=None, 1=Water, 2=Blood
    BYTE FootSurf = 0;
    {
        FName Mat = Hero->LastFootstepSurface;
        if      (Mat == Hero->WaterMaterial) FootSurf = 1;
        else if (Mat == Hero->BloodMaterial) FootSurf = 2;
    }
    N = PutU8(B, N, FootSurf);

    // Struggle cycle anim play rate (0 when not in LM_Struggle)
    {
        AOLPlayerController* OLPC = Hero->OLPC;
        FLOAT rate = (OLPC && Hero->LocomotionMode == LM_Struggle)
            ? OLPC->Struggle.SmoothedAnimPlayRate : 0.f;
        N = PutI16(B, N, (INT)(rate * 1000.f));
    }

    // Nick tail: [nick_utf8_len(1)][nick UTF-8 bytes, max 60]
    FString Nick = GMpConn.Username.Len() > 0 ? GMpConn.Username : FString(TEXT("Player"));
    if (Nick.Len() == 0) Nick = TEXT("Player");
    BYTE NickUtf8[64];
    INT NickUtf8Len = TCHARToUTF8(NickUtf8, 61, *Nick); // max 60 bytes + guard
    N = PutU8(B, N, (BYTE)NickUtf8Len);
    for (INT i = 0; i < NickUtf8Len; i++)
        N = PutU8(B, N, NickUtf8[i]);
}

// ============================================================================
// Receive: decode binary state packet into FHeroStatePacket
// ============================================================================

UBOOL DecodeBinaryState(const BYTE* Data, INT DataLen, FHeroStatePacket& S)
{
    if (DataLen < 1)
        return FALSE;

    INT Off = 0;
    INT Pitch, Yaw, VX, VY, VZ;
    INT Crouched, CamState, SMT, LocMode;
    INT LadderD, HangD, WalkD, Health;
    INT HeatShield, HeatDist, SqueezeD;
    INT Hobbling, HobbInt, TargHobb;
    INT Limping, Lean, PeekRatio, CornerIK;
    INT CamVis, LeftAnim, EyeYaw, InDark, IsGhost;
    INT Parrying, ParryDist, ParryYaw;

    Off = ReadF32(Data, Off, S.Loc.X);
    Off = ReadF32(Data, Off, S.Loc.Y);
    Off = ReadF32(Data, Off, S.Loc.Z);
    Off = ReadU16(Data, Off, Pitch); S.Rot.Pitch = Pitch;
    Off = ReadU16(Data, Off, Yaw);   S.Rot.Yaw   = Yaw;   S.Rot.Roll = 0;
    Off = ReadI16(Data, Off, VX);    S.Vel.X = (FLOAT)VX;
    Off = ReadI16(Data, Off, VY);    S.Vel.Y = (FLOAT)VY;
    Off = ReadI16(Data, Off, VZ);    S.Vel.Z = (FLOAT)VZ;
    Off = ReadU8 (Data, Off, Crouched);
    Off = ReadU8 (Data, Off, CamState);
    Off = ReadU8 (Data, Off, SMT);
    Off = ReadU8 (Data, Off, LocMode);
    Off = ReadI16(Data, Off, LadderD);
    Off = ReadI16(Data, Off, HangD);
    Off = ReadI16(Data, Off, WalkD);
    Off = ReadI16(Data, Off, Health);
    Off = ReadU8 (Data, Off, HeatShield);
    Off = ReadI16(Data, Off, HeatDist);
    Off = ReadI16(Data, Off, SqueezeD);
    Off = ReadU8 (Data, Off, Hobbling);
    Off = ReadI16(Data, Off, HobbInt);
    Off = ReadI16(Data, Off, TargHobb);
    Off = ReadU8 (Data, Off, Limping);
    Off = ReadI16(Data, Off, Lean);
    Off = ReadI16(Data, Off, PeekRatio);
    Off = ReadI16(Data, Off, CornerIK);
    Off = ReadU8 (Data, Off, CamVis);
    Off = ReadU8 (Data, Off, LeftAnim);
    Off = ReadI16(Data, Off, EyeYaw);
    Off = ReadU8 (Data, Off, InDark);
    Off = ReadU8 (Data, Off, IsGhost);
    Off = ReadU8 (Data, Off, Parrying);
    Off = ReadI16(Data, Off, ParryDist);
    Off = ReadI16(Data, Off, ParryYaw);

    INT FootSurf = 0;
    Off = ReadU8(Data, Off, FootSurf);
    S.FootstepSurface = FootSurf;

    // Struggle play rate
    S.StrugglePlayRate = 0.f;
    if (Off + 2 <= DataLen)
    {
        INT SR = 0;
        Off = ReadI16(Data, Off, SR);
        S.StrugglePlayRate = (FLOAT)SR / 1000.f;
    }

    // Optional nick tail: UTF-8 encoded
    S.bHasNick = FALSE;
    if (Off < DataLen)
    {
        INT NickUtf8Len = 0;
        Off = ReadU8(Data, Off, NickUtf8Len);
        NickUtf8Len = Min(NickUtf8Len, Min(63, DataLen - Off));
        FString NickStr;
        UTF8ToFString(Data + Off, NickUtf8Len, NickStr);
        appStrncpy(S.Nick, *NickStr, 33);
        S.bHasNick = (NickStr.Len() > 0);
        Off += NickUtf8Len;
    }

    S.Health                    = Health;
    S.CamcorderState            = CamState;
    S.SpecialMove               = SMT;
    S.LocomotionMode            = LocMode;
    S.bCrouched                 = Crouched   != 0;
    S.bHeatShielding            = HeatShield != 0;
    S.HeatDistance              = (FLOAT)HeatDist   / 10.f;
    S.LadderDelta               = (FLOAT)LadderD    / 1000.f;
    S.LedgeHangDelta            = (FLOAT)HangD      / 1000.f;
    S.LedgeWalkDelta            = (FLOAT)WalkD      / 1000.f;
    S.SqueezeDelta              = (FLOAT)SqueezeD   / 1000.f;
    S.bHobbling                 = Hobbling   != 0;
    S.HobblingIntensity         = (FLOAT)HobbInt    / 1000.f;
    S.TargetHobblingIntensity   = (FLOAT)TargHobb   / 1000.f;
    S.bLimping                  = Limping    != 0;
    S.CurrentLean               = (FLOAT)Lean       / 1000.f;
    S.PeekingRatio              = (FLOAT)PeekRatio  / 1000.f;
    S.CornerIKStrength          = (FLOAT)CornerIK   / 1000.f;
    S.bCamMeshVisible           = CamVis     != 0;
    S.bLeftAnim                 = LeftAnim   != 0;
    S.EyeYaw                    = EyeYaw;
    S.bInDarkness               = InDark     != 0;
    S.bIsGhost                  = IsGhost    != 0;
    S.bParrying                 = Parrying   != 0;
    S.ParryEnemyDist            = (FLOAT)ParryDist;
    S.ParryEnemyRelYaw          = (FLOAT)ParryYaw;

    return TRUE;
}

// ============================================================================
// Apply decoded state to remote player slot and dummy pawn
// ============================================================================

static void ApplyHeroState(AMultiplayerController* Controller, INT Idx, const FHeroStatePacket& S)
{
    URemotePlayer* P = Controller->RemotePlayers(Idx);

    P->LastReceivedLoc          = S.Loc;
    P->LastReceivedVel          = S.Vel;
    P->LastReceivedRot          = S.Rot;
    P->LastRemoteHealth         = S.Health;
    P->bLastRemoteHeatShielding = S.bHeatShielding;
    P->LastRemoteHeatDistance   = S.HeatDistance;

    if (P->DummyPlayer == NULL)
    {
        P->bHasReceivedData = TRUE;
        return;
    }

    AOLHero* Dummy = Cast<AOLHero>(P->DummyPlayer);
    if (Dummy == NULL)
    {
        P->bHasReceivedData = TRUE;
        return;
    }

    if (!P->bHasReceivedData)
    {
        Dummy->SetLocation(S.Loc);
        Dummy->SetRotation(S.Rot);
        // Unhide mesh on first LOC packet — SpawnDummy hides it until we have a valid position.
        if (Dummy->Mesh)          Dummy->Mesh->SetHiddenGame(FALSE);
        if (Dummy->DummyHeadMesh) Dummy->DummyHeadMesh->SetHiddenGame(FALSE);
    }
    P->bHasReceivedData = TRUE;

    Dummy->Health        = S.Health;
    Dummy->PreciseHealth = (FLOAT)S.Health;

    if (S.bCrouched != (UBOOL)P->bLastRemoteCrouched)
    {
        P->bLastRemoteCrouched = S.bCrouched;
        P->bDummyCrouched      = S.bCrouched;
        Dummy->SetDummyCrouched(S.bCrouched);
    }

    Controller->HeroChannel->ApplyCamcorderState(Idx, Dummy, S.CamcorderState);

    if (S.SpecialMove != P->LastRemoteSpecialMove)
        Controller->HeroChannel->ApplySpecialMoveTransition(Idx, Dummy, S.SpecialMove);

    if (S.LocomotionMode != P->LastRemoteLocomotionMode)
        Controller->HeroChannel->ApplyLocomotionMode(Idx, Dummy, S.LocomotionMode);

    Dummy->SetDummyLadderDelta(S.LadderDelta);
    Dummy->SetDummyLedgeHangDelta(S.LedgeHangDelta);
    Dummy->SetDummyLedgeWalkDelta(S.LedgeWalkDelta);
    Dummy->SetDummySqueezeDelta(S.SqueezeDelta);
    Dummy->SetDummyHobblingState(S.bHobbling, S.HobblingIntensity, S.TargetHobblingIntensity);
    Dummy->bLimping              = S.bLimping;
    Dummy->CurrentLean           = S.CurrentLean;
    Dummy->DummyStrugglePlayRate = S.StrugglePlayRate;

    if ((S.LocomotionMode == LM_ContextualLean || Dummy->LocomotionMode == LM_ContextualLean) && Dummy->PeekingAnimNode)
    {
        // Mirror UpdateContextualLean: drive TargetRatio from network; let the AnimNode interpolate CurrentRatio.
        Dummy->PeekingAnimNode->TargetRatio = S.PeekingRatio;
        if (Dummy->ShadowProxyPeekingAnimNode)
            Dummy->ShadowProxyPeekingAnimNode->TargetRatio = S.PeekingRatio;
    }
    // CornerPeek.IKStrength is now driven by UpdateCornerPeek on the dummy directly.

    if (Dummy->CameraMesh != NULL)
        Dummy->CameraMesh->SetHiddenGame(!S.bCamMeshVisible);

    Dummy->bLeftAnim = S.bLeftAnim;
    Dummy->SetDummyBedRelYaw(S.EyeYaw);
    Dummy->bNetInDarkness = S.bInDarkness;
    Dummy->bIsGhost       = S.bIsGhost;

    if (Dummy->ParryingAnimNode)
    {
        Dummy->ParryingAnimNode->SetActive(S.bParrying);
        Dummy->ParryingAnimNode->EnemyDistance = S.ParryEnemyDist;
        Dummy->ParryingAnimNode->EnemyRelYaw   = S.ParryEnemyRelYaw;
    }
    if (Dummy->ShadowProxyParryingAnimNode)
    {
        Dummy->ShadowProxyParryingAnimNode->SetActive(S.bParrying);
        Dummy->ShadowProxyParryingAnimNode->EnemyDistance = S.ParryEnemyDist;
        Dummy->ShadowProxyParryingAnimNode->EnemyRelYaw   = S.ParryEnemyRelYaw;
    }

    // Sync footstep surface so blood/water decals and particles work on dummy.
    if      (S.FootstepSurface == 1) Dummy->LastFootstepSurface = Dummy->WaterMaterial;
    else if (S.FootstepSurface == 2) Dummy->LastFootstepSurface = Dummy->BloodMaterial;
    else                             Dummy->LastFootstepSurface = NAME_None;

    // Keep BlendByPosture in sync with bIsCrouched every tick (mirrors old UC SyncCrouchPosture call).
    Dummy->SyncCrouchPosture();

    // Manage dummy physics per-tick to match old UC Tick logic:
    //   LM_Walk, no relevant SMT           → PHYS_Walking (enables Crouch() via physWalking)
    //   LM_Door                            → PHYS_None    (no gravity; pawn tracks door handle)
    //   LM_LedgeHang / LedgeWalk          → PHYS_Custom
    //   LM_Ladder/Squeeze/Locker/Bed      → PHYS_Custom
    //   LM_Cinematic/Pushing/ContextualLean → PHYS_Custom (no gravity during matinee/push/lean)
    //   SMT active (except Crouch/Uncrouch/ContextualLean) → leave as-is (SMT pipeline owns physics)
    {
        const INT LM  = S.LocomotionMode;
        const INT SMT = S.SpecialMove;
        const UBOOL bSmtActive = (SMT != SMT_None && SMT != SMT_Crouch && SMT != SMT_Uncrouch && SMT != SMT_EnterContextualLean);
        if (!bSmtActive)
        {
            if (LM == LM_Door || Dummy->LocomotionMode == LM_Door)
            {
                if (Dummy->Physics != PHYS_None) Dummy->setPhysics(PHYS_None);
            }
            else if (LM == LM_LedgeHang || LM == LM_LedgeWalk ||
                     Dummy->LocomotionMode == LM_LedgeHang || Dummy->LocomotionMode == LM_LedgeWalk)
            {
                if (Dummy->Physics != PHYS_Custom) Dummy->setPhysics(PHYS_Custom);
            }
            else if (LM == LM_Walk)
            {
                if (Dummy->Physics != PHYS_Walking) Dummy->setPhysics(PHYS_Walking);
            }
            else if (LM == LM_Ladder || LM == LM_Squeeze || LM == LM_Locker || LM == LM_Bed ||
                     LM == LM_Cinematic || LM == LM_Pushing || LM == LM_ContextualLean)
            {
                if (Dummy->Physics != PHYS_Custom) Dummy->setPhysics(PHYS_Custom);
            }
        }
    }
}

// ============================================================================
// UHeroChannel::ApplyCamcorderState
// ============================================================================

void UHeroChannel::ApplyCamcorderState(INT Idx, AOLHero* Dummy, INT NewCamcorderState)
{
    AMultiplayerController* C = Cast<AMultiplayerController>(ControllerOwner);
    if (!C) return;
    URemotePlayer* P = C->RemotePlayers(Idx);

    if (NewCamcorderState == P->LastRemoteCamcorderState)
        return;

    UBOOL bCrouched = Dummy->bIsCrouched;
    P->LastRemoteCamcorderState = NewCamcorderState;

    switch (NewCamcorderState)
    {
    case 2: // CCS_Raising
        if (Dummy->CameraMesh)
            Dummy->CameraMesh->SetHiddenGame(FALSE);
        Dummy->ClearDummyUpperBodyIdleAnim();
        Dummy->TryRaiseCamcorder();
        break;
    case 1: // CCS_Active
        Dummy->CamcorderState = 1;
        break;
    case 3: // CCS_Lowering
        Dummy->LowerCamcorder();
        break;
    case 0: // CCS_Inactive
        Dummy->CamcorderState = 0;
        if (Dummy->CameraMesh)
            Dummy->CameraMesh->SetHiddenGame(TRUE);
        Dummy->ClearDummyUpperBodyIdleAnim();
        break;
    case 4: // CCS_ReloadingActive
        Dummy->ClearDummyUpperBodyIdleAnim();
        Dummy->CamcorderState = 1;
        Dummy->ReloadBatteries();
        break;
    case 5: // CCS_ReloadingInactive
        Dummy->ClearDummyUpperBodyIdleAnim();
        Dummy->CamcorderState = 0;
        Dummy->ReloadBatteries();
        break;
    default: break;
    }
}

// ============================================================================
// UHeroChannel::ApplySpecialMoveTransition
// ============================================================================

static FName PeekAnimName(INT SMT, UBOOL bFromLeft, UBOOL bRounded)
{
    switch (SMT)
    {
    case 118: return bFromLeft ? (bRounded ? FName(TEXT("player_wall_to_peek_from_left_s"))    : FName(TEXT("player_wall_to_peek_from_left_h")))
                               : (bRounded ? FName(TEXT("player_wall_to_peek_from_right_s"))   : FName(TEXT("player_wall_to_peek_from_right_h")));
    case 119: return bFromLeft ? (bRounded ? FName(TEXT("player_wall_leave_peek_from_left_s")) : FName(TEXT("player_wall_leave_peek_from_left_h")))
                               : (bRounded ? FName(TEXT("player_wall_leave_peek_from_right_s")): FName(TEXT("player_wall_leave_peek_from_right_h")));
    case 120: return bFromLeft ? (bRounded ? FName(TEXT("player_wall_exit_from_left_s"))       : FName(TEXT("player_wall_exit_from_left_h")))
                               : (bRounded ? FName(TEXT("player_wall_exit_from_right_s"))      : FName(TEXT("player_wall_exit_from_right_h")));
    case 121: return bFromLeft ? FName(TEXT("player_wall_transition_from_left")) : FName(TEXT("player_wall_transition_from_right"));
    }
    return NAME_None;
}

void UHeroChannel::ApplySpecialMoveTransition(INT Idx, AOLHero* Dummy, INT NewSpecialMove)
{
    AMultiplayerController* C = Cast<AMultiplayerController>(ControllerOwner);
    if (!C) return;
    URemotePlayer* P = C->RemotePlayers(Idx);

    // Leaving SMT_StartPushingObject: unlock all net-locked pushables.
    if (P->LastRemoteSpecialMove == SMT_StartPushingObject && NewSpecialMove != SMT_StartPushingObject)
    {
        for (INT pi = 0; pi < C->CachedPushables.Num(); pi++)
        {
            AOLPushableObject* Push = C->CachedPushables(pi);
            if (Push && Push->bNetLocked)
            {
                Push->bNetLocked = FALSE;
                Push->NetStopPushing();
            }
        }
    }

    P->LastRemoteSpecialMove = NewSpecialMove;

    UBOOL bFromLeft = P->bLastRemotePeekFromLeft;
    UBOOL bRounded  = P->bLastRemotePeekRounded;

    switch (NewSpecialMove)
    {
    case SMT_Crouch:
    case SMT_Uncrouch:
        Dummy->ClearDummyUpperBodyIdleAnim();
        break;
    case SMT_BigLanding:
        Dummy->PlayShadowOnlyAnim(
            P->bLastRemoteCrouched ? FName(TEXT("player_crouch_land_big")) : FName(TEXT("player_landing_big")),
            1.f, 0.1f, 0.25f);
        break;
    case SMT_SlideOver:
        P->bJumpOverActive = TRUE;
        P->JumpGroundZ     = Dummy->Location.Z;
        break;
    case SMT_DropFromLedge:
        P->DummySMTLockUntil = 0.f;
        break;
    case SMT_MP_WallToPeek:
    case SMT_MP_LeavePeek:
    case SMT_MP_WallExit:
        Dummy->PlayShadowOnlyAnim(PeekAnimName(NewSpecialMove, bFromLeft, bRounded),
            1.f, NewSpecialMove == SMT_MP_WallExit ? 0.25f : 0.1f, 0.25f);
        break;
    case SMT_MP_WallTransition:
        Dummy->PlayShadowOnlyAnim(PeekAnimName(SMT_MP_WallTransition, bFromLeft, bRounded), 1.f, 0.1f, 0.f);
        break;
    default:
        if (P->bJumpOverActive)
        {
            P->bJumpOverActive = FALSE;
            FVector SnapLoc    = P->LastReceivedLoc;
            SnapLoc.Z          = P->JumpGroundZ;
            Dummy->SetLocation(SnapLoc);
        }
        if (NewSpecialMove == SMT_None)
            P->LastReceivedLoc = Dummy->Location;
        break;
    }

}

// ============================================================================
// UHeroChannel::ApplyLocomotionMode
// ============================================================================

void UHeroChannel::ApplyLocomotionMode(INT Idx, AOLHero* Dummy, INT NewLocomotionMode)
{
    AMultiplayerController* C = Cast<AMultiplayerController>(ControllerOwner);
    if (!C) return;
    URemotePlayer* P = C->RemotePlayers(Idx);

    INT OldLM = P->LastRemoteLocomotionMode;
    P->LastRemoteLocomotionMode = NewLocomotionMode;

    switch (NewLocomotionMode)
    {
    case LM_SpecialMove: break; // keep stable
    case LM_Walk:
        Dummy->ResetDummyAnimState();
        Dummy->SetDummyLocomotionMode(LM_Walk);
        P->DummySMTLockUntil = 0.f;
        P->bPeekTypeApplied  = FALSE;
        break;
    case LM_Fall:
        Dummy->ResetDummyAnimState();
        P->DummySMTLockUntil = 0.f;
        Dummy->SetDummyLocomotionMode(LM_Fall);
        break;
    case LM_Ladder:
    case LM_LedgeWalk:
        Dummy->SetDummyLocomotionMode(NewLocomotionMode);
        break;
    case LM_Squeeze:
        Dummy->SetDummyActiveSqueeze(Dummy->Location);
        Dummy->SetDummyLocomotionMode(LM_Squeeze);
        break;
    case LM_Locker:
        Dummy->SetDummyActiveLocker(Dummy->Location);
        Dummy->SetDummyLocomotionMode(LM_Locker);
        break;
    case LM_Bed:
        Dummy->SetDummyActiveBed(Dummy->Location);
        Dummy->SetDummyLocomotionMode(LM_Bed);
        break;
    case LM_LedgeHang:
        Dummy->SetLocation(P->LastReceivedLoc);
        Dummy->SetDummyLocomotionMode(LM_LedgeHang);
        break;
    case LM_Door:
        if (Dummy->SpecialMove == SMT_EnterDoorInteraction)
            break;
        // ActiveDoor is set by DoorChannel (DOOR_LOCK packet). If that packet hasn't
        // arrived yet, skip LM_Door — CalcVelocity and UpdateDoorInteraction both
        // check(ActiveDoor) and will crash with a NULL pointer.
        if (!Dummy->ActiveDoor)
            break;
        {
            INT   DoorIdx   = P->LockedDoorIdx;
            FLOAT DoorRatio = 0.f;
            if (DoorIdx != -1 && DoorIdx < C->CachedDoors.Num())
            {
                AOLDoor* D = Cast<AOLDoor>(C->CachedDoors(DoorIdx));
                if (D)
                    DoorRatio = Clamp(D->GetOpenAngle() / 90.f, 0.f, 1.f);
            }
            Dummy->SetDummyLocomotionMode(LM_Door);
            Dummy->InitDummyDoorAnim(P->LastRemoteDoorOpeningType, DoorRatio);
        }
        break;
    case LM_Cinematic:
        Dummy->ResetDummyAnimState();
        Dummy->SetLocation(P->LastReceivedLoc);
        Dummy->SetDummyLocomotionMode(LM_Cinematic);
        break;
    case LM_Pushing:
        Dummy->SetDummyActivePushable(Dummy->Location);
        Dummy->bPushingFromBackEdge = P->bLastRemotePushFromBack;
        if (Dummy->SpecialMove != SMT_StartPushingObject)
            Dummy->SetDummyLocomotionMode(LM_Pushing);
        break;
    case LM_ContextualLean:
        {
            UBOOL bFromLeft = P->bLastRemotePeekFromLeft;
            UBOOL bRounded  = P->bLastRemotePeekRounded;
            // Mirror TryEnterContextualLean: configure PeekingAnimNode then enter the locomotion mode.
            if (Dummy->PeekingAnimNode && Dummy->ShadowProxyPeekingAnimNode)
            {
                Dummy->PeekingAnimNode->SetPeekingType(bFromLeft, bRounded);
                Dummy->ShadowProxyPeekingAnimNode->SetPeekingType(bFromLeft, bRounded);
                Dummy->PeekingAnimNode->StartPeeking(0.f);
                Dummy->ShadowProxyPeekingAnimNode->StartPeeking(0.f);
            }
            Dummy->CornerPeek.CornerLocation  = P->LastRemoteCornerLocation;
            Dummy->CornerPeek.FwdDir          = P->LastRemoteCornerFwdDir;
            {
                FVector right = (P->LastRemoteCornerFwdDir ^ FVector(0,0,1)).SafeNormal();
                Dummy->CornerPeek.SideDir = bFromLeft ? -right : right;
            }
            Dummy->CornerPeek.PeekPosition   = bFromLeft ? CPP_Left : CPP_Right;
            Dummy->CornerPeek.bRoundedCorner = bRounded;
            Dummy->CornerPeek.CornerMarker = FindCornerMarkerNear(Dummy, P->LastRemoteCornerLocation);
            Dummy->SetDummyLocomotionMode(LM_ContextualLean);
        }
        break;
    case LM_Struggle:
        Dummy->SetDummyLocomotionMode(LM_Struggle);
        // If SMT packet already set the cycle anim name, start looping it now.
        // If SMT packet arrives later, it will start the loop itself (see SMT_EnterStruggle case).
        if (Dummy->DummyStruggleCycleAnimPlayer != NAME_None && Dummy->FullBodyAnimSlot)
            Dummy->FullBodyAnimSlot->PlayCustomAnim(
                Dummy->DummyStruggleCycleAnimPlayer,
                Dummy->DummyStrugglePlayRate > 0.f ? Dummy->DummyStrugglePlayRate : 1.f,
                0.1f, 0.0f, TRUE, FALSE);
        break;

    default:
        Dummy->SetDummyLocomotionMode(LM_Walk);
        if (OldLM == LM_Pushing || OldLM == LM_Cinematic)
            Dummy->ClearFullBodyIdleAnim();
        break;
    }
}

// ============================================================================
// SendHeadRotation — binary HEAD_ROT every tick
// ============================================================================

void UHeroChannel::SendHeadRotation()
{
    if (!CanSend()) return;

    FRotator ViewRot = GMultiplayerHero->bIsGhost
        ? GMultiplayerController->DebugCamRot
        : GMultiplayerController->Rotation;

    BYTE B[2 + sizeof(FHeadRotPacket)];
    INT  N = 0;
    N = PutU8(B, N, CH_HERO);
    N = PutU8(B, N, HERO_HEAD_ROT);
    N = PutI32(B, N, ViewRot.Pitch);
    N = PutI32(B, N, ViewRot.Yaw - 16384); // match old UC: HEAD_ROT sent Yaw-16384
    GMpConn.SendBinary(B, N);
}

// ============================================================================
// SendMesh — binary MESH_PRESET on change
// ============================================================================

void UHeroChannel::SendMesh()
{
    if (!CanSend()) return;

    INT CurPreset = GMultiplayerHero->GetMeshPresetIndex();
    if (CurPreset == LastSentMeshPreset) return;
    LastSentMeshPreset = CurPreset;

    BYTE B[2 + sizeof(FMeshPresetPacket)];
    INT  N = 0;
    N = PutU8(B, N, CH_HERO);
    N = PutU8(B, N, HERO_MESH_PRESET);
    N = PutU8(B, N, (BYTE)CurPreset);
    GMpConn.SendBinary(B, N);
}

// ============================================================================
// SendCinematicAnimation — binary CINEMATIC_ANIM on change
// ============================================================================

void UHeroChannel::SendCinematicAnimation()
{
    if (!CanSend()) return;

    if (GMultiplayerHero->LocomotionMode == LM_Cinematic)
    {
        // Find active matinee anim slot
        FString CurAnim;
        USkeletalMeshComponent* MeshComp = GMultiplayerHero->Mesh;
        if (MeshComp)
        {
            for (INT i = 0; i < MeshComp->AnimTickArray.Num(); i++)
            {
                UAnimNodeSlot* Slot = Cast<UAnimNodeSlot>(MeshComp->AnimTickArray(i));
                if (!Slot || !Slot->bIsBeingUsedByInterpGroup) continue;

                UAnimNodeSequence* SeqNode = NULL;
                for (INT ci = 1; ci < Slot->Children.Num(); ci++)
                {
                    SeqNode = Cast<UAnimNodeSequence>(Slot->Children(ci).Anim);
                    if (SeqNode && SeqNode->AnimSeqName != NAME_None) break;
                    SeqNode = NULL;
                }
                if (!SeqNode) continue;

                if (SeqNode->AnimSeq && SeqNode->AnimSeq->GetOuter())
                    CurAnim = FString::Printf(TEXT("%s|%s"),
                        *SeqNode->AnimSeq->GetOuter()->GetPathName(),
                        *SeqNode->AnimSeqName.ToString());
                else
                    CurAnim = FString::Printf(TEXT("|%s"), *SeqNode->AnimSeqName.ToString());
                break;
            }
        }

        if (CurAnim == LastSentCinematicAnim) return;
        LastSentCinematicAnim = CurAnim;

        // Convert to ASCII for wire
        const TCHAR* Path  = *CurAnim;
        INT          PLen  = appStrlen(Path);
        if (PLen > 255) PLen = 255;

        BYTE B[3 + 255];
        INT  N = 0;
        N = PutU8(B, N, CH_HERO);
        N = PutU8(B, N, HERO_CINEMATIC_ANIM);
        if (CurAnim.Len() == 0)
        {
            // empty string after LM_Cinematic entered — send stop
            N = PutU8(B, N, 1);
        }
        else
        {
            N = PutU8(B, N, 0);
            N = PutU8(B, N, (BYTE)PLen);
            for (INT k = 0; k < PLen; k++)
                N = PutU8(B, N, (BYTE)(Path[k] & 0x7F));
        }
        GMpConn.SendBinary(B, N);
    }
    else if (LastSentCinematicAnim.Len() > 0)
    {
        LastSentCinematicAnim = TEXT("");
        BYTE B[3];
        INT  N = 0;
        N = PutU8(B, N, CH_HERO);
        N = PutU8(B, N, HERO_CINEMATIC_ANIM);
        N = PutU8(B, N, 1); // bStop
        GMpConn.SendBinary(B, N);
    }
}

// ============================================================================
// SendSpecialMoveType — binary SMT_TYPE on transition + pre-params
// ============================================================================

static void BuildAndSendSmtTransition(UHeroChannel* Ch, INT CurSMT)
{
    AOLHero* Hero = GMultiplayerHero;

    FSmtTypePacket P;
    appMemzero(&P, sizeof(P));
    P.SMT = CurSMT;

    // Grab position / direction — always filled (receiver picks what applies per SMT).
    // JumpOverAndGrabLedge / ClimbUpLedge pass LastGrabTargetPos (the ledge/far-side point)
    // as the AdjustPosition target; all other SMTs use LastGrabPos (expectedAnimStart).
    // For SMT_CSA, LastGrabPos/LastGrabDir are temporarily overridden by MpSendCSAActivation
    // with the actual expectedAnimStart/expectedAnimFwd computed in TryCSA.
    const FVector& GrabPos =
        (CurSMT == SMT_JumpOverAndGrabLedge)
        ? Hero->LastGrabTargetPos
        : Hero->LastGrabPos;
    P.GrabPosX = GrabPos.X;
    P.GrabPosY = GrabPos.Y;
    P.GrabPosZ = GrabPos.Z;
    P.GrabDirX = Hero->LastGrabDir.X;
    P.GrabDirY = Hero->LastGrabDir.Y;
    P.GrabDirZ = Hero->LastGrabDir.Z;
    P.GrabLength              = Hero->LastGrabLength;
    P.BlendAlphaX1000         = (INT)(Hero->SpecialMoveBlendAlpha * 1000.f);
    P.SpecialMoveTargetYawX1000 = (INT)(Hero->SpecialMoveTargetYaw * 1000.f);

    // Bool flags — always filled
    P.bLeftAnim             = Hero->bLeftAnim                   ? 1 : 0;
    P.bPushingFromBackEdge  = Hero->bPushingFromBackEdge        ? 1 : 0;
    P.bExitLadderLeftHand   = Hero->bExitLadderLeftHand         ? 1 : 0;
    P.bIsCrouched           = Hero->bIsCrouched                 ? 1 : 0;
    P.bBackAnim             = Hero->bBackAnim                   ? 1 : 0;
    P.bRunningTraversalMove = Hero->bRunningTraversalMove        ? 1 : 0;
    P.bMustCrouchAfterSMT   = Hero->bMustCrouchAfterSpecialMove ? 1 : 0;
    P.bJumpRun              = Hero->bRunningTraversalMove        ? 1 : 0;

    // Enum fields — always filled
    P.LedgeTransitionType = (BYTE)Hero->ActiveLedgeTransitionType;
    P.LedgeClimbType      = (BYTE)Hero->LedgeClimbType;
    P.EnemyType           = (BYTE)Hero->EnemyType;
    P.EnemyWeapon         = (BYTE)Hero->EnemyWeapon;

    // Door params (SMT 29-36)
    if (CurSMT == SMT_OpenDoorInstant      ||
        CurSMT == SMT_OpenDoorPartial      ||
        CurSMT == SMT_TryOpenLockedDoor    ||
        CurSMT == SMT_RunThroughDoor       ||
        CurSMT == SMT_CloseDoor            ||
        CurSMT == SMT_CloseDoorPositionned ||
        CurSMT == SMT_ClearClosingDoor     ||
        CurSMT == SMT_DoorClosedFromOtherSide)
    {
        P.DoorOpeningType       = (BYTE)Hero->DoorOpeningType;
        P.DoorPartialOpenType   = (BYTE)Hero->DoorPartialOpenType;
        P.DoorClosingType       = (BYTE)Hero->DoorClosingType;
        P.bQuietDoorInteraction = Hero->bQuietDoorInteraction ? 1 : 0;
    }

    // CSA params (SMT_CSA) — always send anim; SyncMatinees gates only the Kismet event on receive
    if (CurSMT == SMT_CSA && Hero->ActiveCSA)
    {
        if (Hero->ActiveCSA->AnimName != NAME_None)
        {
            FString AnimStr = Hero->ActiveCSA->AnimName.ToString();
            INT Len = Min(AnimStr.Len(), 31);
            P.CSAAnimLen = (BYTE)Len;
            for (INT i = 0; i < Len; i++)
                P.CSAAnimName[i] = (BYTE)((*AnimStr)[i] & 0x7F);
        }
        FString PathStr = Hero->ActiveCSA->GetPathName();
        INT PLen = Min(PathStr.Len(), 127);
        P.CSAPathLen = (BYTE)PLen;
        for (INT i = 0; i < PLen; i++)
            P.CSAPath[i] = (BYTE)((*PathStr)[i] & 0x7F);
    }

    // Struggle params (SMT_EnterStruggle) — send entry + cycle anim names so dummy can play them
    if (CurSMT == SMT_EnterStruggle && Hero->OLPC)
    {
        auto PackName = [](FName N, BYTE* LenOut, BYTE* Buf, INT MaxLen)
        {
            FString S = N.ToString();
            INT Len = Min(S.Len(), MaxLen);
            *LenOut = (BYTE)Len;
            for (INT i = 0; i < Len; i++)
                Buf[i] = (BYTE)((*S)[i] & 0x7F);
        };
        const FStruggleConfig& Cfg = Hero->OLPC->Struggle.Config;
        PackName(Cfg.EntryAnimPlayer,  &P.StruggleEntryAnimPlayerLen, P.StruggleEntryAnimPlayer, 63);
        PackName(Cfg.CycleAnimPlayer,  &P.StruggleCycleAnimPlayerLen, P.StruggleCycleAnimPlayer, 63);
        PackName(Cfg.CycleAnimEnemy,   &P.StruggleCycleAnimEnemyLen,  P.StruggleCycleAnimEnemy,  63);

        // Pack the first HeroAnimSet path so receiver can load it (mirrors Cinematic approach).
        if (Cfg.HeroAnimSets.Num() > 0 && Cfg.HeroAnimSets(0))
        {
            FString SetPath = Cfg.HeroAnimSets(0)->GetPathName();
            INT Len = Min(SetPath.Len(), 127);
            P.StruggleAnimSetPathLen = (BYTE)Len;
            for (INT i = 0; i < Len; i++)
                P.StruggleAnimSetPath[i] = (BYTE)((*SetPath)[i] & 0x7F);
        }
    }

    // ContextualLean params (SMT_EnterContextualLean)
    if (CurSMT == SMT_EnterContextualLean)
    {
        UBOOL bFromLeft = (Hero->CornerPeek.PeekPosition == CPP_Left || Hero->CornerPeek.PeekPosition == CPP_MiddleLeft);
        P.bPeekFromLeft = bFromLeft ? 1 : 0;
        P.bPeekRounded  = Hero->CornerPeek.bRoundedCorner ? 1 : 0;
        P.CornerLocX    = Hero->CornerPeek.CornerLocation.X;
        P.CornerLocY    = Hero->CornerPeek.CornerLocation.Y;
        P.CornerLocZ    = Hero->CornerPeek.CornerLocation.Z;
        P.CornerFwdX    = Hero->CornerPeek.FwdDir.X;
        P.CornerFwdY    = Hero->CornerPeek.FwdDir.Y;
        P.CornerFwdZ    = Hero->CornerPeek.FwdDir.Z;
    }

    // Pickup params (SMT 49)
    if (CurSMT == SMT_PickupObject && Hero->ActivePickup)
    {
        FVector ToPickup       = Hero->ActivePickup->Location - Hero->Location;
        P.PickupDist2DX10      = (INT)(appSqrt(ToPickup.X*ToPickup.X + ToPickup.Y*ToPickup.Y) * 10.f);
        P.PickupDeltaZX10      = (INT)(ToPickup.Z * 10.f);
        P.bPickupCrouched      = Hero->bPickupCrouched ? 1 : 0;
        P.bPickupIsCollectible = Hero->ActivePickup->IsA(AOLCollectiblePickup::StaticClass()) ? 1 : 0;
    }

    BYTE B[2 + sizeof(FSmtTypePacket)];
    INT  N = 0;
    N = PutU8(B, N, CH_HERO);
    N = PutU8(B, N, HERO_SMT_TYPE);
    appMemcpy(B + N, &P, sizeof(P));
    N += (INT)sizeof(P);
    GMpConn.SendBinary(B, N);
}

void MpSendCSAActivation(AOLCSA* CSA, const FVector& AnimStart, const FVector& AnimFwd)
{
    UHeroChannel* Ch = GHeroChannelTicker.Channel;
    if (!Ch || !CanSend() || !CSA) return;

    FString CSAPath = CSA->GetPathName();
    if (Ch->ControllerOwner->CSAActBlacklist.FindItemIndex(CSAPath) != INDEX_NONE)
        return;

    // Temporarily override LastGrabPos/LastGrabDir so BuildAndSendSmtTransition
    // encodes the correct CSA animation start position and direction.
    FVector SavedGrabPos = GMultiplayerHero->LastGrabPos;
    FVector SavedGrabDir = GMultiplayerHero->LastGrabDir;
    GMultiplayerHero->LastGrabPos = AnimStart;
    GMultiplayerHero->LastGrabDir = AnimFwd;

    AOLCSA* SavedCSA = GMultiplayerHero->ActiveCSA;
    GMultiplayerHero->ActiveCSA = CSA;
    BuildAndSendSmtTransition(Ch, SMT_CSA);
    GMultiplayerHero->ActiveCSA = SavedCSA;

    GMultiplayerHero->LastGrabPos = SavedGrabPos;
    GMultiplayerHero->LastGrabDir = SavedGrabDir;

    Ch->LastSentSpecialMove = SMT_CSA;
}

void UHeroChannel::SendSpecialMoveType()
{
    if (!CanSend()) return;

    INT CurSMT = (INT)GMultiplayerHero->SpecialMove;

    if (CurSMT == SMT_JumpOver || CurSMT == SMT_SlideOver)
    {
        if (!bSendingJumpGroundZ) { JumpGroundZ = GMultiplayerHero->Location.Z; bSendingJumpGroundZ = TRUE; }
    }
    else
        bSendingJumpGroundZ = FALSE;

    // SMT_Crouch/Uncrouch are not sent as SMT_TYPE — the receiver's physWalking
    // triggers OnCrouch/OnUncrouch natively when bWantsToCrouch changes via MPKT_STATE.
    // SMT_CSA is sent event-driven via MpSendCSAActivation (called from TryCSA) — not from the tick,
    // because instant CSA clears ActiveCSA before the tick runs.
    if (CurSMT != LastSentSpecialMove
        && CurSMT != SMT_None && CurSMT != SMT_Crouch && CurSMT != SMT_Uncrouch
        && CurSMT != SMT_CSA)
    {
        BuildAndSendSmtTransition(this, CurSMT);
        if (CurSMT == SMT_PickupObject)
            SendPickupStart(CurSMT);
    }

    SendPickupState(CurSMT);
    LastSentSpecialMove = CurSMT;
}
static void SendPickupLocPacket(BYTE PktType, const FVector& Loc)
{
    INT IX = (INT)Loc.X, IY = (INT)Loc.Y, IZ = (INT)Loc.Z;
    BYTE B[14];
    B[0]  = CH_WORLD;
    B[1]  = PktType;
    B[2]  = (BYTE)(IX        & 0xFF); B[3]  = (BYTE)((IX >>  8) & 0xFF);
    B[4]  = (BYTE)((IX >> 16) & 0xFF); B[5]  = (BYTE)((IX >> 24) & 0xFF);
    B[6]  = (BYTE)(IY        & 0xFF); B[7]  = (BYTE)((IY >>  8) & 0xFF);
    B[8]  = (BYTE)((IY >> 16) & 0xFF); B[9]  = (BYTE)((IY >> 24) & 0xFF);
    B[10] = (BYTE)(IZ        & 0xFF); B[11] = (BYTE)((IZ >>  8) & 0xFF);
    B[12] = (BYTE)((IZ >> 16) & 0xFF); B[13] = (BYTE)((IZ >> 24) & 0xFF);
    GMpConn.SendBinary(B, 14);
}

void UHeroChannel::SendPickupStart(INT CurSMT)
{
    if (!GMpConn.SyncPickups) return;
    AOLHero* Hero = Cast<AOLHero>(HeroPawn);
    if (!Hero || !Hero->ActivePickup) return;
    LastPickupLoc = Hero->ActivePickup->Location;
    SendPickupLocPacket(WORLD_PICKUP_START, LastPickupLoc);
}

void UHeroChannel::SendPickupState(INT CurSMT)
{
    if (!GMpConn.SyncPickups) return;
    AOLHero* Hero = Cast<AOLHero>(HeroPawn);
    if (!Hero) return;

    // Attach pickup mesh to dummy hand when anim notify fires
    if (CurSMT == SMT_PickupObject && Hero->bPickupNotifyFired)
    {
        SendPickupLocPacket(WORLD_PICKUP_ATTACH, LastPickupLoc);
        Hero->bPickupNotifyFired = FALSE;
    }

    // When SMT_PickupObject ends, signal remote to finalise pickup
    if (LastSentSpecialMove == SMT_PickupObject && CurSMT != SMT_PickupObject)
        SendPickupLocPacket(WORLD_PICKUP_STATE, LastPickupLoc);
}

void UHeroChannel::SendCornerPeekData() {}

// ============================================================================
// Send: PLAYER_HIT / PLAYER_GRAB / PLAYER_THROW / PLAYER_KILL  (binary)
// ============================================================================

static void SendPlayerEvent(FPlayerEventPacket& Pkt)
{
    if (!GMpConn.bIsHandshaked) return;
    BYTE B[2 + sizeof(FPlayerEventPacket)];
    B[0] = CH_HERO;
    B[1] = HERO_PLAYER_EVENT;
    appMemcpy(B + 2, &Pkt, sizeof(Pkt));
    GMpConn.SendBinary(B, 2 + sizeof(Pkt));
}

void UHeroChannel::SendPlayerHit(INT TargetPlayerID, FLOAT Damage, FLOAT KnockbackPower, FVector HitDir)
{
    if (!GMpConn.SyncEnemies) return;
    FPlayerEventPacket Pkt;
    appMemzero(&Pkt, sizeof(Pkt));
    Pkt.TargetPlayerID  = TargetPlayerID;
    Pkt.EventType       = PEVT_Hit;
    Pkt.DamageX1        = (INT)Damage;
    Pkt.KnockbackX1     = (INT)KnockbackPower;
    Pkt.HitDirX1000[0]  = (INT)(HitDir.X * 1000.f);
    Pkt.HitDirX1000[1]  = (INT)(HitDir.Y * 1000.f);
    Pkt.HitDirX1000[2]  = (INT)(HitDir.Z * 1000.f);
    SendPlayerEvent(Pkt);
}

void UHeroChannel::SendPlayerGrab(INT TargetPlayerID, FVector GrabTargetLoc, FVector CharDir,
    UBOOL bCrouched, INT EnemyTypeInt, FLOAT BlendAlpha, UBOOL bLeftAnim, INT GrabType)
{
    FPlayerEventPacket Pkt;
    appMemzero(&Pkt, sizeof(Pkt));
    Pkt.TargetPlayerID    = TargetPlayerID;
    Pkt.EventType         = PEVT_Grab;
    Pkt.LocX10[0]         = (INT)(GrabTargetLoc.X * 10.f);
    Pkt.LocX10[1]         = (INT)(GrabTargetLoc.Y * 10.f);
    Pkt.LocX10[2]         = (INT)(GrabTargetLoc.Z * 10.f);
    Pkt.DirX10000[0]      = (INT)(CharDir.X * 10000.f);
    Pkt.DirX10000[1]      = (INT)(CharDir.Y * 10000.f);
    Pkt.DirX10000[2]      = (INT)(CharDir.Z * 10000.f);
    Pkt.bCrouched         = bCrouched ? 1 : 0;
    Pkt.EnemyTypeInt      = EnemyTypeInt;
    Pkt.BlendAlphaX10000  = (INT)(BlendAlpha * 10000.f);
    Pkt.bLeftAnim         = bLeftAnim ? 1 : 0;
    Pkt.GrabType          = GrabType;
    SendPlayerEvent(Pkt);
}

void UHeroChannel::SendPlayerThrow(INT TargetPlayerID, FLOAT ThrowRotation)
{
    FPlayerEventPacket Pkt;
    appMemzero(&Pkt, sizeof(Pkt));
    Pkt.TargetPlayerID  = TargetPlayerID;
    Pkt.EventType       = PEVT_Throw;
    Pkt.ThrowRotX100000 = (INT)(ThrowRotation * 100000.f);
    SendPlayerEvent(Pkt);
}

void UHeroChannel::SendPlayerKill(INT TargetPlayerID, INT EnemyTypeInt, INT WeaponType,
    UBOOL bBackAnim, UBOOL bLeftAnim, FLOAT BlendAlpha,
    FVector AnimStart, FVector CharDir, INT KillType, INT VictimYaw)
{
    FPlayerEventPacket Pkt;
    appMemzero(&Pkt, sizeof(Pkt));
    Pkt.TargetPlayerID   = TargetPlayerID;
    Pkt.EventType        = PEVT_Kill;
    Pkt.EnemyTypeInt     = EnemyTypeInt;
    Pkt.WeaponType       = WeaponType;
    Pkt.bBackAnim        = bBackAnim ? 1 : 0;
    Pkt.bLeftAnim        = bLeftAnim ? 1 : 0;
    Pkt.BlendAlphaX10000 = (INT)(BlendAlpha * 10000.f);
    Pkt.LocX10[0]        = (INT)(AnimStart.X * 10.f);
    Pkt.LocX10[1]        = (INT)(AnimStart.Y * 10.f);
    Pkt.LocX10[2]        = (INT)(AnimStart.Z * 10.f);
    Pkt.DirX10000[0]     = (INT)(CharDir.X * 10000.f);
    Pkt.DirX10000[1]     = (INT)(CharDir.Y * 10000.f);
    Pkt.DirX10000[2]     = (INT)(CharDir.Z * 10000.f);
    Pkt.KillType         = KillType;
    Pkt.VictimYaw        = VictimYaw;
    SendPlayerEvent(Pkt);
}

// ============================================================================
// Receive: PLAYER_EVENT binary (HIT / GRAB / THROW / KILL)
// ============================================================================

static AOLHero* GetLocalHeroForTarget(INT TargetPlayerID)
{
    if (TargetPlayerID != GMpConn.LocalPlayerID) return NULL;
    if (!GMultiplayerController || !GMultiplayerController->Pawn) return NULL;
    AOLHero* Hero = Cast<AOLHero>(GMultiplayerController->Pawn);
    if (!Hero || Hero->Health <= 0) return NULL;
    return Hero;
}

void UHeroChannel::OnBinaryPlayerEvent(INT SenderID, BYTE* Data, INT DataLen)
{
    if (DataLen < (INT)sizeof(FPlayerEventPacket)) return;
    const FPlayerEventPacket& Pkt = *reinterpret_cast<const FPlayerEventPacket*>(Data);

    AOLHero* Hero = GetLocalHeroForTarget(Pkt.TargetPlayerID);
    if (!Hero) return;

    switch (Pkt.EventType)
    {
    case PEVT_Hit:
    {
        if (!GMpConn.SyncEnemies) return;
        FLOAT Damage    = (FLOAT)Pkt.DamageX1;
        FLOAT Knockback = (FLOAT)Pkt.KnockbackX1;
        FVector HitDir(Pkt.HitDirX1000[0] / 1000.f,
                       Pkt.HitDirX1000[1] / 1000.f,
                       Pkt.HitDirX1000[2] / 1000.f);
        UClass* DmgClass = FindObject<UClass>(ANY_PACKAGE, TEXT("DamageType"), FALSE);
        Hero->NativeTakeDamage((INT)Damage, NULL, Hero->Location, DmgClass);
        if (Knockback > 0.f)
        {
            struct { FLOAT KnockbackPower; FVector HitDir; } Parms;
            Parms.KnockbackPower = Knockback;
            Parms.HitDir = HitDir;
            Hero->ReactToHit(Knockback, HitDir);
        }
        break;
    }
    case PEVT_Grab:
    {
        if (!GMpConn.SyncEnemies) return;
        FVector GrabTargetLoc(Pkt.LocX10[0] / 10.f,
                              Pkt.LocX10[1] / 10.f,
                              Pkt.LocX10[2] / 10.f);
        FVector CharDir(Pkt.DirX10000[0] / 10000.f,
                        Pkt.DirX10000[1] / 10000.f,
                        Pkt.DirX10000[2] / 10000.f);
        FLOAT BlendAlpha = Pkt.BlendAlphaX10000 / 10000.f;
        UBOOL bLeftAnim  = Pkt.bLeftAnim != 0;

        Hero->SetDummyKillParams(Pkt.EnemyTypeInt, 0, FALSE, bLeftAnim, BlendAlpha);

        switch (Pkt.GrabType)
        {
        case 1: // Squeeze
            Hero->StartDummyGrabbedFromSqueeze(bLeftAnim);
            break;
        case 2: // Locker
            Hero->StartSpecialMove(64, FVector(0,0,0), FVector(0,0,0));
            break;
        case 3: // Bed
            Hero->StartSpecialMove(65, FVector(0,0,0), FVector(0,0,0));
            break;
        case 4: // Under
            Hero->StartSpecialMove(66, GrabTargetLoc, CharDir);
            break;
        default: // Normal
            Hero->StartSpecialMove(62, GrabTargetLoc, CharDir);
            break;
        }
        break;
    }
    case PEVT_Throw:
    {
        if (!GMpConn.SyncEnemies) return;
        Hero->SpecialMoveTargetYaw = Pkt.ThrowRotX100000 / 100000.f;
        Hero->StartSpecialMove(67, FVector(0,0,0), FVector(0,0,0));
        break;
    }
    case PEVT_Kill:
    {
        if (!GMpConn.SyncEnemies) return;
        FVector AnimStart(Pkt.LocX10[0] / 10.f,
                          Pkt.LocX10[1] / 10.f,
                          Pkt.LocX10[2] / 10.f);
        FVector CharDir(Pkt.DirX10000[0] / 10000.f,
                        Pkt.DirX10000[1] / 10000.f,
                        Pkt.DirX10000[2] / 10000.f);
        FLOAT BlendAlpha = Pkt.BlendAlphaX10000 / 10000.f;

        Hero->SetDummyKillParams(Pkt.EnemyTypeInt, Pkt.WeaponType,
                                 Pkt.bBackAnim != 0, Pkt.bLeftAnim != 0, BlendAlpha);

        if (Pkt.KillType == 1)
        {
            Hero->StartSpecialMove(68, FVector(0,0,0), FVector(0,0,0));
        }
        else if (!AnimStart.IsZero())
        {
            // Use the current client rotation — after Grab the victim is already correctly
            // oriented (we snapped to CharDir in PEVT_Grab). Restoring VictimYaw from the
            // server packet would overwrite that with a stale value and cause a spin.
            Hero->StartSpecialMove(69, AnimStart, Hero->Rotation.Vector(), 0);
        }
        else
        {
            Hero->StartSpecialMove(69, FVector(0,0,0), FVector(0,0,0));
        }
        break;
    }
    default: break;
    }
}

// ============================================================================
// Send/Receive: PLAYER_LIFECYCLE binary (Died / Respawned)
// ============================================================================

void UHeroChannel::SendPlayerDied()
{
    if (!GMpConn.bIsHandshaked) return;
    BYTE B[3] = { CH_HERO, HERO_PLAYER_LIFECYCLE, LIFECYCLE_DIED };
    GMpConn.SendBinary(B, 3);
    LastSentSpecialMove   = -1;
    LastSentMeshPreset    = -1;
    LastSentCinematicAnim = TEXT("");
    bSendingJumpGroundZ   = FALSE;
}

void UHeroChannel::SendPlayerRespawned()
{
    if (!GMpConn.bIsHandshaked) return;
    BYTE B[3] = { CH_HERO, HERO_PLAYER_LIFECYCLE, LIFECYCLE_RESPAWNED };
    GMpConn.SendBinary(B, 3);
    LastSentSpecialMove   = -1;
    LastSentMeshPreset    = -1;
    LastSentCinematicAnim = TEXT("");
    bSendingJumpGroundZ   = FALSE;
}

void UHeroChannel::SendDisconnect()
{
    if (!GMpConn.bIsConnected) return;
    BYTE B[2] = { CH_WORLD, WORLD_DISCONNECT };
    GMpConn.SendBinary(B, 2);
}

void UHeroChannel::SendRequestEnemies()
{
    if (!GMpConn.bIsHandshaked) return;
    BYTE B[2] = { CH_WORLD, WORLD_REQUEST_ENEMIES };
    GMpConn.SendBinary(B, 2);
}

void UHeroChannel::SendRequestDoors()
{
    if (!GMpConn.bIsHandshaked) return;
    BYTE B[2] = { CH_WORLD, WORLD_REQUEST_DOORS };
    GMpConn.SendBinary(B, 2);
}

void UHeroChannel::SendRequestPushables()
{
    if (!GMpConn.bIsHandshaked) return;
    BYTE B[2] = { CH_WORLD, WORLD_REQUEST_PUSHABLES };
    GMpConn.SendBinary(B, 2);
}

void UHeroChannel::OnBinaryPlayerLifecycle(INT SenderID, BYTE* Data, INT DataLen)
{
    if (DataLen < 1) return;
    BYTE Type = Data[0];

    AMultiplayerController* Controller = Cast<AMultiplayerController>(ControllerOwner);
    if (!Controller) return;

    INT Idx = Controller->FindRemoteIndex(SenderID);
    if (Idx < 0) return;

    URemotePlayer* P = Controller->RemotePlayers(Idx);

    if (Type == LIFECYCLE_DIED)
    {
        if (P->DummyPlayer)
        {
            GWorld->DestroyActor(P->DummyPlayer);
            P->DummyPlayer = NULL;
        }
        P->LastRemoteSpecialMove    = 0;
        P->LastRemoteLocomotionMode = 0;
        P->DummySMTLockUntil        = 0.f;
        P->bHasReceivedData         = FALSE;
    }
    else if (Type == LIFECYCLE_RESPAWNED)
    {
        if (P->DummyPlayer)
        {
            GWorld->DestroyActor(P->DummyPlayer);
            P->DummyPlayer = NULL;
        }
        P->LastRemoteSpecialMove    = 0;
        P->LastRemoteLocomotionMode = 0;
        P->DummySMTLockUntil        = 0.f;
        P->bHasReceivedData         = FALSE;

        if (Controller->Pawn)
        {
            P->DummyPlayer = SpawnDummy(Controller);
            AOLHero* NewDummy = Cast<AOLHero>(P->DummyPlayer);
            if (NewDummy) NewDummy->DummyOwnerID = P->PlayerID;
            AOLHero* RespawnDummy = Cast<AOLHero>(P->DummyPlayer);
            if (RespawnDummy)
            {
                if (RespawnDummy->Mesh)          RespawnDummy->Mesh->SetHiddenGame(TRUE);
                if (RespawnDummy->DummyHeadMesh) RespawnDummy->DummyHeadMesh->SetHiddenGame(TRUE);
                RespawnDummy->Health = 0;
            }
        }

        // Rebuild remote enemy list for the respawned player's world state
        Controller->NativeDestroyRemoteEnemies();
        if (GMpConn.bIsConnected && GMpConn.SyncEnemies)
            { BYTE B[2] = { CH_WORLD, WORLD_REQUEST_ENEMIES }; GMpConn.SendBinary(B, 2); }
    }
}

// ============================================================================
// Receive: HEAD_ROT binary packet
// ============================================================================

void UHeroChannel::OnBinaryHeadRot(INT SenderID, BYTE* Data, INT DataLen)
{
    if (DataLen < (INT)sizeof(FHeadRotPacket))
        return;

    AMultiplayerController* Controller = Cast<AMultiplayerController>(ControllerOwner);
    if (!Controller)
        return;

    INT Idx = Controller->FindRemoteIndex(SenderID);
    if (Idx == -1)
        return;

    URemotePlayer* P = Controller->RemotePlayers(Idx);

    INT Pitch, Yaw;
    ReadI32(Data, 0, Pitch);
    ReadI32(Data, 4, Yaw);
    P->LastRemoteCamPitch = Pitch;
    P->LastRemoteCamYaw   = Yaw;

    AOLHero* Dummy = Cast<AOLHero>(P->DummyPlayer);
    if (Dummy)
        Dummy->SetDummyHeadPitch(Pitch, Yaw);
}

// ============================================================================
// Receive: MESH_PRESET binary packet
// ============================================================================

void UHeroChannel::OnBinaryMesh(INT SenderID, BYTE* Data, INT DataLen)
{
    if (DataLen < 1)
        return;

    AMultiplayerController* Controller = Cast<AMultiplayerController>(ControllerOwner);
    if (!Controller)
        return;

    INT Idx = Controller->FindRemoteIndex(SenderID);
    if (Idx == -1)
        return;

    URemotePlayer* P = Controller->RemotePlayers(Idx);
    AOLHero* Dummy   = Cast<AOLHero>(P->DummyPlayer);
    if (Dummy)
        Dummy->SetDummyMeshPreset((INT)Data[0]);
}

// ============================================================================
// Receive: CINEMATIC_ANIM binary packet
// ============================================================================

void UHeroChannel::OnBinaryCinematicAnim(INT SenderID, BYTE* Data, INT DataLen)
{
    if (DataLen < 1)
        return;

    AMultiplayerController* Controller = Cast<AMultiplayerController>(ControllerOwner);
    if (!Controller)
        return;

    INT Idx = Controller->FindRemoteIndex(SenderID);
    if (Idx == -1)
        return;

    URemotePlayer* P = Controller->RemotePlayers(Idx);
    AOLHero* Dummy   = Cast<AOLHero>(P->DummyPlayer);
    if (!Dummy)
        return;

    BYTE bStop = Data[0];
    if (bStop)
    {
        Dummy->ClearFullBodyIdleAnim();
        return;
    }

    // [1] = path length, [2..] = "Package.Name|AnimSeqName"
    if (DataLen < 3)
        return;

    INT   PathLen = (INT)Data[1];
    if (PathLen <= 0 || PathLen > DataLen - 2)
        return;

    // Build FString from ASCII bytes, then extract AnimSeqName after '|'
    TCHAR PathBuf[258];
    for (INT i = 0; i < PathLen && i < 256; i++)
        PathBuf[i] = (TCHAR)Data[2 + i];
    PathBuf[PathLen] = 0;
    FString FullPath(PathBuf);

    INT PipeIdx = FullPath.InStr(TEXT("|"));
    FName AnimName;
    if (PipeIdx != INDEX_NONE)
        AnimName = FName(*FullPath.Mid(PipeIdx + 1));
    else
        AnimName = FName(*FullPath);

    if (AnimName == NAME_None)
        return;

    // Load the AnimSet and attach it to dummy's mesh (mirrors old UC ApplyCinematicAnim).
    if (PipeIdx > 0)
    {
        FString AnimSetPath = FullPath.Left(PipeIdx);
        UAnimSet* AnimSet = LoadObject<UAnimSet>(NULL, *AnimSetPath, NULL, LOAD_None, NULL);
        if (AnimSet && Dummy->Mesh)
        {
            if (Dummy->Mesh->AnimSets.FindItemIndex(AnimSet) == INDEX_NONE)
                Dummy->Mesh->AnimSets.AddItem(AnimSet);
            Dummy->Mesh->UpdateAnimations();
        }
    }

    // Stop previous cinematic anim (fires looping sound stop-events like Wheelchair_LOOP_STOP)
    // before starting the new one, so sounds from the outgoing anim are properly cleaned up.
    if (Dummy->FullBodyAnimSlot && Dummy->FullBodyAnimSlot->bIsPlayingCustomAnim)
        Dummy->ClearFullBodyIdleAnim();

    Dummy->PlayCinematicDummyAnim(AnimName, 1.f, 0.2f, 0.2f);
}

// ============================================================================
// Receive: SMT_TYPE binary packet — stores pre-params, triggers SMT on next LOC
// ============================================================================

void UHeroChannel::OnBinarySmtType(INT SenderID, BYTE* Data, INT DataLen)
{
    if (DataLen < (INT)sizeof(FSmtTypePacket))
        return;

    AMultiplayerController* Controller = Cast<AMultiplayerController>(ControllerOwner);
    if (!Controller)
        return;

    INT Idx = Controller->FindRemoteIndex(SenderID);
    if (Idx == -1)
        return;

    URemotePlayer* P = Controller->RemotePlayers(Idx);
    AOLHero* Dummy = Cast<AOLHero>(P->DummyPlayer);
    if (!Dummy)
        return;

    const FSmtTypePacket& Pkt = *(const FSmtTypePacket*)(Data);
    const INT SMT = Pkt.SMT;
    if (SMT <= SMT_None || SMT >= ESpecialMoveType_MAX)
        return;

    // Cancel any pending deferred SMT if a new SMT arrives.
    P->PendingSMT = SMT_None;

    FVector GrabPos(Pkt.GrabPosX, Pkt.GrabPosY, Pkt.GrabPosZ);
    FVector GrabDir(Pkt.GrabDirX, Pkt.GrabDirY, Pkt.GrabDirZ);

    const FSpecialMoveParameters& SMP = Dummy->SpecialMoveParams[SMT];

    switch ((ESpecialMoveType)SMT)
    {
    case SMT_JumpOver:
        Dummy->bRunningTraversalMove = Pkt.bRunningTraversalMove != 0;
        break;

    case SMT_ClimbUpObstacle:
        Dummy->bMustCrouchAfterSpecialMove = Pkt.bMustCrouchAfterSMT  != 0;
        Dummy->bRunningTraversalMove       = Pkt.bRunningTraversalMove != 0;
        break;

    case SMT_LedgeHangTransition:
    case SMT_EnterLedgeWalk:
    case SMT_ExitLedgeWalk:
    case SMT_LedgeWalkTransition:
        Dummy->ActiveLedgeTransitionType = (ELedgeTransitionType)Pkt.LedgeTransitionType;
        break;

    case SMT_ClimbUpLedge:
        switch (Pkt.LedgeClimbType)
        {
            case LCT_ClimbUpToCrouch:    Dummy->LedgeClimbType = LCT_ClimbUpToCrouch;    break;
            case LCT_ClimbOverToFalling: Dummy->LedgeClimbType = LCT_ClimbOverToFalling; break;
            case LCT_ClimbOverToStand:   Dummy->LedgeClimbType = LCT_ClimbOverToStand;   break;
            default:                     Dummy->LedgeClimbType = LCT_ClimbUpToStand;     break;
        }
        break;

    case SMT_EnterSqueeze:
    case SMT_ExitSqueeze:
    case SMT_AutomaticSqueeze:
        Dummy->SetDummyActiveSqueeze(GrabPos);
        if ((ESpecialMoveType)SMT == SMT_EnterSqueeze)
            Dummy->bLeftAnim = Pkt.bLeftAnim != 0;
        break;

    case SMT_OpenDoorInstant:
    case SMT_OpenDoorPartial:
    case SMT_TryOpenLockedDoor:
    case SMT_RunThroughDoor:
    case SMT_CloseDoor:
    case SMT_CloseDoorPositionned:
    case SMT_ClearClosingDoor:
    case SMT_DoorClosedFromOtherSide:
        P->LastRemoteDoorOpeningType     = (INT)Pkt.DoorOpeningType;
        P->LastRemoteDoorPartialOpenType = (INT)Pkt.DoorPartialOpenType;
        P->LastRemoteDoorClosingType     = (INT)Pkt.DoorClosingType;
        P->bLastRemoteDoorQuiet          = Pkt.bQuietDoorInteraction != 0;
        Dummy->SetDummyDoorParams(P->LastRemoteDoorOpeningType, P->LastRemoteDoorPartialOpenType,
                                  P->LastRemoteDoorClosingType, P->bLastRemoteDoorQuiet);
        Dummy->SetDummyLocomotionMode(LM_Walk);
        break;

    case SMT_EnterLocker:
    case SMT_ExitLocker:
        Dummy->SetDummyActiveLocker(GrabPos);
        break;

    case SMT_EnterBed:
    case SMT_ExitBed:
    case SMT_OpenLockerFromOutside:
        Dummy->bLeftAnim = Pkt.bLeftAnim != 0;
        Dummy->SetDummyLocomotionMode(LM_Walk);
        break;

    case SMT_EnterStruggle:
    {
        // Load the HeroAnimSet so struggle anims are available on dummy's mesh
        // (mirrors Cinematic: LoadObject<UAnimSet> + UpdateAnimations).
        if (Pkt.StruggleAnimSetPathLen > 0 && Dummy->Mesh)
        {
            TCHAR PathBuf[128] = {0};
            for (INT i = 0; i < Pkt.StruggleAnimSetPathLen && i < 127; i++)
                PathBuf[i] = (TCHAR)Pkt.StruggleAnimSetPath[i];
            debugf(TEXT("[Struggle] AnimSetPath='%s'"), PathBuf);
            UAnimSet* AnimSet = LoadObject<UAnimSet>(NULL, PathBuf, NULL, LOAD_None, NULL);
            if (AnimSet)
            {
                debugf(TEXT("[Struggle] AnimSet loaded OK"));
                if (Dummy->Mesh->AnimSets.FindItemIndex(AnimSet) == INDEX_NONE)
                    Dummy->Mesh->AnimSets.AddItem(AnimSet);
                Dummy->Mesh->UpdateAnimations();
            }
            else
            {
                debugf(TEXT("[Struggle] AnimSet FAILED to load"));
            }
        }
        else
        {
            debugf(TEXT("[Struggle] AnimSetPathLen=%d, no AnimSet to load"), (INT)Pkt.StruggleAnimSetPathLen);
        }

        // Unpack anim names from the packet into dummy fields.
        // OLAnimStruggleCycle::OnBecomeRelevant and StartSpecialMove case will read them.
        auto UnpackName = [](const BYTE* Buf, BYTE Len) -> FName
        {
            TCHAR Tmp[64] = {0};
            for (INT i = 0; i < Len && i < 63; i++)
                Tmp[i] = (TCHAR)Buf[i];
            return (Len > 0) ? FName(Tmp) : NAME_None;
        };
        Dummy->DummyStruggleEntryAnimPlayer = UnpackName(Pkt.StruggleEntryAnimPlayer, Pkt.StruggleEntryAnimPlayerLen);
        Dummy->DummyStruggleCycleAnimPlayer = UnpackName(Pkt.StruggleCycleAnimPlayer, Pkt.StruggleCycleAnimPlayerLen);
        Dummy->DummyStruggleCycleAnimEnemy  = UnpackName(Pkt.StruggleCycleAnimEnemy,  Pkt.StruggleCycleAnimEnemyLen);

        // If LM_Struggle is already active (LOC arrived before SMT), start cycle loop now.
        // Otherwise ApplyLocomotionMode will start it when LM_Struggle arrives.
        if (Dummy->LocomotionMode == LM_Struggle
            && Dummy->DummyStruggleCycleAnimPlayer != NAME_None
            && Dummy->FullBodyAnimSlot)
        {
            Dummy->FullBodyAnimSlot->PlayCustomAnim(
                Dummy->DummyStruggleCycleAnimPlayer,
                Dummy->DummyStrugglePlayRate > 0.f ? Dummy->DummyStrugglePlayRate : 1.f,
                0.1f, 0.0f, TRUE, FALSE);
        }
        break;
    }

    case SMT_EnterLadderFromAbove:
        Dummy->SetDummyLocomotionMode(LM_Walk);
        break;

    case SMT_EnterLadderFromGround:
    case SMT_ExitLadderOnGround:
    case SMT_ExitLadderOnTop:
        if ((ESpecialMoveType)SMT == SMT_ExitLadderOnTop)
            Dummy->bExitLadderLeftHand = Pkt.bExitLadderLeftHand != 0;
        Dummy->SetDummyLocomotionMode(LM_Ladder);
        break;

    case SMT_StartPushingObject:
    case SMT_StopPushingObject:
        P->bLastRemotePushFromBack  = Pkt.bPushingFromBackEdge != 0;
        Dummy->bPushingFromBackEdge = Pkt.bPushingFromBackEdge != 0;
        Dummy->SetDummyActivePushable(GrabPos);
        break;

    case SMT_EnterContextualLean:
        {
            UBOOL bFromLeft = Pkt.bPeekFromLeft != 0;
            UBOOL bRounded  = Pkt.bPeekRounded  != 0;
            FVector cornerLoc(Pkt.CornerLocX, Pkt.CornerLocY, Pkt.CornerLocZ);
            FVector cornerFwd(Pkt.CornerFwdX, Pkt.CornerFwdY, Pkt.CornerFwdZ);
            // Cache for ApplyLocomotionChange(LM_ContextualLean) that may follow.
            P->bLastRemotePeekFromLeft  = bFromLeft;
            P->bLastRemotePeekRounded   = bRounded;
            P->LastRemoteCornerLocation = cornerLoc;
            P->LastRemoteCornerFwdDir   = cornerFwd;
            if (Dummy->PeekingAnimNode && Dummy->ShadowProxyPeekingAnimNode)
            {
                Dummy->PeekingAnimNode->SetPeekingType(bFromLeft, bRounded);
                Dummy->ShadowProxyPeekingAnimNode->SetPeekingType(bFromLeft, bRounded);
                Dummy->PeekingAnimNode->StartPeeking(0.f);
                Dummy->ShadowProxyPeekingAnimNode->StartPeeking(0.f);
            }
            Dummy->CornerPeek.CornerLocation  = cornerLoc;
            Dummy->CornerPeek.FwdDir          = cornerFwd;
            {
                FVector right = (cornerFwd ^ FVector(0,0,1)).SafeNormal();
                Dummy->CornerPeek.SideDir = bFromLeft ? -right : right;
            }
            Dummy->CornerPeek.PeekPosition   = bFromLeft ? CPP_Left : CPP_Right;
            Dummy->CornerPeek.bRoundedCorner = bRounded;
            Dummy->CornerPeek.CornerMarker   = FindCornerMarkerNear(Dummy, cornerLoc);
        }
        break;

    case SMT_GrabLedgeFromAir:
        Dummy->SpecialMoveBlendAlpha = (FLOAT)Pkt.BlendAlphaX1000 / 1000.f;
        break;

    case SMT_HeroGrabbedNormal:
        Dummy->bLeftAnim = Pkt.bLeftAnim != 0;
        Dummy->SetDummyCrouched(Pkt.bIsCrouched != 0);
        break;

    case SMT_HeroThrown:
        Dummy->SpecialMoveTargetYaw = (FLOAT)Pkt.SpecialMoveTargetYawX1000 / 1000.f;
        break;

    case SMT_HeroDecapitate:
    case SMT_HeroKilled:
        Dummy->SetDummyKillParams(Pkt.EnemyType, Pkt.EnemyWeapon,
                                  Pkt.bBackAnim != 0, Pkt.bLeftAnim != 0,
                                  (FLOAT)Pkt.BlendAlphaX1000 / 1000.f);
        break;

    case SMT_PickupObject:
        Dummy->SetDummyPickupParams(
            (FLOAT)Pkt.PickupDist2DX10 / 10.f,
            (FLOAT)Pkt.PickupDeltaZX10 / 10.f,
            Pkt.bPickupCrouched      != 0,
            Pkt.bPickupIsCollectible != 0);
        break;

    case SMT_CSA:
    {
        // Resolve CSA actor by path. Only set ActiveCSA when SyncMatinees is on —
        // FinishSpecialMove checks ActiveCSA != NULL before firing ObserverActivateCSA/ConsumeItem,
        // so leaving it NULL is the correct way to skip the Kismet event when sync is off.
        if (GMpConn.SyncMatinees && Pkt.CSAPathLen > 0)
        {
            TCHAR PathBuf[128] = {0};
            for (INT i = 0; i < Pkt.CSAPathLen && i < 127; i++)
                PathBuf[i] = (TCHAR)Pkt.CSAPath[i];
            Dummy->ActiveCSA = Cast<AOLCSA>(UObject::StaticFindObject(
                AOLCSA::StaticClass(), NULL, PathBuf, FALSE));
        }

        // Let StartSpecialMove drive the full lifecycle: plays anim (or not for instant CSA),
        // then FinishSpecialMove fires naturally via NativeOnAnimEnd or immediately if no anim.
        // GrabPos/GrabDir carry expectedAnimStart/expectedAnimFwd encoded by MpSendCSAActivation.
        Dummy->StartSpecialMove(SMT_CSA, GrabPos, GrabDir, APTT_TargetAtStart);

        // Mirror TryCSA: for instant CSA (no anim) clear bPlayingSpecialMoveAnim so
        // IsSpecialMoveCompleted() returns TRUE immediately and FinishSpecialMove fires next tick.
        if (Pkt.CSAAnimLen == 0)
            Dummy->bPlayingSpecialMoveAnim = FALSE;

        return;
    }

    default:
        break;
    }

    // Apply blend alpha globally — affects anim selection (high/low variants) for many SMTs.
    // HeroKilled/Decapitate already handle it via SetDummyKillParams above.
    if (SMT != SMT_HeroKilled && SMT != SMT_HeroDecapitate)
        Dummy->SpecialMoveBlendAlpha = (FLOAT)Pkt.BlendAlphaX1000 / 1000.f;

    // SMTs where the local player walks up to the target before triggering (bUsePawnVelocityForPositionning):
    // defer StartSpecialMove until LOC interpolation brings the dummy close enough to GrabPos.
    if (SMP.AdjustPosition && SMP.bPlayAnimWhenPositioned && SMP.bUsePawnVelocityForPositionning
        && !GrabPos.IsZero())
    {
        P->PendingSMT        = SMT;
        P->PendingSMTGrabPos = GrabPos;
        P->PendingSMTGrabDir = GrabDir;
        P->DummySMTLockUntil = 0.f;
        return;
    }

    // Use AdjustOrientation/AdjustPosition flags from SpecialMoveParams — same data the
    // sender's StartSpecialMove uses — so we never rotate or position-adjust on SMTs that
    // don't expect it (e.g. ExitLadderOnTop has neither flag; root motion handles both).
    if (SMP.AdjustOrientation && !GrabDir.IsZero())
        Dummy->SetRotation(GrabDir.Rotation());

    const FVector StartPos = SMP.AdjustPosition ? GrabPos : FVector(0,0,0);
    if (SMT == SMT_EnterStruggle)
        debugf(TEXT("[DummyStruggle] Before StartSM: Entry='%s' Cycle='%s'"),
            *Dummy->DummyStruggleEntryAnimPlayer.ToString(),
            *Dummy->DummyStruggleCycleAnimPlayer.ToString());
    Dummy->StartSpecialMove((ESpecialMoveType)SMT, StartPos, GrabDir);
    P->DummySMTLockUntil = 0.f;
}

// ============================================================================
// Send: global state packet every tick
// ============================================================================

void UHeroChannel::SendGlobalState()
{
    if (!CanSend())
        return;

    BYTE B[255];
    INT  N = 0;
    BuildStatePacket(GMultiplayerHero, bSendingJumpGroundZ, JumpGroundZ, B, N);
    GMpConn.SendBinary(B, N);
}

// ============================================================================
// SpawnDummy — spawn a remote player's dummy pawn
// ============================================================================

AMultiplayerHero* SpawnDummy(AMultiplayerController* Controller)
{
    if (!Controller->Pawn)
        return NULL;

    FVector  SpawnLoc = Controller->Pawn->Location;
    FRotator SpawnRot(0, 0, 0);

    AMultiplayerHero* Dummy = Cast<AMultiplayerHero>(
        GWorld->SpawnActor(AMultiplayerHero::StaticClass(), NAME_None, SpawnLoc, SpawnRot,
                           NULL, TRUE, FALSE, Controller, NULL));
    if (!Dummy)
        return NULL;

    Dummy->SetCollision(FALSE, FALSE, FALSE);
    Dummy->bCollideWorld = FALSE;

    if (Dummy->CylinderComponent)
    {
        Dummy->CylinderComponent->BlockActors        = FALSE;
        Dummy->CylinderComponent->BlockNonZeroExtent = FALSE;
    }

    Dummy->InitDummyMesh();
    Dummy->bIsDummyPawn = TRUE;
    Dummy->bCanCrouch   = TRUE;
    if (Dummy->Mesh)
        Dummy->Mesh->SetHiddenGame(TRUE);
    if (Dummy->HeadMesh)
        Dummy->HeadMesh->SetHiddenGame(TRUE);
    Dummy->Health = 0; // treat as dead until first LOC packet arrives
    Dummy->setPhysics(PHYS_None);

    return Dummy;
}

// ============================================================================
// Receive: binary LOC packet
// ============================================================================

void UHeroChannel::OnBinaryLoc(INT SenderID, BYTE* Data, INT DataLen)
{
    if (!CanReceive())
        return;

    AMultiplayerController* Controller = Cast<AMultiplayerController>(ControllerOwner);
    if (!Controller)
        return;

    FHeroStatePacket S;
    if (!DecodeBinaryState(Data, DataLen, S))
        return;

    INT Idx = Controller->FindRemoteIndex(SenderID);
    if (Idx == -1)
    {
        FString Nick = S.bHasNick ? FString(S.Nick) : FString::Printf(TEXT("Player%d"), SenderID);
        UBOOL bAlreadyKnown = FALSE;
        for (INT i = 0; i < GMpConn.KnownPlayers.Num(); i++)
            if (GMpConn.KnownPlayers(i).PlayerID == SenderID)
                { bAlreadyKnown = TRUE; break; }
        if (!bAlreadyKnown)
        {
            FMpConnection::FRemoteNick Entry;
            Entry.PlayerID = SenderID;
            Entry.Username = Nick;
            GMpConn.KnownPlayers.AddItem(Entry);
        }
        Idx = Controller->RegisterRemotePlayer(SenderID, Nick);
        if (!bAlreadyKnown)
            HUD_AddNotification(Cast<AMultiplayerHUD>(Controller->myHUD), Nick + TEXT(" connected"));
    }
    if (Idx == -1)
        return;

    URemotePlayer* P = Controller->RemotePlayers(Idx);
    if (!P->DummyPlayer && Controller->Pawn)
        P->DummyPlayer = SpawnDummy(Controller);

    ApplyHeroState(Controller, Idx, S);
}

// ============================================================================
// FHeroChannelTicker
// ============================================================================

UBOOL FHeroChannelTicker::IsTickable() const
{
    return Channel != NULL && CanSend();
}

void FHeroChannelTicker::Tick(FLOAT DeltaTime)
{
    Channel->SendGlobalState();
    Channel->SendHeadRotation();
    Channel->SendMesh();
    Channel->SendCinematicAnimation();
    Channel->SendSpecialMoveType();
}

// ============================================================================
// FHeroChannelReceiveTicker — interpolates remote dummy positions every tick.
// Mirrors the VInterpTo/RInterpTo loop from the old UC PlayerTick.
// ============================================================================

UBOOL FHeroChannelReceiveTicker::IsTickable() const
{
    return Channel != NULL && CanReceive();
}

void FHeroChannelReceiveTicker::Tick(FLOAT DeltaTime)
{
    if (!Channel) return;

    AMultiplayerController* Controller = Cast<AMultiplayerController>(Channel->ControllerOwner);
    if (!Controller) return;

    const FLOAT InterpSpeed = 12.0f;

    for (INT i = 0; i < Controller->RemotePlayers.Num(); i++)
    {
        URemotePlayer* P = Controller->RemotePlayers(i);
        if (!P || !P->DummyPlayer || !P->bHasReceivedData) continue;

        AOLHero* Dummy = Cast<AOLHero>(P->DummyPlayer);
        if (!Dummy) continue;

        // Don't fight root motion: skip VInterpTo entirely when dummy mesh is in RMM_Accel.
        // AdjustPosition velocity correction runs via UpdateSpecialMove (see TickPrePhysics).
        if (Dummy->Mesh && Dummy->Mesh->RootMotionMode == RMM_Accel)
            continue;


        // SMT_None but lock still active (e.g. ledge/climb settled): hold position.
        if (P->DummySMTLockUntil >= GWorld->GetTimeSeconds())
            continue;

        // LM_Pushing: dummy position is driven by pushable attachment — skip interpolation.
        const INT LM = (INT)Dummy->LocomotionMode;
        if (LM == LM_Pushing)
            continue;

        // VInterpTo/RInterpTo modify DeltaTime in-place; use a local copy per player.
        FLOAT DT = DeltaTime;

        // JumpOver: Z is locked to JumpGroundZ until SMT ends (handled in ApplySpecialMoveTransition).
        if (P->bJumpOverActive)
        {
            FVector TargetXY = P->LastReceivedLoc;
            TargetXY.Z = Dummy->Location.Z;
            FVector SmoothedLoc = VInterpTo(Dummy->Location, TargetXY, DT, InterpSpeed);
            Dummy->SetLocation(SmoothedLoc);
        }
        else
        {
            FVector SmoothedLoc = VInterpTo(Dummy->Location, P->LastReceivedLoc, DT, InterpSpeed);
            Dummy->SetLocation(SmoothedLoc);
        }

        // Skip rotation interpolation while a deferred SMT is waiting to fire — the rotation
        // is pre-set to GrabDir just before StartSpecialMove and must not be overwritten by LOC.
        // Also skip when an SMT is active but RMM_Accel hasn't kicked in yet (first tick after
        // StartSpecialMove) to avoid RInterpTo wrapping through 360° on large yaw deltas.
        const UBOOL bSMTActive = (Dummy->SpecialMove != SMT_None);
        if (P->PendingSMT == SMT_None && !bSMTActive)
        {
            DT = DeltaTime;
            FRotator SmoothedRot = RInterpTo(Dummy->Rotation, P->LastReceivedRot, DT, InterpSpeed);
            Dummy->SetRotation(SmoothedRot);
        }
        else if (P->PendingSMT != SMT_None && !P->PendingSMTGrabDir.IsZero())
        {
            // Hold at the expected grab direction while waiting for proximity trigger.
            Dummy->SetRotation(P->PendingSMTGrabDir.Rotation());
        }

        // Enforce zero collision every tick.
        Dummy->SetCollision(FALSE, FALSE, FALSE);
        Dummy->bCollideWorld = FALSE;
        if (Dummy->CylinderComponent)
            Dummy->CylinderComponent->BlockActors = FALSE;

        // Speedrun mode: hide body and head when dummy is within 200 units of the camera.
        if (GMpConn.SpeedrunMode && Dummy->Mesh)
        {
            AOLPlayerController* LocalPC = Utils::GetOLPC();
            FVector CamLoc(0.f, 0.f, 0.f);
            FRotator CamRot(0, 0, 0);
            if (LocalPC)
                LocalPC->eventGetPlayerViewPoint(CamLoc, CamRot);
            FLOAT DistSq = (Dummy->Location - CamLoc).SizeSquared();
            UBOOL bTooClose = DistSq < (200.f * 200.f);
            Dummy->Mesh->SetHiddenGame(bTooClose);
            if (Dummy->DummyHeadMesh)
                Dummy->DummyHeadMesh->SetHiddenGame(bTooClose);
        }

        // Fire deferred SMT once LOC interpolation has brought the dummy close enough to GrabPos.
        // Mirrors how the local player walks up to the target before StartSpecialMove is called.
        if (P->PendingSMT != SMT_None)
        {
            FVector toGrab = P->PendingSMTGrabPos - Dummy->Location;
            toGrab.Z = 0.f;
            if (toGrab.SizeSquared() < 40.f * 40.f)
            {
                const INT PendingSMT = P->PendingSMT;
                const FSpecialMoveParameters& PSMP = Dummy->SpecialMoveParams[PendingSMT];
                P->PendingSMT = SMT_None;
                if (PSMP.AdjustOrientation && !P->PendingSMTGrabDir.IsZero())
                    Dummy->SetRotation(P->PendingSMTGrabDir.Rotation());
                Dummy->StartSpecialMove((ESpecialMoveType)PendingSMT,
                                        P->PendingSMTGrabPos, P->PendingSMTGrabDir);
            }
        }
    }

    // ---- Remote enemy interpolation ----
    if (!GMpConn.SyncEnemies)
        return;
    const FLOAT EnemyInterpSpeed = 8.0f;
    for (INT i = 0; i < Controller->RemoteEnemies.Num(); i++)
    {
        FRemoteEnemyState& RS = Controller->RemoteEnemies(i);
        if (!RS.DummyEnemy) continue;
        FLOAT DT = DeltaTime;
        FVector SmoothedLoc = VInterpTo(RS.DummyEnemy->Location, RS.TargetLoc, DT, EnemyInterpSpeed);
        RS.DummyEnemy->SetLocation(SmoothedLoc);
        DT = DeltaTime;
        if (!RS.bAnimating)
        {
            FRotator SmoothedRot = RInterpTo(RS.DummyEnemy->Rotation, RS.TargetRot, DT, EnemyInterpSpeed);
            RS.DummyEnemy->SetRotation(SmoothedRot);
        }
        DT = DeltaTime;
        RS.DummyEnemy->Velocity = VInterpTo(RS.DummyEnemy->Velocity, RS.TargetVel, DT, EnemyInterpSpeed);
        RS.DummyEnemy->Acceleration = RS.DummyEnemy->Velocity;
    }
}

// ============================================================================
