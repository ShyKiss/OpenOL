 #include "OLGame.h"
#include "EngineAnimClasses.h"
#include "UDKBaseAnimationClasses.h"
#include "OLGameAnimClasses.h"
#include "OLUtilities.h"
#include "OLDingo.h"

IMPLEMENT_CLASS(AOLHero);

const FLOAT Fudge = 5.0f;

////////////////////////////////////////////////////////////////////////////////////////////
// Special Moves
////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////


UBOOL AOLHero::IsSpecialMoveCompleted()
{
	// temp special case
	if ((SpecialMove == SMT_Dying || SpecialMove == SMT_HeroDecapitate || SpecialMove == SMT_HeroKilled || SpecialMove == SMT_KilledInStruggle) && Health == 0)
	{
		return (GWorld->GetTimeSeconds() > TimeOfDeath + DeathScreenDuration);
	}
	
	if (ProceduralAnims.Num() > 0)
	{
		return FALSE;
	} 

	if (AdjustPosition.Active && !AdjustPosition.Done)
	{
		return FALSE;
	}

	if (!bPlayingSpecialMoveAnim)
	{
		return TRUE;
	}

	// Check whether we're stuck - failsafe against missed notifies (should fix those but i'm not sure how it happens)
	if (RealVelocity.IsNearlyZero(KINDA_SMALL_NUMBERF) && appIsNearlyEqual(LocomotionAnimNode->NodeTotalWeight, 1.0f))
	{
		if (SpecialMoveStalledTimestamp <= 0.0f)
		{
			// set the timer
			SpecialMoveStalledTimestamp = GWorld->GetTimeSeconds();		
		}
		else if (GWorld->GetTimeSeconds() > SpecialMoveStalledTimestamp + 1.0f)
		{
			// timer's done
			warnf(TEXT("### ABORTING SPECIAL MOVE!"));
			return TRUE;
		}
	}
	else
	{
		// invalidate timer
		SpecialMoveStalledTimestamp = -1.0f; 
	}

	return FALSE;
}

void AOLHero::SpecialMoveCompleted() 
{
	ESpecialMoveType completedMove = (ESpecialMoveType)SpecialMove;
	Super::SpecialMoveCompleted();
	FinishSpecialMove(completedMove, FALSE);
}

void AOLHero::CancelSpecialMove()
{	
	ESpecialMoveType moveType = (ESpecialMoveType)SpecialMove;
	Super::SpecialMoveCompleted(); // Note: NOT calling the super version of CancelSpecialMove, but instead calling special move completed
	FinishSpecialMove(moveType, TRUE);
}

void AOLHero::FinishSpecialMove(ESpecialMoveType completedMove, UBOOL bCancelling)
{
	switch (completedMove)
	{
	case SMT_JumpOnSpot:
		{
			UBOOL bLandedInWater = (GetMaterialBelowFeet() == WaterMaterial) || IsInWaterVolume();
			MakeNoise(bLandedInWater ? LandingSmallWaterLoudness : LandingSmallLoudness);
		}
		break;
	case SMT_JumpOver:
	case SMT_ClimbOverWall:
	case SMT_ClimbUpObstacle:
	case SMT_ClimbUpWall:
	case SMT_SlideOver:
		{
			if (!bCancelling && bWantToRun && bRunningTraversalMove) // keep momentum after these moves
			{
				GroundSpeed = RunSpeed;
				CurrentRunSpeed = GroundSpeed;
				Velocity = GroundSpeed * CharForward;
			}
		}
		break;
	case SMT_ExitSqueeze:
	case SMT_HeroGrabbedFromSqueeze:
	case SMT_AutomaticSqueeze:
		{
			ActiveSqueeze = NULL;
		}
		break;
	case SMT_ExitLedgeWalk:
	case SMT_DropFromLedge:
	case SMT_ClimbUpLedge:
		{
			ActiveLedge = NULL;
		}
		break;
	case SMT_EnterDoorInteraction:
		{
			if (bCancelling)
			{
				ActiveDoor->DoorUser = NULL;
				ActiveDoor = NULL;
			}
			else
			{
				check(ActiveDoor);
				ActiveDoor->StartedInteractiveOpening(this);
			}
		}
		break;
	case SMT_OpenDoorInstant:
	case SMT_TryOpenLockedDoor:
	case SMT_OpenDoorPartial:
	case SMT_RunThroughDoor:
	case SMT_CloseDoor:
	case SMT_CloseDoorPositionned:
		{
			check(ActiveDoor);
			ActiveDoor->SetCollisionType(COLLIDE_BlockAll);
			ActiveDoor->DoorUser = NULL;
			ActiveDoor = NULL;
		}
		break;
	case SMT_DoorClosedFromOtherSide:
	case SMT_EnterLocker:
	case SMT_OpenLockerFromOutside:
		{
			ActiveDoor->DoorUser = NULL;
			ActiveDoor = NULL;
		}
		break;
	case SMT_PickupObject:
		{
			if (ActiveAttachment.AttachedComp)
			{
				ActiveAttachment.AttachedComp->DetachFromAny();
				ActiveAttachment.AttachedComp->SetHiddenGame(TRUE);
			}
			appMemZero(ActiveAttachment);

			if (!bCancelling && ActivePickup)
			{
				if (!ActivePickup->bUsed)
				{
					ActivePickup->Pickup(this);
				}

				ActivePickup->PickupMesh->SetHiddenGame(TRUE);
				ActivePickup = NULL;
			}
		}
		break;
	case SMT_CSA:
		{
			if (!bCancelling && ActiveCSA)
			{
				ActiveCSA->Completed(this);
			}
			ActiveCSA = NULL;
		}
		break;
	case SMT_ExitLadderOnGround:
	case SMT_ExitLadderOnTop:
		{
			ActiveLadder = NULL;
		}
		break;
	case SMT_ExitLocker:
		{
			if (ActiveLocker && ActiveLocker->AssociatedDoor)
			{
				ActiveLocker->AssociatedDoor->Close(this);
			}
			ActiveLocker = NULL;
			ActiveDoor->DoorUser = NULL;
			ActiveDoor = NULL;
		}
		break;
	case SMT_ExitBed:
		{
			ActiveBed = NULL;
		}
		break;
	case SMT_HeroGrabbedFromLocker:
		{
			ActiveLocker = NULL;
		}
		break;
	case SMT_HeroGrabbedFromBed:
		{
			ActiveBed = NULL;
		}
		break;	
	case SMT_EnterStruggle:
		{
			OLPC->StruggleEntryCompleted();
		}
		break;
	case SMT_ExitStruggle:
		{
			OLPC->StruggleExitCompleted();
		}
		break;
	case SMT_StopPushingObject:
		{
			ActivePushable = NULL;
		}
		break;
	case SMT_KilledInStruggle:
		{
			if (!bCancelling && !bStartingSpecialMove) // starting SMT_Dying, when no killed anim
			{
				// We're done dying, respawn
				eventRespawnHero();			
			}
		}
		break;
	case SMT_Dying:
	case SMT_HeroDecapitate:
	case SMT_HeroKilled:	
		{
			if (!bCancelling)
			{
				// We're done dying, respawn
				eventRespawnHero();			
			}
		}
		break;	
	}

	bShouldHideLeftHandDuringSM = FALSE;
	bShouldHideRightHandDuringSM = FALSE;
	bBothHandsNeeded = FALSE;
	bProceduralAnimsDelayedAfterSpecialMove = FALSE;
	LastSpecialMoveFinishedTime = GWorld->GetTimeSeconds();
	
	if (completedMove == SMT_ClimbUpObstacle)
	{
		LastClimbUpObstacleFinishedTime = GWorld->GetTimeSeconds();
	}
	else if (completedMove == SMT_EnterLocker || completedMove == SMT_EnterBed)
	{
		OLPC->bUsedHidingSpot = TRUE;
	}

	if (completedMove == SMT_HeroGrabbedFromUnder || completedMove == SMT_ExitLocker)
	{
		bWantsToCrouch = FALSE;
		OLPC->bDuck = FALSE;
	}

	if (!bStartingSpecialMove)
	{
		if (bMustCrouchAfterSpecialMove)
		{
			bWantsToCrouch = TRUE;
			bForcedCrouch = TRUE;
		}
		bMustCrouchAfterSpecialMove = FALSE;
		
		if (bCancelling)
		{
			EnterLocomotionMode(LM_Walk); // Dangerous?
		}
		else if (completedMove == SMT_ExitStruggle && bEnterLedgeWalkAfterStruggle && ActiveLedge)
		{
			EnterLocomotionMode(LM_LedgeWalk); // Works for now, but may need more checks eventually
			bEnterLedgeWalkAfterStruggle = FALSE;
		}
		else if (GetNextLocomotionMode(completedMove) >= 0 && !SpecialMoveParams[completedMove].KeepLocomotionMode)
		{
			EnterLocomotionMode(GetNextLocomotionMode(completedMove));
		}
		else
		{
			// We kept the locomotion mode, but we may have changed some parameters
			// Make sure to reset to the normal setup for this loco mode
			bBothHandsNeeded = LocomotionModeParams[LocomotionMode].GP.BothHandsNeeded;

			if (bBothHandsNeeded)
			{
				if (CamcorderState != CCS_Inactive)
				{
					LowerCamcorder();
				}
				else
				{
					SetBodySetup(HBS_Normal);
				}
			}

			SetCamParams(LocomotionModeParams[LocomotionMode].GP);

			if (Physics != LocomotionModeParams[LocomotionMode].GP.Physics)
			{
				setPhysics(LocomotionModeParams[LocomotionMode].GP.Physics);
			}

			SetRootMotionMode((ERootMotionMode)LocomotionModeParams[LocomotionMode].GP.RMM);

			bFailedCollisionSet = !TryAdjustCollisionSizeForLocomotionMode((ELocomotionMode)LocomotionMode);
		}
	}
}

void AOLHero::SpecialMovePhaseCompleted()
{
	if (SpecialMove == SMT_HeroGrabbedFromSqueeze)
	{
		PlayGrabFromSqueezePhaseTwo();
	}
}

void AOLHero::ApplySpecialMoveParams(ESpecialMoveType moveType)
{
	ShadowProxyFullBodyAnimSlot->StopCustomAnim(0.25f);

	Super::ApplySpecialMoveParams(moveType);

	const FSpecialMoveParameters& params = SpecialMoveParams[moveType];		

	if (params.bTargettedYawSmoothing)
	{
		FLOAT animLength = -1.0f;
	
		if (CustomBlendNode && CustomBlendNode->bActive)
		{
			animLength = CustomBlendNode->PlaybackTime;
		}
		else if (FullBodyAnimSlot)
		{
			UAnimNodeSequence* animNodeSeq = Cast<UAnimNodeSequence>(FullBodyAnimSlot->GetCustomAnimNodeSeq());

			if (animNodeSeq)
			{
				animLength = animNodeSeq->GetAnimPlaybackLength();
			}
		}

		check(animLength > 0.0f);
		Camera->ActivateTargettedYawSmoothing(SpecialMoveTargetYaw, animLength);
	}
	
	if (!params.KeepLocomotionMode)
	{
		EnterLocomotionMode(LM_SpecialMove);
	}

	bBothHandsNeeded = params.GP.BothHandsNeeded;
	if (bBothHandsNeeded)
	{
		if (CamcorderState != CCS_Inactive)
		{
			LowerCamcorder();
		}
		else
		{
			SetBodySetup(HBS_Normal);
		}
	}

	SetCamParams(params.GP);
}

void AOLHero::StartSpecialMove(BYTE moveType, FVector targetPosition, FVector targetDirection, BYTE targetType)
{
	bStartingSpecialMove = TRUE;

	if (IsReloading() && moveType != SMT_SqueezeReload && moveType != SMT_BedReload)
	{
		CancelReload();
	}

	Super::StartSpecialMove(moveType, targetPosition, targetDirection, targetType);

	if (moveType == SMT_OpenDoorInstant || moveType == SMT_OpenDoorPartial || moveType == SMT_TryOpenLockedDoor)
	{
		TWEAKABLE FLOAT LockedDoorMaxApproachTime = 0.3f;
		TWEAKABLE FLOAT OpenDoorMaxApproachTime = 0.5f;

		FLOAT maxApproachTime = (moveType == SMT_TryOpenLockedDoor) ? LockedDoorMaxApproachTime : OpenDoorMaxApproachTime;

		// Make sure that the animation doesn't finish before the correct gets us at the door (looks stupid)
		if (AdjustPosition.CorrectionTime > maxApproachTime)
		{
			FLOAT slowDownRatio = maxApproachTime / AdjustPosition.CorrectionTime;
			UAnimNodeSequence* animNodeSeq = Cast<UAnimNodeSequence>(FullBodyAnimSlot->GetCustomAnimNodeSeq());
			if (animNodeSeq)
			{
				animNodeSeq->Rate = slowDownRatio;
			}

			UAnimNodeSequence* shadowAnimNodeSeq = Cast<UAnimNodeSequence>(ShadowProxyFullBodyAnimSlot->GetCustomAnimNodeSeq());
			if (shadowAnimNodeSeq)
			{
				shadowAnimNodeSeq->Rate = slowDownRatio;
			}
		}
	}

	MeshZOffset = 0.0f;
	SpecialMoveStalledTimestamp = -1.0f;
	bStartingSpecialMove = FALSE;
}

void AOLHero::UpdateSpecialMove(FLOAT deltaTime)
{
	if (SpecialMove == SMT_TryOpenLockedDoor)
	{
		TWEAKABLE FLOAT LockedDoorMaxApproachTime = 0.3f;

		// Reset playrate if we're done with the approach
		
		UAnimNodeSequence* animNodeSeq = Cast<UAnimNodeSequence>(FullBodyAnimSlot->GetCustomAnimNodeSeq());
		if (animNodeSeq && animNodeSeq->CurrentTime > LockedDoorMaxApproachTime)
		{
			animNodeSeq->Rate = 1.0f;

			UAnimNodeSequence* shadowAnimNodeSeq = Cast<UAnimNodeSequence>(ShadowProxyFullBodyAnimSlot->GetCustomAnimNodeSeq());
			if (shadowAnimNodeSeq)
			{
				shadowAnimNodeSeq->Rate = 1.0f;
			}
		}		
	}

	Super::UpdateSpecialMove(deltaTime);
}

ELocomotionMode AOLHero::GetNextLocomotionMode(ESpecialMoveType moveType)
{
	ELocomotionMode nextLocomotionMode = (ELocomotionMode)SpecialMoveParams[moveType].NextLocomotionMode;

	switch(moveType)
	{
	case SMT_HeroGrabbedFromBed:
	case SMT_HeroGrabbedFromLocker:
	case SMT_HeroGrabbedFromUnder:
		{
			switch(EnemyType)
			{
			case ET_Soldier:
			case ET_Swarm:
			case ET_Other:
			case ET_Groom:
				nextLocomotionMode = LM_Grabbed;
				break;
			default:
				nextLocomotionMode = LM_Walk;
				break;
			}
		}
		break;
	}

	return nextLocomotionMode;
}

void AOLHero::PlayAnimForSpecialMove(ESpecialMoveType moveType)
{
	switch (moveType)
	{
	case SMT_Crouch:
	case SMT_Uncrouch:
		{
			UAnimNodeSequence* animSeq = FullBodyAnimSlot->GetCustomAnimNodeSeq();
			FName animName = (moveType == SMT_Crouch) ? AnimNameCrouch : AnimNameUncrouch;

			FLOAT startPct = 0.0f;
			UBOOL performingOppositeMove = FALSE;

			if (animSeq)
			{
				if ((animName == AnimNameCrouch && animSeq->AnimSeqName == AnimNameUncrouch) || 
					(animName == AnimNameUncrouch && animSeq->AnimSeqName == AnimNameCrouch))
				{
					FLOAT currentPct = animSeq->CurrentTime / animSeq->GetAnimPlaybackLength();
					startPct = (1.0f - currentPct);
					performingOppositeMove = TRUE;
				}

				FullBodyAnimSlot->StopCustomAnim(0.1f);
				ShadowProxyFullBodyAnimSlot->StopCustomAnim(0.1f);				
			}
			
			if (!performingOppositeMove)
			{			
				UOLAnimBlendByPosture* blendByPostureNode = NULL;
				if (BlendByPostureWalkingAnimNode && BlendByPostureWalkingAnimNode->bRelevant)
				{
					blendByPostureNode = BlendByPostureWalkingAnimNode;
				}

				if (BlendByPostureFallingAnimNode && BlendByPostureFallingAnimNode->bRelevant && (!blendByPostureNode || (BlendByPostureFallingAnimNode->NodeTotalWeight > blendByPostureNode->NodeTotalWeight)))
				{
					blendByPostureNode = BlendByPostureFallingAnimNode;
				}

				if (blendByPostureNode)
				{
					// If already blending between crouch and walk posture, 
					INT targetIdx = (moveType == SMT_Crouch) ? 1 : 0;
					FLOAT deltaWeightToGo = 1.0f - blendByPostureNode->Children(targetIdx).Weight;
					startPct = (1.0f - deltaWeightToGo);
				}
			}

			UAnimSequence* newAnimSeq = Mesh->FindAnimSequence(animName);
			check(newAnimSeq);
			FLOAT startTime = startPct * newAnimSeq->SequenceLength;

			TWEAKABLE FLOAT minPlayTime = 0.35f; // we need a minimum time to allow the normal movegroup to blend out
			startTime = Min(startTime, Max(0.0f, newAnimSeq->SequenceLength - minPlayTime));

			PlayFullBodyAnim(animName, 1.0f, 0.15f, 0.15f, startTime);
		}
		break;
	case SMT_BigLanding:
		{
			UBOOL bConsiderCrouched = (BlendByPostureFallingAnimNode && BlendByPostureFallingAnimNode->ActiveChildIndex == 1);
			PlayFullBodyAnim(bConsiderCrouched ? AnimNameBigLandingCrouched : AnimNameBigLandingStanding, 1.f, 0.1f, 0.25f);
		}
		break;
	case SMT_JumpOver:
		{
			if (bRunningTraversalMove)
			{
				PlayFullBodyAnim(AnimNameJumpOverFromRun, 1.f, 0.1f, 0.25f);
			}
			else
			{
				PlayFullBodyAnim(AnimNameJumpOverFromWalk, 1.f, 0.1f, 0.25f);
			}
		}
		break;
	case SMT_ClimbOverWall:
		{
			PlayFullBodyAnim(AnimNameClimbOverWall200, 1.f, 0.1f, 0.25f);
		}
		break;
	case SMT_ClimbUpObstacle:
		{
			if (bMustCrouchAfterSpecialMove)
			{
				PlayFullBodyAnim(AnimNameClimbUpToCrouch, 1.f, 0.1f, 0.25f);
			}
			else if (bRunningTraversalMove)
			{
				PlayFullBodyAnim(AnimNameClimbUpFromRun, 1.f, 0.1f, 0.25f);
			}
			else
			{
				PlayFullBodyAnim(AnimNameClimbUpFromWalk, 1.f, 0.1f, 0.25f);
			}
		}
		break;
	case SMT_ClimbUpWall:
		{
			PlayBlendedAnim(AnimNameClimbUpWall200, AnimNameClimbUpWall150, SpecialMoveBlendAlpha, 0.25f, 0.25f);
		}
		break;
	case SMT_SlideOver:
		{
			TWEAKABLE FLOAT BlendInTime = 0.3f;
			TWEAKABLE FLOAT BlendOutTime = 0.35f;
			PlayFullBodyAnim(AnimNameSlideOverFromRun, 1.f, BlendInTime, BlendOutTime);
		}
		break;
	case SMT_LedgeHangTransition:
		{
			switch (ActiveLedgeTransitionType)
			{
			case LTT_LeftInside:
				{
					PlayFullBodyAnim(AnimNameLedgeHangLeftInside, 1.f, 0.1f, 0.1f);					
				}
				break;
			case LTT_LeftOutside:
				{
					PlayFullBodyAnim(AnimNameLedgeHangLeftOutside, 1.f, 0.1f, 0.1f);					
				}
				break;
			case LTT_RightInside:
				{
					PlayFullBodyAnim(AnimNameLedgeHangRightInside, 1.f, 0.1f, 0.1f);					
				}
				break;
			case LTT_RightOutside:
				{
					PlayFullBodyAnim(AnimNameLedgeHangRightOutside, 1.f, 0.1f, 0.1f);					
				}
				break;
			}
		}
		break;
	case SMT_ClimbUpLedge:
		{
			FLOAT blendInTime = 0.1f;
			if (FullBodyAnimSlot->GetCustomAnimNodeSeq())
			{
				blendInTime = 0.0f;
				FullBodyAnimSlot->StopCustomAnim(0.0f); // if there's a grab ledge in progress that we're interrupting, kill it now so it doesn't interfere with root motion
			}
			switch (LedgeClimbType)
			{
			case LCT_ClimbUpToStand:
				{
					PlayFullBodyAnim(AnimNameClimbUpLedgeToStand, 1.f, blendInTime, 0.5f);
				}
				break;
			case LCT_ClimbUpToCrouch:
				{
					PlayFullBodyAnim(AnimNameClimbUpLedgeToCrouch, 1.f, blendInTime, 0.5f);
				}
				break;
			case LCT_ClimbOverToFalling:
				{
					PlayFullBodyAnim(AnimNameClimbOverLedgeToFalling, 1.f, blendInTime, 0.1f);
				}
				break;
			case LCT_ClimbOverToStand:
				{
					PlayFullBodyAnim(AnimNameClimbOverLedgeToStand, 1.f, blendInTime, 0.5f);
				}
				break;
			}
		}
		break;
	case SMT_GrabLedgeFromGround:
		{
			PlayBlendedAnim(AnimNameGrabLedgeFromWalkHigh, AnimNameGrabLedgeFromWalkLow, SpecialMoveBlendAlpha, 0.25f, 0.25f);
		}
		break;
	case SMT_GrabLedgeFromAir:
		{
			PlayFullBodyAnim(AnimNameGrabLedgeFromAir, 1.f, 0.1f, 0.25f);
		}
		break;
	case SMT_GrabAndClimb:
		{
			PlayFullBodyAnim(AnimNameGrabAndClimb, 1.f, 0.1f, 0.25f);
		}
		break;
	case SMT_JumpOverAndGrabLedge:
		{
			PlayFullBodyAnim(AnimNameJumpOverToGrabLedge, 1.f, 0.1f, 0.25f);
		}
		break;
	case SMT_EnterLedgeWalk:
		{
			switch (ActiveLedgeTransitionType)
			{
			case LTT_LeftInside:
				{
					PlayBlendedAnim(AnimNameEnterLedgeWalkInsideLeftPrl, AnimNameEnterLedgeWalkInsideLeftPerp, SpecialMoveBlendAlpha, 0.1f, 0.1f);
				}
				break;
			case LTT_RightInside:
				{
					PlayBlendedAnim(AnimNameEnterLedgeWalkInsideRightPrl, AnimNameEnterLedgeWalkInsideRightPerp, SpecialMoveBlendAlpha, 0.1f, 0.1f);
				}
				break;
			case LTT_LeftOutside:
				{
					PlayBlendedAnim(AnimNameEnterLedgeWalkOutsideLeftPrl, AnimNameEnterLedgeWalkOutsideLeftPerp, SpecialMoveBlendAlpha, 0.1f, 0.1f);
				}
				break;
			case LTT_RightOutside:
				{
					PlayBlendedAnim(AnimNameEnterLedgeWalkOutsideRightPrl, AnimNameEnterLedgeWalkOutsideRightPerp, SpecialMoveBlendAlpha, 0.1f, 0.1f);
				}
				break;
			default:
				check(FALSE);
			}
		}
		break;
	case SMT_ExitLedgeWalk:
		{
			switch (ActiveLedgeTransitionType)
			{
			case LTT_LeftInside:
				{
					PlayFullBodyAnim(AnimNameExitLedgeWalkLeftInside, 1.f, 0.25f, 0.5f);					
				}
				break;
			case LTT_LeftOutside:
				{
					PlayFullBodyAnim(AnimNameExitLedgeWalkLeftOutside, 1.f, 0.25f, 0.5f);					
				}
				break;
			case LTT_RightInside:
				{
					PlayFullBodyAnim(AnimNameExitLedgeWalkRightInside, 1.f, 0.25f, 0.5f);					
				}
				break;
			case LTT_RightOutside:
				{
					PlayFullBodyAnim(AnimNameExitLedgeWalkRightOutside, 1.f, 0.25f, 0.5f);					
				}
				break;
			}
		}
		break;
	case SMT_LedgeWalkTransition:
		{
			switch (ActiveLedgeTransitionType)
			{
			case LTT_LeftInside:
				{
					PlayFullBodyAnim(AnimNameLedgeWalkTransitionLeftInside, 1.f, 0.1f, 0.1f);					
				}
				break;
			case LTT_LeftOutside:
				{
					PlayFullBodyAnim(AnimNameLedgeWalkTransitionLeftOutside, 1.f, 0.1f, 0.1f);					
				}
				break;
			case LTT_RightInside:
				{
					PlayFullBodyAnim(AnimNameLedgeWalkTransitionRightInside, 1.f, 0.1f, 0.1f);					
				}
				break;
			case LTT_RightOutside:
				{
					PlayFullBodyAnim(AnimNameLedgeWalkTransitionRightOutside, 1.f, 0.1f, 0.1f);					
				}
				break;
			}
		}
		break;
	case SMT_JumpFromLedgeWalk:
		{
			if (bJumpFromLedgeWalkWithVelocity)
			{
				PlayFullBodyAnim(AnimNameJumpFromLedgeWalk, 1.f, 0.1f, 0.15f);
			}
			else
			{
				PlayFullBodyAnim(AnimNameStepOffFromLedgeWalk, 1.f, 0.1f, 0.15f);
			}
		}
		break;
	case SMT_EnterSqueeze:
		{
			if (bLeftAnim)
			{
				PlayFullBodyAnim(AnimNameEnterSqueezeLeft, 1.f, 0.25f, 0.25f);					
			}
			else
			{
				PlayFullBodyAnim(AnimNameEnterSqueezeRight, 1.f, 0.25f, 0.25f);					
			}
		}
		break;
	case SMT_ExitSqueeze:
		{
			if (bLeftAnim)
			{
				PlayFullBodyAnim(AnimNameExitSqueezeLeft, 1.f, 0.25f, 0.1f);					
			}
			else
			{
				PlayFullBodyAnim(AnimNameExitSqueezeRight, 1.f, 0.25f, 0.1f);					
			}
		}
		break;
	case SMT_AutomaticSqueeze:
		{
			PlayFullBodyAnim(AnimNameAutomaticSqueeze, 1.f, 0.25f, 0.25f);
		}
		break;
	case SMT_HeroGrabbedFromSqueeze:
		{
			switch (SqueezeTransitionType)
			{
			case STT_Left_Back:
			case STT_Left_Facing:
				{
					if (SpecialMoveBlendAlpha < 0.5f)
					{
						PlayBlendedAnim(AnimNameGrabFromSqueezePhase1Left0, AnimNameGrabFromSqueezePhase1Left90, 2.0f*(0.5f - SpecialMoveBlendAlpha), 0.1f, 0.5f);
					}
					else
					{
						PlayBlendedAnim(AnimNameGrabFromSqueezePhase1Left180, AnimNameGrabFromSqueezePhase1Left90, 2.0f*(SpecialMoveBlendAlpha - 0.5f), 0.1f, 0.5f);
					}
				}
				break;
			case STT_Right_Back:
			case STT_Right_Facing:
				{
					if (SpecialMoveBlendAlpha < 0.5f)
					{
						PlayBlendedAnim(AnimNameGrabFromSqueezePhase1Right0, AnimNameGrabFromSqueezePhase1Right90, 2.0f*(0.5f - SpecialMoveBlendAlpha), 0.1f, 0.5f);
					}
					else
					{
						PlayBlendedAnim(AnimNameGrabFromSqueezePhase1Right180, AnimNameGrabFromSqueezePhase1Right90, 2.0f*(SpecialMoveBlendAlpha - 0.5f), 0.1f, 0.5f);
					}
				}
				break;
			default:
				check(FALSE);
			}
			CustomBlendNode->bKeepLastPose = TRUE;
			ShadowProxyCustomBlendNode->bKeepLastPose = TRUE;
		}
		break;
	case SMT_EnterLadderFromGround:
		{
			if (bLeftAnim)
			{
				PlayBlendedAnim(AnimNameEnterLadderGroundStraight, AnimNameEnterLadderGround45Left, SpecialMoveBlendAlpha, 0.25f, 0.05f);
			}
			else
			{
				PlayBlendedAnim(AnimNameEnterLadderGroundStraight, AnimNameEnterLadderGround45Right, SpecialMoveBlendAlpha, 0.25f, 0.05f);
			}
			break;			
		}
		break;
	case SMT_ExitLadderOnTop:
		{
			if (bExitLadderLeftHand)
			{
				PlayFullBodyAnim(AnimNameExitLadderOnTopLH, 1.f, 0.25f, 0.5f);
			}
			else
			{
				PlayFullBodyAnim(AnimNameExitLadderOnTopRH, 1.f, 0.25f, 0.5f);
			}
			break;			
		}
		break;
	case SMT_PickupObject:
		{
			check(ActivePickup);

			FRotator relRotation = ActivePickup->AttachRotationOffset;
			FVector relLoc = ActivePickup->AttachPositionOffset;
			FVector pickupFwd = ActivePickup->Rotation.Right();

			if ((pickupFwd.SafeNormal2D() | EyeForward.SafeNormal2D()) < 0.0f)
			{
				// add an extra 180 rotation
				relRotation.Yaw = FRotator::NormalizeAxis(relRotation.Yaw + 180.0f*DEG_TO_UNR);
			}

			FVector posOffsetCS = relRotation.Quaternion().RotateVector(relLoc);
			FVector posOffsetWS = ActivePickup->Rotation.Quaternion().RotateVector(posOffsetCS);
			FVector effectivePickupLocation = ActivePickup->Location + posOffsetWS;
			FVector toPickup = effectivePickupLocation - Location;

			FLOAT distHorz = toPickup.Size2D();
			FLOAT distVert = toPickup.Z;

			UBOOL bIsDocument = ActivePickup->IsA(AOLCollectiblePickup::StaticClass());

			if (bPickupCrouched)
			{
				const FBlendSpaceNode crouchedNodes[] =
				{
					FBlendSpaceNode( AnimNamePickupObjectCrouched_h30vm10,	FVector2D(30.0f, -10.0f)	),
					FBlendSpaceNode( AnimNamePickupObjectCrouched_h30v60,		FVector2D(30.0f, 60.0f)		),
					FBlendSpaceNode( AnimNamePickupObjectCrouched_h60vm10,	FVector2D(60.0f, -10.0f)	),
					FBlendSpaceNode( AnimNamePickupObjectCrouched_h60v60,		FVector2D(60.0f, 60.0f)		),
					FBlendSpaceNode( AnimNamePickupObjectCrouched_h45v35,		FVector2D(45.0f, 35.0f)		),
				};

				const FBlendSpaceNode crouchedNodesDocument[] =
				{
					FBlendSpaceNode( AnimNamePickupDocCrouched_h30vm10,	FVector2D(30.0f, -10.0f)	),
					FBlendSpaceNode( AnimNamePickupDocCrouched_h30v60,		FVector2D(30.0f, 60.0f)		),
					FBlendSpaceNode( AnimNamePickupDocCrouched_h60vm10,	FVector2D(60.0f, -10.0f)	),
					FBlendSpaceNode( AnimNamePickupDocCrouched_h60v60,		FVector2D(60.0f, 60.0f)		),
					FBlendSpaceNode( AnimNamePickupDocCrouched_h45v35,		FVector2D(45.0f, 35.0f)		),
				};

				const INT crouchedBlendSpaces[][3] = 
				{
					{ 0, 4, 1 },
					{ 0, 2, 4 },
					{ 4, 2, 3 },
					{ 1, 4, 3 },
				};

				PlayBlendSpace(bIsDocument ? crouchedNodesDocument : crouchedNodes, 5, crouchedBlendSpaces, 4, FVector2D(distHorz, distVert), 0.25f, 0.25f);
			}
			else
			{
				const FBlendSpaceNode standingNodes[] =
				{
					FBlendSpaceNode( AnimNamePickupObject_h40v70,	FVector2D(40.0f, 70.0f)		),
					FBlendSpaceNode( AnimNamePickupObject_h40v140,	FVector2D(40.0f, 140.0f)	),
					FBlendSpaceNode( AnimNamePickupObject_h85v70,	FVector2D(85.0f, 70.0f)		),
					FBlendSpaceNode( AnimNamePickupObject_h85v140,	FVector2D(85.0f, 140.0f)	),
					FBlendSpaceNode( AnimNamePickupObject_h62v105,	FVector2D(62.0f, 105.0f)	),
				};

				const FBlendSpaceNode standingNodesDocument[] =
				{
					FBlendSpaceNode( AnimNamePickupDoc_h40v70,	FVector2D(40.0f, 70.0f)		),
					FBlendSpaceNode( AnimNamePickupDoc_h40v140,	FVector2D(40.0f, 140.0f)	),
					FBlendSpaceNode( AnimNamePickupDoc_h85v70,	FVector2D(85.0f, 70.0f)		),
					FBlendSpaceNode( AnimNamePickupDoc_h85v140,	FVector2D(85.0f, 140.0f)	),
					FBlendSpaceNode( AnimNamePickupDoc_h62v105,	FVector2D(62.0f, 105.0f)	),
				};

				const INT standingBlendSpaces[][3] = 
				{
					{ 0, 4, 1 },
					{ 0, 2, 4 },
					{ 4, 2, 3 },
					{ 1, 4, 3 },
				};

				PlayBlendSpace(bIsDocument ? standingNodesDocument : standingNodes, 5, standingBlendSpaces, 4, FVector2D(distHorz, distVert), 0.25f, 0.25f);
			}
		}
		break;
	case SMT_CSA:
		{
			if (ActiveCSA->AnimName != NAME_None)
			{
				PlayFullBodyAnim(ActiveCSA->AnimName, 1.f, 0.25f, 0.5f);
			}
		}
		break;
	case SMT_EnterDoorInteraction:
		{
			check(ActiveDoor);
			if (appIsNearlyZero(ActiveDoor->GetOpenAngle(), 0.1f))
			{
				switch (DoorOpeningType)
				{
				case DOT_RightPush:				
				case DOT_RightPull:
					PlayFullBodyAnim(AnimNameDoorAccessRight, 1.f, 0.25f, 0.25f);
					break;
				case DOT_LeftPush:
				case DOT_LeftPull:
					PlayFullBodyAnim(AnimNameDoorAccessLeft, 1.f, 0.25f, 0.25f);
					break;
				default:
					check(FALSE);
				}
			}
			else
			{
				check(appIsNearlyEqual(ActiveDoor->GetOpenAngle(), 15.0f, 0.1f));
				switch (DoorOpeningType)
				{
				case DOT_RightPush:
					PlayFullBodyAnim(AnimNameDoorAccessRightPush15, 1.f, 0.25f, 0.25f);
					break;
				case DOT_RightPull:
					PlayFullBodyAnim(AnimNameDoorAccessRightPull15, 1.f, 0.25f, 0.25f);
					break;
				case DOT_LeftPush:
					PlayFullBodyAnim(AnimNameDoorAccessLeftPush15, 1.f, 0.25f, 0.25f);
					break;
				case DOT_LeftPull:
					PlayFullBodyAnim(AnimNameDoorAccessLeftPull15, 1.f, 0.25f, 0.25f);
					break;
				default:
					check(FALSE);
				}
			}
		}
		break;		
	case SMT_TryOpenLockedDoor:
		{
			switch (DoorOpeningType)
			{
			case DOT_RightPush:				
			case DOT_RightPull:
				PlayFullBodyAnim(AnimNameDoorLockedRight, 1.f, 0.25f, 0.25f);
				break;
			case DOT_LeftPush:
			case DOT_LeftPull:
				PlayFullBodyAnim(AnimNameDoorLockedLeft, 1.f, 0.25f, 0.25f);
				break;
			default:
				check(FALSE);
			}
		}
		break;
	case SMT_OpenDoorInstant: 
		{
			switch (DoorOpeningType)
			{
			case DOT_RightPush:
				PlayFullBodyAnim(AnimNameDoorOpenPushRight, 1.f, 0.25f, 0.25f);
				break;
			case DOT_RightPull:
				PlayFullBodyAnim(AnimNameDoorOpenPullRight, 1.f, 0.25f, 0.25f);
				break;
			case DOT_LeftPush:
				PlayFullBodyAnim(AnimNameDoorOpenPushLeft, 1.f, 0.25f, 0.25f);
				break;
			case DOT_LeftPull:
				PlayFullBodyAnim(AnimNameDoorOpenPullLeft, 1.f, 0.25f, 0.25f);
				break;
			default:
				check(FALSE);
			}
		}
		break;			
	case SMT_OpenDoorPartial:
		{
			switch (DoorPartialOpenType)
			{
			case DPOT_LeftPush:
				PlayFullBodyAnim(AnimNameDoorOpenInsidePushLeft, 1.f, 0.25f, 0.25f);
				break;
			case DPOT_LeftPull:
				PlayFullBodyAnim(AnimNameDoorOpenPullLeft, 1.f, 0.25f, 0.25f);
				break;
			case DPOT_LeftSwipe:
				PlayFullBodyAnim(AnimNameCloseDoorLeftSide, 1.f, 0.25f, 0.25f);
				break;
			case DPOT_RightPush:
				PlayFullBodyAnim(AnimNameDoorOpenInsidePushRight, 1.f, 0.25f, 0.25f);
				break;
			case DPOT_RightPull:
				PlayFullBodyAnim(AnimNameDoorOpenPullRight, 1.f, 0.25f, 0.25f);
				break;
			case DPOT_RightSwipe:
				PlayFullBodyAnim(AnimNameCloseDoorRightSide, 1.f, 0.25f, 0.25f);
				break;
			default:
				check(FALSE);
			}
		}
		break;
	case SMT_RunThroughDoor:
		{
			switch (DoorPartialOpenType)
			{
			case DPOT_LeftPush:
				PlayFullBodyAnim(AnimNameDoorRunThroughLeft, 1.f, 0.15f, 0.15f);
				break;
			case DPOT_RightPush:
				PlayFullBodyAnim(AnimNameDoorRunThroughRight, 1.f, 0.15f, 0.15f);
				break;
			default:
				check(FALSE);
			}
		}
		break;
	case SMT_CloseDoor:
	case SMT_CloseDoorPositionned:
		{
			if (bQuietDoorInteraction)
			{
				switch (DoorClosingType)
				{
				case DCT_LeftFront:
					PlayFullBodyAnim(AnimNameCloseDoorLeftFrontSlow, 1.0f, 0.25f, 0.25f);
					break;
				case DCT_LeftSide:
					PlayFullBodyAnim(AnimNameCloseDoorLeftSideSlow, 1.0f, 0.25f, 0.25f);
					break;
				case DCT_LeftBack:
					PlayFullBodyAnim(AnimNameCloseDoorLeftBackSlow, 1.0f, 0.25f, 0.25f);
					break;
				case DCT_LeftInside:
					PlayFullBodyAnim(AnimNameCloseDoorLeftInsideSlow, 1.0f, 0.25f, 0.25f);
					break;
				case DCT_RightFront:
					PlayFullBodyAnim(AnimNameCloseDoorRightFrontSlow, 1.0f, 0.25f, 0.25f);
					break;
				case DCT_RightSide:
					PlayFullBodyAnim(AnimNameCloseDoorRightSideSlow, 1.0f, 0.25f, 0.25f);
					break;
				case DCT_RightBack:
					PlayFullBodyAnim(AnimNameCloseDoorRightBackSlow, 1.0f, 0.25f, 0.25f);
					break;			
				case DCT_RightInside:
					PlayFullBodyAnim(AnimNameCloseDoorRightInsideSlow, 1.0f, 0.25f, 0.25f);
					break;
				default:
					check(FALSE);
				}		

				DoorSlowClosingAnimStartTime = GWorld->GetTimeSeconds();
			}
			else
			{
				switch (DoorClosingType)
				{
				case DCT_LeftFront:
					PlayFullBodyAnim(AnimNameCloseDoorLeftFront, 1.f, 0.25f, 0.25f);
					break;
				case DCT_LeftSide:
					PlayFullBodyAnim(AnimNameCloseDoorLeftSide, 1.f, 0.25f, 0.25f);
					break;
				case DCT_LeftBack:
					PlayFullBodyAnim(AnimNameCloseDoorLeftBack, 1.f, 0.25f, 0.25f);
					break;
				case DCT_LeftInside:
					PlayFullBodyAnim(AnimNameCloseDoorLeftInside, 1.f, 0.25f, 0.25f);
					break;
				case DCT_RightFront:
					PlayFullBodyAnim(AnimNameCloseDoorRightFront, 1.f, 0.25f, 0.25f);
					break;
				case DCT_RightSide:
					PlayFullBodyAnim(AnimNameCloseDoorRightSide, 1.f, 0.25f, 0.25f);
					break;
				case DCT_RightBack:
					PlayFullBodyAnim(AnimNameCloseDoorRightBack, 1.f, 0.25f, 0.25f);
					break;			
				case DCT_RightInside:
					PlayFullBodyAnim(AnimNameCloseDoorRightInside, 1.f, 0.25f, 0.25f);
					break;
				default:
					check(FALSE);
				}		
			}
		}
		break;
	case SMT_OpenLockerFromOutside:
		{
			if (bLeftAnim)
			{
				PlayBlendedAnim(AnimNameLockerOpenStraight, AnimNameLockerOpen45Left, SpecialMoveBlendAlpha, 0.25f, 0.5f);
			}
			else
			{
				PlayBlendedAnim(AnimNameLockerOpenStraight, AnimNameLockerOpen45Right, SpecialMoveBlendAlpha, 0.25f, 0.5f);
			}		
		}
		break;
	case SMT_EnterLocker:
		{
			PlayFullBodyAnim(AnimNameHideInLocker, 1.f, 0.25f, 0.25f);
		}
		break;
	case SMT_HeroGrabbedFromLocker:
		{
			switch (EnemyType)
			{
			case ET_Soldier:
			case ET_Swarm:
			case ET_Other:
			case ET_Groom:
				PlayFullBodyAnim(AnimNameGrabFromLocker, 1.f, 0.25f, 0.25f);
				break;
			default:
				PlayFullBodyAnim(AnimNameGenericGrabFromLocker, 1.f, 0.25f, 0.25f);
				break;
			}
		}
		break;
	case SMT_EnterBed:
		{
			if (bLeftAnim)
			{
				PlayFullBodyAnim(bIsCrouched ? AnimNameEnterBedLeft : AnimNameEnterBedLeftFromStand, 1.f, 0.25f, 0.15f);
			}
			else
			{
				PlayFullBodyAnim(bIsCrouched ? AnimNameEnterBedRight : AnimNameEnterBedRightFromStand, 1.f, 0.25f, 0.15f);
			}			
		}	
		break;
	case SMT_ExitBed:
		{
			if (bLeftAnim)
			{
				PlayFullBodyAnim(bMustCrouchAfterSpecialMove ? AnimNameExitBedLeftToCrouch : AnimNameExitBedLeft, 1.f, 0.25f, 0.5f);
			}
			else
			{
				PlayFullBodyAnim(bMustCrouchAfterSpecialMove ? AnimNameExitBedRightToCrouch : AnimNameExitBedRight, 1.f, 0.25f, 0.5f);
			}			
		}	
		break;
	case SMT_BedReload:
		{
			if (CamcorderState == CCS_ReloadingActive)
			{
				PlayFullBodyAnim(AnimNameReloadBatteriesBed, 1.f, 0.0f, 0.0f);
			}
			else
			{
				PlayFullBodyAnim(AnimNameReloadBatteriesBedInactive, 1.f, 0.25f, 0.25f);
			}
		}	
		break;
	case SMT_SqueezeReload:
		{
			if (CamcorderState == CCS_ReloadingActive)
			{
				PlayFullBodyAnim(AnimNameReloadBatteriesSqueeze, 1.f, 0.0f, 0.0f);
			}
			else
			{
				PlayFullBodyAnim(AnimNameReloadBatteriesSqueezeInactive, 1.f, 0.25f, 0.25f);
			}
		}	
		break;
	case SMT_HeroGrabbedFromBed:
		{
			switch (EnemyType)
			{
			case ET_Soldier:
			case ET_Swarm:
			case ET_Other:
			case ET_Groom:
				if (bLeftAnim)
				{
					PlayFullBodyAnim(AnimNameGrabFromBedLeft, 1.f, 0.25f, 0.5f);
				}
				else
				{
					PlayFullBodyAnim(AnimNameGrabFromBedRight, 1.f, 0.25f, 0.5f);
				}
				break;
			default:
				if (bLeftAnim)
				{
					PlayFullBodyAnim(AnimNameGenericGrabFromBedLeft, 1.f, 0.25f, 0.5f);
				}
				else
				{
					PlayFullBodyAnim(AnimNameGenericGrabFromBedRight, 1.f, 0.25f, 0.5f);
				}
				break;
			}
		}
		break;
	case SMT_Dying:
		{
			if (bIsCrouched)
			{
				TWEAKABLE FLOAT startTime = 0.5f;
				TWEAKABLE FLOAT playRate = 0.25f;
				PlayFullBodyAnim(AnimNameGenericDeath, playRate, 0.25f, 0.1f, startTime);
			}
			else
			{
				TWEAKABLE FLOAT playRate = 0.5f;
				PlayFullBodyAnim(AnimNameGenericDeath, playRate, 0.25f, 0.1f);
			}
		}
		break;
	case SMT_HeroDecapitate:
		{
			if (EnemyType == ET_Groom)
			{
				PlayFullBodyAnim(AnimNameFatalityGroom, 1.0f, 0.25f, 0.1f);
			}
			else
			{
				PlayFullBodyAnim(AnimNameFatalitySoldier, 1.0f, 0.25f, 0.1f);
			}
		}
		break;
	case SMT_EnterStruggle:
		{
			PlayFullBodyAnim(OLPC->Struggle.Config.EntryAnimPlayer, 1.0f, OLPC->Struggle.Config.EntryPlayerBlendInTime, 0.1f);	
		}
		break;
	case SMT_ExitStruggle:
		{
			PlayFullBodyAnim(OLPC->Struggle.Config.ExitAnimPlayer, 1.0f, 0.25f, OLPC->Struggle.Config.ExitPlayerBlendOutTime);	
		}
		break;
	case SMT_KilledInStruggle:
		{
			if (OLPC->Struggle.Config.KilledAnimPlayer != NAME_None)
			{
				PlayFullBodyAnim(OLPC->Struggle.Config.KilledAnimPlayer, 1.0f, 0.25f, 0.0f);	
			}
			else
			{
				Die();
			}
		}
		break;
	case SMT_StartPushingObject:
		{
			PlayFullBodyAnim(bPushingFromBackEdge ? AnimNameEnterPushObjectLeft : AnimNameEnterPushObjectRight, 1.0f, 0.25f, 0.25f);	
		}
		break;
	case SMT_StopPushingObject:
		{
			PlayFullBodyAnim(bPushingFromBackEdge ? AnimNameExitPushObjectLeft : AnimNameExitPushObjectRight, 1.0f, 0.25f, 0.5f);	
		}
		break;
	case SMT_HeroGrabbedNormal:
		{
			FName animNameA = NAME_None;
			FName animNameB = NAME_None;

			if (bIsCrouched)
			{
				if (bLeftAnim)
				{
					animNameA = (SpecialMoveBlendAlpha < 0.5f) ? AnimNameGrabCrouched : AnimNameGrabCrouchedLeft180;
					animNameB = AnimNameGrabCrouchedLeft90;
				}
				else
				{
					animNameA = (SpecialMoveBlendAlpha < 0.5f) ? AnimNameGrabCrouched : AnimNameGrabCrouchedRight180;
					animNameB = AnimNameGrabCrouchedRight90;
				}
			}
			else
			{
				if (bLeftAnim)
				{
					animNameA = (SpecialMoveBlendAlpha < 0.5f) ? AnimNameGrabNormal : AnimNameGrabNormalLeft180;
					animNameB = AnimNameGrabNormalLeft90;
				}
				else
				{
					animNameA = (SpecialMoveBlendAlpha < 0.5f) ? AnimNameGrabNormal : AnimNameGrabNormalRight180;
					animNameB = AnimNameGrabNormalRight90;
				}
			}

			FLOAT alpha = (SpecialMoveBlendAlpha < 0.5f) ? 2.0f*(0.5f - SpecialMoveBlendAlpha) : 2.0f*(SpecialMoveBlendAlpha - 0.5f);
			PlayBlendedAnim(animNameA, animNameB, alpha, 0.1f, 0.5f);

			CustomBlendNode->bKeepLastPose = TRUE;
			ShadowProxyCustomBlendNode->bKeepLastPose = TRUE;
		}
		break;
	case SMT_HeroThrown:
		{
			TWEAKABLE FLOAT BlendOutTime = 0.25f;

			if (SpecialMoveTargetYaw >= 0.f && SpecialMoveTargetYaw <= HALF_PI)
			{
				PlayBlendedAnim(AnimNameThrownRight90, AnimNameThrown, SpecialMoveTargetYaw/HALF_PI, 0.2f, BlendOutTime);
			}
			else if (SpecialMoveTargetYaw > HALF_PI)
			{
				PlayBlendedAnim(AnimNameThrownRight180, AnimNameThrownRight90, (SpecialMoveTargetYaw - HALF_PI)/HALF_PI, 0.2f, BlendOutTime);
			}
			else if (SpecialMoveTargetYaw < 0.f && SpecialMoveTargetYaw >= -HALF_PI)
			{
				PlayBlendedAnim(AnimNameThrownLeft90, AnimNameThrown, SpecialMoveTargetYaw/-HALF_PI, 0.2f, BlendOutTime);
			}
			else if (SpecialMoveTargetYaw < -HALF_PI)
			{
				PlayBlendedAnim(AnimNameThrownLeft180, AnimNameThrownLeft90, (SpecialMoveTargetYaw + HALF_PI)/-HALF_PI, 0.2f, BlendOutTime);
			}
		}
		break;
	case SMT_HeroGrabbedFromUnder:
		{
			UBOOL bUseGenericAnims = FALSE;

			if (EnemyType == ET_Generic || EnemyType == ET_Surgeon || EnemyType == ET_Cannibal || EnemyType == ET_Other)
			{
				bUseGenericAnims = TRUE;
			}

			FName animNameA = NAME_None;
			FName animNameB = NAME_None;

			if (bLeftAnim)
			{
				if (bUseGenericAnims)
				{
					animNameA = (SpecialMoveBlendAlpha < 0.5f) ? AnimNameGenericGrabUnder : AnimNameGenericGrabUnderLeft180;
					animNameB = AnimNameGenericGrabUnderLeft90;
				}
				else
				{
					animNameA = (SpecialMoveBlendAlpha < 0.5f) ? AnimNameGrabUnder : AnimNameGrabUnderLeft180;
					animNameB = AnimNameGrabUnderLeft90;
				}
			}
			else
			{
				if (bUseGenericAnims)
				{
					animNameA = (SpecialMoveBlendAlpha < 0.5f) ? AnimNameGenericGrabUnder : AnimNameGenericGrabUnderRight180;
					animNameB = AnimNameGenericGrabUnderRight90;
				}
				else
				{
					animNameA = (SpecialMoveBlendAlpha < 0.5f) ? AnimNameGrabUnderCCW : AnimNameGrabUnderRight180;
					animNameB = AnimNameGrabUnderRight90;
				}
			}

			FLOAT alpha = (SpecialMoveBlendAlpha < 0.5f) ? 2.0f*(0.5f - SpecialMoveBlendAlpha) : 2.0f*(SpecialMoveBlendAlpha - 0.5f);
			PlayBlendedAnim(animNameA, animNameB, alpha, 0.1f, 0.5f);

			CustomBlendNode->bKeepLastPose = TRUE;
			ShadowProxyCustomBlendNode->bKeepLastPose = TRUE;
		}
		break;
	case SMT_ContextualLeanInsideTransition:
		{
			if (CornerPeek.PeekPosition == CPP_MiddleLeft)
			{
				PlayFullBodyAnim(AnimNameCornerTransitionFromLeft, 1.f, 0.1f, 0.0f);
			}
			else
			{
				PlayFullBodyAnim(AnimNameCornerTransitionFromRight, 1.f, 0.1f, 0.0f);
			}
		}
		break;
	case SMT_ExitContextualLeanForward:
		{
			if (CornerPeek.PeekPosition == CPP_MiddleLeft || CornerPeek.PeekPosition == CPP_Left)
			{
				PlayFullBodyAnim(CornerPeek.bRoundedCorner ? AnimNameExitCornerPeekForwardLeftSoft : AnimNameExitCornerPeekForwardLeftHard, 1.0f, 0.25f, 0.25f);	
			}
			else
			{
				PlayFullBodyAnim(CornerPeek.bRoundedCorner ? AnimNameExitCornerPeekForwardRightSoft : AnimNameExitCornerPeekForwardRightHard, 1.0f, 0.25f, 0.25f);	
			}
		}
		break;
	case SMT_HeroKilled:
		{
			FName animNameA = NAME_None;
			FName animNameB = NAME_None;

			if (EnemyType == ET_Swarm)
			{
				if (IsInLocker())
				{
					PlayFullBodyAnim(AnimNameFatalityLocker, 1.0f, 0.25f, 0.25f);
				}
				else if (bBackAnim)
				{
					PlayFullBodyAnim(AnimNameFatalityXplodeBack, 1.0f, 0.25f, 0.25f);
				}
				else
				{
					if (bLeftAnim)
					{
						animNameA = AnimNameFatalityXplodeFront;
						animNameB = AnimNameFatalityXplodeLeft;
					}
					else
					{
						animNameA = AnimNameFatalityXplodeFront;
						animNameB = AnimNameFatalityXplodeRight;
					}

					PlayBlendedAnim(animNameA, animNameB, SpecialMoveBlendAlpha, 0.2f, 0.5f);

					CustomBlendNode->bKeepLastPose = TRUE;
					ShadowProxyCustomBlendNode->bKeepLastPose = TRUE;
				}
			}
			else
			{
				if (bLeftAnim)
				{
					if (bBackAnim)
					{
						if (EnemyWeapon == WeaponType_Blunt)
						{
							animNameA = AnimNameFatalityClubBack;
							animNameB = AnimNameFatalityClubLeft;
						}
						else if (EnemyWeapon == WeaponType_Stab)
						{
							animNameA = AnimNameFatalityBackstabBack;
							animNameB = AnimNameFatalityBackstabLeft;
						}
						else
						{
							// Not Implemented, don't call.
							check(false);
						}
					}
					else
					{
						if (EnemyWeapon == WeaponType_Blunt)
						{
							animNameA = AnimNameFatalityClubFront;
							animNameB = AnimNameFatalityClubLeft;
						}
						else if (EnemyWeapon == WeaponType_Stab)
						{
							animNameA = AnimNameFatalityStabChopFront;
							animNameB = AnimNameFatalityStabChopLeft;
						}
						else
						{
							animNameA = AnimNameFatalityChokeFront;
							animNameB = AnimNameFatalityChokeLeft;
						}
					}
				}
				else
				{
					if (bBackAnim)
					{
						if (EnemyWeapon == WeaponType_Blunt)
						{
							animNameA = AnimNameFatalityClubBack;
							animNameB = AnimNameFatalityClubRight;
						}
						else if (EnemyWeapon == WeaponType_Stab)
						{
							animNameA = AnimNameFatalityBackstabBack;
							animNameB = AnimNameFatalityBackstabRight;
						}
						else
						{
							// Not Implemented, don't call.
							check(false);
						}
					}
					else
					{
						if (EnemyWeapon == WeaponType_Blunt)
						{
							animNameA = AnimNameFatalityClubFront;
							animNameB = AnimNameFatalityClubRight;
						}
						else if (EnemyWeapon == WeaponType_Stab)
						{
							animNameA = AnimNameFatalityStabChopFront;
							animNameB = AnimNameFatalityStabChopRight;
						}
						else
						{
							animNameA = AnimNameFatalityChokeFront;
							animNameB = AnimNameFatalityChokeRight;
						}
					}
				}

				PlayBlendedAnim(animNameA, animNameB, SpecialMoveBlendAlpha, 0.2f, 0.5f);

				CustomBlendNode->bKeepLastPose = TRUE;
				ShadowProxyCustomBlendNode->bKeepLastPose = TRUE;
			}
		}
		break;
	default:
		check(FALSE);
	}
}

UBOOL AOLHero::CouldStartSpecialMove(ESpecialMoveType moveType)
{
	UBOOL ok = FALSE;

	switch (moveType)
	{
	case SMT_SlideOver:	
		ok = (LocomotionMode == LM_Walk || LocomotionMode == LM_LookBack);
		break;
	case SMT_EnterSqueeze:
	case SMT_AutomaticSqueeze:
	case SMT_EnterLedgeWalk:
	case SMT_EnterDoorInteraction:
	case SMT_OpenDoorInstant:
	case SMT_OpenDoorPartial:
	case SMT_TryOpenLockedDoor:
	case SMT_Crouch:
	case SMT_Uncrouch:
	case SMT_GrabLedgeFromGround:	
	case SMT_CloseDoor:	
	case SMT_CloseDoorPositionned:
	case SMT_OpenLockerFromOutside:
	case SMT_EnterLocker:	
	case SMT_EnterLookBack:
	case SMT_StartPushingObject:	
		ok = (LocomotionMode == LM_Walk);
		break;
	case SMT_EnterLadderFromGround:
	case SMT_EnterLadderFromAbove:
	case SMT_CSA:
	case SMT_EnterContextualLean:
	case SMT_PickupObject:
	case SMT_RunThroughDoor:
		ok = (LocomotionMode == LM_Walk) && !IsReloading();
		break;
	case SMT_JumpOver:
	case SMT_ClimbUpObstacle:
		ok = (LocomotionMode == LM_Walk || LocomotionMode == LM_Fall || LocomotionMode == LM_LookBack);
		break;
	case SMT_JumpOverAndGrabLedge:
	case SMT_BigLanding:
		ok = (LocomotionMode == LM_Walk || LocomotionMode == LM_Fall);
		break;	
	case SMT_ClimbOverWall:
	case SMT_ClimbUpWall:
		ok = (LocomotionMode == LM_Walk || LocomotionMode == LM_LookBack);
		break;	
	case SMT_LedgeWalkTransition:
	case SMT_ExitLedgeWalk:
	case SMT_JumpFromLedgeWalk:
		ok = (LocomotionMode == LM_LedgeWalk);
		break;	
	case SMT_GrabLadderFromAir:
		ok = (LocomotionMode == LM_Fall) && !IsReloading();
		break;
	case SMT_GrabLedgeFromAir:
	case SMT_GrabAndClimb:
		ok = (LocomotionMode == LM_Fall);
		break;
	case SMT_StepUpAndLand:
		ok = (LocomotionMode == LM_Fall && bJumping);
		break;
	case SMT_DropFromLedge:
	case SMT_ClimbUpLedge:
		ok = (LocomotionMode == LM_LedgeHang) || (SpecialMove == SMT_GrabLedgeFromAir) || (SpecialMove == SMT_GrabLedgeFromGround);
		break;	
	case SMT_LedgeHangTransition:
		ok = (LocomotionMode == LM_LedgeHang);
		break;
	case SMT_ExitSqueeze:
	case SMT_HeroGrabbedFromSqueeze:
	case SMT_SqueezeReload:
		ok = (LocomotionMode == LM_Squeeze);
		break;
	case SMT_ExitLocker:
	case SMT_HeroGrabbedFromLocker:
		ok = (LocomotionMode == LM_Locker);
		break;
	case SMT_ExitLadderOnGround:
	case SMT_ExitLadderOnTop:
	case SMT_DropFromLadder:
		ok = (LocomotionMode == LM_Ladder);
		break;
	case SMT_EnterBed:
		ok = (LocomotionMode == LM_Walk);
		break;
	case SMT_ExitBed:
	case SMT_HeroGrabbedFromBed:
	case SMT_BedReload:
		ok = (LocomotionMode == LM_Bed);
		break;
	case SMT_ExitLookBack:
		ok = (LocomotionMode == LM_LookBack);
		break;
	case SMT_StopPushingObject:
		ok = (LocomotionMode == LM_Pushing);
		break;
	case SMT_PushFromLedgeProcedural:
		ok = (LocomotionMode == LM_Walk || LocomotionMode == LM_Fall);
		break;
	case SMT_PushFromLedgeAnimated:
		ok = (LocomotionMode == LM_Walk) && bIsCrouched;
		break;
	case SMT_ExitContextualLean:
	case SMT_ExitContextualLeanForward:
	case SMT_ContextualLeanInsideTransition:
		ok = (LocomotionMode == LM_ContextualLean);
		break;
	default:
		ok = TRUE; // no specific condition
	}
	
	return ok && Super::CouldStartSpecialMove(moveType);
}

UBOOL AOLHero::TryEnterLedgeWalk(const FVector& playerIntentDirection)
{
	TWEAKABLE FLOAT MinPlayerInputCosAngle = 0.707f; // 45 degs
	TWEAKABLE FLOAT activationRadius = 50.0f;

	if (SpecialMove != SMT_None || !CouldStartSpecialMove(SMT_EnterLedgeWalk))
	{
		return FALSE;
	}

	for (INT markerIdx = 0; markerIdx < CachedMarkers.Num(); markerIdx++)
	{
		AOLLedgeMarker* ledgeMarker = CachedMarkers(markerIdx);

		if (!ledgeMarker || !ledgeMarker->IsValid())
		{
			continue;
		}

		FVector toMarker = ledgeMarker->Location - Location;
		if (toMarker.SizeSquared() < Square(activationRadius) && Abs(toMarker.Z) < MaxStepHeight)
		{
			AOLLedgeMarker* nextMarker = NULL;

			if (ledgeMarker->Next && ((ledgeMarker->Next->Location - ledgeMarker->Location).SafeNormal2D() | toMarker.SafeNormal2D()) > 0.0f)
			{
				nextMarker = ledgeMarker->Next;

				if (!ledgeMarker->bCanLedgeWalk)
				{
					// Ledge disabled for ledge walk
					continue;
				}
			}
			else if (ledgeMarker->Prev && ((ledgeMarker->Prev->Location - ledgeMarker->Location).SafeNormal2D() | toMarker.SafeNormal2D()) > 0.0f)
			{
				nextMarker = ledgeMarker->Prev;

				if (!nextMarker->bCanLedgeWalk)
				{
					// Ledge disabled for ledge walk
					continue;
				}
			}
			else
			{
				continue; // no valid continuation
			}

			FVector passageDir = (nextMarker->Location - ledgeMarker->Location).SafeNormal();

			if ((playerIntentDirection | passageDir) < 0.0f)
			{
				return FALSE; // input isn't targetted
			}

			if ((CharForward | passageDir) < 0.5f)
			{
				return FALSE; // not looking towards ledge
			}
			
			FVector front = (passageDir ^ FVector(0, 0, 1.0f)).SafeNormal2D(); // Towards the drop. At this point we're not sure it's the right direction

			FCheckResult Hit(1.f);
			FVector startTrace = ledgeMarker->Location + 50.0f*passageDir + FVector(0, 0, 100.0f); // on the ledge, above the ground
			FVector endTrace = startTrace + LedgeWalkMaxWallDist*front;

			// Find the wall
			UBOOL clearInFront = GWorld->SingleLineCheck( Hit, this, endTrace, startTrace, TRACE_AllBlocking | TRACE_StopAtAnyHit, FVector(0.0f));

			endTrace = startTrace - LedgeWalkMaxWallDist*front;

			UBOOL clearBehind = GWorld->SingleLineCheck( Hit, this, endTrace, startTrace, TRACE_AllBlocking | TRACE_StopAtAnyHit, FVector(0.0f));

			if (clearBehind == clearInFront)
			{
				// No wall or no room - no go
				return FALSE;
			}
			else if (clearBehind)
			{
				front =- front; // We guessed wrong, that's ok.
			}

			UBOOL facingBwd = (toMarker | front) < 0.0f; // if true, we have a wall in front, and a drop to our side
			
			FVector slice(DefaultHero->CylinderComponent->CollisionRadius, DefaultHero->CylinderComponent->CollisionRadius, 1.0f);
			startTrace += (DefaultHero->CylinderComponent->CollisionRadius + Fudge)*front; // moved above the drop
			endTrace = startTrace - FVector(0, 0, 100.0f + LedgeWalkMinDrop);
			if (!GWorld->SingleLineCheck( Hit, this, endTrace, startTrace, TRACE_AllBlocking | TRACE_StopAtAnyHit, slice))
			{
				return FALSE; // not a ledge walk - no drop in front
			}

			AOLLedgeMarker* activeLedgeMarker = (nextMarker == ledgeMarker->Next) ? ledgeMarker : nextMarker; // set the active ledge to the first in the relation

			if (!TryCommitToSpecialMove(SMT_EnterLedgeWalk, activeLedgeMarker))
			{
				return FALSE;
			}

			// Seems good
			ActiveLedge = activeLedgeMarker;

			UBOOL toRight = ((toMarker.SafeNormal2D() ^ FVector(0, 0, 1.0f)) | passageDir) < 0.0f;
			
			FLOAT sideDistPerp = 0.0f;
			FLOAT sideDistPrl = 0.0f;
			FLOAT fwdDistPerp = 0.0f;
			FLOAT fwdDistPrl = 0.0f;

			if (toRight && facingBwd)
			{
				ActiveLedgeTransitionType = LTT_LeftInside;

				sideDistPerp = -LedgeWalkEnterExpectedInsidePerpSideDist;
				sideDistPrl = -LedgeWalkEnterExpectedInsidePrlSideDist;
				fwdDistPerp = LedgeWalkEnterExpectedInsidePerpFwdDist;
				fwdDistPrl = LedgeWalkEnterExpectedInsidePrlFwdDist;
			}
			else if (toRight && !facingBwd)
			{
				ActiveLedgeTransitionType = LTT_RightOutside;

				sideDistPerp = LedgeWalkEnterExpectedOutsidePerpSideDist;
				sideDistPrl = LedgeWalkEnterExpectedOutsidePrlSideDist;
				fwdDistPerp = LedgeWalkEnterExpectedOutsidePerpFwdDist;
				fwdDistPrl = LedgeWalkEnterExpectedOutsidePrlFwdDist;
			}
			else if (!toRight && facingBwd)
			{
				ActiveLedgeTransitionType = LTT_RightInside;

				sideDistPerp = -LedgeWalkEnterExpectedInsidePerpSideDist;
				sideDistPrl = -LedgeWalkEnterExpectedInsidePrlSideDist;
				fwdDistPerp = LedgeWalkEnterExpectedInsidePerpFwdDist;
				fwdDistPrl = LedgeWalkEnterExpectedInsidePrlFwdDist;
			}
			else
			{
				ActiveLedgeTransitionType = LTT_LeftOutside;

				sideDistPerp = LedgeWalkEnterExpectedOutsidePerpSideDist;
				sideDistPrl = LedgeWalkEnterExpectedOutsidePrlSideDist;
				fwdDistPerp = LedgeWalkEnterExpectedOutsidePerpFwdDist;
				fwdDistPrl = LedgeWalkEnterExpectedOutsidePrlFwdDist;
			}

			FVector toMarkerDir = toMarker.SafeNormal2D();			

			FLOAT angleToPassage = appAcos(CharForward | passageDir) * RAD_TO_DEG;
			SpecialMoveBlendAlpha = 1.0f - Clamp(angleToPassage / 90.0f, 0.0f, 1.0f); // 0.0f - 90 degs, 1.0f - straight

			FVector expectedAnimStartPerp = ledgeMarker->Location - sideDistPerp*front - fwdDistPerp*passageDir;
			FVector expectedAnimStartPrl = ledgeMarker->Location - sideDistPrl*front - fwdDistPrl*passageDir;
			FVector expectedAnimStart = SpecialMoveBlendAlpha*expectedAnimStartPrl + (1.0f - SpecialMoveBlendAlpha)*expectedAnimStartPerp;
			FVector expectedStartFwd = SpecialMoveBlendAlpha*passageDir + (1.0f - SpecialMoveBlendAlpha)*(facingBwd ? -1.0f : 1.0f)*front;

			StartSpecialMove(SMT_EnterLedgeWalk, expectedAnimStart, expectedStartFwd, APTT_TargetAtStart);

			return TRUE;						
		}
	}

	return FALSE;
}

UBOOL AOLHero::TryEnterSqueeze(const FVector& playerIntentDirection, AOLGameplayVolume* gameplayVolume)
{
	TWEAKABLE FLOAT MinPlayerInputCosAngle = 0.707f; // 45 degs

	if (SpecialMove != SMT_None || !CouldStartSpecialMove(SMT_EnterSqueeze))
	{
		return FALSE;
	}

	AOLSqueezeVolume* squeezeVolume = Cast<AOLSqueezeVolume>(gameplayVolume);
	if (!squeezeVolume || !squeezeVolume->IsValid())
	{
		return FALSE;
	}

	// Find start target and direction; check angles and distance
	FLOAT distSqToNode1 = (squeezeVolume->Node1->Location - Location).SizeSquared2D();
	FLOAT distSqToNode2 = (squeezeVolume->Node2->Location - Location).SizeSquared2D();

	AOLGameplayMarker* closestMarker = (distSqToNode2 > distSqToNode1) ? squeezeVolume->Node1 : squeezeVolume->Node2;
	AOLGameplayMarker* farMarker = (closestMarker == squeezeVolume->Node2) ? squeezeVolume->Node1 : squeezeVolume->Node2;
	FVector passageDir = (farMarker->Location - closestMarker->Location).SafeNormal();
	FVector toMarker = (closestMarker->Location - Location).SafeNormal2D();
	FVector targetPoint = closestMarker->Location + FVector(0,0,CylinderComponent->CollisionHeight);

	if ((passageDir | toMarker) < 0.0f)
	{
		// We're between both markers
		FVector levelLocation(Location.X, Location.Y, squeezeVolume->Node1->Location.Z);
		FLOAT dist = PointDistToSegment(levelLocation, squeezeVolume->Node1->Location, squeezeVolume->Node2->Location, targetPoint);
		if (dist > SqueezeInteractDist)
		{
			return FALSE; // too far away
		}
	}
	else if (Min(distSqToNode1, distSqToNode2) > Square(SqueezeInteractDist))
	{
		return FALSE; // too far away
	}
	
	if ((playerIntentDirection | passageDir) < MinPlayerInputCosAngle)
	{
		return FALSE; // input isn't targetted
	}
	else if ((CharForward | passageDir) < 0.707f)
	{
		return FALSE; // not looking towards the passage
	}

	if (squeezeVolume->bNoHands)
	{
		if (!TryCommitToSpecialMove(SMT_AutomaticSqueeze, farMarker))
		{
			return FALSE;
		}

		TWEAKABLE FLOAT AssumedSqueezeWidth = 50.0f;
		TWEAKABLE FLOAT ExpectedStartDist = 40.0f;
		FVector expectedAnimStart = farMarker->Location - (AssumedSqueezeWidth + ExpectedStartDist)*passageDir;
		expectedAnimStart.Z = Max(closestMarker->Location.Z, farMarker->Location.Z);
		FVector expectedAnimFwd = passageDir;

		StartSpecialMove(SMT_AutomaticSqueeze, expectedAnimStart, expectedAnimFwd, APTT_TargetAtStart);
		ActiveSqueeze = squeezeVolume;

		return TRUE;
	}
	else
	{
		if (!TryCommitToSpecialMove(SMT_EnterSqueeze, closestMarker))
		{
			return FALSE;
		}

		FVector facingDirection;
		UBOOL enterFacing = TRUE; // do we need to reverse our orientation
	
		if (squeezeVolume->bCanLookLeft && squeezeVolume->bCanLookRight)
		{
			FVector wallDir = (passageDir ^ FVector(0,0,1.0f)).SafeNormal();
			facingDirection = ((wallDir | CharForward) > 0.0f) ? wallDir : -wallDir;
		}
		else
		{
			FVector leftOfPassage = ((squeezeVolume->Node2->Location - squeezeVolume->Node1->Location).SafeNormal2D() ^ FVector(0, 0, 1.0f));						
			facingDirection = squeezeVolume->bCanLookLeft ? leftOfPassage : -leftOfPassage;
			enterFacing = (facingDirection | CharForward) > 0.0f;
		}

		bLeftAnim = ((facingDirection ^ FVector(0,0,1.0f)).SafeNormal() | passageDir) < 0.0f;
	
		FVector expectedAnimStart = closestMarker->Location - SqueezeEnterExpectedDistFwd*passageDir;
		expectedAnimStart.Z = Location.Z;
		FVector expectedAnimFwd = passageDir;
	
		StartSpecialMove(SMT_EnterSqueeze, expectedAnimStart, expectedAnimFwd, APTT_TargetAtStart);
		ActiveSqueeze = squeezeVolume;

		return TRUE;
	}
}

void AOLHero::GetSqueezeExitParams(const FVector& baseLoc, FVector& closestMarkerLoc, FVector& exitDir)
{
	FLOAT distSqToNode1 = (ActiveSqueeze->Node1->Location - baseLoc).SizeSquared2D();
	FLOAT distSqToNode2 = (ActiveSqueeze->Node2->Location - baseLoc).SizeSquared2D();

	const AOLGameplayMarker* closestMarker = (distSqToNode2 > distSqToNode1) ? ActiveSqueeze->Node1 : ActiveSqueeze->Node2;
	closestMarkerLoc = closestMarker->Location;
	exitDir = (closestMarkerLoc - ((distSqToNode2 > distSqToNode1) ? ActiveSqueeze->Node2->Location : ActiveSqueeze->Node1->Location)).SafeNormal2D(); // towards the exit
}

ESqueezeTransitionType AOLHero::GetSqueezeExitType(UBOOL bAIAttackRight, const FVector& exitDir)
{
	UBOOL goingRight = (exitDir | Rotation.Right()) > 0.0f;

	if (bAIAttackRight)
	{
		return goingRight ? STT_Right_Back : STT_Left_Facing;
	}
	else
	{
		return goingRight ? STT_Right_Facing : STT_Left_Back;
	}
}

UBOOL AOLHero::TryExitSqueeze(const FVector& playerIntentDirection, UBOOL bForce)
{
	if (!CouldStartSpecialMove(SMT_ExitSqueeze))
	{
		return FALSE;
	}

	if (!ActiveSqueeze || !Touching.ContainsItem(ActiveSqueeze))
	{
		// failsafe - should not happen
		StartSpecialMove(SMT_ExitSqueeze, Location, CharForward, APTT_TargetAtStart);
	}

	if (!bForce && Utils::IsBetweenMarkers(Location, ActiveSqueeze->Node1->Location, ActiveSqueeze->Node2->Location, -SqueezeExitExpectedDistFwd - 5.0f))
	{
		// Not close to an edge
		return FALSE;
	}
		
	FVector closestMarkerLoc;
	FVector exitDir;

	if (bForce)
	{
		exitDir = playerIntentDirection.SafeNormal2D();

		// when forced, we trust the playerIntentDirection
		FLOAT distToNode1 = (ActiveSqueeze->Node1->Location - Location) | exitDir;
		FLOAT distToNode2 = (ActiveSqueeze->Node2->Location - Location) | exitDir;

		closestMarkerLoc = (distToNode1 > distToNode2) ? ActiveSqueeze->Node1->Location : ActiveSqueeze->Node2->Location;
	}
	else
	{
		GetSqueezeExitParams(Location, closestMarkerLoc, exitDir);	

		if ((RealVelocity | exitDir) <= 0.0f)
		{
			return FALSE; // not moving towards exit
		}
		else if ((playerIntentDirection | exitDir) <= 0.0f)
		{
			return FALSE; // player intent isn't in this direction
		}
	}

	if (!TryCommitToSpecialMove(SMT_ExitSqueeze, ActiveSqueeze))
	{
		return FALSE;
	}

	SpecialMoveTargetYaw = UNR_TO_DEG * exitDir.Rotation().Yaw;	

	FVector expectedAnimStart = closestMarkerLoc - SqueezeExitExpectedDistFwd*exitDir;
	expectedAnimStart.Z = bForce ? closestMarkerLoc.Z : Location.Z; // Forcing marker Z when bForce to fix going through floor in prison
	bLeftAnim = (exitDir | Rotation.Right()) < 0.0f;
	StartSpecialMove(SMT_ExitSqueeze, expectedAnimStart, CharForward, APTT_TargetAtStart);
	
	return TRUE;
}

void AOLHero::SetEnemyType(AOLEnemyPawn* attacker)
{
	if (attacker->IsA(AOLEnemyGroom::StaticClass()))
	{
		EnemyType = ET_Groom;
	}
	else if (attacker->IsA(AOLEnemyCannibal::StaticClass()))
	{
		EnemyType = ET_Cannibal;
	}
	else if (attacker->IsA(AOLEnemySoldier::StaticClass()))
	{
		EnemyType = ET_Soldier;
	}
	else if (attacker->IsA(AOLEnemyGenericPatient::StaticClass()))
	{
		EnemyType = ET_Generic;
	}
	else if (attacker->IsA(AOLEnemySurgeon::StaticClass()))
	{
		EnemyType = ET_Surgeon;
	}
	else if (attacker->IsA(AOLEnemyNanoCloud::StaticClass()))
	{
		EnemyType = ET_Swarm;
	}
	else // Should never hit this.
	{
		EnemyType = ET_Other;
	}
}

UBOOL AOLHero::TryGrabFromSqueeze(AOLEnemyPawn* attacker)
{
	if (!CouldStartSpecialMove(SMT_HeroGrabbedFromSqueeze))
	{
		return FALSE;
	}

	check(ActiveSqueeze);
	check(attacker);

	if (Utils::IsBetweenMarkers(Location, ActiveSqueeze->Node1->Location, ActiveSqueeze->Node2->Location, -GrabFromSqueezeMaxDistance))
	{
		// Not close to an edge
		return FALSE;
	}

	SetEnemyType(attacker);

	FVector closestMarkerLoc;
	FVector exitDir;
	GetSqueezeExitParams(attacker->Location, closestMarkerLoc, exitDir);	
	SqueezeTransitionType = GetSqueezeExitType(attacker->Bot->bAttackRight, exitDir);

	FVector toCenterLine = (closestMarkerLoc - Location).ProjectOnTo(CharForward).SafeNormal2D();
	UBOOL onWrongSideOfCenter = (toCenterLine | CharForward) < 0.0f;
	FVector expectedAnimStart = closestMarkerLoc - GrabFromSqueezeExpectedDistance*exitDir - (onWrongSideOfCenter ? -1.0f : 1.0f)*SqueezeDistFromCenter*toCenterLine;
	expectedAnimStart.Z = Location.Z;

	FLOAT lookAngleFromFwd = UNR_TO_DEG * FRotator::NormalizeAxis(EyeRotation.Yaw - Rotation.Yaw);
	if (SqueezeTransitionType == STT_Left_Back || SqueezeTransitionType == STT_Left_Facing)
	{
		// make it so a positive angle is away from the exit
		lookAngleFromFwd = -lookAngleFromFwd;
	}

	SpecialMoveBlendAlpha = Saturate(-lookAngleFromFwd / 180.0f + 0.5f); // 0.0f = 0 angle (straight exit direction), 0.5f = 90 angle (looking straight ahead), 1.0f = 180 angle (looking away from the exit)
	
	StartSpecialMove(SMT_HeroGrabbedFromSqueeze, expectedAnimStart, CharForward, APTT_TargetAtStart);
	bPendingSpecialMoveAnims = TRUE;

	return TRUE;
}

UBOOL AOLHero::CanGrabFromSqueeze()
{
	if (!CouldStartSpecialMove(SMT_HeroGrabbedFromSqueeze))
	{
		return FALSE;
	}

	if (Utils::IsBetweenMarkers(Location, ActiveSqueeze->Node1->Location, ActiveSqueeze->Node2->Location, -GrabFromSqueezeMaxDistance))
	{
		// Not close to an edge
		return FALSE;
	}

	return TRUE;
}

void AOLHero::PlayGrabFromSqueezePhaseTwo()
{
	bPlayingSpecialMoveAnim = TRUE;
	bPendingSpecialMoveAnims = FALSE;
	
	switch (SqueezeTransitionType)
	{
	case STT_Left_Back:
		{
			PlayFullBodyAnim(AnimNameGrabFromSqueezeLeftBack, 1.0f, 0.1f, 0.5f);					
		}
		break;
	case STT_Left_Facing:
		{
			PlayFullBodyAnim(AnimNameGrabFromSqueezeLeftFacing, 1.0f, 0.1f, 0.5f);					
		}
		break;
	case STT_Right_Back:
		{
			PlayFullBodyAnim(AnimNameGrabFromSqueezeRightBack, 1.0f, 0.1f, 0.5f);					
		}
		break;
	case STT_Right_Facing:
		{
			PlayFullBodyAnim(AnimNameGrabFromSqueezeRightFacing, 1.0f, 0.1f, 0.5f);					
		}
		break;
	default:
		check(FALSE);
	}

	FullBodyAnimSlot->SetRootBoneAxisOption(RBA_Translate, RBA_Translate, RBA_Translate);
	FullBodyAnimSlot->SetRootBoneRotationOption(RRO_Extract, RRO_Extract, RRO_Extract);
}

UBOOL AOLHero::TryGrabFromLocker(AOLEnemyPawn* attacker)
{
	if (!CouldStartSpecialMove(SMT_HeroGrabbedFromLocker))
	{
		return FALSE;
	}

	SetEnemyType(attacker);

	StartSpecialMove(SMT_HeroGrabbedFromLocker);
	return TRUE;
}


UBOOL AOLHero::TryGrabNormal(AOLEnemyPawn* attacker, FVector startLocation, FVector attackStartLocation)
{
	if (!CouldStartSpecialMove(SMT_HeroGrabbedNormal))
	{
		return FALSE;
	}

	SetEnemyType(attacker);

	FVector toEnemy = (attackStartLocation - startLocation).SafeNormal2D();

	// a positive angle is away from the attacker
	FLOAT lookAngleToAttacker = UNR_TO_DEG * FRotator::NormalizeAxis(EyeRotation.Yaw - toEnemy.Rotation().Yaw);
	SpecialMoveBlendAlpha = Saturate( Abs(lookAngleToAttacker / 180.0f)); // 0.0f = 0 angle (facing attacker), 0.5f = 90 angle, 1.0f = 180 angle (looking away from attacker)
	bLeftAnim = lookAngleToAttacker > 0.0f;

	bWantsToCrouch = FALSE;

	if (EnemyType == ET_Groom)
	{
		TWEAKABLE FLOAT adjustRight = 5.5f;
		startLocation += adjustRight * Rotation.Right();
	}
	
	StartSpecialMove(SMT_HeroGrabbedNormal, startLocation, CharForward, APTT_TargetAtStart);
	return TRUE;
}

UBOOL AOLHero::TryThrowPlayer(AOLEnemyPawn* attacker, FLOAT throwRotation)
{
	if (!CouldStartSpecialMove(SMT_HeroThrown))
	{
		return FALSE;
	}

	SetEnemyType(attacker);

	if (SpecialMove != SMT_None)
	{
		// failsafe, as sometimes the enemy may trigger a throw before the anim end callback from the grab has been sent
		CustomBlendNode->bInhibitEndNotifies = TRUE;
	}

	SpecialMoveTargetYaw = throwRotation;

	//FVector animStart = attacker->Location + FVector(77.9f, 1.7f, 42.6f).RotateAngleAxis((-direction).Rotation().Yaw, FVector(0.f, 0.f, 1.f));
	StartSpecialMove(SMT_HeroThrown);

	return TRUE;
}

UBOOL AOLHero::TryDecapitate(AOLEnemyPawn* attacker)
{
	if (!CouldStartSpecialMove(SMT_HeroDecapitate))
	{
		return FALSE;
	}

	SetEnemyType(attacker);

	StartSpecialMove(SMT_HeroDecapitate);
	return TRUE;
}

UBOOL AOLHero::TryKillHero(AOLEnemyPawn* attacker, FVector enemyStartLocation, FVector direction)
{
	if (!CouldStartSpecialMove(SMT_HeroKilled))
	{
		return FALSE;
	}

	SetEnemyType(attacker);

	FVector animStart = enemyStartLocation + direction * attacker->KillDistance;

	// a positive angle is away from the attacker
	FVector ToEnemy = (attacker->Location - Location).SafeNormal2D();
	FLOAT lookAngleToAttacker = UNR_TO_DEG * FRotator::NormalizeAxis(EyeRotation.Yaw - ToEnemy.Rotation().Yaw);
	if (Abs(lookAngleToAttacker) > 90.0f)
	{
		bBackAnim = TRUE;
		SpecialMoveBlendAlpha = Saturate( (Abs(lookAngleToAttacker) - 90.0f) / 90.0f);
	}
	else
	{
		bBackAnim = FALSE;
		SpecialMoveBlendAlpha = 1.0f - Saturate( Abs(lookAngleToAttacker / 90.0f));
	}
	bLeftAnim = lookAngleToAttacker > 0.0f;

	EnemyWeapon = attacker->WeaponType;

	if (EnemyType == ET_Swarm && bBackAnim)
	{
		StartSpecialMove(SMT_HeroKilled);
	}
	else
	{
		StartSpecialMove(SMT_HeroKilled, animStart, CharForward, APTT_TargetAtStart);
	}

	if (EnemyType != ET_Swarm && (EnemyWeapon == WeaponType_Blunt || EnemyWeapon == WeaponType_Stab) && CustomBlendNode->bActive)
	{
		// check for collision
		FBoneAtom camEndPosCS = CustomBlendNode->GetCameraFinalRelativePosition();
		FVector camMvmt = Vec2D(Rotation.Quaternion().RotateVector(camEndPosCS.GetTranslation()));
		FVector endCamLoc = animStart + camMvmt;
		FVector adjustment = -camMvmt;

		UBOOL bNeedAdjustment = WouldEncroach(endCamLoc);

		if (!bNeedAdjustment)
		{
			FCheckResult Hit(1.f);
			FVector startTrace = EyeLocation;
			FVector endTrace = endCamLoc + VecZ(25.0f);

			if (!GWorld->SingleLineCheck( Hit, this, endTrace, startTrace, TRACE_AllBlocking | TRACE_StopAtAnyHit, FVector(0.0f)))
			{
				bNeedAdjustment = TRUE;
			}
		}
		
		UBOOL bDoAdjust = FALSE;

		if (bNeedAdjustment)
		{			
			if (WouldEncroach(Location + adjustment) || attacker->WouldEncroach(attacker->Location + adjustment))
			{
				debugf(TEXT("### Can't adjust death anim with delta %s"), *adjustment.ToString());
				CustomBlendNode->BlendOutTime = 0.0f;
				CustomBlendNode->StartBlendingOut();
				CustomBlendNode->StopTickingSequences();

				CancelSpecialMove();

				return FALSE;
			}
				
			debugf(TEXT("### Adjusting death anim with delta %s"), *adjustment.ToString());

			TWEAKABLE FLOAT AdjustTime = 0.65f;
			FProceduralAnimData animData;
			animData.PositionDelta = adjustment;
			FLOAT linearVel = camMvmt.Size2D() / AdjustTime;
			QueueProceduralAnim(animData, linearVel, 360.0f);
			attacker->QueueProceduralAnim(animData, linearVel, 360.0f);
		}
	}

	return TRUE;
}

UBOOL AOLHero::TryKillInLocker(class AOLEnemyPawn* attacker)
{
	SetEnemyType(attacker);

	if (!CouldStartSpecialMove(SMT_HeroKilled) || EnemyType != ET_Swarm || !IsInLocker())
	{
		return FALSE;
	}

	StartSpecialMove(SMT_HeroKilled);
	return TRUE;
}

UBOOL AOLHero::TryGrabFromUnder(AOLEnemyPawn* attacker, FVector startLocation, FVector attackerStartLocation)
{
	if (!CouldStartSpecialMove(SMT_HeroGrabbedFromUnder))
	{
		return FALSE;
	}

	SetEnemyType(attacker);

	FVector toEnemy = (attackerStartLocation - startLocation).SafeNormal2D();

	// a positive angle is away from the attacker
	FLOAT lookAngleToAttacker = UNR_TO_DEG * FRotator::NormalizeAxis(EyeRotation.Yaw - toEnemy.Rotation().Yaw);
	SpecialMoveBlendAlpha = Saturate( Abs(lookAngleToAttacker / 180.0f)); // 0.0f = 0 angle (facing attacker), 0.5f = 90 angle, 1.0f = 180 angle (looking away from attacker)
	bLeftAnim = lookAngleToAttacker > 0.0f;
	
	StartSpecialMove(SMT_HeroGrabbedFromUnder, startLocation, CharForward, APTT_TargetAtStart);
	return TRUE;
}

UBOOL AOLHero::TryExitLedgeWalk()
{
	if (TryAdjustCollisionSizeForLocomotionMode(LM_Walk))
	{
		FVector ledgeDir(0.0f);

		// Find the next segment
		if (Location.DistanceSquared(ActiveLedge->Location) < Location.DistanceSquared(ActiveLedge->Next->Location))
		{
			ledgeDir = (ActiveLedge->Next->Location - ActiveLedge->Location).SafeNormal2D();
		}
		else
		{
			ledgeDir = (ActiveLedge->Location - ActiveLedge->Next->Location).SafeNormal2D();
		}

		if (!TryCommitToSpecialMove(SMT_ExitLedgeWalk, ActiveLedge))
		{
			return FALSE;
		}

		if ((Rotation.Right() | ledgeDir) > 0.0f)
		{
			ActiveLedgeTransitionType = LTT_LeftInside;
		}
		else
		{
			ActiveLedgeTransitionType = LTT_LeftOutside;
		}		


		StartSpecialMove(SMT_ExitLedgeWalk);
		return TRUE;
	}	

	return FALSE;
}

AOLLedgeMarker* AOLHero::FindClosestLedge(FLOAT minRelZ, FLOAT maxRelZ, FLOAT maxFwdDist)
{
	AOLLedgeMarker* closestLedgeMarker = NULL;
	FLOAT bestDist = -1.0f;

	FLOAT minNodeZ = Location.Z + minRelZ;
	FLOAT maxNodeZ = Location.Z + maxRelZ;

	for (INT markerIdx = 0; markerIdx < CachedMarkers.Num(); markerIdx++)
	{
		AOLLedgeMarker* node1 = CachedMarkers(markerIdx);

		// check validity
		if (node1 && node1->IsValid() && node1->Next && node1->Next->IsValid())
		{			
			// Quick checks

			AOLLedgeMarker* node2 = node1->Next;

			const FVector& node1Loc = node1->Location;
			const FVector& node2Loc = node2->Location;

			if (Max(node1Loc.Z, node2Loc.Z) < minNodeZ || Min(node1Loc.Z, node2Loc.Z) > maxNodeZ)
			{
				continue;
			}

			FVector closestPoint;
			FVector dummy;
			SegmentDistToSegment(Vec2D(node1Loc), Vec2D(node2Loc), Vec2D(Location), Vec2D(Location + maxFwdDist*CharForward), closestPoint, dummy);

			FVector toClosestPoint = (closestPoint - Location);
			if ((toClosestPoint.SafeNormal2D() | CharForward) < 0.99f)
			{
				// not intersecting
				continue;
			}			
			
			FLOAT distToLedge = toClosestPoint | CharForward; // not the size of toClosestPoint to discard ledges at our back

			if (distToLedge > 0.0f && distToLedge < maxFwdDist && (!closestLedgeMarker || distToLedge < bestDist))
			{
				bestDist = distToLedge;
				closestLedgeMarker = node1;
			}
		}
	}

	return closestLedgeMarker;
}

AOLLedgeMarker* AOLHero::FindFarEdge(FVector& farEdge, const FVector& targetPoint, const FVector& crossingDirection, AOLLedgeMarker* node1, AOLLedgeMarker* node2, FLOAT maxWidth)
{
	for (INT markerIdx = 0; markerIdx < CachedMarkers.Num(); markerIdx++)
	{
		AOLLedgeMarker* farNode1 = CachedMarkers(markerIdx);

		if (farNode1 && farNode1->IsValid() && farNode1->Next && farNode1->Next->IsValid() && farNode1 != node1 && farNode1 != node2 && farNode1->Next != node1 && farNode1->Next != node2)
		{
			AOLLedgeMarker* farNode2 = farNode1->Next;

			FVector edge = farNode2->Location - farNode1->Location;
			FVector edgeDir = edge.SafeNormal2D();

			if (Abs(edgeDir | crossingDirection) > 0.17f)
			{
				// Not parallel within 10 degrees
				continue;
			}

			FVector dummy;
			SegmentDistToSegment(farNode1->Location, farNode2->Location, targetPoint, targetPoint + maxWidth*crossingDirection, farEdge, dummy);

			if (farEdge.DistanceSquared(targetPoint) < Square(maxWidth) && Abs(farEdge.Z - targetPoint.Z) < 10.0f)
			{
				FVector toFarEdge = (farEdge - targetPoint).SafeNormal2D();

				if ((toFarEdge | crossingDirection) > 0.98f) // must be parallel to the supposed crossing direction (and not e.g. slightly overlapping segment but side by side)
				{				
					return farNode1;
				}
			}
		}
	}

	return NULL;
}

UBOOL AOLHero::TryPassObstacle(const FVector& playerIntentDirection)
{	
	if (LocomotionMode != LM_Fall && LocomotionMode != LM_Walk && LocomotionMode != LM_LookBack)
	{
		return FALSE;		
	}

	// Just for early checks
	FLOAT MaxObstacleZ = 250.0f;
	FLOAT MaxObstacleDist = 500.0f;

	AOLLedgeMarker* ledgeMarker = FindClosestLedge(0.0f, MaxObstacleZ, MaxObstacleDist);

	if (ledgeMarker)
	{
		const FVector& effectiveIntentDir = (LocomotionMode == LM_LookBack) ? CharForward : playerIntentDirection;
				
		UBOOL moveTaken = TryPassObstacle(ledgeMarker, effectiveIntentDir);

		if (moveTaken)
		{
			return TRUE;
		}
	}

	return FALSE;
}

UBOOL AOLHero::TryPassObstacle(AOLLedgeMarker* node1, const FVector& playerIntentDirection)
{
	TWEAKABLE FLOAT MinPlayerInputCosAngle = 0.866f; // 30 degs

	UBOOL isFalling = Physics == PHYS_Falling;

	if (!isFalling && Physics != PHYS_Walking)
	{
		return FALSE;
	}

	UBOOL bRunningIntention = bWantToRun && IsRunning();

	FLOAT maxInteractDistance = 0.0f;
	FLOAT ObstacleMinZ = 0.0f;
	FLOAT ObstacleMaxZ = 0.0f;
	
	if (isFalling)
	{
		maxInteractDistance = Max(JumpOverInteractDistFalling, ClimbUpInteractDistFalling);
		maxInteractDistance = Max(maxInteractDistance, StepUpAndLandInteractDist);
		ObstacleMinZ = ObstacleMinZFalling;
		ObstacleMaxZ = ObstacleMaxZFalling;
	}
	else
	{
		if (bRunningIntention)
		{
			maxInteractDistance = Max(JumpOverInteractDistRunning, ClimbUpInteractDistRunning);
			maxInteractDistance = Max(maxInteractDistance, SlideOverInteractDistMax);
		}
		else
		{
			maxInteractDistance = Max(JumpOverInteractDistWalking, ClimbUpInteractDistWalking);
		}
		ObstacleMinZ = ObstacleMinZWalking;
		ObstacleMaxZ = ObstacleMaxZWalking;
	}

	AOLLedgeMarker* node2 = node1->Next;
	const FVector& node1Loc = node1->Location;
	const FVector& node2Loc = node2->Location;

	FVector ledge = node2Loc - node1Loc;

	// First some quick checks

	if ( Abs(CharForward | ledge.SafeNormal2D()) > MaxLedgeCosAngle)
	{
		// Too parallel to the ledge
		return FALSE;
	}

	if (!Utils::IsBetweenMarkers(Location, node1, node2, FALSE, 0.0f) && !Utils::IsBetweenMarkers(Location + 100.0f*CharForward, node1, node2, FALSE, 0.0f))
	{
		// Outside of the markers
		return FALSE;
	}

	FVector closestPoint;
	FVector dummy;
	SegmentDistToSegment(node1Loc, node2Loc, Location, Location + maxInteractDistance*CharForward, closestPoint, dummy);
	FVector toClosestPoint = (closestPoint - Location);
	FVector toClosestPoint2D(toClosestPoint.X, toClosestPoint.Y, 0.0f);
	FVector directionToLedge;
	FLOAT distToObstacle;
	toClosestPoint2D.ToDirectionAndLength(directionToLedge, distToObstacle);

	if ( (Location.Z + ObstacleMinZ) > closestPoint.Z)
	{
		// Ledge too low
		return FALSE;
	}

	if ( (Location.Z + ObstacleMaxZ) < closestPoint.Z)
	{
		// Ledge too high
		return FALSE;
	}

	if (distToObstacle > maxInteractDistance)
	{
		// Too far from ledge
		return FALSE;
	}
	else if ((directionToLedge | CharForward) < MinCosAngleToObstacle)
	{
		// Not looking towards the ledge
		return FALSE;
	}
	else if (isFalling && (RealVelocity.SafeNormal2D() | toClosestPoint2D.SafeNormal2D()) < 0.0f)
	{
		return FALSE; // velocity isn't towards the ledge (falling only)
	}

	// Check that this side of the ledge is free (make sure we don't have the far edge of a narrow obstacle)
	FCheckResult Hit(1.f);
	{
		FVector startTrace = closestPoint - 10.0f * directionToLedge + FVector(0, 0, 50.0f);
		FVector endTrace = FVector(startTrace.X, startTrace.Y, Location.Z + 50.0f); // 50 cm above our current location
		if (!GWorld->SingleLineCheck( Hit, this, endTrace, startTrace, TRACE_AllBlocking | TRACE_StopAtAnyHit, FVector(0.0f)))
		{
			// far edge of an obstacle, or 2nd step in a series
			return FALSE;
		}
	}

	TWEAKABLE FLOAT MinVelocityForStepUpAndLand = 100.0f;
	if (isFalling && CouldStartSpecialMove(SMT_StepUpAndLand) && RealVelocity.SizeSquared2D() > Square(MinVelocityForStepUpAndLand) && (distToObstacle <= StepUpAndLandInteractDist))
	{
		// We're falling towards a ledge. Can we step up and land?

		FVector testPoint = closestPoint + MinWidthForClimbWalking * CharForward;

		if (!WouldEncroach(testPoint))
		{			
			// Check that there is solid floor in front
			FVector startTrace = closestPoint + MinWidthForClimbWalking * directionToLedge + VecZ(50.0f);
			FVector endTrace = startTrace - VecZ(100.0f); // 50 cm down
			UBOOL canStepUpAndLand = !GWorld->SingleLineCheck( Hit, this, endTrace, startTrace, TRACE_AllBlocking | TRACE_StopAtAnyHit, FVector(0.0f));

			if (canStepUpAndLand && TryCommitToSpecialMove(SMT_StepUpAndLand))
			{	
				StartSpecialMove(SMT_StepUpAndLand);

				FLOAT deltaZ = closestPoint.Z - Location.Z;
				Location.Z += deltaZ;
				MeshZOffset = -deltaZ;
				FProceduralAnimData animData;
				animData.PositionDelta = CylinderComponent->CollisionRadius * CharForward;
				QueueProceduralAnim(animData);

				return TRUE;
			}
		}
	}

	UBOOL bTargettedInput = (playerIntentDirection | toClosestPoint2D.SafeNormal2D()) >= MinPlayerInputCosAngle;
	FLOAT obstacleHeight = toClosestPoint.Z;

	FVector crossingDirection = (ledge.SafeNormal2D() ^ VecZ(1.0f)); // perpendicular to the ledge
	if ((crossingDirection | directionToLedge) < 0.0f)
	{
		crossingDirection = -crossingDirection;
	}
	
	// Slide?
	UBOOL bCanSlide = bTargettedInput && bRunningIntention && (distToObstacle >= SlideOverInteractDistMin) && (distToObstacle <= SlideOverInteractDistMax) && node1->bCanSlide;
	bCanSlide = bCanSlide && IsBetween(obstacleHeight, MinHeightForSlide, MaxHeightForSlide) && CouldStartSpecialMove(SMT_SlideOver);
	if (bCanSlide)
	{
		// Check for a drop after the slide
		FVector startTrace = closestPoint + ( (MaxWidthForSlide + Fudge + CylinderComponent->CollisionRadius) * directionToLedge) + VecZ(50.0f);
		FVector endTrace(startTrace.X, startTrace.Y, startTrace.Z - 100.0f); // 50 cm down
		FVector slice(CylinderComponent->CollisionRadius, CylinderComponent->CollisionRadius, 1.0f);
		UBOOL slideOver = GWorld->SingleLineCheck( Hit, this, endTrace, startTrace, TRACE_AllBlocking | TRACE_StopAtAnyHit, slice);

		// Check that we can reach the end point (i.e. not the other side of a wall)
		if (slideOver)
		{
			endTrace = startTrace;
			startTrace = closestPoint + FVector(0, 0, 50.0f);
			slideOver = GWorld->SingleLineCheck( Hit, this, endTrace, startTrace, TRACE_AllBlocking | TRACE_StopAtAnyHit, slice);
		}

		// Check that there is still ground at the min width		
		if (slideOver)
		{
			startTrace = closestPoint + MinWidthForSlide * directionToLedge + FVector(0, 0, 50.0f);
			endTrace = FVector(startTrace.X, startTrace.Y, startTrace.Z - 100.0f); // 50 cm down
			slideOver = !GWorld->SingleLineCheck( Hit, this, endTrace, startTrace, TRACE_AllBlocking | TRACE_StopAtAnyHit, FVector(0.0f));

			if (slideOver && TryCommitToSpecialMove(SMT_SlideOver))
			{
				bRunningTraversalMove = TRUE;
				FVector expectedStartPos = closestPoint - SlideOverExpectedDist * directionToLedge - FVector(0, 0, SlideOverExpectedHeight);
				StartSpecialMove(SMT_SlideOver, expectedStartPos, CharForward, APTT_TargetAtStart);
				return TRUE;
			}
		}
	}	

	// None of the following moves can be applied to a foot-level obstacle
	if (obstacleHeight < ObstacleMinZWalking)
	{
		return FALSE;
	}
	
	// Jump over and grab ledge?
	if (CouldStartSpecialMove(SMT_JumpOverAndGrabLedge) && node1->bCanLedgeHang) 
	{	
		FLOAT ledgeSizeSq = ledge.SizeSquared();
		FVector targetPoint = closestPoint;

		if (ledgeSizeSq < Square(125.0f))
		{
			// Small ledge, probably an airvent. Center the target location
			targetPoint = 0.5f * (node1Loc + node2Loc);
		}	
		else if (!Utils::IsBetweenMarkers(Location, node1, node2, TRUE, -CylinderComponent->CollisionRadius))
		{			
			// Too close to one of the end markers, adjust the position to a collision radius from the closest marker
			if (node1Loc.DistanceSquared(Location) < node2Loc.DistanceSquared(Location))
			{
				targetPoint = node1Loc + CylinderComponent->CollisionRadius*ledge.SafeNormal();
			}
			else
			{
				targetPoint = node2Loc - CylinderComponent->CollisionRadius*ledge.SafeNormal();
			}
		}

		TWEAKABLE FLOAT JumpOverAndGrabMaxLedgeWidth = 55.0f;

		FVector farEdge = targetPoint;
		AOLLedgeMarker* farNode = FindFarEdge(farEdge, targetPoint, crossingDirection, node1, node2, JumpOverAndGrabMaxLedgeWidth);

		FLOAT ledgeWidth = (farNode != NULL) ? (farEdge - targetPoint).Size2D() : 0.0f;

		// Check for a large drop after the jump
		FLOAT testDist = ledgeWidth + 10.0f;
		FVector startTrace = closestPoint + ((testDist + CylinderComponent->CollisionRadius) * crossingDirection) + VecZ(50.0f);
		FVector endTrace = startTrace - VecZ(50.0f + GrabLedgeFromJumpOverMinFloorClearance); // enough clearance to grab a ledge
		FVector slice(CylinderComponent->CollisionRadius, CylinderComponent->CollisionRadius, 1.0f);
		UBOOL jumpOverToGrabLedge = GWorld->SingleLineCheck(Hit, this, endTrace, startTrace, TRACE_AllBlocking | TRACE_StopAtAnyHit, slice);

		if (jumpOverToGrabLedge)
		{
			FLOAT maxDist = 0.0f;

			if (Physics == PHYS_Falling)
			{
				maxDist = JumpOverAndGrabLedgeInteractDistFalling;
			}
			else
			{
				maxDist = bRunningIntention ? JumpOverAndGrabLedgeInteractDistRunning : JumpOverAndGrabLedgeInteractDistWalking;
			}

			if (distToObstacle > maxDist)
			{
				return FALSE; // too far
			}

			if (ledgeSizeSq < Square(CylinderComponent->CollisionRadius))
			{
				// ledge is too small
				return FALSE;
			}

			if (!TryCommitToSpecialMove(SMT_JumpOverAndGrabLedge))
			{
				return FALSE;
			}

			ActiveLedge = farNode ? farNode : node1;

			if (ledgeWidth > 0.0f)
			{
				FProceduralAnimData animData;
				animData.PositionDelta = ledgeWidth*crossingDirection;
				animData.bWaitForNotify = TRUE; // drive it through a notify only during the horizontal movement section
				QueueProceduralAnim(animData);
			}

			bRunningTraversalMove = bRunningIntention;

			FLOAT startDist = JumpOverAndGrabLedgeExpectedDist;
			FLOAT startHeight = JumpOverAndGrabLedgeExpectedHeightFromLedge;
			FVector expectedStartPos = targetPoint - JumpOverAndGrabLedgeExpectedDist * crossingDirection - VecZ(JumpOverAndGrabLedgeExpectedHeightFromLedge);
			StartSpecialMove(SMT_JumpOverAndGrabLedge, expectedStartPos, crossingDirection, APTT_TargetAtStart);

			return TRUE;
		}
	}

	// Jump over?
	if (CouldStartSpecialMove(SMT_JumpOver))
	{
		TWEAKABLE FLOAT MaxHeightForRegularJumpOver = 180.0f;
		UBOOL bJumpOverWall = obstacleHeight > MaxHeightForRegularJumpOver;

		FLOAT maxDistToObstacle = 0.0f;
		FLOAT maxObstacleWidth = 0.0f;

		if (Physics == PHYS_Falling)
		{
			maxDistToObstacle = JumpOverInteractDistFalling;
			maxObstacleWidth = MinWidthForClimbWalking;
		}
		else
		{
			maxDistToObstacle = bRunningIntention ? JumpOverInteractDistRunning : JumpOverInteractDistWalking;

			if (bJumpOverWall)
			{
				maxObstacleWidth = MinWidthForClimbUpWall;
			}
			else
			{
				maxObstacleWidth = bWantToRun ? MinWidthForClimbRunning : MinWidthForClimbWalking;
			}
		}

		UBOOL bRunningJumpOver = bWantToRun;

		// Check for a drop after the jump (normal jumpover)
		FVector startTrace = closestPoint + ( (maxObstacleWidth + Fudge + CylinderComponent->CollisionRadius) * directionToLedge) + VecZ(50.0f);
		FVector endTrace = startTrace - VecZ(100.0f); // 50 cm down
		FVector slice(CylinderComponent->CollisionRadius, CylinderComponent->CollisionRadius, 1.0f);
		UBOOL jumpOver = GWorld->SingleLineCheck( Hit, this, endTrace, startTrace, TRACE_AllBlocking | TRACE_StopAtAnyHit, slice);

		if (!jumpOver && !bJumpOverWall && Physics != PHYS_Falling && !bWantToRun)
		{
			// Give a second chance to walking interactions, using the run anim if the obstacle would allow it - for these cases we don't want the climb up

			maxObstacleWidth = MinWidthForClimbRunning;
			startTrace = closestPoint + ( (maxObstacleWidth + Fudge + CylinderComponent->CollisionRadius) * directionToLedge) + VecZ(50.0f);
			endTrace = startTrace - VecZ(100.0f); // 50 cm down
			jumpOver = GWorld->SingleLineCheck( Hit, this, endTrace, startTrace, TRACE_AllBlocking | TRACE_StopAtAnyHit, slice);

			if (jumpOver)
			{
				bRunningJumpOver = TRUE;
			}
		}

		if (jumpOver)
		{
			// horizontal trace to check that we're not going through a wall
			endTrace = startTrace;
			startTrace = closestPoint + VecZ(50.0f);
			jumpOver = GWorld->SingleLineCheck( Hit, this, endTrace, startTrace, TRACE_AllBlocking | TRACE_StopAtAnyHit, slice);
		}

		if (jumpOver)
		{
			if ((closestPoint.Z - Location.Z) < JumpOverMinObstacleZ)
			{
				// not a climb up, not a jump over (too low) - likely a marker elevated slightly above our surface (e.g. beds)
				return FALSE;
			}

			if (distToObstacle > maxDistToObstacle)
			{
				return FALSE; // too far
			}

			// Ensure no encroaching at the high point (prevent colliding against e.g. window frame)

			FVector testPoint = closestPoint + CylinderComponent->CollisionRadius * CharForward;
			if (WouldEncroach(closestPoint, SpecialMoveParams[SMT_JumpOver].GP.CollisionRadius, SpecialMoveParams[SMT_JumpOver].GP.CollisionHeight))
			{
				return FALSE; // would encroach against the side
			}

			if (bJumpOverWall && CouldStartSpecialMove(SMT_ClimbOverWall))
			{
				if (!TryCommitToSpecialMove(SMT_ClimbOverWall))
				{
					return FALSE;
				}

				FVector expectedStartPos = closestPoint - ClimbOverWallExpectedDist * directionToLedge - VecZ(ClimbOverWallExpectedHeight);			
				StartSpecialMove(SMT_ClimbOverWall, expectedStartPos, crossingDirection, APTT_TargetAtStart);

				return TRUE;
			}
			else if (bTargettedInput)
			{
				if (!TryCommitToSpecialMove(SMT_JumpOver))
				{
					return FALSE;
				}

				bRunningTraversalMove = bRunningJumpOver;

				FLOAT startDist = bRunningTraversalMove ? JumpOverExpectedDistRunning : JumpOverExpectedDistWalking;
				FLOAT startHeight = bRunningTraversalMove ? JumpOverExpectedHeightRunning : JumpOverExpectedHeightWalking;

				FVector expectedStartPos = closestPoint - startDist * directionToLedge - VecZ(startHeight);			
				StartSpecialMove(SMT_JumpOver, expectedStartPos, CharForward, APTT_TargetAtStart);

				return TRUE;
			}

			return FALSE;
		}
	}

	// Climb up?
	if (bTargettedInput && CouldStartSpecialMove(SMT_ClimbUpObstacle) && CouldStartSpecialMove(SMT_ClimbUpWall))
	{
		FLOAT maxDist = 0.0f;
		FLOAT maxHeight = 0.0f;

		if (Physics == PHYS_Falling)
		{
			maxDist = ClimbUpInteractDistFalling;
			maxHeight = GrabLedgeMinHeightInAir;
		}
		else
		{
			maxDist = bRunningIntention ? ClimbUpInteractDistRunning : ClimbUpInteractDistWalking;
			maxHeight = GrabLedgeMinHeightOnGround;
		}

		if (distToObstacle > maxDist)
		{
			return FALSE; // too far
		}

		if (toClosestPoint.Z >= maxHeight)
		{
			return FALSE; // too high - this is grab ledge stuff
		}

		FVector expectedStartPos(0.0f);		
		UBOOL bWallClimb = toClosestPoint.Z >= ClimbUpWallInteractHeightMin;

		FVector testPoint = closestPoint + CylinderComponent->CollisionRadius * directionToLedge;

		UBOOL bMustCrouch = FALSE;
		
		// Check whether we need to end up crouched
		if (WouldEncroach(testPoint))
		{
			if (WouldEncroach(testPoint, CrouchRadius, CrouchHeight))
			{
				return FALSE; // we'd be encroaching
			}

			if (toClosestPoint.Z < ClimbUpToCrouchMinHeight)
			{
				return FALSE; // too low for climb to crouch - more likely the side of a jump over with a ceiling obstacle 
			}

			bMustCrouch = TRUE;
			expectedStartPos = closestPoint - ClimbUpToCrouchExpectedDist * directionToLedge - VecZ(ClimbUpToCrouchExpectedHeight);
		}
		else if (bWallClimb)
		{
			FLOAT zOffset = 0.0f;
			if (toClosestPoint.Z < ClimbUpWallExpectedHeightMin)
			{
				zOffset = toClosestPoint.Z - ClimbUpWallExpectedHeightMin;
				SpecialMoveBlendAlpha = 0.0f;
			}
			else if (toClosestPoint.Z > ClimbUpWallExpectedHeightMax)
			{
				zOffset = toClosestPoint.Z - ClimbUpWallExpectedHeightMax;
				SpecialMoveBlendAlpha = 1.0f;
			}
			else
			{
				SpecialMoveBlendAlpha = MapClamped(toClosestPoint.Z, ClimbUpWallExpectedHeightMin, ClimbUpWallExpectedHeightMax, 0.0f, 1.0f);
			}

			expectedStartPos = closestPoint - ClimbUpWallExpectedDist * directionToLedge;
			expectedStartPos.Z = Location.Z + zOffset;
		}
		else if (bWantToRun)
		{
			expectedStartPos = closestPoint - ClimbUpExpectedDistRunning * directionToLedge;
			expectedStartPos.Z = closestPoint.Z - ClimbUpExpectedHeightRunning;
		}
		else
		{
			expectedStartPos = closestPoint - ClimbUpExpectedDistWalking * directionToLedge - VecZ(ClimbUpExpectedHeightWalking);
		}

		if (bWallClimb && !bMustCrouch)
		{
			if (!TryCommitToSpecialMove(SMT_ClimbUpWall))
			{
				return FALSE;
			}

			bRunningTraversalMove = bWantToRun;
			bMustCrouchAfterSpecialMove = bMustCrouch;

			StartSpecialMove(SMT_ClimbUpWall, expectedStartPos, crossingDirection, APTT_TargetAtStart);
		}
		else
		{
			if (!TryCommitToSpecialMove(SMT_ClimbUpObstacle))
			{
				return FALSE;
			}

			bRunningTraversalMove = bWantToRun;
			bMustCrouchAfterSpecialMove = bMustCrouch;

			StartSpecialMove(SMT_ClimbUpObstacle, expectedStartPos, directionToLedge, APTT_TargetAtStart);
		}
		
		return TRUE;	
	}

	return FALSE;
}

UBOOL AOLHero::TryGrabLedge(const FVector& playerIntentVelocity)
{	
	TWEAKABLE FLOAT ValidLedgeMaxZDelta = 20.0f;

	if (!CouldStartSpecialMove(SMT_GrabLedgeFromAir) && !CouldStartSpecialMove(SMT_GrabLedgeFromGround))
	{
		return FALSE;
	}

	TWEAKABLE FLOAT MinRegrabDelay = 2.0f;
	TWEAKABLE FLOAT RegrabZThreshold = 250.0f;

	if (Physics == PHYS_Falling && ((GWorld->GetTimeSeconds() - LastActiveLedgeTimestamp) < MinRegrabDelay) && (Location.Z > LastActiveLedgeZ - RegrabZThreshold))
	{
		// trying to grab a ledge right after leaving one (probably the same)
		return FALSE;
	}

	// Just for early checks
	TWEAKABLE FLOAT MinObstacleZ = 100.0f;
	TWEAKABLE FLOAT MaxObstacleZ = 350.0f;
	TWEAKABLE FLOAT MaxObstacleDist = 500.0f;

	AOLLedgeMarker* ledgeMarker = FindClosestLedge(MinObstacleZ, MaxObstacleZ, MaxObstacleDist);

	if (ledgeMarker && ledgeMarker->bCanLedgeHang && ledgeMarker->Next->bCanLedgeHang)
	{
		UBOOL moveTaken = TryGrabLedge(ledgeMarker, playerIntentVelocity);

		if (moveTaken)
		{
			return TRUE;
		}
	}	

	return FALSE;
}

UBOOL AOLHero::TryGrabLedge(AOLLedgeMarker* node1, const FVector& playerIntentVelocity)
{
	if (Physics != PHYS_Falling && Physics != PHYS_Walking)
	{
		return FALSE;
	}

	AOLLedgeMarker* node2 = node1->Next;
	const FVector& node1Loc = node1->Location;
	const FVector& node2Loc = node2->Location;

	FVector ledge = node2Loc - node1Loc;

	// First some quick checks

	UBOOL bGoingDown = RealVelocity.Z < 0.0f;
	UBOOL bGoingFullyDownwards = RealVelocity.SafeNormal().Z < -0.96f; // If true, we're basically falling vertically (the 2d movement direction is unreliable)	
	UBOOL bConsiderRunning = (bWantToRun && IsRunning());
	FLOAT minInteractDistance = bConsiderRunning ? GrabLedgeInteractDistRunning : GrabLedgeInteractDistWalking;

	FLOAT minHeight = 0.0f;
	FLOAT maxHeight = 0.0f;

	if (Physics == PHYS_Falling)
	{
		minInteractDistance = bGoingDown ? GrabLedgeInteractDistGoingDown : GrabLedgeInteractDistWalking;
		minHeight = bGoingDown ? GrabLedgeMinHeightInAirGoingDown : GrabLedgeMinHeightInAir; 
		maxHeight = bGoingDown ? GrabLedgeMaxHeightInAirGoingDown : GrabLedgeMaxHeightInAir;
	}
	else
	{
		minHeight = GrabLedgeMinHeightOnGround;
		maxHeight = GrabLedgeMaxHeightOnGround;
	}

	if ( Abs(CharForward | ledge.SafeNormal2D()) > MaxLedgeCosAngle)
	{
		// Too parallel to the ledge
		return FALSE;
	}

	FVector targetPoint;
	FLOAT ledgeSizeSq = ledge.SizeSquared();

	if (ledgeSizeSq < Square(CylinderComponent->CollisionRadius))
	{
		// ledge is too small
		return FALSE;
	}
	else if (!Utils::IsBetweenMarkers(Location, node1, node2, TRUE, 0.0f))
	{
		// not between the two markers
		return FALSE;
	}
	else if (ledgeSizeSq < Square(125.0f))
	{
		// Small ledge, probably an airvent. Center the target location
		targetPoint = 0.5f * (node1Loc + node2Loc);
	}	
	else if (!Utils::IsBetweenMarkers(Location, node1, node2, TRUE, -CylinderComponent->CollisionRadius))
	{			
		// Too close to one of the end markers, adjust the position to a collision radius from the closest marker
		if (node1Loc.DistanceSquared(Location) < node2Loc.DistanceSquared(Location))
		{
			targetPoint = node1Loc + CylinderComponent->CollisionRadius*ledge.SafeNormal();
		}
		else
		{
			targetPoint = node2Loc - CylinderComponent->CollisionRadius*ledge.SafeNormal();
		}
	}
	else
	{
		// find the closest point
		FVector dummy;
		SegmentDistToSegment(node1Loc, node2Loc, Location, Location + 200.0f*CharForward, targetPoint, dummy);
	}

	if ( (Location.Z + minHeight) > targetPoint.Z)
	{
		// Ledge too low
		return FALSE;
	}

	if ( (Location.Z + maxHeight) < targetPoint.Z)
	{
		// Ledge too high
		return FALSE;
	}

	FVector toTargetPoint = (targetPoint - Location);
	FVector toTargetPoint2D(toTargetPoint.X, toTargetPoint.Y, 0.0f);
	FVector directionToLedge;
	FLOAT distToObstacle;
	toTargetPoint2D.ToDirectionAndLength(directionToLedge, distToObstacle);

	FCheckResult Hit(1.f);

	if ((directionToLedge | CharForward) < MinCosAngleToObstacle)
	{
		// Not looking towards the ledge
		return FALSE;
	}
	else if ((Physics == PHYS_Falling) && !bGoingFullyDownwards && (RealVelocity.SafeNormal2D() | directionToLedge) < -0.707f)
	{
		// In air but moving horizontally away from ledge
		return FALSE;
	}
	else if (distToObstacle > minInteractDistance)
	{
		// Too far from ledge
		if (Physics == PHYS_Walking && !bConsiderRunning && distToObstacle < GrabLedgeInteractDistRunning)
		{
			// Let's give a chance and expand the distance for overhanging ledges - delay the line check after the cheaper math though
			FVector startTrace = targetPoint - 10.0f * directionToLedge;
			startTrace.Z = EyeLocation.Z;
			FVector endTrace = startTrace + 40.0f * directionToLedge;
			if (!GWorld->SingleLineCheck( Hit, this, endTrace, startTrace, TRACE_AllBlocking | TRACE_StopAtAnyHit, FVector(0.0f)))
			{
				// Not an overhang - bail out
				return FALSE;
			}
		}
		else
		{
			return FALSE;
		}
	}

	// Check that this side of the ledge is free (make sure we don't have the far edge of a narrow obstacle)	
	{
		FVector startTrace = targetPoint - 10.0f * directionToLedge + FVector(0, 0, 50.0f);
		FVector endTrace = FVector(startTrace.X, startTrace.Y, Location.Z + 50.0f); // 50 cm above our current location
		if (!GWorld->SingleLineCheck( Hit, this, endTrace, startTrace, TRACE_AllBlocking | TRACE_StopAtAnyHit, FVector(0.0f)))
		{
			// far edge of an obstacle, or 2nd step in a series
			return FALSE;
		}
	}

	FVector ledgeNormal = (ledge.SafeNormal2D() ^ VecZ(1.0f)); // away from ledge
	if ((ledgeNormal | directionToLedge) > 0.0f)
	{
		ledgeNormal = -ledgeNormal;
	}
	
	// Check if we have enough downward clearance to hang there
	{
		FLOAT colRadius = GetCollisionRadiusForLocomotionMode(LM_LedgeHang);
		FVector slice(colRadius, colRadius, 1.0f);

		FVector startTrace = targetPoint + ((LedgeHangDistToWall + Fudge) * ledgeNormal) + VecZ(20.0f);		
		FVector endTrace(startTrace.X, startTrace.Y, targetPoint.Z - GrabLedgeMinFloorClearance);
		
		FCheckResult Hit(1.f);
		if (!GWorld->SingleLineCheck( Hit, this, endTrace, startTrace, TRACE_AllBlocking | TRACE_StopAtAnyHit, slice))
		{
			// not enough downwards clearance - don't grab
			return FALSE;
		}
	}

	if (!TryCommitToSpecialMove((Physics == PHYS_Falling) ? SMT_GrabLedgeFromAir : SMT_GrabLedgeFromGround))
	{
		return FALSE;
	}

	// Execute the GrabLedge move
	{
		ActiveLedge = node1;

		FVector toWall = ledge ^ FVector(0,0,1.0f);
		if ((CharForward | toWall) < 0.0f)
		{
			toWall = - toWall;
		}
		
		FVector expectedAnimStart = Location + toTargetPoint2D + (LedgeHangDistToWall * ledgeNormal);

		if (Physics == PHYS_Falling)
		{
			expectedAnimStart.Z = targetPoint.Z - LedgeHangHeightToLedge - GrabLedgeFromAirExpectedHeight;
			StartSpecialMove(SMT_GrabLedgeFromAir, expectedAnimStart, toWall, APTT_TargetAtStart);			
		}
		else
		{
			FLOAT ledgeHeightFromGround = targetPoint.Z - Location.Z;
			FLOAT zOffset = 0.0f;

			if (ledgeHeightFromGround < 200.0f)
			{
				SpecialMoveBlendAlpha = 0.0f;
				zOffset = ledgeHeightFromGround - 200.0f;
			}
			else if (ledgeHeightFromGround > 300.0f)
			{
				SpecialMoveBlendAlpha = 1.0f;
				zOffset = ledgeHeightFromGround - 300.0f;
			}
			else
			{
				SpecialMoveBlendAlpha = MapClamped(ledgeHeightFromGround, 200.0f, 300.0f, 0.0f, 1.0f);
			}

			expectedAnimStart.Z = Location.Z + zOffset;

			StartSpecialMove(SMT_GrabLedgeFromGround, expectedAnimStart, toWall, APTT_TargetAtStart);
		}
	}

	return TRUE;	
}


UBOOL AOLHero::TryGrabAndClimb(const FVector& playerIntentVelocity)
{	
	if (!CouldStartSpecialMove(SMT_GrabAndClimb))
	{
		return FALSE;
	}

	if (RealVelocity.Z >= 0.0f)
	{
		// Must be going downwards
		return FALSE;
	}

	for (INT markerIdx = 0; markerIdx < CachedMarkers.Num(); markerIdx++)
	{
		AOLLedgeMarker* ledgeMarker = CachedMarkers(markerIdx);

		// check validity
		if (ledgeMarker && ledgeMarker->IsValid() && ledgeMarker->Next && ledgeMarker->Next->IsValid())
		{			
			UBOOL moveTaken = TryGrabAndClimb(ledgeMarker, playerIntentVelocity);

			if (moveTaken)
			{
				return TRUE;
			}
		}
	}

	return FALSE;
}

UBOOL AOLHero::TryGrabAndClimb(AOLLedgeMarker* node1, const FVector& playerIntentVelocity)
{
	AOLLedgeMarker* node2 = node1->Next;
	const FVector& node1Loc = node1->Location;
	const FVector& node2Loc = node2->Location;

	FVector ledge = node2Loc - node1Loc;

	// First some quick checks

	if ( (Location.Z + GrabAndClimbMinHeight) > Min(node1Loc.Z, node2Loc.Z))
	{
		// Ledge too low
		return FALSE;
	}

	if ( (Location.Z + GrabAndClimbMaxHeight) < Max(node1Loc.Z, node2Loc.Z))
	{
		// Ledge too high
		return FALSE;
	}

	if ( Abs(CharForward | ledge.SafeNormal2D()) > MaxLedgeCosAngle)
	{
		// Too parallel to the ledge
		return FALSE;
	}

	FVector targetPoint;
	FLOAT ledgeSizeSq = ledge.SizeSquared();

	if (ledgeSizeSq < Square(CylinderComponent->CollisionRadius))
	{
		// ledge is too small
		return FALSE;
	}
	else if (!Utils::IsBetweenMarkers(Location, node1, node2, TRUE, 0.0f))
	{
		// not between the two markers
		return FALSE;
	}
	else if (ledgeSizeSq < Square(125.0f))
	{
		// Small ledge, probably an airvent. Center the target location
		targetPoint = 0.5f * (node1Loc + node2Loc);
	}	
	else if (!Utils::IsBetweenMarkers(Location, node1, node2, TRUE, -CylinderComponent->CollisionRadius))
	{			
		// Too close to one of the end markers, adjust the position to a collision radius from the closest marker
		if (node1Loc.DistanceSquared(Location) < node2Loc.DistanceSquared(Location))
		{
			targetPoint = node1Loc + CylinderComponent->CollisionRadius*ledge.SafeNormal();
		}
		else
		{
			targetPoint = node2Loc - CylinderComponent->CollisionRadius*ledge.SafeNormal();
		}
	}
	else
	{
		// find the closest point
		FVector dummy;
		SegmentDistToSegment(node1Loc, node2Loc, Location, Location + GrabAndClimbInteractDist*CharForward, targetPoint, dummy);
	}

	FVector toTargetPoint = (targetPoint - Location);
	FVector toTargetPoint2D(toTargetPoint.X, toTargetPoint.Y, 0.0f);
	FVector directionToLedge;
	FLOAT distToObstacle;
	toTargetPoint2D.ToDirectionAndLength(directionToLedge, distToObstacle);

	TWEAKABLE FLOAT MinVelTowardsLedge = 0.0f;

	if (distToObstacle > GrabAndClimbInteractDist)
	{
		// Too far from ledge
		return FALSE;
	}
	else if ((directionToLedge | CharForward) < MinCosAngleToObstacle)
	{
		// Not looking towards the ledge
		return FALSE;
	}
	else if ((RealVelocity | directionToLedge) < MinVelTowardsLedge)
	{
		// Not moving towards the ledge at a fast enough speed
		return FALSE;
	}

	FVector ledgeNormal = (ledge.SafeNormal2D() ^ VecZ(1.0f)); // away from ledge
	if ((ledgeNormal | directionToLedge) > 0.0f)
	{
		ledgeNormal = -ledgeNormal;
	}

	// Check that there's a surface to stand on on the other side
	FVector testLocation = targetPoint + 50.0f * CharForward - VecZ(50.0f);
	if (!WouldEncroach(testLocation))
	{
		// There's a drop after the ledge - can't perform this move
		return FALSE;
	}

	// Check that we wouldn't encroach standing up
	testLocation = targetPoint + (DefaultHero->CylinderComponent->CollisionRadius + Fudge) * CharForward;
	if (WouldEncroach(testLocation))
	{
		// no clearance to stand up
		return FALSE;
	}

	if (!TryCommitToSpecialMove(SMT_GrabAndClimb))
	{
		return FALSE;
	}

	// Proceed to GrabAndClimb
	{
		FVector expectedAnimStart = targetPoint - GrabAndClimbExpectedDistFwd*directionToLedge - VecZ(GrabAndClimbExpectedDistHeight);
		FVector toWall = ledge ^ FVector(0,0,1.0f);
		if ((CharForward | toWall) < 0.0f)
		{
			toWall = - toWall;
		}

		StartSpecialMove(SMT_GrabAndClimb, expectedAnimStart, toWall, APTT_TargetAtStart);
	}

	return TRUE;	
}

UBOOL AOLHero::TryClimbUpLedge(UBOOL playerInteraction, const FVector& playerIntentVelocity)
{
	if (!CouldStartSpecialMove(SMT_ClimbUpLedge))
	{
		// early out if possible
		return FALSE;
	}

	if (!ActiveLedge->bCanClimbUp)
	{
		// Climbing up disabled on this ledge
		return FALSE;
	}

	ELedgeClimbType climbType = (ELedgeClimbType)0;
	
	FVector closestPoint;
	FVector dummy;
	SegmentDistToSegment(ActiveLedge->Location, ActiveLedge->Next->Location, Location, Location + 100.0f*CharForward, closestPoint, dummy);

	// First check to make sure we have clearance to get above the ledge crouching
	FVector testLocation = closestPoint + CrouchRadius * CharForward + VecZ(Fudge);
	if (WouldEncroach(testLocation, CrouchRadius, CrouchHeight))
	{
		// No move is possible
		return FALSE;
	}

	UBOOL bNeedToCrouchAfterMove = FALSE;

	// Then check which type of clearance we have
	testLocation = closestPoint + (LedgeHangClimbOverMaxLedgeWidth + Fudge) * CharForward - VecZ(LedgeHangClimbOverFullHeightClearance);
	if (!WouldEncroach(testLocation))
	{
		// full clearance on other side of thin ledge
		climbType = LCT_ClimbOverToStand;
	}
	else
	{
		testLocation = closestPoint + (LedgeHangClimbOverMaxLedgeWidth + Fudge) * CharForward - VecZ(LedgeHangClimbOverMinHeightClearance);
		if (!WouldEncroach(testLocation))
		{
			// limited clearance on other side of thin ledge
			climbType = LCT_ClimbOverToFalling;
		}
		else
		{
			testLocation = closestPoint + LedgeHangClimbUpMinFwdClearance * CharForward + VecZ(Fudge);
			if (!WouldEncroach(testLocation))
			{
				// can climb up and stand
				climbType = LCT_ClimbUpToStand;
			}
			else
			{
				// climb up and crouch
				climbType = LCT_ClimbUpToCrouch;
				bNeedToCrouchAfterMove = TRUE;				
			}
		}
	}

	UBOOL playerIntent = playerInteraction;
	if (!playerIntent && LocomotionMode == LM_LedgeHang)
	{
		FVector toClosestPoint2d = (closestPoint - Location).SafeNormal2D();
		UBOOL bLargeIntent = (playerIntentVelocity.SizeSquared2D() > Square(500.0f));
		UBOOL bIntentTowardsLedge = ((playerIntentVelocity.SafeNormal2D() | toClosestPoint2d) > 0.707f);
		UBOOL bLookingTowardsLedge = ((EyeForward.SafeNormal2D() | toClosestPoint2d) > 0.707f);
		playerIntent = bLargeIntent && bIntentTowardsLedge && bLookingTowardsLedge;
	}

	if (!playerIntent)
	{
		OLPC->AddAvailableInteraction(PIT_ClimbUpLedge);
		return FALSE;
	}

	if (!TryCommitToSpecialMove(SMT_ClimbUpLedge))
	{
		return FALSE;
	}

	if (climbType == LCT_ClimbOverToFalling || climbType == LCT_ClimbOverToStand)
	{
		// See if this is a wide ledge (otherwise assuming thin rail)
		FVector farEdge = closestPoint;
		AOLLedgeMarker* farNode = FindFarEdge(farEdge, closestPoint, CharForward, ActiveLedge, ActiveLedge->Next, LedgeHangClimbOverMaxLedgeWidth);
		if (farNode)
		{
			// Wide - use a procedural forward push
			FProceduralAnimData animData;
			animData.PositionDelta = LedgeHangClimbOverMaxLedgeWidth * CharForward;				
			animData.bWaitForNotify = TRUE; // drive it through a notify only during the horizontal movement section
			QueueProceduralAnim(animData);
		}
	}

	bMustCrouchAfterSpecialMove = bNeedToCrouchAfterMove;
	LedgeClimbType = climbType;

	FVector expectedAnimStart = Location;
	expectedAnimStart.Z = closestPoint.Z - LedgeHangHeightToLedge;

	StartSpecialMove(SMT_ClimbUpLedge, expectedAnimStart, CharForward, APTT_TargetAtStart);
	return TRUE;
}

UBOOL AOLHero::TryLedgeTransition(const FVector& playerIntentDirection)
{
	TWEAKABLE FLOAT ContinuousSegmentsMinCosAngle = 0.86f;

	UBOOL inLedgeHang = (LocomotionMode == LM_LedgeHang);

	if (!CouldStartSpecialMove(SMT_LedgeHangTransition) && !CouldStartSpecialMove(SMT_LedgeWalkTransition))
	{
		// early out if possible
		return FALSE;
	}

	check(ActiveLedge);

	AOLLedgeMarker* closestMarker = NULL;
	AOLLedgeMarker* nextMarker = NULL;
	AOLLedgeMarker* prevMarker = NULL;

	// Find the next segment
	if (Location.DistanceSquared(ActiveLedge->Location) < Location.DistanceSquared(ActiveLedge->Next->Location))
	{
		closestMarker = ActiveLedge;
		nextMarker = ActiveLedge->Prev;
		prevMarker = ActiveLedge->Next;
	}
	else
	{
		closestMarker = ActiveLedge->Next;
		nextMarker = ActiveLedge->Next->Next;
		prevMarker = ActiveLedge;
	}

	if (inLedgeHang && (!nextMarker || !nextMarker->bCanLedgeHang))
	{
		return FALSE; // no next segment, or ledge hang not enabled on next segment
	}
	
	FVector currentLedgeDir = (closestMarker->Location - prevMarker->Location).SafeNormal(); // towards current motion
	FVector toClosestMarker = (closestMarker->Location - Location);
	FLOAT distSqToMarker = toClosestMarker.ProjectOnTo(currentLedgeDir).SizeSquared2D();

	FLOAT maxInteractDist = inLedgeHang ? LedgeHangTransitionInteractDist : Max(LedgeWalkTransitionInteractDistInside, LedgeWalkTransitionInteractDistOutside);
	
	if (distSqToMarker > Square(maxInteractDist))
	{
		return FALSE; // Too far from the edge
	}

	if ((playerIntentDirection | currentLedgeDir) < 0.0f)
	{
		return FALSE; // Intent isn't directed
	}
	
	if (!inLedgeHang && distSqToMarker <= Square(LedgeWalkTransitionInteractDistExit)) 
	{
		// See if we should get out of ledge walk

		FVector wallDirection = nextMarker ? (nextMarker->Location - closestMarker->Location).SafeNormal() : currentLedgeDir;
		FVector wallSide = (wallDirection ^ FVector(0, 0, 1.0f)).SafeNormal2D();

		FCheckResult Hit(1.f);
		FVector startTrace = closestMarker->Location + 50.0f*wallDirection + FVector(0, 0, 100.0f); // on the next ledge, above the ground
		FVector endTrace = startTrace + LedgeWalkMaxWallDist*wallSide;

		FVector targetPoint(0.0f);
		UBOOL validWall = FALSE;

		// Find the wall
		if (!GWorld->SingleLineCheck( Hit, this, endTrace, startTrace, TRACE_AllBlocking | TRACE_StopAtAnyHit, FVector(0.0f)))
		{
			validWall = TRUE;
		}
		else
		{
			// No wall on this side
			wallSide = -wallSide;
			
			endTrace = startTrace + LedgeWalkMaxWallDist*wallSide;
			if (!GWorld->SingleLineCheck( Hit, this, endTrace, startTrace, TRACE_AllBlocking | TRACE_StopAtAnyHit, FVector(0.0f)))
			{
				validWall = TRUE;
			}
			else
			{
				// No wall on either side
				targetPoint = closestMarker->Location + 1.5f*DefaultHero->CylinderComponent->CollisionRadius*wallDirection + FVector(0,0, Fudge);
			}
		}

		if (!nextMarker || !nextMarker->bCanLedgeWalk || !validWall)
		{
			// Nowhere to go - get out of ledge walk

			if (validWall)
			{
				targetPoint = closestMarker->Location + 1.5f*DefaultHero->CylinderComponent->CollisionRadius*wallDirection - DefaultHero->CylinderComponent->CollisionRadius*wallSide + FVector(0,0, Fudge);
			}

			FVector slice(CylinderComponent->CollisionRadius, CylinderComponent->CollisionRadius, 1.0f);
			startTrace = targetPoint + FVector(0, 0, 100.0f);
			endTrace = startTrace - FVector(0, 0, 100.0f + LedgeWalkMinDrop);
			if (GWorld->SingleLineCheck( Hit, this, endTrace, startTrace, TRACE_AllBlocking | TRACE_StopAtAnyHit, slice))
			{
				// we'd drop - don't move
				return FALSE;
			}

			FVector realLoc = Location;
			Location = targetPoint; // TODO: replace this with a cleaner test...
			UBOOL clear = TryAdjustCollisionSizeForLocomotionMode(LM_Walk);
			Location = realLoc;

			if (clear)
			{
				if (!TryCommitToSpecialMove(SMT_ExitLedgeWalk))
				{
					return FALSE;
				}

				UBOOL inside = !nextMarker || (Abs((nextMarker->Location - closestMarker->Location).SafeNormal() | currentLedgeDir) < ContinuousSegmentsMinCosAngle);
				UBOOL goingRight = (Rotation.Right() | currentLedgeDir) > 0.0f;

				if (inside && goingRight)
				{
					ActiveLedgeTransitionType = LTT_RightInside;
				}
				else if (inside && !goingRight)
				{
					ActiveLedgeTransitionType = LTT_LeftInside;
				}
				else if (!inside && goingRight)
				{
					ActiveLedgeTransitionType = LTT_RightOutside;
				}
				else
				{
					ActiveLedgeTransitionType = LTT_LeftOutside;
				}

				StartSpecialMove(SMT_ExitLedgeWalk);				
				return TRUE;
			}

			return FALSE;
		}
	}

	if (!nextMarker || !nextMarker->bCanLedgeWalk)
	{
		return FALSE; // can't transition, but not yet in range for exit
	}

	FVector nextStretch = (nextMarker->Location - closestMarker->Location).SafeNormal();
	UBOOL continuousStretch = (Abs(nextStretch | currentLedgeDir) >= ContinuousSegmentsMinCosAngle);
	
	if (continuousStretch)
	{
		return FALSE; // do not need a special transition - handled by CalcVelocity()
	}

	FVector currentToWall = (inLedgeHang ? CharForward : -CharForward);
	UBOOL inside = (nextStretch | currentToWall) < 0.0f;

	if (inLedgeHang)
	{
		// Check if we have enough downward clearance to hang there
		FLOAT colRadius = GetCollisionRadiusForLocomotionMode(LM_LedgeHang);
		FVector slice(colRadius, colRadius, 1.0f);

		FVector startTrace = closestMarker->Location + LedgeHangDistToWall*nextStretch + LedgeHangDistToWall*(inside ? -currentLedgeDir : currentLedgeDir) + VecZ(20.0f);		
		FVector endTrace(startTrace.X, startTrace.Y, closestMarker->Location.Z - GrabLedgeMinFloorClearance);

		FCheckResult Hit(1.f);
		if (!GWorld->SingleLineCheck( Hit, this, endTrace, startTrace, TRACE_AllBlocking | TRACE_StopAtAnyHit, slice))
		{
			// not enough downwards clearance - can't transition
			return FALSE;
		}

	}
	else
	{
		FLOAT interactDist = inside ? LedgeWalkTransitionInteractDistInside : LedgeWalkTransitionInteractDistOutside;
		if (distSqToMarker > Square(interactDist))
		{
			return FALSE; // Too far from the edge for this type of transition
		}
	}

	if (!TryCommitToSpecialMove(inLedgeHang ? SMT_LedgeHangTransition : SMT_LedgeWalkTransition))
	{
		return FALSE;
	}

	// At this point we should be good for a transition
	
	ActiveLedge = (nextMarker == closestMarker->Next) ? closestMarker : nextMarker;

	UBOOL goingRight = (Rotation.Right() | currentLedgeDir) > 0.0f;
	ActiveLedgeTransitionType = inside ? (goingRight ? LTT_RightInside : LTT_LeftInside) : (goingRight ? LTT_RightOutside : LTT_LeftOutside);

	if (inLedgeHang)
	{	
		FLOAT expectedDistToLedge = (inside ? LedgeHangTransitionInsideExpectedDist : LedgeHangTransitionOutsideExpectedDist);
		FVector expectedAnimStart = closestMarker->Location - expectedDistToLedge*currentLedgeDir - LedgeHangDistToWall*currentToWall - FVector(0, 0, LedgeHangHeightToLedge);
				
		StartSpecialMove(SMT_LedgeHangTransition, expectedAnimStart, currentToWall, APTT_TargetAtStart);
	}
	else
	{		
		FLOAT expectedDistToLedge = (inside ? LedgeWalkTransitionExpectedDistInside : LedgeWalkTransitionExpectedDistOutside);
		FVector expectedAnimStart = closestMarker->Location - expectedDistToLedge*currentLedgeDir + LedgeWalkDistFromEdge*currentToWall;

		StartSpecialMove(SMT_LedgeWalkTransition, expectedAnimStart, CharForward, APTT_TargetAtStart);
	}
	return TRUE;
}

UBOOL AOLHero::TryExitLocker()
{
	if (!CouldStartSpecialMove(SMT_ExitLocker))
	{
		return FALSE;
	}

	check(ActiveLocker && ActiveLocker->AssociatedDoor);

	if (ActiveLocker->AssociatedDoor->IsClosing())
	{
		return FALSE;
	}

	if (!TryCommitToSpecialMove(SMT_ExitLocker, ActiveLocker))
	{
		return FALSE;
	}

	StartSpecialMove(SMT_ExitLocker);

	return TRUE;
}


UBOOL AOLHero::TryEnterBed(UBOOL playerInteraction)
{	
	if (!CouldStartSpecialMove(SMT_EnterBed))
	{
		return FALSE;
	}

	AOLBed* bed = NULL;
	FLOAT closestBedDistSq = -1.0f;

	for (INT i = 0; i < CachedBeds.Num(); i++)
	{
		AOLBed* testBed = CachedBeds(i);

		if (testBed && testBed->IsValid())
		{
			FLOAT distSq = testBed->Location.DistanceSquared(Location);

			if (!bed || distSq <= closestBedDistSq)
			{
				closestBedDistSq = distSq;
				bed = testBed;
			}
		}
	}

	if (bed && closestBedDistSq < Square(500.0f)) // the 500 distance isn't really used, it's just a safe early out
	{
		TWEAKABLE FLOAT MinCosAngleToBedView = 0.707f;
		TWEAKABLE FLOAT MinCosAngleToBedPos = 0.86f;
		TWEAKABLE FLOAT BedDistFwd = 35.0f;
		TWEAKABLE FLOAT BedDistBwd = -185.0f;
		TWEAKABLE FLOAT BedHalfWidth = 50.0f;

		FVector fromBed = Location - bed->Location;
		FVector bedFwd = bed->Rotation.Vector();
		FVector bedRight = bed->Rotation.Right();
		FVector bedCenter = bed->Location + 0.5f*(BedDistFwd + BedDistBwd)*bedFwd;
		FVector fromCenter = Location - bedCenter;
		FLOAT distBedRight = (fromCenter | bedRight);
		FLOAT distBedFwd = (fromBed | bedFwd);

		UBOOL closeToBedSide = Abs(distBedRight) < BedInteractDistance;
		UBOOL nextToBed = (distBedFwd < BedDistFwd) && (distBedFwd > BedDistBwd);
		UBOOL lookingAtBed = (-fromCenter.SafeNormal2D() | EyeForward.SafeNormal2D()) > MinCosAngleToBedView;

		if (!nextToBed) // We allow either directly by the side of the bed (within the corners), or within some angle off the corners
		{
			// calculate the closest corner pos
			FLOAT cornerFwdDist = (distBedFwd > 0.0f) ? BedDistFwd : BedDistBwd;
			FLOAT cornerRightDist = (distBedRight > 0.0f) ? BedHalfWidth : -BedHalfWidth;
			FVector closestCorner = bed->Location + cornerFwdDist*bedFwd + cornerRightDist*bedRight;
			FVector fromCorner = Location - closestCorner;
			nextToBed = Abs(fromCorner.SafeNormal2D() | bedRight) > MinCosAngleToBedPos;						
		}

		if (closeToBedSide && nextToBed && lookingAtBed)
		{
			// check that we have unobstructed entry (to the bed center, not the ending position)
			FLOAT colHeight = GetCollisionHeightForLocomotionMode(LM_Bed);
			FLOAT colRadius = 0.71f*GetCollisionRadiusForLocomotionMode(LM_Bed); // 0.71 = factor allowing oriented shapes, as the AABB test otherwise could fail for valid cases
			FCheckResult Hit(1.f);
			FVector startTrace = Location + VecZ(colHeight + Fudge);
			FVector endTrace = bedCenter + VecZ(colHeight + Fudge);

			if (GWorld->SingleLineCheck( Hit, this, endTrace, startTrace, TRACE_AllBlocking | TRACE_StopAtAnyHit, FVector(colRadius, colRadius, colHeight)))
			{
				if (!playerInteraction)
				{
					OLPC->AddAvailableInteraction(PIT_EnterBed);
				}
				else
				{
					if (!TryCommitToSpecialMove(SMT_EnterBed, bed))
					{
						return FALSE;
					}

					ActiveBed = bed;
					EnterBedZ = Location.Z;

					FVector expectedAnimStart = bed->Location - BedEnterExpectedFwdDist*bedFwd + Sgn(distBedRight)*BedEnterExpectedSideDist*bedRight;
					FVector expectedAnimFwd = Sgn(-distBedRight)*bedRight;

					bLeftAnim = distBedRight < 0.0f;

					StartSpecialMove(SMT_EnterBed, expectedAnimStart, expectedAnimFwd, APTT_TargetAtStart);

					return TRUE;
				}
			}
		}
	}

	ActiveBed = NULL;

	return FALSE;
}

UBOOL AOLHero::TryExitBed(FVector playerIntentDirection)
{
	if (!CouldStartSpecialMove(SMT_ExitBed))
	{
		return FALSE;
	}

	check(ActiveBed);

	FVector charRight = Rotation.Right();
	FVector eye2D = EyeForward.SafeNormal2D();

	FLOAT inputDotFwd = (playerIntentDirection | CharForward);
	FLOAT inputDotRight = (playerIntentDirection | charRight);
	FLOAT camDotFwd = (eye2D | CharForward);
	FLOAT camDotRight = (eye2D | charRight);
	FLOAT inputDotCam = (eye2D | playerIntentDirection);

	UBOOL bExitLeft = TRUE;
	
	if (inputDotFwd > 0.707f)
	{
		// input towards bed's head
		return FALSE;
	}

	if (camDotFwd > 0.707f)
	{
		// looking forward

		if (inputDotFwd < -0.707f)
		{
			// ambiguous back input
			return FALSE;
		}

		bExitLeft = (inputDotRight < 0.0f);
	}
	else if (camDotRight < -0.707f)
	{
		// looking left
		bExitLeft = (inputDotCam >= -0.707f); // if the input is along the camera, exit this direction
	}
	else 
	{
		// looking right
		bExitLeft = (inputDotCam < -0.707f); // exit left only if pushing backwards
	}
	
	// check that we have unobstructed exit
	FLOAT colHeight = GetCollisionHeightForLocomotionMode(LM_Bed);
	FLOAT colRadius = 0.71f*GetCollisionRadiusForLocomotionMode(LM_Bed);
	FCheckResult Hit(1.f);
	FVector delta = 100.0f * Rotation.Right() * (bExitLeft ? -1.0f : 1.0f);
	FVector startTrace = Location;		
	startTrace.Z = ActiveBed->Location.Z + colHeight + Fudge;
	FVector endTrace = Location + delta;
	endTrace.Z = startTrace.Z;

	if (GWorld->SingleLineCheck( Hit, this, endTrace, startTrace, TRACE_AllBlocking | TRACE_StopAtAnyHit, FVector(colRadius, colRadius, colHeight)))
	{
		if (!TryCommitToSpecialMove(SMT_ExitBed, ActiveBed))
		{
			return FALSE;
		}

		bLeftAnim = bExitLeft;
		bMustCrouchAfterSpecialMove = OLPC->bDuck;
		SpecialMoveTargetYaw = UNR_TO_DEG * ((bLeftAnim ? -1.0f : 1.0f) * charRight).Rotation().Yaw;	

		FVector expectedAnimStart = Location;
		expectedAnimStart.Z = Clamp(EnterBedZ, Location.Z - 10.0f, Location.Z + 10.0f);
		StartSpecialMove(SMT_ExitBed, expectedAnimStart, CharForward, APTT_TargetAtStart);

		return TRUE;
	}

	return FALSE;
}

UBOOL AOLHero::TryGrabFromBed(AOLEnemyPawn* attacker)
{
	if (!CouldStartSpecialMove(SMT_HeroGrabbedFromBed))
	{
		return FALSE;
	}

	check(ActiveBed);

	SetEnemyType(attacker);

	FLOAT dotRight = ((attacker->Location - Location) | Rotation.Right());
	
	// Assuming no collision issues since attacker should be there.
	bLeftAnim = (dotRight < 0.0f);
	StartSpecialMove(SMT_HeroGrabbedFromBed);
	return TRUE;
}

UBOOL AOLHero::TryEnterLadder(const FVector& playerIntentDirection)
{
	if (!CouldStartSpecialMove(SMT_EnterLadderFromGround) && !CouldStartSpecialMove(SMT_EnterLadderFromAbove))
	{
		return FALSE;
	}

	for (INT i = 0; i < CachedLadders.Num(); i++)
	{
		AOLLadderMarker* ladder = CachedLadders(i);

		if (ladder && ladder->IsValid())
		{
			TWEAKABLE FLOAT MinCosAngleForPositionFromGround = 0.707f;
			TWEAKABLE FLOAT MinCosAngleForPositionFromAbove = -0.5f;
			TWEAKABLE FLOAT MinDelayForRegrab = 0.5f;

			AOLLadderMarker* bottomMarker = (ladder->Location.Z < ladder->OtherMarker->Location.Z) ? ladder : ladder->OtherMarker;
			FVector toLadder = ladder->Location - Location;
			FVector toLadderDir = toLadder.SafeNormal2D();
			FVector ladderFwd = bottomMarker->Rotation.Vector();
			UBOOL enterFromGround =  (bottomMarker == ladder);
			FLOAT interactDist = enterFromGround ? LadderEnterFromGroundInteractDist : LadderEnterFromAboveInteractDist;
			FVector entryDir = enterFromGround ? -ladderFwd : ladderFwd;

			UBOOL afterRegrabDelay = (GWorld->GetTimeSeconds() - LastActiveLadderTimestamp) >= MinDelayForRegrab;
			UBOOL withinInteractDist = ladder->Location.DistanceSquared(Location) < Square(interactDist);
			UBOOL inFrontOfLadder = (toLadderDir | entryDir) >= (enterFromGround ? MinCosAngleForPositionFromGround : MinCosAngleForPositionFromAbove);
			UBOOL targettedInput = enterFromGround ? ((toLadderDir | playerIntentDirection) >= 0.707f) : ((playerIntentDirection | entryDir) > 0.5f);
			UBOOL lookingAtLadder = (CharForward | entryDir) >= 0.707f;
			
			if (withinInteractDist && inFrontOfLadder && targettedInput && (lookingAtLadder || !enterFromGround) && afterRegrabDelay)
			{			
				if (enterFromGround)
				{	
					if (!TryCommitToSpecialMove(SMT_EnterLadderFromGround, bottomMarker))
					{
						return FALSE;
					}

					FLOAT angleToLadder = appAcos(CharForward | -ladderFwd) * RAD_TO_DEG;
					SpecialMoveBlendAlpha = 1.0f - Saturate(angleToLadder / 45.0f); // 0.0f - 45 degs, 1.0f - straight
					bLeftAnim = (Rotation.Right() | ladderFwd) > 0.0f;

					FVector expectedAnimEnd = bottomMarker->Location + LadderDistFwd*ladderFwd + VecZ(LadderRootHeightOffsetFromBar);

					if (expectedAnimEnd.Z < (Location.Z + LadderEnterFromGroundDeltaZ))
					{
						expectedAnimEnd.Z += LadderBarSpacing; // adjust up to the next bar, not down
					}

					StartSpecialMove(SMT_EnterLadderFromGround, expectedAnimEnd, -ladderFwd, APTT_TargetAtEnd);
					ActiveLadder = bottomMarker;
				}
				else
				{
					if (!TryCommitToSpecialMove(SMT_EnterLadderFromAbove, bottomMarker->OtherMarker))
					{
						return FALSE;
					}

					FVector expectedAnimStart = bottomMarker->OtherMarker->Location - LadderEnterFromAboveExpectedDist*ladderFwd;
					StartSpecialMove(SMT_EnterLadderFromAbove, expectedAnimStart, ladderFwd, APTT_TargetAtStart);
					ActiveLadder = bottomMarker;
				}

				return TRUE;
			}
		}
	}

	return FALSE;
}

UBOOL AOLHero::TryGrabLadder(const FVector& playerIntentVelocity)
{
	if (!CouldStartSpecialMove(SMT_GrabLadderFromAir))
	{
		return FALSE;
	}

	TWEAKABLE FLOAT MinDelayForRegrab = 0.5f;
	
	for (INT i = 0; i < CachedLadders.Num(); i++)
	{
		AOLLadderMarker* ladder = CachedLadders(i);

		if (ladder && ladder->IsValid() && (ladder->Location - Location).SizeSquared2D() < Square(LadderGrabDist))
		{
			AOLLadderMarker* bottomMarker = (ladder->Location.Z < ladder->OtherMarker->Location.Z) ? ladder : ladder->OtherMarker;

			if (Location.Z < (bottomMarker->Location.Z + LadderExitOnGroundInteractDist) || Location.Z > (bottomMarker->OtherMarker->Location.Z - LadderExitOnTopInteractDist))
			{
				continue;
			}

			FVector toLadder = ladder->Location - Location;
			FVector ladderFwd = bottomMarker->Rotation.Vector();

			TWEAKABLE FLOAT MinCosAngleToLadder = 0.707f;

			FVector toLadderDir = toLadder.SafeNormal2D();

			UBOOL inFrontOfLadder = (toLadderDir | -ladderFwd) > MinCosAngleToLadder;
			UBOOL targettedInput = (toLadderDir | playerIntentVelocity.SafeNormal2D()) > MinCosAngleToLadder;
			UBOOL movingTowardsLadder = (toLadderDir | RealVelocity.SafeNormal2D()) > MinCosAngleToLadder;
			UBOOL afterRegrabDelay = (GWorld->GetTimeSeconds() - LastActiveLadderTimestamp) >= MinDelayForRegrab;
			UBOOL lookingAtLadder = (toLadderDir | CharForward) > MinCosAngleToLadder;

			if (inFrontOfLadder && lookingAtLadder && (targettedInput || movingTowardsLadder) && afterRegrabDelay)
			{
				if (!TryCommitToSpecialMove(SMT_GrabLadderFromAir, bottomMarker))
				{
					return FALSE;
				}

				FVector expectedAnimEnd = FVector(bottomMarker->Location.X, bottomMarker->Location.Y, Location.Z) + LadderDistFwd*ladderFwd;
				StartSpecialMove(SMT_GrabLadderFromAir, expectedAnimEnd, -ladderFwd, APTT_TargetAtEnd);
				ActiveLadder = bottomMarker;
				return TRUE;
			}
		}
	}

	return FALSE;
}

UBOOL AOLHero::TryExitLadder()
{
	if (!CouldStartSpecialMove(SMT_ExitLadderOnGround) && !CouldStartSpecialMove(SMT_ExitLadderOnTop))
	{
		return FALSE;
	}

	if (!ActiveLadder)
	{
		// failsafe
		StartSpecialMove(SMT_DropFromLadder);
		return TRUE;
	}

	check(LadderAnimNode);
	bExitLadderLeftHand = LadderAnimNode->CloserToA();

	if (Location.Z <= (ActiveLadder->Location.Z + LadderExitOnGroundInteractDist) && RealVelocity.Z < 0.0f)
	{
		if (!TryCommitToSpecialMove(SMT_ExitLadderOnGround, ActiveLadder))
		{
			return FALSE;
		}

		// leave at bottom
		StartSpecialMove(SMT_ExitLadderOnGround);

		FProceduralAnimData animData;
		animData.PositionDelta = 50.0f*ActiveLadder->Rotation.Vector();				
		QueueProceduralAnim(animData);

		return TRUE;
	}
	else if (Location.Z >= (ActiveLadder->OtherMarker->Location.Z - LadderExitOnTopInteractDist) && RealVelocity.Z > 0.0f)
	{
		if (!TryCommitToSpecialMove(SMT_ExitLadderOnTop, ActiveLadder->OtherMarker))
		{
			return FALSE;
		}

		StartSpecialMove(SMT_ExitLadderOnTop);
		return TRUE;
	}

	return FALSE;
}

UBOOL AOLHero::TryDropFromLadder()
{
	if (!CouldStartSpecialMove(SMT_DropFromLadder))
	{
		return FALSE;
	}

	if (Location.Z > (ActiveLadder->Location.Z + LadderMaxHeightForDrop))
	{
		// Too high to allow dropping (you'd hurt yourself)
		return FALSE;
	}

	if (!TryCommitToSpecialMove(SMT_DropFromLadder, ActiveLadder))
	{
		return FALSE;
	}

	EnterLocomotionMode(LM_Fall);
	StartSpecialMove(SMT_DropFromLadder);

	LastActiveLadderTimestamp = GWorld->GetTimeSeconds();
	LastActiveLedgeTimestamp = GWorld->GetTimeSeconds(); // also set the ledge threshold - we don't want to grab the upper floor ledge when dropping from a ladder
	LastActiveLedgeZ = Location.Z;

	Velocity = -100.0f*CharForward;

	return TRUE;
}

UBOOL AOLHero::TryEnterContextualLean(UBOOL wantsToLeanLeft, UBOOL wantsToLeanRight)
{
	if (!CouldStartSpecialMove(SMT_EnterContextualLean))
	{
		return FALSE;
	}

	if (IsRunning() || bWantToRun)
	{
		// running
		return FALSE;
	}

	if (bIsCrouched)
	{
		// TODO
		return FALSE;
	}
	
	AOLCornerMarker* cornerMarker = CornerPeek.CornerMarker;

	if (cornerMarker)
	{
		UBOOL bPeekingFromLeft = (CornerPeek.PeekPosition == CPP_Left || CornerPeek.PeekPosition == CPP_MiddleLeft);

		if ((bPeekingFromLeft && !wantsToLeanRight) || (!bPeekingFromLeft && !wantsToLeanLeft))
		{
			// Input doesn't match
			return FALSE;
		}

		FVector toCorner = CornerPeek.CornerLocation - Location;
		FLOAT distFwd = toCorner | CornerPeek.FwdDir;
		FLOAT distSide = -(toCorner | CornerPeek.SideDir);

		// Are we in range to actually get in peeking position
		if (distSide < PeekingEnterInteractDistFromEdgeMin || distSide > PeekingEnterInteractDistFromEdgeMax)
		{
			// not in range along the side axis
			return FALSE;
		}

		if (distFwd < 20.0f || distFwd > PeekingEnterInteractDistFromWall)
		{
			// not in range along the forward axis
			return FALSE;
		}

		if (!TryCommitToSpecialMove(SMT_EnterContextualLean))
		{
			return FALSE;
		}

		FLOAT distFromEdge = bPeekingFromLeft ? PeekingEnterExpectedDistFromEdgeLeft : PeekingEnterExpectedDistFromEdgeRight;
		FVector expectedAnimStart = CornerPeek.CornerLocation - PeekingEnterExpectedDistFromWall*CornerPeek.FwdDir + distFromEdge*CornerPeek.SideDir;
		FVector expectedAnimFwd = CornerPeek.FwdDir;
		expectedAnimStart.Z = Location.Z;

		check(PeekingAnimNode);
		PeekingAnimNode->SetPeekingType(bPeekingFromLeft, CornerPeek.bRoundedCorner);
		ShadowProxyPeekingAnimNode->SetPeekingType(bPeekingFromLeft, CornerPeek.bRoundedCorner);

		FLOAT initialRatio = 0.0f;

		if (!OLPC->IsUsingGamepad())
		{
			FLOAT eyeDistSide = Vec2D(EyeLocation - CornerPeek.CornerLocation) | CornerPeek.SideDir;
			initialRatio = -0.0004f * (eyeDistSide * eyeDistSide) - 0.0201f * eyeDistSide + 0.857f; // simple regression on current anim
		}

		PeekingAnimNode->StartPeeking(initialRatio);
		ShadowProxyPeekingAnimNode->StartPeeking(initialRatio);
					
		StartSpecialMove(SMT_EnterContextualLean, expectedAnimStart, expectedAnimFwd, APTT_TargetAtStart);

		return TRUE;
	}

	return FALSE;
}

UBOOL AOLHero::TryPushFromLedge(const FVector& playerIntentVelocity)
{
	if (!CouldStartSpecialMove(SMT_PushFromLedgeProcedural) && !CouldStartSpecialMove(SMT_PushFromLedgeAnimated))
	{
		return FALSE;
	}

	if (playerIntentVelocity.IsNearlyZero(KINDA_SMALL_NUMBERF))
	{
		return FALSE;
	}

	for (INT markerIdx = 0; markerIdx < CachedMarkers.Num(); markerIdx++)
	{
		AOLLedgeMarker* node1 = CachedMarkers(markerIdx);

		// check validity
		if (node1 && node1->IsValid() && node1->Next && node1->Next->IsValid())
		{
			AOLLedgeMarker* node2 = node1->Next;
			const FVector& node1Loc = node1->Location;
			const FVector& node2Loc = node2->Location;

			FVector ledge = node2Loc - node1Loc;

			if (!Utils::IsBetweenMarkers(Location, node1, node2, FALSE, 0.0f))
			{
				// Outside of the markers
				continue;
			}

			FVector closestPoint;
			FVector dummy;
			FLOAT charRadius = DefaultHero->CylinderComponent->CollisionRadius;
			SegmentDistToSegment(node1Loc, node2Loc, Location, Location + charRadius*CharForward, closestPoint, dummy);
			FVector toClosestPoint = (closestPoint - Location);
			FVector toClosestPoint2D(toClosestPoint.X, toClosestPoint.Y, 0.0f);
			FVector directionToLedge;
			FLOAT distToLedge;
			toClosestPoint2D.ToDirectionAndLength(directionToLedge, distToLedge);

			if (distToLedge > charRadius)
			{
				// too far
				continue;
			}
			
			if (Abs(toClosestPoint.Z) > 10.0f)
			{
				// not same level
				continue;
			}
			
			FVector ledgePerp = (ledge.SafeNormal2D() ^ VecZ(1.0f));
			FVector towardsDrop = ledgePerp;

			// Check whether we picked the right drop side
			{
				FCheckResult Hit(1.f);
				FVector startTrace = closestPoint + 10.0f * towardsDrop + VecZ(25.0f);
				FVector endTrace = startTrace - VecZ(50.0f); // 25 cm down
				if (!GWorld->SingleLineCheck( Hit, this, endTrace, startTrace, TRACE_AllBlocking | TRACE_StopAtAnyHit, FVector(0.0f)))
				{
					towardsDrop = -towardsDrop;
				}
			}

			UBOOL bOnDropSide = (toClosestPoint | towardsDrop) < 0.0f;
			UBOOL bGoingTowardsDrop = (playerIntentVelocity | towardsDrop) > 0.0f;

			// check whether this is an animated exit airvent
			if (CouldStartSpecialMove(SMT_PushFromLedgeAnimated) && bIsCrouched && !bOnDropSide && bGoingTowardsDrop)
			{
				TWEAKABLE FLOAT MinAngleForAnimatedPush = 0.707f;
				if (distToLedge <= PushFromLedgeAnimatedInteractDist && (CharForward | towardsDrop) >= MinAngleForAnimatedPush)
				{
					// check that there's a sufficient drop for the move
					FCheckResult Hit(1.f);
					FVector slice(DefaultHero->CylinderComponent->CollisionRadius, DefaultHero->CylinderComponent->CollisionRadius, 1.0f);
					FVector startTrace = closestPoint + (DefaultHero->CylinderComponent->CollisionRadius + 10.0f) * towardsDrop + VecZ(25.0f); // 25cm above the drop
					FVector endTrace = startTrace - VecZ(25.0f + PushFromLedgeAnimatedMinHeight); // dropped down PushFromLedgeAnimatedMinHeight
					
					if (GWorld->SingleLineCheck( Hit, this, endTrace, startTrace, TRACE_AllBlocking | TRACE_StopAtAnyHit, slice))
					{
						if (!TryCommitToSpecialMove(SMT_PushFromLedgeAnimated))
						{
							return FALSE;
						}

						// clear - let's do it

						FVector expectedAnimStart = closestPoint - PushFromLedgeAnimatedExpectedDist*towardsDrop;
						FVector expectedAnimFwd = towardsDrop;
						expectedAnimStart.Z = Location.Z;
						StartSpecialMove(SMT_PushFromLedgeAnimated, expectedAnimStart, expectedAnimFwd, APTT_TargetAtStart);

						return TRUE;
					}
				}				
			}

			// otherwise try a procedural push
			if (CouldStartSpecialMove(SMT_PushFromLedgeProcedural))
			{
				TWEAKABLE FLOAT MaxVelocityForPush = 50.0f;
				if (Abs(Velocity | ledgePerp) > MaxVelocityForPush)
				{
					// player either wants to drop / jump or is going fast enough to save himself
					continue;
				}
								
				if (!bOnDropSide && distToLedge > PushFromLedgeProceduralInteractDist)
				{
					continue; // too far
				}

				// We want to kick the player out of a ledge walk he's not ledging on - probe to see if there's a wall
				FCheckResult Hit(1.f);
				FVector startTrace = closestPoint + VecZ(50.0f);
				FVector endTrace = startTrace - 40.0f * towardsDrop; // 40 cm forward, towards a possible wall
				if (GWorld->SingleLineCheck( Hit, this, endTrace, startTrace, TRACE_AllBlocking | TRACE_StopAtAnyHit, FVector(0.0f)))
				{
					// doesn't seem like a ledge walk
										
					if (!bOnDropSide || !bGoingTowardsDrop)
					{
						// on the good side, or going towards inside
						continue;
					}
				}

				if (!TryCommitToSpecialMove(SMT_PushFromLedgeProcedural))
				{
					return FALSE;
				}
			
				// let's kick it
			
				FProceduralAnimData animData;
				FVector desiredLoc = Location + 1.5f*DefaultHero->CylinderComponent->CollisionRadius*towardsDrop;
				animData.PositionDelta = desiredLoc - Location;
				TWEAKABLE FLOAT linearVel = 160.0f;
				FLOAT duration = animData.PositionDelta.Size2D() / linearVel;
				animData.PositionDelta += Vec2D(Velocity) * duration; // add the current velocity back in, to feel smoother if going fastish
				FLOAT effectiveVel = animData.PositionDelta.Size2D() / duration; // and recompute the velocity accordingly

				QueueProceduralAnim(animData, effectiveVel, 360.0f);

				StartSpecialMove(SMT_PushFromLedgeProcedural);
				return TRUE;
			}
		}
	}

	return FALSE;
}

UBOOL AOLHero::AdjustJumpTargetToClearObstacle(FVector& out_JumpTarget, AOLLedgeMarker* node1, const FVector& startPoint, const FVector& endPoint)
{
	TWEAKABLE FLOAT Max2DDeviation = 50.0f; // on the side or after endPoint
	
	AOLLedgeMarker* node2 = node1->Next;
	const FVector& node1Loc = node1->Location;
	const FVector& node2Loc = node2->Location;
	FVector ledge = node2Loc - node1Loc;

	FVector closestPoint; // on ledge
	FVector closestTrajPoint; // on trajectory
	// find the closest points in 2d, then add back the heights
	SegmentDistToSegment(FVector(node1Loc.X, node1Loc.Y, 0), FVector(node2Loc.X, node2Loc.Y, 0), FVector(startPoint.X, startPoint.Y, 0), FVector(endPoint.X, endPoint.Y, 0), closestPoint, closestTrajPoint);
	FLOAT closestPointRatio = (node2Loc.X - node1Loc.X) != 0.0f ? (closestPoint.X - node1Loc.X) / (node2Loc.X - node1Loc.X) : (closestPoint.Y - node1Loc.Y) / (node2Loc.Y - node1Loc.Y);
	closestPoint.Z = node1Loc.Z + closestPointRatio * (node2Loc.Z - node1Loc.Z);
	FLOAT closestTrajPointRatio = (endPoint.X - startPoint.X) != 0.0f ? (closestTrajPoint.X - startPoint.X) / (endPoint.X - startPoint.X) : (closestTrajPoint.Y - startPoint.Y) / (endPoint.Y - startPoint.Y);
	closestTrajPoint.Z = startPoint.Z + closestTrajPointRatio * (endPoint.Z - startPoint.Z);

	FLOAT interceptHeight = closestPoint.Z - closestTrajPoint.Z;

	if ((closestTrajPoint - closestPoint).SizeSquared2D() > Square(Max2DDeviation))
	{
		return FALSE; // not close enough
	}
	else if (interceptHeight < (GrabLedgeMinHeightInAir - MaxGrabLedgeAdjustment))
	{
		return FALSE; // ledge is too low
	}	
	else if (interceptHeight > (GrabLedgeMaxHeightInAir + MaxGrabLedgeAdjustment))
	{
		return FALSE; // ledge is too high
	}	
	else if ( ((closestPoint - startPoint) | (endPoint - startPoint)) < 0.0f)
	{
		return FALSE; // behind the start point
	}
	else if (!Utils::IsBetweenMarkers(closestTrajPoint, node1, node2, TRUE, -CylinderComponent->CollisionRadius))
	{
		// Too close to one of the end markers
		return FALSE;
	}
	else if (Abs(CharForward | ledge.SafeNormal2D()) > MaxLedgeCosAngle)
	{
		// Interception angle is too large
		return FALSE;
	}
		
	UBOOL validTarget = FALSE;
	FVector targetPoint(0.0f);

	// Adjust up or down to grab the ledge
	targetPoint = closestPoint - FVector(0, 0, LedgeHangHeightToLedge) - LedgeHangDistToWall*CharForward;
	FLOAT colRadius = GetCollisionRadiusForLocomotionMode(LM_LedgeHang);
	FLOAT colHeight = GetCollisionHeightForLocomotionMode(LM_LedgeHang);
		
	if (!WouldEncroach(targetPoint, colRadius, colHeight))
	{
		validTarget = TRUE;
		debugf(TEXT("Jump adjustment for GRAB LEDGE at height %.0fcm and distance %.0fcm"), interceptHeight, (closestPoint - Location).Size2D());
	}

	if (validTarget)
	{
		// Reject the move if we don't have LoS to the target
		FCheckResult Hit(1.f);
		FVector startTrace = EyeLocation;
		FVector endTrace = targetPoint;

		if (!GWorld->SingleLineCheck( Hit, this, endTrace, startTrace, TRACE_AllBlocking | TRACE_StopAtAnyHit, FVector(0.0f)))
		{
			validTarget = FALSE;
		}
	}

	if (validTarget)
	{
		out_JumpTarget = targetPoint;
		return TRUE;
	}

	return FALSE;
}

UBOOL AOLHero::TryJump(const FVector& playerIntentVelocity)
{
	if (LocomotionMode != LM_Walk && LocomotionMode != LM_LookBack)
	{
		return FALSE;
	}

	if (bLimping || bHobbling)
	{
		return FALSE;
	}

	if (bElectrified)
	{
		FLOAT timeSinceLastLanding = GWorld->GetTimeSeconds() - LastLandingTimestamp;
		if (timeSinceLastLanding < ElectrifiedJumpDelay)
		{
			// prevent jumping arond like a fool when electrified
			return FALSE;
		}
	}

	// Check if disabled by physics volume
	for (INT i = 0; i < Touching.Num(); i++)
	{	
		APhysicsVolume* volume = Cast<APhysicsVolume>(Touching(i));
		if (volume && volume->bEnabled && volume->bDisableJump)
		{
			return FALSE;
		}
	}

	if (bJumpCapable && !bIsCrouched && !IsPeeking() && !bWantsToCrouch && (Physics == PHYS_Walking) && (LocomotionMode == LM_Walk))
	{
		if (playerIntentVelocity.IsNearlyZero() || Max(playerIntentVelocity.SizeSquared2D(), RealVelocity.SizeSquared2D()) < Square(ForwardJumpSpeedThreshold))
		{
			if (!TryCommitToSpecialMove(SMT_JumpOnSpot))
			{
				return FALSE;
			}

			bJumping = TRUE;

			StartSpecialMove(SMT_JumpOnSpot);

			if (HiddenRightArmControl && HiddenLeftArmControl)
			{
				FCheckResult Hit(1.f);
				TWEAKABLE FLOAT TraceSideDist = 50.0f;
				TWEAKABLE FLOAT TraceFwdDist = 40.0f; 
				FVector startTrace = Location + VecZ(120.0f);			
				FVector endTrace = startTrace - TraceSideDist * Rotation.Right() + TraceFwdDist * CharForward;
						
				if (!GWorld->SingleLineCheck( Hit, this, endTrace, startTrace, TRACE_AllBlocking | TRACE_StopAtAnyHit, FVector(0.0f)))
				{
					bShouldHideLeftHandDuringSM = TRUE;
				}

				endTrace = startTrace + TraceSideDist * Rotation.Right() + TraceFwdDist * CharForward;

				if (!GWorld->SingleLineCheck( Hit, this, endTrace, startTrace, TRACE_AllBlocking | TRACE_StopAtAnyHit, FVector(0.0f)))
				{
					bShouldHideRightHandDuringSM = TRUE;
				}
			}
		}
		else
		{
			FLOAT xySpeed = IsRunning() ? ForwardSpeedForJumpRunning : ForwardSpeedForJumpWalking;
			FLOAT desiredDist = IsRunning() ? JumpClearanceRunning : JumpClearanceWalking;
			FLOAT reqTime = desiredDist / xySpeed;
			FLOAT zSpeed = -GetGravityZ() * reqTime; // what's this double gravity in phys_falling bullshit?

			FVector jumpDirection = playerIntentVelocity.SafeNormal2D();

			FVector jumpVelocity = xySpeed * jumpDirection + FVector(0, 0, zSpeed);

			// Check to see whether we'll encounter a ledge - adjust velocity to clear landing or grab ledge if within allowed range
			if ((jumpDirection | CharForward) > 0.707f)
			{
				FVector straightDest = Location + desiredDist*CharForward; // destination if landing at the same height
				FLOAT halfTime = 0.5f*reqTime;
				FLOAT apexHeight = -GetGravityZ()*halfTime*halfTime;
				FVector apex = Location + 0.5f*desiredDist * CharForward + FVector(0, 0, apexHeight); // high point in the trajectory (at half the straight jump time)
				FVector downPt = Location + 1.5f*desiredDist * CharForward - FVector(0, 0, apexHeight); // low point after the straight jump destination (1.5 the straight jump time)
				FVector closePt = Location + 50.0f * CharForward + FVector(0, 0, apexHeight); // just in front and at the apex height

				for (INT markerIdx = 0; markerIdx < CachedMarkers.Num(); markerIdx++)
				{
					AOLLedgeMarker* ledgeMarker = CachedMarkers(markerIdx);

					if (ledgeMarker && ledgeMarker->IsValid() && ledgeMarker->Next && ledgeMarker->Next->IsValid())
					{	
						FVector jumpTarget(0.0f);
						UBOOL obstacleFound = AdjustJumpTargetToClearObstacle(jumpTarget, ledgeMarker, closePt, apex); // from apex to straight point
						obstacleFound = obstacleFound || AdjustJumpTargetToClearObstacle(jumpTarget, ledgeMarker, apex, straightDest); // from apex to straight point
						obstacleFound = obstacleFound || AdjustJumpTargetToClearObstacle(jumpTarget, ledgeMarker, straightDest, downPt); // from straight down to the low point

						if (obstacleFound)
						{
							FVector toTarget = jumpTarget - Location;
							FLOAT newDesiredDist = toTarget.Size2D();
							FLOAT newReqTime = newDesiredDist / xySpeed;
							FLOAT newZSpeed = toTarget.Z/newReqTime - GetGravityZ() * newReqTime;

							if (newZSpeed > zSpeed && newZSpeed < 1.25f * zSpeed) // don't slow down, and limit to a 25% adjustment room
							{
								jumpVelocity = xySpeed * toTarget.SafeNormal2D() + FVector(0, 0, newZSpeed);
								debugf(TEXT("Jump adjust changed z speed from %.0f to %.0f"), zSpeed, newZSpeed);
							}
							break;
						}
					}
				}
			}

			if (Base && !Base->bWorldGeometry && Base->Velocity.Z > 0.0f)
			{
				jumpVelocity.Z += Base->Velocity.Z;
			}

			// Adjust for angled floors
			if (Floor.Z < 0.99f)
			{
				FVector jumpDirection = jumpVelocity.SafeNormal();
				FLOAT floorFwdComp = -(Floor | jumpDirection.SafeNormal2D());

				if (floorFwdComp > 0.0f)
				{
					FLOAT floorAngle = appAsin(floorFwdComp);
					FLOAT jumpAngle = appAsin(jumpDirection.Z);

					FLOAT totalAngle = Min(floorAngle + jumpAngle, 1.48f); // 85 degs cap
					jumpDirection.Z = appSin(totalAngle);
					FLOAT targetLength2d = appCos(totalAngle);
					FLOAT currentLength2d = jumpDirection.Size2D();
					jumpDirection.X *= targetLength2d / currentLength2d;
					jumpDirection.Y *= targetLength2d / currentLength2d;

					jumpVelocity = jumpVelocity.Size() * jumpDirection;
				}
			}

			Velocity = jumpVelocity;			

			EnterLocomotionMode(LM_Fall);

			bApplyLandingPenalty = TRUE;
			bJumping = TRUE;

			TWEAKABLE FLOAT blendInTime = 0.15f;
			TWEAKABLE FLOAT blendOutTime = 0.25f;

			PlayFullBodyAnim(IsRunning() ? AnimNameJumpFromRun : AnimNameJumpFromWalk, 1.f, blendInTime, blendOutTime);			
		}

		return TRUE;
	}
	return FALSE;
}

UBOOL AOLHero::CanCrouch() const
{
	if (bLimping)
	{
		return FALSE;
	}

	for (INT i = 0; i < Touching.Num(); i++)
	{	
		APhysicsVolume* volume = Cast<APhysicsVolume>(Touching(i));
		if (volume && volume->bEnabled && volume->bDisableCrouch)
		{
			return FALSE;
		}
	}

	return LocomotionMode == LM_Walk && Physics == PHYS_Walking;
}

UBOOL AOLHero::TryCrouch()
{
	if ((!bWantsToCrouch || bForcedCrouch) && LocomotionMode == LM_Walk && Physics == PHYS_Walking && !bIsCrouched && CouldStartSpecialMove(SMT_Crouch))
	{
		bWantsToCrouch = TRUE;
		return TRUE;
	}

	return FALSE;
}


UBOOL AOLHero::CanUncrouch()
{
	for (INT i = 0; i < Touching.Num(); i++)
	{	
		APhysicsVolume* volume = Cast<APhysicsVolume>(Touching(i));
		if (volume && volume->bEnabled && volume->bDisableUncrouch)
		{
			return FALSE;
		}
	}

	return !bIsCrouched || !WouldEncroach(Location);
}

UBOOL AOLHero::TryUncrouch()
{
	if (!CanUncrouch())
	{
		return FALSE;
	}

	if (bWantsToCrouch && LocomotionMode == LM_Walk && Physics == PHYS_Walking && bIsCrouched && !bMustCrouchAfterSpecialMove && CouldStartSpecialMove(SMT_Uncrouch))
	{
		bWantsToCrouch = FALSE;
		return TRUE;
	}

	return FALSE;
}

void AOLHero::OnCrouch()
{
	UBOOL bConsiderCrouched = ((GWorld->GetTimeSeconds() - LastLandingTimestamp) < 0.15f && BlendByPostureFallingAnimNode && BlendByPostureFallingAnimNode->bRelevant && BlendByPostureFallingAnimNode->ActiveChildIndex == 1);
	UBOOL bPlayAnim = !bForcedCrouch && !bConsiderCrouched;

	if (bPlayAnim) // when forced, do not play animations
	{
		StartSpecialMove(SMT_Crouch);
	}
	bForcedCrouch = FALSE;
	bWantToRun = FALSE;
}

void AOLHero::OnUncrouch() 
{
	if (Physics == PHYS_Walking)
	{
		StartSpecialMove(SMT_Uncrouch);
	}
	bForcedCrouch = FALSE;
}

void AOLHero::Crouch(INT bClientSimulation)
{
	if (bWantsToCrouch)
	{
		Super::Crouch();
	}
}

void AOLHero::UnCrouch(INT bClientSimulation)
{
	if (!bWantsToCrouch)
	{
		Super::UnCrouch();
	}
}

UBOOL	AOLHero::TryDropFromLedge()
{
	if (!CouldStartSpecialMove(SMT_DropFromLedge))
	{
		// early out if possible
		return FALSE;
	}

	if (ActiveLedge && !ActiveLedge->bCanDropDown)
	{
		// disallowed
		return FALSE;
	}

	if (!TryCommitToSpecialMove(SMT_DropFromLedge))
	{
		return FALSE;
	}

	StartSpecialMove(SMT_DropFromLedge);
	LastActiveLedgeTimestamp = GWorld->GetTimeSeconds();
	LastActiveLedgeZ = Location.Z;

	Velocity = -100.0f*CharForward;

	return TRUE;
}

UBOOL AOLHero::TryJumpFromLedgeWalk(UBOOL withForwardVelocity)
{
	TWEAKABLE FLOAT MinPlayerInputIntentForSpecialMove = 100.0f;

	if (!CouldStartSpecialMove(SMT_JumpFromLedgeWalk))
	{
		// early out if possible
		return FALSE;
	}

	if (ActiveLedge && !ActiveLedge->bCanDropDown)
	{
		// disallowed
		return FALSE;
	}

	if (!TryCommitToSpecialMove(SMT_JumpFromLedgeWalk))
	{
		return FALSE;
	}

	bJumpFromLedgeWalkWithVelocity = withForwardVelocity;

	StartSpecialMove(SMT_JumpFromLedgeWalk);
		
	if (withForwardVelocity)
	{
		Velocity = JumpForwardFromLedgeWalkXYSpeed*CharForward;
		Velocity.Z = JumpForwardFromLedgeWalkZSpeed;
	}
	else
	{
		Velocity = DropFromLedgeWalkXYSpeed*CharForward;
		Velocity.Z = DropFromLedgeWalkZSpeed;
	}

	return TRUE;
}

UBOOL AOLHero::CanRun() const
{
	if (bLimping)
	{
		return FALSE;
	}

	for (INT i = 0; i < Touching.Num(); i++)
	{	
		APhysicsVolume* volume = Cast<APhysicsVolume>(Touching(i));
		if (volume && volume->bEnabled && volume->bDisableRun)
		{
			return FALSE;
		}
	}

	return TRUE;
}

UBOOL AOLHero::TryRun()
{
	if (LocomotionMode == LM_Walk && !bIsCrouched && (SpecialMove == SMT_None || SpecialMove == SMT_Uncrouch))
	{
		if (!IsRunning() && !bWantToRun)
		{
			CurrentRunSpeed = WalkSpeed;
		}
		bWantToRun = TRUE;
		return TRUE;
	}

	return FALSE;
}

void AOLHero::Walk()
{
	bWantToRun = FALSE;
}

UBOOL AOLHero::IsRunning()
{
	return (RunSpeed > WalkSpeed + 40.0f) && (RealVelocity.SizeSquared2D() > Square(0.5f*(WalkSpeed+RunSpeed)));
}

UBOOL AOLHero::IsBarefeet()
{
	return Utils::IsPlayingDLC() && (Utils::GetCurrentCheckpointName() == NAME_None || Utils::IsCheckpointReached(PrisonerUniformCheckpoint) || !Utils::IsCheckpointReached(ITUniformCheckpoint));
}

UBOOL AOLHero::IsInLocker()
{
	return LocomotionMode == LM_Locker;
}

UBOOL AOLHero::IsInDarkness()
{
	for (INT i = 0; i < Touching.Num(); i++)
	{
		AOLDarknessVolume* darkVolume = Cast<AOLDarknessVolume>(Touching(i));
		if (darkVolume && darkVolume->bDark && (!darkVolume->bOnlyDarkWhenCrouched || bIsCrouched))
		{
			return TRUE;
		}
	}

	return FALSE;
}

UBOOL AOLHero::IsInWaterVolume()
{
	for (INT i = 0; i < Touching.Num(); i++)
	{	
		APhysicsVolume* volume = Cast<APhysicsVolume>(Touching(i));
		if (volume && volume->bEnabled && volume->bWaistDeepWater)
		{
			return TRUE;
		}
	}

	return FALSE;
}

UBOOL AOLHero::IsInHeatVolume()
{
	for (INT i = 0; i < Touching.Num(); i++)
	{	
		AOLHeatVolume* volume = Cast<AOLHeatVolume>(Touching(i));
		if (volume && volume->bEnabled)
		{
			return TRUE;
		}
	}

	return FALSE;
}

UBOOL AOLHero::IsBeingChased()
{
	for (AController* C = GWorld->GetWorldInfo()->ControllerList; C != NULL; C = C->NextController)
	{
		AOLBot* bot = Cast<AOLBot>(C);

		if (bot && bot->EnemyPawn && (bot->EnemyPawn->Modifiers.bShouldAttack || bot->EnemyPawn->Modifiers.bAttackOnProximity) && bot->SightComponent->CanSeeTarget && bot->BehaviorState == AIBS_Chasing)
		{
			return TRUE;
		}
	}

	return FALSE;
}

UBOOL AOLHero::CanBeAttacked()
{
	return (Health > 0) && (LocomotionMode != LM_Cinematic) && (SpecialMove != SMT_Dying && SpecialMove != SMT_HeroDecapitate && SpecialMove != SMT_HeroKilled && SpecialMove != SMT_KilledInStruggle);
}

UBOOL AOLHero::CanBeFatalitized()
{
	return (Health > 0) && (!OLPC->bGodMode) && (LocomotionMode != LM_Cinematic && LocomotionMode != LM_Door &&
		SpecialMove != SMT_OpenDoorInstant && SpecialMove != SMT_OpenDoorPartial && SpecialMove != SMT_TryOpenLockedDoor && 
		SpecialMove != SMT_CloseDoor && SpecialMove != SMT_CloseDoorPositionned && SpecialMove != SMT_RunThroughDoor);
}

UBOOL AOLHero::CanBeGrabbedUnder()
{
	return bIsCrouched && !CanUncrouch() && IsInGrabbableState();
}

UBOOL AOLHero::TryLean(FLOAT leanInput)
{
	if (bLimping)
	{
		CurrentLean = 0.0f;
		return FALSE;
	}

	if (LocomotionMode == LM_Walk && !IsDoingSpecialMove() && RealVelocity.SizeSquared() < Square(LeanSpeedThreshold))
	{		
		UBOOL bRight = leanInput > 0.0f;

		FCheckResult Hit(1.f);
		FVector startTrace = Location;
		startTrace.Z = EyeLocation.Z;
		TWEAKABLE FLOAT TraceOffsetSide = 45.0f;
		TWEAKABLE FLOAT TraceOffsetFwdStanding = 20.0f;
		TWEAKABLE FLOAT TraceOffsetFwdCrouched = 12.0f;
		TWEAKABLE FLOAT TraceWidth = 15.0f;
		FLOAT offsetFwd = bIsCrouched ? TraceOffsetFwdCrouched : TraceOffsetFwdStanding;
		FVector endTrace = startTrace + TraceOffsetSide*(bRight ? Rotation.Right() : -Rotation.Right()) + offsetFwd*CharForward;

		if (!GWorld->SingleLineCheck( Hit, this, endTrace, startTrace, TRACE_AllBlocking | TRACE_StopAtAnyHit, FVector(TraceWidth, TraceWidth, TraceWidth)))
		{
			// Next to a wall or other object - can't lean
			return FALSE;
		}

		FLOAT newLeanTarget = leanInput;

		if (appIsNearlyZero(CurrentLean, KINDA_SMALL_NUMBERF) && !appIsNearlyZero(newLeanTarget, KINDA_SMALL_NUMBERF) && (GWorld->GetTimeSeconds() > LastLeanSndTime + 1.0f))
		{
			LastLeanSndTime = GWorld->GetTimeSeconds();
			TriggerSoundEvent(SndStartPeek);
		}

		CurrentLean = newLeanTarget;

		return TRUE;
	}

	return FALSE;
}

void AOLHero::StopLeaning()
{
	if (!appIsNearlyZero(CurrentLean, KINDA_SMALL_NUMBERF))
	{
		if (GWorld->GetTimeSeconds() > LastLeanSndTime + 0.5f)
		{
			LastLeanSndTime = GWorld->GetTimeSeconds();
			TriggerSoundEvent(SndStopPeek);
		}
	}

	CurrentLean = 0.0f;
}

void AOLHero::UpdateLookBackIntent(UBOOL bLeanInput)
{
	bWantLookBack = bLeanInput && (IsRunning() || (SpecialMove == SMT_JumpOver || SpecialMove == SMT_ClimbUpObstacle || SpecialMove == SMT_SlideOver));
}

UBOOL AOLHero::CanLookBack()
{
	return (LocomotionMode == LM_Walk || IsConsideredLookingBack()) && 
			(SpecialMove == SMT_None || SpecialMove == SMT_EnterLookBack || SpecialMove == SMT_ExitLookBack) && 
			!CornerPeek.CornerMarker && !IsInCamcorderTransition() && 
			(bWantToRun && (RunSpeed > WalkSpeed + 40.0f) && RealVelocity.SizeSquared2D() > Square(1.05f*WalkSpeed));
}

UBOOL AOLHero::IsConsideredLookingBack()
{
	if (LocomotionMode == LM_LookBack)
	{
		return TRUE;
	}
	
	if (bWantLookBack && Camera->LookBackRatio > 0.9f && (LocomotionMode == LM_Walk || LocomotionMode == LM_Fall) && GWorld->GetTimeSeconds() < FallingStartedTime + 1.5f)
	{
		// Semi-hack - keep lookback effect alive for small drops, even if not technically in lookback mode
		return TRUE;
	}

	return FALSE;
}

UBOOL AOLHero::TryLookBack(UBOOL bLeftSide)
{
	if (LocomotionMode == LM_Walk && CanLookBack() && CouldStartSpecialMove(SMT_EnterLookBack))
	{
		TWEAKABLE FLOAT MinReLookDelay = 1.25f;
		TWEAKABLE FLOAT MinTimeAfterSM = 0.75f;
		if (GWorld->GetTimeSeconds() < LastEnterLookBackTime + MinReLookDelay)
		{
			return FALSE;
		}

		if (GWorld->GetTimeSeconds() < LastClimbUpObstacleFinishedTime + MinTimeAfterSM)
		{
			return FALSE;
		}

		if (!TryCommitToSpecialMove(SMT_EnterLookBack))
		{
			return FALSE;
		}

		bLookingBackLeftSide = bLeftSide;
		StartSpecialMove(SMT_EnterLookBack);
		TriggerSoundEvent(SndStartLookBack);
		
		LastEnterLookBackTime = GWorld->GetTimeSeconds();
		TargetCamcorderZoomFactor = 0.0f; // unzoom and lose zoom control when in lookback
		return TRUE;
	}

	return FALSE;
}

UBOOL AOLHero::TryStopLookBack()
{
	if (CouldStartSpecialMove(SMT_ExitLookBack))
	{
		if (!TryCommitToSpecialMove(SMT_ExitLookBack))
		{
			return FALSE;
		}

		StartSpecialMove(SMT_ExitLookBack);
		EyeRotation.Yaw = Rotation.Yaw;
		TriggerSoundEvent(SndStopLookBack);
		return TRUE;
	}

	return FALSE;
}

UBOOL AOLHero::TryLeanPushing(UBOOL bRight)
{
	if (LocomotionMode == LM_Pushing)
	{		
		FCheckResult Hit(1.f);
		FVector startTrace = Location;
		startTrace.Z = EyeLocation.Z;
		TWEAKABLE FLOAT TraceOffsetSide = 45.0f;
		TWEAKABLE FLOAT TraceOffsetFwd = 20.0f;
		TWEAKABLE FLOAT TraceWidth = 15.0f;
		FVector endTrace = startTrace + TraceOffsetSide*(bRight ? Rotation.Right() : -Rotation.Right()) + TraceOffsetFwd*CharForward;

		if (!GWorld->SingleLineCheck( Hit, this, endTrace, startTrace, TRACE_AllBlocking | TRACE_StopAtAnyHit, FVector(TraceWidth, TraceWidth, TraceWidth)))
		{
			// Next to a wall or other object - can't lean
			return FALSE;
		}

		if ((bRight && !bLeaningRightPushing) || (!bRight && !bLeaningLeftPushing))
		{
			bLeaningLeftPushing = !bRight;
			bLeaningRightPushing = bRight;			
		}

		return TRUE;
	}

	return FALSE;
}

UBOOL AOLHero::TryLeanLeftPushing()
{
	return TryLeanPushing(FALSE);
}

UBOOL AOLHero::TryLeanRightPushing()
{
	return TryLeanPushing(TRUE);
}

void AOLHero::StopLeanPushing()
{
	bLeaningLeftPushing = FALSE;
	bLeaningRightPushing = FALSE;
}


////////////////////////////////////////////////////////////////////////////////////////////
// Control Modes
////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////

void AOLHero::EnterCinematicMode(USeqAct_ToggleCinematicMode* seqAction)
{
	if (SpecialMove != SMT_None)
	{
		CancelSpecialMove();

		if (FullBodyAnimSlot->bIsPlayingCustomAnim)
		{
			FullBodyAnimSlot->StopCustomAnim(0.1f);
		}
	}

	if (LocomotionMode == LM_LedgeWalk && ActiveLedge)
	{
		// rcharpentier - specific code for prison struggle that has been replaced by matinee
		bEnterLedgeWalkAfterStruggle = TRUE;
	}

	ActiveDoor = NULL;
	ActiveSqueeze = NULL;
	ActiveBed = NULL;
	ActiveLocker = NULL;

	if (seqAction->bAllowCameraOffset)
	{
		LocomotionModeParams[LM_Cinematic].GP.CameraMode = CRM_Spring;
		LocomotionModeParams[LM_Cinematic].GP.MinPitchCS = seqAction->MinPitch;
		LocomotionModeParams[LM_Cinematic].GP.MaxPitchCS = seqAction->MaxPitch;
		LocomotionModeParams[LM_Cinematic].GP.MinYaw = seqAction->MinYaw;
		LocomotionModeParams[LM_Cinematic].GP.MaxYaw = seqAction->MaxYaw;
		LocomotionModeParams[LM_Cinematic].NeckOffsetSide = seqAction->NeckOffsetSide;
		LocomotionModeParams[LM_Cinematic].NeckOffsetFwd = seqAction->NeckOffsetFwd;
	}
	else
	{
		LocomotionModeParams[LM_Cinematic].GP.CameraMode = CRM_FullyAnimated;
		LocomotionModeParams[LM_Cinematic].NeckOffsetSide = 0.0f;
		LocomotionModeParams[LM_Cinematic].NeckOffsetFwd = 0.0f;
	}

	LocomotionModeParams[LM_Cinematic].GP.BothHandsNeeded = seqAction->bDeactivateCamcorder;

	if (seqAction->bDeactivateCamcorder)
	{
		bLastCinematicModeDisabledCamcorder = IsCamcorderActive();
		bCamcorderDesired = FALSE;
	}

	Camera->bLocalSpacePlayerControl = seqAction->bLocalSpacePlayerControl;

	Velocity = FVector(0.0f);
	Acceleration = FVector(0.0f);
	ExternalImpulse = FVector(0.0f);

	if (seqAction->bDisableCollision)
	{
		SetCollision(TRUE, FALSE, TRUE);
		bCollideWorld = FALSE;
	}

	if (seqAction->NewBase)
	{
		Super::SetBase(seqAction->NewBase); // super version to bypass our overriden SetBase(), disabled during cinematic mode
		setPhysics(PHYS_Custom);
	}

	EnterLocomotionMode(LM_Cinematic);
	
	SetBodySetup(HBS_NoProxy);

	OLPC->HUD->bCrosshairDesired = FALSE;
}

void AOLHero::ExitCinematicMode(USeqAct_ToggleCinematicMode* seqAction)
{
	if (HeroControl)
	{
		HeroControl->bCompleted = TRUE;
		HeroControl->bPendingKill = TRUE;
		HeroControl = NULL;
	}

	SetCollision(TRUE, TRUE, FALSE);
	bCollideWorld = TRUE;
	setPhysics(PHYS_Walking);

	if (bEnterLedgeWalkAfterStruggle && ActiveLedge)
	{
		EnterLocomotionMode(LM_LedgeWalk);
		bEnterLedgeWalkAfterStruggle = FALSE;
	}
	else
	{
		EnterLocomotionMode(LM_Walk);
	}

	if (CamcorderState == CCS_Inactive)
	{
		SetBodySetup(HBS_Normal);
	}
	else if (IsInNightVision())
	{
		SetBodySetup(HBS_CamcorderRaisedNoShadow);
	}
	else
	{
		SetBodySetup(HBS_CamcorderRaised);
	}

	if (seqAction->bRestoreCamcorder && bLastCinematicModeDisabledCamcorder)
	{
		bCamcorderDesired = TRUE;
		TryRaiseCamcorder();
	}

	Camera->bLocalSpacePlayerControl = FALSE;

	OLPC->HUD->bCrosshairDesired = TRUE;
}

UBOOL AOLHero::HeroControlActivated(UOLSeqAct_HeroControl* heroControlAction)
{
	if (LocomotionMode == LM_Door)
	{
		EnterLocomotionMode(LM_Walk);
		ActiveDoor->DoorUser = NULL;
		ActiveDoor = NULL;
	}
	
	HeroControl = heroControlAction;
	HeroControl->ElapsedTime = 0.0f;
	HeroControl->OriginalCamLocation = Camera->BaseLocation;
	HeroControl->OriginalCamRotation = Camera->BaseRotation;

	if (HeroControl->GoToTarget)
	{
		FProceduralAnimData animData;
		animData.HeadingDelta = UNR_TO_DEG * (FLOAT)FRotator::NormalizeAxis(HeroControl->GoToTarget->Rotation.Yaw - Rotation.Yaw);
		animData.PositionDelta = HeroControl->GoToTarget->Location - Location;

		FLOAT linearVel = HeroControl->MovementSpeed;
		FLOAT angularVel = HeroControl->RotationSpeed;

		if (HeroControl->FixedDuration > 0.0f)
		{
			linearVel = animData.PositionDelta.Size() / HeroControl->FixedDuration;
			angularVel = animData.HeadingDelta / HeroControl->FixedDuration;
			HeroControl->Duration = HeroControl->FixedDuration;

			if (appIsNearlyZero(linearVel, KINDA_SMALL_NUMBERF))
			{
				linearVel = HeroControl->MovementSpeed;
			}
			if (appIsNearlyZero(angularVel, KINDA_SMALL_NUMBERF))
			{
				angularVel = HeroControl->RotationSpeed;
			}
		}
		else
		{
			FLOAT timeForHeading = animData.HeadingDelta / angularVel;
			FLOAT timeForPos = animData.PositionDelta.Size() / linearVel;
			HeroControl->Duration = Max(timeForHeading, timeForPos);
		}

		QueueProceduralAnim(animData, linearVel, angularVel);
	}
	else if (HeroControl->LookAtTarget)
	{
		HeroControl->Duration = HeroControl->FixedDuration;
	}

	return TRUE;
}

void AOLHero::UpdateHeroControl(FLOAT deltaTime)
{
	HeroControl->ElapsedTime += deltaTime;

	UBOOL running = (HeroControl->GoToTarget && ProceduralAnims.Num() > 0) || (HeroControl->FixedDuration > 0.0f && HeroControl->ElapsedTime < HeroControl->FixedDuration);
		
	if (!running)
	{
		HeroControl->bCompleted = TRUE;
		HeroControl->bPendingKill = TRUE;
		HeroControl = NULL;
	}
}

UBOOL AOLHero::EnterLocomotionMode(ELocomotionMode newMode)
{
	if (newMode == LocomotionMode)
	{
		return TRUE;
	}

	ELocomotionMode prevMode = (ELocomotionMode)LocomotionMode;
	
	if (newMode != LM_SpecialMove)
	{
		bFailedCollisionSet = !TryAdjustCollisionSizeForLocomotionMode(newMode);

		if (bFailedCollisionSet)
		{
			debugf(TEXT("TryAdjustCollisionSizeForLocomotionMode failed for mode %s"), *Utils::GetEnumString("ELocomotionMode", newMode));
		}
	}

	if (LocomotionModeParams[LocomotionMode].GP.DisableCollisions)
	{
		SetCollision(TRUE, TRUE, FALSE);
		bCollideWorld = TRUE;
	}

	LocomotionMode = newMode;

	if (newMode == LM_Fall)
	{
		bApplyLandingPenalty = FALSE;
		FallingStartedTime = GWorld->GetTimeSeconds();
	}
	else if (newMode == LM_Walk)
	{
		bJumping = FALSE;
	}

	if (bIsCrouched && CanUncrouch() && ((prevMode != LM_Walk && prevMode != LM_Fall) || (newMode != LM_Walk && newMode != LM_Fall)))
	{
		bWantsToCrouch = FALSE;
	}

	if (newMode != LM_SpecialMove)
	{
		if (Physics != LocomotionModeParams[newMode].GP.Physics)
		{
			setPhysics(LocomotionModeParams[newMode].GP.Physics);
		}

		if (LocomotionModeParams[newMode].GP.DisableCollisions)
		{
			SetCollision(TRUE, FALSE, TRUE);
			bCollideWorld = FALSE;
		}	

		SetRootMotionMode((ERootMotionMode)LocomotionModeParams[newMode].GP.RMM);

		bBothHandsNeeded = LocomotionModeParams[newMode].GP.BothHandsNeeded;

		if (bBothHandsNeeded)
		{
			if (CamcorderState != CCS_Inactive)
			{
				LowerCamcorder();
			}
			else
			{
				SetBodySetup(HBS_Normal);
			}
		}		
	}

	SetCamParams(LocomotionModeParams[LocomotionMode].GP);

	return TRUE;
}

FLOAT AOLHero::GetCollisionRadiusForLocomotionMode(ELocomotionMode locoMode) const
{
	if (LocomotionModeParams[locoMode].GP.CollisionRadius > 0.0f)
	{
		return LocomotionModeParams[locoMode].GP.CollisionRadius;
	}

	return DefaultHero->CylinderComponent->CollisionRadius;
}

FLOAT AOLHero::GetCollisionHeightForLocomotionMode(ELocomotionMode locoMode) const
{
	if (locoMode == LM_Walk && bIsCrouched)
	{
		return CrouchHeight;
	}
	else if (LocomotionModeParams[locoMode].GP.CollisionHeight > 0.0f)
	{
		return LocomotionModeParams[locoMode].GP.CollisionHeight;
	}

	return DefaultHero->CylinderComponent->CollisionHeight;
}

UBOOL AOLHero::TryAdjustCollisionSizeForLocomotionMode(ELocomotionMode newMode)
{	
	FLOAT NewHeight = GetCollisionHeightForLocomotionMode(newMode);
	FLOAT NewRadius = GetCollisionRadiusForLocomotionMode(newMode);

	return Super::TryAdjustCollisionSize(NewHeight, NewRadius);
}

UBOOL AOLHero::IsPlayerInputEnabled() const
{
	return (SpecialMove == SMT_None || SpecialMoveParams[SpecialMove].PlayerInputEnabled);
}

void AOLHero::StartStruggle(const FStruggleConfig& struggleConfig, const FVector& refLocation, const FVector& refDirection)
{
	if (LocomotionMode == LM_LedgeWalk && ActiveLedge)
	{
		bEnterLedgeWalkAfterStruggle = TRUE; // may need eventually to be exposed in the struggle config
	}

	if (struggleConfig.HeroAnimSets.Num() > 0)
	{
		UpdateAnimSetList(); // add the required anim set
	}

	LowerCamcorder();
	StartSpecialMove(SMT_EnterStruggle, refLocation, refDirection, APTT_TargetAtStart);
}

void AOLHero::FinishStruggle(UBOOL bSucceeded)
{
	if (bSucceeded)
	{
		StartSpecialMove(SMT_ExitStruggle);
	}
	else
	{
		StartSpecialMove(SMT_KilledInStruggle);
	}
}

////////////////////////////////////////////////////////////////////////////////////////////
// Camcorder
////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////

UBOOL AOLHero::CanZoom()
{
	return IsCamcorderActive() && LocomotionMode != LM_LookBack && (SpecialMove == SMT_None || SpecialMove == SMT_Crouch || SpecialMove == SMT_Uncrouch || SpecialMove == SMT_JumpOnSpot);
}

void AOLHero::ZoomImpulse(FLOAT impulse)
{
	if (CanZoom())
	{		
		TWEAKABLE FLOAT InputScaling = 0.25f;

		FLOAT prevFactor = TargetCamcorderZoomFactor;

		TargetCamcorderZoomFactor += impulse * InputScaling;
		TargetCamcorderZoomFactor = Saturate(TargetCamcorderZoomFactor);

		if (TargetCamcorderZoomFactor != prevFactor)
		{
			TriggerSoundEvent((impulse > 0.0f) ? SndZoomIn : SndZoomOut);
		}
	}
}

void AOLHero::StartedActiveZoom(UBOOL bZoomingIn)
{
	if (CanZoom())
	{
		if ((bZoomingIn && TargetCamcorderZoomFactor < 1.0f) || (!bZoomingIn && TargetCamcorderZoomFactor > 0.0f))
		{
			TriggerSoundEvent(bZoomingIn ? SndZoomIn : SndZoomOut);
		}
	}
}

void AOLHero::SetTargetZoom(FLOAT newTarget)
{
	if (CanZoom())
	{
		TargetCamcorderZoomFactor = Saturate(newTarget);
	}
}

UBOOL AOLHero::TryRaiseCamcorder()
{
	if (bBothHandsNeeded || LocomotionMode == LM_Cinematic || !OLPC || !OLPC->bHasCamcorder)
	{
		return FALSE;
	}

	if (CamcorderState != CCS_Active && CamcorderState != CCS_Raising)
	{
		UAnimNodeSequence* animSeq = RightArmAnimSlot->GetCustomAnimNodeSeq();
		FName animName = NAME_None;
		FName oppositeAnimName = NAME_None;

		if (LocomotionMode == LM_Bed)
		{
			animName = AnimNameRaiseCamcorderBed;
			oppositeAnimName = AnimNameLowerCamcorderBed;
		}
		else if (bIsCrouched)
		{
			animName = AnimNameRaiseCamcorderCrouched;
			oppositeAnimName = AnimNameLowerCamcorderCrouched;
		}
		else
		{
			animName = AnimNameRaiseCamcorder;
			oppositeAnimName = AnimNameLowerCamcorder;
		}

		FLOAT startPct = 0.0f;
		UBOOL performingOppositeMove = FALSE;

		if (animSeq)
		{
			if (animSeq->AnimSeqName == oppositeAnimName)
			{
				FLOAT currentPct = animSeq->CurrentTime / animSeq->GetAnimPlaybackLength();
				startPct = (1.0f - currentPct);
				performingOppositeMove = TRUE;
			}

			RightArmAnimSlot->StopCustomAnim(0.1f);
			ShadowProxyRightArmAnimSlot->StopCustomAnim(0.1f);
		}

		if (CamcorderState == CCS_Lowering && (!performingOppositeMove || startPct > 0.85f))
		{
			// Right at the start of the lowering motion, just cancel it

			if (RightArmAnimSlot->bIsPlayingCustomAnim)
			{
				RightArmAnimSlot->StopCustomAnim(0.1f);
				ShadowProxyRightArmAnimSlot->StopCustomAnim(0.1f);
			}

			ActivateCamcorder();
		}
		else
		{
			if (performingOppositeMove)
			{		
				UAnimSequence* newAnimSeq = Mesh->FindAnimSequence(animName);
				check(newAnimSeq);
				FLOAT startTime = startPct * newAnimSeq->SequenceLength;

				TWEAKABLE FLOAT BlendTime = 0.1f;
				startTime = Max(0.0f, startTime - BlendTime); // substract the blend time, to catch up in the middle

				TWEAKABLE FLOAT minPlayTime = 0.15f; // we need a strict minimum time to allow the notifies to trigger
				startTime = Min(startTime, Max(0.0f, newAnimSeq->SequenceLength - minPlayTime));			
		
				PlayRightArmAnim(animName, 1.0f, BlendTime, 0.0f, startTime);
			}
			else
			{
				PlayRightArmAnim(animName, 1.0f, 0.25f, 0.0f);
			}
			CamcorderState = CCS_Raising;
			LastCamcorderSwitchTime = GWorld->GetTimeSeconds();
		}
	}

	return TRUE;
}

void AOLHero::CamcorderRaisedNotify()
{
	if (CamcorderState == CCS_ReloadingActive && LocomotionMode == LM_Walk)
	{
		// reset collision
		TryAdjustCollisionSizeForLocomotionMode((ELocomotionMode)LocomotionMode);
	}

	if (CamcorderState == CCS_Raising || CamcorderState == CCS_ReloadingActive)
	{
		ActivateCamcorder();
	}

	if (LocomotionMode == LM_Bed)
	{
		// last minute hacks
		Camera->NeckOffsetFwd = LocomotionModeParams[LM_Bed].NeckOffsetFwd;
		Camera->NeckOffsetSide = LocomotionModeParams[LM_Bed].NeckOffsetSide;
	}
}

void AOLHero::LowerCamcorder()
{
	if (CamcorderState == CCS_Lowering)
	{
		return;
	}

	if (CamcorderState != CCS_Inactive)
	{
		UAnimNodeSequence* animSeq = RightArmAnimSlot->GetCustomAnimNodeSeq();
		FName animName = NAME_None;
		FName oppositeAnimName = NAME_None;

		if (LocomotionMode == LM_Bed)
		{
			animName = AnimNameLowerCamcorderBed;
			oppositeAnimName = AnimNameRaiseCamcorderBed;
		}
		else if (bIsCrouched)
		{
			animName = AnimNameLowerCamcorderCrouched;
			oppositeAnimName = AnimNameRaiseCamcorderCrouched;
		}
		else
		{
			animName = AnimNameLowerCamcorder;
			oppositeAnimName = AnimNameRaiseCamcorder;
		}

		FLOAT startPct = 0.0f;
		UBOOL performingOppositeMove = FALSE;

		if (animSeq)
		{
			if (animSeq->AnimSeqName == oppositeAnimName)
			{
				FLOAT currentPct = animSeq->CurrentTime / animSeq->GetAnimPlaybackLength();
				startPct = (1.0f - currentPct);
				performingOppositeMove = TRUE;
			}

			RightArmAnimSlot->StopCustomAnim(0.1f);
			ShadowProxyRightArmAnimSlot->StopCustomAnim(0.1f);
		}

		if (CamcorderState == CCS_Raising && (!performingOppositeMove || startPct > 0.85f))
		{
			// Right at the start of the raising motion, just cancel it

			if (RightArmAnimSlot->bIsPlayingCustomAnim)
			{
				RightArmAnimSlot->StopCustomAnim(0.1f);
				ShadowProxyRightArmAnimSlot->StopCustomAnim(0.1f);
			}

			CamcorderState = CCS_Inactive;
			SetBodySetup(HBS_Normal);
		}
		else
		{
			if (performingOppositeMove)
			{		
				UAnimSequence* newAnimSeq = Mesh->FindAnimSequence(animName);
				check(newAnimSeq);
				FLOAT startTime = startPct * newAnimSeq->SequenceLength;

				TWEAKABLE FLOAT BlendTime = 0.1f;
				startTime = Max(0.0f, startTime - BlendTime); // substract the blend time, to catch up in the middle

				TWEAKABLE FLOAT minPlayTime = 0.15f; // we need a strict minimum time to allow the notifies to trigger
				startTime = Min(startTime, Max(0.0f, newAnimSeq->SequenceLength - minPlayTime));			

				PlayRightArmAnim(animName, 1.0f, BlendTime, 0.25f, startTime);
			}
			else
			{
				PlayRightArmAnim(animName, 1.0f, 0.0f, 0.25f);
			}
		
			if (CamcorderState == CCS_Active)
			{
				DeactivateCamcorder();
			}

			CamcorderState = CCS_Lowering;
			LastCamcorderSwitchTime = GWorld->GetTimeSeconds();
		}

		if (LocomotionMode == LM_Bed)
		{
			CurrentFOV = LocomotionModeParams[LM_Bed].GP.FOVOverride;
		}
	}
}

void AOLHero::CamcorderLoweredNotify()
{
	if (CamcorderState == CCS_ReloadingInactive && LocomotionMode == LM_Walk)
	{
		// reset collision
		TryAdjustCollisionSizeForLocomotionMode((ELocomotionMode)LocomotionMode);
	}

	if (CamcorderState == CCS_Lowering || CamcorderState == CCS_ReloadingInactive)
	{
		SetBodySetup(HBS_Normal);
		CamcorderState = CCS_Inactive;
	}
}

void AOLHero::TriggerNVEvent(INT eventIdx)
{
	TArray<USequenceObject*> seqEvents;
	USequence* rootSeq = GWorld->GetGameSequence();
	if (rootSeq)
	{
		rootSeq->FindSeqObjectsByClass(UOLSeqEvent_NightVision::StaticClass(), seqEvents, TRUE);
		for(INT i=0; i < seqEvents.Num(); i++)
		{
			UOLSeqEvent_NightVision* nvEvent = (UOLSeqEvent_NightVision*)(seqEvents(i));
			TArray<INT> indices;
			indices.AddItem(eventIdx);
			nvEvent->CheckActivate(this, this, FALSE, &indices);
		}
	}
}

void AOLHero::ActivateCamcorder()
{
	CurrentCamcorderZoomFactor = 0.0f;
	TargetCamcorderZoomFactor = 0.0f;
	NVLightInterpFactor = 0.0f;
	
	ActivateCamcorderMode((ECamcorderMode)-1, (ECamcorderMode)CamcorderMode);

	OLPC->HUD->ShowCamcorderHUD();
			
	TriggerNVEvent(NVE_CamcorderActivated);
	TriggerSoundEvent(SndCamStart);
	SetAudioValue(RTPCZoom, 0.0f);

	if (bCameraEffectActive)
	{
		DeactivateCameraEffect();
	}

	CamcorderState = CCS_Active;
}

void AOLHero::DeactivateCamcorder()
{
	CurrentFOV = DefaultFOV;

	DarkLight->SetEnabled(TRUE);
	NVLightDefault->SetEnabled(FALSE);
	NVLightPowered->SetEnabled(FALSE);

	Utils::GetFXManager()->DeactivateNightVisionEffect();

	OLPC->HUD->HideCamcorderHUD();

	TriggerNVEvent(NVE_CamcorderDeactivated);	
	TriggerSoundEvent(SndCamStop);

	if (CamcorderMode == CCM_PoweredNightVision || CamcorderMode == CCM_NightVision)
	{
		TriggerSoundEvent(SndCamOnNVOff);
	}

	if (bPlayingNVGlitchSound)
	{
		TriggerSoundEvent(SndLowBatteryStop);
		bPlayingNVGlitchSound = FALSE;
		SetAudioValue(RTPCBatteryIntensity, 100.0f);
	}

	if (bCameraEffectActive)
	{
		DeactivateCameraEffect();
	}

	SetBodySetup(HBS_CamcorderVisible);

	CamcorderState = CCS_Inactive;
}

void AOLHero::ForceActivateNightVision()
{
	CamcorderMode = CCM_PoweredNightVision;
	ActivateCamcorder();
}

void AOLHero::ForceDeactivateNightVision()
{
	CamcorderMode = CCM_Default;
	bCamcorderDesired = FALSE;
	DeactivateCamcorder();
}

UBOOL AOLHero::TryToggleCamcorder()
{
	if (IsReloading())
	{
		return FALSE;
	}

	if (bBothHandsNeeded || LocomotionMode == LM_Cinematic || !OLPC || !OLPC->bHasCamcorder || Health == 0 || (GWorld->GetTimeSeconds() < CamcorderDisabledEndTime))
	{
		return FALSE;
	}

	if (CamcorderState == CCS_Active || CamcorderState == CCS_Raising)
	{
		bCamcorderDesired = FALSE;
		LowerCamcorder();		
		return TRUE;
	}
	else
	{
		bCamcorderDesired = TRUE;
		return TryRaiseCamcorder();
	}
}

void AOLHero::TryToggleNightVision()
{
	if (bBothHandsNeeded || LocomotionMode == LM_Cinematic || !OLPC || !OLPC->bHasCamcorder)
	{
		return;
	}

	if (IsCamcorderActive())
	{
		if (IsInNightVision())
		{
			DeactivateNightVision();			
		}
		else
		{
			ActivateNightVision();
		}
	}	
}

void AOLHero::ActivateNightVision()
{
	ECamcorderMode prevCamcorderMode = (ECamcorderMode)CamcorderMode;
	ECamcorderMode newCamcorderMode = appIsNearlyZero(CurrentBatterySetEnergy, KINDA_SMALL_NUMBERF) ? CCM_NightVision : CCM_PoweredNightVision;

	CamcorderMode = newCamcorderMode;
	ActivateCamcorderMode(prevCamcorderMode, (ECamcorderMode)CamcorderMode);	
}

void AOLHero::DeactivateNightVision()
{
	ECamcorderMode prevCamcorderMode = (ECamcorderMode)CamcorderMode;
	ECamcorderMode newCamcorderMode = CCM_Default;

	CamcorderMode = newCamcorderMode;
	ActivateCamcorderMode(prevCamcorderMode, (ECamcorderMode)CamcorderMode);
}

void AOLHero::ActivateCamcorderMode(ECamcorderMode prevCamcorderMode, ECamcorderMode newCamcorderMode)
{
	switch (newCamcorderMode)
	{
	case CCM_Default:
		DarkLight->SetEnabled(TRUE);
		NVLightDefault->SetEnabled(FALSE);
		NVLightPowered->SetEnabled(FALSE);
		TriggerNVEvent(NVE_DefaultViewSelected);
		break;
	case CCM_NightVision:
		DarkLight->SetEnabled(FALSE);
		NVLightDefault->SetEnabled(TRUE);
		NVLightPowered->SetEnabled(FALSE);		
		TriggerNVEvent(NVE_PassiveNVSelected);
		break;
	case CCM_PoweredNightVision:
		DarkLight->SetEnabled(FALSE);		
		NVLightDefault->SetEnabled(FALSE);
		NVLightPowered->InnerConeAngle = DefaultHero->NVLightPowered->InnerConeAngle;
		NVLightPowered->OuterConeAngle = DefaultHero->NVLightPowered->OuterConeAngle;
		NVLightPowered->Radius = DefaultHero->NVLightPowered->Radius;
		NVLightPowered->Brightness = DefaultHero->NVLightPowered->Brightness;
		NVLightPowered->SetLightProperties(DefaultHero->NVLightPowered->Brightness, NVLightPowered->LightColor, NVLightPowered->Function);
		NVLightPowered->SetEnabled(TRUE);
		TriggerNVEvent(NVE_ActiveNVSelected);
		break;
	}
	
	if (newCamcorderMode == CCM_NightVision || newCamcorderMode == CCM_PoweredNightVision)
	{
		Utils::GetFXManager()->ActivateNightVisionEffect(newCamcorderMode == CCM_PoweredNightVision);
		SetBodySetup(HBS_CamcorderRaisedNoShadow);

		if (prevCamcorderMode != CCM_NightVision && prevCamcorderMode != CCM_PoweredNightVision)
		{		
			TriggerSoundEvent(SndCamOnNVOn);
		}
	}
	else if (newCamcorderMode == CCM_Default)
	{		
		Utils::GetFXManager()->ActivateCamcorderEffect();
		SetBodySetup(HBS_CamcorderRaised);

		if (prevCamcorderMode == CCM_NightVision || prevCamcorderMode == CCM_PoweredNightVision)
		{
			TriggerSoundEvent(SndCamOnNVOff);
		}
	}

	if (bPlayingNVGlitchSound && newCamcorderMode != CCM_PoweredNightVision)
	{
		TriggerSoundEvent(SndLowBatteryStop);
		bPlayingNVGlitchSound = FALSE;
		SetAudioValue(RTPCBatteryIntensity, 100.0f);
	}
	else if (((CurrentBatterySetEnergy * BatteryDuration) < NVGlitchTimeThreshold) && newCamcorderMode == CCM_PoweredNightVision)
	{
		TriggerSoundEvent(SndLowBatteryStart);
		bPlayingNVGlitchSound = TRUE;
	}
}

void AOLHero::SetDarkLightOverride(FLOAT brightness, FLOAT radius)
{
	bOverrideDarkLight = TRUE;
	DarkLightOverrideBrightness = brightness;
	DarkLightOverrideRadius = radius;
}

void AOLHero::ResetDarkLightOverride()
{
	bOverrideDarkLight = FALSE;
}

void AOLHero::GiveCamcorder(UBOOL andUseRightAway, UBOOL bWithNV)
{
	OLPC->bHasCamcorder = TRUE;

	if (andUseRightAway)
	{
		bCamcorderDesired = TRUE;
		CamcorderMode = bWithNV ? (appIsNearlyZero(CurrentBatterySetEnergy, KINDA_SMALL_NUMBERF) ? CCM_NightVision : CCM_PoweredNightVision) : CCM_Default;
		ActivateCamcorder();
	}
}

void AOLHero::RemoveCamcorder()
{
	OLPC->bHasCamcorder = FALSE; // Assume that this always takes place offscreen (matinee), never while it is being used
}

void AOLHero::UpdateCamcorder(FLOAT deltaTime)
{
	if (LocomotionMode == LM_Cinematic)
	{
		return;
	}

	if (IsInCamcorderTransition())
	{
		if (GWorld->GetTimeSeconds() > LastCamcorderSwitchTime + 3.5f)
		{
			warnf(TEXT("### Detected stale camcorder transition! (CamcorderState: %s). Aborting!"), *Utils::GetEnumString("ECamcorderState", CamcorderState));
			CamcorderState = CCS_Inactive;
			bCamcorderDesired = FALSE;
			SetBodySetup(HBS_Normal);			
		}

		return;
	}

	UBOOL bUseCamcorder = IsCamcorderActive() || bCamcorderDesired;
	
	// Update desired state
	if (bBothHandsNeeded || !OLPC || !OLPC->bHasCamcorder || Health == 0 || (GWorld->GetTimeSeconds() < CamcorderDisabledEndTime))
	{
		// Must be disabled
		bUseCamcorder = FALSE;
	}
	
	if (bUseCamcorder && IsCamcorderInactive())
	{
		TryRaiseCamcorder();
	}
	else if (!bUseCamcorder && IsCamcorderActive())
	{
		LowerCamcorder();
	}
}

void AOLHero::UpdateRainEffect(FLOAT deltaTime)
{
	TWEAKABLE FLOAT DeactivatedUpdateTime = 45.0f; // time to continue updating after we deactivate, for left-over particles
	UBOOL leftOverEffect = LastRainEffectActiveTime > 0.0f && (GWorld->GetTimeSeconds() - LastRainEffectActiveTime) <= DeactivatedUpdateTime;
	UBOOL effectActive = (bRainEffectDesired || leftOverEffect);

	if (IsCamcorderActive())
	{
		if (effectActive && RainEffect->HiddenGame)
		{
			RainEffect->SetHiddenGame(FALSE);
		}
		else if (!effectActive && !RainEffect->HiddenGame)
		{
			RainEffect->SetHiddenGame(TRUE);
		}

		if (bRainEffectDesired)
		{			
			RainEffect->SetActive(TRUE);
			LastRainEffectActiveTime = GWorld->GetTimeSeconds();
		}		
	}
	else if (!RainEffect->HiddenGame)
	{
		RainEffect->SetHiddenGame(TRUE);
	}

	if (!bRainEffectDesired || !IsCamcorderActive())
	{	
		RainEffect->SetActive(FALSE);
	}
	
	if (RainEffect->IsAttached() && effectActive)
	{
		FVector camActorSpace = LocalToWorld().InverseTransformFVector(EyeLocation);
		RainEffect->Rotation = FRotator(EyeRotation.Pitch, EyeRotation.Yaw - Rotation.Yaw, EyeRotation.Roll);

		// FOV compensate
		TWEAKABLE FLOAT PlaneDist = 20.0f;
		FLOAT scaleFactor = 1.0f / appTan(0.5f * CurrentFOV * DEG_TO_RAD); // 0.666f = atan(45 degs)
		FLOAT planeFwdOffset = PlaneDist * (scaleFactor - 1.0f);
		FVector offset = planeFwdOffset * RainEffect->Rotation.Vector();

		RainEffect->Translation = camActorSpace + offset;
		RainEffect->ConditionalUpdateTransform();
	}
}

void AOLHero::DeactivateCameraEffect()
{
	GenericCameraEffect->SetHiddenGame(TRUE);
	GenericCameraEffect->SetActive(FALSE);
	GenericCameraEffect->ConditionalDetach();
	GenericCameraEffect->SetTemplate(NULL);
	bCameraEffectActive = FALSE;
}

void AOLHero::UpdateCameraEffect(FLOAT deltaTime)
{
	if (bCameraEffectActive)
	{
		if (GWorld->GetTimeSeconds() > CameraEffectEndTime)
		{
			// done
			DeactivateCameraEffect();
		}
		else if (GenericCameraEffect->IsAttached())
		{
			FVector camActorSpace = LocalToWorld().InverseTransformFVector(EyeLocation);
			GenericCameraEffect->Rotation = FRotator(EyeRotation.Pitch, EyeRotation.Yaw - Rotation.Yaw, EyeRotation.Roll);

			// FOV compensate
			FLOAT scaleFactor = 1.0f / appTan(0.5f * CurrentFOV * DEG_TO_RAD); // 0.666f = atan(45 degs)
			FLOAT planeFwdOffset = CameraEffectPlaneDist * (scaleFactor - 1.0f);
			FVector offset = planeFwdOffset * GenericCameraEffect->Rotation.Vector();

			GenericCameraEffect->Translation = camActorSpace + offset;
			GenericCameraEffect->ConditionalUpdateTransform();
		}
	}
}

void AOLHero::StartElectricGlitch(FLOAT glitchIntensity, FLOAT glitchFrequency, FLOAT glitchDuration)
{
	ElectricGlitchStartTime = GWorld->GetTimeSeconds();
	ElectricGlitchMaxIntensity = glitchIntensity;
	ElectricGlitchDuration = glitchDuration;
	ElectricGlitchFrequency = glitchFrequency;
	bElectricGlitchActive = TRUE;
}

void AOLHero::UpdateElectricGlitch(FLOAT deltaTime)
{
	//FLOAT electricEffect = 0.0f;

	//if (bElectricGlitchActive && IsCamcorderActive())
	//{
	//	FLOAT timeElapsed = GWorld->GetTimeSeconds() - ElectricGlitchStartTime;
	//	if (timeElapsed > ElectricGlitchDuration)
	//	{
	//		bElectricGlitchActive = TRUE;
	//	}
	//	else
	//	{
	//		FLOAT intensity = ElectricGlitchMaxIntensity * Saturate(1.0f - timeElapsed / ElectricGlitchDuration);				
	//		FLOAT sineVal = 0.5f + 0.5f*appSin(ElectricGlitchFrequency * timeElapsed * 2.0f * PI); // [0,1]
	//		electricEffect = intensity * sineVal;
	//	}
	//}		
	//
	//Utils::GetFXManager()->UpdateElectricityEffect(deltaTime, electricEffect);
}

void AOLHero::UpdateNightVision(FLOAT deltaTime)
{
	USpotLightComponent* activeLight = NULL;

	if (IsCamcorderActive())
	{
		TWEAKABLE FLOAT ActiveZoomRate = 1.5f;
		TWEAKABLE FLOAT ZoomApproachCoeff = 0.99995f;
		TWEAKABLE FLOAT NVLightParamsApproachCoeff = 0.996f;

		if (!CanZoom())
		{
			TargetCamcorderZoomFactor = 0.0f;
		}
		else if (OLPC->ZoomInput != 0)
		{
			// active zooming (gamepad)
			FLOAT deltaZoom = deltaTime * ActiveZoomRate * OLPC->ZoomInput;
			TargetCamcorderZoomFactor = Saturate(TargetCamcorderZoomFactor + deltaZoom);			
		}
				
		CurrentCamcorderZoomFactor = Utils::Approach(CurrentCamcorderZoomFactor, TargetCamcorderZoomFactor, ZoomApproachCoeff, deltaTime);
		SetAudioValue(RTPCZoom, CurrentCamcorderZoomFactor*100.0f);
				
		NVLightInterpFactor = Utils::Approach(NVLightInterpFactor, TargetCamcorderZoomFactor, NVLightParamsApproachCoeff, deltaTime);
	}

	if (IsCamcorderActive() && CamcorderMode == CCM_PoweredNightVision)
	{
		activeLight = NVLightPowered;
				
		if ((!Utils::GetCheatManager() || !Utils::GetCheatManager()->bUnlimitedBatteries) && BatteryDuration > 0.0f)
		{
			CurrentBatterySetEnergy -= deltaTime / BatteryDuration;

			if (CurrentBatterySetEnergy <= 0.0f)
			{
				if (bPlayingNVGlitchSound)
				{
					SetAudioValue(RTPCBatteryIntensity, 0.0f);
				}

				TriggerNVEvent(NVE_OutOfBatteries);
				ActivateCamcorderMode(CCM_PoweredNightVision, CCM_NightVision);
				CamcorderMode = CCM_NightVision;
				activeLight = NVLightDefault;
				CurrentBatterySetEnergy = 0.0f;				
			}
		}

		UpdateNVGlitch(deltaTime);
		
		FLOAT targetInnerConeAngle = Lerp(DefaultHero->NVLightPowered->InnerConeAngle, NVLightZoomedInInnerAngle, NVLightInterpFactor);
		FLOAT targetOuterConeAngle = Lerp(DefaultHero->NVLightPowered->OuterConeAngle, NVLightZoomedInOuterAngle, NVLightInterpFactor);
		FLOAT targetRadius = Lerp(DefaultHero->NVLightPowered->Radius, NVLightZoomedInRadius, NVLightInterpFactor);			
		FLOAT targetBrightness = Lerp(DefaultHero->NVLightPowered->Brightness, NVLightZoomedInBrightness, NVLightInterpFactor);

		if (NVGlitch.bGlitching)
		{
			targetBrightness = Lerp(0.0f, targetBrightness, NVGlitch.CurrentLevel);
		}

		if (	!appIsNearlyEqual(NVLightPowered->InnerConeAngle, targetInnerConeAngle, KINDA_SMALL_NUMBERF) || 
				!appIsNearlyEqual(NVLightPowered->OuterConeAngle, targetOuterConeAngle, KINDA_SMALL_NUMBERF) || 
				!appIsNearlyEqual(NVLightPowered->Radius, targetRadius, KINDA_SMALL_NUMBERF) || 
				!appIsNearlyEqual(NVLightPowered->Brightness, targetBrightness, KINDA_SMALL_NUMBERF))
		{
			NVLightPowered->InnerConeAngle = targetInnerConeAngle;
			NVLightPowered->OuterConeAngle = targetOuterConeAngle;
			NVLightPowered->Radius = targetRadius;
			NVLightPowered->Brightness = targetBrightness;

			FComponentReattachContext ReattachContextMesh(NVLightPowered);
		}

		CurrentDarkLightRadius = DarkLightRadiusDefault;
		CurrentDarkLightBrightness = DarkLightBrightnessDefault;
	}	
	else if (IsCamcorderActive() && CamcorderMode == CCM_NightVision)
	{
		activeLight = NVLightDefault;

		CurrentDarkLightRadius = DarkLightRadiusDefault;
		CurrentDarkLightBrightness = DarkLightBrightnessDefault;
	}
	else
	{
		TWEAKABLE FLOAT DarkLightBrightnessApproachCoeff = 0.9f;
		TWEAKABLE FLOAT DarkLightRadiusApproachCoeff = 0.9f;

		activeLight = DarkLight;

		FLOAT desiredBrightness = 0.0f;
		FLOAT desiredRadius = 0.0f;

		UBOOL bAttacked = (SpecialMove == SMT_HeroKilled || SpecialMove == SMT_HeroThrown || SpecialMove == SMT_HeroDecapitate || SpecialMove == SMT_Dying || 
								SpecialMove == SMT_HeroGrabbedNormal || SpecialMove == SMT_HeroGrabbedFromBed || SpecialMove == SMT_HeroGrabbedFromLocker || SpecialMove == SMT_HeroGrabbedFromSqueeze || SpecialMove == SMT_HeroGrabbedFromUnder);

		if (bOverrideDarkLight)
		{
			desiredBrightness = DarkLightOverrideBrightness;
			desiredRadius = DarkLightOverrideRadius;
		}
		else if (bAttacked)
		{
			desiredBrightness = DarkLightBrightnessAttacked;
			desiredRadius = DarkLightRadiusAttacked;
		}
		else if (bParrying)
		{
			desiredBrightness = DarkLightBrightnessParrying;
			desiredRadius = DarkLightRadiusParrying;
		}
		else if (!OLPC->bHasCamcorder)
		{
			desiredBrightness = DarkLightBrightnessNoCamcorder;
			desiredRadius = DarkLightRadiusNoCamcorder;
		}
		else if (GP().BothHandsNeeded)
		{
			desiredBrightness = DarkLightBrightnessBothHandsNeeded;
			desiredRadius = DarkLightRadiusBothHandsNeeded;
		}
		else
		{
			desiredBrightness = DarkLightBrightnessDefault;
			desiredRadius = DarkLightRadiusDefault;
		}

		CurrentDarkLightBrightness = Utils::Approach(CurrentDarkLightBrightness, desiredBrightness, DarkLightBrightnessApproachCoeff, deltaTime);
		CurrentDarkLightRadius = Utils::Approach(CurrentDarkLightRadius, desiredRadius, DarkLightRadiusApproachCoeff, deltaTime);

		if (!appIsNearlyEqual(DarkLight->Radius, CurrentDarkLightRadius, KINDA_SMALL_NUMBERF) || !appIsNearlyEqual(DarkLight->Brightness, CurrentDarkLightBrightness, KINDA_SMALL_NUMBERF))
		{
			// dark light settings changed - update them
			DarkLight->SetLightProperties(CurrentDarkLightBrightness, DarkLight->LightColor, DarkLight->Function);
			DarkLight->Radius = CurrentDarkLightRadius;
			DarkLight->BeginDeferredReattach();
		}
	}

	if (activeLight)
	{
		if (OLPC->bDebugFreeCam)
		{
			activeLight->Translation = LocalToWorld().InverseTransformFVector(OLPC->DebugCamPos);
			activeLight->SetRotation(FRotator(OLPC->DebugCamRot.Pitch, OLPC->DebugCamRot.Yaw - Rotation.Yaw, 0));
		}
		else
		{
			activeLight->Translation = LocalToWorld().InverseTransformFVector(EyeLocation);
			activeLight->SetRotation(FRotator(EyeRotation.Pitch, EyeRotation.Yaw - Rotation.Yaw, 0));
		}
	}
}

void AOLHero::UpdateNVGlitch(FLOAT deltaTime)
{
	TWEAKABLE FLOAT BuzzGlitchDuration = 0.3f;
	TWEAKABLE INT NbBuzzGlitchCycles = 3;
	TWEAKABLE FLOAT LastBreathDuration = 0.6f;
	TWEAKABLE INT LastBreathCycles = 6;

	FLOAT remainingBatTime = CurrentBatterySetEnergy * BatteryDuration;

	if (remainingBatTime > NVGlitchTimeThreshold)
	{
		NVGlitch.CurrentLevel = 1.0f;
		NVGlitch.TargetLevel = 1.0f;
		NVGlitch.NextGlitchTime = 0.0f;
		NVGlitch.bGlitching = FALSE;
	}
	else
	{
		// We're in the free buzz zone

		FLOAT interpFactor = remainingBatTime/NVGlitchTimeThreshold; // to change behavior as we get closer to the end

		UBOOL firstGlitch = appIsNearlyZero(NVGlitch.NextGlitchTime, KINDA_SMALL_NUMBERF);
		UBOOL lastGlitch = (!NVGlitch.bGlitching || NVGlitch.GlitchType != NVGT_LastBreath) && (remainingBatTime < LastBreathDuration);
		UBOOL startNewGlitch = firstGlitch || lastGlitch || (GWorld->GetTimeSeconds() > NVGlitch.NextGlitchTime);

		if (startNewGlitch)
		{
			NVGlitch.StartTime = GWorld->GetTimeSeconds();
			NVGlitch.bGlitching = TRUE;

			if (lastGlitch)
			{
				NVGlitch.GlitchType = NVGT_LastBreath;
				NVGlitch.Duration = LastBreathDuration;
				NVGlitch.TargetLevel = 0.0f;
			}
			else
			{
				NVGlitch.GlitchType = (NVGlitchType)RandHelper(3); 

				if (NVGlitch.GlitchType == NVGT_Buzz)
				{
					NVGlitch.Duration = BuzzGlitchDuration;
				}
				else
				{
					NVGlitch.Duration = Lerp(NVGlitchMaxDuration, NVGlitchMinDuration, interpFactor);
				}

				NVGlitch.TargetLevel = Lerp(0.0f, NVGlitchMaxLevel, interpFactor);

				// Prepare the next one
				FLOAT maxDelay = Lerp(NVGlitchMaxDelayEnd, NVGlitchMaxDelayStart, interpFactor);
				FLOAT delay = RandRange(0.0f, maxDelay);
				NVGlitch.NextGlitchTime = NVGlitch.StartTime + NVGlitch.Duration + delay;
			}

			if (!bPlayingNVGlitchSound)
			{
				TriggerSoundEvent(SndLowBatteryStart);
				bPlayingNVGlitchSound = TRUE;
			}
		}

		if (NVGlitch.bGlitching)
		{
			FLOAT elapsedTime = GWorld->GetTimeSeconds() - NVGlitch.StartTime;
			FLOAT alpha = Saturate(elapsedTime / NVGlitch.Duration);

			if (alpha >= 1.0f)
			{
				// Done
				NVGlitch.bGlitching = FALSE;
				NVGlitch.CurrentLevel = 1.0f;
			}
			else
			{
				FLOAT intensity = 1.0f;

				switch (NVGlitch.GlitchType)
				{
				case NVGT_SuddenDrop:
					intensity = 1.0f - (1.0f - alpha)*(1.0f - alpha);
					break;
				case NVGT_SlowDrop:
					intensity = 0.5f + 0.5f*appCos(alpha * 2.0f * PI);
					break;
				case NVGT_Buzz:
					intensity = 0.5f + 0.5f*appCos(alpha * 2.0f * PI * (FLOAT)NbBuzzGlitchCycles);
					break;
				case NVGT_LastBreath:
					intensity = alpha*(0.5f + 0.5f*appCos(alpha * 2.0f * PI * (FLOAT)LastBreathCycles));
					break;
				}

				NVGlitch.CurrentLevel = NVGlitch.TargetLevel + intensity*(1.0f-NVGlitch.TargetLevel);
			}
		}
	}

	if (bPlayingNVGlitchSound)
	{
		SetAudioValue(RTPCBatteryIntensity, NVGlitch.CurrentLevel*100.0f);
	}
	else
	{
		SetAudioValue(RTPCBatteryIntensity, 100.0f);
	}
}

void AOLHero::ActivateRainEffect()
{
	bRainEffectDesired = TRUE;
}

void AOLHero::DeactivateRainEffect()
{
	bRainEffectDesired = FALSE;
}

UBOOL AOLHero::IsRecordingMarkerInSight(AOLRecordingMarker* recordingMarker)
{
	TWEAKABLE FLOAT NormalizedMinDist = 1000.0f; // for a 1m radius objet with 90 fov

	FVector toMarker = recordingMarker->Location - EyeLocation;
	FVector toMarkerDir;
	FLOAT distToMarker;
	toMarker.ToDirectionAndLength(toMarkerDir, distToMarker);

	if ((toMarkerDir | EyeForward) < 0.0f)
	{
		// behind us
		return FALSE;
	}

	// Distance check, taking zoom into account
	FLOAT minMarkerDist = NormalizedMinDist * 0.01f * recordingMarker->Radius / appTan(0.5f * CurrentFOV * DEG_TO_RAD); // trig for 1m normalized dist at 90 fov
	if (distToMarker > minMarkerDist)
	{
		// Too far / not zoomed in enough
		return FALSE;
	}

	// Angle check
	FLOAT distToEyeLine = PointDistToLine(recordingMarker->Location, EyeForward, EyeLocation);
	if (distToEyeLine > recordingMarker->Radius)
	{
		// Not aiming within the radius
		return FALSE;
	}

	// Check LoS
	FCheckResult Hit(1.f);
	if (!GWorld->SingleLineCheck(Hit, this, recordingMarker->Location, EyeLocation, TRACE_AllBlocking | TRACE_ComplexCollision | TRACE_StopAtAnyHit | TRACE_AISight, FVector(0.0f)))
	{
		// No LoS
		return FALSE;
	}

	return TRUE;
}

void AOLHero::UpdateRecording(FLOAT deltaTime)
{
	for (INT i = 0; i < CachedRecordingMarkers.Num(); i++)
	{
		AOLRecordingMarker* recordingMarker = CachedRecordingMarkers(i);
		if (recordingMarker && recordingMarker->IsValid())
		{
			if (!IsCamcorderActive() || !IsRecordingMarkerInSight(recordingMarker))
			{
				if (recordingMarker->bRecording)
				{
					recordingMarker->AccumulatedRecordingTime += (GWorld->GetTimeSeconds() - recordingMarker->StartedRecordingTime);
				}

				recordingMarker->bRecording = FALSE;
			}
			else
			{
				// valid for recording
				if (recordingMarker->bRecording)
				{
					UBOOL bDone = FALSE;
					FLOAT thisStretchDuration = (GWorld->GetTimeSeconds() - recordingMarker->StartedRecordingTime);

					if (recordingMarker->bAllowNonContinuousRecording)
					{
						bDone = (recordingMarker->AccumulatedRecordingTime + thisStretchDuration) > recordingMarker->MinRecordingDuration;
					}
					else
					{
						bDone = thisStretchDuration > recordingMarker->MinRecordingDuration;
					}

					if (bDone)
					{
						// Done!
						OLPC->RecordingCompleted(recordingMarker);
					}
				}
				else
				{
					recordingMarker->bRecording = TRUE;
					recordingMarker->StartedRecordingTime = GWorld->GetTimeSeconds();
				}
			}
		}
	}
}

UBOOL AOLHero::CanReloadBatteries() const
{
	return	(OLPC->NumBatteries > 0 && !IsInCamcorderTransition() && !bBothHandsNeeded && OLPC->bHasCamcorder && 
				(LocomotionMode == LM_Walk || LocomotionMode == LM_Squeeze || LocomotionMode == LM_Locker || LocomotionMode == LM_LedgeWalk || LocomotionMode == LM_Bed));
}

void AOLHero::ReloadBatteries()
{
	if (!appIsNearlyEqual(CurrentBatterySetEnergy, 1.0f, KINDA_SMALL_NUMBERF) && CanReloadBatteries())
	{
		UBOOL bCamcorderActive = IsCamcorderActive();
		if (bCamcorderActive)
		{
			DeactivateCamcorder();
		}
		else if (LocomotionMode != LM_Squeeze && LocomotionMode != LM_Bed)
		{
			SetBodySetup(HBS_CamcorderVisible);
		}

		CamcorderState = bCamcorderActive ? CCS_ReloadingActive : CCS_ReloadingInactive;
		LastCamcorderSwitchTime = GWorld->GetTimeSeconds();

		if (LocomotionMode == LM_Squeeze)
		{
			StartSpecialMove(SMT_SqueezeReload);
		}
		else if (LocomotionMode == LM_Bed)
		{
			StartSpecialMove(SMT_BedReload);
		}
		else if (bCamcorderActive)
		{
			PlayCamSpaceAnim(bIsCrouched ? AnimNameReloadBatteriesCrouched : AnimNameReloadBatteries, 1.0f, 0.1f, 0.0f);
		}
		else
		{
			PlayCamSpaceAnim(bIsCrouched ? AnimNameReloadBatteriesCrouchedInactive : AnimNameReloadBatteriesInactive, 1.0f, 0.1f, 0.1f);
		}
	}
}

void AOLHero::SetCamcorderVisibleNotify()
{
	if (CamcorderState == CCS_Raising || CamcorderState == CCS_ReloadingInactive)
	{
		SetBodySetup(HBS_CamcorderVisible);
	}
	else
	{
		debugf(TEXT("### - Ignoring SetCamcorderVisibleNotify()"));
	}
}

void AOLHero::BatteriesReloadedNotify()
{
	if (IsReloading())
	{
		OLPC->NumBatteries = Max(0, OLPC->NumBatteries - 1);
		CurrentBatterySetEnergy = 1.0f;
		NVGlitch.NextGlitchTime = 0.0f; 		

		if (CamcorderMode == CCM_NightVision)
		{
			CamcorderMode = CCM_PoweredNightVision;
		}

		if (!OLPC->bReloadedBatteries)
		{
			debugf(TEXT("## First battery reload"));
			OLPC->bReloadedBatteries = TRUE;
		}
	}
}

void AOLHero::CancelReload()
{
	if (LocomotionMode == LM_Walk)
	{
		// reset collision
		TryAdjustCollisionSizeForLocomotionMode((ELocomotionMode)LocomotionMode);
	}

	if (SpecialMove == SMT_SqueezeReload || SpecialMove == SMT_BedReload)
	{
		CancelSpecialMove();
	}
	else
	{
		if (UpperBodyBlendNode->bActive)
		{
			UpperBodyBlendNode->StartBlendingOut();
			ShadowProxyUpperBodyBlendNode->StartBlendingOut();
		}
	}
	CamcorderState = CCS_Inactive;
	SetBodySetup(HBS_Normal);
}

////////////////////////////////////////////////////////////////////////////////////////////
// Objects
////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////

AOLDoor* AOLHero::FindTargetDoor()
{
	FLOAT bestScore = -1.0f;
	AOLDoor* door = NULL;

	TWEAKABLE FLOAT MaxZDelta = 7.0f;

	for (INT doorIdx = 0; doorIdx < CachedDoors.Num(); doorIdx++)
	{
		AOLDoor* doorCandidate = CachedDoors(doorIdx);
	
		if (doorCandidate && !doorCandidate->IsBroken() && !doorCandidate->IsPendingKill())
		{
			FVector toDoorKnob = doorCandidate->GetKnobLocation() - EyeLocation;
			FLOAT interactDistSq = (doorCandidate->IsOpened() ? Square(DoorCloseInteractionDist) : Square(DoorOpenInteractionDist));
			FLOAT distSq = toDoorKnob.SizeSquared();
			if (distSq > interactDistSq)
			{
				continue; // not close enough
			}

			if (Abs(doorCandidate->GetCenterLocation().Z - Location.Z) > MaxZDelta)
			{
				continue; // bad Z 
			}

			// Check that we're looking at the door (or close to)
			FVector pivot = Vec2D(doorCandidate->GetPivotLocation());
			FVector edge = Vec2D(doorCandidate->GetEdgeLocation());
			FVector pivotToEdge = edge - pivot;
			FLOAT doorWidth = pivotToEdge.Size2D();
			FVector pivotToEdgeDir = pivotToEdge.SafeNormal2D();
			FVector eye2d = Vec2D(EyeLocation);
			FVector eyeFwd2d = EyeForward.SafeNormal2D();
			FLOAT sideOffset = Max(0.5f*doorWidth, 50.0f);

			FVector pt1;
			FVector pt2;
			SegmentDistToSegment(pivot - sideOffset*pivotToEdgeDir, edge + sideOffset*pivotToEdgeDir, eye2d, eye2d + 1000.0f*eyeFwd2d, pt1, pt2);

			FLOAT cosAngleViewToDoorDir = (pivotToEdgeDir | eyeFwd2d);
			if (!FPointsAreNear(pt1, pt2, 1.0f))
			{
				// we may be looking an open door edge-on; allow more room in that case
				if (!doorCandidate->IsOpened() || (cosAngleViewToDoorDir > -0.806f) || !FPointsAreNear(pt1, pt2, 25.0f))
				{
					// not looking towards the door
					continue;
				}				
			}

			FLOAT playerDistToPivot = (Location - pivot) | pivotToEdgeDir;
			if ((playerDistToPivot < 45.0f && cosAngleViewToDoorDir < -0.707f) || (playerDistToPivot < 10.0f && cosAngleViewToDoorDir < 0.0f))
			{
				// looking almost along the door, past the pivot but from near it
				continue;
			}

			FLOAT playerDistToEdge = (Location - edge) | pivotToEdgeDir;
			if (playerDistToEdge > 0.0f && cosAngleViewToDoorDir > 0.0f)
			{
				// we're past the edge and looking away from the door
				continue;
			}

			FLOAT distToPivot = pt1.Distance(pivot);
			FLOAT distToEdge = pt1.Distance(edge);
			FLOAT aimingError = Max(0.0f, Max(distToEdge, distToPivot) - doorWidth);
			FLOAT score = aimingError + eye2d.Distance(pt2); // total travel distance from eye to closest door edge, following the eye forward vector

			if (door && (score >= bestScore))
			{
				continue; // we already have a better door
			}

			if (doorCandidate->bBlocked)
			{
				if (((doorCandidate->GetCenterLocation() - EyeLocation).SafeNormal2D() | doorCandidate->GetStaticDirection()) < 0.0f)
				{
					// we can only try pushing a blocked door - we can't pull them (don't interfere with the push interaction)
					continue;
				}
			}

			FCheckResult Hit(1.f);
			FVector startTrace = EyeLocation;
			FVector endTrace = doorCandidate->GetKnobLocation() - 10.0f*toDoorKnob.ProjectOnTo(doorCandidate->GetDynamicDirection()).SafeNormal2D();

			// check that we have LoS to the door knob
			if (!GWorld->SingleLineCheck( Hit, this, endTrace, startTrace, TRACE_AllBlocking | TRACE_StopAtAnyHit, FVector(0.0f)))
			{
				// no LoS to the knob - can't interact with this door
				continue;
			}

			// Check that we have unobstructed access to the door
			FLOAT traceWidth = 20.0f;
			startTrace = Location + VecZ(100.0f);
			FVector toCenterPerp = (doorCandidate->GetCenterLocation() - Location).ProjectOnTo(doorCandidate->GetStaticDirection()).SafeNormal2D();
			endTrace = doorCandidate->GetCenterLocation() + VecZ(100.0f) - 2.0f*traceWidth*toCenterPerp;
			if (!GWorld->SingleLineCheck( Hit, this, endTrace, startTrace, TRACE_AllBlocking, FVector(traceWidth, traceWidth, 50.0f)))
			{
				if (Hit.Actor != doorCandidate)
				{
					continue; // Something's in the way
				}
			}

			// Got a valid door. Keep looking in case there's a better one
			door = doorCandidate;
			bestScore = score;
		}		
	}

	if (door && door->bLocked)
	{
		if (door->bNoLockedInteraction)
		{
			// no interaction allowed
			return NULL;
		}

		if (door->UnlockingCSA && door->UnlockingCSA->RequiredItem != NAME_None && OLPC->InventoryManager->OwnsItem(door->UnlockingCSA->RequiredItem))
		{
			// we can activate a CSA that would unlock this door - let it do its job without the locked interaction interfering
			return NULL;
		}
	}

	return door;
}

UBOOL AOLHero::TryDoorInstantInteraction(UBOOL playerInteraction, const FVector& playerIntentDirection)
{
	check(ActiveDoor == NULL);
	AOLDoor* door = FindTargetDoor();		

	if (!door)
	{
		return FALSE;
	}

	if (door->DoorState != DS_Idle)
	{
		// Must be idle
		return FALSE;
	}

	if (door->DoorUser != NULL && door->DoorUser->IsA(AOLEnemyPawn::StaticClass()))
	{
		// Not a good idea
		return FALSE;
	}

	if (door->DoorType == DT_Locker)
	{
		return FALSE;
	}

	UBOOL closing = door->IsOpened() && !door->IsPartiallyOpened();

	if ((closing && (!CouldStartSpecialMove(SMT_CloseDoor) || !door->CanClose(this))) || (!closing && !CouldStartSpecialMove(SMT_OpenDoorInstant)))
	{
		return FALSE;
	}

	FVector toCenter = door->GetCenterLocation() - Location;
	FVector pivotToEdge = (door->GetEdgeLocation() - door->GetPivotLocation());
	FVector doorDirection = door->GetStaticDirection(); // from the door frame

	UBOOL bClosePositionned = FALSE;

	// from inside : on the other side of the door frame (not the side that opens) or within the door frame's half-circle
	UBOOL closingFromInside = FALSE; 	
	if (closing)
	{
		UBOOL playerInside = ((toCenter | doorDirection) > 0.0f); // on the other side

		if (playerInside)
		{
			closingFromInside = TRUE;
		}
		else
		{
			FLOAT projVel = Velocity | doorDirection;
			TWEAKABLE FLOAT VelThreshForLimitChange = 150.0f;
			TWEAKABLE FLOAT CloseFromInsideDistanceGoingIn = 0.0f;
			TWEAKABLE FLOAT CloseFromInsideDistanceGoingOut = 100.0f;

			if (Abs(projVel) < VelThreshForLimitChange)
			{
				if (toCenter.SizeSquared2D() < (0.5f*pivotToEdge).SizeSquared2D()) // half-circle around the center
				{
					closingFromInside = TRUE;
					bClosePositionned = TRUE;
				}
			}
			else
			{
				FLOAT testDist = projVel > 0.0f ? CloseFromInsideDistanceGoingIn : CloseFromInsideDistanceGoingOut;
				FLOAT projDist = -(toCenter | doorDirection);

				if (projDist < testDist)
				{
					closingFromInside = TRUE;
					bClosePositionned = TRUE;
				}
			}
		}
	}
	
	if (closingFromInside && (!appIsNearlyEqual(door->GetOpenAngle(), door->PlayerOpenedAngle, 1.0f) || door->bNoPushKnob))
	{
		return FALSE; // we can't close from inside a door that we didn't fully open (the anim won't allow it), or that doesn't have a push knob (e.g. prison door)
	}

	FVector knobLocation = door->GetKnobLocation();
	knobLocation.Z = Location.Z;
	FVector toDoorKnob = Vec2D(knobLocation - Location);
	FVector pivotToEdgeDir = pivotToEdge.SafeNormal2D();
	FVector closingDirection = -door->GetDynamicDirection();
	FVector charLeft = (toDoorKnob.SafeNormal2D() ^ FVector(0,0,1.0f));
	UBOOL pivotOnLeft = (pivotToEdgeDir | charLeft) < 0.0f;
	UBOOL pushDoor = (doorDirection | toDoorKnob) > 0.0f;

	UBOOL bRunThroughOpen = bWantToRun && IsRunning() && !closing && pushDoor && door->IsPartiallyOpened();
	if (bRunThroughOpen)
	{
		// more checks!
		FVector toCenter = door->GetCenterLocation() - Location;
		FVector toCenterDir = toCenter.SafeNormal2D();
		UBOOL bCloseEnough = (toCenter.SizeSquared2D() < Square(DoorRunThroughInteractDist));
		UBOOL bInFront = ((toCenter.SafeNormal2D() | doorDirection) > 0.707f);
		UBOOL bGoingTowardsDoor = (RealVelocity.SafeNormal2D() | toCenterDir) > 0.707f;

		bRunThroughOpen = bCloseEnough && bInFront && bGoingTowardsDoor;
	}
		
	if (!playerInteraction && !bRunThroughOpen)
	{
		EPlayerInteractionType interactionType = PIT_OpenDoor;

		if (closing)
		{
			interactionType = PIT_CloseDoor;
		}
		else if ((door->bLocked || door->bBlocked) && !door->bFakeUnlocked)
		{
			interactionType = PIT_LockedDoor;
		}
		else if (door->bAutoClose)
		{
			interactionType = PIT_AutoCloseDoor;
		}
		else if (door->GetOpenAngle() > DoorMaxAngleForInteractiveOpen)
		{
			interactionType = PIT_OpenPartiallyOpenDoor;
		}
		OLPC->AddAvailableInteraction(interactionType);
		return FALSE;
	}

	if (bIsCrouched) 
	{
		if (!TryUncrouch())
		{
			return FALSE;
		}
	}

	// At this point we can commit to a move

	FVector expectedAnimStart = Location;
	FVector expectedAnimFwd = CharForward;

	if (closing)
	{
		// Closing the door

		if (!TryCommitToSpecialMove(SMT_CloseDoor, door))
		{
			return FALSE;
		}

		ActiveDoor = door;
		ActiveDoor->DoorUser = this;
		door->SetCollisionType(COLLIDE_NoCollision);

		UBOOL closingFromSide = ((toDoorKnob.SafeNormal2D() | -pivotToEdgeDir) > 0.707f);

		bQuietDoorInteraction = !IsBeingChased();

		if (closingFromInside)
		{
			UBOOL closesOnRight = (pivotToEdgeDir | charLeft) < 0.0f;
			expectedAnimFwd = door->GetStaticDirection();
			expectedAnimStart = door->GetCenterLocation();
			expectedAnimStart.Z = Location.Z;

			DoorClosingType = closesOnRight ? DCT_LeftInside : DCT_RightInside;
		}
		else if (closingFromSide)
		{
			UBOOL closesOnRight = (closingDirection | charLeft) < 0.0f;

			if (bQuietDoorInteraction)
			{
				// correct for a 5deg difference in the animations (they were animated supposing a door closed at 90degs, instead of 95)
				FVector animatedKnobLocation = knobLocation + 8.28f * closingDirection - 0.36f * pivotToEdgeDir; // location where a 90deg door knob would be
				FVector toAnimatedDoorKnob = Vec2D(animatedKnobLocation - Location); // fake the knob location
				FVector adjustedPivotToEdgeDir = pivotToEdgeDir.RotateAngleAxis(5.0f * DEG_TO_UNR * (closesOnRight ? -1.0f : 1.0f), VecZ(1.0f));

				expectedAnimStart = Location + toAnimatedDoorKnob + DoorCloseExpectedDistFwd*adjustedPivotToEdgeDir;
				expectedAnimFwd = -adjustedPivotToEdgeDir;
			}
			else
			{
				expectedAnimStart = Location + toDoorKnob + DoorCloseExpectedDistFwd*pivotToEdgeDir;
				expectedAnimFwd = -pivotToEdgeDir;
			}

			DoorClosingType = closesOnRight ? DCT_RightSide : DCT_LeftSide;
		}
		else
		{
			UBOOL pushToClose = (toDoorKnob | closingDirection) > 0.0f;
			UBOOL closesOnRight = (pivotToEdgeDir | charLeft) < 0.0f;	

			if (bQuietDoorInteraction)
			{
				// correct for a 5deg difference in the animations (they were animated supposing a door closed at 90degs, instead of 95)
				FVector animatedKnobLocation = knobLocation + 8.28f * closingDirection - 0.36f * pivotToEdgeDir; // location where a 90deg door knob would be
				FVector toAnimatedDoorKnob = Vec2D(animatedKnobLocation - Location); // fake the knob location
				UBOOL negativeAngle = (pushToClose == closesOnRight);
				FVector adjustedClosingDirection = closingDirection.RotateAngleAxis(5.0f * DEG_TO_UNR * (negativeAngle ? -1.0f : 1.0f), VecZ(1.0f));

				expectedAnimFwd = toAnimatedDoorKnob.ProjectOnTo(adjustedClosingDirection).SafeNormal2D();
				expectedAnimStart = Location + toAnimatedDoorKnob - DoorCloseExpectedDistFwd*expectedAnimFwd;
			}
			else
			{
				expectedAnimFwd = toDoorKnob.ProjectOnTo(closingDirection).SafeNormal2D();
				expectedAnimStart = Location + toDoorKnob - DoorCloseExpectedDistFwd*expectedAnimFwd;
			}
					
			if (pushToClose)
			{
				DoorClosingType = closesOnRight ? DCT_RightFront : DCT_LeftFront;
			}
			else
			{
				DoorClosingType = closesOnRight ? DCT_LeftBack : DCT_RightBack;
			}
		}

		door->TriggerEvent(DET_StartedClosing, this);
		door->NotifyHandlesToWait();

		StartSpecialMove(bClosePositionned ? SMT_CloseDoorPositionned : SMT_CloseDoor, expectedAnimStart, expectedAnimFwd, APTT_TargetAtStart);
	}
	else 
	{	
		if (bRunThroughOpen)
		{
			if (!TryCommitToSpecialMove(SMT_RunThroughDoor, door))
			{
				return FALSE;
			}

			ActiveDoor = door;
			ActiveDoor->DoorUser = this;
			door->SetCollisionType(COLLIDE_NoCollision);

			expectedAnimStart = door->GetCenterLocation() - DoorRunThroughExpectedDist * doorDirection;
			expectedAnimFwd = doorDirection;

			UBOOL opensOnRight = (pivotToEdgeDir | charLeft) > 0.0f;
			DoorPartialOpenType = opensOnRight ? DPOT_RightPush : DPOT_LeftPush;

			StartSpecialMove(SMT_RunThroughDoor, expectedAnimStart, expectedAnimFwd, APTT_TargetAtStart);
		}
		else if (door->IsPartiallyOpened() || (pushDoor && door->bNoPushKnob))
		{
			if (!TryCommitToSpecialMove(SMT_OpenDoorPartial, door))
			{
				return FALSE;
			}

			ActiveDoor = door;
			ActiveDoor->DoorUser = this;
			door->SetCollisionType(COLLIDE_NoCollision);

			// Opening the door (instant) from partially open, or a door we can push without holding the handle

			UBOOL fromSide = (door->GetOpenAngle() > 25.0f) && (toDoorKnob.SafeNormal2D() | -pivotToEdgeDir) > 0.5f; // within 60 degs angle of the edge, and door open at least at 25 degs (otherwise it creates collision issues)
			
			if (fromSide)
			{
				UBOOL opensOnRight = (closingDirection | charLeft) > 0.0f;
				DoorPartialOpenType = opensOnRight ? DPOT_RightSwipe : DPOT_LeftSwipe;
				expectedAnimStart = Location + toDoorKnob + DoorCloseExpectedDistFwd*pivotToEdgeDir;
				expectedAnimFwd = -pivotToEdgeDir;
			}
			else
			{
				UBOOL onInside = (toCenter | doorDirection) < 0.0f;
				pushDoor = pushDoor && (!onInside || door->GetOpenAngle() > 25.0f); // on the pushing side, but not if we'd be pulling a slightly open door from the side
				
				if (pushDoor)
				{
					UBOOL opensOnRight = (pivotToEdgeDir | charLeft) > 0.0f;
					DoorPartialOpenType = opensOnRight ? DPOT_RightPush : DPOT_LeftPush;

					expectedAnimFwd = doorDirection;
					expectedAnimStart = door->GetCenterLocation() - DoorOpenInsideExpectedDistFwd*expectedAnimFwd;
					expectedAnimStart.Z = Location.Z;
				}
				else
				{
					pivotOnLeft = (pivotToEdgeDir ^ closingDirection).Z < 0.0f;
					DoorPartialOpenType = pivotOnLeft ? DPOT_LeftPull : DPOT_RightPull;

					expectedAnimFwd = closingDirection;
					expectedAnimStart = knobLocation - DoorOpenExpectedSideDist * pivotToEdgeDir - DoorOpenExpectedFwdDist*expectedAnimFwd;
				}
			}

			StartSpecialMove(SMT_OpenDoorPartial, expectedAnimStart, expectedAnimFwd, APTT_TargetAtStart);
		}
		else
		{
			ESpecialMoveType moveType = (door->bLocked || door->bBlocked) ? SMT_TryOpenLockedDoor : SMT_OpenDoorInstant;

			if (!TryCommitToSpecialMove(moveType, door))
			{
				return FALSE;
			}

			ActiveDoor = door;
			ActiveDoor->DoorUser = this;

			if (moveType != SMT_TryOpenLockedDoor)
			{
				door->SetCollisionType(COLLIDE_NoCollision);
			}

			// Opening the door (instant) from closed			
			FVector traversalDir = pushDoor ? doorDirection : -doorDirection;

			if (pivotOnLeft)
			{
				DoorOpeningType = (pushDoor ? DOT_LeftPush : DOT_LeftPull);
			}
			else
			{
				DoorOpeningType = (pushDoor ? DOT_RightPush : DOT_RightPull);				
			}
		
			expectedAnimStart = knobLocation - DoorOpenExpectedSideDist * pivotToEdgeDir - DoorOpenExpectedFwdDist*traversalDir;
			expectedAnimFwd = traversalDir;

			StartSpecialMove(moveType, expectedAnimStart, expectedAnimFwd, APTT_TargetAtStart);	
		}
	}

	return TRUE;
}

UBOOL AOLHero::TryDoorInteractiveOpen()
{
	check(ActiveDoor == NULL);
	AOLDoor* door = FindTargetDoor();		

	if (!door)
	{
		return FALSE;
	}

	if (door->DoorType != DT_Normal)
	{
		// only normal doors
		return FALSE;
	}

	if (door->DoorUser != NULL && door->DoorUser->IsA(AOLEnemyPawn::StaticClass()) && !door->DoorUser->IsPendingKill())
	{
		// Not a good idea
		return FALSE;
	}

	if (door->GetOpenAngle() > DoorMaxAngleForInteractiveOpen)
	{
		// must be closed or very slightly open
		return FALSE;
	}

	if (door->DoorState != DS_Idle)
	{
		// Must be idle
		return FALSE;
	}

	if (bIsCrouched) 
	{
		if (!TryUncrouch())
		{
			return FALSE;
		}
	}
	
	if (!CouldStartSpecialMove(SMT_EnterDoorInteraction))
	{
		return FALSE;
	}

	ESpecialMoveType moveType = (door->bLocked || door->bBlocked) ? SMT_TryOpenLockedDoor : SMT_OpenDoorInstant;
	if (!TryCommitToSpecialMove(moveType, door))
	{
		return FALSE;
	}

	ActiveDoor = door;
	ActiveDoor->DoorUser = this;
	door->SetCollisionType(COLLIDE_NoCollision);

	// Open the door, with the scrubbing interaction

	ActiveDoor = door;
	ActiveDoor->DoorUser = this;

	FVector knobLocation = door->GetStaticKnobLocation();
	knobLocation.Z = Location.Z;
	FVector toDoorKnob = Vec2D(knobLocation - Location);
	FVector pivotToEdge = door->GetStaticPivotToEdge();
	FVector charLeft = (toDoorKnob.SafeNormal2D() ^ FVector(0,0,1.0f));	
	FVector doorDirection = door->GetStaticDirection(); 
	UBOOL pushDoor = (doorDirection | toDoorKnob) > 0.0f;
	UBOOL pivotOnLeft = (pivotToEdge | charLeft) < 0.0f;
	FVector traversalDir = pushDoor ? doorDirection : -doorDirection;

	if (door->bLocked || door->bBlocked)
	{
		// For a locked door, use the open instant move
		DoorOpeningType = (pivotOnLeft ? DOT_LeftPull : DOT_RightPull); // pull is fine (whichever)
		FVector expectedAnimStart = knobLocation - DoorOpenExpectedSideDist * pivotToEdge.SafeNormal2D() - DoorOpenExpectedFwdDist*traversalDir;
		FVector expectedAnimFwd = traversalDir;
		StartSpecialMove(SMT_TryOpenLockedDoor, expectedAnimStart, expectedAnimFwd, APTT_TargetAtStart);	
		return TRUE;
	}

	if (pivotOnLeft)
	{
		DoorOpeningType = (pushDoor ? DOT_LeftPush : DOT_LeftPull);
	}
	else
	{
		DoorOpeningType = (pushDoor ? DOT_RightPush : DOT_RightPull);				
	}

	FVector expectedAnimStart = knobLocation - DoorOpenExpectedSideDist * pivotToEdge - DoorOpenExpectedFwdDist*traversalDir;
	FVector expectedAnimFwd = traversalDir;

	check(DoorAnimNode);
	DoorAnimNode->SetActiveChild(DoorOpeningType, 0.0f);
	
	TWEAKABLE FLOAT OpenDoorRate = 0.5f;
	DoorAnimNode->PlayRate = OpenDoorRate;
	DoorAnimNode->MaxRatio = (door->MaxOpenAngle < DoorInteractiveOpenMaxAngle) ? (door->MaxOpenAngle / DoorInteractiveOpenMaxAngle) : 1.0f;
	FLOAT initialDoorRatio = door->GetOpenAngle() / DoorInteractiveOpenMaxAngle;
	DoorAnimNode->InitialRatio = initialDoorRatio; 
	DoorAnimNode->CurrentRatio = initialDoorRatio;

	TWEAKABLE FLOAT MinOpenRatio = 0.03f;
	DoorInteractionStartingRatio = MinOpenRatio;
	bDoorStartingRatioReached = FALSE;
				
	StartSpecialMove(SMT_EnterDoorInteraction, expectedAnimStart, expectedAnimFwd, APTT_TargetAtStart);
	
	return TRUE;
}

void AOLHero::StopInteractiveOpen()
{
	check(LocomotionMode == LM_Door);
	check(ActiveDoor);

	LastCompletedDoorInteractionTime = GWorld->GetTimeSeconds();

	EnterLocomotionMode(LM_Walk);
	
	UBOOL closingDoor = (ActiveDoor->TargetOpenRatio < ActiveDoor->OpenRatio) || ((DesiredMoveDirection | ActiveDoor->GetStaticDirection()) < 0.0f);
	
	if (closingDoor || (ActiveDoor->bAutoClose && ActiveDoor->GetOpenAngle() < 45.0f))
	{
		// close but don't pass
		TWEAKABLE FLOAT RotationSpeed = 100.0f;
		ActiveDoor->Close(this, RotationSpeed);
		ActiveDoor->SetCollisionType(COLLIDE_BlockAll); // make sure we are colliding once done		
		
		if (DoorOpeningType == DOT_LeftPush || DoorOpeningType == DOT_RightPush)
		{
			// move back to allow the door to close
			FProceduralAnimData animData;
			TWEAKABLE FLOAT BackDistance = 100.0f;
			FVector desiredLoc = ActiveDoor->GetCenterLocation() - BackDistance * ActiveDoor->GetStaticDirection();
			animData.PositionDelta = desiredLoc - Location;
			QueueProceduralAnim(animData);

			StartSpecialMove(SMT_ClearClosingDoor);
		}
	}
	else if (ActiveDoor->bAutoClose)
	{
		// close behind the player
		TWEAKABLE FLOAT RotationSpeed = 100.0f;
		ActiveDoor->Close(this, RotationSpeed);
		ActiveDoor->SetCollisionType(COLLIDE_BlockAll); // make sure we are colliding once done

		if (DoorOpeningType == DOT_LeftPush || DoorOpeningType == DOT_RightPush)
		{
			// move sideways to allow the door to close behind us
			FVector toEdge = (ActiveDoor->GetCenterLocation() - ActiveDoor->GetPivotLocation()).SafeNormal2D();
			FProceduralAnimData animData;
			TWEAKABLE FLOAT SideDistance = 40.0f;
			TWEAKABLE FLOAT FwdDistance = 100.0f;
			FVector desiredLoc = ActiveDoor->GetCenterLocation() + FwdDistance * ActiveDoor->GetStaticDirection() + SideDistance * toEdge;
			animData.PositionDelta = desiredLoc - Location;
			QueueProceduralAnim(animData);

			StartSpecialMove(SMT_ClearClosingDoor);
		}
	}
	else
	{
		// open
		TWEAKABLE FLOAT RotationSpeed = 100.0f;
		ActiveDoor->Open(this, RotationSpeed);
		ActiveDoor->SetCollisionType(COLLIDE_BlockAll); // make sure we are colliding once done
	}

	ActiveDoor->DoorUser = NULL;
	ActiveDoor = NULL;
}

UBOOL AOLHero::TryOpenAndEnterLocker(UBOOL playerInteraction)
{
	check(ActiveDoor == NULL);
	AOLDoor* door = FindTargetDoor();

	if (!door)
	{
		return FALSE;
	}

	if (door->DoorType != DT_Locker)
	{
		return FALSE;
	}

	if (door->DoorState != DS_Idle)
	{
		// Must be idle
		return FALSE;
	}

	if (!door->IsClosed())
	{
		// Must be closed
		return FALSE;
	}

	if (!playerInteraction)
	{
		OLPC->AddAvailableInteraction(PIT_EnterLocker);
		return FALSE;
	}

	AOLHidingSpot* hidingSpot = NULL;

	for (INT i = 0; i < CachedHidingSpots.Num(); i++)
	{
		AOLHidingSpot* testHidingSpot = CachedHidingSpots(i);

		if (testHidingSpot && testHidingSpot->IsValid() && testHidingSpot->AssociatedDoor == door)
		{
			hidingSpot = testHidingSpot;
			break;
		}
	}
	
	if (hidingSpot)
	{
		if (!CouldStartSpecialMove(SMT_EnterLocker))
		{
			return FALSE;
		}

		if (!TryCommitToSpecialMove(SMT_EnterLocker, hidingSpot))
		{
			return FALSE;
		}

		ActiveDoor = door;
		ActiveDoor->DoorUser = this;
		door->SetCollisionType(COLLIDE_NoCollision);
		
		// no obstruction check: a locker with a valid hiding spot must be clear (no trash or shelves)

		FVector doorDirection = door->GetStaticDirection(); // points outside
		FVector pivot = door->GetPivotLocation();
		FVector pivotToEdgeDir = (VecZ(1.0f) ^ doorDirection).SafeNormal2D();

		FVector expectedAnimStart = pivot + LockerEnterExpectedDistSide*pivotToEdgeDir + LockerEnterExpectedDistFwd*doorDirection;
		expectedAnimStart.Z = Location.Z;
		FVector expectedAnimFwd = -doorDirection;

		ActiveDoor = door;
		ActiveDoor->DoorUser = this;
		ActiveLocker = hidingSpot;
		StartSpecialMove(SMT_EnterLocker, expectedAnimStart, expectedAnimFwd, APTT_TargetAtStart);		
	}		
	else 
	{
		if (!CouldStartSpecialMove(SMT_OpenLockerFromOutside))
		{
			return FALSE;
		}

		// No hiding spot inside the locker - we simply open the door

		ActiveDoor = door;
		ActiveDoor->DoorUser = this;

		FVector doorDirection = door->GetStaticDirection(); // from the door frame
		FVector pivot = door->GetPivotLocation();
		FVector pivotToEdgeDir = (door->GetEdgeLocation() - pivot).SafeNormal2D();

		FLOAT angleToDoor = appAcos(CharForward | -doorDirection) * RAD_TO_DEG;
		SpecialMoveBlendAlpha = 1.0f - Saturate(angleToDoor / 45.0f); // 0.0f - 45 degs, 1.0f - straight
		bLeftAnim = (Rotation.Right() | doorDirection) > 0.0f;

		FLOAT sideDist = bLeftAnim ? LockerOpenLeftExpectedDistSide : LockerOpenRightExpectedDistSide;

		FVector expectedAnimStartSide = pivot + sideDist*pivotToEdgeDir + LockerOpenExpectedDistFwd*doorDirection;
		FVector expectedAnimStartStraight = pivot + LockerOpenStraightExpectedDistSide*pivotToEdgeDir + LockerOpenExpectedDistFwd*doorDirection;

		FVector expectedAnimFwdSide = (-doorDirection + (bLeftAnim ? -pivotToEdgeDir : pivotToEdgeDir)).SafeNormal2D();
		FVector expectedAnimFwdStraight = -doorDirection;

		FVector expectedAnimStart = SpecialMoveBlendAlpha*expectedAnimStartStraight + (1.0f - SpecialMoveBlendAlpha)*expectedAnimStartSide;
		FVector expectedAnimFwd = (SpecialMoveBlendAlpha*expectedAnimFwdStraight + (1.0f - SpecialMoveBlendAlpha)*expectedAnimFwdSide).SafeNormal2D();

		TWEAKABLE FLOAT zOffsetFromDoor = 0.0f;
		expectedAnimStart.Z = door->GetCenterLocation().Z + zOffsetFromDoor;
		
		StartSpecialMove(SMT_OpenLockerFromOutside, expectedAnimStart, expectedAnimFwd, APTT_TargetAtStart);
	}	

	return TRUE;	
}

UBOOL AOLHero::TryOpenAndExitLocker(UBOOL playerInteraction)
{
	// Already in the locker

	if (!CouldStartSpecialMove(SMT_ExitLocker))
	{
		return FALSE;
	}

	if (!playerInteraction)
	{
		OLPC->AddAvailableInteraction(PIT_ExitLocker);
	}
	else
	{
		check(ActiveLocker);

		if (!TryCommitToSpecialMove(SMT_ExitLocker, ActiveLocker))
		{
			return FALSE;
		}

		ActiveDoor = ActiveLocker->AssociatedDoor;
		check(ActiveDoor);
		ActiveDoor->DoorUser = this;

		if (!ActiveDoor->IsOpened())
		{
			StartSpecialMove(SMT_ExitLocker);

			return TRUE;
		}
	}
	return FALSE;
}

void AOLHero::UpdateDoorInteraction()
{
	check(DoorAnimNode);
	check(ActiveDoor);

	FLOAT currentRatio = DoorAnimNode->CurrentRatio;
	TWEAKABLE FLOAT ConsideredOpenRatio = 0.6f;
	
	if (!DesiredMoveDirection.IsNearlyZero())
	{
		UBOOL goingToDoorFront = (DesiredMoveDirection | ActiveDoor->GetStaticDirection()) > 0.0f;
		UBOOL goingFwd = goingToDoorFront;

		if (currentRatio == 0.0f)
		{
			if (!goingFwd)
			{
				// leaving
				LastCompletedDoorInteractionTime = GWorld->GetTimeSeconds();
				EnterLocomotionMode(LM_Walk);
				ActiveDoor->Close(this);
				ActiveDoor->SetCollisionType(COLLIDE_BlockAll);
				ActiveDoor->DoorUser = NULL;
				ActiveDoor = NULL;

				return;
			}
		}		
		else if (currentRatio >= ConsideredOpenRatio && goingFwd)
		{
			// done
			EnterLocomotionMode(LM_Walk);
			LastCompletedDoorInteractionTime = GWorld->GetTimeSeconds();

			if (ActiveDoor->bAutoClose)
			{
				TWEAKABLE FLOAT RotationSpeed = 80.0f;
				ActiveDoor->Close(this, RotationSpeed);
				ActiveDoor->SetCollisionType(COLLIDE_BlockAll);

				TWEAKABLE FLOAT SideDistancePull = 0.0f;
				TWEAKABLE FLOAT FwdDistancePull = -100.0f;
				TWEAKABLE FLOAT SideDistancePush = 80.0f;
				TWEAKABLE FLOAT FwdDistancePush = 80.0f;		

				FLOAT sideDist = (DoorOpeningType == DOT_LeftPush || DoorOpeningType == DOT_RightPush) ? SideDistancePush : SideDistancePull;
				FLOAT fwdDist = (DoorOpeningType == DOT_LeftPush || DoorOpeningType == DOT_RightPush) ? FwdDistancePush : FwdDistancePull;				

				FVector toEdge = (ActiveDoor->GetCenterLocation() - ActiveDoor->GetPivotLocation()).SafeNormal2D();
				FProceduralAnimData animData;
				FVector desiredLoc = ActiveDoor->GetCenterLocation() + fwdDist * ActiveDoor->GetStaticDirection() + sideDist * toEdge;
				animData.PositionDelta = desiredLoc - Location;
				QueueProceduralAnim(animData);

				StartSpecialMove(SMT_ClearClosingDoor);
			}
			else
			{
				ActiveDoor->Open(this);
				ActiveDoor->SetCollisionType(COLLIDE_BlockAll);
			}
			ActiveDoor->DoorUser = NULL;
			ActiveDoor = NULL;

			return;
		}
	}
	
	TWEAKABLE FLOAT doorMovementSpeed = 300.0f;
	ActiveDoor->CurrentSpeed = doorMovementSpeed;
	
	ActiveDoor->SetTargetOpenAngle(currentRatio * DoorInteractiveOpenMaxAngle);
}

void AOLHero::UpdateContextualLean(FLOAT leanInput, const FVector& playerIntentVelocity)
{
	UBOOL bPeekingFromLeft = (CornerPeek.PeekPosition == CPP_Left || CornerPeek.PeekPosition == CPP_MiddleLeft);
	FLOAT normalizedInput = bPeekingFromLeft ? leanInput : -leanInput;

	if (LocomotionMode == LM_ContextualLean)
	{
		TWEAKABLE FLOAT InsideTransitionYaw = 20.0f;
		TWEAKABLE FLOAT ExitForwardMinIntentVelocity = 200.0f;

		if (!CornerPeek.CornerMarker)
		{
			StartSpecialMove(SMT_ExitContextualLean);
			return;
		}
	
		if (CornerPeek.CornerMarker->b3Sided)
		{
			// check for an inside transition

			if ((CornerPeek.PeekPosition == CPP_Left && Camera->ViewCS.Yaw < -InsideTransitionYaw) ||
				(CornerPeek.PeekPosition == CPP_Right && Camera->ViewCS.Yaw > InsideTransitionYaw))
			{
				// Transition

				UBOOL bOK = TRUE;

				// Check if we're in a doorframe
				for (INT j = 0; j < CachedDoors.Num(); j++)
				{
					AOLDoor * door = CachedDoors(j);

					if (door && door->DoorBreakState != DBS_Broken && door->GetPivotLocation().DistanceSquared(CornerPeek.CornerMarker->Location) < Square(25.0f))
					{
						// door pivot - can't do it.
						bOK = FALSE;
						break;
					}
				}

				if (bOK)
				{
					if (!TryCommitToSpecialMove(SMT_ContextualLeanInsideTransition, CornerPeek.CornerMarker))
					{
						return;
					}

					FVector wallDir = CornerPeek.CornerMarker->Rotation.Vector();
					FVector frameRight = CornerPeek.CornerMarker->Rotation.Right();
					FVector leftCorner = CornerPeek.CornerMarker->Location - 0.5f * CornerPeek.CornerMarker->WallThickness * frameRight;
					FVector rightCorner = CornerPeek.CornerMarker->Location + 0.5f * CornerPeek.CornerMarker->WallThickness * frameRight;

					CornerPeek.PeekPosition = bPeekingFromLeft ? CPP_MiddleLeft : CPP_MiddleRight;
					CornerPeek.CornerLocation = bPeekingFromLeft ? rightCorner : leftCorner;
					CornerPeek.FwdDir = wallDir;
					CornerPeek.SideDir = bPeekingFromLeft ? -frameRight : frameRight;						

					check(PeekingAnimNode);
					PeekingAnimNode->SetPeekingType(bPeekingFromLeft, CornerPeek.bRoundedCorner);
					ShadowProxyPeekingAnimNode->SetPeekingType(bPeekingFromLeft, CornerPeek.bRoundedCorner);

					LastValidCornerPeekPosition = CornerPeek.PeekPosition;

					CornerPeek.IKStrength = 0.0f;

					StartSpecialMove(SMT_ContextualLeanInsideTransition);
					return;
				}
			}
		}
		
		if (appIsNearlyZero(normalizedInput, KINDA_SMALL_NUMBERF) && (PeekingAnimNode->CurrentRatio < 0.05f))
		{
			// No input and we're at 0 - exit back
			StartSpecialMove(SMT_ExitContextualLean);
			return;
		}
		else if ((PeekingAnimNode->CurrentRatio >= 0.9f) && playerIntentVelocity.SizeSquared2D() >= Square(ExitForwardMinIntentVelocity))
		{
			FVector playerIntentDirection = playerIntentVelocity.SafeNormal2D();
			if ((playerIntentDirection | CornerPeek.FwdDir) > 0.707f)
			{
				// Player wants out - leave forward
				StartSpecialMove(SMT_ExitContextualLeanForward);

				CurrentLean = 0.0f;
				OLPC->bInvalidateLeanInput = TRUE;
				return;
			}
		}
	}

	PeekingAnimNode->TargetRatio = normalizedInput;
	ShadowProxyPeekingAnimNode->TargetRatio = normalizedInput;
}

void AOLHero::UpdateBedAnimation(FLOAT deltaTime)
{
	if (BedAnimNode && BedAnimNode->AnimSeq)
	{
		FLOAT animLen = BedAnimNode->AnimSeq->SequenceLength;
		FLOAT curRelYawDeg = UNR_TO_DEG * (FRotator::NormalizeAxis(EyeRotation.Yaw - Rotation.Yaw));
		FLOAT desiredRatio = Saturate((90.0f + curRelYawDeg) / 180.0f);

		if (desiredRatio == 1.0f)
		{
			desiredRatio -= KINDA_SMALL_NUMBERF; // 100% won't set the time properly
		}

		FLOAT desiredTime = desiredRatio * BedAnimNode->AnimSeq->SequenceLength;

		TWEAKABLE FLOAT ApproachCoeff = 0.999f;
		FLOAT newTime = Utils::Approach(BedAnimNode->CurrentTime, desiredTime, ApproachCoeff, deltaTime);

		BedAnimNode->PreviousTime = BedAnimNode->CurrentTime;
		BedAnimNode->CurrentTime = newTime;
		BedAnimNode->ConditionalClearCachedData();

		ShadowProxyBedAnimNode->PreviousTime = ShadowProxyBedAnimNode->CurrentTime;
		ShadowProxyBedAnimNode->CurrentTime = newTime;
		ShadowProxyBedAnimNode->ConditionalClearCachedData();
	}
}

UBOOL AOLHero::TryObjectPickup(UBOOL playerInteraction)
{
	if (!CouldStartSpecialMove(SMT_PickupObject))
	{
		return FALSE;
	}

	for (INT idx = 0; idx < CachedPickables.Num(); idx++)
	{
		AOLPickableObject* pickable = CachedPickables(idx);		

		if (pickable && pickable->IsValid())
		{
			FVector toPickup = pickable->Location - Location;
			FLOAT distHorzSq = toPickup.SizeSquared2D();

			if (distHorzSq < Square(PickupObjectInteractDistMinHorz) || distHorzSq > Square(PickupObjectInteractDistMaxHorz))
			{
				// outside of 2d range
				continue;
			}

			UBOOL bCrouchedMove = FALSE;

			if (toPickup.Z >= PickupObjectInteractDistMinVertCrouched && toPickup.Z <= PickupObjectInteractDistMaxVertCrouched)
			{
				bCrouchedMove = TRUE;
			}
			else if (toPickup.Z < PickupObjectInteractDistMinVertStanding || toPickup.Z > PickupObjectInteractDistMaxVertStanding)
			{
				// outside of height range
				continue;
			}

			if (((pickable->Location - EyeLocation) | EyeForward) < 0.0f)
			{
				// behind us
				continue;
			}

			// check 1: aiming within radius near the object (the closer, the easier)
			if (PointDistToLine(pickable->Location, EyeForward, EyeLocation) > PickupInteractRadius)
			{
				// if that fails, check 2: pickup within angle from eye direction (the farther, the easier)
				FVector eyeToPickable = pickable->Location - EyeLocation;
				if ((EyeForward | eyeToPickable.SafeNormal()) < MinCosAngleForPickup)
				{
					// if both failed, we won't pick that up
					continue;
				}
			}

			FCheckResult Hit(1.f);
			FVector startTrace = EyeLocation;
			FVector endTrace = pickable->Location + VecZ(5.0f);
			
			// check that we have LoS to the pickup
			if (GWorld->SingleLineCheck( Hit, this, endTrace, startTrace, TRACE_AllBlocking | TRACE_ComplexCollision | TRACE_StopAtAnyHit, FVector(0.0f)))
			{
				if (!pickable->CanPickup(this))
				{
					pickable->TryShowInvalidPickupMessage();
				}
				else if (!playerInteraction)
				{
					OLPC->AddAvailableInteraction(PIT_PickupObject);
					OLPC->PickupTargetName = pickable->GetPickableName();
				}
				else
				{
					if (!TryCommitToSpecialMove(SMT_PickupObject, pickable))
					{
						return FALSE;
					}

					ActivePickup = pickable;
					bMustCrouchAfterSpecialMove = bIsCrouched;
					bPickupCrouched = bCrouchedMove;

					StartSpecialMove(SMT_PickupObject, Location, (pickable->Location - Location).SafeNormal2D(), APTT_TargetAtStart);

					return TRUE;
				}
			}
		}
	}

	return FALSE;
}

UBOOL AOLHero::TryCSA(UBOOL playerInteraction)
{
	if (!CouldStartSpecialMove(SMT_CSA))
	{
		return FALSE;
	}

	for (INT idx = 0; idx < CachedCSAs.Num(); idx++)
	{
		AOLCSA* csa = CachedCSAs(idx);		

		if (csa && csa->TryActivate(this, playerInteraction))
		{
			ActiveCSA = csa;
			
			if (csa->AnimName == NAME_None)
			{
				StartSpecialMove(SMT_CSA, Location, CharForward, APTT_TargetAtStart);
				bPlayingSpecialMoveAnim = FALSE;
			}
			else
			{
				FVector expectedAnimFwd(0.0f);
				FVector expectedAnimStart(0.0f);

				if (csa->ReferenceAnimActor)
				{
					expectedAnimFwd = -csa->ReferenceAnimActor->Rotation.Vector();
					expectedAnimStart = csa->ReferenceAnimActor->Location - csa->AnimStartDistFwd*expectedAnimFwd - csa->AnimStartDistRight*csa->ReferenceAnimActor->Rotation.Right();
				}
				else
				{
					expectedAnimFwd = -csa->Rotation.Vector();
					expectedAnimStart = csa->Location - csa->AnimStartDistFwd*expectedAnimFwd - csa->AnimStartDistRight*csa->Rotation.Right();
				}
				expectedAnimStart.Z = Location.Z;
				StartSpecialMove(SMT_CSA, expectedAnimStart, expectedAnimFwd, APTT_TargetAtStart);
			}

			return TRUE;
		}
	}

	return FALSE;
}


UBOOL AOLHero::TryPushObject(UBOOL playerInteraction)
{
	check(ActivePushable == NULL);

	if (!CouldStartSpecialMove(SMT_StartPushingObject))
	{
		return FALSE;
	}

	for (INT pushableIdx = 0; pushableIdx < CachedPushables.Num(); pushableIdx++)
	{
		AOLPushableObject* pushable = CachedPushables(pushableIdx);

		if (pushable && pushable->bEnabled)
		{
			UBOOL startedPushing = TryPushObject(playerInteraction, pushable);

			if (startedPushing)
			{
				return TRUE;
			}			
		}		
	}

	return FALSE;
}

UBOOL AOLHero::TryPushObject(UBOOL playerInteraction, AOLPushableObject* pushable)
{
	FVector edgeBack = pushable->GetCurrentBackEdge();
	FVector edgeFwd = pushable->GetCurrentFwdEdge();
	FLOAT distBackSq = edgeBack.DistanceSquared(Location);
	FLOAT distFwdSq = edgeFwd.DistanceSquared(Location);
	UBOOL bBackEdge = distBackSq < distFwdSq;
	FLOAT distSqToEdge = bBackEdge ? distBackSq : distFwdSq;

	if (distSqToEdge > Square(PushableInteractDist))
	{
		// Too far
		return FALSE;
	}

	if ((bBackEdge && !pushable->CanPushFwd()) || (!bBackEdge && !pushable->CanPushBack()))
	{
		// Can't push on this edge
		return FALSE;
	}

	const FVector& edge = bBackEdge ? edgeBack : edgeFwd;
	FVector pushDirection = bBackEdge ? pushable->GetFwdDirection() : pushable->GetBackDirection();
	FVector toEdgeDir2D = (edge - Location).SafeNormal2D();

	if ((pushDirection | toEdgeDir2D) < 0.0f)
	{
		// On the wrong side of the edge
		return FALSE;
	}

	if ((EyeForward.SafeNormal2D() | toEdgeDir2D) < PushableInteractMinLookCosAngle)
	{
		// Not looking at the edge
		return FALSE;
	}	

	FCheckResult Hit(1.f);
	FVector startTrace = EyeLocation;
	FVector endTrace = edge - 10.0f*pushDirection + VecZ(150.0f);

	// check that we have LoS to the edge
	if (!GWorld->SingleLineCheck( Hit, this, endTrace, startTrace, TRACE_AllBlocking | TRACE_StopAtAnyHit, FVector(0.0f)))
	{
		// no LoS to the edge
		return FALSE;
	}

	FVector targetLoc = edge - PushableExpectedDistFromEdge*pushDirection + PushableExpectedSideOffset*pushable->GetSideDirection();
	targetLoc.Z = Location.Z;

	if (WouldEncroach(targetLoc - 10.0f * pushDirection))
	{
		// Colliding, maybe too close to a wall
		return FALSE;
	}

	if (!playerInteraction)
	{
		OLPC->AddAvailableInteraction(PIT_PushObject);
		return FALSE;
	}

	if (!TryCommitToSpecialMove(SMT_StartPushingObject, pushable))
	{
		return FALSE;
	}

	// Let's do it

	if (bIsCrouched) 
	{
		if (!TryUncrouch())
		{
			return FALSE;
		}
	}

	ActivePushable = pushable;
	bPushingFromBackEdge = bBackEdge;

	pushable->StartPushing();
	StartSpecialMove(SMT_StartPushingObject, targetLoc, pushDirection, APTT_TargetAtEnd);

	return TRUE;
}

void AOLHero::StopPushing()
{
	if (CouldStartSpecialMove(SMT_StopPushingObject))
	{
		if (ActivePushable)
		{
			ActivePushable->StopPushing();
		}
		StartSpecialMove(SMT_StopPushingObject);
	}
}

void AOLHero::UpdateCachedObjects()
{
	TWEAKABLE FLOAT cacheRadius = 2000.0f;
	TWEAKABLE FLOAT MaxLedgeZDelta = 20.0f;

	TArray<ULevel*> visibleLevels;

	for( INT LevelIndex = 0 ; LevelIndex < WorldInfo->StreamingLevels.Num() ; ++LevelIndex )
	{
		ULevelStreaming* LevelStreamingObject = WorldInfo->StreamingLevels(LevelIndex);
		if( LevelStreamingObject && LevelStreamingObject->LoadedLevel && LevelStreamingObject->bIsVisible)
		{
			visibleLevels.AddItem(LevelStreamingObject->LoadedLevel);
		}
	}

	UBOOL levelsChanged = FALSE;

	for (INT i = 0; i < visibleLevels.Num(); i++)
	{
		ULevel* level = visibleLevels(i);
		if (!CachedLevelList.ContainsItem(level))
		{
			levelsChanged = TRUE;
			break;
		}
	}

	if (levelsChanged)
	{
		Utils::GetSoundEnvManager()->bEnvironmentsDirty = TRUE;
		OLPC->LoadedLevelListChanged();
	}

	if ( levelsChanged || (Location - CachedObjectsPos).SizeSquared() > Square(0.5f*cacheRadius)) // We update the cache at half it's radius as we need to interact with ledges, not markers
	{
		CachedMarkers.Empty();
		CachedDoors.Empty();
		CachedPickables.Empty();
		CachedHidingSpots.Empty();
		CachedLadders.Empty();
		CachedBeds.Empty();
		CachedCSAs.Empty();
		CachedScares.Empty();
		CachedPushables.Empty();
		CachedCorners.Empty();
		CachedHeatMarkers.Empty();
		CachedRecordingMarkers.Empty();
		CachedPreferredPathMarkers.Empty();
		CachedLights.Empty();
		
		for (FActorIterator It; It; ++It)
		{
			AActor* actor = *It;
			if (actor && actor->IsAStaticMeshActor())
			{
				// early out of the most common case
				continue;
			}

			// Support long distances for recording markers
			AOLRecordingMarker* recordingMarker = Cast<AOLRecordingMarker>(*It);
			if (recordingMarker)
			{
				CachedRecordingMarkers.AddItem(recordingMarker);
				continue;
			}

			// Support long distances for lights
			ALight* light = Cast<ALight>(*It);
			if (light)
			{
				CachedLights.AddItem(light);
				continue;
			}

			// And preferred path markers (for enemies)
			AOLPreferredPathMarker* pathMarker = Cast<AOLPreferredPathMarker>(*It);
			if (pathMarker)
			{
				CachedPreferredPathMarkers.AddItem(pathMarker);
				continue;
			}

			// check within cache radius
			if (((*It)->Location - Location).SizeSquared() > Square(cacheRadius))
			{
				continue;
			}

			if ((*It)->IsA(AOLGameplayMarker::StaticClass()))
			{
				AOLLedgeMarker* ledgeMarker = Cast<AOLLedgeMarker>(*It);
				if (ledgeMarker)
				{
					if (ledgeMarker->Next)
					{
						// Update the reverse side
						ledgeMarker->Next->Prev = ledgeMarker;
						
						// check that ledge is even
						if (Abs(ledgeMarker->Location.Z - ledgeMarker->Next->Location.Z) <= MaxLedgeZDelta)
						{
							CachedMarkers.AddItem(ledgeMarker);
						}
						else
						{
							// Invalidate the ledge (should be done in editor)
							ledgeMarker->Next->Prev = NULL;
							ledgeMarker->Next = NULL;
						}
					}
					else
					{
						CachedMarkers.AddItem(ledgeMarker);
					}
					continue;
				}

				AOLHidingSpot* hidingSpot = Cast<AOLHidingSpot>(*It);
				if (hidingSpot)
				{
					CachedHidingSpots.AddItem(hidingSpot);
					continue;
				}

				AOLBed* bed = Cast<AOLBed>(*It);
				if (bed)
				{
					CachedBeds.AddItem(bed);
					continue;
				}

				AOLLadderMarker* ladder = Cast<AOLLadderMarker>(*It);
				if (ladder)
				{
					CachedLadders.AddItem(ladder);
					continue;
				}

				AOLCSA* csa = Cast<AOLCSA>(*It);
				if (csa)
				{
					CachedCSAs.AddItem(csa);
					continue;
				}

				AOLScareMoment* scare = Cast<AOLScareMoment>(*It);
				if (scare)
				{
					CachedScares.AddItem(scare);
					continue;
				}

				AOLCornerMarker* corner = Cast<AOLCornerMarker>(*It);
				if (corner)
				{
					CachedCorners.AddItem(corner);
					continue;
				}

				AOLHeatMarker* heatMarker = Cast<AOLHeatMarker>(*It);
				if (heatMarker)
				{
					CachedHeatMarkers.AddItem(heatMarker);
					continue;
				}

				continue;
			}
			
			AOLDoor* door = Cast<AOLDoor>(*It);
			if (door)
			{
				CachedDoors.AddItem(door);
				continue;
			}

			AOLPickableObject* pickable = Cast<AOLPickableObject>(*It);
			if (pickable)
			{
				CachedPickables.AddItem(pickable);
				continue;
			}

			AOLPushableObject* pushable = Cast<AOLPushableObject>(*It);
			if (pushable)
			{
				CachedPushables.AddItem(pushable);
				continue;
			}
		}

		CachedObjectsPos = Location;
		CachedLevelList = visibleLevels;
	}
	else
	{
		CachedMarkers.RemoveItem(NULL);
		CachedDoors.RemoveItem(NULL);
		CachedPickables.RemoveItem(NULL);
		CachedHidingSpots.RemoveItem(NULL);
		CachedLadders.RemoveItem(NULL);
		CachedBeds.RemoveItem(NULL);
		CachedCSAs.RemoveItem(NULL);
		CachedScares.RemoveItem(NULL);
		CachedPushables.RemoveItem(NULL);
		CachedCorners.RemoveItem(NULL);
		CachedHeatMarkers.RemoveItem(NULL);
		CachedRecordingMarkers.RemoveItem(NULL);
		CachedPreferredPathMarkers.RemoveItem(NULL);
		CachedLights.RemoveItem(NULL);
	}
}

//////////////////////////////////////////////////////////////////////////
// Camera
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

UBOOL AOLHero::HeadingLockedToCamera() const
{
	return GP().CameraMode == CRM_UserControlled;
}

void AOLHero::GetCamera(FVector& out_CamLoc, FRotator& out_CamRot, FLOAT& out_FOV)
{
	out_CamLoc = EyeLocation;
	out_CamRot = EyeRotation;
	out_FOV = CurrentFOV;
}

void AOLHero::UpdateFOV(FLOAT deltaTime)
{
	if (IsCamcorderActive() && !IsConsideredLookingBack()) // lookback overrides the camera fov
	{
		FLOAT maxFov = IsInNightVision() ? CamcorderNVMaxFOV : CamcorderMaxFOV;

		FLOAT effectiveMinFOV = OverriddenMinCamcorderFOV > 0.0f ? OverriddenMinCamcorderFOV : CamcorderMinFOV;
		CurrentFOV = CurrentCamcorderZoomFactor * (effectiveMinFOV - maxFov) + maxFov;
	}
	else
	{
		const FGameplayParams& gp = GP();

		FLOAT targetFOV = DefaultFOV;
		FLOAT openingCoeff = FOVApproachCoeffOpening;
		FLOAT closingCoeff = FOVApproachCoeffClosing;

		if (!IsCamcorderInactive() && LocomotionMode != LM_Bed)
		{
			TWEAKABLE FLOAT CamcorderFOVApproachCoeff = 0.98f;
			openingCoeff = CamcorderFOVApproachCoeff;
			closingCoeff = CamcorderFOVApproachCoeff;
			targetFOV = DefaultFOV;
		}
		else if (IsConsideredLookingBack())
		{
			targetFOV = LocomotionModeParams[LM_LookBack].GP.FOVOverride;
		}
		else if (gp.FOVOverride > 0.0f)
		{
			targetFOV = gp.FOVOverride;
		}
		else if (LocomotionMode == LM_Walk)
		{
			if (bWantToRun && IsRunning())
			{
				targetFOV = RunningFOV;
			}

			openingCoeff = FOVApproachCoeffRun;
			closingCoeff = FOVApproachCoeffWalk;
		}

		FLOAT approachCoeff = (targetFOV > CurrentFOV) ? openingCoeff : closingCoeff;
		CurrentFOV = Utils::Approach(CurrentFOV, targetFOV, approachCoeff, deltaTime);
	}	
}

void AOLHero::UpdateCamera(FLOAT deltaTime)
{
	Camera->Update(deltaTime);
	
	EyeLocation = Camera->ViewWS.Loc;
	EyeRotation = Camera->ViewWS.Rotator();
	EyeForward = EyeRotation.Vector();
	CharForward = Rotation.Vector();

	FRotationTranslationMatrix camWS(EyeRotation, EyeLocation);
	CameraCompSpace = camWS * Mesh->LocalToWorld.Inverse();

	// Update camera-dependent effects
	UpdateNightVision(deltaTime); 
	UpdateRainEffect(deltaTime);
	UpdateCameraEffect(deltaTime);

	UpdateFOV(deltaTime);
}

FVector AOLHero::GetPawnViewLocation()
{
	return EyeLocation;
}

FRotator AOLHero::GetViewRotation()
{
	return EyeRotation;
}

void AOLHero::ApplyDynamicCameraLimits(const FRotator& baseRot, const FCamView& viewWS, FViewLimits& CurrentLimits, FLOAT& minYawCS, FLOAT& maxYawCS, FLOAT& minPitchCS, FLOAT& maxPitchCS, FLOAT& minPitchWS, FLOAT& maxPitchWS)
{
	TWEAKABLE FLOAT MinPitchToHideRightArmHack = -60.0f;	
	TWEAKABLE FLOAT MinPitchReloading = -30.0f;	
	TWEAKABLE FLOAT MinPitchCornerIK = -55.0f;	
	TWEAKABLE FLOAT MinPitchHeatShielding = -55.0f;	
	TWEAKABLE FLOAT MinPitchParrying = -55.0f;	
	TWEAKABLE FLOAT MinPitchHobblingSpecial = -55.0f;
	TWEAKABLE FLOAT MinPitchHobblingMoving = -60.0f;
	TWEAKABLE FLOAT MinPitchHobblingIdle = -65.0f;
	TWEAKABLE UBOOL bCanSeeBodyWithCamcorder = TRUE;

	if (IsInCamcorderTransition())
	{
		minPitchWS =  Max(minPitchWS, MinPitchReloading);
	}
	else if ((!bCanSeeBodyWithCamcorder && !IsCamcorderInactive()) || (RightArmAnimSlot && RightArmAnimSlot->GetCustomAnimNodeSeq() != NULL) || (LeftArmAnimSlot && LeftArmAnimSlot->GetCustomAnimNodeSeq() != NULL) || (UpperBodyBlendNode && UpperBodyBlendNode->bActive))
	{
		minPitchWS = Max(minPitchWS, MinPitchToHideRightArmHack);
	}
	else if (CornerPeek.CornerMarker && CornerPeek.IKStrength > 0.5f)
	{
		minPitchWS =  Max(minPitchWS, MinPitchCornerIK);
	}
	else if (bHeatShielding)
	{
		minPitchWS =  Max(minPitchWS, MinPitchHeatShielding);
	}
	else if (bParrying)
	{
		minPitchWS =  Max(minPitchWS, MinPitchParrying);
	}
	else if (bHobbling)
	{
		if (SpecialMove == SMT_Uncrouch || !appIsNearlyZero(CurrentLean, KINDA_SMALL_NUMBERF))
		{
			minPitchWS =  Max(minPitchWS, MinPitchHobblingSpecial);
		}
		else if (RealVelocity.SizeSquared2D() >= Square(25.0f))
		{
			minPitchWS =  Max(minPitchWS, MinPitchHobblingMoving);
		}
		else
		{
			minPitchWS =  Max(minPitchWS, MinPitchHobblingIdle);
		}
	}
}

void AOLHero::SetCamParams(const FGameplayParams& gameplayParams)
{
	CamParams.MinYaw = gameplayParams.MinYaw;
	CamParams.MaxYaw = gameplayParams.MaxYaw;
	CamParams.MinPitchWS = gameplayParams.MinPitchWS;
	CamParams.MaxPitchWS = gameplayParams.MaxPitchWS;
	CamParams.MinPitchCS = gameplayParams.MinPitchCS;
	CamParams.MaxPitchCS = gameplayParams.MaxPitchCS;
}

void AOLHero::OverrideCameraSettingsNotify(UOLAnimNotify_OverrideCameraParams* camParamsNotify)
{
	if (camParamsNotify->bResetToDefault)
	{
		SetCamParams(GP());

		Camera->bLocalSpacePlayerControl = FALSE;
		return;
	}

	if (LocomotionMode != LM_Cinematic && LocomotionMode != LM_Grabbed && SpecialMove == SMT_None)
	{
		// Ignore - only allowed in cinematics and during special moves
		// (ignore required to filter out rare cases where the notify is received after the special moves ends)
		return;
	}

	if (camParamsNotify->bMinYaw)
	{
		CamParams.MinYaw = camParamsNotify->MinYaw;
	}

	if (camParamsNotify->bMaxYaw)
	{
		CamParams.MaxYaw = camParamsNotify->MaxYaw;
	}

	if (camParamsNotify->bMinPitchWS)
	{
		CamParams.MinPitchWS = camParamsNotify->MinPitchWS;
	}

	if (camParamsNotify->bMaxPitchWS)
	{
		CamParams.MaxPitchWS = camParamsNotify->MaxPitchWS;
	}

	if (camParamsNotify->bMinPitchCS)
	{
		CamParams.MinPitchCS = camParamsNotify->MinPitchCS;
	}

	if (camParamsNotify->bMaxPitchCS)
	{
		CamParams.MaxPitchCS = camParamsNotify->MaxPitchCS;
	}

	Camera->bLocalSpacePlayerControl = camParamsNotify->bLocalSpacePlayerControl;
}

void AOLHero::ResetNeckOffsetNotify()
{
	if (LocomotionMode == LM_Cinematic)
	{
		LocomotionModeParams[LM_Cinematic].NeckOffsetSide = 0.0f;
		LocomotionModeParams[LM_Cinematic].NeckOffsetFwd = 0.0f;
	}
}

//////////////////////////////////////////////////////////////////////////
// Animation
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void AOLHero::NativePlayLanded(FLOAT impactVel)
{
	if (GWorld->GetTimeSeconds() < SpawnTime + 2.0f)
	{
		// no sound if just spawned
		return;
	}

	UBOOL bLandedInWater = (GetMaterialBelowFeet() == WaterMaterial) || IsInWaterVolume();

	if (CouldStartSpecialMove(SMT_BigLanding) && -impactVel > ImpactVelThresholdForBigLanding && !IsInWaterVolume())
	{
		TWEAKABLE FLOAT TestHalfHeight = 25.0f;
		TWEAKABLE FLOAT AnimHeightThresh = 8.0f;
		TWEAKABLE FLOAT FwdTestDist = 60.0f;
		// test that we have some clearance ahead to put our hand down

		if (!bHeatShielding)
		{
			FCheckResult Hit(1.f);
			FVector startTrace = Location + VecZ(TestHalfHeight) + FwdTestDist*CharForward;
			FVector endTrace = Location - VecZ(TestHalfHeight) + FwdTestDist*CharForward;

			UBOOL bCanLandBig = FALSE;

			if (!GWorld->SingleLineCheck( Hit, Owner, endTrace, startTrace, TRACE_AllBlocking, FVector(0.0f)))
			{
				FLOAT floorHeight = TestHalfHeight - 2.0f*Hit.Time*TestHalfHeight;
				bCanLandBig = IsBetween(floorHeight, -AnimHeightThresh, AnimHeightThresh);
			}

			if (bCanLandBig)
			{
				StartSpecialMove(SMT_BigLanding);
			}
			else
			{
				UBOOL bConsiderCrouched = (BlendByPostureFallingAnimNode && BlendByPostureFallingAnimNode->ActiveChildIndex == 1);
				PlayFullBodyAnim(bConsiderCrouched ? AnimNameLandingSmallCrouched : AnimNameLandingSmallStanding, 1.0f, 0.1f, 0.2f);
			}
		}

		TriggerSoundEvent(SndBigLanding);
		OLPC->eventClientPlayForceFeedbackWaveform(BigLandingFFWaveform);

		MakeNoise(bLandedInWater ? LandingBigWaterLoudness : LandingBigLoudness);
	}
	else if (-impactVel > ImpactVelThresholdForSmallLanding) 
	{
		if (!bHeatShielding)
		{
			UBOOL bConsiderCrouched = (BlendByPostureFallingAnimNode && BlendByPostureFallingAnimNode->ActiveChildIndex == 1);
			PlayFullBodyAnim(bConsiderCrouched ? AnimNameLandingSmallCrouched : AnimNameLandingSmallStanding, 1.0f, 0.1f, 0.2f); // todo: use additive?
		}

		TriggerSoundEvent(SndSmallLanding);
		OLPC->eventClientPlayForceFeedbackWaveform(SmallLandingFFWaveform);

		FCameraShakeData camShakeData = Camera->SmallLandingShakeData;			
		camShakeData.Intensity *= MapClamped(-impactVel, ImpactVelThresholdForSmallLanding, ImpactVelThresholdForBigLanding, 0.0f, 1.0f);
		Camera->ActivateCameraShake(camShakeData, Location);

		MakeNoise(bLandedInWater ? LandingSmallWaterLoudness : LandingSmallLoudness);
	}
	
	if (-impactVel > ImpactVelThresholdForSpeedPenalty)
	{
		bApplyLandingPenalty = TRUE;
	}
	
	LastLandingTimestamp = GWorld->GetTimeSeconds();
	bJumping = FALSE;
}

void AOLHero::NotifyDoorClosing(AOLDoor* door)
{
	if (LocomotionMode == LM_Door && ActiveDoor == door && (DoorOpeningType == DOT_LeftPush || DoorOpeningType == DOT_RightPush))
	{
		// The door we're interacting with will close. do something about it
		// Should be an anim (special move), right now it's just procedural
		StartSpecialMove(SMT_DoorClosedFromOtherSide);

		TWEAKABLE FLOAT EndOffset = 90.0f;
		FVector targetPos = door->GetCenterLocation() - EndOffset * door->GetStaticDirection();

		FProceduralAnimData animData;
		animData.PositionDelta = targetPos - Location;
		QueueProceduralAnim(animData);
	}
}

void AOLHero::PickupNotify()
{
	if (SpecialMove == SMT_PickupObject && ActivePickup && !ActivePickup->bUsed)
	{
		if (ActivePickup->bPickupOnNotify)
		{
			ActivePickup->Pickup(this);
		}

		FRotator relRotation = ActivePickup->AttachRotationOffset;
		FVector relLoc = ActivePickup->AttachPositionOffset;

		FVector pickupFwd = ActivePickup->Rotation.Right();
			
		if ((pickupFwd.SafeNormal2D() | EyeForward.SafeNormal2D()) < 0.0f)
		{
			// add an extra 180 rotation
			relRotation.Yaw = FRotator::NormalizeAxis(relRotation.Yaw + 180.0f*DEG_TO_UNR);
		}

		AttachProp(ActivePickup->PickupMesh, -1.0f, 0.0f, 0.0f, -relLoc, relRotation, TRUE, TRUE);

		OLPC->eventClientPlayForceFeedbackWaveform(PickupFFWaveform);
	}
}

void AOLHero::DoorAnimNotify()
{
	if (!ActiveDoor)
	{
		return;
	}

	switch (SpecialMove)
	{
	case SMT_TryOpenLockedDoor:
		{
			if (!ActiveDoor->IsOpening())
			{
				ActiveDoor->TriedOpening(this);
			}
		}
		break;
	case SMT_OpenDoorInstant:
	case SMT_OpenLockerFromOutside:
		{
			if (!ActiveDoor->IsOpening())
			{
				if (ActiveDoor->bAutoClose)
				{
					ActiveDoor->PlayAutoCloseAnim();
				}
				else
				{
					ActiveDoor->Open(this);

					MakeNoise(DoorOpenInstantLoudness, DoorMajorNoise);
				}
			}
		}
		break;
	case SMT_OpenDoorPartial:
		{
			if (!ActiveDoor->IsOpening())
			{
				TWEAKABLE FLOAT InsidePushOpenSpeed = 350.0f;
				FLOAT openSpeed = (DoorPartialOpenType == DPOT_LeftPush || DoorPartialOpenType == DPOT_RightPush) ? InsidePushOpenSpeed : 0.0f;
				ActiveDoor->Open(this, openSpeed);

				MakeNoise(DoorOpenPartialLoudness);
			}
		}
		break;
	case SMT_CloseDoor:
	case SMT_CloseDoorPositionned:
		{
			if (!ActiveDoor->IsClosing())
			{
				if (bQuietDoorInteraction)
				{
					TWEAKABLE FLOAT ClosingStartDelay = 0.567f;
					TWEAKABLE FLOAT TotalClosingTime = 0.767f;
					ActiveDoor->CloseQuiet(this, DoorSlowClosingAnimStartTime + ClosingStartDelay, TotalClosingTime);
				}
				else
				{
					ActiveDoor->Close(this);
					MakeNoise(DoorCloseFastLoudness);
				}
			}
		}
		break;
	case SMT_EnterLocker:
	case SMT_ExitLocker:
		{
			if (!ActiveDoor->IsOpened())
			{
				ActiveDoor->Open(this);
			}
			else
			{
				ActiveDoor->Close(this);

				if (SpecialMove == SMT_EnterLocker)
				{
					MakeNoise(DoorEnterLockerLoudness, DoorMajorNoise);
				}
				else
				{
					MakeNoise(DoorExitLockerLoudness, DoorMajorNoise);
				}
			}
		}
		break;
	case SMT_RunThroughDoor:
		{
			if (!ActiveDoor->IsOpened())
			{
				TWEAKABLE FLOAT DoorOpenSpeed = 500.0f;
				ActiveDoor->Open(this, DoorOpenSpeed);

				MakeNoise(DoorRunThroughLoudness, DoorMajorNoise);
			}
		}
		break;
	}	

	OLPC->eventClientPlayForceFeedbackWaveform(SpecialMove == SMT_RunThroughDoor ? RunThroughDoorFFWaveform : DoorInteractionFFWaveform);
}

void AOLHero::CamcorderAvailableNotify()
{
	bBothHandsNeeded = FALSE;
}

FLOAT AOLHero::PlayFullBodyAnim(const FName& animName, FLOAT playRate, FLOAT blendInTime, FLOAT blendOutTime, FLOAT startTime, FLOAT endTime)
{
	if (FullBodyAnimSlot->bIsPlayingCustomAnim)
	{
		FullBodyAnimSlot->StopCustomAnim(0.1f);
		ShadowProxyFullBodyAnimSlot->StopCustomAnim(0.1f);
	}
	FLOAT RetValue = FullBodyAnimSlot->PlayCustomAnim(animName, playRate, blendInTime, blendOutTime, FALSE, FALSE, startTime, endTime);
	ShadowProxyFullBodyAnimSlot->PlayCustomAnim(animName, playRate, blendInTime, blendOutTime, FALSE, FALSE, startTime, endTime);
	FullBodyAnimSlot->SetActorAnimEndNotification(TRUE);

	if (RetValue > 0.f)
	{
		PlayingSpecialMoveAnims.AddItem(animName);
	}

	return RetValue;
}

FLOAT AOLHero::PlayBlendedAnim(const FName& animNameA, const FName& animNameB, FLOAT weightA, FLOAT blendInTime, FLOAT blendOutTime, FLOAT rate, FLOAT startRatio)
{
	if (CustomBlendNode->bActive)
	{
		CustomBlendNode->StartBlendingOut();
		ShadowProxyCustomBlendNode->StartBlendingOut();
	}

	FLOAT RetValue = CustomBlendNode->PlayCustomBlend(animNameA, animNameB, weightA, blendInTime, blendOutTime, rate, startRatio);
	ShadowProxyCustomBlendNode->PlayCustomBlend(animNameA, animNameB, weightA, blendInTime, blendOutTime, rate, startRatio);

	if (RetValue > 0.f)
	{
		PlayingSpecialMoveAnims.AddItem(animNameA);
		PlayingSpecialMoveAnims.AddItem(animNameB);
	}

	return RetValue;
}

FLOAT AOLHero::PlayBlendedAnim3(const FName& animNameA, const FName& animNameB, const FName& animNameC, FLOAT blendWeightA, FLOAT blendWeightB, FLOAT blendInTime, FLOAT blendOutTime, FLOAT rate, FLOAT startRatio)
{
	if (CustomBlendNode->bActive)
	{
		CustomBlendNode->StartBlendingOut();
		ShadowProxyCustomBlendNode->StartBlendingOut();

	}

	FLOAT RetValue = CustomBlendNode->PlayCustomBlend(animNameA, animNameB, animNameC, blendWeightA, blendWeightB, blendInTime, blendOutTime, rate, startRatio);
	ShadowProxyCustomBlendNode->PlayCustomBlend(animNameA, animNameB, animNameC, blendWeightA, blendWeightB, blendInTime, blendOutTime, rate, startRatio);

	if (RetValue > 0.f)
	{
		PlayingSpecialMoveAnims.AddItem(animNameA);
		PlayingSpecialMoveAnims.AddItem(animNameB);
		PlayingSpecialMoveAnims.AddItem(animNameC);
	}

	return RetValue;
}

FLOAT AOLHero::PlayBlendSpace(const FBlendSpaceNode nodes[], INT numNodes, const INT blendSpaces[][3], INT numSpaces, const FVector2D& coord, FLOAT blendInTime, FLOAT blendOutTime, FLOAT rate, FLOAT startRatio)
{
	if (CustomBlendNode->bActive)
	{
		CustomBlendNode->StartBlendingOut();
		ShadowProxyCustomBlendNode->StartBlendingOut();
	}

	FLOAT RetValue = CustomBlendNode->PlayBlendSpace(nodes, numNodes, blendSpaces, numSpaces, coord, blendInTime, blendOutTime, rate, startRatio);
	ShadowProxyCustomBlendNode->PlayBlendSpace(nodes, numNodes, blendSpaces, numSpaces, coord, blendInTime, blendOutTime, rate, startRatio);

	if (RetValue > 0.f)
	{
		// we don't really know which anims actually play so we add them all
		for (INT i = 0; i < numNodes; i++)
		{
			PlayingSpecialMoveAnims.AddItem(nodes[i].AnimName);
		}
	}

	return RetValue;
}

void AOLHero::PlayRightArmAnim(const FName& animName, FLOAT playRate, FLOAT blendInTime, FLOAT blendOutTime, FLOAT startTime, FLOAT endTime)
{
	if (RightArmAnimSlot->bIsPlayingCustomAnim)
	{
		RightArmAnimSlot->StopCustomAnim(0.1f);
		ShadowProxyRightArmAnimSlot->StopCustomAnim(0.1f);
	}

	RightArmAnimSlot->PlayCustomAnim(animName, playRate, blendInTime, blendOutTime, FALSE, FALSE, startTime, endTime);
	RightArmAnimSlot->SetActorAnimEndNotification(TRUE);
	ShadowProxyRightArmAnimSlot->PlayCustomAnim(animName, playRate, blendInTime, blendOutTime, FALSE, FALSE, startTime, endTime);	
}

void AOLHero::PlayLeftArmAnim(const FName& animName, FLOAT playRate, FLOAT blendInTime, FLOAT blendOutTime, FLOAT startTime, FLOAT endTime)
{
	if (LeftArmAnimSlot->bIsPlayingCustomAnim)
	{
		LeftArmAnimSlot->StopCustomAnim(0.1f);
		ShadowProxyLeftArmAnimSlot->StopCustomAnim(0.1f);
	}

	LeftArmAnimSlot->PlayCustomAnim(animName, playRate, blendInTime, blendOutTime, FALSE, FALSE, startTime, endTime);
	LeftArmAnimSlot->SetActorAnimEndNotification(TRUE);
	ShadowProxyLeftArmAnimSlot->PlayCustomAnim(animName, playRate, blendInTime, blendOutTime, FALSE, FALSE, startTime, endTime);	
}

void AOLHero::PlayShadowedLeftArmAnim(const FName& animNameCamSpace, const FName& animNameShadow, FLOAT playRate, FLOAT blendInTime, FLOAT blendOutTime, FLOAT startTime, FLOAT endTime)
{
	if (LeftArmAnimSlot->bIsPlayingCustomAnim)
	{
		LeftArmAnimSlot->StopCustomAnim(0.1f);
		ShadowProxyLeftArmAnimSlot->StopCustomAnim(0.1f);
	}

	LeftArmAnimSlot->PlayCustomAnim(animNameCamSpace, playRate, blendInTime, blendOutTime, FALSE, FALSE, startTime, endTime);
	LeftArmAnimSlot->SetActorAnimEndNotification(TRUE);
	ShadowProxyLeftArmAnimSlot->PlayCustomAnim(animNameShadow, playRate, blendInTime, blendOutTime, FALSE, FALSE, startTime, endTime);	
}

void AOLHero::PlayCamSpaceAnim(const FName& animName, FLOAT playRate, FLOAT blendInTime, FLOAT blendOutTime, FLOAT startRatio)
{
	PlayUpperBodyAnim(animName, blendInTime, blendOutTime, playRate, startRatio);
}

void AOLHero::PlayUpperBodyAnim(const FName& animName, FLOAT blendInTime, FLOAT blendOutTime, FLOAT rate, FLOAT startRatio)
{
	if (UpperBodyBlendNode->bActive)
	{
		UpperBodyBlendNode->StartBlendingOut();
		ShadowProxyUpperBodyBlendNode->StartBlendingOut();
	}

	UpperBodyBlendNode->PlaySingleAnim(animName, blendInTime, blendOutTime, rate, startRatio);
	ShadowProxyUpperBodyBlendNode->PlaySingleAnim(animName, blendInTime, blendOutTime, rate, startRatio);
}

void AOLHero::PlayUpperBodyBlendedAnim(const FName& animNameA, const FName& animNameB, FLOAT weightA, FLOAT blendInTime, FLOAT blendOutTime, FLOAT rate, FLOAT startRatio)
{
	if (UpperBodyBlendNode->bActive)
	{
		UpperBodyBlendNode->StartBlendingOut();
		ShadowProxyUpperBodyBlendNode->StartBlendingOut();
	}

	UpperBodyBlendNode->PlayCustomBlend(animNameA, animNameB, weightA, blendInTime, blendOutTime, rate, startRatio);
	ShadowProxyUpperBodyBlendNode->PlayCustomBlend(animNameA, animNameB, weightA, blendInTime, blendOutTime, rate, startRatio);
}

void AOLHero::UpdateMeshOffset(FLOAT deltaTime)
{
	FLOAT targetOffset = 0.0f;

	if (LocomotionMode == LM_Walk || LocomotionMode == LM_Fall || SpecialMove == SMT_ClimbUpObstacle || SpecialMove == SMT_JumpOver || SpecialMove == SMT_BigLanding || SpecialMove == SMT_Dying)
	{
		// When running or during certain animations (e.g. uncrouch), the camera gets outside the bouding cylinder proper, so check ahead to see if there's an impending collision
		FCheckResult Hit(1.f);
		FVector startTrace = Location;
		startTrace.Z = EyeLocation.Z;
		FVector eyeFwd2d = EyeForward.SafeNormal2D();
		TWEAKABLE FLOAT SpeedLookahead = 0.15f; // Trace ahead the distance travelled during this time if keeping the same velocity
		FLOAT travDist = (RealVelocity | eyeFwd2d) * SpeedLookahead; 
		FVector endTrace = EyeLocation + (MinEyeDistToWall - MeshXOffset + travDist)*eyeFwd2d;

		if (!GWorld->SingleLineCheck( Hit, this, endTrace, startTrace, TRACE_AllBlocking, FVector(0.0f)))
		{
			FLOAT interpDist = (1.0f - Hit.Time) * endTrace.Distance(startTrace);
			targetOffset = -Min(interpDist, 30.0f);
		}
	}

	// Offset for enemies, if in a normal state and we don't have a shrunk collision (which could cause going through a wall)
	UBOOL bNormalCollision = (CylinderComponent->CollisionRadius >= 25.0f);
	if (bNormalCollision && LocomotionMode == LM_Walk && (SpecialMove == SMT_None || SpecialMove == SMT_RunThroughDoor || SpecialMove == SMT_ExitLocker))
	{
		TWEAKABLE FLOAT MaxOffsetForEnemy = 20.0f;
		TWEAKABLE FLOAT EnemyDistForMeshOffset = 150.0f;
		FLOAT closestEnemyDist = -1.0f;

		for (APawn* pawn = GWorld->GetWorldInfo()->PawnList; pawn != NULL; pawn = pawn->NextPawn)
		{
			AOLEnemyPawn* enemy = Cast<AOLEnemyPawn>(pawn);
			if (enemy && (enemy->Modifiers.bShouldAttack || enemy->Modifiers.bAttackOnProximity) && enemy->Location.DistanceSquared(Location) < Square(EnemyDistForMeshOffset))
			{			
				FVector toEnemy = enemy->Location - Location;
				if (((CharForward | toEnemy.SafeNormal2D()) > 0.5f) && Abs(toEnemy.Z) < 50.0f)
				{
					FLOAT distToEnemy = toEnemy.Size2D();

					if (closestEnemyDist < 0.0f || distToEnemy < closestEnemyDist)
					{
						closestEnemyDist = distToEnemy;
					}
				}
			}
		}

		if (closestEnemyDist > 0.0f)
		{
			FLOAT offset = MapClamped(closestEnemyDist, 60.0f, 150.0f, MaxOffsetForEnemy, 0.0f);
			targetOffset = Min(targetOffset, -offset);
		}
	}

	TWEAKABLE FLOAT ReloadingOffset = 20.0f;
	TWEAKABLE FLOAT ReloadingOffsetLocker = 13.0f;
	if (LocomotionMode == LM_Walk && IsReloading())
	{
		targetOffset = -ReloadingOffset;
	}
	else if (LocomotionMode == LM_Locker && IsReloading())
	{
		targetOffset = -ReloadingOffsetLocker;
	}

	TWEAKABLE FLOAT ApproachCoeff = 0.999999f;
	MeshXOffset = Utils::Approach(MeshXOffset, targetOffset, ApproachCoeff, deltaTime);

	if (Abs(MeshXOffset) < 0.01f)
	{
		MeshXOffset = 0.0f;
	}

	if (Abs(Mesh->Translation.X - MeshXOffset) > KINDA_SMALL_NUMBERF)
	{
		Mesh->Translation.X = MeshXOffset;
		Mesh->ConditionalUpdateTransform();
		ShadowProxy->Translation.X = MeshXOffset;
		ShadowProxy->ConditionalUpdateTransform();
	}
}

void AOLHero::UpdateHeatShielding(FLOAT deltaTime)
{
	UBOOL bInHeatVolume = FALSE;

	for (INT i = 0; i < Touching.Num(); i++)
	{	
		AOLHeatVolume* volume = Cast<AOLHeatVolume>(Touching(i));
		if (volume && volume->bEnabled)
		{
			bInHeatVolume = TRUE;
			break;
		}
	}

	FLOAT closestHeatSrcDist = -1.0f;
	FLOAT closestDamagingHeatDist = -1.0f;

	if (bInHeatVolume)
	{
		for (INT i = 0; i < CachedHeatMarkers.Num(); i++)
		{
			AOLHeatMarker* heatMarker = CachedHeatMarkers(i);
			if (heatMarker && heatMarker->IsValid())
			{
				if ((CharForward | (heatMarker->Location - Location).SafeNormal2D()) > 0.5f)
				{
					FLOAT distToMarker = heatMarker->Location.Distance(Location);
					closestHeatSrcDist = closestHeatSrcDist < 0.0f ? distToMarker : Min(distToMarker, closestHeatSrcDist);
					
					if (!heatMarker->bNoDamage)
					{
						closestDamagingHeatDist = closestDamagingHeatDist < 0.0f ? distToMarker : Min(distToMarker, closestDamagingHeatDist);
					}
				}
			}
		}
	}

	bHeatShielding = bInHeatVolume && closestHeatSrcDist > 0.0f;
	HeatDistance = closestHeatSrcDist;
	
	if (bInHeatVolume && closestDamagingHeatDist > 0.0f && closestDamagingHeatDist <= HeatDamageDist)
	{
		if (GWorld->GetTimeSeconds() > LastHeatDamageTime + HeatDamageInterval)
		{
			FLOAT dmg = HeatDamagePerSec * HeatDamageInterval;
			NativeTakeDamage((INT)dmg, NULL, Location, UOLDmgType_Fire::StaticClass());
			LastHeatDamageTime = GWorld->GetTimeSeconds();
		}
	}

	FLOAT targetHeatBlur = 0.0f;
	if (bHeatShielding && closestHeatSrcDist <= HeatMinBlurDist)
	{
		targetHeatBlur = MapClamped(closestHeatSrcDist, HeatMaxBlurDist, HeatMinBlurDist, 1.0f, HeatMinBlurAmount);
	}
	CurrentHeatBlur = Utils::Approach(CurrentHeatBlur, targetHeatBlur, targetHeatBlur > CurrentHeatBlur ? HeatBlurApproachCoeffIn : HeatBlurApproachCoeffOut, deltaTime);

	Utils::GetFXManager()->SetHeatBlur(CurrentHeatBlur);
}

void AOLHero::UpdateParrying(FLOAT deltaTime)
{
	TWEAKABLE FLOAT MinCosAngleForParry = 0.5f;
	TWEAKABLE FLOAT MaxDistanceForParry = 250.0f;

	FLOAT closestEnemyDist = -1.0f;
	FLOAT enemyRelYaw = 0.0f;
	AOLEnemyPawn* enemyPawn = NULL;

	if (LocomotionMode == LM_Walk && !bIsCrouched && !IsInCamcorderTransition() && SpecialMove == SMT_None)
	{
		for (AController* C = GWorld->GetWorldInfo()->ControllerList; C != NULL; C = C->NextController)
		{
			AOLBot* bot = Cast<AOLBot>(C);

			if (bot && bot->EnemyPawn && (bot->EnemyPawn->Modifiers.bShouldAttack || bot->EnemyPawn->Modifiers.bAttackOnProximity) && bot->EnemyPawn->Location.DistanceSquared(Location) < Square(MaxDistanceForParry))
			{			
				FVector toEnemy = bot->EnemyPawn->Location - Location;
				if (((CharForward | toEnemy.SafeNormal2D()) > MinCosAngleForParry) && Abs(toEnemy.Z) < 50.0f)
				{
					FLOAT distToEnemy = toEnemy.Size2D();

					if (closestEnemyDist < 0.0f || distToEnemy < closestEnemyDist)
					{
						closestEnemyDist = distToEnemy;
						enemyPawn = bot->EnemyPawn;
						enemyRelYaw = UNR_TO_DEG * FRotator::NormalizeAxis(toEnemy.Rotation().Yaw - Rotation.Yaw);
					}
				}
			}
		}

		if (enemyPawn && !bParrying)
		{
			// before we start parrying, make sure that there is no obstacle in the way (e.g. through a door)
			FCheckResult Hit(1.f);
			FVector startTrace = EyeLocation;
			FVector endTrace = enemyPawn->Location + VecZ(150.0f);
			UBOOL allClear = GWorld->SingleLineCheck( Hit, this, endTrace, startTrace, TRACE_AllBlocking, FVector(0.0f));

			if (!allClear && Hit.Actor != enemyPawn)
			{
				// Nope - something's in the way
				closestEnemyDist = -1.0f;
				enemyRelYaw = 0.0f;
				enemyPawn = NULL;
			}
		}
				
		bParrying = closestEnemyDist > 0.0f;
	}
	else
	{
		bParrying = FALSE;
	}

	if (ParryingAnimNode)
	{
		ParryingAnimNode->SetActive(bParrying);
		ParryingAnimNode->EnemyDistance = closestEnemyDist;
		ParryingAnimNode->EnemyRelYaw = enemyRelYaw;

		ShadowProxyParryingAnimNode->SetActive(bParrying);
		ShadowProxyParryingAnimNode->EnemyDistance = closestEnemyDist;
		ShadowProxyParryingAnimNode->EnemyRelYaw = enemyRelYaw;
	}
}


void AOLHero::UpdateFootPlacement(FLOAT deltaTime)
{
	TWEAKABLE FLOAT ApproachCoeffWalk = 0.995f;
	TWEAKABLE FLOAT ApproachCoeffOther = 0.9999f;

	const FLOAT MaxZOffset = 50.0f;

	FLOAT ApproachCoeff = (LocomotionMode == LM_Walk) ? ApproachCoeffWalk : ApproachCoeffOther;
	MeshZOffset = Utils::Approach(MeshZOffset, 0.0f, ApproachCoeff, deltaTime);
	MeshZOffset = Clamp(MeshZOffset, -MaxZOffset, MaxZOffset);

	if (Abs(MeshZOffset) < 0.01f)
	{
		MeshZOffset = 0.0f;
	}

	FLOAT thisFrameOffset = MeshZOffset - LastFrameMeshZOffset;
	LastFrameMeshZOffset = MeshZOffset;

	FLOAT targetOffset = BaseTranslationOffset + MeshZOffset;

	if (Abs(Mesh->Translation.Z - targetOffset) > KINDA_SMALL_NUMBERF)
	{
		Mesh->Translation.Z = targetOffset;
		Mesh->ConditionalUpdateTransform();
		ShadowProxy->Translation.Z = targetOffset;
		ShadowProxy->ConditionalUpdateTransform();
	}
	
	// early compensate for quick mesh movement - must do it now for this frame's render
	if (Abs(thisFrameOffset) > 2.0f)
	{
		if (IsCamcorderActive() && CamcorderMode == CCM_PoweredNightVision)
		{
			NVLightPowered->Translation.Z += thisFrameOffset;
			NVLightPowered->ConditionalUpdateTransform();
		}
		else if (IsCamcorderActive() && CamcorderMode == CCM_NightVision)
		{
			NVLightDefault->Translation.Z += thisFrameOffset;
			NVLightDefault->ConditionalUpdateTransform();
		}
		else
		{
			DarkLight->Translation.Z += thisFrameOffset;
			DarkLight->ConditionalUpdateTransform();
		}
	}

	if (LeftFootPlacementNode && RightFootPlacementNode)
	{
		TWEAKABLE FLOAT BlendTime = 0.1f;
		TWEAKABLE FLOAT ExagerateFactor = 1.5f;
		FLOAT realTargetWeight = -MeshZOffset / MaxZOffset;
		FLOAT nodeWeight = Saturate(ExagerateFactor * realTargetWeight);

		LeftFootPlacementNode->SetBlendTarget(nodeWeight, BlendTime);
		RightFootPlacementNode->SetBlendTarget(nodeWeight, BlendTime);

		ShadowProxyLeftFootPlacementNode->SetBlendTarget(nodeWeight, BlendTime);
		ShadowProxyRightFootPlacementNode->SetBlendTarget(nodeWeight, BlendTime);
	}
}

UBOOL AOLHero::IsPeeking() const
{ 
	if (LocomotionMode == LM_ContextualLean || SpecialMove == SMT_EnterContextualLean)
	{
		return TRUE;
	}
	else if (!appIsNearlyZero(CurrentLean))
	{
		return TRUE;
	}
	else if (LeanCrouchedAnimNode && LeanStandingAnimNode)
	{
		if (bIsCrouched)
		{
			return !appIsNearlyZero(LeanCrouchedAnimNode->CurrentRatio, KINDA_SMALL_NUMBERF);
		}
		else
		{
			return !appIsNearlyZero(LeanStandingAnimNode->CurrentRatio, KINDA_SMALL_NUMBERF);
		}
	}

	return FALSE;
}

UBOOL AOLHero::IsCornerPeeking() const
{
	return (LocomotionMode == LM_ContextualLean || SpecialMove == SMT_EnterContextualLean || SpecialMove == SMT_ExitContextualLean || SpecialMove == SMT_ExitContextualLeanForward || SpecialMove == SMT_ContextualLeanInsideTransition);
}

void AOLHero::UpdateCornerPeek(FLOAT deltaTime)
{
	 // Searches for nearby corners for contextual leans, hand placement, and obstacle avoidance

	TWEAKABLE FLOAT MinCosAngleFwdBroad = 0.5f;
	TWEAKABLE FLOAT MaxCosAngleBackSideBroad = 0.5f;			
	TWEAKABLE FLOAT MinDistFromEdgeSide = -50.0f;
	TWEAKABLE FLOAT MaxDistFromEdgeSide = 100.0f;
	TWEAKABLE FLOAT MinDistFromEdgeFwd = 20.0f;
	TWEAKABLE FLOAT MaxDistFromEdgeFwd = 100.0f;

	UBOOL bLockedInLean = IsCornerPeeking();
	
	if (!bLockedInLean)
	{
		CornerPeek.CornerMarker = NULL;

		if (LocomotionMode == LM_Walk && !bHeatShielding && !bIsCrouched)
		{		
			AOLCornerMarker* cornerMarker = NULL;
			FLOAT bestDistSq = -1.0f;

			// Loop through them all to find the closest one
			for (INT i = 0; i < CachedCorners.Num(); i++)
			{
				AOLCornerMarker* testCornerMarker = CachedCorners(i);

				if (!testCornerMarker)
				{
					continue;
				}

				FVector toCornerMarker = testCornerMarker->Location - Location;
						
				if (toCornerMarker.SizeSquared2D() > Square(500.0f) || Abs(toCornerMarker.Z) > 50.0f) // much bigger threshold than needed, just for a quick ignore on the majority case
				{
					// not even close
					continue;
				}

				if (!cornerMarker || toCornerMarker.SizeSquared() < bestDistSq)
				{
					cornerMarker = testCornerMarker;
					bestDistSq = toCornerMarker.SizeSquared();
				}
			}

			do // just to allow breaking out instead of indenting a dozen conditionnals
			{
				if (!cornerMarker)
				{
					break;
				}

				FVector toCornerMarker = cornerMarker->Location - Location;
			
				FVector eyeForward2D = EyeForward.SafeNormal2D();
				FVector localMarkerFwd = cornerMarker->Rotation.Vector();
				FVector localMarkerRight = cornerMarker->Rotation.Right();
			
				FVector peekFwd(0.0f); // towards the peeking direction
				FVector peekSide(0.0f); // from the corner along the wall we're pushing against
				FLOAT fwdPeekComp = 0.0f;
				FLOAT sidePeekComp = 0.0f;
				FVector cornerLocation(0.0f);
				CornerPeekPosition peekPosition = CPP_Left;

				if (cornerMarker->b3Sided)
				{
					// 3-sided

					// First search for an associated door, make sure it's open
					UBOOL bNoGo = FALSE;
					UBOOL bDisableFromLeft = FALSE;
					UBOOL bDisableFromRight = FALSE;

					for (INT j = 0; j < CachedDoors.Num(); j++)
					{
						AOLDoor * door = CachedDoors(j);

						if (!door || door->DoorBreakState == DBS_Broken)
						{
							continue;
						}

						if (door->GetOpenAngle() < 90.0f)
						{
							// closed door - both markers disabled
							if (door->GetStaticEdgeLocation().DistanceSquared(cornerMarker->Location) < Square(25.0f))
							{
								bNoGo = TRUE;
								break;
							}
							else if (door->GetPivotLocation().DistanceSquared(cornerMarker->Location) < Square(25.0f))
							{
								bNoGo = TRUE;
								break;
							}
						}
						else if (door->GetPivotLocation().DistanceSquared(cornerMarker->Location) < Square(25.0f))
						{
							// open door - inside pivot marker disabled

							if ((localMarkerRight | door->GetStaticDirection()) > 0.0f)
							{
								// door opens on marker right
								bDisableFromRight = TRUE;
							}
							else
							{
								bDisableFromLeft = TRUE;
							}

							break;
						}
					}

					if (bNoGo)
					{
						break;
					}

					const FVector& wallDir = localMarkerFwd;
					const FVector& frameRight = localMarkerRight;

					FVector leftCorner = cornerMarker->Location - 0.5f * cornerMarker->WallThickness * frameRight;
					FVector rightCorner = cornerMarker->Location + 0.5f * cornerMarker->WallThickness * frameRight;
					FVector toLeftCorner = (leftCorner - Location);
					FVector toRightCorner = (rightCorner - Location);
					FVector toLeftCornerDir = toLeftCorner.SafeNormal2D();
					FVector toRightCornerDir = toRightCorner.SafeNormal2D();

					FLOAT leftOnWall = (toLeftCornerDir | wallDir);
					FLOAT rightOnWall = (toRightCornerDir | wallDir);
					FLOAT leftOnFrame = (toLeftCornerDir | frameRight);
					FLOAT rightOnFrame = (toRightCornerDir | frameRight);

					UBOOL bConsiderLeft = (leftOnWall < 0.707f) && (leftOnFrame> 0.0f);
					UBOOL bConsiderRight = (rightOnWall < 0.707f) && (rightOnFrame < 0.0f);
					UBOOL bConsiderMiddle = (!bConsiderLeft && !bConsiderRight) && (leftOnWall > 0.0f && rightOnWall > 0.0f);

					if (bConsiderLeft || bConsiderRight)
					{
						if ((bConsiderLeft && bDisableFromLeft) || (!bConsiderLeft && bDisableFromRight))
						{
							// on the side of an open door hinge, abort
							break;
						}

						peekPosition = bConsiderLeft ? CPP_Left : CPP_Right;
						peekFwd = bConsiderLeft ? frameRight : -frameRight;
						peekSide = wallDir;
						fwdPeekComp = (eyeForward2D | peekFwd);
						sidePeekComp = (eyeForward2D | peekSide);										
						cornerLocation = bConsiderLeft ? leftCorner : rightCorner;
					}
					else if (bConsiderMiddle)
					{
						if (bDisableFromRight || bDisableFromLeft)
						{
							// in the middle of an open door hinge, abort (no room for the arm, even of the correct peek side)
							break;
						}

						if (leftOnFrame > 0.0f)
						{
							// middle left
							peekSide = -frameRight;
							cornerLocation = rightCorner;
							peekPosition = CPP_MiddleLeft;
						}
						else if (rightOnFrame < 0.0f)
						{
							// middle right
							peekSide = frameRight;
							cornerLocation = leftCorner;
							peekPosition = CPP_MiddleRight;
						}
						else
						{
							// in middle of frame
							break;
						}

						peekFwd = wallDir;
						fwdPeekComp = (eyeForward2D | peekFwd);
						sidePeekComp = (eyeForward2D | peekSide);
					}
					else
					{
						// in middle of frame on wrong side
						break;
					}
				
					if (fwdPeekComp <= MinCosAngleFwdBroad || sidePeekComp >= MaxCosAngleBackSideBroad)
					{
						// not looking in the correct direction
						break;
					}
				}
				else
				{
					// 2-sided

					const FVector& leftWallDir = localMarkerFwd;
					const FVector& rightWallDir = localMarkerRight;

					FLOAT rightPeekComp = (eyeForward2D | rightWallDir);
					FLOAT leftPeekComp = (eyeForward2D | leftWallDir);
				
					// First checks broad enough for all dependent systems (IK, contextual peek, obstacle avoidance)
					UBOOL bFromLeft = rightPeekComp > MinCosAngleFwdBroad && leftPeekComp < MaxCosAngleBackSideBroad;
					UBOOL bFromRight = leftPeekComp > MinCosAngleFwdBroad && rightPeekComp < MaxCosAngleBackSideBroad;

					if (!bFromLeft && !bFromRight)
					{
						// not looking in the correct direction
						break;
					}

					check (!bFromLeft || !bFromRight); // can't be both

					peekPosition = bFromLeft ? CPP_Left : CPP_Right;
					peekFwd = bFromLeft ? rightWallDir : leftWallDir; // towards the peeking direction
					peekSide = bFromLeft ? leftWallDir : rightWallDir; // from the corner along the wall we're pushing against
					fwdPeekComp = (eyeForward2D | peekFwd);
					sidePeekComp = (eyeForward2D | peekSide);
					cornerLocation = cornerMarker->Location;				
				}
			
				FVector toCorner = cornerLocation - Location;
				FLOAT distFwd = toCorner | peekFwd;
				FLOAT distSide = -(toCorner | peekSide);

				if (distSide < MinDistFromEdgeSide || distSide > MaxDistFromEdgeSide)
				{
					// not in range along the side axis
					break; 
				}

				if (distFwd < MinDistFromEdgeFwd || distFwd > MaxDistFromEdgeFwd)
				{
					// not in range along the forward axis
					break; 
				}

				// Valid corner marker - cache some data
				CornerPeek.CornerMarker = cornerMarker;
				CornerPeek.CornerLocation = cornerLocation;
				CornerPeek.FwdDir = peekFwd;
				CornerPeek.SideDir = peekSide;
				CornerPeek.PeekPosition = peekPosition;
				CornerPeek.bRoundedCorner = cornerMarker->bRoundedCorner;
			} while (0);
		}
	}

	TWEAKABLE FLOAT MinTimeBetweenPositions = 2.5f;
	TWEAKABLE FLOAT MinTimeBetweenMarkers = 0.5f;
	UBOOL bDifferentMarker = CornerPeek.CornerMarker && (LastValidCornerMarker != CornerPeek.CornerMarker);
	UBOOL bDifferentPosition = CornerPeek.CornerMarker && (LastValidCornerMarker == CornerPeek.CornerMarker) && (LastValidCornerPeekPosition != CornerPeek.PeekPosition);

	if (!bLockedInLean && (bDifferentMarker || bDifferentPosition) && (GWorld->GetTimeSeconds() - LastValidCornerPeekTime) <= (bDifferentMarker ? MinTimeBetweenMarkers : bDifferentPosition))
	{
		appMemZero(CornerPeek);
	}
	else if (CornerPeek.CornerMarker)
	{
		LastValidCornerMarker = CornerPeek.CornerMarker;
		LastValidCornerPeekPosition = CornerPeek.PeekPosition;
		LastValidCornerPeekTime = GWorld->GetTimeSeconds();
	}

	UpdateCornerHandIK(deltaTime);
}

void AOLHero::UpdateCornerHandIK(FLOAT deltaTime)
{
	if (deltaTime <= 0.001f)
	{
		return;
	}

	TWEAKABLE FLOAT DistForMaxStrength = 60.0f;
	TWEAKABLE FLOAT DistForExpectStrength = 70.0f;
	TWEAKABLE FLOAT DistForNoStrength = 130.0f;
	TWEAKABLE FLOAT ExpectStrength = 0.6f;
	TWEAKABLE FLOAT DistOutsideSoftThresh = 60.0f;
	TWEAKABLE FLOAT DistOutsideHardThresh = 70.0f;
	TWEAKABLE FLOAT DistInsideFromLeftSoftThresh = -10.0f;
	TWEAKABLE FLOAT DistInsideFromLeftHardThresh = -20.0f;
	TWEAKABLE FLOAT DistInsideFromRightSoftThresh = -5.0f;	
	TWEAKABLE FLOAT DistInsideFromRightHardThresh = -15.0f;
	TWEAKABLE FLOAT DistMinFwdSoftThresh = 25.0f;
	TWEAKABLE FLOAT DistMinFwdHardThresh = 15.0f;
	TWEAKABLE FLOAT DistMaxFwdSoftThresh = 100.0f;
	TWEAKABLE FLOAT DistMaxFwdHardThresh = 130.0f;
	TWEAKABLE FLOAT AngleDownSoftThresh = -65.0f;
	TWEAKABLE FLOAT AngleDownHardThresh = -70.0f;	
	TWEAKABLE FLOAT AngleInSoftThresh = -23.0f;
	TWEAKABLE FLOAT AngleInHardThresh = -25.0f;
	TWEAKABLE FLOAT AngleOutSoftThresh = 60.0f;
	TWEAKABLE FLOAT AngleOutHardThresh = 70.0f;
	TWEAKABLE FLOAT ShoulderDist = 18.0f;		

	TWEAKABLE FLOAT MaxScrewingAroundTime = 4.0f;
	TWEAKABLE FLOAT MinDisconnectedTime = 2.0f;
	TWEAKABLE FLOAT CriticalArmDistLow = 20.0f;
	TWEAKABLE FLOAT CriticalArmDistMed = 35.0f;
	TWEAKABLE FLOAT CriticalArmDistHigh = 45.0f;
	TWEAKABLE FLOAT CriticalDownPitchLow = -10.0f;
	TWEAKABLE FLOAT CriticalDownPitchMed = -30.0f;
	TWEAKABLE FLOAT CriticalDownPitchHigh = -40.0f;
	TWEAKABLE FLOAT StrengthApproachCoeffIn = 0.999f;
	TWEAKABLE FLOAT StrengthApproachCoeffTransition = 0.8f;
	TWEAKABLE FLOAT StrengthApproachCoeffOutNormal = 0.9f;
	TWEAKABLE FLOAT StrengthApproachCoeffOutFast = 0.99f;
	TWEAKABLE FLOAT StrengthApproachCoeffOutIdling = 0.6f;

	UBOOL bLockedInLean = IsCornerPeeking();
	UBOOL bWasAttached = appIsNearlyEqual(CornerPeek.IKStrength, 1.0f, 0.01f);
	UBOOL bWasActive = !appIsNearlyZero(CornerPeek.IKStrength, 0.01f);

	UBOOL bPeekingFromLeft = (CornerPeek.PeekPosition == CPP_Left || CornerPeek.PeekPosition == CPP_MiddleLeft);
	FVector toCorner = CornerPeek.CornerLocation - Location;
	FVector toCornerDir = toCorner.SafeNormal2D();
	FVector armLoc = Location - ShoulderDist*Rotation.Right();
	FVector armToCorner = (CornerPeek.CornerLocation - armLoc);
	FLOAT armDist = armToCorner.Size2D();
	FLOAT armToWallDist = (armToCorner | CornerPeek.FwdDir);
	FVector eyeForward2D = EyeForward.SafeNormal2D();
	FLOAT angleToFwd = RAD_TO_DEG * appAcos(eyeForward2D | CornerPeek.FwdDir); // positive to the outside (looking out), negative to the inside (looking in)
	UBOOL bLookingRightOfCorner = (CornerPeek.FwdDir | Rotation.Right()) < 0.0f;
	if ((bPeekingFromLeft && !bLookingRightOfCorner) || (!bPeekingFromLeft && bLookingRightOfCorner))
	{
		angleToFwd = -angleToFwd;
	}
	FLOAT distFwd = toCorner | CornerPeek.FwdDir;
	FLOAT distSide = -(toCorner | CornerPeek.SideDir);	
	
	FLOAT desiredStrength = 0.0f;
	{
		// Get the instant desired strength
	
		if (CornerPeek.CornerMarker == NULL || IsRunning() || IsInCamcorderTransition())
		{
			desiredStrength = 0.0f;
		}
		else if ((bPeekingFromLeft && CurrentLean < -0.05f) || (!bPeekingFromLeft && CurrentLean > 0.05f))
		{
			desiredStrength = 0.0f;
		}
		else if (bLockedInLean)
		{
			desiredStrength = 1.0f;
		}
		else
		{	
			FLOAT effectiveExpectStrength = ExpectStrength;
			FLOAT effectiveDistForExpectStrength = DistForExpectStrength;

			if (!bPeekingFromLeft)
			{
				effectiveExpectStrength = 0.5f;
				effectiveDistForExpectStrength = 65.0f;
			}

			FLOAT velStrength = (!bWasActive && RealVelocity.SizeSquared2D() >= Square(25.0f) && (RealVelocity.SafeNormal2D() | toCorner) < 0.707f) ? 0.0f : 1.0f; // don't activate if moving away from the corner
			FLOAT angleInStrength = Unlerp(angleToFwd, AngleInHardThresh, AngleInSoftThresh);
			FLOAT angleOutStrength = Unlerp(angleToFwd, AngleOutHardThresh, AngleOutSoftThresh);
			FLOAT angleDownStrength = Unlerp(Camera->ViewCS.Pitch, AngleDownHardThresh, AngleDownSoftThresh);
			FLOAT distOutStrength = Unlerp(distSide, DistOutsideHardThresh, DistOutsideSoftThresh);
			FLOAT distInStrength = bPeekingFromLeft ? Unlerp(distSide, DistInsideFromLeftHardThresh, DistInsideFromLeftSoftThresh) : Unlerp(distSide, DistInsideFromRightHardThresh, DistInsideFromRightSoftThresh);
			FLOAT distMinFwdStrength = Unlerp(distFwd, DistMinFwdHardThresh, DistMinFwdSoftThresh);
			FLOAT distMaxFwdStrength = Unlerp(distFwd, DistMaxFwdHardThresh, DistMaxFwdSoftThresh);	
			FLOAT armDistStrength = (armDist < DistForExpectStrength ? MapClamped(armDist, effectiveDistForExpectStrength, DistForMaxStrength, effectiveExpectStrength, 1.0f) : (armDist > DistForNoStrength ? 0.0f : effectiveExpectStrength));

			desiredStrength = velStrength;
			desiredStrength = Min(desiredStrength, angleInStrength);
			desiredStrength = Min(desiredStrength, angleOutStrength);
			desiredStrength = Min(desiredStrength, angleDownStrength);
			desiredStrength = Min(desiredStrength, distOutStrength);
			desiredStrength = Min(desiredStrength, distInStrength);
			desiredStrength = Min(desiredStrength, distMinFwdStrength);
			desiredStrength = Min(desiredStrength, distMaxFwdStrength);
			desiredStrength = Min(desiredStrength, armDistStrength);
		}
	}

	UBOOL bIsAttached = appIsNearlyEqual(desiredStrength, 1.0f, 0.01f);
	UBOOL bIsActive = !appIsNearlyZero(desiredStrength, 0.01f);
	UBOOL bActivatedInterp = (!bWasActive || bWasAttached) && bIsActive && !bIsAttached;
	FLOAT timeSinceInterpActivation = GWorld->GetTimeSeconds() - CornerPeek.LastInterpActivatedTime;

	if (bWasAttached && !bIsAttached)
	{
		CornerPeek.bDisconnecting = TRUE;
		CornerPeek.LastDisconnectTime = GWorld->GetTimeSeconds();
	}
	else if (!bIsActive || ((GWorld->GetTimeSeconds() - CornerPeek.LastDisconnectTime) > MinDisconnectedTime))
	{
		CornerPeek.bDisconnecting = FALSE; // reset so we can reconnect if we get in range again - this flag is just so that when we start putting the hand down, it fully goes down.
	}

	FLOAT criticalArmDist = 0.0f;

	if (Camera->ViewWS.Pitch < CriticalDownPitchHigh)
	{
		criticalArmDist = CriticalArmDistHigh;		
	}
	else if (Camera->ViewWS.Pitch < CriticalDownPitchMed)
	{
		criticalArmDist = MapClamped(Camera->ViewWS.Pitch, CriticalDownPitchMed, CriticalDownPitchHigh, CriticalArmDistMed, CriticalArmDistHigh);
	}
	else if (Camera->ViewWS.Pitch < CriticalDownPitchLow)
	{
		criticalArmDist = MapClamped(Camera->ViewWS.Pitch, CriticalDownPitchLow, CriticalDownPitchMed, CriticalArmDistLow, CriticalArmDistMed);
	}

	UBOOL bInCriticalRange = (armToWallDist < criticalArmDist);

	if (bInCriticalRange)
	{
		// don't let the arm idle - it would visibly interpenetrate the wall
		if (desiredStrength > 0.95f)
		{
			desiredStrength = 1.0f;
		}
		else
		{
			desiredStrength = 0.0f;
			bIsAttached = FALSE;
			bIsActive = FALSE;
		}
	}

	UBOOL bScrewingAround = !bIsAttached && bIsActive && bWasActive && timeSinceInterpActivation > MaxScrewingAroundTime;
	
	if (bScrewingAround || CornerPeek.bDisconnecting)
	{
		desiredStrength = 0.0f;
		bIsAttached = FALSE;
		bIsActive = FALSE;
	}	

	if (bActivatedInterp)
	{
		CornerPeek.LastInterpActivatedTime = GWorld->GetTimeSeconds();
	}

	// Update IK Strength
	{		
		FLOAT coeff = 1.0f;

		if (SpecialMove == SMT_ContextualLeanInsideTransition)
		{
			coeff = StrengthApproachCoeffTransition;
		}
		else if (desiredStrength > CornerPeek.IKStrength)
		{
			coeff = StrengthApproachCoeffIn;
		}
		else if (bInCriticalRange)
		{
			coeff = StrengthApproachCoeffOutFast;
		}
		else if (bScrewingAround)
		{
			coeff = StrengthApproachCoeffOutIdling;
		}
		else 
		{
			coeff = StrengthApproachCoeffOutNormal;
		}

		CornerPeek.IKStrength = Utils::Approach(CornerPeek.IKStrength, desiredStrength, coeff, deltaTime);

		if (CornerPeek.IKStrength < 0.01f)
		{
			CornerPeek.IKStrength = 0.0f;
		}	
	}

	FLOAT animPct = CornerPeek.IKStrength;

	// Setup IK anchors
	if (CornerPeek.IKStrength > 0.0f)
	{
		FRotationTranslationMatrix cornerMtx(CornerPeek.FwdDir.Rotation(), CornerPeek.CornerLocation);

		FMatrix effectorMtx;
		PeekingAnimNode->GetHandEffectorMtx(effectorMtx, cornerMtx, bPeekingFromLeft, CornerPeek.bRoundedCorner);
		CornerPeek.AnchorPos = effectorMtx.GetOrigin();
		CornerPeek.AnchorRot = effectorMtx.ToQuat();
		CornerPeek.JointTargetPos = bPeekingFromLeft ? CornerPeekJointTargetPosLeft : CornerPeekJointTargetPosRight;

		if (!bPeekingFromLeft)
		{
			CornerPeek.JointTargetPos = FVector(20.0f, -30.0f, 50.0f);
		}
	}

	// Setup contact anim
	if (LeftArmWallContactAnimSequence && CornerPeek.IKStrength > 0.0f)
	{
		FName animName = NAME_None;
		UBOOL bSoftEdge = CornerPeek.bRoundedCorner;
		if (bPeekingFromLeft && bSoftEdge)
		{
			animName = AnimNameWallContactLeftSoft;
		}
		else if (bPeekingFromLeft && !bSoftEdge)
		{
			animName = AnimNameWallContactLeftHard;
		}
		else if (!bPeekingFromLeft && bSoftEdge)
		{
			animName = AnimNameWallContactRightSoft;
		}
		else 
		{
			animName = AnimNameWallContactRightHard;
		}

		if (!LeftArmWallContactAnimSequence->AnimSeq || LeftArmWallContactAnimSequence->AnimSeqName != animName)
		{
			LeftArmWallContactAnimSequence->SetAnim(animName);			
		}
		check(LeftArmWallContactAnimSequence->AnimSeq);

		if (!ShadowProxyLeftArmWallContactAnimSequence->AnimSeq || ShadowProxyLeftArmWallContactAnimSequence->AnimSeqName != animName)
		{
			ShadowProxyLeftArmWallContactAnimSequence->SetAnim(animName);			
		}
		check(ShadowProxyLeftArmWallContactAnimSequence->AnimSeq);

		LeftArmWallContactAnimSequence->CurrentTime = animPct * LeftArmWallContactAnimSequence->AnimSeq->SequenceLength;
		LeftArmWallContactAnimSequence->PreviousTime = LeftArmWallContactAnimSequence->CurrentTime;
		LeftArmWallContactAnimSequence->ConditionalClearCachedData();

		ShadowProxyLeftArmWallContactAnimSequence->CurrentTime = animPct * ShadowProxyLeftArmWallContactAnimSequence->AnimSeq->SequenceLength;
		ShadowProxyLeftArmWallContactAnimSequence->PreviousTime = ShadowProxyLeftArmWallContactAnimSequence->CurrentTime;
		ShadowProxyLeftArmWallContactAnimSequence->ConditionalClearCachedData();
	}

	// Update filter anim nodes
	if (LeftArmWallContactFilterNode)
	{
		LeftArmWallContactFilterNode->SetBlendTarget(CornerPeek.IKStrength, 0.0f);
		ShadowProxyLeftArmWallContactFilterNode->SetBlendTarget(CornerPeek.IKStrength, 0.0f);
	}
}

void AOLHero::UpdateHandIK(FLOAT deltaTime)
{
	UBOOL bDebug = Utils::GetCheatManager() && Utils::GetCheatManager()->bDebugGameplay;

	TWEAKABLE UBOOL bLHIKEnabled = TRUE;

	FLOAT strengthLH = 0.0f;
	FVector posLH(0.0f);
	FRotator rotLH(0,0,0);
	FVector jointTargetLocLH = DefaultLeftHandJointTargetRotation;
	UBOOL bLHUseRotation = FALSE;

	UBOOL bCamcorderRaised = (BodySetup == HBS_CamcorderRaised || BodySetup == HBS_CamcorderRaisedNoShadow);
	UBOOL bCanSeeRightArmUp = (LocomotionMode == LM_Walk || LocomotionMode == LM_Fall || LocomotionMode == LM_LedgeWalk) && appIsNearlyZero(Camera->LookBackRatio);

	UBOOL bHideLeftArm = (bShouldHideLeftHandDuringSM);
	UBOOL bHideRightArm = (bShouldHideRightHandDuringSM) || (bCamcorderRaised && !bCanSeeRightArmUp);

	if (bLHIKEnabled)
	{
		if (bHideLeftArm || IsInCamcorderTransition())
		{
			strengthLH = 0.0f;
		}
		else if (CrouchTurnOnSpotAnimNode && LocomotionMode == LM_Walk && bIsCrouched && RealVelocity.SizeSquared2D() < Square(10.0f))
		{
			strengthLH = CrouchTurnOnSpotAnimNode->IKStrength * CrouchTurnOnSpotAnimNode->NodeTotalWeight;
			posLH = CrouchTurnOnSpotAnimNode->IKPosition;
			rotLH = FRotator(Mesh->LocalToWorldBoneAtom.GetRotation().Inverse() * CrouchTurnOnSpotAnimNode->IKRotationWS.Quaternion());
			bLHUseRotation = TRUE;
		}
		else if (LeftHandIKData.bActive)
		{
			FLOAT elapsedTime = GWorld->GetTimeSeconds() - LeftHandIKData.StartTime;

			if (elapsedTime > LeftHandIKData.Duration || (LeftHandIKData.IKTarget == IKTT_DoorKnob && !ActiveDoor))
			{
				// done
				LeftHandIKData.bActive = FALSE;
			}
			else 
			{
				FLOAT fadeInStrength = 1.0f;
				FLOAT fadeOutStrength = 1.0f;

				if (LeftHandIKData.FadeInTime > 0.0f && elapsedTime < LeftHandIKData.FadeInTime)
				{
					// fade in
					fadeInStrength = Utils::SmootherStep(elapsedTime / LeftHandIKData.FadeInTime);
				}
			
				if (LeftHandIKData.FadeOutTime > 0.0f && elapsedTime > (LeftHandIKData.Duration - LeftHandIKData.FadeOutTime))
				{
					// fade out
					fadeOutStrength = Utils::SmootherStep((LeftHandIKData.Duration - elapsedTime) / LeftHandIKData.FadeInTime);
				}

				strengthLH = Min(fadeInStrength, fadeOutStrength);

				if (LeftHandIKData.IKTarget == IKTT_DoorKnob)
				{
					FVector fwdVec = ActiveDoor->GetDynamicDirection();

					if ((fwdVec | CharForward) < 0.0f)
					{
						fwdVec = -fwdVec; // make sure this is the direction going through the door
					}

					FVector pivotToEdge = (ActiveDoor->GetEdgeLocation() - ActiveDoor->GetPivotLocation()).SafeNormal2D();

					posLH = ActiveDoor->GetKnobLocation() + LeftHandIKData.EffectorOffset.X*fwdVec + LeftHandIKData.EffectorOffset.Y*pivotToEdge + VecZ(LeftHandIKData.EffectorOffset.Z);
				}
				else if (LeftHandIKData.IKTarget == IKTT_CSAPropDestination)
				{
					if (ActiveCSA && ActiveCSA->AnimatedProp)
					{
						posLH = ActiveCSA->AnimatedProp->LocalToWorld().TransformFVector(LeftHandIKData.EffectorOffset);
					}
				}
			}
		}
		else if (CornerPeek.IKStrength > 0.01f)
		{
			strengthLH = CornerPeek.IKStrength;
			posLH = CornerPeek.AnchorPos;
			rotLH = FRotator(Mesh->LocalToWorldBoneAtom.GetRotation().Inverse() * CornerPeek.AnchorRot);
			jointTargetLocLH = CornerPeek.JointTargetPos;
			bLHUseRotation = TRUE;		
		}
	}

	UBOOL bLHActive = strengthLH > 0.01f;
	
	if (bLeftHandIKActive && !bLHActive)
	{
		LeftHandIK->SetSkelControlActive(FALSE);
		LeftForeTwistControl->SetSkelControlActive(FALSE);
		LeftForeTwist1Control->SetSkelControlActive(FALSE);
		LeftUpArmTwistControl->SetSkelControlActive(FALSE);
		ShadowProxyLeftHandIK->SetSkelControlActive(FALSE);
		ShadowProxyLeftForeTwistControl->SetSkelControlActive(FALSE);
		ShadowProxyLeftForeTwist1Control->SetSkelControlActive(FALSE);
		ShadowProxyLeftUpArmTwistControl->SetSkelControlActive(FALSE);

		bLeftHandIKActive = FALSE;
	}
	else if (bLHActive)
	{
		if (bDebug)
		{
			GWorld->GetWorldInfo()->DrawDebugSphere(LeftHandIK->EffectorLocation, 5.0f, 6, 255,255,0,FALSE);
		}

		if (!bLeftHandIKActive)
		{
			LeftHandIK->SetSkelControlActive(TRUE);
			LeftForeTwistControl->SetSkelControlActive(TRUE);
			LeftForeTwist1Control->SetSkelControlActive(TRUE);
			LeftUpArmTwistControl->SetSkelControlActive(TRUE);
			ShadowProxyLeftHandIK->SetSkelControlActive(TRUE);
			ShadowProxyLeftForeTwistControl->SetSkelControlActive(TRUE);
			ShadowProxyLeftForeTwist1Control->SetSkelControlActive(TRUE);
			ShadowProxyLeftUpArmTwistControl->SetSkelControlActive(TRUE);			
			bLeftHandIKActive = TRUE;
		}

		FLOAT twistStrength = Saturate(strengthLH / 0.5f);

		LeftHandIK->bUseEffectorRotationLS = bLHUseRotation;
		LeftHandIK->EffectorLocation = posLH;		
		LeftHandIK->EffectorRotation = rotLH;
		LeftHandIK->JointTargetLocation = jointTargetLocLH;	
		LeftHandIK->ControlStrength = strengthLH;
		LeftForeTwistControl->ControlStrength = twistStrength;
		LeftForeTwist1Control->ControlStrength = twistStrength;
		LeftUpArmTwistControl->ControlStrength = twistStrength;		
		ShadowProxyLeftHandIK->bUseEffectorRotationLS = bLHUseRotation;
		ShadowProxyLeftHandIK->EffectorLocation = posLH;		
		ShadowProxyLeftHandIK->EffectorRotation = rotLH;
		ShadowProxyLeftHandIK->JointTargetLocation = jointTargetLocLH;
		ShadowProxyLeftHandIK->ControlStrength = strengthLH;
		ShadowProxyLeftForeTwistControl->ControlStrength = twistStrength;
		ShadowProxyLeftForeTwist1Control->ControlStrength = twistStrength;
		ShadowProxyLeftUpArmTwistControl->ControlStrength = twistStrength;
	}	

	if (HiddenLeftArmControl)
	{
		HiddenLeftArmControl->SetSkelControlStrength(bHideLeftArm ? 1.0f : 0.0f, 0.0f);
	}
	
	UBOOL bWantRightHandIK = bCamcorderRaised && bCanSeeRightArmUp && !bHideRightArm;

	if (RightHandIK)
	{		
		if (!bWantRightHandIK && bRightHandIKActive)
		{
			RightHandIK->SetSkelControlActive(FALSE);
			RightForeTwistControl->SetSkelControlActive(FALSE);
			RightUpArmTwistControl->SetSkelControlActive(FALSE);
			ShadowProxyRightHandIK->SetSkelControlActive(FALSE);
			ShadowProxyRightForeTwistControl->SetSkelControlActive(FALSE);
			ShadowProxyRightUpArmTwistControl->SetSkelControlActive(FALSE);
			ShadowProxyRightClavicleFixup->SetSkelControlActive(FALSE);
			bRightHandIKActive = FALSE;
		}
		else if (bWantRightHandIK && !bRightHandIKActive)
		{
			RightHandIK->SetSkelControlActive(TRUE);
			RightForeTwistControl->SetSkelControlActive(TRUE);
			RightUpArmTwistControl->SetSkelControlActive(TRUE);
			ShadowProxyRightHandIK->SetSkelControlActive(TRUE);
			ShadowProxyRightForeTwistControl->SetSkelControlActive(TRUE);
			ShadowProxyRightUpArmTwistControl->SetSkelControlActive(TRUE);			
			ShadowProxyRightClavicleFixup->SetSkelControlActive(TRUE);
			bRightHandIKActive = TRUE;
		}		
	}

	if (HiddenRightArmControl)
	{
		HiddenRightArmControl->SetSkelControlStrength(bHideRightArm ? 1.0f : 0.0f, 0.0f);
	}
}

void AOLHero::ActivateLeftHandIK(IKTargetType targetType, FLOAT duration, FLOAT fadeInTime, FLOAT fadeOutTime, const FVector& knobOffset)
{
	checkSlow(!LeftHandIKData.bActive || (LeftHandIKData.IKTarget == targetType && appIsNearlyEqual(GWorld->GetTimeSeconds(), LeftHandIKData.StartTime, 0.1f))); 
	appMemZero(LeftHandIKData);
	LeftHandIKData.IKTarget = targetType;
	LeftHandIKData.FadeInTime = fadeInTime;
	LeftHandIKData.FadeOutTime = fadeOutTime;
	LeftHandIKData.Duration = duration;
	LeftHandIKData.bActive = TRUE;
	LeftHandIKData.StartTime = GWorld->GetTimeSeconds();
	LeftHandIKData.EffectorOffset = knobOffset;
}

void AOLHero::AttachCSAProp(FLOAT duration, FLOAT blendInTime, FLOAT blendOutTime, const FVector& positionOffset, const FRotator& orientationOffset, UBOOL bDiscardCurrentOffsets, UBOOL bHideWhenDone)
{
	if (ActiveCSA && ActiveCSA->AnimatedProp && ActiveCSA->AnimatedProp->StaticMeshComponent)
	{
		AttachProp(ActiveCSA->AnimatedProp->StaticMeshComponent, duration, blendInTime, blendOutTime, positionOffset, orientationOffset, bDiscardCurrentOffsets, bHideWhenDone);
	}
}

// positionOffset and orientationOffset are in component-space
void AOLHero::AttachProp(UPrimitiveComponent* comp, FLOAT duration, FLOAT blendInTime, FLOAT blendOutTime, const FVector& positionOffset, const FRotator& orientationOffset, UBOOL bDiscardCurrentOffsets, UBOOL bHideWhenDone)
{
	INT leftHandAuxBoneIdx = Mesh->MatchRefBone(LeftHandAuxBoneName);
	if (leftHandAuxBoneIdx != INDEX_NONE)
	{
		FVector relativeLoc(0.0f);
		FQuat relativeRot = FQuat::Identity;
				
		FBoneAtom leftAuxAtom = Mesh->GetBoneAtom(leftHandAuxBoneIdx);
		FBoneAtom inverseLH = leftAuxAtom.Inverse();

		if (bDiscardCurrentOffsets)
		{
			relativeLoc = positionOffset;
			relativeRot = orientationOffset.Quaternion();
		}
		else
		{
			FMatrix unscaledMtx = comp->LocalToWorld;
			unscaledMtx.ExtractScaling();

			unscaledMtx = FQuatRotationTranslationMatrix(-FQuat(orientationOffset), -positionOffset) * unscaledMtx;
			relativeLoc = inverseLH.TransformFVector(unscaledMtx.GetOrigin());
			relativeRot = inverseLH.GetRotation() * unscaledMtx.ToQuat();
		}

		comp->DetachFromAny();
		Mesh->AttachComponent(comp, LeftHandAuxBoneName, relativeLoc, relativeRot.Rotator());

		appMemZero(ActiveAttachment);
		ActiveAttachment.bActive = TRUE;
		ActiveAttachment.bAttached = TRUE;
		ActiveAttachment.AttachedComp = comp;
		ActiveAttachment.Duration  =  duration;		
		ActiveAttachment.BlendInTime= blendInTime;
		ActiveAttachment.BlendOutTime= blendOutTime;
		ActiveAttachment.BlendStartPos = relativeLoc;
		ActiveAttachment.BlendStartRot =  relativeRot;
		ActiveAttachment.bHideWhenDone = bHideWhenDone;
		ActiveAttachment.PositionOffset = positionOffset;
		ActiveAttachment.RotationOffset = orientationOffset;
		ActiveAttachment.StartTimestamp=GWorld->GetTimeSeconds();		
	}
}

void AOLHero::UpdateAttachments(FLOAT deltaTime)
{
	if (ActiveAttachment.bActive)
	{
		FLOAT elapsedTime = GWorld->GetTimeSeconds() - ActiveAttachment.StartTimestamp;

		if (ActiveAttachment.Duration > 0.0f && elapsedTime >= ActiveAttachment.Duration)
		{
			if (ActiveAttachment.bAttached)
			{
				// DONE - Detach, and set blend out params if appropriate

				if (ActiveCSA && ActiveCSA->AnimatedProp && ActiveCSA->AnimatedProp->StaticMeshComponent == ActiveAttachment.AttachedComp)
				{
					if (ActiveAttachment.BlendOutTime > 0.0f)
					{
						FRotationTranslationMatrix destActorMtx(ActiveCSA->AnimatedProp->Rotation, ActiveCSA->AnimatedProp->Location);
						FMatrix destActorMtxInv = destActorMtx.Inverse();
						FQuat invDestActorQuat(destActorMtxInv);
					
						FMatrix unscaledL2W = ActiveAttachment.AttachedComp->LocalToWorld;
						unscaledL2W.ExtractScaling();
						FVector relativeLoc = destActorMtxInv.TransformFVector(unscaledL2W.GetOrigin());
						FQuat relativeRot = invDestActorQuat * unscaledL2W.ToQuat();

						ActiveAttachment.AttachedComp->DetachFromAny();
						ActiveAttachment.AttachedComp->Translation = relativeLoc;
						ActiveAttachment.AttachedComp->Rotation = relativeRot.Rotator();
						ActiveAttachment.AttachedComp->BeginDeferredUpdateTransform();
						ActiveCSA->AnimatedProp->AttachComponent(ActiveCSA->AnimatedProp->StaticMeshComponent);

						ActiveAttachment.BlendStartPos = relativeLoc;
						ActiveAttachment.BlendStartRot = relativeRot;
					}
					else
					{
						ActiveAttachment.AttachedComp->DetachFromAny();
						ActiveAttachment.AttachedComp->Translation = FVector(0.0f);
						ActiveAttachment.AttachedComp->Rotation = FRotator(0,0,0);
						ActiveAttachment.AttachedComp->BeginDeferredUpdateTransform();
						ActiveCSA->AnimatedProp->AttachComponent(ActiveCSA->AnimatedProp->StaticMeshComponent);
					}
				}
				else 
				{
					ActiveAttachment.AttachedComp->DetachFromAny();
					ActiveAttachment.BlendOutTime = 0.0f; // make sure there's no blend out if we don't have a target
				}

				ActiveAttachment.bAttached = FALSE;
			}

			FLOAT elapsedBlendOutTime = elapsedTime - ActiveAttachment.Duration;
			if (ActiveAttachment.BlendOutTime > 0.0f && elapsedBlendOutTime < ActiveAttachment.BlendOutTime && ActiveCSA && ActiveCSA->AnimatedProp)
			{
				// Blending out
				FLOAT blendOutRatio = elapsedTime / ActiveAttachment.BlendOutTime;

				FVector relPos = blendOutRatio * ActiveAttachment.BlendStartPos;
				FQuat relQuat = SlerpQuat(ActiveAttachment.BlendStartRot, FQuat::Identity, blendOutRatio);
				relQuat.Normalize();

				ActiveAttachment.AttachedComp->Translation = relPos;
				ActiveAttachment.AttachedComp->Rotation = relQuat.Rotator();
				ActiveAttachment.AttachedComp->BeginDeferredUpdateTransform();
			}
			else
			{
				// Fully done - deactivate

				if (ActiveAttachment.AttachedComp)
				{
					ActiveAttachment.AttachedComp->Translation = FVector(0.0f);
					ActiveAttachment.AttachedComp->Rotation = FRotator(0,0,0);
					ActiveAttachment.AttachedComp->BeginDeferredUpdateTransform();

					if (ActiveAttachment.bHideWhenDone)
					{
						ActiveAttachment.AttachedComp->SetHiddenGame(TRUE);
					}
				}

				appMemZero(ActiveAttachment);
			}
		}
		else
		{
			if (ActiveCSA && ActiveCSA->AnimatedProp && ActiveCSA->AnimatedProp->StaticMeshComponent == ActiveAttachment.AttachedComp)
			{
				if (ActiveCSA->AnimatedProp->StaticMeshComponent->HiddenGame && elapsedTime > 0.0f)
				{
					ActiveCSA->AnimatedProp->StaticMeshComponent->SetHiddenGame(FALSE);
				}
			}

			FLOAT blendInRatio = 1.0f;
			if (ActiveAttachment.BlendInTime > 0.0f && elapsedTime < ActiveAttachment.BlendInTime)
			{
				blendInRatio = Min(elapsedTime / ActiveAttachment.BlendInTime, 1.0f);
			}

			FVector currentRelativePos(0.0f);
			FQuat currentRelativeQuat = FQuat::Identity;

			if (blendInRatio < 1.0f)
			{
				currentRelativePos = (1.0f - blendInRatio) * ActiveAttachment.BlendStartPos;
				currentRelativeQuat = SlerpQuat(ActiveAttachment.BlendStartRot, FQuat::Identity, blendInRatio);
				currentRelativeQuat.Normalize();
			}		

			currentRelativePos += ActiveAttachment.PositionOffset;
			currentRelativeQuat = (FQuat(ActiveAttachment.RotationOffset) * currentRelativeQuat); 

			// Update the relative offsets
			for (INT i = 0; i < Mesh->Attachments.Num(); i++)
			{
				if (Mesh->Attachments(i).Component == ActiveAttachment.AttachedComp)
				{
					Mesh->Attachments(i).RelativeLocation = currentRelativePos;
					Mesh->Attachments(i).RelativeRotation = currentRelativeQuat.Rotator();
					break;
				}
			}
		}
	}
}

void AOLHero::CrackCameraGlass()
{
	bCameraCracked = TRUE;
	Utils::GetFXManager()->CameraGlitch.NextGlitchDelay = 10.0f;
}

void AOLHero::ActivateWaterFootstepParticles(UBOOL bRightFoot)
{
	if (bRightFoot)
	{
		WaterFootstepParticlesRight->ActivateSystem();
	}
	else
	{
		WaterFootstepParticlesLeft->ActivateSystem();
	}
}


void AOLHero::OnEnterDeepWater(APhysicsVolume* waterVolume)
{
	TWEAKABLE FLOAT MinDownVelocity = -100.0f;
	if (Velocity.Z < MinDownVelocity)
	{
		WaterSplashParticles->Translation.Z = waterVolume->WaistDeepWaterHeight - Location.Z;
		WaterSplashParticles->ActivateSystem();
	}
}

UBOOL AOLHero::IsInMainMenu()
{
	return Utils::IsInMainMenu();
}

void AOLHero::GetFootstepDecalTransform(UBOOL bLeftFoot, FVector& decalLocation, FRotator& decalRotation)
{
	INT footBoneIdx = Mesh->MatchRefBone(bLeftFoot ? FName(TEXT("Hero-L-Foot")) : FName(TEXT("Hero-R-Foot")));
	check(footBoneIdx != INDEX_NONE);

	FMatrix footMtx = Mesh->GetBoneMatrix(footBoneIdx);

	FVector footFwd = -footMtx.GetAxis(1).SafeNormal2D();
	FVector footRight = footMtx.GetAxis(2).SafeNormal2D();

	TWEAKABLE FLOAT FwdOffsetDist = 7.0f;
	decalLocation = footMtx.GetOrigin() + FwdOffsetDist * footFwd;
	decalLocation.Z = Location.Z;

	decalRotation = FMatrix(-Floor, footRight, footFwd, FVector(0.0f)).Rotator();
}

//////////////////////////////////////////////////////////////////////////
// Sound
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void AOLHero::TriggerSoundEvent(UAkEvent* sndEvent)
{
	UAkAudioDevice * AudioDevice = UAkAudioDevice::Get();
	if( AudioDevice )
	{
		AudioDevice->PostEvent(sndEvent, this, NAME_None);
	}
}

void AOLHero::SetAudioState(const FString& stateName, const FString& newState)
{
	UAkAudioDevice* AudioDevice = UAkAudioDevice::Get();
	if (AudioDevice)
	{
		AudioDevice->SetState(*stateName, *newState);
	}
}

void AOLHero::SetAudioValue(const FString& valueName, FLOAT newValue)
{
	UAkAudioDevice* AudioDevice = UAkAudioDevice::Get();
	if (AudioDevice)
	{
		AudioDevice->SetRTPCValue(*valueName, newValue, NULL);
	}
}

void AOLHero::UpdateBreath(FLOAT deltaTime)
{
	FLOAT effectiveSpeed = Min(GroundSpeed, RealVelocity.Size2D()); // realvelocity to account for e.g. obstacles, groundspeed so that we don't freak out for ghost or thrown
	UBOOL bGoingFast = (RunSpeed > WalkSpeed + 40.0f) && effectiveSpeed > WalkSpeed + 0.95f*(RunSpeed - WalkSpeed);
	UBOOL bGoingSlow = effectiveSpeed < WalkSpeed + 0.05f*(RunSpeed - WalkSpeed);

	if (bGoingFast && !bPlayingRunSnd)
	{
		TriggerSoundEvent(SndStartRun);
		bPlayingRunSnd = TRUE;
	}
	else if (bGoingSlow && bPlayingRunSnd)
	{
		TriggerSoundEvent(SndStopRun);
		bPlayingRunSnd = FALSE;
	}
}

void AOLHero::UpdateRTPCs(FLOAT deltaTime)
{
	TWEAKABLE FLOAT MovingThresh = 5.0f;
	FLOAT targetVal = 0.0f;

	FLOAT realSpeed = RealVelocity.Size2D();

	if (realSpeed > MovingThresh)
	{
		targetVal = MapClamped(realSpeed, 0.0f, 450.0f, 50.0f, 100.0f);
	}
	else
	{
		targetVal = 0.0f;
	}

	UBOOL bAccel = targetVal > CurrentPlayerSpeedRTPC;

	CurrentPlayerSpeedRTPC = Utils::Approach(CurrentPlayerSpeedRTPC, targetVal, bAccel ? PlayerSpeedRTPCApproachUp : PlayerSpeedRTPCApproachDown, deltaTime);
	TargetPlayerSpeedRTPC = targetVal; // for debug info

	SetAudioValue(RTPCPlayerSpeed, CurrentPlayerSpeedRTPC);
}

//////////////////////////////////////////////////////////////////////////
// Health
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void AOLHero::NativeTakeDamage(INT damage, AController* instigatedBy, FVector hitLocation, UClass* damageType)
{	
	UBOOL bNoDmgLocoMode = (LocomotionMode == LM_Locker || LocomotionMode == LM_Bed);
	UBOOL bCanTakeDamage = (Health > 0 && damage > 0 && !bNoDmgLocoMode);

	if (bCanTakeDamage)
	{
		LastDamageTime = GWorld->GetTimeSeconds();
		LastDamageType = damageType;

		if (!OLPC->bGodMode)
		{
			PreciseHealth -= (FLOAT)damage;
		}

		if (LocomotionMode == LM_Cinematic)
		{
			// we can't die in a matinee
			PreciseHealth = Max(PreciseHealth, 1.0f);
		}

		AOLBot* botInstigator = Cast<AOLBot>(instigatedBy);

		UBOOL bSwarmAttacker = (botInstigator && botInstigator->EnemyPawn && botInstigator->EnemyPawn->IsA(AOLEnemyNanoCloud::StaticClass()));
		UBOOL bDmgTypeHasBlood = !bSwarmAttacker && (damageType == UOLDmgType_SoldierThrow::StaticClass() || damageType == UOLDmgType_SoldierPunch::StaticClass() || damageType == UOLDmgType_GenericHit::StaticClass());
		UBOOL bDmgTypeHasShake = (damageType == UOLDmgType_SoldierThrow::StaticClass() || damageType == UOLDmgType_SoldierPunch::StaticClass() || damageType == UOLDmgType_GenericHit::StaticClass() || (!bSwarmAttacker && damageType == UOLDmgType_Scripted::StaticClass()));
		
		if (bDmgTypeHasBlood)
		{
			BloodEffect->ActivateSystem();
			TriggerSoundEvent(SndSoldierHit);
		}

		if (bDmgTypeHasShake && !Camera->ShakeData.bActive)
		{
			TWEAKABLE FLOAT MaxShakeIntensity = 1.0;
			TWEAKABLE FLOAT ShakeDuration = 0.4f;
			UOLHeroCamera* DefaultCam = (UOLHeroCamera*)(Camera->GetClass()->GetDefaultActor());
			FCameraShakeData camShakeData = DefaultCam->ShakeData;			
			camShakeData.Intensity = MaxShakeIntensity * ((FLOAT)damage / 100.0f);
			camShakeData.Duration = ShakeDuration;
			camShakeData.bPositionless = TRUE;

			Camera->ActivateCameraShake(camShakeData, Location);
		}

		if (PreciseHealth < 25.0f)
		{
			SetAudioState(StateHitIntensityGroup, StateHitIntensityHigh);
		}
		else if (PreciseHealth < 60.0f)
		{
			SetAudioState(StateHitIntensityGroup, StateHitIntensityMed);
		}
		else
		{
			SetAudioState(StateHitIntensityGroup, StateHitIntensityLow);
		}	

		if (damageType == UOLDmgType_Electrified::StaticClass())
		{
			TriggerSoundEvent(SndHitElectrified);
		}
		else
		{
			TriggerSoundEvent(SndHit);
		}

		if (damageType != UOLDmgType_Fire::StaticClass())
		{
			Utils::GetFXManager()->TriggerBlur(1.0f, 1.5f, 0.7f, 0.0f, 1.5f);
		}

		if (damageType)
		{
			UForceFeedbackWaveform* ffWaveform = ((UDamageType*)damageType->GetDefaultObject())->DamagedFFWaveform;
			if (ffWaveform)
			{
				OLPC->eventClientPlayForceFeedbackWaveform(ffWaveform);
			}
		}

		if (damageType != UOLDmgType_Electrified::StaticClass() && damageType != UOLDmgType_NanoFog::StaticClass() && damageType != UOLDmgType_Fire::StaticClass())
		{
			// Bring us out of a special stance in case of an attack

			if (LocomotionMode == LM_Ladder)
			{
				TryDropFromLadder();
			}
			else if (LocomotionMode == LM_Door)
			{
				StopInteractiveOpen();
			}
			else if (LocomotionMode == LM_Pushing)
			{
				StopPushing();
			}
			else if (LocomotionMode == LM_LookBack)
			{
				TryStopLookBack();
			}

			if (SpecialMove != SMT_None)
			{
				switch (SpecialMove)
				{
				case SMT_JumpOnSpot:
				case SMT_Crouch:
				case SMT_Uncrouch:
				case SMT_EnterDoorInteraction:
				case SMT_OpenDoorInstant:
				case SMT_TryOpenLockedDoor:
				case SMT_OpenDoorPartial:
				case SMT_CloseDoor:
					{
						CancelSpecialMove();						
					}
					break;
				case SMT_EnterLookBack:
					{
						CancelSpecialMove();
						TryStopLookBack();
					}
					break;
				}
			}
		}
	}
}

void AOLHero::NativeTakeFallingDamage()
{
	FLOAT downSpeed = -Velocity.Z;
	if (downSpeed > 0.5f*FallSpeedForDamage)
	{
		if (downSpeed > FallSpeedForDamage)
		{			
			FLOAT damage = Saturate((downSpeed - FallSpeedForDamage) / (FallSpeedForDeath - FallSpeedForDamage));
			damage = appPow(damage, FallDamageExponent);
			NativeTakeDamage((INT)(damage * 100.0f), NULL, Location, UOLDmgType_Fell::StaticClass());
		}
	}
}

void AOLHero::TakeElectricDamage(INT dmg, FLOAT knockback, FVector hitNormal, UAkEvent* akEvent)
{
	if (GWorld->GetTimeSeconds() < LastElectricDamageTime + 1.0f)
	{
		// enforce a 1s delay before retrigger (guard against repeated Touch events)
		return;
	}

	LastElectricDamageTime = GWorld->GetTimeSeconds();

	if (SpecialMove == SMT_TryOpenLockedDoor)
	{
		CancelSpecialMove();
		if (FullBodyAnimSlot->bIsPlayingCustomAnim)
		{
			FullBodyAnimSlot->StopCustomAnim(0.1f);
			ShadowProxyFullBodyAnimSlot->StopCustomAnim(0.1f);
		}
	}

	NativeTakeDamage(dmg, NULL, Location, UOLDmgType_Electrified::StaticClass());
	ReactToHit(knockback, hitNormal);
	
	TWEAKABLE FLOAT OffsetFwd = 100.0f;
	FVector sparksPos = Location - OffsetFwd * hitNormal + VecZ(EyeLocation.Z - Location.Z);

	Utils::GetFXManager()->TriggerElectricSparks(sparksPos, hitNormal);

	if (akEvent)
	{
		PostAkEvent(akEvent);
	}
}

void AOLHero::ReactToHit(FLOAT hitStrength, FVector hitDirection, UBOOL bForceFullPower /*= FALSE*/)
{
	UBOOL bCanReact = (LocomotionMode == LM_Walk || 
		LocomotionMode == LM_Fall || 
		SpecialMove == SMT_Crouch || 
		SpecialMove == SMT_Uncrouch || 
		SpecialMove == SMT_BigLanding || 
		SpecialMove == SMT_JumpFromLedgeWalk || 
		SpecialMove == SMT_StopPushingObject || 
		SpecialMove == SMT_ExitLookBack);

	bCanReact = bCanReact && !bFailedCollisionSet;

	if (bCanReact)
	{
		if (IsReloading())
		{
			// the react anim will override the reload one. Kill the reload but set a pending flag
			if (CamcorderState == CCS_ReloadingActive)
			{
				TWEAKABLE FLOAT RaiseDelay = 1.0f;
				CamcorderDisabledEndTime = GWorld->GetTimeSeconds() + RaiseDelay;
			}
			CamcorderState = CCS_Inactive;
			SetBodySetup(HBS_Normal);
		}

		FLOAT fwdComp = hitDirection | CharForward;
		FLOAT rightComp = hitDirection | Rotation.Right();
		fwdComp /=  Abs(fwdComp) + Abs(rightComp); // normalize

		FName animFwd = NAME_None;
		FName animSide = NAME_None;

		if (bIsCrouched)
		{
			animFwd = fwdComp > 0.0f ? AnimNameHitReactionCrouchedFwd : AnimNameHitReactionCrouchedBwd;
			animSide = rightComp > 0.0f ? AnimNameHitReactionCrouchedRight : AnimNameHitReactionCrouchedLeft;
		}
		else
		{
			animFwd = fwdComp > 0.0f ? AnimNameHitReactionFwd : AnimNameHitReactionBwd;
			animSide = rightComp > 0.0f ? AnimNameHitReactionRight : AnimNameHitReactionLeft;
		}

		PlayUpperBodyBlendedAnim(animFwd, animSide, Abs(fwdComp), 0.1f, 0.25f);

		FLOAT impulseVel = (bIsCrouched && !bForceFullPower) ? ExternalImpulseMaxVelCrouched : ExternalImpulseMaxVel;
		ExternalImpulse = (hitStrength / 100.0f) * impulseVel * hitDirection;
	}
}

void AOLHero::SetHealth(FLOAT newHealth)
{
	if (newHealth < PreciseHealth)
	{
		LastDamageTime = GWorld->GetTimeSeconds();
		LastDamageType = UOLDmgType::StaticClass();
	}

	PreciseHealth = newHealth;
}

void AOLHero::UpdateHealth(FLOAT deltaTime)
{
	FLOAT deltaHealth = 0.0f;	

	// Hurt effect
	{
		FLOAT hurtEffectTarget = 0.0f;
		
		if (bElectrified)
		{
			hurtEffectTarget = 0.0f; // for now
		}
		else if (PreciseHealth < 50.0f)
		{
			hurtEffectTarget = 1.0f;
		}
		else if (PreciseHealth < 100.0f)
		{
			hurtEffectTarget = 1.0f - 0.02f*(PreciseHealth - 50.0f);
		}

		Utils::GetFXManager()->UpdateHurtEffect(deltaTime, hurtEffectTarget);
	}

	// Hobble
	{
		if (bHobbling)
		{
			FLOAT deltaHobble = TargetHobblingIntensity - HobblingIntensity;
			
			if (!appIsNearlyZero(deltaHobble, KINDA_SMALL_NUMBERF))
			{				
				FLOAT thisFrame = deltaTime * HobbleApproachRate;
				HobblingIntensity = Max(TargetHobblingIntensity, HobblingIntensity - thisFrame);
			}
		}
	}

	// Update health
	{
		if (PreciseHealth > 0.0f && PreciseHealth < (FLOAT)HealthMax)
		{
			if (GWorld->GetTimeSeconds() > (LastDamageTime + HealthRegenDelay))
			{
				// regen
				deltaHealth = HealthRegenRate * deltaTime;			
			}			
		}

		FLOAT prevHealth = PreciseHealth;
		INT prevHealthI = Health;

		PreciseHealth = Clamp(PreciseHealth + deltaHealth, 0.0f, (FLOAT)HealthMax);	
		Health = (INT)PreciseHealth;

		// Sound update
		if (Health != prevHealthI)
		{
			SetAudioValue(RTPCHealth, PreciseHealth);

			if (prevHealthI == HealthMax)
			{
				// first dip below 100
				TriggerSoundEvent(SndStartDamage);
			}
			else if (Health == HealthMax)
			{
				// fully regen'd
				TriggerSoundEvent(SndStopDamage);
			}
		}

	}

	// Check Death
	if (Health == 0 && TimeOfDeath == 0.f)
	{
		Die();
	}
}

void AOLHero::DecapitatedNotify()
{
	DecapitatedBloodEffect->ConditionalDetach();
	Mesh->AttachComponent(DecapitatedBloodEffect, FName(TEXT("Hero-Neck")));
	DecapitatedBloodEffect->ActivateSystem();
}

void AOLHero::BloodOnScreenNotify()
{
	DeathParticles->ConditionalDetach();
	Mesh->AttachComponent(DeathParticles, Utils::GetCameraBoneName());
	DeathParticles->ActivateSystem();
}

void AOLHero::AttachCameraEffect(UParticleSystem* particleEffectTemplate, FLOAT duration, FLOAT planeDist)
{
	GenericCameraEffect->ConditionalDetach();
	GenericCameraEffect->SetTemplate(particleEffectTemplate);
	AttachComponent(GenericCameraEffect);
	GenericCameraEffect->ActivateSystem();

	bCameraEffectActive = TRUE;
	CameraEffectEndTime = GWorld->GetTimeSeconds() + duration;
	CameraEffectPlaneDist = planeDist;
}

void AOLHero::DieNotify()
{
	UBOOL bInStruggle = (SpecialMove == SMT_KilledInStruggle);

	if (bInStruggle)
	{
		UAnimNodeSequence* animNodeSeq = FullBodyAnimSlot->GetCustomAnimNodeSeq();
		if (animNodeSeq)
		{
			animNodeSeq->Rate = 0.001f;
		}
	}

	UBOOL bValidDeath = TimeOfDeath <= 0.0f && (SpecialMove == SMT_KilledInStruggle || SpecialMove == SMT_HeroDecapitate || SpecialMove == SMT_HeroKilled || SpecialMove == SMT_Dying);

	if (bValidDeath)
	{
		Die();
	}

	if (bInStruggle)
	{
		OLPC->StruggleExitCompleted();
	}
}

void AOLHero::Die()
{
	PreciseHealth = 0.0f;
	Health = 0;

	if (bElectrified)
	{
		TriggerSoundEvent(SndElectricHitStop);
	}

	if (CamcorderState != CCS_Inactive)
	{
		DeactivateCamcorder();

		if (SpecialMove == SMT_HeroKilled)
		{
			// last minute hack - camcorder isn't positionned right - hide it
			SetBodySetup(HBS_Normal);
		}
		CamcorderState = CCS_Inactive;
	}

	TriggerSoundEvent(SndStopDamage);
	SetAudioValue(RTPCHealth, 100.0f);

	UAkAudioDevice * audioDevice = UAkAudioDevice::Get();
	if (audioDevice)
	{
		audioDevice->StopAllSounds();
		audioDevice->PostEvent(SndDieMusicEvent, NULL, NAME_None);
	}

	UOLVoiceManager* VOManager = Utils::GetOLGame() ? Utils::GetOLGame()->VoiceManager : NULL;
	if (VOManager)
	{
		VOManager->ClearAllVOQueues();
	}
	
	TriggerSoundEvent(SndDie);		

	Utils::GetFXManager()->SetPPS(PPS_Death);
	TimeOfDeath = GWorld->GetTimeSeconds();

	if (SpecialMove != SMT_HeroKilled && SpecialMove != SMT_HeroDecapitate)
	{
		DeathParticles->ConditionalDetach();
		Mesh->AttachComponent(DeathParticles, Utils::GetCameraBoneName());
		DeathParticles->ActivateSystem();		

		if (SpecialMove != SMT_KilledInStruggle || OLPC->Struggle.Config.KilledAnimPlayer == NAME_None)
		{
			StartSpecialMove(SMT_Dying);
		}
	}

#if DINGO
	GOLDingo->EventPlayerDied(Utils::GetCurrentCheckpointName(), Utils::GetOLGame()->DifficultyMode, Location);
#endif

	OLPC->eventClientPlayForceFeedbackWaveform(((UOLDmgType*)UOLDmgType::StaticClass()->GetDefaultObject())->KilledFFWaveform);
	OLPC->eventClientSetCameraFade(TRUE, FColor(0, 0, 0), FVector2D(0.0f, 1.0f), 5.0f, TRUE);
	Utils::GetFXManager()->TriggerBlur(1.0f, 5.0f, 0.7f, 4.0f, 0.0f);
}


//////////////////////////////////////////////////////////////////////////
// Systems
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void AOLHero::NativePostBeginPlay()
{
	Super::NativePostBeginPlay();

	if (!HasAnyFlags(RF_ClassDefaultObject))
	{
		DefaultHero = (AOLHero*)(GetClass()->GetDefaultActor());

		CurrentBatterySetEnergy = 1.0f;
		PreciseHealth = (FLOAT)HealthMax;

		EyeRotation = Rotation;
		CurrentFOV = DefaultFOV;

		EyeForward = EyeRotation.Vector();
		CharForward = Rotation.Vector();

		Camera = ConstructObject<UOLHeroCamera>(UOLHeroCamera::StaticClass(), this);
		Camera->Init(this);

		CachedObjectsPos = FVector(-100000.0f);

		BloodEffect->ConditionalDetach();
		Mesh->AttachComponent(BloodEffect, Utils::GetCameraBoneName());

		HeadMesh->DetachFromAny();
		ShadowProxy->AttachComponent(HeadMesh, HeadBoneName);
		CameraMesh->ConditionalDetach(TRUE);
		CameraMesh->SetHiddenGame(TRUE);
		Mesh->AttachComponent(CameraMesh, RightHandAuxBoneName);	
		CameraMeshShadowProxy->ConditionalDetach(TRUE);
		CameraMeshShadowProxy->bCastHiddenShadow = FALSE;
		CameraMeshShadowProxy->SetHiddenGame(TRUE);
		ShadowProxy->AttachComponent(CameraMeshShadowProxy, RightHandAuxBoneName);	

		WaterFootstepParticlesLeft->ConditionalDetach();
		Mesh->AttachComponent(WaterFootstepParticlesLeft, FName(TEXT("Hero-L-Foot")));

		WaterFootstepParticlesRight->ConditionalDetach();
		Mesh->AttachComponent(WaterFootstepParticlesRight, FName(TEXT("Hero-R-Foot")));

		WaterSplashParticles->ConditionalDetach();
		Mesh->AttachComponent(WaterSplashParticles, FName(TEXT("Hero-Root")));
				
		CamcorderScreenLight->ConditionalDetach(TRUE);
		Mesh->AttachComponent(CamcorderScreenLight, RightHandAuxBoneName, FVector(0.0f, -10.0f, 2.5f));	
		CamcorderScreenLight->SetEnabled(FALSE);

		const FLOAT pitchCorr = 90.0f;
		const FLOAT yawCorr = 90.0f;
		const FLOAT rollCorr = -90.0f;
		FRotator correctionRot(pitchCorr * DEG_TO_UNR, yawCorr * DEG_TO_UNR, rollCorr * DEG_TO_UNR);
		HeadMesh->Rotation = correctionRot;

		Utils::GetFXManager()->SetPPS(PPS_Default);
		Utils::GetFXManager()->ResetEffects();

		LocomotionMode = LM_Fall; // force a transition so the EnterLocomotionMode below actually does something
		EnterLocomotionMode(LM_Walk);

		GenerateAIPositions();

		RunStartedTime = -1.0f;

		CurrentDarkLightRadius = DarkLightRadiusDefault;
		CurrentDarkLightBrightness = DarkLightBrightnessDefault;

		ApplyCheckpointState(Utils::GetCurrentCheckpointName());

		if (CameraScreenMat)
		{
			CameraScreenMat->SetScalarParameterValue(CameraScreenParamName, 0.0f);
		}
	}
}

void AOLHero::BuildAnimSetList()
{
	// This will be done only once, the first time.
	if (Mesh)
	{
		Mesh->SaveAnimSets();
	}

	if ( ShadowProxy )
	{
		ShadowProxy->SaveAnimSets();
	}

	// Add the AnimSets from Matinee
	for(INT i=0; i<InterpGroupList.Num(); i++)
	{
		UInterpGroup* AnInterpGroup = InterpGroupList(i);
		if( AnInterpGroup )
		{
			AddAnimSets( AnInterpGroup->GroupAnimSets );
		}
	}

	if (OLPC && OLPC->Struggle.Config.HeroAnimSets.Num() > 0)
	{
		AddAnimSets(OLPC->Struggle.Config.HeroAnimSets);
	}
}

void AOLHero::AddAnimSets(const TArray<class UAnimSet*>& CustomAnimSets)
{
	for(INT i=0; i<CustomAnimSets.Num(); i++)
	{
		Mesh->AnimSets.AddItem( CustomAnimSets(i) );
		ShadowProxy->AnimSets.AddItem( CustomAnimSets(i) );
	}
}

void AOLHero::ApplyCheckpointState(const FName checkpointName)
{
}

void AOLHero::ResetAfterTeleport()
{
	EnterLocomotionMode(LM_Fall);
	ActiveSqueeze = NULL;
	ActiveLedge = NULL;

	if (ActiveDoor != NULL)
	{
		ActiveDoor->DoorUser = NULL;
	}

	ActiveDoor = NULL;
	ActiveLocker = NULL;
	ActiveBed = NULL;
	ActivePickup = NULL;
	ActivePushable = NULL;
	SpecialMove = NULL;
	Velocity = FVector(0.0f);
	bBothHandsNeeded = FALSE;
	Utils::GetFXManager()->KillBlur();
	ExternalImpulse = FVector(0.0f);
	appMemZero(CornerPeek);
	RunStartedTime = -1.0f;
	bHeatShielding = FALSE;
	bParrying = FALSE;

	UOLSoundEmitter* sndEmitter = Utils::GetSoundEnvManager()->GetSoundEmitterForActor(this);
	if (sndEmitter)
	{
		sndEmitter->bDirty = TRUE;
	}
}

void AOLHero::OnPossess()
{
	if (!OLPC)
	{
		return; // happens sometimes when quickly swapping between checkpoints
	}

	AOLGame* olGame = Utils::GetOLGame();
	check(olGame);
	
	INT defaultNbBats = (olGame->DifficultyMode == EDM_Normal) ? OLPC->DefaultNumBatteries : 1;

	if (olGame->DifficultyMode == EDM_Nightmare)
	{
		OLPC->NumBatteries = OLPC->NumBatteriesAtLastCheckpoint;
		CurrentBatterySetEnergy = OLPC->BatteryEnergyAtLastCheckpoint;
	}
	else
	{
		OLPC->NumBatteries = Max(defaultNbBats, OLPC->NumBatteries);
		CurrentBatterySetEnergy = 1.0f;
	}

	OLPC->NumBatteries = Min(OLPC->NumBatteries, OLPC->MaxNumBatteries); // recheck against max, in case user changed to a more difficult setting on a save with more than the max at last checkpoint
}

void AOLHero::TickPrePhysics(FLOAT deltaTime)
{
	UBOOL dying = (SpecialMove == SMT_Dying);

	Super::TickPrePhysics(deltaTime);

	if (!OLPC || dying)
	{
		return;
	}

	MovingNoiseTimer += deltaTime;

	ProcessRotation(deltaTime);	
}

void AOLHero::TickPostPhysics(FLOAT deltaTime)
{
	UBOOL dying = (SpecialMove == SMT_Dying);

	Super::TickPostPhysics(deltaTime);

	if (!OLPC || dying)
	{
		return;
	}

	UpdateCachedObjects();

	UpdateHealth(deltaTime);	
	UpdateFootPlacement(deltaTime);
	UpdateMeshOffset(deltaTime);
	UpdateCornerPeek(deltaTime);
	UpdateHandIK(deltaTime);
	UpdateAttachments(deltaTime);
	UpdateBreath(deltaTime);
	UpdateCamcorder(deltaTime);
	UpdateHeatShielding(deltaTime);
	UpdateParrying(deltaTime);
	UpdateRecording(deltaTime);
	UpdateAIPositions(deltaTime);
	UpdateElectricGlitch(deltaTime);
	UpdateAvgVelocity(deltaTime);
	UpdateRTPCs(deltaTime);
	
	if (bFailedCollisionSet && LocomotionMode != LM_SpecialMove)
	{
		bFailedCollisionSet = !TryAdjustCollisionSizeForLocomotionMode((ELocomotionMode)LocomotionMode);
	}

	switch (LocomotionMode)
	{
	case LM_LedgeWalk:
		if (!ActiveLedge|| !Utils::IsBetweenMarkers(Location, ActiveLedge->Location, ActiveLedge->Next->Location, DefaultHero->CylinderComponent->CollisionRadius))
		{
			TryExitLedgeWalk();
		}
		break;
	case LM_Door:
		{
			UpdateDoorInteraction();
		}
		break;
	case LM_Walk:
		{
			if (Velocity.IsNearlyZero())
			{
				if (!MovingNoiseActive)
				{
					MovingNoiseTimer = 0.f;
				}
				else if (MovingNoiseTimer >= MovingNoiseClearTime)
				{
					MovingNoiseTimer = 0.f;
					MovingNoiseActive = FALSE;
				}
			}
			else
			{
				if (!MovingNoiseActive && MovingNoiseTimer >= MovingNoiseStartTime)
				{
					MovingNoiseActive = TRUE;
				}

				if (MovingNoiseActive && MovingNoiseTimer >= MovingNoiseRate)
				{
					if (IsRunning())
					{
						if (bHobbling)
						{
							MakeNoise(HobblingRunLoudness, WalkingNoise);
						}
						else
						{
							MakeNoise(RunningLoudness, RunningNoise);
						}
					}
					else if (bIsCrouched)
					{
						if (IsInWaterVolume() || GetMaterialBelowFeet() == WaterMaterial)
						{
							MakeNoise(CrouchWaterLoudness, CrouchNoise);
						}
						else
						{
							MakeNoise(CrouchLoudness, CrouchNoise);
						}
					}
					else if (IsInWaterVolume() || GetMaterialBelowFeet() == WaterMaterial)
					{
						MakeNoise(WalkingWaterLoudness, WalkingNoise);
					}
					else if (bHobbling)
					{
						MakeNoise(HobblingWalkLoudness, WalkingNoise);
					}
					else
					{
						MakeNoise(WalkingLoudness, WalkingNoise);
					}

					MovingNoiseTimer = 0.0f;
				}
			}

			if (IsRunning())
			{
				if (RunStartedTime <= 0.0f)
				{
					RunStartedTime = GWorld->GetTimeSeconds();
				}
			}
			else
			{
				RunStartedTime = -1.0f;
			}
		}
		break;
	case LM_Bed:
		{
			UpdateBedAnimation(deltaTime);
		}
		break;
	case LM_Pushing:
		{
			if (!ActivePushable || (bPushingFromBackEdge && !ActivePushable->CanPushFwd()) || (!bPushingFromBackEdge && !ActivePushable->CanPushBack()))
			{
				StopPushing();
			}
		}
		break;
	}

	if (HeroControl)
	{
		UpdateHeroControl(deltaTime);
	}

	if (Physics == PHYS_Falling && LocomotionMode != LM_Fall)
	{
		EnterLocomotionMode(LM_Fall);
	}
	else if (LocomotionMode == LM_Fall && Physics != PHYS_Falling)
	{
		EnterLocomotionMode(LM_Walk);
	}

	SyncShadowProxy();

	if (Utils::GetCheatManager() && Utils::GetCheatManager()->bDebugTrajectory)
	{
		FDebugTrajectoryPoint point;
		point.Position = EyeLocation;
		point.Speed = RealVelocity.Size();
		point.TimeStamp = GWorld->GetTimeSeconds();
		point.PointType = DTT_Camera;
		Utils::GetCheatManager()->AddDebugTrajectoryPoint(point);
	}
}

void AOLHero::performPhysics(FLOAT deltaSeconds)
{
	TWEAKABLE FLOAT EyeSmoothZThreshold = 5.0f;
	FLOAT previousZ = Location.Z;
	BYTE previousPhysics = Physics;
	
	if (Physics == PHYS_Falling && RealVelocity.SizeSquared() < Square(25.0f) && Velocity.SizeSquared() < Square(25.0f))
	{
		AirControl = 0.5f; // failsafe to help the player get unstuck from some collision bugs where he can't get to ground
	}
	else
	{
		AirControl = 0.0f;
	}

	if (!IsPlayerInputEnabled())
	{
		Acceleration = FVector(0.0f);
	}

	Super::performPhysics(deltaSeconds);

	FLOAT deltaZ = Location.Z - OldZ;

	TWEAKABLE FLOAT MaxTimeOnSlopeForAdjustZ = 1.0f;
	TWEAKABLE FLOAT LargeSlopeZ = 0.96f;

	UBOOL bLargeSlope = FALSE;

	if (Floor.Z > LargeSlopeZ)
	{
		bLargeSlope = FALSE;
		LargeSlopeStartedTime = -1.0f;
	}
	else if (LargeSlopeStartedTime <= 0.0f)
	{
		bLargeSlope = TRUE;
		LargeSlopeStartedTime = GWorld->GetTimeSeconds();
	}

	if (previousPhysics == PHYS_Walking && Physics == PHYS_Walking && Abs(deltaZ) > EyeSmoothZThreshold && (!bLargeSlope || (GWorld->GetTimeSeconds() < LargeSlopeStartedTime + MaxTimeOnSlopeForAdjustZ)))
	{
		MeshZOffset -= deltaZ;
	}
}

void AOLHero::SetBodySetup(EHeroBodySetup bodySetup)
{
	UBOOL bMainCastShadow = FALSE;
	UBOOL bShadowProxyCastHiddenShadow = TRUE;
	UBOOL bCamVisible = FALSE;
	UBOOL bCamCastShadow = FALSE;
	UBOOL bCamShadowProxyCastHiddenShadow = FALSE;

	// Special case - always use HBS_NoProxy during cinematics
	if (OLPC->bCinematicMode)
	{
		bodySetup = HBS_NoProxy;
	}

	switch(bodySetup)
	{
	case HBS_Normal:
		bMainCastShadow = FALSE;
		bShadowProxyCastHiddenShadow = TRUE;
		bCamVisible = FALSE;
		bCamCastShadow = FALSE;
		bCamShadowProxyCastHiddenShadow = FALSE;
		break;
	case HBS_NoProxy:
		bMainCastShadow = TRUE;
		bShadowProxyCastHiddenShadow = FALSE;
		bCamVisible = FALSE;
		bCamCastShadow = FALSE;
		bCamShadowProxyCastHiddenShadow = FALSE;
		break;
	case HBS_CamcorderRaised:
		bMainCastShadow = FALSE;
		bShadowProxyCastHiddenShadow = TRUE;
		bCamVisible = FALSE;
		bCamCastShadow = FALSE;
		bCamShadowProxyCastHiddenShadow = TRUE;
		break;
	case HBS_CamcorderRaisedNoShadow:
		bMainCastShadow = FALSE;
		bShadowProxyCastHiddenShadow = FALSE;
		bCamVisible = FALSE;
		bCamCastShadow = FALSE;
		bCamShadowProxyCastHiddenShadow = FALSE;
		break;
	case HBS_CamcorderVisible:
		bMainCastShadow = FALSE;
		bShadowProxyCastHiddenShadow = TRUE;
		bCamVisible = TRUE;
		bCamCastShadow = FALSE;
		bCamShadowProxyCastHiddenShadow = TRUE;
		break;
	}

	if (bodySetup == HBS_NoProxy && HeadMesh->ShadowParent == ShadowProxy)
	{
		HeadMesh->DetachFromAny();
		HeadMesh->SetShadowParent(Mesh);
		Mesh->AttachComponent(HeadMesh, HeadBoneName);
	}
	else if (bodySetup != HBS_NoProxy && HeadMesh->ShadowParent == Mesh)
	{
		HeadMesh->DetachFromAny();
		HeadMesh->SetShadowParent(ShadowProxy);
		ShadowProxy->AttachComponent(HeadMesh, HeadBoneName);
	}

	if (bMainCastShadow != Mesh->CastShadow)
	{
		FComponentReattachContext ReattachContextMesh(Mesh);
		Mesh->CastShadow = bMainCastShadow;
	}

	if (bShadowProxyCastHiddenShadow != ShadowProxy->bCastHiddenShadow)
	{
		FComponentReattachContext ReattachContextSP(ShadowProxy);
		ShadowProxy->bCastHiddenShadow = bShadowProxyCastHiddenShadow;
	}
	
	if (HeadMesh->CastShadow != (bMainCastShadow || bShadowProxyCastHiddenShadow))
	{
		FComponentReattachContext ReattachContextMesh(HeadMesh);
		HeadMesh->CastShadow = (bMainCastShadow || bShadowProxyCastHiddenShadow);
	}

	if (bCamCastShadow != CameraMesh->CastShadow)
	{
		FComponentReattachContext ReattachContextCM(CameraMesh);
		CameraMesh->CastShadow = bCamCastShadow;
	}

	if (bCamShadowProxyCastHiddenShadow != CameraMeshShadowProxy->bCastHiddenShadow)
	{
		FComponentReattachContext ReattachContextCMSP(CameraMeshShadowProxy);
		CameraMeshShadowProxy->bCastHiddenShadow = bCamShadowProxyCastHiddenShadow;
	}

	if (CameraMesh->HiddenGame == bCamVisible)
	{
		CameraMesh->SetHiddenGame(!bCamVisible);
		CamcorderScreenLight->SetEnabled(bCamVisible);
	}

	if (HiddenLeftArmControl)
	{
		HiddenLeftArmControl->SetSkelControlStrength(0.0f, 0.0f);
	}

	if (bCamVisible && CameraScreenMat)
	{
		FLOAT matVal = (CamcorderMode == CCM_NightVision || CamcorderMode == CCM_PoweredNightVision) ? 1.0f : 0.0f;
		CameraScreenMat->SetScalarParameterValue(CameraScreenParamName, matVal);
	}

	BodySetup = bodySetup;
}

void AOLHero::SyncShadowProxy()
{
	// not exhaustive and not strictly needed, just failsafe against nodes when desync could diverge long enough to be noticeable

	Cast<UAnimTree>(ShadowProxy->Animations)->SyncGroupsWithMasterTree(Cast<UAnimTree>(Mesh->Animations));

	ShadowProxyLadderAnimNode->CurrentIdx = LadderAnimNode->CurrentIdx;
	ShadowProxyLadderAnimNode->CurrentRatio = LadderAnimNode->CurrentRatio;
	ShadowProxyLadderAnimNode->SmoothedDelta = LadderAnimNode->SmoothedDelta;

	ShadowProxyDoorAnimNode->InitialRatio = DoorAnimNode->InitialRatio;
	ShadowProxyDoorAnimNode->CurrentRatio = DoorAnimNode->CurrentRatio;
	ShadowProxyDoorAnimNode->MaxRatio = DoorAnimNode->MaxRatio;	
	ShadowProxyDoorAnimNode->SmoothedDelta = DoorAnimNode->SmoothedDelta;
	ShadowProxyDoorAnimNode->PlayRate = DoorAnimNode->PlayRate;	

	ShadowProxyLedgeWalkAnimNode->bAutomaticMotion = LedgeWalkAnimNode->bAutomaticMotion;
	ShadowProxyLedgeWalkAnimNode->LastTargetIdx = LedgeWalkAnimNode->LastTargetIdx;
	ShadowProxyLedgeWalkAnimNode->SmoothedDelta = LedgeWalkAnimNode->SmoothedDelta;
	ShadowProxyLedgeWalkAnimNode->CurrentRatio = LedgeWalkAnimNode->CurrentRatio;

	ShadowProxyLedgeHangAnimNode->bAutomaticMotion = LedgeHangAnimNode->bAutomaticMotion;
	ShadowProxyLedgeHangAnimNode->LastTargetIdx = LedgeHangAnimNode->LastTargetIdx;
	ShadowProxyLedgeHangAnimNode->SmoothedDelta = LedgeHangAnimNode->SmoothedDelta;
	ShadowProxyLedgeHangAnimNode->CurrentRatio = LedgeHangAnimNode->CurrentRatio;

	ShadowProxySqueezeAnimNode->bAutomaticMotion = SqueezeAnimNode->bAutomaticMotion;
	ShadowProxySqueezeAnimNode->LastTargetIdx = SqueezeAnimNode->LastTargetIdx;
	ShadowProxySqueezeAnimNode->SmoothedDelta = SqueezeAnimNode->SmoothedDelta;
	ShadowProxySqueezeAnimNode->CurrentRatio = SqueezeAnimNode->CurrentRatio;
}

void AOLHero::SetBase( AActor* NewBase, FVector NewFloor, INT bNotifyActor, USkeletalMeshComponent* SkelComp, FName AttachName )
{
	if (!OLPC || !OLPC->bCinematicMode)
	{
		Super::SetBase(NewBase, NewFloor, bNotifyActor, SkelComp, AttachName);
	}
}

void AOLHero::physCustom(FLOAT deltaTime, INT iterations)
{
	AirSpeed = GroundSpeed;
	
	FVector AccelDir = Acceleration;
	CalcVelocity(AccelDir, deltaTime, AirSpeed, 0.5f * PhysicsVolume->FluidFriction, 1, 0, 0);

	iterations++;
	bJustTeleported = 0;

	FVector OldLocation = Location;
	const FVector Adjusted = (Velocity + PhysicsVolume->GetZoneVelocityForActor(this)) * deltaTime;

	FCheckResult Hit(1.f);
	GWorld->MoveActor(this, Adjusted, Rotation, 0, Hit);

	if( Hit.Time < 1.f )
	{
		Floor = Hit.Normal;
		FVector GravDir = FVector(0,0,-1);
		FVector DesiredDir = Adjusted.SafeNormal();
		FVector VelDir = Velocity.SafeNormal();
		FLOAT UpDown = GravDir | VelDir;

		// rcharpentier : disabled as it causes issue with ledge hang moves, and I can't see when we'd want to do actual step ups in phys_custom
		/*if( (Abs(Hit.Normal.Z) < 0.2f) && (UpDown < 0.5f) && (UpDown > -0.2f) )
		{
			FLOAT stepZ = Location.Z;
			stepUp(GravDir, DesiredDir, Adjusted * (1.f - Hit.Time), Hit);
			OldLocation.Z = Location.Z + (OldLocation.Z - stepZ);
		}
		else*/
		{
			processHitWall(Hit, deltaTime);
			//adjust and try again
			FVector OldHitNormal = Hit.Normal;
			FVector Delta = (Adjusted - Hit.Normal * (Adjusted | Hit.Normal)) * (1.f - Hit.Time);
			if( (Delta | Adjusted) >= 0 )
			{
				GWorld->MoveActor(this, Delta, Rotation, 0, Hit);

				if( Hit.Time < 1.f ) //hit second wall
				{
					processHitWall(Hit, deltaTime*(1.f-Hit.Time));
					TwoWallAdjust(DesiredDir, Delta, Hit.Normal, OldHitNormal, Hit.Time);
					GWorld->MoveActor(this, Delta, Rotation, 0, Hit);
				}
			}
		}
	}
	else
	{
		Floor = FVector(0.f,0.f,1.f);
	}

	if( !bJustTeleported )
	{
		Velocity = (Location - OldLocation) / deltaTime;
	}
}

void AOLHero::physWalking(FLOAT deltaTime, INT Iterations)
{
	if (bIsCrouched && Floor.Z < 0.87f)
	{
		Super::physWalking(deltaTime, Iterations);
		return;
	}

	if( !Controller && !bRunPhysicsWithNoController )
	{
		Acceleration	= FVector(0.f);
		Velocity		= FVector(0.f);
		return;
	}

	//bound acceleration
	Velocity.Z = 0.f;
	Acceleration.Z = 0.f;
	FVector AccelDir = Acceleration.IsZero() ? Acceleration : Acceleration.SafeNormal();
	CalcVelocity(AccelDir, deltaTime, GroundSpeed, PhysicsVolume->GroundFriction, 0, 1, 0);

	// Add effect of velocity zone
	// Rather than constant velocity, hacked to make sure that velocity being clamped when walking doesn't
	// cause the zone velocity to have too much of an effect at fast frame rates
	// @todo actually clamp this ^^
	FVector DesiredMove = Velocity;
	if ( PhysicsVolume->bVelocityAffectsWalking )
	{
		DesiredMove += PhysicsVolume->GetZoneVelocityForActor(this) * 25.f * deltaTime;
	}
	DesiredMove.Z = 0.f;

	//Perform the move
	const FVector GravDir = FVector(0.f,0.f,-1.f);
	const FVector Down = GravDir * (MaxStepHeight + MAXSTEPHEIGHTFUDGE);
	FCheckResult Hit(1.f);
	const FVector OldLocation = Location;
	const FVector OldFloor = Floor;
	AActor* OldBase = Base;
	FVector OldBaseLocation = (Base != NULL) ? Base->Location : FVector(0.f, 0.f, 0.f);
	bJustTeleported = 0;
	INT bCheckedFall = 0;
	INT bMustJump = 0;
	UBOOL bRejectMove = FALSE;
	FLOAT remainingTime = deltaTime;
	UBOOL bSlidedDown = FALSE;

	UCylinderComponent* CylComp = Cast<UCylinderComponent>(CollisionComponent);
	TWEAKABLE FLOAT pctRadius = 0.85f;
	FVector floorCheckComp(pctRadius*CylComp->CollisionRadius, pctRadius*CylComp->CollisionRadius, CylComp->CollisionHeight);

	while ( (remainingTime > 0.f) && (Iterations < 8) && (Controller || bRunPhysicsWithNoController) )
	{
		Iterations++;
		// subdivide moves to be no longer than 0.05 seconds
		const FLOAT timeTick = (remainingTime > 0.05f) ? Min(0.05f, remainingTime * 0.5f) : remainingTime;
		remainingTime -= timeTick;
		FVector Delta = timeTick * DesiredMove;
		FVector subLoc = Location;
		FLOAT NewHitTime = 0.f;
		FVector NewFloor(0.f,0.f,0.f);
		AActor *NewBase = NULL;
		const UBOOL bZeroDelta = Delta.IsNearlyZero();
		UBOOL bDontMoveToFloor = FALSE;

		if ( bZeroDelta )
		{
			remainingTime = 0.f;
		}
		else
		{
			// try to move forward
			if ( (Floor.Z < 0.98f) && ((Floor | Delta) < 0.f) )
			{
				Hit.Time = 0.f;
				Hit.Normal = Floor;
			}
			else
			{
				GWorld->MoveActor(this, Delta, Rotation, 0, Hit);
			}

			if ( Hit.Time < 1.f )
			{
				// Handle pawn bumping into mesh that can become dynamic
				if ( Hit.Actor && Hit.Actor->bWorldGeometry && (Base != Hit.Actor) )
				{
					UStaticMeshComponent *HitStaticMesh = Cast<UStaticMeshComponent>(Hit.Component);
					if ( HitStaticMesh && HitStaticMesh->CanBecomeDynamic() )
					{
						AKActorFromStatic* NewKActor = Cast<AKActorFromStatic>(AKActorFromStatic::StaticClass()->GetDefaultActor())->MakeDynamic(HitStaticMesh);
						if ( NewKActor )
						{
							FVector HitDir =  Hit.Location - Location;
							HitDir.Z = ::Max(HitDir.Z, 0.f);
							NewKActor->eventApplyImpulse(HitDir, GroundSpeed,  Hit.Location); 
							Hit.Actor = NewKActor;
						}
					}
				}

				if ( !Hit.Actor || Hit.Actor->bCanStepUpOn || Base == Hit.Actor )
				{
					// hit a barrier, try to step up
					const FLOAT DesiredDist = Delta.Size();
					const FVector DesiredDir = Delta/DesiredDist;
					stepUp(GravDir, DesiredDir, Delta * (1.f - Hit.Time), Hit);
					if ( Physics == PHYS_Falling ) // pawn decided to jump up
					{
						const FLOAT ActualDist = (Location - subLoc).Size2D();
						remainingTime += timeTick * (1 - Min(1.f,ActualDist/DesiredDist));
						eventFalling();
						if ( Physics == PHYS_Flying )
						{
							Velocity = FVector(0.f,0.f,AirSpeed);
							Acceleration = FVector(0.f,0.f,AccelRate);
						}
						startNewPhysics(remainingTime,Iterations);
						return;
					}

					// see if I already found a floor
					if ( Hit.Time < 1.f )
					{
						NewHitTime = Hit.Time;
						NewFloor = Hit.Normal;
						NewBase = Hit.Actor;
						bDontMoveToFloor = TRUE;
					}
				}
				else if ( Hit.Actor && !Hit.Actor->bCanStepUpOn )
				{
					const FLOAT DesiredDist = Delta.Size();
					const FVector DesiredDir = Delta/DesiredDist;

					// notify script that pawn ran into a wall
					processHitWall(Hit);
					if ( Physics == PHYS_Falling )
					{
						// pawn decided to jump up
						const FLOAT ActualDist = (Location - subLoc).Size2D();
						remainingTime += timeTick * (1 - Min(1.f,ActualDist/DesiredDist));
						eventFalling();
						if ( Physics == PHYS_Flying )
						{
							Velocity = FVector(0.f,0.f,AirSpeed);
							Acceleration = FVector(0.f,0.f,AccelRate);
						}
						startNewPhysics(remainingTime,Iterations);
						return;
					}

					// adjust along wall
					Hit.Normal.Z = 0.f;	// treat barrier as vertical;
					Hit.Normal = Hit.Normal.SafeNormal();
					FVector NewDelta = Delta;
					const FVector OldHitNormal = Hit.Normal;
					NewDelta = (Delta - Hit.Normal * (Delta | Hit.Normal)) * (1.f - Hit.Time);
					if( (NewDelta | Delta) >= 0.f && !NewDelta.IsNearlyZero() )
					{
						GWorld->MoveActor(this, NewDelta, Rotation, 0, Hit);
						if (Hit.Time < 1.f)
						{
							processHitWall(Hit);
							if ( Physics == PHYS_Falling )
								return;
							TwoWallAdjust(DesiredDir, NewDelta, Hit.Normal, OldHitNormal, Hit.Time);
							GWorld->MoveActor(this, NewDelta, Rotation, 0, Hit);
						}
					}
				}
			}
		}

		//drop to floor
		FLOAT FloorDist;

		if (Base != NULL && !Base->bCollideActors && Base != WorldInfo)
		{
			bForceFloorCheck = TRUE;
		}

		if (( !NewBase && bZeroDelta && Base && Base->bWorldGeometry && (RelativeLocation == Location - Base->Location) && !bForceFloorCheck ) || bDontMoveToFloor)
		{
			if (!bDontMoveToFloor)
			{
				Hit.Actor = Base;
				Hit.Normal = Floor;
				Hit.Time = 0.1f;
			}

			UBOOL bUseZeroDist = (bIsCrouched || (Floor.Z > 0.98f)) && bDontMoveToFloor;
			FloorDist = bUseZeroDist ? 0.0f : MAXFLOORDIST;
		}
		else
		{
			bForceFloorCheck = FALSE;
			const FVector ColLocation = CollisionComponent ? CollisionComponent->Bounds.Origin : Location;
			DWORD	TraceFlags	= TRACE_AllBlocking;
			if ( bMoveIgnoresDestruction )
			{
				TraceFlags |= TRACE_MoveIgnoresDestruction;
			}
			GWorld->SingleLineCheck( Hit, this, ColLocation + Down, ColLocation, TraceFlags, floorCheckComp );
			FloorDist = Hit.Time * (MaxStepHeight + MAXSTEPHEIGHTFUDGE);
		}

		Floor = Hit.Normal;

		if ( (Hit.Normal.Z < WalkableFloorZ) && !Delta.IsNearlyZero() && ((Delta | Hit.Normal) < 0) )
		{
			// slide down slope
			FVector Slide = Hit.Normal * (FVector(0.f,0.f,MaxStepHeight) | Hit.Normal) - FVector(0.f,0.f,MaxStepHeight);

			// slight fudge to angle away from the slope, to help prevent getting caught due to
			// precision errors.  helps climbing vertical walls.
			static const FLOAT NormalFudge=0.1f;
			Slide += Hit.Normal * NormalFudge;

			GWorld->MoveActor(this, Slide, Rotation, 0, Hit);
			if ( (Hit.Actor != Base) && (Physics == PHYS_Walking) )
			{
				SetBase(Hit.Actor, Hit.Normal);
			}
			Floor = Hit.Normal;

			if ( NewBase && (Floor.Z < WalkableFloorZ) && Hit.Time < 1.f )
			{
				// If we're here, stepup put us an unwalkable surface and we could't resolve it with a slide.
				// Reject the movement outright.  This tends to happen when pressing into skewed corners.
				bRejectMove = TRUE;
			}
			else
			{
				bSlidedDown = TRUE;
			}
		}
		else if( Hit.Time< 1.f && !Hit.bStartPenetrating && (Hit.Actor!=Base || FloorDist>MAXFLOORDIST) )
		{
			if ( ShouldCatchAir(OldFloor, Floor) )
			{
				StartFalling(Iterations, remainingTime, timeTick, Delta, subLoc);
				return;
			}
			else
			{
				// move down to correct position
				const FVector RealNorm = Hit.Normal;
				AActor* RealHitActor = Hit.Actor;

				GWorld->MoveActor(this, FVector(0.0f,0.0f,0.5f*(MINFLOORDIST+MAXFLOORDIST) - FloorDist), Rotation, 0, Hit);
				if ( Hit.Time == 1.f )
				{
					Hit.Time = 0.f;
					Hit.Normal = RealNorm;
					Hit.Actor = RealHitActor;
				}
				if ( (Hit.Actor != Base) && (Physics == PHYS_Walking) && IsBlockedBy(Hit.Actor, Hit.Component) )
				{
					SetBase(Hit.Actor, Hit.Normal);
				}
			}
		}		
		else if (FloorDist < MINFLOORDIST && !Hit.bStartPenetrating)
		{
			// move up to correct position (average of MAXFLOORDIST and MINFLOORDIST above floor)
			const FVector RealNorm = Hit.Normal;
			GWorld->MoveActor(this, FVector(0.f,0.f,0.5f*(MINFLOORDIST+MAXFLOORDIST) - FloorDist), Rotation, 0, Hit);
			Hit.Time = 0.f;
			Hit.Normal = RealNorm;
		}

		if( !bMustJump && Hit.Time<1.f && Hit.Normal.Z>=WalkableFloorZ )
		{
			// standing on walkable surface...

			if( (Hit.Normal.Z < 0.99f) && ((Hit.Normal.Z * PhysicsVolume->GroundFriction) < 3.3f) ) 
			{
				// slide down slope, depending on friction and gravity
				const FVector Slide(0.f, 0.f, (deltaTime * GetGravityZ()/(2.f * ::Max(0.5f, PhysicsVolume->GroundFriction))) * deltaTime);
				Delta = Slide - Hit.Normal * (Slide | Hit.Normal);
				if( (Delta | Slide) >= 0.f )
				{
					GWorld->MoveActor(this, Delta, Rotation, 0, Hit);
				}				
			}				
		}
		else
		{
			// If we haven't checked for a ledge already
			// Make sure we can walk off this one
			if( bRejectMove ||
				(!bJustTeleported && 
				!bMustJump && 
				( !bCanJump || 
				(!bCanWalkOffLedges && (bIsWalking || bIsCrouched) && !bZeroDelta ) ) ) )
			{
				// this pawn shouldn't fall, so undo its move
				Velocity = FVector(0.f,0.f,0.f);
				Acceleration = FVector(0.f,0.f,0.f);
				GWorld->FarMoveActor(this, OldLocation, FALSE, FALSE);
				bJustTeleported = FALSE;
				// if our previous base couldn't have moved or changed in any physics-affecting way, restore it
				if (OldBase != NULL && (OldBase->IsStatic() || OldBase->bWorldGeometry || !OldBase->bMovable || (OldBase->IsEncroacher() && OldBase->Location == OldBaseLocation)))
				{
					SetBase(OldBase,OldFloor);
				}
				if ( Controller )
				{
					Controller->FailMove();
				}
				return;
			}
			else
			{
				StartFalling(Iterations, remainingTime, timeTick, Delta, subLoc);
				return;
			}
		}
	}

	// Allow touch events and such to change physics state and velocity
	if( Physics == PHYS_Walking ) 
	{
		// Make velocity reflect actual move
		if (!bJustTeleported)
		{
			FVector actualVelocity = (Location - OldLocation) / deltaTime;
			if ((actualVelocity | AccelDir) >= 0.0f || !bSlidedDown)
			{
				Velocity = actualVelocity;
			}
		}
		Velocity.Z = 0.f;
	}
}

void AOLHero::stepUp(const FVector& GravDir, const FVector& DesiredDir, const FVector& Delta, FCheckResult &Hit)
{
	if (bIsCrouched && Floor.Z < 0.87f)
	{
		Super::stepUp(GravDir, DesiredDir, Delta, Hit);
		return;
	}

	FVector Down = GravDir * (MaxStepHeight + MAXSTEPHEIGHTFUDGE);
	UBOOL bStepDown = TRUE;

	// If walking up a slope that is walkable (step up - used instead of trying to slide up)
	FLOAT StepSideZ = -1.f * (Hit.Normal | GravDir);
	if( (StepSideZ < MAXSTEPSIDEZ) || (Hit.Normal.Z >= WalkableFloorZ) )
	{
		// step up - treat as vertical wall
		FVector originalLocation = Location;
		
		FLOAT additionalStepHeight = 15.0f;
		GWorld->MoveActor(this, -1.f * Down + VecZ(additionalStepHeight), Rotation, 0, Hit);
		
		FVector testDelta = CylinderComponent->CollisionRadius * Delta.SafeNormal2D();
		GWorld->MoveActor(this, testDelta, Rotation, 0, Hit);

		if (Hit.Time < 1.0f)
		{
			// Obstacle ahead, even after stepping up. Try without the additional height to see if it's a matter of hitting the ceiling
			Location = originalLocation;
			GWorld->MoveActor(this, -1.f * Down, Rotation, 0, Hit);
			GWorld->MoveActor(this, Delta, Rotation, 0, Hit);		
		}
		else
		{
			FCheckResult dummyHit;
			FVector correctionDelta = Delta - testDelta;
			GWorld->MoveActor(this, correctionDelta, Rotation, 0, Hit);
			GWorld->MoveActor(this, -VecZ(additionalStepHeight), Rotation, 0, dummyHit);
		}	
	}
	else if ( Physics != PHYS_Walking )
	{
		// slide up slope
		FLOAT Dist = Delta.Size();
		GWorld->MoveActor(this, Delta + FVector(0,0,Dist*Hit.Normal.Z), Rotation, 0, Hit);
		bStepDown = FALSE;
	}

	if (Hit.Time < 1.f)
	{
		// Handle pawn bumping into mesh that can become dynamic
		if ( Hit.Actor && Hit.Actor->bWorldGeometry && (Base != Hit.Actor) )
		{
			UStaticMeshComponent *HitStaticMesh = Cast<UStaticMeshComponent>(Hit.Component);
			if ( HitStaticMesh && HitStaticMesh->CanBecomeDynamic() )
			{
				AKActorFromStatic* NewKActor = Cast<AKActorFromStatic>(AKActorFromStatic::StaticClass()->GetDefaultActor())->MakeDynamic(HitStaticMesh);
				if ( NewKActor )
				{
					FVector HitDir =  Hit.Location - Location;
					HitDir.Z = ::Max(HitDir.Z, 0.f);
					NewKActor->eventApplyImpulse(HitDir, GroundSpeed,  Hit.Location); 
					Hit.Actor = NewKActor;
				}
			}
		}

		// step up again if went far enough to consider a valid step, and step side is ~vertical, and can step onto the hit actor
		if ( ( -1.f * (Hit.Normal | GravDir) < MAXSTEPSIDEZ) && (Hit.Time * Delta.SizeSquared() > MINSTEPSIZESQUARED) 
			&& (!Hit.Actor || Hit.Actor->bCanStepUpOn) )
		{
			if ( bStepDown )
			{
				FCheckResult StepDownHit(1.f);
				GWorld->MoveActor(this, Down, Rotation, 0, StepDownHit);
			}
			stepUp(GravDir, DesiredDir, Delta * (1 - Hit.Time), Hit);
			return;
		}

		// notify script that pawn ran into a wall
		processHitWall(Hit);
		if ( Physics == PHYS_Falling )
			return;

		//adjust and try again
		Hit.Normal.Z = 0.f;	// treat barrier as vertical;
		Hit.Normal = Hit.Normal.SafeNormal();
		FVector NewDelta = Delta;
		FVector OldHitNormal = Hit.Normal;
		NewDelta = (Delta - Hit.Normal * (Delta | Hit.Normal)) * (1.f - Hit.Time);
		if( (NewDelta | Delta) >= 0.f )
		{
			GWorld->MoveActor(this, NewDelta, Rotation, 0, Hit);
			if (Hit.Time < 1.f)
			{
				processHitWall(Hit);
				if ( Physics == PHYS_Falling )
					return;
				TwoWallAdjust(DesiredDir, NewDelta, Hit.Normal, OldHitNormal, Hit.Time);
				GWorld->MoveActor(this, NewDelta, Rotation, 0, Hit);
			}
		}
	}
	if ( bStepDown )
	{
		GWorld->MoveActor(this, Down, Rotation, 0, Hit);
	}
}

void AOLHero::processLanded(FVector const& HitNormal, AActor *HitActor, FLOAT remainingTime, INT Iterations)
{
	FailedLandingCount = 0;

	Floor = HitNormal;
	if( !Controller || !Controller->eventNotifyLanded(HitNormal, HitActor) )
	{
		eventLanded(HitNormal, HitActor);
	}

	if( Physics == PHYS_Falling )
	{
		SetPostLandedPhysics(HitActor, HitNormal);
	}

	if( Physics == PHYS_Walking )
	{
		Acceleration = Acceleration.SafeNormal();
	}

	startNewPhysics(remainingTime, Iterations);

	if( Controller && Controller->bNotifyPostLanded )
	{
		Controller->eventNotifyPostLanded();
	}
}

void AOLHero::UpdateGroundSpeed(FLOAT deltaTime, const FVector &AccelDir)
{
	WalkSpeed = NormalWalkSpeed;
	RunSpeed = NormalRunSpeed;

	if (LocomotionMode == LM_Walk || LocomotionMode == LM_LookBack || SpecialMove == SMT_EnterLookBack || SpecialMove == SMT_ExitLookBack)
	{
		if (bElectrified)
		{
			if (bIsCrouched)
			{
				GroundSpeed = Min(CrouchedSpeed, ElectrifiedSpeed);
			}

			GroundSpeed = ElectrifiedSpeed;
			CurrentRunSpeed = GroundSpeed;
		}
		else
		{
			if (bIsCrouched)
			{
				GroundSpeed = CrouchedSpeed;
				CurrentRunSpeed = GroundSpeed;
			}
			else
			{
				UBOOL bNoAccel = AccelDir.IsNearlyZero();

				if (bLimping)
				{
					WalkSpeed = LimpingWalkSpeed;
					RunSpeed = LimpingWalkSpeed; // no run
				}
				else if (bHobbling)
				{
					WalkSpeed = LerpClamped(HobblingIntensity, NormalWalkSpeed, HobblingWalkSpeed);
					RunSpeed = LerpClamped(HobblingIntensity, NormalRunSpeed, HobblingRunSpeed);
				}
				else if (WalkSpeedOverride > 0.0f || RunSpeedOverride > 0.0f)
				{
					WalkSpeed = WalkSpeedOverride > 0.0f ? WalkSpeedOverride : NormalWalkSpeed;
					RunSpeed = RunSpeedOverride > 0.0f ? RunSpeedOverride : NormalRunSpeed;
				}
				else
				{
					for (INT i = 0; i < Touching.Num(); i++)
					{		
						APhysicsVolume* volume = Cast<APhysicsVolume>(Touching(i));
						if (volume && volume->bEnabled)
						{
							if (volume->bWaistDeepWater)
							{
								WalkSpeed = WaterWalkSpeed;
								RunSpeed = WaterRunSpeed;					
								break;
							}
							else if (volume->MaxHeroSpeed > 0.0f)
							{
								WalkSpeed = Min(NormalWalkSpeed, volume->MaxHeroSpeed);
								RunSpeed = Min(NormalRunSpeed, volume->MaxHeroSpeed);
								break;
							}
							else if (volume->bForcePawnWalk)
							{
								WalkSpeed = NormalWalkSpeed;
								RunSpeed = NormalWalkSpeed;
								break;
							}
						}
					}
				}

				FLOAT effectiveRunSpeed = RunSpeed;
				
				if (!bNoAccel)
				{
					FLOAT backwardSlowDownFactor = Saturate(AccelDir | -CharForward) * SpeedPenaltyBackwards;
					FLOAT strafeSlowDownFactor = Saturate(Abs(AccelDir | Rotation.Right())) * SpeedPenaltyStrafe;
					FLOAT slowDown = Saturate(backwardSlowDownFactor + strafeSlowDownFactor);

					effectiveRunSpeed = Max(WalkSpeed, RunSpeed * (1.0f - slowDown));
				}
				
				FLOAT targetSpeed = (bWantToRun && !bNoAccel) ? effectiveRunSpeed : WalkSpeed;

				if (targetSpeed > CurrentRunSpeed)
				{
					CurrentRunSpeed = Utils::Approach(CurrentRunSpeed, targetSpeed, AccelApproachFactor, deltaTime);
				}
				else
				{
					CurrentRunSpeed = Utils::Approach(CurrentRunSpeed, targetSpeed, DecelApproachFactor, deltaTime);
				}
				
				if (bNoAccel)
				{
					CurrentRunSpeed = Max(CurrentRunSpeed, Velocity.Size2D());
				}

				GroundSpeed = CurrentRunSpeed;
			}

			// Apply the Hurt speed penalty
			if (LastDamageType)
			{
				FLOAT timeSinceLastHit = (GWorld->GetTimeSeconds() - LastDamageTime);

				UOLDmgType* dmgType = Cast<UOLDmgType>(LastDamageType->GetDefaultActor());

				if (dmgType && timeSinceLastHit < dmgType->SpeedPenaltyDuration)
				{
					FLOAT timeScaleFactor = 1.0f - timeSinceLastHit/dmgType->SpeedPenaltyDuration; // 100% to 0% as time passes
					FLOAT baseHurtPenalty = 1.0f - Saturate(timeScaleFactor*dmgType->SpeedPenaltyPctOnDamage);
					GroundSpeed *= baseHurtPenalty;
				}
			}

			// Apply the health-relative speed penalty
			if (PreciseHealth < 100.0f)
			{
				FLOAT speedPctLost = MaxSpeedPenaltyPctForInjuries * (100.0f - PreciseHealth)/100.0f;
				GroundSpeed *= (1.0f - speedPctLost);
			}
		}

		// Adjust for slopes
		if (Floor.Z < 0.98f && Floor.Z > 0.5f && Physics == PHYS_Walking && !Velocity.IsNearlyZero(KINDA_SMALL_NUMBERF))
		{
			FVector velDirection = Velocity.SafeNormal();
			FLOAT floorFwdComp = -(Floor | velDirection.SafeNormal2D());

			if (floorFwdComp > 0.0f || !IsRunning())
			{
				FLOAT scaleFactor = appSqrt(1.0f - Square(floorFwdComp)); // good old trig
				GroundSpeed *= scaleFactor;
			}
		}
	}
	else if (LocomotionMode == LM_Fall)
	{
		GroundSpeed = Max(WalkSpeed, RealVelocity.Size2D());
		CurrentRunSpeed = GroundSpeed;
	}
	else
	{
		GroundSpeed = WalkSpeed;
		CurrentRunSpeed = GroundSpeed;
	}
}

void AOLHero::CalcVelocity(FVector &AccelDir, FLOAT deltaTime, FLOAT MaxSpeed, FLOAT Friction, INT bFluid, INT bBrake, INT bBuoyant)
{
	if (appIsNaN(AccelDir.X) || appIsNaN(AccelDir.Y) || appIsNaN(AccelDir.Z))
	{
		debugf(TEXT("### CalcVelocity -- AccelDir contains NaN - resetting to 0"));
		AccelDir = FVector(0.0f);
	}

	if (appIsNaN(Velocity.X) || appIsNaN(Velocity.Y) || appIsNaN(Velocity.Z))
	{
		debugf(TEXT("### CalcVelocity -- Velocity contains NaN - resetting to 0"));
		Velocity = FVector(0.0f);
	}

	DesiredMoveDirection = AccelDir.SafeNormal();

	UpdateGroundSpeed(deltaTime, AccelDir);

	FLOAT timeSinceLastLanding = GWorld->GetTimeSeconds() - LastLandingTimestamp;
	if (bApplyLandingPenalty && timeSinceLastLanding < LandingPenaltyDuration)
	{
		MovementSpeedModifier = InputMovementScaling * (LandingSpeedModifier + Utils::SmootherStep(timeSinceLastLanding/LandingPenaltyDuration)*(1.0f - LandingSpeedModifier));
	}
	else
	{
		MovementSpeedModifier = InputMovementScaling;
	}

	bBlockConstrainedMovement = FALSE;
	bKillConstrainedMovement = FALSE;

	switch (LocomotionMode)
	{
	case LM_LedgeHang:
	case LM_LedgeWalk:
		{
			check(ActiveLedge && ActiveLedge->Next);
				
			TWEAKABLE FLOAT ContinuousSegmentsMinCosAngle = 0.86f;
			TWEAKABLE FLOAT MinDistToLastSegment = 35.0f;
			TWEAKABLE FLOAT MaxLedgeHangIdleTime = 1.5f;

			FVector ledgeDir = (ActiveLedge->Next->Location - ActiveLedge->Location).SafeNormal();
		
			UBOOL lookingAtLedge = Abs(ledgeDir.Dot2d(EyeForward)) > 0.707f;		
			UBOOL bClearIntent = Abs(DesiredMoveDirection | ledgeDir) > 0.707f;
			if (!AccelDir.IsNearlyZero() && (lookingAtLedge || bClearIntent))
			{
				FVector intentDirection = AccelDir.SafeNormal2D();
				FVector eyeFwd2D = EyeForward.SafeNormal2D();

				if (lookingAtLedge && !bClearIntent && Abs(intentDirection | eyeFwd2D) < 0.25f)
				{
					// looking at the ledge with a large difference between input and camera (likely keeping pressing A/D while looking straight at the ledge)
					UBOOL bInputRight = (intentDirection | EyeRotation.Right()) > 0.0f;
					AccelDir = AccelDir.Size2D() * (bInputRight ? 1.0f : -1.0f) * Rotation.Right();
				}

				AccelDir = AccelDir.ProjectOnTo(ledgeDir);
				DesiredMoveDirection = AccelDir.SafeNormal();
				LedgeHangIdleStartTime = -1.0f;
			}
			else
			{
				AccelDir = FVector(0.0f); // At less than 45 degs from desired direction - null out move
				Velocity = FVector(0.0f);
				DesiredMoveDirection = FVector(0.0f);

				if (LedgeHangIdleStartTime <= 0.0f)
				{
					LedgeHangIdleStartTime = GWorld->GetTimeSeconds();
				}
			}

			Super::CalcVelocity(AccelDir, deltaTime, MaxSpeed, Friction, bFluid, bBrake, bBuoyant);

			if (LocomotionMode == LM_LedgeHang && LedgeHangIdleStartTime > 0.0f && (GWorld->GetTimeSeconds() - LedgeHangIdleStartTime) > MaxLedgeHangIdleTime)
			{
				// idling
				bKillConstrainedMovement = TRUE;
			}
			else if (LocomotionMode == LM_LedgeHang && !ActiveLedge->Prev && !ActiveLedge->Next->Next && ActiveLedge->Location.DistanceSquared(ActiveLedge->Next->Location) < Square(125.0f))
			{
				// special case to intercept airvent ledges - don't ever allow moving on those
				bBlockConstrainedMovement = TRUE;
			}
			else
			{
				FVector deltaThisFrame = Velocity * deltaTime;
				FVector nextPos = Location + deltaThisFrame;

				FLOAT maxDistFromLedge = (LocomotionMode == LM_LedgeHang) ? LedgeHangTransitionInteractDist : Min(LedgeWalkTransitionInteractDistInside, LedgeWalkTransitionInteractDistOutside);
				maxDistFromLedge -= 0.5f; // give just a little room for the transitions to activate

				AOLLedgeMarker* closestMarker = NULL;
				AOLLedgeMarker* nextMarker = NULL;

				if (Location.DistanceSquared(ActiveLedge->Location) < Location.DistanceSquared(ActiveLedge->Next->Location))
				{
					closestMarker = ActiveLedge;
					nextMarker = ActiveLedge->Prev;
				}
				else
				{
					closestMarker = ActiveLedge->Next;
					nextMarker = ActiveLedge->Next->Next;
					ledgeDir = -ledgeDir; // make ledgeDir face from the closest marker
				}

				if ((DesiredMoveDirection | ledgeDir) <= 0.0f)
				{
					// going towards segment end

					if (!nextMarker || ((LocomotionMode == LM_LedgeHang && !nextMarker->bCanLedgeHang) || (LocomotionMode == LM_LedgeWalk && !nextMarker->bCanLedgeWalk)))
					{
						// there's no further (valid) segment
						FLOAT distToMarkerAlongLedge = (Location - closestMarker->Location) | ledgeDir;
						
						if (distToMarkerAlongLedge < MinDistToLastSegment)
						{
							bBlockConstrainedMovement = TRUE;
							bKillConstrainedMovement = TRUE;
						}

					}
					else if (!Utils::IsBetweenMarkers(nextPos, ActiveLedge->Location, ActiveLedge->Next->Location, -maxDistFromLedge))
					{
						// going out of the ledge, but there's a possible transition
				
						bBlockConstrainedMovement = TRUE;
						FVector nextStretch = (nextMarker->Location - closestMarker->Location).SafeNormal();

						if (Abs(nextStretch | ledgeDir) > ContinuousSegmentsMinCosAngle)
						{
							bBlockConstrainedMovement = FALSE;

							// treat as the same segment
							if (!Utils::IsBetweenMarkers(nextPos, ActiveLedge->Location, ActiveLedge->Next->Location, 0.0f))
							{
								if (LocomotionMode == LM_LedgeHang)
								{
									ActiveLedge = (nextMarker == closestMarker->Next) ? closestMarker : nextMarker;
								}
								else
								{
									// confirm that there's still a valid wall
									FCheckResult Hit(1.f);
									FVector startTrace = closestMarker->Location + 50.0f*nextStretch + FVector(0, 0, 100.0f); // on the ledge, above the ground
									FVector endTrace = startTrace - LedgeWalkMaxWallDist*CharForward;

									if (!GWorld->SingleLineCheck( Hit, this, endTrace, startTrace, TRACE_AllBlocking | TRACE_StopAtAnyHit, FVector(0.0f)))
									{
										// ledge transition
										ActiveLedge = (nextMarker == closestMarker->Next) ? closestMarker : nextMarker;
									}
									else
									{
										bBlockConstrainedMovement = TRUE;
									}
								}
							}
						}
					}
				}
			}
		}
		break;
	case LM_Squeeze:
		{
			check(ActiveSqueeze);
			FVector passageDir = (ActiveSqueeze->Node2->Location - ActiveSqueeze->Node1->Location).SafeNormal();

			UBOOL lookingAtPassage = Abs(passageDir.Dot2d(EyeForward)) > 0.707f;		
			if (lookingAtPassage || Abs(DesiredMoveDirection | passageDir) > 0.707f)
			{
				AccelDir = AccelDir.ProjectOnTo(passageDir);
				DesiredMoveDirection = AccelDir.SafeNormal();
			}
			else
			{
				AccelDir = FVector(0.0f); // At less than 45 degs from desired direction - null out move
				Velocity = FVector(0.0f);
				DesiredMoveDirection = FVector(0.0f);
			}

			Super::CalcVelocity(AccelDir, deltaTime, MaxSpeed, Friction, bFluid, bBrake, bBuoyant);
		}
		break;
	case LM_Door:
		{
			check(ActiveDoor);
			FVector doorTraversalDir = ActiveDoor->GetStaticDirection();

			if (ActiveDoor->bLocked || ActiveDoor->bBlocked)
			{
				AccelDir = FVector(0.0f); 
				Velocity = FVector(0.0f);
				DesiredMoveDirection = FVector(0.0f);
			}
			else if (!bDoorStartingRatioReached)
			{				
				if (DoorAnimNode->CurrentRatio >= DoorInteractionStartingRatio)
				{
					bDoorStartingRatioReached = TRUE;
					AccelDir = FVector(0.0f); 
					Velocity = FVector(0.0f);
					DesiredMoveDirection = FVector(0.0f);
				}
				DesiredMoveDirection = doorTraversalDir;
			}
			else 
			{
				AccelDir = AccelDir.ProjectOnTo(doorTraversalDir);
				DesiredMoveDirection = AccelDir.SafeNormal();
			}

			Super::CalcVelocity(AccelDir, deltaTime, MaxSpeed, Friction, bFluid, bBrake, bBuoyant);			
		}
		break;	
	case LM_Bed:
		{
			Velocity = FVector(0.0f);
			AccelDir = FVector(0.0f);
			DesiredMoveDirection = FVector(0.0f);
		}
		break;
	case LM_Ladder:
		{
			check(ActiveLadder);

			if (!appIsNearlyZero(AccelDir.Z, KINDA_SMALL_NUMBERF))
			{
				DesiredMoveDirection = FVector(0, 0, Sgn(AccelDir.Z));
			}
			else
			{
				AccelDir = FVector(0.0f); // At less than 45 degs from desired direction - null out move
				Velocity = FVector(0.0f);
				DesiredMoveDirection = FVector(0.0f);
			}

			Super::CalcVelocity(AccelDir, deltaTime, MaxSpeed, Friction, bFluid, bBrake, bBuoyant);
		}
		break;
	case LM_Pushing:
		{
			check(ActivePushable);
			FVector pushDirection = bPushingFromBackEdge ? ActivePushable->GetFwdDirection() : ActivePushable->GetBackDirection();

			UBOOL bWantToPush = (DesiredMoveDirection | pushDirection) > 0.707f;
			UBOOL bPushing = FALSE;

			if (bWantToPush && bPushingFromBackEdge)
			{
				bPushing = ActivePushable->TryPushFwd();
			}
			else if (bWantToPush && !bPushingFromBackEdge)
			{
				bPushing = ActivePushable->TryPushBack();
			}
			else
			{
				ActivePushable->StopMoving();
			}
			
			// We don't move explicitely, we just move the object and let our correction code put us as the right place
			AccelDir = FVector(0.0f);
			Velocity = FVector(0.0f);

			if (bPushing)
			{
				DesiredMoveDirection = pushDirection;
			}
			else
			{
				DesiredMoveDirection = FVector(0.0f);
			}
		}
		break;
	case LM_Cinematic:
		{
			Velocity = FVector(0);
			Acceleration = FVector(0);
			Super::CalcVelocity(AccelDir, deltaTime, MaxSpeed, Friction, bFluid, bBrake, bBuoyant);			
		}
		break;
	default:
		{
			Super::CalcVelocity(AccelDir, deltaTime, MaxSpeed, Friction, bFluid, bBrake, bBuoyant);
			ApplyObstacleAvoidance(deltaTime, AccelDir);
		}
	}

	ApplyPositioningCorrection(deltaTime);
	ApplyExternalImpulse(deltaTime);	
}

void AOLHero::ApplyPositioningCorrection(FLOAT deltaTime)
{
	switch (LocomotionMode)
	{
	case LM_LedgeHang:
	case LM_LedgeWalk:
		{
			TWEAKABLE FLOAT EndPointBufferDistance = 30.0f;

			// Apply required correction to make sure we're precisely at the right distance from the ledge
			FLOAT distToLedge = (LocomotionMode == LM_LedgeHang) ? LedgeHangDistToWall : LedgeWalkDistFromEdge;
			FLOAT heightToLedge = (LocomotionMode == LM_LedgeHang) ? LedgeHangHeightToLedge : 0.0f;
			FVector closestPoint;

			FVector effectiveNode1Loc = ActiveLedge->Location;
			FVector effectiveNode2Loc = ActiveLedge->Next->Location;
			FVector ledge = (effectiveNode2Loc - effectiveNode1Loc).SafeNormal();
			FVector ledgePerp = ledge ^ VecZ(1.0f); // in front of character

			if ((ledgePerp | CharForward) < 0.0f)
			{
				ledgePerp = -ledgePerp;
			}
			
			if (!ActiveLedge->Prev)
			{
				effectiveNode1Loc += EndPointBufferDistance * ledge;
			}
			if (!ActiveLedge->Next->Next)
			{
				effectiveNode2Loc -= EndPointBufferDistance * ledge;
			}

			PointDistToSegment(Location, effectiveNode1Loc, effectiveNode2Loc, closestPoint);
			FVector anchoredPos = closestPoint - distToLedge*ledgePerp - FVector(0, 0, heightToLedge);
			FVector anchoringError = anchoredPos - Location;

			if (anchoringError.SizeSquared2D() > 1.0f || Abs(anchoringError.Z) > 0.1f)
			{
				FVector correctionVelocity = appIsNearlyZero(deltaTime) ? FVector(0.0f) : anchoringError/deltaTime;
				TWEAKABLE FLOAT anchoringCorrectionSpeedLedgeHang = 60.0f;
				TWEAKABLE FLOAT anchoringCorrectionSpeedLedgeWalk = 15.0f;
				FLOAT anchoringCorrectionSpeed = (LocomotionMode == LM_LedgeHang) ? anchoringCorrectionSpeedLedgeHang : anchoringCorrectionSpeedLedgeWalk;

				if (correctionVelocity.SizeSquared() > Square(anchoringCorrectionSpeed))
				{
					correctionVelocity = anchoringCorrectionSpeed*correctionVelocity.SafeNormal();
				}
				Velocity += correctionVelocity;
			}
		}
		break;
	case LM_Squeeze:
		{
			// Apply required correction to position on our walk line
			FVector closestPoint;
			PointDistToSegment(Location, ActiveSqueeze->Node2->Location, ActiveSqueeze->Node1->Location, closestPoint);
			FVector toClosestPoint2d = (closestPoint - Location).SafeNormal2D();
			UBOOL onWrongSideOfCenter = (toClosestPoint2d | CharForward) < 0.0f;
			FVector anchoredPos = closestPoint - (onWrongSideOfCenter ? -1.0f : 1.0f)*SqueezeDistFromCenter*toClosestPoint2d;
			anchoredPos.Z = Location.Z;
			FVector anchoringError = anchoredPos - Location;
			FVector correctionVelocity = appIsNearlyZero(deltaTime) ? FVector(0.0f) : anchoringError/deltaTime;
			TWEAKABLE FLOAT AnchoringCorrectionSpeedSqueeze = 60.0f;
			if (correctionVelocity.SizeSquared() > Square(AnchoringCorrectionSpeedSqueeze))
			{
				correctionVelocity = AnchoringCorrectionSpeedSqueeze*correctionVelocity.SafeNormal();
			}
			Velocity += correctionVelocity;
		}
		break;
	case LM_Locker:
		{
			// Correct any loss of precision during blending - accurately position on spot
			check (ActiveLocker);
			FVector anchoredPos = ActiveLocker->Location;
			anchoredPos.Z = Location.Z;
			FVector anchoringError = anchoredPos - Location;
			FVector correctionVelocity = appIsNearlyZero(deltaTime) ? FVector(0.0f) : anchoringError/deltaTime;
			TWEAKABLE FLOAT AnchoringCorrectionSpeedSqueeze = 60.0f;
			if (correctionVelocity.SizeSquared() > Square(AnchoringCorrectionSpeedSqueeze))
			{
				correctionVelocity = AnchoringCorrectionSpeedSqueeze*correctionVelocity.SafeNormal();
			}
			Velocity = correctionVelocity;
			DesiredMoveDirection = FVector(0.0f);
		}
		break;
	case LM_Ladder:
		{
			TWEAKABLE FLOAT MaxSpeedForCorrection = 100.0f; // limit correction to slower speeds, as it can be unreliable at low FPS
			if (Abs(RealVelocity.Z) < MaxSpeedForCorrection)
			{
				// Apply correction
				TWEAKABLE FLOAT AnchoringCorrectionSpeedLadder = 60.0f;

				FVector anchoredPos = ActiveLadder->Location + LadderDistFwd*ActiveLadder->Rotation.Vector();
				anchoredPos.Z = Location.Z;

				check(LadderAnimNode);
				FLOAT currentRatio = LadderAnimNode->GetCurrentPositionRatio();

				FLOAT distToBottom = Location.Z - LadderRootHeightOffsetFromBar - ActiveLadder->Location.Z;
				INT nbFullBars = (INT)appFloor(distToBottom / LadderBarSpacing);
				FLOAT leftOver = (distToBottom - LadderBarSpacing*(FLOAT)nbFullBars) / LadderBarSpacing;

				// Fix slight imprecisions when just around a bar
				if (currentRatio < 0.25f && leftOver > 0.75f)
				{
					nbFullBars++; // we're technically just past a bar but our distance is slightly below
				}
				else if (currentRatio > 0.75f && leftOver < 0.25f)
				{
					nbFullBars--; // we're technically just below a bar but our distance is slightly above
				}

				FLOAT distToLowerBar = nbFullBars * LadderBarSpacing;
				FLOAT distToUpperBar = (nbFullBars + 1) * LadderBarSpacing;
				FLOAT desiredDist = distToLowerBar + currentRatio*LadderBarSpacing;

				anchoredPos.Z = ActiveLadder->Location.Z + desiredDist + LadderRootHeightOffsetFromBar;

				FVector anchoringError = anchoredPos - Location;
				FVector correctionVelocity = appIsNearlyZero(deltaTime) ? FVector(0.0f) : anchoringError/deltaTime;			
				if (correctionVelocity.SizeSquared() > Square(AnchoringCorrectionSpeedLadder))
				{
					correctionVelocity = AnchoringCorrectionSpeedLadder*correctionVelocity.SafeNormal();
				}
				Velocity += correctionVelocity;
			}
		}
		break;
	case LM_Pushing:
		{
			check(ActivePushable);
			const FVector& edge = bPushingFromBackEdge ? ActivePushable->GetCurrentBackEdge() : ActivePushable->GetCurrentFwdEdge();
			FVector pushDirection = bPushingFromBackEdge ? ActivePushable->GetFwdDirection() : ActivePushable->GetBackDirection();
			FVector anchoredPos = edge - PushableExpectedDistFromEdge*pushDirection + PushableExpectedSideOffset*ActivePushable->GetSideDirection();
			anchoredPos.Z = Location.Z;
			FVector anchoringError = anchoredPos - Location;
			FVector correctionVelocity = appIsNearlyZero(deltaTime) ? FVector(0.0f) : anchoringError/deltaTime;
			TWEAKABLE FLOAT AnchoringCorrectionSpeedPushable = 60.0f;
			if (correctionVelocity.SizeSquared() > Square(AnchoringCorrectionSpeedPushable))
			{
				correctionVelocity = AnchoringCorrectionSpeedPushable*correctionVelocity.SafeNormal();
			}
			Velocity += correctionVelocity;
		}
		break;
	}
}

void AOLHero::ApplyObstacleAvoidance(FLOAT deltaTime, const FVector& AccelDir)
{
	if (LocomotionMode != LM_Walk || Physics != PHYS_Walking || SpecialMove != SMT_None)
	{
		return;
	}

	if (!IsRunning() && CornerPeek.CornerMarker)
	{
		// We disable obstacle avoidance when closing in towards a valid corner marker
		return;
	}

	if (Velocity.SizeSquared2D() < Square(10.0f))
	{
		// Barely moving
		return;
	}

	static UBOOL debugAvoidance = FALSE;

	TWEAKABLE FLOAT SideOffsetDist = DefaultPawn->CylinderComponent->CollisionRadius + 10.0f;
	TWEAKABLE FLOAT HorzRayLength = 200.0f;
	TWEAKABLE FLOAT HorzRayHeight = 50.0f;
	TWEAKABLE FLOAT VertRayFwdDist = 80.0f;
	TWEAKABLE FLOAT VertRayStartHeightStand = 180.0f;
	TWEAKABLE FLOAT VertRayStartHeightCrouched = 80.0f;
	TWEAKABLE FLOAT VertRayEndHeight = 50.0f;

	FVector fwdVec = Velocity.SafeNormal();
	FVector rightVec = Velocity.Rotation().Right();

	FLOAT leftObstDist = -1.0f;
	FLOAT rightObstDist = -1.0f;

	// Left
	FCheckResult Hit(1.f);
	FVector startTrace = Location - SideOffsetDist*rightVec + VecZ(HorzRayHeight);
	FVector endTrace = startTrace + HorzRayLength*fwdVec;
	UBOOL clearLeft = GWorld->SingleLineCheck( Hit, this, endTrace, startTrace, TRACE_AllBlocking, FVector(0.0f));
	
	if (!clearLeft)
	{
		leftObstDist = Hit.Time * HorzRayLength;

		if (debugAvoidance)
		{
			GWorld->GetWorldInfo()->DrawDebugLine(startTrace, startTrace + leftObstDist*fwdVec, 255, 0, 0, FALSE);
		}
	}
	else
	{
		if (debugAvoidance)
		{
			GWorld->GetWorldInfo()->DrawDebugLine(startTrace, endTrace, 0, 255, 0, FALSE);
		}

		// Double-check with a vertical trace (to catch e.g. tables)

		// Left vertical
		FVector baseLoc = Location - SideOffsetDist*rightVec + VertRayFwdDist*fwdVec;
		startTrace = baseLoc + (bIsCrouched ? VecZ(VertRayStartHeightCrouched) : VecZ(VertRayStartHeightStand));
		endTrace = baseLoc + VecZ(VertRayEndHeight);
		clearLeft = GWorld->SingleLineCheck( Hit, this, endTrace, startTrace, TRACE_AllBlocking | TRACE_StopAtAnyHit, FVector(0.0f));

		if (!clearLeft)
		{
			leftObstDist = VertRayFwdDist;

			if (debugAvoidance)
			{
				GWorld->GetWorldInfo()->DrawDebugLine(startTrace, endTrace, 255, 0, 0, FALSE);
			}
		}
		else if (debugAvoidance)
		{
			GWorld->GetWorldInfo()->DrawDebugLine(startTrace, endTrace, 0, 255, 0, FALSE);
		}
	}

	// Right horizontal
	startTrace = Location + SideOffsetDist*rightVec + VecZ(HorzRayHeight);
	endTrace = startTrace + HorzRayLength*fwdVec;
	UBOOL clearRight = GWorld->SingleLineCheck( Hit, this, endTrace, startTrace, TRACE_AllBlocking, FVector(0.0f));
	
	if (!clearRight)
	{
		rightObstDist = Hit.Time * HorzRayLength;

		if (debugAvoidance)
		{
			GWorld->GetWorldInfo()->DrawDebugLine(startTrace, startTrace + rightObstDist*fwdVec, 255, 0, 0, FALSE);
		}
	}
	else
	{
		if (debugAvoidance)
		{
			GWorld->GetWorldInfo()->DrawDebugLine(startTrace, endTrace, 0, 255, 0, FALSE);
		}

		// Double-check with a vertical trace (to catch e.g. tables)

		// Left vertical
		FVector baseLoc = Location + SideOffsetDist*rightVec + VertRayFwdDist*fwdVec;
		startTrace = baseLoc + (bIsCrouched ? VecZ(VertRayStartHeightCrouched) : VecZ(VertRayStartHeightStand));
		endTrace = baseLoc + VecZ(VertRayEndHeight);
		clearRight = GWorld->SingleLineCheck( Hit, this, endTrace, startTrace, TRACE_AllBlocking | TRACE_StopAtAnyHit, FVector(0.0f));

		if (!clearLeft)
		{
			rightObstDist = VertRayFwdDist;

			if (debugAvoidance)
			{
				GWorld->GetWorldInfo()->DrawDebugLine(startTrace, endTrace, 255, 0, 0, FALSE);
			}
		}
		else if (debugAvoidance)
		{
			GWorld->GetWorldInfo()->DrawDebugLine(startTrace, endTrace, 0, 255, 0, FALSE);
		}
	}

	if (clearLeft && clearRight)
	{
		// All clear
		return;
	}
	else if (!clearLeft && !clearRight)
	{
		// About to hit something, but can't do much here
		return;
	}

	FLOAT obstDist = Max(leftObstDist, rightObstDist);

	TWEAKABLE FLOAT MaxCorrectionDist = 50.0f;
	TWEAKABLE FLOAT MinCorrectionDist = 100.0f;
	TWEAKABLE FLOAT MaxCorrectionPct = 0.25f;

	FLOAT correctionPct = (obstDist - MinCorrectionDist) / (MaxCorrectionDist - MinCorrectionDist);
	correctionPct = Clamp(correctionPct, 0.0f, 1.0f) * MaxCorrectionPct;
	FVector correctionVel = correctionPct * Velocity.Size2D() * (clearLeft ? -rightVec : rightVec);

	Velocity += correctionVel;

	if (debugAvoidance)
	{
		GWorld->GetWorldInfo()->DrawDebugLine(Location + VecZ(100.0f), Location + VecZ(100.0f) + Velocity, 158, 108, 204, FALSE);
		GWorld->GetWorldInfo()->DrawDebugLine(Location + VecZ(100.0f), Location + VecZ(100.0f) + correctionVel, 242, 110, 44, FALSE);
	}
}

void AOLHero::ApplyExternalImpulse(FLOAT deltaTime)
{
	if (!ExternalImpulse.IsNearlyZero())
	{
		if (LocomotionMode == LM_Walk || LocomotionMode == LM_Fall)
		{
			Velocity += ExternalImpulse;
		}

		ExternalImpulse = Utils::Approach(ExternalImpulse, FVector(0.0f), ExternalImpulseDecelCoeff, deltaTime);		
		if (ExternalImpulse.SizeSquared2D() < Square(ExternalImpulseMinVel))
		{
			ExternalImpulse = FVector(0.0f);
		}
	}
}

void AOLHero::ProcessRotation(FLOAT deltaTime)
{
	if (HeroControl && HeroControl->LookAtTarget)
	{
		if (HeadingLockedToCamera())
		{
			SetRotation(FRotator(0, Camera->BaseRotation.Yaw, 0));
		}
	}
	if (HeadingLockedToCamera())
	{		
		FLOAT camInput = Camera->InputYaw;
		FLOAT correctionInput = Camera->ConsumeYawCorrection(deltaTime);
		FLOAT totalDelta = camInput + correctionInput;
		FRotator newRot(0, Rotation.Yaw + (INT)(DEG_TO_UNR * totalDelta), 0);
		SetRotation(newRot.GetNormalized());
	}
	else if (LocomotionMode == LM_LedgeHang || LocomotionMode == LM_LedgeWalk)
	{
		TWEAKABLE FLOAT rotationCorrectionRate = 30.0f * DEG_TO_UNR;

		FVector closestPoint;
		PointDistToSegment(Location, ActiveLedge->Location, ActiveLedge->Next->Location, closestPoint);
		FVector toClosestPoint2d = (closestPoint - Location).SafeNormal2D();
		FVector desiredFwd = (toClosestPoint2d | CharForward) > 0.0f ? toClosestPoint2d : -toClosestPoint2d;
		FLOAT deltaYaw = (FLOAT)FRotator::NormalizeAxis(desiredFwd.Rotation().Yaw - Rotation.Yaw);		
		FLOAT maxCorrectionThisFrame = rotationCorrectionRate * deltaTime;
		FLOAT correction = Abs(deltaYaw) < maxCorrectionThisFrame ? deltaYaw : maxCorrectionThisFrame*Sgn(deltaYaw);
		FRotator newRot = FRotator(Rotation.Pitch, Rotation.Yaw + (INT)correction, 0);

		SetRotation(newRot);
	}
	else if (LocomotionMode == LM_Squeeze)
	{
		TWEAKABLE FLOAT rotationCorrectionRate = 30.0f * DEG_TO_UNR;

		// Apply required correction to position on our walk line
		FVector squeezeLine = (ActiveSqueeze->Node2->Location - ActiveSqueeze->Node1->Location).SafeNormal2D();
		FVector desiredFwd = squeezeLine ^ VecZ(1.0f);
		if ((desiredFwd | CharForward) < 0.0f)
		{
			desiredFwd = -desiredFwd;
		}
		
		FLOAT deltaYaw = (FLOAT)FRotator::NormalizeAxis(desiredFwd.Rotation().Yaw - Rotation.Yaw);		
		FLOAT maxCorrectionThisFrame = rotationCorrectionRate * deltaTime;
		FLOAT correction = Abs(deltaYaw) < maxCorrectionThisFrame ? deltaYaw : maxCorrectionThisFrame*Sgn(deltaYaw);
		FRotator newRot = FRotator(Rotation.Pitch, Rotation.Yaw + (INT)correction, 0);

		SetRotation(newRot);
	}

	EyeForward = EyeRotation.Vector();
	CharForward = Rotation.Vector();
}

void AOLHero::NotifyBump(AActor *Other, UPrimitiveComponent* OtherComp, const FVector &HitNormal)
{
	AOLEnemyPawn* enemy = Cast<AOLEnemyPawn>(Other);

	if (GIsGame && enemy != NULL && enemy->CanBeKnockedback())
	{
		TWEAKABLE FLOAT maxAngleToKnockback = 60.0f;

		FVector toEnemy = (enemy->Location - Location);
		
		if (toEnemy.Z > 50.0f)
		{
			// he's on top of us (we're likely crouched, possibly after a jump over) - send him straight ahead
			enemy->StartKnockback(this, enemy->Rotation.Vector());
		}
		else
		{
			FVector toEnemy2D = toEnemy.SafeNormal2D();
			FLOAT angleToEnemy = Utils::NormalizeRotAngle((Rotation.Yaw - toEnemy2D.Rotation().Yaw) * UNR_TO_DEG);

			if (Abs(angleToEnemy) < maxAngleToKnockback)
			{
				enemy->StartKnockback(this, toEnemy2D);
			}
		}
	}
}

////////////////////////////////////////////////////////////////////////////////////////////
// AI
////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////

FVector AOLHero::GetAIPosition(AOLEnemyPawn* EnemyPawn)
{
	if (EnemyPawn == NULL)
	{
		return FVector(0.f);
	}

	for (INT i = 0; i < AIPositionPawns.Num(); ++i)
	{
		if (AIPositionPawns(i).Enemy == EnemyPawn)
		{
			if (AIPositionPawns(i).PointIndex == -1)
			{
				return Location + (EnemyPawn->Location - Location).SafeNormal2D() * AIPositionDistance;
			}

			checkf(AIPositionPawns(i).PointIndex >= 0 && AIPositionPawns(i).PointIndex < AIPositionPoints.Num(), TEXT("Index is %i"), AIPositionPawns(i).PointIndex);

			FAIPositionPoint& Point = AIPositionPoints(AIPositionPawns(i).PointIndex);

			INT Layer = 0;
			while (Layer < Point.PawnsOccupying.Num())
			{
				if (Point.PawnsOccupying(Layer) == EnemyPawn)
				{
					break;
				}

				++Layer;
			}

			return Location + Point.Location + Point.Location.SafeNormal() * Layer * AIPositionLayerBuffer;
		}
	}

	// Could not find Pawn in AIPositionPawns
	INT PawnIndex = AIPositionPawns.AddZeroed();
	
	AIPositionPawns(PawnIndex).Enemy = EnemyPawn;
	AIPositionPawns(PawnIndex).PointIndex = -1;

	UpdateAIPositionPawn(0.f, PawnIndex, TRUE);

	if (AIPositionPawns(PawnIndex).PointIndex == -1)
	{
		return Location + (EnemyPawn->Location - Location).SafeNormal2D() * AIPositionDistance;
	}

	return Location + AIPositionPoints(AIPositionPawns(PawnIndex).PointIndex).Location;
}

void AOLHero::RemoveEnemyFromAIPositionPawns(AOLEnemyPawn* EnemyPawn)
{
	INT Idx = 0;
	while (Idx < AIPositionPawns.Num())
	{
		if (AIPositionPawns(Idx).Enemy == EnemyPawn)
		{
			RemoveEnemyFromPoint(AIPositionPawns(Idx).Enemy, AIPositionPawns(Idx).PointIndex);

			AIPositionPawns.Remove(Idx);

			break;
		}
		else
		{
			++Idx;
		}
	}
}

void AOLHero::GenerateAIPositions()
{
	AIPositionPoints.Empty();
	AIPositionPawns.Empty();

	FLOAT CurrRotation;
	FVector CurrPosition;
	for (INT i = 0; i < NumAIPositions; ++i)
	{
		CurrRotation = i * ((2 * PI)/NumAIPositions);
		CurrPosition = FVector(1.0f, 0.0f, 0.0f).RotateAngleAxis(CurrRotation * RAD_TO_UNR, FVector(0.f, 0.f, 1.f)) * AIPositionDistance;

		INT Idx = AIPositionPoints.AddZeroed();
		AIPositionPoints(Idx).Location = CurrPosition;
	}
}

void AOLHero::AddEnemyToPoint(AOLEnemyPawn* Enemy, INT PointIndex, INT Layer)
{
	if (Enemy == NULL || PointIndex == -1 || Layer == -1)
	{
		return;
	}

	while (AIPositionPoints(PointIndex).PawnsOccupying.Num() <= Layer)
	{
		AIPositionPoints(PointIndex).PawnsOccupying.AddZeroed();
	}
	checkSlow(AIPositionPoints(PointIndex).PawnsOccupying(Layer) == NULL || AIPositionPoints(PointIndex).PawnsOccupying(Layer) == Enemy);
	AIPositionPoints(PointIndex).PawnsOccupying(Layer) = Enemy;

	INT PtIdx;
	for (INT i = 1; i <= NumAIPositionsPerSidePerPawn; ++i)
	{
		PtIdx = PointIndex + i;

		if (PtIdx >= AIPositionPoints.Num())
		{
			PtIdx -= AIPositionPoints.Num();
		}

		while (AIPositionPoints(PtIdx).PawnsOccupying.Num() <= Layer)
		{
			AIPositionPoints(PtIdx).PawnsOccupying.AddZeroed();
		}
		checkSlow(AIPositionPoints(PtIdx).PawnsOccupying(Layer) == NULL || AIPositionPoints(PtIdx).PawnsOccupying(Layer) == Enemy);
		AIPositionPoints(PtIdx).PawnsOccupying(Layer) = Enemy;

		PtIdx = PointIndex - i;

		if (PtIdx < 0)
		{
			PtIdx += AIPositionPoints.Num();
		}

		while (AIPositionPoints(PtIdx).PawnsOccupying.Num() <= Layer)
		{
			AIPositionPoints(PtIdx).PawnsOccupying.AddZeroed();
		}
		checkSlow(AIPositionPoints(PtIdx).PawnsOccupying(Layer) == NULL || AIPositionPoints(PtIdx).PawnsOccupying(Layer) == Enemy);
		AIPositionPoints(PtIdx).PawnsOccupying(Layer) = Enemy;
	}
}

void AOLHero::RemoveEnemyFromPoint(AOLEnemyPawn* Enemy, INT PointIndex)
{
	if (Enemy == NULL || PointIndex == -1)
	{
		return;
	}

	INT Index = AIPositionPoints(PointIndex).PawnsOccupying.FindItemIndex(Enemy);
	checkSlow(Index != -1);
	AIPositionPoints(PointIndex).PawnsOccupying(Index) = NULL;

	INT PtIdx;
	for (INT i = 1; i <= NumAIPositionsPerSidePerPawn; ++i)
	{
		PtIdx = PointIndex + i;

		if (PtIdx >= AIPositionPoints.Num())
		{
			PtIdx -= AIPositionPoints.Num();
		}

		Index = AIPositionPoints(PtIdx).PawnsOccupying.FindItemIndex(Enemy);
		checkSlow(Index != -1);
		AIPositionPoints(PtIdx).PawnsOccupying(Index) = NULL;

		PtIdx = PointIndex - i;

		if (PtIdx < 0)
		{
			PtIdx += AIPositionPoints.Num();
		}

		Index = AIPositionPoints(PtIdx).PawnsOccupying.FindItemIndex(Enemy);
		checkSlow(Index != -1);
		AIPositionPoints(PtIdx).PawnsOccupying(Index) = NULL;
	}
}

void AOLHero::UpdateAIPositions(FLOAT deltaTime)
{
	AOLEnemyPawn* TestPawn = NULL;

	INT i = 0;
	while (i < AIPositionPawns.Num())
	{
		if (AIPositionPawns(i).Enemy != NULL && !AIPositionPawns(i).Enemy->IsPendingKill()
			&& AIPositionPawns(i).Enemy->Bot != NULL && !AIPositionPawns(i).Enemy->Bot->IsPendingKill())
		{
			TestPawn = AIPositionPawns(i).Enemy;
			break;
		}
		else
		{
			++i;
		}
	}

	for (INT i = 0; i < NumAIPositionsToCheckPerFrame; ++i)
	{
		++LastAIPositionChecked;

		if (LastAIPositionChecked >= AIPositionPoints.Num())
		{
			LastAIPositionChecked -= AIPositionPoints.Num();
		}

		FAIPositionPoint& CurrentPoint = AIPositionPoints(LastAIPositionChecked);

		FLOAT ZAdjust = MaxStepHeight * 0.5f;

		FVector Extent = GetCylinderExtent();
		Extent.Z -= ZAdjust;

		UBOOL bValid = TRUE;
		FMemMark Mark(GMainThreadMemStack);
		FCheckResult* FirstHit = NULL;
		FirstHit = GWorld->MultiLineCheck(GMainThreadMemStack, Location + CurrentPoint.Location + CollisionComponent->Translation + VecZ(ZAdjust), Location + CollisionComponent->Translation + VecZ(ZAdjust), Extent, TRACE_AllBlocking, this);
		for( FCheckResult* Hit = FirstHit; Hit; Hit = Hit->GetNext() )
		{
			if(Hit->Actor != NULL && (Hit->Actor->bWorldGeometry || Hit->Actor->IsA(AOLDoor::StaticClass())))
			{
				bValid = FALSE;
				break;
			}
		}
		Mark.Pop();

		CurrentPoint.bValid = bValid;

		FVector PointOnNavMesh(0.f);
		if (CurrentPoint.bValid && TestPawn != NULL && TestPawn->Bot->GetClosestPointOnNavMesh(PointOnNavMesh, Location + CurrentPoint.Location))
		{
			CurrentPoint.LocationOnNavMesh = PointOnNavMesh;
		}
		else
		{
			CurrentPoint.LocationOnNavMesh = FVector(0.f);
		}
	}

	UBOOL bUnder = CanBeGrabbedUnder();

	INT Idx = 0;
	while (Idx < AIPositionPawns.Num())
	{
		if (AIPositionPawns(Idx).Enemy == NULL || AIPositionPawns(Idx).Enemy->IsPendingKill())
		{
			RemoveEnemyFromPoint(AIPositionPawns(Idx).Enemy, AIPositionPawns(Idx).PointIndex);

			AIPositionPawns.Remove(Idx);
		}
		else
		{
			UpdateAIPositionPawn(deltaTime, Idx, bWasUnder != bUnder);
			++Idx;
		}
	}

	bWasUnder = bUnder;
}

UBOOL AOLHero::CanEnemyOccupy(AOLEnemyPawn* Enemy, INT PointIndex, INT Layer)
{
	if (AIPositionPoints(PointIndex).PawnsOccupying.Num() > Layer
		&& AIPositionPoints(PointIndex).PawnsOccupying(Layer) != NULL 
		&& AIPositionPoints(PointIndex).PawnsOccupying(Layer) != Enemy)
	{
		return FALSE;
	}

	INT PtIdx;
	for (INT i = 1; i <= NumAIPositionsPerSidePerPawn; ++i)
	{
		PtIdx = PointIndex + i;

		if (PtIdx >= AIPositionPoints.Num())
		{
			PtIdx -= AIPositionPoints.Num();
		}

		if (AIPositionPoints(PtIdx).PawnsOccupying.Num() > Layer
			&& AIPositionPoints(PtIdx).PawnsOccupying(Layer) != NULL 
			&& AIPositionPoints(PtIdx).PawnsOccupying(Layer) != Enemy)
		{
			return FALSE;
		}

		PtIdx = PointIndex - i;

		if (PtIdx < 0)
		{
			PtIdx += AIPositionPoints.Num();
		}

		if (AIPositionPoints(PtIdx).PawnsOccupying.Num() > Layer
			&& AIPositionPoints(PtIdx).PawnsOccupying(Layer) != NULL 
			&& AIPositionPoints(PtIdx).PawnsOccupying(Layer) != Enemy)
		{
			return FALSE;
		}
	}

	return TRUE;
}

void AOLHero::UpdateAIPositionPawn(FLOAT deltaTime, INT PawnIndex, UBOOL bForce)
{
	TWEAKABLE FLOAT ClosestThreshold = 150.0f;
	TWEAKABLE FLOAT UpdateTimeMin = 2.0f;
	TWEAKABLE FLOAT UpdateTimeMax = 3.0f;
	TWEAKABLE INT PointChangeRotationMin = 1;
	TWEAKABLE INT PointChangeRotationMax = 4;
	TWEAKABLE FLOAT DotAngle = 60.0f * DEG_TO_RAD;

	if (PawnIndex < 0 || PawnIndex >= AIPositionPawns.Num())
	{
		return;
	}

	FAIPositionPawnInfo& PawnInfo = AIPositionPawns(PawnIndex);

	PawnInfo.TimeToNextChange -= deltaTime;

	FLOAT DistToPoint = 0.f;
	FLOAT DotEnemyToPoint = -1.f;
	if (PawnInfo.PointIndex != -1)
	{
		DistToPoint = (PawnInfo.Enemy->Location - (Location + AIPositionPoints(PawnInfo.PointIndex).Location)).Size2D();

		DotEnemyToPoint = (PawnInfo.Enemy->Location - Location).SafeNormal2D() | AIPositionPoints(PawnInfo.PointIndex).Location.SafeNormal2D();
	}

	FLOAT CosAngle = appCos(DotAngle);

	if (PawnInfo.TimeToNextChange <= 0.f || (PawnInfo.PointIndex == -1 || DistToPoint >= ClosestThreshold || DotEnemyToPoint < CosAngle) || bForce)
	{
		// Find Best Spot for Me
		static TArray<INT> ClosestIndices;
		ClosestIndices.Reset();
		ClosestIndices.AddItem(-1);

		static TArray<FLOAT> ClosestLengthSqr;
		ClosestLengthSqr.Reset();
		ClosestLengthSqr.AddItem(-1.f);

		static TArray<UBOOL> InAttackRange;
		InAttackRange.Reset();
		InAttackRange.AddZeroed(AIPositionPoints.Num());

		INT ClosestInAttackRange = -1;
		FLOAT ClosestInAttackRangeLengthSqr = -1.f;
		
		for (INT i = 0; i < AIPositionPoints.Num(); ++i)
		{
			if (AIPositionPoints(i).bValid)
			{
				INT Layer = 0;
				while(!CanEnemyOccupy(PawnInfo.Enemy, i, Layer))
				{
					++Layer;
				}

				while (Layer >= ClosestIndices.Num())
				{
					ClosestIndices.AddItem(-1);
					ClosestLengthSqr.AddItem(-1.f);
				}

				FLOAT NewDistSquared = (Location + AIPositionPoints(i).Location - PawnInfo.Enemy->Location).SizeSquared2D();
				if (ClosestLengthSqr(Layer) == -1.f || NewDistSquared < ClosestLengthSqr(Layer))
				{
					ClosestLengthSqr(Layer) = NewDistSquared;
					ClosestIndices(Layer) = i;
				}
				
				if (!AIPositionPoints(i).LocationOnNavMesh.IsZero())
				{
					FLOAT RealRangeToPointSquared = (AIPositionPoints(i).LocationOnNavMesh - Location).SizeSquared2D();
					if (Layer == 0 && RealRangeToPointSquared < Square(PawnInfo.Enemy->AttackRange - 10.0f) && PawnInfo.Enemy->Bot->CheckAttackZDiff(AIPositionPoints(i).LocationOnNavMesh))
					{
						InAttackRange(i) = TRUE;

						if (ClosestInAttackRangeLengthSqr == -1.f || NewDistSquared < ClosestInAttackRangeLengthSqr)
						{
							ClosestInAttackRange = i;
							ClosestInAttackRangeLengthSqr = NewDistSquared;
						}
					}
				}
			}
		}

		INT CurrentLayer = -1;
		INT NewIndex = -1;

		if (PawnInfo.PointIndex != -1)
		{
			CurrentLayer = AIPositionPoints(PawnInfo.PointIndex).PawnsOccupying.FindItemIndex(PawnInfo.Enemy);
		}

		if (PawnInfo.PointIndex != -1 && DistToPoint < ClosestThreshold && DotEnemyToPoint >= CosAngle
			&& ClosestIndices.Num() > CurrentLayer && ClosestIndices(CurrentLayer) != -1)
		{
			UBOOL bInvert = appFrand() > 0.5f;
			INT PointChange = PointChangeRotationMin + RandHelper(PointChangeRotationMax - PointChangeRotationMin);

			for (INT i = PointChange; i >= 0; --i)
			{
				INT PtIdx = PawnInfo.PointIndex;

				if (bInvert)
				{
					PtIdx -= i;
					if (PtIdx < 0)
					{
						PtIdx += AIPositionPoints.Num();
					}
				}
				else
				{
					PtIdx += i;
					if (PtIdx >= AIPositionPoints.Num())
					{
						PtIdx -= AIPositionPoints.Num();
					}
				}

				if (AIPositionPoints(PtIdx).bValid 
					&& CanEnemyOccupy(PawnInfo.Enemy, PtIdx, CurrentLayer)
					&& (ClosestInAttackRange == -1 || InAttackRange(i)))
				{
					NewIndex = PtIdx;
					break;
				}
			}
		}

		if (NewIndex == -1)
		{
			if (ClosestInAttackRange != -1)
			{
				NewIndex = ClosestInAttackRange;
				CurrentLayer = 0;
			}
			else
			{
				for (INT i = 0; i < ClosestIndices.Num(); ++i)
				{
					if (ClosestIndices(i) != -1)
					{
						NewIndex = ClosestIndices(i);
						CurrentLayer = i;
						break;
					}
				}
			}
		}

		if (PawnInfo.PointIndex != -1)
		{
			RemoveEnemyFromPoint(PawnInfo.Enemy, PawnInfo.PointIndex);
		}

		PawnInfo.PointIndex = NewIndex;

		if (PawnInfo.PointIndex != -1)
		{
			AddEnemyToPoint(PawnInfo.Enemy, PawnInfo.PointIndex, CurrentLayer);
		}

		PawnInfo.TimeToNextChange = RandRange(UpdateTimeMin, UpdateTimeMax);
	}
}

void AOLHero::DrawDebugAIPositions()
{
	for (INT i = 0; i < AIPositionPoints.Num(); ++i)
	{
		UBOOL bDrawn = FALSE;

		if (!AIPositionPoints(i).bValid)
		{
			DrawDebugSphere( Location + AIPositionPoints(i).Location, 5.0f, 6, 255, 20, 20 );
			bDrawn = TRUE;
		}
		else
		{
			for (INT j = 0; j < AIPositionPoints(i).PawnsOccupying.Num(); ++j)
			{
				if (AIPositionPoints(i).PawnsOccupying(j) != NULL)
				{
					DrawDebugSphere( Location + AIPositionPoints(i).Location + AIPositionPoints(i).Location.SafeNormal() * j * AIPositionLayerBuffer, 5.0f, 6, 255, 20, 255 );
					bDrawn = TRUE;
				}
			}
		}

		if (!bDrawn)
		{
			DrawDebugSphere( Location + AIPositionPoints(i).Location, 5.0f, 6, 20, 255, 20 );
		}
	}
}

void AOLHero::UpdateAvgVelocity(FLOAT deltaTime)
{
	AvgVelocityTimer += deltaTime;
	if (AvgVelocityTimer > AvgVelPointLength/NumAvgVelPoints)
	{
		AvgVelocityTimer = 0.f;

		if (AvgVelPoints.Num() == NumAvgVelPoints)
		{
			AvgVelPoints.Remove(0);
		}

		AvgVelPoints.AddItem(FVector2D(Location));

		if (AvgVelPoints.Num() > 1)
		{
			AvgVelocity = FVector(AvgVelPoints.Last() - AvgVelPoints(0), 0.f);
		}
		else
		{
			AvgVelocity = Velocity;
		}
	}
}

FVector AOLHero::GetFutureDestination(AOLPawn* Agent)
{
	AOLEnemyPawn* EnemyPawn = Cast<AOLEnemyPawn>(Agent);

	if (EnemyPawn == NULL)
	{
		return Super::GetFutureDestination(Agent);
	}

	FVector TestPosition = GetAIPosition(EnemyPawn);

	FLOAT DistanceToAgent = Agent->Location.Distance(TestPosition);
	FLOAT TimeToTarget = (DistanceToAgent / Agent->GroundSpeed) + 0.5f;

	FLOAT PredictionDistance = Clamp(AvgVelocity.Size() * TimeToTarget * DestinationPredictionFactor, 0.0f, DestinationPredictionMax);

	FVector NewDestination = TestPosition + AvgVelocity.SafeNormal() * PredictionDistance;

	if (PredictionDistance > 0.f)
	{
		FVector ToPlayerNormal = (Location - EnemyPawn->Location).SafeNormal2D();
		FVector ToDestination = NewDestination - EnemyPawn->Location;
		FVector ToDestinationNormal = ToDestination.SafeNormal2D();

		if (ToPlayerNormal.Dot2d(ToDestinationNormal) < 0.f)
		{
			NewDestination = EnemyPawn->Location + ToDestination.ProjectOnTo(ToPlayerNormal.RotateAngleAxis(90.0f * DEG_TO_UNR, FVector(0.f, 0.f, 1.f)));
		}
	}

	FLOAT ZAdjust = MaxStepHeight * 0.5f;

	FVector Extent = GetCylinderExtent();
	Extent.Z -= ZAdjust;

	FMemMark Mark(GMainThreadMemStack);
	FCheckResult* FirstHit = NULL;
	FirstHit = GWorld->MultiLineCheck(GMainThreadMemStack, NewDestination + CollisionComponent->Translation + VecZ(ZAdjust), Location + CollisionComponent->Translation + VecZ(ZAdjust), Extent, TRACE_AllBlocking, this);
	for( FCheckResult* Hit = FirstHit; Hit; Hit = Hit->GetNext() )
	{
		if(Hit->Actor != NULL && (Hit->Actor->bWorldGeometry || Hit->Actor->IsA(AOLDoor::StaticClass())))
		{
			NewDestination = Hit->Location - CollisionComponent->Translation;
			break;
		}
	}
	Mark.Pop();

	if (Abs(EnemyPawn->Bot->DistToGround(NewDestination)) > CylinderComponent->CollisionHeight * 2.f)
	{
		return TestPosition;
	}

	return NewDestination;
}

FVector AOLHero::GetDisturbanceLocation(AOLEnemyPawn* Enemy)
{
	if (LocomotionMode == LM_Locker 
		|| (LocomotionMode == LM_SpecialMove && SpecialMove == SMT_EnterLocker))
	{
		AOLDoor* Door = ActiveLocker->AssociatedDoor;
		return Door->GetCenterLocation() + Door->GetStaticDirection() * 120.0f;
	}
	
	if (LocomotionMode == LM_Bed
		|| (LocomotionMode == LM_SpecialMove && SpecialMove == SMT_EnterBed))
	{
		BYTE BedSide = 0;
		return Enemy->Bot->GetBestBedDestination(ActiveBed, BedSide, TRUE);
	}

	return Location;
}

FVector AOLHero::GetGrabUnderDestination(AOLEnemyPawn* Enemy)
{
	return GetAIPosition(Enemy);
}

////////////////////////////////////////////////////////////////////////////////////////////
// Debug
////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////

void AOLHero::QuickHeroTest()
{	
	AOLPlayerController* OLPC = Utils::GetOLPC();

	if (OLPC)
	{
		OLPC->GameOver();
	}
}

void AOLHero::NativeDisplayDebug(UCanvas* canvas, FLOAT& out_YL, FLOAT& out_YPos)
{
	canvas->SetDrawColor(32, 255, 196);

	if (OLPC->InventoryManager->OwnedInventory.Num() > 0)
	{
		FString buf(TEXT("Inventory: "));

		for (INT i = 0; i < OLPC->InventoryManager->OwnedInventory.Num(); i++)
		{
			if (i == 0)
			{
				buf += OLPC->InventoryManager->OwnedInventory(i).ToString();
			}
			else
			{
				buf += FString(TEXT(", ")) + OLPC->InventoryManager->OwnedInventory(i).ToString();
			}
		}
		
		canvas->SetPos(4, out_YPos);
		canvas->DrawText(buf);
		out_YPos += out_YL;	
	}
		
	canvas->SetPos(4, out_YPos);
	canvas->DrawText(FString::Printf(TEXT("LocomotionMode: %s, Physics: %s, In Darkness: %s"), *Utils::GetEnumString("ELocomotionMode", LocomotionMode), *Utils::GetEnumString("EPhysics", Physics), IsInDarkness() ? TEXT("Yes") : TEXT("No")));
	out_YPos += out_YL;	

	canvas->SetPos(4, out_YPos);
	canvas->DrawText(FString::Printf(TEXT("Forward: [%s], EyeForward: [%s], EyeLocation: [%s], FOV: %.0f"), *CharForward.ToString(), *EyeForward.ToString(), *EyeLocation.ToString(), CurrentFOV));
	out_YPos += out_YL;	

	canvas->SetPos(4, out_YPos);
	canvas->DrawText(FString::Printf(TEXT("MeshZOffset: %.1f, MeshXOffset: %.1f"), MeshZOffset, MeshXOffset));
	out_YPos += out_YL;	

	if (bLeftHandIKActive)
	{
		canvas->SetPos(4, out_YPos);

		FString ikSrc = TEXT("[unknown]");

		if (CrouchTurnOnSpotAnimNode && LocomotionMode == LM_Walk && bIsCrouched)
		{
			ikSrc = TEXT("CrouchedToS");
		}
		else if (LeftHandIKData.bActive && (LeftHandIKData.IKTarget == IKTT_DoorKnob))
		{
			ikSrc = TEXT("DoorKnob");
		}
		else if (LeftHandIKData.bActive && (LeftHandIKData.IKTarget == IKTT_CSAPropDestination))
		{
			ikSrc = TEXT("CSAProp");
		}
		else if (CornerPeek.IKStrength > 0.01f)
		{
			ikSrc = TEXT("CornerMarker");
		}

		canvas->DrawText(FString::Printf(TEXT("[Left Hand IK] Strength: %.0f%%, With Rotation: %s, Source: %s"), 100.0f*LeftHandIK->ControlStrength, LeftHandIK->bUseEffectorRotationLS ? TEXT("Yes") : TEXT("No"), *ikSrc));
		out_YPos += out_YL;	
	}

	if (OLPC->Struggle.bActiveStrugging)
	{
		canvas->SetPos(4, out_YPos);
		canvas->DrawText(FString::Printf(TEXT("[Struggle] Shakes: %.1f, TotalDeltas: %.0f, WinPoints: %.0f"), OLPC->Struggle.NbShakes, OLPC->Struggle.TotalDeltas, (OLPC->Struggle.NbShakes, OLPC->Struggle.TotalDeltas)));
		out_YPos += out_YL;	
	}

	if (bHobbling)
	{
		canvas->SetPos(4, out_YPos);
		canvas->DrawText(FString::Printf(TEXT("[Hobbling] Target Intensity: %.1f%%, Current Intensity: %.1f%%"), 100.0f*TargetHobblingIntensity, 100.0f*HobblingIntensity));
		out_YPos += out_YL;	
	}

	{
		canvas->SetDrawColor(255, 137, 33);
		canvas->SetPos(4, out_YPos);
		canvas->DrawText(FString::Printf(TEXT("[RTPC_PlayerSpeed] Target: %.1f, Current: %.1f"), TargetPlayerSpeedRTPC, CurrentPlayerSpeedRTPC));
		out_YPos += out_YL;	
		canvas->SetDrawColor(32, 255, 196);
	}

	if (AdjustPosition.Active)
	{
		canvas->SetPos(4, out_YPos);
		FLOAT pctDone = AdjustPosition.Done ? 100.0f : 100.0f*AdjustPosition.ElapsedTime / AdjustPosition.CorrectionTime;
		canvas->DrawText(FString::Printf(TEXT("AdjustPosition: %.0f%% (%.2fs/%.2fs), pos: [%s], rot: %.1f degs"), pctDone, AdjustPosition.ElapsedTime, AdjustPosition.CorrectionTime, *AdjustPosition.PositionError.ToString(), AdjustPosition.HeadingError));
		out_YPos += out_YL;	
	}

	for (INT i = 0; i < ProceduralAnims.Num(); i++)
	{
		FProceduralAnimData& animData = ProceduralAnims(i);

		canvas->SetPos(4, out_YPos);
		FLOAT pctDone = 100.0f*animData.ElapsedTime / animData.TotalTime;
		FString pendingStr = (animData.bWaitForNotify || i > 0) ? TEXT("PENDING - ") : TEXT("");
		canvas->DrawText(FString::Printf(TEXT("ProceduralAnims[%d]: %s%.0f%% (%.2fs/%.2fs), pos: [%s], rot: %.1f degs"), i, *pendingStr, pctDone, animData.ElapsedTime, animData.TotalTime, *animData.PositionDelta.ToString(), animData.HeadingDelta));
		out_YPos += out_YL;
	}

	if (HeroControl)
	{
		canvas->SetPos(4, out_YPos);
		FLOAT pctDone = 100.0f*HeroControl->ElapsedTime / HeroControl->Duration;
		FString gotoStr = HeroControl->GoToTarget ? HeroControl->GoToTarget->GetName() : TEXT("[N/A]");
		FString lookatStr = HeroControl->LookAtTarget ? HeroControl->LookAtTarget->GetName() : TEXT("[N/A]");
		canvas->DrawText(FString::Printf(TEXT("Hero Control: %.0f%% (%.2fs/%.2fs), GoTo: %s, LookAt: %s"), pctDone, HeroControl->ElapsedTime, HeroControl->Duration, *gotoStr, *lookatStr));
		out_YPos += out_YL;
	}

	Camera->DisplayDebug(canvas, out_YL, out_YPos);

	UAnimNode* animNode = Mesh->Animations;
	FLOAT XPos = 4.0f;
	TWEAKABLE FLOAT XL = 7.0f;

	FLOAT startY = out_YPos;

	TArray<UAnimNode*> visitedAnimNodes;
	DrawDebugAnimNode(NAME_None, animNode, visitedAnimNodes, canvas, out_YL, out_YPos, XL, XPos, 0.0f);

	TWEAKABLE UBOOL DebugShadowAnims = FALSE;

	if (DebugShadowAnims)
	{
		animNode = ShadowProxy->Animations;
		out_YPos = startY;
		XPos = 550.0f;
		visitedAnimNodes.Empty();
		DrawDebugAnimNode(NAME_None, animNode, visitedAnimNodes, canvas, out_YL, out_YPos, XL, XPos, 0.0f);
	}
}
