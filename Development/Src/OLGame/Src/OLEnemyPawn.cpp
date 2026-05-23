/**
 * Implementation of OLEnemyPawn for Outlast.
 *
 * Copyright 2012 Red Barrels, Inc. All Rights Reserved.
 */

#include "OLGame.h"
#include "EngineAnimClasses.h"
#include "GameFrameworkAnimClasses.h"
#include "UDKBaseAnimationClasses.h"
#include "OLGameAnimClasses.h"
#include "OLUtilities.h"

IMPLEMENT_CLASS(AOLEnemyPawn);
IMPLEMENT_CLASS(AOLEnemySoldier);
IMPLEMENT_CLASS(AOLEnemyGenericPatient);
IMPLEMENT_CLASS(AOLEnemySurgeon);
IMPLEMENT_CLASS(AOLEnemyGroom);
IMPLEMENT_CLASS(AOLEnemyCannibal);
IMPLEMENT_CLASS(UOLAISteering);
IMPLEMENT_CLASS(UOLAISteeringFollowPath);
IMPLEMENT_CLASS(UOLAISteeringAvoidance);
IMPLEMENT_CLASS(UOLAIContextualVOAsset);

/*============================================================================*/

static const FName RegisterNavMeshObstacleUpdateNaveMeshName( TEXT("UpdateNavMeshObstacle") ); // define at global scope to avoid branch

INT AOLEnemyPawn::CostFor( const FNavMeshPathParams& PathParams, const FVector& PreviousPoint, FVector& out_PathEdgePoint, FNavMeshPathObjectEdge* Edge, FNavMeshPolyBase* SourcePoly )
{
	AOLBot* Bot = Cast<AOLBot>(PathParams.Interface->GetUObjectInterfaceInterface_NavigationHandle());
	if( Bot != NULL && Bot->Pawn != this )
	{
		// if it's someone else, return a super expensive cost so they go around unless there is no other path
		return 5000;
	}

	return Edge->FNavMeshEdgeBase::CostFor(PathParams, PreviousPoint, out_PathEdgePoint, SourcePoly);
}

void AOLEnemyPawn::RegisterNavMeshObstacle()
{
	if ( !bNavMeshRegistered )
	{
		RegisterObstacleWithNavMesh();
		bNavMeshRegistered=TRUE;
		LastNavMeshCheckLocation=Location;
		LastNavMeshObstacleRegisterTime=GWorld->GetTimeSeconds();
		SetTimer(0.25, FALSE, RegisterNavMeshObstacleUpdateNaveMeshName);
	}
}

void AOLEnemyPawn::UnregisterNavmeshObstacle()
{
	if ( bNavMeshRegistered )
	{
		bNavMeshRegistered=FALSE;
		UnregisterObstacleWithNavMesh();
		ClearTimer(RegisterNavMeshObstacleUpdateNaveMeshName);
	}
}

FLOAT AOLEnemyPawn::PlayTopHalfBlendedAnim(const FName& animNameA, const FName& animNameB, FLOAT alpha, FLOAT blendInTime, FLOAT blendOutTime, FLOAT rate, FLOAT startRatio)
{
	FLOAT RetValue = TopHalfBlend->PlayCustomBlend(animNameA, animNameB, alpha, blendInTime, blendOutTime, rate, startRatio);

	if(RetValue > 0.f)
	{
		PlayingSpecialMoveAnims.AddItem(animNameA);
		PlayingSpecialMoveAnims.AddItem(animNameB);
	}

	return RetValue;
}

void AOLEnemyPawn::StartSpecialMove(BYTE moveType, FVector targetPosition, FVector targetDirection, BYTE targetType)
{
	Super::StartSpecialMove(moveType, targetPosition, targetDirection, targetType);

	SpecialMoveStalledTimestamp = -1.0f;
	MeshZOffset = 0.0f;
}

UBOOL AOLEnemyPawn::ShouldAllowOtherPawnSpecialMove(AOLPawn* otherPawn, ESpecialMoveType moveType, AActor* refActor)
{
	// TODO - Check if we should deny the hero his special move

	return TRUE;
}

void AOLEnemyPawn::OnOtherPawnStartSpecialMove(AOLPawn* otherPawn, ESpecialMoveType moveType, AActor* refActor)
{
	if (!Bot || otherPawn != Utils::GetHero())
	{
		return;
	}

	// TODO - Check to abort / reset our current state			
}

void AOLEnemyPawn::CancelSpecialMove()
{
	if (SpecialMove == SMT_None)
	{
		return;
	}

	const FSpecialMoveParameters& params = SpecialMoveParams[SpecialMove];

	if (!params.bNoAnim)
	{
		if (FullBodyAnimSlot->bIsPlayingCustomAnim)
		{
			FullBodyAnimSlot->StopCustomAnim(0.2f);
		}
		if (CustomBlendNode->bActive)
		{
			CustomBlendNode->StartBlendingOut();
		}
		if (TopHalfBlend->bActive)
		{
			TopHalfBlend->StartBlendingOut();
		}

		bPlayingSpecialMoveAnim = FALSE;
	}

	if (AdjustPosition.Active)
	{
		AdjustPosition.Done = TRUE;
	}

	ClearProceduralAnims();

	SpecialMoveCompleted();
}

UBOOL AOLEnemyPawn::IsSpecialMoveCompleted()
{
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
	if (RealVelocity.IsNearlyZero(KINDA_SMALL_NUMBERF) && appIsNearlyEqual(AnimNodeSelectEnemyMode->NodeTotalWeight, 1.0f))
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

void AOLEnemyPawn::SpecialMoveCompleted()
{
	const FSpecialMoveParameters& params = SpecialMoveParams[SpecialMove];

	ESpecialMoveType completedMove = (ESpecialMoveType)SpecialMove;
	Super::SpecialMoveCompleted();
	
	if (EnemyMode == EM_SqueezeGrabLeft || EnemyMode == EM_SqueezeGrabRight)
	{
		setPhysics(PHYS_Custom);
	}
	else if (Physics != WalkingPhysics)
	{
		setPhysics(WalkingPhysics);
	}

	if (params.GP.CollisionHeight > 0.0f || params.GP.CollisionRadius > 0.0f)
	{
		FLOAT NewHeight = DefaultPawn->CylinderComponent->CollisionHeight;
		FLOAT NewRadius = DefaultPawn->CylinderComponent->CollisionRadius;
		bFailedCollisionSet = !TryAdjustCollisionSize(NewHeight, NewRadius);
	}
}

void AOLEnemyPawn::SpecialMovePhaseCompleted()
{
	//PlayAnimForSpecialMove((ESpecialMoveType)SpecialMove);
}

void AOLEnemyPawn::TickPrePhysics(FLOAT deltaTime)
{
	if (bFailedCollisionSet)
	{
		FLOAT NewHeight = DefaultPawn->CylinderComponent->CollisionHeight;
		FLOAT NewRadius = DefaultPawn->CylinderComponent->CollisionRadius;
		bFailedCollisionSet = !TryAdjustCollisionSize(NewHeight, NewRadius);
	}

	Super::TickPrePhysics(deltaTime);

	if (Bot)
	{
		UpdateMovePath(deltaTime);
		UpdateLocomotion(deltaTime);
		UpdateDelayedVO(deltaTime);
	}
}

void AOLEnemyPawn::TickPostPhysics(FLOAT deltaTime)
{
	Super::TickPostPhysics(deltaTime);

	UpdateFootPlacement(deltaTime);
	UpdateMeshOffset(deltaTime);

	if (Location.Z < FallingDeathZ)
	{
		GWorld->DestroyActor(this);
		return;
	}

	SetRotation(FRotator(0.f, Rotation.Yaw, 0.f));

	MovingTestTimer -= deltaTime;

	if (MovingTestTimer <= 0.f)
	{
		if (MovingTestPoints.Num() >= NumMovingTestPoints)
		{
			MovingTestPoints.Remove(0, 1 + MovingTestPoints.Num() - NumMovingTestPoints);
		}

		MovingTestPoints.AddItem(Location);

		MovingTestTimer = MovingTestLength/NumMovingTestPoints;
	}
}

FLOAT AOLEnemyPawn::CalculateLedgeRatio()
{
	if (Bot->ActiveLedge != NULL)
	{
		FLOAT Height = Bot->ActiveLedge->TopPoint.Z - Bot->ActiveLedge->BottomPoint.Z;
		return Clamp((Height - 50.0f)/50.0f, 0.f, 1.f);
	}
	else
	{
		return 1.f;
	}
}

void AOLEnemyPawn::PlayAnimForSpecialMove(ESpecialMoveType moveType)
{
	switch (moveType)
	{
	case SMT_ClimbUpLedge:
		{
			FLOAT LedgeRatio = CalculateLedgeRatio();

			if (EnemyMode == EM_Chase)
			{
				PlayBlendedAnim(AnimNameClimbUp100Chase, AnimNameClimbUp50Chase, LedgeRatio, 0.1f, 0.15f);
			}
			else
			{
				PlayBlendedAnim(AnimNameClimbUp100, AnimNameClimbUp50, LedgeRatio, 0.2f, 0.25f);
			}
		}
		break;
	case SMT_DropFromLedge:
		{
			FLOAT LedgeRatio = CalculateLedgeRatio();

			if (EnemyMode == EM_Chase)
			{
				PlayBlendedAnim(AnimNameClimbDown100Chase, AnimNameClimbDown50Chase, LedgeRatio, 0.1f, 0.15f);
			}
			else
			{
				PlayBlendedAnim(AnimNameClimbDown100, AnimNameClimbDown50, LedgeRatio, 0.2f, 0.25f);
			}
		}
		break;
	case SMT_EnterDoorInteraction:
		{
			if (Bot->ActiveDoor != NULL)
			{
				if (Bot->bBreachingDoor)
				{
					if (EnemyMode == EM_Chase)
					{
						PlayFullBodyAnim(AnimNameBashDoorChase, 1.0f, 0.25f, 0.25f);
					}
					else
					{
						PlayFullBodyAnim(AnimNameBashDoor, 1.0f, 0.25f, 0.25f);
					}
				}
				else
				{
					UOLAICmd_MoveAbility_Door* DoorCmd = Cast<UOLAICmd_MoveAbility_Door>(Bot->GetActiveCommand());

					if (DoorCmd != NULL)
					{
						if ((EnemyMode == EM_Patrol || Bot->ActiveDoor->bAIAlwaysCloses) && bCloseDoorInPatrol)
						{
							if (DoorCmd->bReversed)
							{
								if (Bot->ActiveDoor->bReverseDirection)
								{
									PlayFullBodyAnim(AnimNameOpenDoorLeftPullWithClose, 1.f, 0.2f, 0.2f);
								}
								else
								{
									PlayFullBodyAnim(AnimNameOpenDoorRightPullWithClose, 1.f, 0.2f, 0.2f);
								}
							}
							else
							{
								if (Bot->ActiveDoor->bReverseDirection)
								{
									PlayFullBodyAnim(AnimNameOpenDoorRightPushWithClose, 1.f, 0.2f, 0.2f);
								}
								else
								{
									PlayFullBodyAnim(AnimNameOpenDoorLeftPushWithClose, 1.f, 0.2f, 0.2f);
								}
							}
						}
						else
						{
							if (DoorCmd->bReversed)
							{
								if (Bot->ActiveDoor->bReverseDirection)
								{
									PlayFullBodyAnim(AnimNameOpenDoorLeftPull, 1.f, 0.2f, 0.2f);
								}
								else
								{
									PlayFullBodyAnim(AnimNameOpenDoorRightPull, 1.f, 0.2f, 0.2f);
								}
							}
							else
							{
								if (Bot->ActiveDoor->bReverseDirection)
								{
									PlayFullBodyAnim(AnimNameOpenDoorRightPush, 1.f, 0.2f, 0.2f);
								}
								else
								{
									PlayFullBodyAnim(AnimNameOpenDoorLeftPush, 1.f, 0.2f, 0.2f);
								}
							}
						}
					}
				}
			}
		}
		break;

	case SMT_AttackNormal:
		{
			if (bUsesDirectionalAttacks)
			{
				switch (AttackSide)
				{
				case EAS_Left:
					PlayFullBodyAnim(AnimNameAttackLeft, 1.f, 0.2, 0.2);
					break;
				case EAS_Right:
					PlayFullBodyAnim(AnimNameAttackRight, 1.f, 0.2, 0.2);
					break;
				case EAS_Middle:
					PlayFullBodyAnim(AnimNameAttackMiddle, 1.f, 0.2, 0.2);
					break;
				}
			}
			else
			{
				PlayFullBodyAnim(AnimNameAttack, 1.f, 0.2, 0.2);
			}
		}
		break;

	case SMT_AttackQuick:
		{
			PlayFullBodyAnim(AnimNameAttackRight,4.0f, 0.2f, 0.2f);
		}
		break;
	// Squeeze Anims
	case SMT_AttackSqueezeStart:
		{
			if (Bot->bAttackRight)
			{
				PlayFullBodyAnim(AnimNameGrabSqueezeRightStart, 1.f, 0.2f, 0.0f);
			}
			else
			{
				PlayFullBodyAnim(AnimNameGrabSqueezeLeftStart, 1.f, 0.2f, 0.0f);
			}
		}
		break;
	case SMT_AttackSqueezeStartToWait:
		{
			if (Bot->bAttackRight)
			{
				PlayFullBodyAnim(AnimNameGrabSqueezeRightStartToWait, 1.f, 0.0f, 0.2f);
			}
			else
			{
				PlayFullBodyAnim(AnimNameGrabSqueezeLeftStartToWait, 1.f, 0.0f, 0.2f);
			}
		}
		break;
	case SMT_AttackSqueezeWaitToFail:
		{
			if (Bot->bAttackRight)
			{
				PlayFullBodyAnim(AnimNameGrabSqueezeRightWaitToFail, 1.f, 0.2f, 0.02f);
			}
			else
			{
				PlayFullBodyAnim(AnimNameGrabSqueezeLeftWaitToFail, 1.f, 0.2f, 0.2f);
			}
		}
		break;
	case SMT_AttackSqueezeWaitToSuccess:
		{
			if (Bot->bAttackRight)
			{
				PlayFullBodyAnim(AnimNameGrabSqueezeRightWaitToSuccess, 1.f, 0.2f, 0.0f);
			}
			else
			{
				PlayFullBodyAnim(AnimNameGrabSqueezeLeftWaitToSuccess, 1.f, 0.2f, 0.0f);
			}
		}
		break;
	case SMT_AttackSqueezeSuccess:
		{
			FLOAT BlendOutTime = 0.3f;

			if (bCanThrow)
			{
				BlendOutTime = 0.0f;
			}

			if (Bot->bAttackRight)
			{
				PlayFullBodyAnim(AnimNameGrabSqueezeRightSuccess, 1.0f, 0.0f, BlendOutTime);
			}
			else
			{
				PlayFullBodyAnim(AnimNameGrabSqueezeLeftSuccess, 1.0f, 0.0f, BlendOutTime);
			}
		}
		break;
	case SMT_AttackBed:
		{
			FLOAT BlendOutTime = 0.3f;

			if (bCanThrow)
			{
				BlendOutTime = -1.0f;
			}

			if (Bot->CurrentBedSide == 0)
			{
				PlayFullBodyAnim(AnimNameGrabBedLeft, 1.f, 0.2f, BlendOutTime);
			}
			else
			{
				PlayFullBodyAnim(AnimNameGrabBedRight, 1.f, 0.2f, BlendOutTime);
			}
		}
		break;
	case SMT_AttackGrab:
		{
			PlayFullBodyAnim(AnimNameGrabNormal, 1.f, 0.2f, -1.0f);
		}
		break;
	case SMT_ThrowHero:
		{
			TWEAKABLE FLOAT BlendOutTime = 0.25f;
			
			if (ThrowRotation >= 0.f && ThrowRotation <= HALF_PI)
			{
				PlayBlendedAnim(AnimNameThrowPlayerRight90, AnimNameThrowPlayer, ThrowRotation/HALF_PI, 0.0f, BlendOutTime);
			}
			else if (ThrowRotation > HALF_PI)
			{
				PlayBlendedAnim(AnimNameThrowPlayerRight180, AnimNameThrowPlayerRight90, (ThrowRotation - HALF_PI)/HALF_PI, 0.0f, BlendOutTime);
			}
			else if (ThrowRotation < 0.f && ThrowRotation >= -HALF_PI)
			{
				PlayBlendedAnim(AnimNameThrowPlayerLeft90, AnimNameThrowPlayer, ThrowRotation/-HALF_PI, 0.0f, BlendOutTime);
			}
			else if (ThrowRotation < -HALF_PI)
			{
				PlayBlendedAnim(AnimNameThrowPlayerLeft180, AnimNameThrowPlayerLeft90, (ThrowRotation + HALF_PI)/-HALF_PI, 0.0f, BlendOutTime);
			}
		}
		break;
	case SMT_AttackGrabUnder:
		{
			FLOAT BlendOutTime = 0.3f;

			if (bCanThrow)
			{
				BlendOutTime = -1.0f;
			}

			PlayFullBodyAnim(AnimNameGrabUnder, 1.f, 0.2f, BlendOutTime);
		}
		break;
	case SMT_AttackLocker:
		{
			if (this->IsA(AOLEnemyNanoCloud::StaticClass()))
			{
				PlayFullBodyAnim(AnimNameFatalityLocker, 1.f, 0.2f, 0.2f);
			}
			else
			{
				FLOAT BlendOutTime = 0.3f;

				if (bCanThrow)
				{
					BlendOutTime = -1.0f;
				}

				PlayFullBodyAnim(AnimNameGrabLocker, 1.f, 0.2f, BlendOutTime);
			}
		}
		break;
	case SMT_AttackCrouch:
		{
			PlayFullBodyAnim(AnimNameGrabCrouch, 1.f, 0.2f, -1.0f);
		}
		break;
	case SMT_KillHero:
		{
			if (this->IsA(AOLEnemyGenericPatient::StaticClass()))
			{
				if (bUsingWeapon)
				{
					if (WeaponType == WeaponType_Blunt)
					{
						if (bBackAnim)
						{
							PlayFullBodyAnim(AnimNameClubFatalityBack, 1.f, 0.2f, 0.2f);
						}
						else
						{
							PlayFullBodyAnim(AnimNameClubFatalityFront, 1.f, 0.2f, 0.2f);
						}
					}
					else // if (WeaponType == WeaponType_Stab)
					{
						if (bBackAnim)
						{
							PlayFullBodyAnim(AnimNameBackstabFatality, 1.f, 0.2f, 0.2f);
						}
						else
						{
							PlayFullBodyAnim(AnimNameStabChopFatality, 1.f, 0.2f, 0.2f);
						}
					}
				}
				else
				{
					PlayFullBodyAnim(AnimNameChokeFatality, 1.f, 0.2f, 0.2f);
				}

			}
			else if (IsA(AOLEnemyNanoCloud::StaticClass()))
			{
				if (bBackAnim)
				{
					PlayFullBodyAnim(AnimNameFatalityXplodeBack, 1.f, 0.2f, 0.2f);
				}
				else
				{
					PlayFullBodyAnim(AnimNameFatalityXplode, 1.f, 0.2f, 0.2f);
				}
			}
			else
			{
				PlayFullBodyAnim(AnimNameGrabFatality, 1.f, 0.1f, 0.2f);
			}
		}
		break;
	case SMT_InvestigateLocker:
		{
			PlayFullBodyAnim(AnimNameSearchLocker, 1.f, 0.2f, 0.3f);
		}
		break;
	case SMT_InvestigateBed:
		{
			UBOOL bUseAlternate = FALSE;
			if (appFrand() < InvestigateBedAlternateChance)
			{
				bUseAlternate = TRUE;
			}

			if (Bot->CurrentBedSide == 0)
			{
				PlayFullBodyAnim(bUseAlternate ? AnimNameSearchBedLeftAlt : AnimNameSearchBedLeft, 1.f, 0.25f, 0.25f);
			}
			else
			{
				PlayFullBodyAnim(bUseAlternate ? AnimNameSearchBedRightAlt : AnimNameSearchBedRight, 1.f, 0.25f, 0.25f);
			}
		}
		break;
	case SMT_Bash:
		{
			if (Bot->ActiveBashable != NULL)
			{
				switch(Bot->ActiveBashable->BashableType)
				{
				case EOLBT_Wall:
					PlayFullBodyAnim(AnimNameBashWallChase, 1.f, 0.2f, 0.2f);
					break;
				case EOLBT_Table:
					PlayFullBodyAnim(AnimNameBashTableChase, 0.9f, 0.2f, 0.3f);
					break;
				}
			}
		}
		break;
	case SMT_TurnOnSpot:
		{
			FLOAT StartRatio = 0.f;

			//if (OldTurnAmount != 0.f)
			//{
			//	StartRatio = OldTurnAmount / TurnAmount; 
			//}

			if (EnemyMode == EM_Chase)
			{
				TWEAKABLE FLOAT ChaseBlendOutTime = 0.1f;

				if (TurnAmount >= 0.f && TurnAmount <= HALF_PI)
				{
					PlayBlendedAnim(AnimNameTurnOnSpotRight90Chase, AnimNameIdlePoseChase, TurnAmount/HALF_PI, 0.1f, ChaseBlendOutTime, 1.0f, StartRatio);
				}
				else if (TurnAmount > HALF_PI)
				{
					PlayBlendedAnim(AnimNameTurnOnSpotRight180Chase, AnimNameTurnOnSpotRight90Chase, (TurnAmount - HALF_PI)/HALF_PI, 0.1f, ChaseBlendOutTime, 1.0f, StartRatio);
				}
				else if (TurnAmount < 0.f && TurnAmount >= -HALF_PI)
				{
					PlayBlendedAnim(AnimNameTurnOnSpotLeft90Chase, AnimNameIdlePoseChase, TurnAmount/-HALF_PI, 0.1f, ChaseBlendOutTime, 1.0f, StartRatio);
				}
				else if (TurnAmount < -HALF_PI)
				{
					PlayBlendedAnim(AnimNameTurnOnSpotLeft180Chase, AnimNameTurnOnSpotLeft90Chase, (TurnAmount + HALF_PI)/-HALF_PI, 0.1f, ChaseBlendOutTime, 1.0f, StartRatio);
				}
			}
			else
			{
				TWEAKABLE FLOAT BlendOutTime = 0.1f;

				if (TurnAmount >= 0.f && TurnAmount <= HALF_PI)
				{
					PlayBlendedAnim(AnimNameTurnOnSpotRight90, AnimNameIdlePose, TurnAmount/HALF_PI, 0.1f, BlendOutTime, 1.0f, StartRatio);
				}
				else if (TurnAmount > HALF_PI)
				{
					PlayBlendedAnim(AnimNameTurnOnSpotRight180, AnimNameTurnOnSpotRight90, (TurnAmount - HALF_PI)/HALF_PI, 0.1f, BlendOutTime, 1.0f, StartRatio);
				}
				else if (TurnAmount < 0.f && TurnAmount >= -HALF_PI)
				{
					PlayBlendedAnim(AnimNameTurnOnSpotLeft90, AnimNameIdlePose, TurnAmount/-HALF_PI, 0.1f, BlendOutTime, 1.0f, StartRatio);
				}
				else if (TurnAmount < -HALF_PI)
				{
					PlayBlendedAnim(AnimNameTurnOnSpotLeft180, AnimNameTurnOnSpotLeft90, (TurnAmount + HALF_PI)/-HALF_PI, 0.1f, BlendOutTime, 1.0f, StartRatio);
				}
			}
		}
		break;
	case SMT_AttackPush:
		{
			FVector ToTarget = (Bot->TargetPlayer->Location - Location).SafeNormal2D();

			FLOAT AngleToTarget = UNR_TO_DEG * FRotator::NormalizeAxis(Rotation.Yaw - ToTarget.Rotation().Yaw);
			FLOAT BlendAlpha = Clamp( Abs(AngleToTarget / 90.0f), 0.0f, 1.0f );

			if (AngleToTarget > 0.0f)
			{
				PlayTopHalfBlendedAnim(AnimNamePushLeft, AnimNamePushForward, BlendAlpha, 0.2f, 0.2f);
			}
			else
			{
				PlayTopHalfBlendedAnim(AnimNamePushRight, AnimNamePushForward, BlendAlpha, 0.2f, 0.2f);
			}
		}
		break;
	case SMT_AIVault:
		{
			TWEAKABLE FLOAT BlendOutTime = 0.5f;
			PlayFullBodyAnim(AnimNameVault, 1.f, 0.2f, BlendOutTime);
		}
		break;

	case SMT_BashDoorStart:
		{
			PlayFullBodyAnim(AnimNameBashDoorStart, 1.f, 0.25f, 0.0f);
		}
		break;

	case SMT_BashDoorLoop:
		{
			PlayFullBodyAnim(AnimNameBashDoorLoop, 1.f, 0.0f, 0.0f);
		}
		break;

	case SMT_BashDoorFinish:
		{
			if (bUsingWeapon && !bHasWeaponEquipped)
			{
				PlayFullBodyAnim(AnimNameBashDoorEndEquip, 1.f, 0.0f, 0.25f);
			}
			else
			{
				PlayFullBodyAnim(AnimNameBashDoorEnd, 1.f, 0.0f, 0.25f);
			}
		}
		break;

	case SMT_BashDoorFailed:
		{
			PlayFullBodyAnim(AnimNameBashDoorFailed, 1.f, 0.0f, 0.2f);
			//PlayFullBodyAnim(AnimNameIdle, 1.f, 0.2f, 0.2f);
		}
		break;

	case SMT_NanoThroughDoor:
		{
			PlayFullBodyAnim(AnimNameNanoDoor, 1.f, 0.2f, 0.2f);
		}
		break;

	case SMT_Avoiding:
		{
			if (bLeftAnim)
			{
				PlayFullBodyAnim(AnimNameAvoidLeft, SpecialMoveRate, 0.2f, 0.2f);
			}
			else
			{
				PlayFullBodyAnim(AnimNameAvoidRight, SpecialMoveRate, 0.2f, 0.2f);
			}
		}
		break;

	case SMT_Knockedback:
		{
			if (bLeftAnim)
			{
				PlayFullBodyAnim(AnimNameKnockbackLeft, 1.0f, 0.2f, 0.2f);
			}
			else
			{
				PlayFullBodyAnim(AnimNameKnockbackRight, 1.0f, 0.2f, 0.2f);
			}
		}
		break;
	case SMT_Disturbed:
		{
			FName animNameA = NAME_None;
			FName animNameB = NAME_None;

			if (bLeftAnim)
			{
				animNameA = (SpecialMoveBlendAlpha < 0.5f) ? AnimNameDisturbedFrontLeft : AnimNameDisturbedLeft180;
				animNameB = AnimNameDisturbedLeft90;
			}
			else
			{
				animNameA = (SpecialMoveBlendAlpha < 0.5f) ? AnimNameDisturbedFrontRight : AnimNameDisturbedRight180;
				animNameB = AnimNameDisturbedRight90;
			}

			FLOAT alpha = (SpecialMoveBlendAlpha < 0.5f) ? 2.0f*(0.5f - SpecialMoveBlendAlpha) : 2.0f*(SpecialMoveBlendAlpha - 0.5f);
			PlayBlendedAnim(animNameA, animNameB, alpha, 0.2f, 0.2f);
		}
		break;
	}
}

void AOLEnemyPawn::TurnOnSpot(FRotator EndRotation)
{
	// Clear any rotation before starting attack.
	ResetDesiredRotation();
	Bot->SetFocalPoint(FVector(0,0,0));

	TurnAmount = FRotator::NormalizeAxis(EndRotation.Yaw - Rotation.Yaw)* UNR_TO_RAD;

	if (Abs(TurnAmount) > KINDA_SMALL_NUMBERF)
	{
		if (IsSpecialMoveCompleted() || SpecialMove != SMT_TurnOnSpot)
		{
			if (TurnAmount < -HALF_PI/2 || TurnAmount > HALF_PI/2)
			{
				StartSpecialMove(SMT_TurnOnSpot);
			}
		}

		if (CustomBlendNode->TimeRemaining > CustomBlendNode->BlendOutTime)
		{
			ClearProceduralAnims();

			FProceduralAnimData animData;

			animData.HeadingDelta = TurnAmount * RAD_TO_DEG;

			debugf(TEXT("Turn On Spot - Procedural Anim: Heading - %f, Time - %f"), animData.HeadingDelta, CustomBlendNode->TimeRemaining);

			QueueProceduralAnim(animData, 0.f, Abs(animData.HeadingDelta) / (CustomBlendNode->TimeRemaining - CustomBlendNode->BlendOutTime));
		}
	}
}

void AOLEnemyPawn::StartAvoid(AOLEnemyPawn* OtherPawn)
{
	if (Bot == NULL || Bot->IsPerformingMoveAbility() || Bot->IsInState(Bot->InterruptionState) || Bot->bAttacking || Bot->bInvestigatingObject)
	{
		return;
	}

	FVector toOther = (OtherPawn->Location - Location).SafeNormal2D();
	FLOAT angleToOther = UNR_TO_DEG * FRotator::NormalizeAxis(Rotation.Yaw - toOther.Rotation().Yaw);

	bLeftAnim = angleToOther < 0.f;

	switch(OtherPawn->EnemyMode)
	{
	case EM_Investigate:
		SpecialMoveRate = AvoidRateInvestigate;
		break;
	case EM_Chase:
		SpecialMoveRate = AvoidRateChase;
		break;
	case EM_Patrol:
	default:
		SpecialMoveRate = AvoidRatePatrol;
		break;
	}

	Bot->bAvoiding = TRUE;

	EGotoState ResultState = GOTOSTATE_Success;
	ResultState = Bot->GotoState(FName(TEXT("Avoiding"), FNAME_Find), TRUE);
	if ( ResultState == GOTOSTATE_Success )
	{
		Bot->GotoLabel( FName(NAME_Begin) );
	}
}

UBOOL AOLEnemyPawn::CanStrafe()
{
	if (AnimNodeSelectEnemyMode)
	{
		UOLAnimLocomotion* locoNode = Cast<UOLAnimLocomotion>(AnimNodeSelectEnemyMode->Children(AnimNodeSelectEnemyMode->ActiveChildIndex).Anim);
		if (locoNode && locoNode->IsTurning())
		{
			return FALSE;
		}
	}

	return TRUE;
}

UBOOL AOLEnemyPawn::CanStartAndStop()
{
	return RotationMode == RM_FaceVelocity;
}

void AOLEnemyPawn::MoveOutOfTheWay()
{
	if (!bCanBeKnockedback || Modifiers.bDisableKnockbackFromPlayer)
	{
		return;
	}

	UGameAICommand* MoveCmd = Bot->FindCommandOfClass(UOLAICmd_MoveAbility::StaticClass());
	if (MoveCmd != NULL)
	{
		if (MoveCmd->GetStateFrame()->StateNode->GetFName() == FName(TEXT("Approaching"), FNAME_Find) || MoveCmd->GetStateFrame()->StateNode->GetFName() == FName(TEXT("Waiting"), FNAME_Find))
		{
			Bot->AbortCommand(MoveCmd);
		}
		else
		{
			return;
		}
	}

	TWEAKABLE FLOAT MinDistForPush = 80.0f;
	TWEAKABLE FLOAT KnockbackDist = 80.0f;

	FVector moveDir(0.0f);
	UBOOL bMoveOut = FALSE;

	for (APawn* pawn = GWorld->GetWorldInfo()->PawnList; pawn != NULL; pawn = pawn->NextPawn)
	{
		AOLEnemyPawn* otherPawn = Cast<AOLEnemyPawn>(pawn);
		if (otherPawn && otherPawn != this && Abs(otherPawn->Location.Z - Location.Z) < 50.0f)
		{	
			FVector otherToMe = (Location - otherPawn->Location);
			if (otherToMe.SizeSquared2D() < Square(MinDistForPush))
			{
				moveDir += otherToMe.SafeNormal2D();
				bMoveOut = TRUE;

				debugf(TEXT("%s moving out from %s"), *GetName(), *otherPawn->GetName());
			}
		}
	}

	if (bMoveOut)
	{
		moveDir = moveDir.SafeNormal2D();

		bLeftAnim = (moveDir | Rotation.Right()) > 0.0f;

		// Create procedural path for the anim
		ClearProceduralAnims();

		FProceduralAnimData animData;
		animData.PositionDelta = moveDir * KnockbackDist;

		QueueProceduralAnim(animData);

		EGotoState ResultState = GOTOSTATE_Success;
		ResultState = Bot->GotoState(FName(TEXT("Knockback"), FNAME_Find), TRUE);
		if ( ResultState == GOTOSTATE_Success )
		{
			Bot->GotoLabel( FName(NAME_Begin) );
		}
	}
}

UBOOL AOLEnemyPawn::CanBeKnockedback(UBOOL bForced /*= FALSE*/)
{
	UBOOL bReturn = FALSE;

	if (Bot && !Bot->IsInState(Bot->InterruptionState) && !Bot->IsPerformingMoveAbility() && !Bot->bInvestigatingObject && (bForced || (!Bot->bAttacking && !Modifiers.bDisableKnockbackFromPlayer && bCanBeKnockedback)) )
	{
		bReturn = TRUE;
	}

	return bReturn;
}

void AOLEnemyPawn::StartKnockback(AOLHero* Hero, FVector HitNormal)
{
	if (Hero == NULL || HitNormal.IsNearlyZero())
	{
		return;
	}

	TWEAKABLE FLOAT KnockbackDistancePartOne = 30.0f;
	TWEAKABLE FLOAT KnockbackDistancePartTwo = 60.0f;

	// Decide whether to use left or right anim
	bLeftAnim = (HitNormal | Rotation.Right()) > 0.0f;
	
	// Create procedural path for the anim
	ClearProceduralAnims();

	FProceduralAnimData animData;
	animData.PositionDelta = HitNormal * KnockbackDistancePartOne;

	QueueProceduralAnim(animData);

	FRotator partTwoDirection(0.f, Hero->Rotation.Yaw - (90.0f * DEG_TO_UNR), 0.f);
	FVector partTwoVector = partTwoDirection.Vector().SafeNormal2D();

	if ((partTwoVector | (Location - Hero->Location).SafeNormal2D()) > 0.f)
	{
		animData.PositionDelta = partTwoVector * KnockbackDistancePartTwo;
	}
	else
	{
		animData.PositionDelta = -1.f * partTwoVector * KnockbackDistancePartTwo;
	}

	QueueProceduralAnim(animData);
	
	EGotoState ResultState = GOTOSTATE_Success;
	ResultState = Bot->GotoState(FName(TEXT("Knockback"), FNAME_Find), TRUE);
	if ( ResultState == GOTOSTATE_Success )
	{
		Bot->GotoLabel( FName(NAME_Begin) );
	}
}

void AOLEnemyPawn::StartDoorKnockback(FVector Direction, UBOOL bLocker)
{
	if (Direction.IsNearlyZero())
	{
		return;
	}

	TWEAKABLE FLOAT KnockbackDistance = 100.0f;
	TWEAKABLE FLOAT KnockbackDistanceLocker = 60.0f;

	// Create procedural path for the anim

	bLeftAnim = (Direction | Rotation.Right()) > 0.0f;

	ClearProceduralAnims();

	FProceduralAnimData animData;
	animData.PositionDelta = Direction * (bLocker ? KnockbackDistanceLocker : KnockbackDistance);

	QueueProceduralAnim(animData);

	EGotoState ResultState = GOTOSTATE_Success;
	ResultState = Bot->GotoState(FName(TEXT("Knockback"), FNAME_Find), TRUE);
	if ( ResultState == GOTOSTATE_Success )
	{
		Bot->GotoLabel( FName(NAME_Begin) );
	}
}

void AOLEnemyPawn::StartNormalAttack()
{	
	if(bUsesDirectionalAttacks)
	{
		UBOOL bCanUseLeft = TRUE;
		UBOOL bCanUseRight = TRUE;

		INT numAttackSides = 1;

		TWEAKABLE FLOAT ExtentZReduction = 20.0f;
		TWEAKABLE FLOAT SideCheckDistance = 30.0f;

		FVector Extent = GetCylinderExtent();
		Extent.Z -= ExtentZReduction;

		FCheckResult Hit;

		FVector CheckLocation = Location + CylinderComponent->Translation + Rotation.Vector().RotateAngleAxis(90.0f * DEG_TO_UNR, VecZ(1.0f)) * SideCheckDistance;
		if (GWorld->EncroachingWorldGeometry(Hit, CheckLocation, Extent, FALSE, this))
		{
			bCanUseRight = FALSE;
		}

		CheckLocation = Location + CylinderComponent->Translation + Rotation.Vector().RotateAngleAxis(-90.0f * DEG_TO_UNR, VecZ(1.0f)) * SideCheckDistance;
		if (GWorld->EncroachingWorldGeometry(Hit, CheckLocation, Extent, FALSE, this))
		{
			bCanUseLeft = FALSE;
		}

		if (bCanUseLeft) { ++numAttackSides; }
		if (bCanUseRight) { ++numAttackSides; }

		INT attackToUse = RandHelper(numAttackSides);

		if (!bCanUseLeft && attackToUse == EAS_Left) { ++attackToUse; }
		if (!bCanUseRight && attackToUse == EAS_Right) { ++attackToUse; }

		check(attackToUse < EAS_MAX);

		AttackSide = attackToUse;

		// We don't have good middle animations for non weapon characters.
		if (AttackSide == EAS_Middle && !bHasWeaponEquipped)
		{
			AttackSide = EAS_Right;
		}
	}
	
	StartSpecialMove(SMT_AttackNormal);
}

void AOLEnemyPawn::StartDisturbed()
{
	FVector DisturbPoint = FVector(0.f);
	if (Bot->AudioDisturbance.TimeSinceUpdate >= 0.f 
		&& (Bot->VisualDisturbance.TimeSinceUpdate == -1.f || Bot->AudioDisturbance.TimeSinceUpdate < Bot->VisualDisturbance.TimeSinceUpdate))
	{
		DisturbPoint = Bot->AudioDisturbance.Location;
	}
	else if (Bot->VisualDisturbance.TimeSinceUpdate >= 0.f)
	{
		DisturbPoint = Bot->VisualDisturbance.Location;
	}
	else
	{
		return;
	}

	FVector ToDisturbance = (DisturbPoint - Location).SafeNormal2D();

	FLOAT ToDisturbanceAngle = UNR_TO_DEG * FRotator::NormalizeAxis(Rotation.Yaw - ToDisturbance.Rotation().Yaw);
	SpecialMoveBlendAlpha = Saturate( Abs(ToDisturbanceAngle / 180.0f));
	bLeftAnim = ToDisturbanceAngle > 0.0f;

	StartSpecialMove(SMT_Disturbed);
}


void AOLEnemyPawn::EnableRootMotion()
{
	SetRootMotionMode(RMM_Accel);

	bAnimRootMotionActive = TRUE;
}

void AOLEnemyPawn::DisableRootMotion()
{
	bAnimRootMotionActive = FALSE;

	SetRootMotionMode(RMM_Ignore);
}

void AOLEnemyPawn::EnableNightVisionEffect()
{
	for (INT i = 0; i < NanoSwarmEmitters.Num(); i++)
	{
		UParticleSystemComponent* psc = NanoSwarmEmitters(i).Emitter;
		psc->DetachFromAny();
		Mesh->AttachComponent(psc, NanoSwarmEmitters(i).BoneName);
	}

	// Hide the shadow
	if (!bCastShadowInNV && Mesh->CastShadow)
	{
		FComponentReattachContext ReattachContextMesh(Mesh);
		Mesh->CastShadow = FALSE;
	}
}

void AOLEnemyPawn::DisableNightVisionEffect()
{
	for (INT i = 0; i < NanoSwarmEmitters.Num(); i++)
	{
		UParticleSystemComponent* psc = NanoSwarmEmitters(i).Emitter;
		psc->DetachFromAny();
	}

	// Get the shadow back to its original state
	AOLEnemyPawn* defaultEnemyPawn = Cast<AOLEnemyPawn>(GetClass()->GetDefaultObject());
	if (Mesh->CastShadow != defaultEnemyPawn->Mesh->CastShadow)
	{
		FComponentReattachContext ReattachContextMesh(Mesh);
		Mesh->CastShadow = defaultEnemyPawn->Mesh->CastShadow;
	}
}

UBOOL AOLEnemyPawn::moveToward(const FVector &Dest, AActor *GoalActor )
{
	if ( !Controller )
		return FALSE;

	if ( Controller->bAdjusting )
	{
		GoalActor = NULL;
	}
	FVector Direction = Dest - Location;
	FLOAT ZDiff = Direction.Z;

	if( Physics == PHYS_Walking )
	{
		Direction.Z = 0.f;
	}
	else if (Physics == PHYS_Falling)
	{
		// use air control if low grav or above destination and falling towards it
		if (Velocity.Z < 0.f && (ZDiff < 0.f || GetGravityZ() > 0.9f * GWorld->GetDefaultGravityZ()))
		{
			if ( ZDiff > 0.f )
			{
				if ( ZDiff > 2.f * MaxJumpHeight )
				{
					Controller->FailMove();
					Controller->eventNotifyMissedJump();
				}
			}
			else
			{
				if ( (Velocity.X == 0.f) && (Velocity.Y == 0.f) )
					Acceleration = FVector(0.f,0.f,0.f);
				else
				{
					FLOAT Dist2D = Direction.Size2D();
					Direction.Z = 0.f;
					Acceleration = Direction;
					Acceleration = Acceleration.SafeNormal();
					Acceleration *= AccelRate;
					if ( (Dist2D < 0.5f * Abs(Direction.Z)) && ((Velocity | Direction) > 0.5f*Dist2D*Dist2D) )
						Acceleration *= -1.f;

					if ( Dist2D < 1.5f*CylinderComponent->CollisionRadius )
					{
						Velocity.X = 0.f;
						Velocity.Y = 0.f;
						Acceleration = FVector(0.f,0.f,0.f);
					}
					else if ( (Velocity | Direction) < 0.f )
					{
						FLOAT M = ::Max(0.f, 0.2f - AvgPhysicsTime);
						Velocity.X *= M;
						Velocity.Y *= M;
					}
				}
			}
		}
		return FALSE; // don't end move until have landed
	}
	
	if ( Controller->MoveTarget && Controller->MoveTarget->IsA(APickupFactory::StaticClass()) 
		&& (Abs(Location.Z - Controller->MoveTarget->Location.Z) < CylinderComponent->CollisionHeight)
		&& (Square(Location.X - Controller->MoveTarget->Location.X) + Square(Location.Y - Controller->MoveTarget->Location.Y) < Square(CylinderComponent->CollisionRadius)) )
	{
		Controller->MoveTarget->eventTouch(this, this->CollisionComponent, Location, (Controller->MoveTarget->Location - Location) );
	}

	FLOAT Distance = Direction.Size();
	const UBOOL bGlider = IsGlider();
	FCheckResult Hit(1.f);

	if ( ReachedDestination(Location, Dest, GoalActor, TRUE) )
	{
		if ( !bGlider )
		{
			Acceleration = FVector(0.f,0.f,0.f);
		}

		// if Pawn just reached a navigation point, set a new anchor
		ANavigationPoint *Nav = Cast<ANavigationPoint>(GoalActor);
		if ( Nav )
			SetAnchor(Nav);
		return TRUE;
	}
	//else 
	// if walking, and within radius, and goal is null or
	// the vertical distance is greater than collision + step height and trace hit something to our destination
	//if (Physics == PHYS_Walking 
	//	&& Distance < (CylinderComponent->CollisionRadius+DestinationOffset) &&
	//	(GoalActor == NULL ||
	//	(ZDiff > CylinderComponent->CollisionHeight + 2.f * MaxStepHeight && 
	//	!GWorld->SingleLineCheck(Hit, this, Dest, Location, TRACE_World))))
	//{
	//	Controller->eventMoveUnreachable(Dest,GoalActor);
	//	return TRUE;
	//}
	else if ( bGlider )
	{
		Direction = Rotation.Vector();
	}
	else if ( Distance > 0.f )
	{
		Direction = Direction/Distance;
	}

	Acceleration = Direction * AccelRate;

	if ( !Controller->bAdjusting && Controller->MoveTarget && Controller->MoveTarget->GetAPawn() )
	{
		return (Distance < CylinderComponent->CollisionRadius + Controller->MoveTarget->GetAPawn()->CylinderComponent->CollisionRadius + 0.8f * MeleeRange);
	}

	FLOAT speed = Velocity.Size();

	if ( !bGlider && (speed > FASTWALKSPEED) )
	{
		//		FVector VelDir = Velocity/speed;
		//		Acceleration -= 0.2f * (1 - (Direction | VelDir)) * speed * (VelDir - Direction);
	}
	if ( Distance < 1.4f * AvgPhysicsTime * speed )
	{
		// slow pawn as it nears its destination to prevent overshooting
		if ( !bReducedSpeed ) 
		{
			//haven't reduced speed yet
			DesiredSpeed = 0.51f * DesiredSpeed;
			bReducedSpeed = 1;
		}
		if ( speed > 0.f )
			DesiredSpeed = Min(DesiredSpeed, (2.f*FASTWALKSPEED)/speed);
		if ( bGlider )
			return TRUE;
	}
	return FALSE;
}

UBOOL AOLEnemyPawn::moveAlongPath(const TArray<FVector>& PathPoints)
{
	if (PathPoints.Num() == 0)
	{
		return FALSE;
	}

	CurrentMovePathStart = Location;
	CurrentMovePath = PathPoints;
	CurrentMovePathIdx = 0;
	LastMovePathPoint = Location;

	// Add additional points to CurrentMovePath to prevent large paths
	
	if (CurrentMovePath.Num() > 1)
	{
		TWEAKABLE FLOAT MinPathLength = 50.0f;

		FVector LastPoint;
		FVector CurrentPoint;
		FVector NextPoint;

		check(!CurrentMovePath.Last().ContainsNaN() && !CurrentMovePath.Last().IsNearlyZero() && CurrentMovePath.Last().GetAbsMax() < 1000000.0f);

		UBOOL bPathChanged = TRUE;
		while (bPathChanged)
		{
			bPathChanged = FALSE;

			LastPoint = Location;
			CurrentPoint = CurrentMovePath(0);
			NextPoint = CurrentMovePath(1);

			INT Idx = 0;
			while (Idx < CurrentMovePath.Num())
			{
				FVector LastToCurr = CurrentPoint - LastPoint;
				FLOAT LastToCurrDist = Max(LastToCurr.Size(), MinPathLength);

				FVector CurrToNext = NextPoint - CurrentPoint;
				FLOAT CurrToNextDist = Max(CurrToNext.Size(), MinPathLength);

				if (LastToCurrDist * MaxPathSegmentRatio < CurrToNextDist)
				{
					FLOAT NewLength = Min(LastToCurrDist * MaxPathSegmentRatio, CurrToNextDist * 0.5f);

					CurrentMovePath.InsertZeroed(Idx+1);
					FVector newPointPos = CurrentPoint + CurrToNext.SafeNormal() * NewLength;
					TryPullIntermediateDestOnPreferredPath(newPointPos, CurrentPoint, NextPoint);
					CurrentMovePath(Idx+1) = newPointPos;

					bPathChanged = TRUE;
					break;
				}
				else if (CurrToNextDist * MaxPathSegmentRatio < LastToCurrDist)
				{
					FLOAT NewLength = Min(CurrToNextDist * MaxPathSegmentRatio, LastToCurrDist * 0.5f);

					CurrentMovePath.InsertZeroed(Idx);
					FVector newPointPos = CurrentPoint - LastToCurr.SafeNormal() * NewLength;
					TryPullIntermediateDestOnPreferredPath(newPointPos, LastPoint, CurrentPoint);
					CurrentMovePath(Idx) = newPointPos;

					bPathChanged = TRUE;
					break;
				}

				++Idx;

				if (Idx+1 < CurrentMovePath.Num())
				{
					LastPoint = CurrentPoint;
					CurrentPoint = NextPoint;
					NextPoint = CurrentMovePath(Idx+1);
				}
			}
		}
	}

	// Fill out Catmull Rom Path
	CRPathSegments.Reset();
	CRPathSegments.AddItem(FVector2D(Location));
	CRPathSegments.AddItem(FVector2D(Location));

	for (INT I = 0; I < CurrentMovePath.Num(); ++I)
	{
		CRPathSegments.AddItem(FVector2D(CurrentMovePath(I)));
	}

	CRPathSegments.AddItem(FVector2D(CurrentMovePath.Last()));

	CalculateCRSubSegments(0, CRPathNumSubSegments, CRPathSubSegments);

	setMoveTimer((CurrentMovePath.Last() - Location).SafeNormal() * EstimateLengthOfPathLeft());

	return moveToward(CurrentMovePath.Last(), NULL);
}

UBOOL AOLEnemyPawn::TryPullIntermediateDestOnPreferredPath(FVector& newPoint, const FVector& prevPoint, const FVector& nextPoint)
{
	TWEAKABLE FLOAT MinDistToEdgePoint = 200.0f;
	TWEAKABLE FLOAT MaxDeltaZ = 30.0f;
	TWEAKABLE FLOAT MaxDistToPath = 200.0f; 
	TWEAKABLE FLOAT MaxDeviationAngle = 15.0f;
	
	AOLHero* hero = Utils::GetHero(); // we piggyback on its cached paths list

	if (!hero || !bUsePreferredPaths)
	{
		return FALSE;
	}

	FVector fromPrev = newPoint - prevPoint;
	FVector toNext = nextPoint - newPoint;
	FLOAT distFromPrev = 0.0f;
	FLOAT distToNext = 0.0f;
	FVector dirFromPrev(0.0f);
	FVector dirToNext(0.0f);
	fromPrev.ToDirectionAndLength(dirFromPrev, distFromPrev);
	toNext.ToDirectionAndLength(dirToNext, distToNext);

	// check that the intermediate point is some distance from the ends
	if (distFromPrev < MinDistToEdgePoint || distToNext < MinDistToEdgePoint)
	{
		return FALSE;
	}

	// Check that the intermediate point lies on a straight line between the other two
	if (Abs(dirFromPrev | dirToNext) < 0.97f) // 15 degs
	{
		return FALSE;
	}

	for (INT i = 0; i < hero->CachedPreferredPathMarkers.Num(); i++)
	{
		AOLPreferredPathMarker* node1 = hero->CachedPreferredPathMarkers(i);

		if (node1 && node1->IsValid() && node1->Next && node1->Next->IsValid())
		{
			AOLPreferredPathMarker* node2 = node1->Next;

			const FVector& node1Loc = node1->Location;
			const FVector& node2Loc = node2->Location;

			// Height check - are we on the same floor?
			if ( (Max(node1Loc.Z, node2Loc.Z) < (newPoint.Z - MaxDeltaZ)) || (Min(node1Loc.Z, node2Loc.Z) > (newPoint.Z + MaxDeltaZ)))
			{
				continue;
			}

			const FVector& path = node2Loc - node1Loc;
			FVector pathDir = path.SafeNormal();

			FVector closestPoint(0.0f);
			FLOAT distToPath = PointDistToSegment(newPoint, node1Loc, node2Loc, closestPoint);

			// Distance to path - are we even close?
			if (distToPath > MaxDistToPath)
			{
				continue;
			}

			// Are we within the path?
			if (!Utils::IsBetweenMarkers(newPoint, node1Loc, node2Loc, 0.0f))
			{
				continue;
			}

			// We're good - we can pull the point

			FLOAT tanMaxDeviation = appTan(DEG_TO_RAD * MaxDeviationAngle);
			FLOAT maxPullDistanceForPrev = distFromPrev * tanMaxDeviation;
			FLOAT maxPullDistanceForNext = distToNext * tanMaxDeviation;

			FLOAT pullDistance = Min(distToPath, Min(maxPullDistanceForPrev, maxPullDistanceForNext));
			FVector pullDir = (closestPoint - newPoint).SafeNormal2D();

			newPoint = newPoint + pullDistance * pullDir;

			return TRUE;
		}
	}

	return FALSE;
}

void AOLEnemyPawn::CalculateCRSubSegments(INT Idx, INT NumSubSegments, TArray<FVector2D>& SubSegments)
{
	SubSegments.Reset();
	CRPathLastIndex = 0;

	for (INT i = 0; i <= NumSubSegments; ++i)
	{
		SubSegments.AddItem(Bot->OLNavHandle->CatmullRomPointOnCurve(CRPathSegments(Idx), CRPathSegments(Idx+1), CRPathSegments(Idx+2), CRPathSegments(Idx+3), i * (1.f/NumSubSegments)));
	}
}

FLOAT AOLEnemyPawn::EstimateLengthOfPathLeft() const
{
	FLOAT Length = 0.f;

	for (INT i = 0; i < CRPathSubSegments.Num()-1; ++i)
	{
		Length += (CRPathSubSegments(i) - CRPathSubSegments(i+1)).Size();
	}

	for (INT i = CurrentMovePathIdx; i < CurrentMovePath.Num()-1; ++i)
	{
		Length += (CurrentMovePath(i) - CurrentMovePath(i+1)).Size2D();
	}

	return Length;
}

void AOLEnemyPawn::UpdateMovePath(FLOAT deltaTime)
{
	// Check to see if we're between the first and second path point.
	if (CurrentMovePathIdx < CurrentMovePath.Num() - 1)
	{
		UBOOL bPopFront = FALSE;

		FVector ZeroToOne = CurrentMovePath(CurrentMovePathIdx) - LastMovePathPoint;
		FVector OneToTwo = CurrentMovePath(CurrentMovePathIdx+1) - CurrentMovePath(CurrentMovePathIdx);
		FVector MeToOne = Location - CurrentMovePath(CurrentMovePathIdx);

		if (MeToOne.Size2D() <= 10.0f && Abs(MeToOne.Z) < CylinderComponent->CollisionHeight)
		{
			bPopFront = TRUE;
		}
		else if ((ZeroToOne | MeToOne) < 0.f)
		{
			FCheckResult Hit(1.f);
			FVector Origin = Location + FVector(0.f, 0.f, CylinderComponent->CollisionHeight);
			FVector Destination = CurrentMovePath(CurrentMovePathIdx+1) + FVector(0.f, 0.f, CylinderComponent->CollisionHeight);
			FVector Extent = FVector(10.0f, 10.0f, CylinderComponent->CollisionHeight);
			if (UNavigationHandle::StaticObstacleLineCheck(Bot, Hit, Origin, Destination, Extent, TRUE))
			{
				bPopFront = TRUE;
			}
		}

		if (bPopFront)
		{
			LastMovePathPoint = CurrentMovePath(CurrentMovePathIdx);

			++CurrentMovePathIdx;

			CalculateCRSubSegments(CurrentMovePathIdx, CRPathNumSubSegments, CRPathSubSegments);

			Bot->SetFocalPoint( CurrentMovePath(CurrentMovePathIdx), FALSE );
			setMoveTimer((CurrentMovePath.Last() - Location).SafeNormal() * EstimateLengthOfPathLeft());
		}
	}
}

FVector AOLEnemyPawn::GetAIRepulsion(const class AOLEnemyPawn* Agent)
{
	if (!Modifiers.bDisableRepulsion)
	{
		return Super::GetAIRepulsion(Agent);
	}

	return FVector(0.f);
}

//
// Locomotion
//

void AOLEnemyPawn::UpdateLocomotion(FLOAT deltaTime)
{
	UpdateGroundSpeed(deltaTime);
	UpdateTargetVelocity(deltaTime);
	UpdateRotation(deltaTime);
	ApplyTurning(deltaTime);
	ApplyPreferredPaths(deltaTime);

	DesiredRotation = FRotator(0, DEG_TO_UNR * TargetYaw, 0);
}

void AOLEnemyPawn::UpdateGroundSpeed(FLOAT deltaTime)
{
	if (MoveSpeedMode == EMSM_Default)
	{
		FSpeedValues* Speeds = NULL;

		switch (Bot->CurrentEnvironment)
		{
		case AIE_Darkness:
			Speeds = &DarknessSpeedValues;
			break;
		case AIE_Electricity:
			Speeds = &ElectricitySpeedValues;
			break;
		case AIE_Normal:
		default:
			Speeds = &NormalSpeedValues;
			break;
		}

		switch(EnemyMode)
		{
		case EM_Investigate:
			GroundSpeed = Speeds->InvestigateSpeed;
			break;
		case EM_Chase:
			GroundSpeed = Speeds->ChaseSpeed;
			break;
		case EM_Patrol:
		default:
			GroundSpeed = Speeds->PatrolSpeed;
			break;
		}	

		MoveSpeed_Target = GroundSpeed;
	}
	else
	{
		FLOAT targetSpeed = 0.0f;

		if (MoveSpeedMode == EMSM_Override)
		{
			switch(EnemyMode)
			{
			case EM_Investigate:
				targetSpeed = MoveSpeed_Override.InvestigateSpeed;
				break;
			case EM_Chase:
				targetSpeed = MoveSpeed_Override.ChaseSpeed;
				break;
			case EM_Patrol:
			default:
				targetSpeed = MoveSpeed_Override.PatrolSpeed;
				break;
			}	
		}
		else // MoveSpeedMode == EMSM_RubberBanding
		{
			AOLHero* hero = Utils::GetHero();
			if (EnemyMode != EM_Chase || !Bot->SightComponent->CanSeeTarget || !hero)
			{
				targetSpeed = MoveSpeed_SpeedNoVisibility;
			}
			else
			{
				// in chase, target in sight
				
				FLOAT distToHero = (hero->Location - Location).Size2D();
				targetSpeed = MapClamped(distToHero, MoveSpeed_ChaseDistMin, MoveSpeed_ChaseDistMax, MoveSpeed_ChaseSpeedAtMinDist, MoveSpeed_ChaseSpeedAtMaxDist);
			}			
		}

		MoveSpeed_Target = targetSpeed; // for debug info

		FLOAT accelRate = 0.0f;

		if (targetSpeed > GroundSpeed)
		{
			accelRate = MoveSpeed_AccelRate;
		}
		else
		{
			accelRate = MoveSpeed_DecelRate;
		}

		if (accelRate <= 0.0f)
		{
			GroundSpeed = targetSpeed;
		}
		else
		{
			FLOAT delta = targetSpeed - GroundSpeed;
			FLOAT thisFrame = Min(accelRate * deltaTime, Abs(delta)); // positive

			if (delta >= 0.0f)
			{
				GroundSpeed += thisFrame;
			}
			else
			{
				GroundSpeed -= thisFrame;
			}
		}
	}
}

FLOAT AOLEnemyPawn::CalculateSpeed(FLOAT deltaTime, FLOAT currentSpeed, FVector moveDirection)
{
	TWEAKABLE FLOAT AccelCoeff = 0.999f;
	TWEAKABLE FLOAT DecelCoeff = 0.9f;
	TWEAKABLE FLOAT ReducedSpeedDistOuter = 200.0f;
	TWEAKABLE FLOAT ReducedSpeedDistInner = 50.0f;
	TWEAKABLE FLOAT ReducedSpeedFactor = 0.5f;

	TWEAKABLE FLOAT DecelDistance = 50.0f; // cm
	TWEAKABLE FLOAT MinDecel = 100.0f; // cm/s^2

	FLOAT targetSpeed = GroundSpeed;
	FLOAT distToDestination = (Location - Bot->GetDestinationPosition()).Size2D();
	FVector Projected = FVector(0.f);
	if (Bot->TargetPlayer != NULL && !Bot->TargetPlayer->bDeleteMe && FocusTarget == Bot->TargetPlayer && distToDestination < ReducedSpeedDistOuter)
	{
		FLOAT alpha = Clamp((distToDestination - ReducedSpeedDistInner)/(ReducedSpeedDistOuter - ReducedSpeedDistInner), 0.f, 1.f);
		FLOAT factor = LerpClamped(1.f - (((moveDirection | Rotation.Vector()) + 1.0f) / 2.0f), ReducedSpeedFactor, 1.0f);

		FVector PlayerVel = Bot->TargetPlayer->Velocity;
		FVector Projected = PlayerVel.ProjectOnTo(moveDirection);
		FLOAT minSpeed = Clamp(targetSpeed * factor + Projected.Size2D(), targetSpeed * factor, targetSpeed);

		targetSpeed = LerpClamped(alpha, minSpeed, targetSpeed);
	}

	FLOAT newSpeed = 0.f;
	if (Acceleration.IsZero())
	{
		newSpeed = Utils::Approach(currentSpeed, 0.f, DecelCoeff, deltaTime);
	}
	else if (2 * distToDestination * MinDecel <= Square(currentSpeed) && distToDestination <= DecelDistance && Projected.Size2D() < 10.0f)
	{
		newSpeed = currentSpeed - (deltaTime * Square(currentSpeed)) / (2.f * distToDestination);
	}
	else
	{
		newSpeed = Utils::Approach(currentSpeed, targetSpeed, AccelCoeff, deltaTime);
	}

	newSpeed = Clamp(newSpeed, 0.f, targetSpeed);

	return newSpeed;
}

void AOLEnemyPawn::UpdateTargetVelocity(FLOAT deltaTime)
{
	if (Physics == WalkingPhysics && !bAnimRootMotionActive)
	{
		FVector velocityDir(0.f);
		FVector toDest = (Bot->GetDestinationPosition() - Location);

		if (Acceleration.IsZero())
		{
			if (!Bot->GetDestinationPosition().IsZero())
			{
				velocityDir = toDest.SafeNormal();
			}
		}
		else
		{
			for (INT i = 0; i < SteeringBehaviors.Num(); ++i)
			{
				FVector currentVec = SteeringBehaviors(i)->GetSteeringVector();

#if !FINAL_RELEASE && !SHIPPING_PC_GAME
				if (bDrawSteeringDebug)
				{
					DrawDebugLine(Location + FVector(0.f, 0.f, 5.f), Location + currentVec * CylinderComponent->CollisionRadius + FVector(0.f, 0.f, 5.f), 255, 51, 51, FALSE);
				}
#endif

				velocityDir += currentVec;
			}
		}

		FLOAT currentSpeed = CalculateSpeed(deltaTime, TargetVelocity.Size2D(), velocityDir);

		if (!Bot->GetDestinationPosition().IsZero())
		{
			FLOAT toDestLength = toDest.Size2D(); 

			if (toDestLength < currentSpeed * deltaTime)
			{
				currentSpeed = toDestLength / deltaTime;
			}
		}
		else
		{
			currentSpeed = 0.f;
		}

		TargetVelocity = currentSpeed * velocityDir.SafeNormal();
	}
	else
	{
		if (Acceleration.IsNearlyZero())
		{
			TargetVelocity = FVector(0.f);
		}
		else
		{
			TargetVelocity = RealVelocity;
		}
	}
}

void AOLEnemyPawn::ApplyTurning(FLOAT deltaTime)
{
	if (Physics != WalkingPhysics || RotationMode == RM_Explicit)
	{
		Turning.bActive = FALSE;
		return;
	}

	TWEAKABLE FLOAT MinAngleForTurnLowSpeed = 65.0f;
	TWEAKABLE FLOAT MinAngleForTurnHighSpeed = 115.0f;
	TWEAKABLE FLOAT SpeedThreshold = 150.0f;
	TWEAKABLE FLOAT TurnTimeFast = 0.5f;
	TWEAKABLE FLOAT TurnTimeSlow = 1.0f;
	TWEAKABLE FLOAT BlendTimePct = 0.25f;
	TWEAKABLE FLOAT MinSpeedPct = 0.35f;
	TWEAKABLE FLOAT RotAngleApproachCoeff = 0.9f;
	TWEAKABLE FLOAT MaxAngleChange = 45.0f; // don't go crazy with dynamic angle modifications
	TWEAKABLE FLOAT MinDestDist = 200.0f;
	TWEAKABLE UBOOL bSmoothRotation = TRUE;	
	TWEAKABLE UBOOL bSquareVelocity = TRUE; // don't let velocity follow the current direction
	TWEAKABLE FLOAT MinAngleDiff = 45.0f;

	FLOAT targetYaw = (RotationMode == RM_FaceVelocity) ? (TargetVelocity.Rotation().Yaw * UNR_TO_DEG) : (TargetYaw);

	if (!Turning.bActive && TargetVelocity.IsNearlyZero(25.0f))
	{
		return;
	}

	if (Physics == WalkingPhysics && !Turning.bActive)
	{
		FLOAT deltaYaw = Utils::NormalizeRotAngle(targetYaw - UNR_TO_DEG * Rotation.Yaw);
		FLOAT distSqToDest = (Bot->GetDestinationPosition() - Location).SizeSquared2D();
		FLOAT currentVelocity = TargetVelocity.Size2D();
		FLOAT angleForTurn = currentVelocity > SpeedThreshold ? MinAngleForTurnHighSpeed : MinAngleForTurnLowSpeed;

		if (Abs(deltaYaw) > angleForTurn && distSqToDest >= Square(MinDestDist))
		{
			FLOAT turnDuration = 0.0f;
			if (EnemyMode == EM_Chase)
			{
				turnDuration = TurnTimeFast;
			}
			else
			{
				turnDuration = MapClamped(Abs(deltaYaw), MinAngleForTurnLowSpeed, 180.0f, TurnTimeFast, TurnTimeSlow);
			}

			Turning.bActive = TRUE;
			Turning.StartTime = GWorld->GetTimeSeconds();		
			Turning.StartYaw = UNR_TO_DEG * Rotation.Yaw;
			Turning.InitialDeltaYaw	= deltaYaw;
			Turning.Duration = turnDuration;
			Turning.FocalPoint = Bot->GetFocalPoint();
			Turning.StartSpeed = currentVelocity;
			Turning.SmoothedTargetYaw = targetYaw;
		}
	}	

	if (Turning.bActive)
	{
		FLOAT elapsedTime = GWorld->GetTimeSeconds() - Turning.StartTime;
		FLOAT timeToGo = Turning.Duration - elapsedTime;

		if (timeToGo <= 0.0f)
		{
			// Done
			Turning.bActive = FALSE;
			Bot->SetFocalPoint(Turning.FocalPoint);
		}
		else 
		{
			FLOAT deltaTargetToCurrent = Utils::NormalizeRotAngle(targetYaw - Turning.SmoothedTargetYaw);
			if (Abs(deltaTargetToCurrent) > MaxAngleChange)
			{
				// Abort -- we have a large angle change
				Turning.bActive = FALSE;
				Bot->SetFocalPoint(Turning.FocalPoint);
			}
			else
			{
				Bot->SetFocalPoint(FVector(0,0,0));

				FLOAT startSpeed = Turning.StartSpeed;
				FLOAT middleSpeed = MinSpeedPct*Turning.StartSpeed;
				FLOAT endSpeed = GroundSpeed;
				FLOAT desiredYaw = 0.0f;

				Turning.SmoothedTargetYaw = Utils::ApproachAngle(Turning.SmoothedTargetYaw, targetYaw, RotAngleApproachCoeff, deltaTime);

				FLOAT deltaYaw = Utils::NormalizeRotAngle(Turning.SmoothedTargetYaw - Turning.StartYaw);

				if (Sgn(deltaYaw) != Sgn(Turning.InitialDeltaYaw))
				{
					// Prevent switching turn direction, if the delta angle goes past the 180/-180 mark
					deltaYaw += 360.0f * Sgn(Turning.InitialDeltaYaw);
				}

				deltaYaw = Clamp(deltaYaw, Turning.InitialDeltaYaw - MaxAngleChange, Turning.InitialDeltaYaw + MaxAngleChange);

				if (bSmoothRotation)
				{
					// smooth
					desiredYaw = Utils::NormalizeRotAngle(Turning.StartYaw + Utils::SmoothLerp(elapsedTime / Turning.Duration, 0.0f, deltaYaw));
				}
				else
				{
					// linear
					desiredYaw = Utils::NormalizeRotAngle(Turning.StartYaw + (elapsedTime / Turning.Duration) * deltaYaw);
				}

				FLOAT desiredSpeed = 0.0f;
				FLOAT blendTime = BlendTimePct * Turning.Duration;

				if (elapsedTime < blendTime)
				{
					desiredSpeed = Utils::SmoothLerp(elapsedTime / blendTime, startSpeed, middleSpeed);					
				}
				else if (timeToGo < blendTime)
				{
					desiredSpeed = Utils::SmoothLerp(timeToGo / blendTime, endSpeed, middleSpeed);
				}
				else
				{
					desiredSpeed = middleSpeed;
				}

				TargetYaw = desiredYaw;

				if (bSquareVelocity)
				{
					if (elapsedTime <= 0.5f * Turning.Duration)
					{
						TargetVelocity = desiredSpeed * FRotator(0, Turning.StartYaw * DEG_TO_UNR, 0).Vector();
					}
					else
					{
						FLOAT initialFinalYaw = Utils::NormalizeRotAngle(Turning.StartYaw + Turning.InitialDeltaYaw);
						TargetVelocity = desiredSpeed * FRotator(0, initialFinalYaw * DEG_TO_UNR, 0).Vector();
					}
				}
				else
				{
					TargetVelocity = desiredSpeed * FRotator(0, DEG_TO_UNR * TargetYaw, 0).Vector();
				}
			}
		}
	}
}

void AOLEnemyPawn::ApplyPreferredPaths(FLOAT deltaTime)
{
	TWEAKABLE FLOAT MaxPctVelocity = 0.10f;

	if (!Turning.bActive && bHasPreferredPath && TargetVelocity.SizeSquared2D() > Square(25.0f))
	{
		FVector toAnchor = PreferredPathAnchor - Location;
		FLOAT distToAnchor = 0.0f;
		FVector dirToAnchor(0.0f);
		toAnchor.ToDirectionAndLength(dirToAnchor, distToAnchor);

		FLOAT curVel = 0.0f;
		FVector curDir(0.0f);
		TargetVelocity.ToDirectionAndLength(curDir, curVel);

		FVector approachVec = MaxPctVelocity * dirToAnchor;
		FVector newDir = (approachVec + curDir).SafeNormal();
		TargetVelocity = curVel * newDir;
	}
}

void AOLEnemyPawn::UpdateRotation(FLOAT deltaTime)
{
	TWEAKABLE FLOAT FaceVelocityApproachCoeff = 0.99f;

	if (IsDesiredRotationInUse() || Physics != WalkingPhysics || bAnimRootMotionActive)
	{
		RotationMode = RM_Explicit;
		TargetYaw = DesiredRotation.Yaw * UNR_TO_DEG;
	}
	else if (CanStrafe() && FocusTarget != NULL && (CurrentMovePathIdx == CurrentMovePath.Num() - 1 || CurrentMovePath.Num() == 0))
	{
		RotationMode = RM_FaceTarget;
		TargetYaw = (FocusTarget->Location - Location).SafeNormal2D().Rotation().Yaw * UNR_TO_DEG;		
	}
	else 
	{
		FLOAT instantTargetYaw = (TargetVelocity.IsNearlyZero(25.0f) ? Rotation : TargetVelocity.Rotation()).Yaw * UNR_TO_DEG;

		if (RotationMode != RM_FaceVelocity)
		{
			TargetYaw = instantTargetYaw;
		}
		else 
		{
			TargetYaw = Utils::ApproachAngle(TargetYaw, instantTargetYaw, FaceVelocityApproachCoeff, deltaTime);
		}		

		RotationMode = RM_FaceVelocity;
	}
}

void AOLEnemyPawn::CalcVelocity(FVector &AccelDir, FLOAT DeltaTime, FLOAT MaxSpeed, FLOAT Friction, INT bFluid, INT bBrake, INT bBuoyant)
{
	if (!bForceRMVelocity && Physics == WalkingPhysics && ProceduralAnims.Num() == 0)
	{
		Velocity = TargetVelocity; // computed in UpdateLocomotion()
	}
	else
	{
		Super::CalcVelocity(AccelDir,DeltaTime,MaxSpeed,Friction,bFluid,bBrake,bBuoyant);
	}
}

//
// 
//

void AOLEnemyPawn::performPhysics(FLOAT deltaSeconds)
{
	TWEAKABLE FLOAT MeshSmoothZThreshold = 5.0f;
	BYTE previousPhysics = Physics;

	Super::performPhysics(deltaSeconds);

	FLOAT deltaZ = Location.Z - OldZ;

	if (previousPhysics == WalkingPhysics && Physics == WalkingPhysics && Abs(deltaZ) > MeshSmoothZThreshold)
	{
		MeshZOffset -= deltaZ;
	}
}

void AOLEnemyPawn::UpdateLookAt(const FVector& lookAtTarget)
{
	TWEAKABLE FLOAT MaxCosAngleForLookAt = 0.5f; // 60 degs

	if (HeadTrackingLookAt != NULL)
	{
		FVector dirToTarget = (lookAtTarget - Location).SafeNormal2D();		
		UBOOL bWithinAngleRange = (dirToTarget | Rotation.Vector()) >= MaxCosAngleForLookAt;
		UBOOL bStateAllows = Bot->bEnableHeadTracking && bWithinAngleRange && !Turning.bActive && SpecialMove != SMT_AIVault;
		
		UBOOL bDoHeadTracking = bStateAllows && (Modifiers.bAlwaysLookAtPlayer || (Bot->BehaviorState == AIBS_Chasing && Bot->SightComponent->CanSeeTarget));

		if (bDoHeadTracking)
		{
			HeadTrackingLookAt->SetControlTargetLocation(lookAtTarget);
			HeadTrackingLookAt->SetSkelControlActive(TRUE);
		}
		else
		{
			HeadTrackingLookAt->SetSkelControlActive(FALSE);
		}
	}
}

void AOLEnemyPawn::UpdateFootPlacement(FLOAT DeltaSeconds)
{
	TWEAKABLE FLOAT ApproachCoeff = 0.995f;

	const FLOAT MaxZOffset = 50.0f;

	MeshZOffset = Utils::Approach(MeshZOffset, 0.0f, ApproachCoeff, DeltaSeconds);
	MeshZOffset = Clamp(MeshZOffset, -MaxZOffset, MaxZOffset);

	FLOAT targetOffset = BaseTranslationOffset + MeshZOffset;

	if (Abs(Mesh->Translation.Z - targetOffset) > KINDA_SMALL_NUMBERF)
	{
		Mesh->Translation.Z = targetOffset;
		Mesh->ConditionalUpdateTransform();
	}
}

void AOLEnemyPawn::UpdateMeshOffset(FLOAT deltaTime)
{
	AOLHero* hero = Utils::GetHero();
	if (!hero || !Bot)
	{
		return;
	}

	TWEAKABLE FLOAT MinDistForOffset = 90.0f;
	TWEAKABLE FLOAT MaxMeshOffset = 20.0f;

	FVector targetOffsetDir(0.0f);
	FLOAT targetOffsetDist = 0.0f;

	if (!Bot->bAttacking && !Modifiers.bDisableKnockbackFromPlayer && !Bot->bInvestigatingObject)
	{
		for (APawn* pawn = GWorld->GetWorldInfo()->PawnList; pawn != NULL; pawn = pawn->NextPawn)
		{
			AOLPawn* otherPawn = Cast<AOLPawn>(pawn);
			if (otherPawn && otherPawn != this && Abs(otherPawn->Location.Z - Location.Z) < 100.0f)
			{			
				FLOAT distSq2d = (otherPawn->Location - Location).SizeSquared2D();

				if (distSq2d < Square(MinDistForOffset))
				{		
					FLOAT offsetDist = MapClamped(appSqrt(distSq2d), 60.0f, MinDistForOffset, MaxMeshOffset, 0.0f);
					FVector fromOtherPawn2d = (Location - otherPawn->Location).SafeNormal2D();
					targetOffsetDir += fromOtherPawn2d;
					targetOffsetDist += offsetDist;
				}			
			}
		}
	}

	if (targetOffsetDist > 0.01f)
	{
		targetOffsetDist = Min(targetOffsetDist, MaxMeshOffset); // clamp
		targetOffsetDir = targetOffsetDir.SafeNormal2D();
	}

	FVector targetOffset2D = targetOffsetDist * targetOffsetDir;

	TWEAKABLE FLOAT ApproachCoeff = 0.99999f;
	MeshOffset2D = Utils::Approach(MeshOffset2D, targetOffset2D, ApproachCoeff, deltaTime);

	if (MeshOffset2D.SizeSquared2D() < Square(0.01f))
	{
		MeshOffset2D = FVector(0.0f);
	}

	FLOAT offsetDelta = (Vec2D(Mesh->Translation) - MeshOffset2D).SizeSquared2D();

	if (offsetDelta > Square(KINDA_SMALL_NUMBERF))
	{
		FVector newOffsetCS = Rotation.Quaternion().Inverse().RotateVector(MeshOffset2D);
		Mesh->Translation.X = newOffsetCS.X;
		Mesh->Translation.Y = newOffsetCS.Y;
		Mesh->ConditionalUpdateTransform();
	}
}

void AOLEnemyPawn::InitContextualVO()
{
	if (VOAsset != NULL)
	{
		VOInstances.Reset();
		VOInstances.AddZeroed(VOAsset->Mappings.Num());

		for (INT i = 0; i < VOInstances.Num(); ++i)
		{
			INT NumEvents = VOAsset->Mappings(i).Events.Num();
			if (NumEvents > 0)
			{
				VOInstances(i).EventsPlayed.AddZeroed(NumEvents);
				VOInstances(i).NumUnplayedEvents = NumEvents;
			}
		}

		PlayContextualVO(EVOC_Spawned);
	}
}

void AOLEnemyPawn::PlayContextualVO(BYTE VOContext, FLOAT DelayTime /*= 0.f*/)
{
	if (VOAsset == NULL || (EVOContext)VOContext == EVOC_Undefined)
	{
		return;
	}
	
	if (DelayTime > 0.f)
	{
		INT Idx = DelayedVOContexts.AddZeroed();
		DelayedVOContexts(Idx).TimeRemaining = DelayTime;
		DelayedVOContexts(Idx).VOContext = VOContext;
		return;
	}

	FVOMapping* Mapping = NULL;
	INT MappingIndex = -1;
	for (INT Idx = 0; Idx < VOAsset->Mappings.Num(); ++Idx)
	{
		if ((EVOContext)VOContext == VOAsset->Mappings(Idx).VOContext)
		{
			Mapping = &VOAsset->Mappings(Idx);
			MappingIndex = Idx;
			break;
		}
	}

	if (MappingIndex >= 0 &&  Mapping != NULL && Mapping->Events.Num() > 0)
	{
		check(MappingIndex < VOInstances.Num());

		FVOInstance& Instance = VOInstances(MappingIndex);

		if (Instance.NumUnplayedEvents == 0)
		{
			for (INT i = 0; i < Instance.EventsPlayed.Num(); ++i)
			{
				Instance.EventsPlayed(i) = FALSE;
			}

			Instance.NumUnplayedEvents = Instance.EventsPlayed.Num();
		}

		INT RandomSelection = appFloor(appFrand() * Instance.NumUnplayedEvents);
		INT EventToPlay = -1;

		for (INT i = 0; i < Instance.EventsPlayed.Num(); ++i)
		{
			if (!Instance.EventsPlayed(i))
			{
				if (RandomSelection == 0)
				{
					EventToPlay = i;
					break;
				}
				else
				{
					--RandomSelection;	
				}
			}
		}

		Instance.EventsPlayed(EventToPlay) = TRUE;
		--Instance.NumUnplayedEvents;
		UAkEvent* Event = Mapping->Events(EventToPlay);
		PlayAkEvent(Event);

		debugf(TEXT("[%s] Contextual VO: %s - %s"), *GetName(), *Utils::GetEnumString("EVOContext", VOContext), *Event->GetName());
	}
}

void AOLEnemyPawn::UpdateDelayedVO(FLOAT DeltaTime)
{
	INT i = 0;
	while (i < DelayedVOContexts.Num())
	{
		DelayedVOContexts(i).TimeRemaining -= DeltaTime;

		if (DelayedVOContexts(i).TimeRemaining <= 0.f)
		{
			PlayContextualVO(DelayedVOContexts(i).VOContext);

			DelayedVOContexts.Remove(i);
		}
		else
		{
			++i;
		}
	}
}

void AOLEnemyPawn::StartMatinee(FVector startLoc, FRotator startRot, FLOAT blendTime)
{
	// Do Nothing
}

void AOLEnemyPawn::setPhysics(BYTE NewPhysics, AActor* NewFloor /* = NULL */, FVector NewFloorV /* = FVector */)
{
	if (NewPhysics == WalkingPhysics)
	{
		ClearProceduralAnims();
	}

	Super::setPhysics(NewPhysics, NewFloor, NewFloorV);
}

FVector AOLEnemyPawn::GetTrueDoorDestination(AOLDoor* Door, FNavMeshPathObjectEdge* Edge)
{
	FVector Destination;
	Door->GetFinalEdgeDestination(Destination, Edge);

	FVector Direction = (Destination - Door->GetCenterLocation()).SafeNormal2D();
	FLOAT DirYaw = Direction.Rotation().Yaw;

	Destination = Door->Location;

	if (Door->eventShouldBreak(Bot))
	{
		Destination += DoorBashEndOffset.RotateAngleAxis(DirYaw, VecZ(1.f));
	}
	else
	{
		UBOOL bReversed = Edge->InternalPathObjectID == 2;

		if ((EnemyMode == EM_Patrol || Door->bAIAlwaysCloses) && bCloseDoorInPatrol)
		{
			if (bReversed)
			{
				if (Door->bReverseDirection)
				{
					Destination += DoorOpenEndOffsetPullLeftWithClose.RotateAngleAxis(DirYaw, VecZ(1.f));
				}
				else
				{
					Destination += DoorOpenEndOffsetPullRightWithClose.RotateAngleAxis(DirYaw, VecZ(1.f));
				}
			}
			else
			{
				if (Door->bReverseDirection)
				{
					Destination += DoorOpenEndOffsetPushRightWithClose.RotateAngleAxis(DirYaw, VecZ(1.f));
				}
				else
				{
					Destination += DoorOpenEndOffsetPushLeftWithClose.RotateAngleAxis(DirYaw, VecZ(1.f));
				}
			}
		}
		else
		{
			if (bReversed)
			{
				if (Door->bReverseDirection)
				{
					Destination += DoorOpenEndOffsetPullLeft.RotateAngleAxis(DirYaw, VecZ(1.f));
				}
				else
				{
					Destination += DoorOpenEndOffsetPullRight.RotateAngleAxis(DirYaw, VecZ(1.f));
				}
			}
			else
			{
				if (Door->bReverseDirection)
				{
					Destination += DoorOpenEndOffsetPushRight.RotateAngleAxis(DirYaw, VecZ(1.f));
				}
				else
				{
					Destination += DoorOpenEndOffsetPushLeft.RotateAngleAxis(DirYaw, VecZ(1.f));
				}
			}
		}
	}

	TWEAKABLE FLOAT ExtraBuffer = 10.0f;

	Destination += FVector(ExtraBuffer, 0.f, 0.f).RotateAngleAxis(DirYaw, VecZ(1.f));

	return Destination;
}


UBOOL AOLEnemyPawn::IsMoving(FLOAT Threshold /*= 10.0f*/)
{
	if (MovingTestPoints.Num() <= 0)
	{
		return FALSE;
	}
	
	return (MovingTestPoints.Top() - MovingTestPoints(0)).Size2D() > Threshold;
}

////////////////////////////////////////////////////////////////////////////////////////////
// UOLAISteeringFollowPath
////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////

FVector UOLAISteeringFollowPath::GetSteeringVector()
{
	AOLEnemyPawn* MyPawn = GetOuterAOLEnemyPawn();
	check(MyPawn);

	FVector Steering = FVector(0.f);

	if (MyPawn->CurrentMovePath.Num() == 0)
	{
		Steering = MyPawn->Acceleration.SafeNormal2D();
	}
	else if (MyPawn->CurrentMovePath.Num() == 1)
	{
		Steering = (MyPawn->CurrentMovePath.Last() - MyPawn->Location).SafeNormal2D();
	}
	else
	{
		UBOOL bPastLastPoint = FALSE;
		FLOAT T = MyPawn->Bot->OLNavHandle->CatmullRomGetBestTForPoint(
			FVector2D(MyPawn->Location),
			MyPawn->CRPathSubSegments,
			MyPawn->CRPathSegments(MyPawn->CurrentMovePathIdx),
			MyPawn->CRPathSegments(MyPawn->CurrentMovePathIdx+1),
			MyPawn->CRPathSegments(MyPawn->CurrentMovePathIdx+2),
			MyPawn->CRPathSegments(MyPawn->CurrentMovePathIdx+3),
			bPastLastPoint,
			MyPawn->CRPathLastIndex);

		if (bPastLastPoint || (1.0f - T) < 0.01f)
		{
			Steering = (MyPawn->CurrentMovePath(MyPawn->CurrentMovePathIdx) - MyPawn->Location).SafeNormal2D();
		}
		else
		{
			FVector2D Tangent = MyPawn->Bot->OLNavHandle->CatmullRomTangentOnCurve(
				MyPawn->CRPathSegments(MyPawn->CurrentMovePathIdx),
				MyPawn->CRPathSegments(MyPawn->CurrentMovePathIdx+1),
				MyPawn->CRPathSegments(MyPawn->CurrentMovePathIdx+2),
				MyPawn->CRPathSegments(MyPawn->CurrentMovePathIdx+3),
				T);

			FVector2D ClosestPoint = MyPawn->Bot->OLNavHandle->CatmullRomPointOnCurve(
				MyPawn->CRPathSegments(MyPawn->CurrentMovePathIdx),
				MyPawn->CRPathSegments(MyPawn->CurrentMovePathIdx+1),
				MyPawn->CRPathSegments(MyPawn->CurrentMovePathIdx+2),
				MyPawn->CRPathSegments(MyPawn->CurrentMovePathIdx+3),
				T);

			FVector2D ToCurve = ClosestPoint - FVector2D(MyPawn->Location);
			FLOAT Elasticity = (ToCurve.Size() - MyPawn->PathMinElasticityDistance) / (MyPawn->PathMaxElasticityDistance - MyPawn->PathMinElasticityDistance);

			Elasticity = Clamp(Elasticity, 0.0f, 1.0f);

			if (!Tangent.IsNearlyZero())
			{
				Steering = FVector((Tangent.SafeNormal() * (1.f - Elasticity) + ToCurve.SafeNormal() * Elasticity).SafeNormal(), 0.f);
			}
		}
	}

	return Steering;
}

FVector UOLAISteeringAvoidance::GetSteeringVector()
{
	TWEAKABLE FLOAT MAX_RADIUS = 500.0f;
	TWEAKABLE FLOAT MAX_ZDIFF = 100.0f;

	AOLEnemyPawn* MyPawn = GetOuterAOLEnemyPawn();
	check(MyPawn);

	FVector TotalRepulsion = FVector(0.f);

	AWorldInfo* WorldInfo = GWorld->GetWorldInfo();
	if (WorldInfo != NULL)
	{
		for (APawn* CheckPawn = WorldInfo->PawnList; CheckPawn != NULL; CheckPawn = CheckPawn->NextPawn)
		{
			AOLPawn* aPawn = Cast<AOLPawn>(CheckPawn);
			if (aPawn != NULL && aPawn != MyPawn && (aPawn->Location - MyPawn->Location).SizeSquared2D() <= Square(MAX_RADIUS) && Abs(aPawn->Location.Z - MyPawn->Location.Z) <= MAX_ZDIFF)
			{
				TotalRepulsion += aPawn->GetAIRepulsion(MyPawn);
			}
		}
	}

	FCheckResult Hit(1.f);
	FVector Origin = MyPawn->Location + FVector(0.f, 0.f, MyPawn->CylinderComponent->CollisionHeight);
	if (!UNavigationHandle::StaticObstacleLineCheck(MyPawn->Bot, Hit, Origin, Origin + (TotalRepulsion.SafeNormal()*5.0f), FVector(0.f), TRUE))
	{
		TotalRepulsion = FVector(0.f);
	}

	return TotalRepulsion;
}

////////////////////////////////////////////////////////////////////////////////////////////
// Surgeon
////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////

void AOLEnemySurgeon::OnPlayWeaponAnimNotify(UAnimNodeSequence* animNodeSeq)
{
	if (!bHasWeaponEquipped || !animNodeSeq)
	{
		return;
	}

	BoneShears->DetachFromAny();
	Mesh->AttachComponent(BoneShears, WeaponAttachBone);
	BoneShears->SetHiddenGame(FALSE);
	WeaponMesh->SetHiddenGame(TRUE);
	
	UAnimNodeSequence* animSeq = Cast<UAnimNodeSequence>(BoneShears->Animations);
	if (animSeq)
	{
		animSeq->SetAnim(animNodeSeq->AnimSeqName); // weapon anim name is the same as the originator anim
		animSeq->PlayAnim();
	}
}

void AOLEnemySurgeon::NativeOnAnimEnd(UAnimNodeSequence* seqNode, FLOAT playedTime, FLOAT excessTime)
{
	if (seqNode == Cast<UAnimNodeSequence>(BoneShears->Animations))
	{
		// Hide BoneShears skel mesh
		BoneShears->DetachFromAny();
		BoneShears->SetHiddenGame(TRUE);
		WeaponMesh->SetHiddenGame(FALSE);
	}
	else
	{
		Super::NativeOnAnimEnd(seqNode, playedTime, excessTime);
	}
}

////////////////////////////////////////////////////////////////////////////////////////////
// Cannibal
////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////

void AOLEnemyCannibal::OnPlayWeaponAnimNotify(UAnimNodeSequence* animNodeSeq)
{
	if (!bHasWeaponEquipped || !animNodeSeq)
	{
		return;
	}

	CannibalDrill->DetachFromAny();
	Mesh->AttachComponent(CannibalDrill, WeaponAttachBone);
	CannibalDrill->SetHiddenGame(FALSE);
	WeaponMesh->SetHiddenGame(TRUE);

	UAnimNodeSequence* animSeq = Cast<UAnimNodeSequence>(CannibalDrill->Animations);
	if (animSeq)
	{
		animSeq->SetAnim(animNodeSeq->AnimSeqName); // weapon anim name is the same as the originator anim
		animSeq->PlayAnim();
	}
}

void AOLEnemyCannibal::NativeOnAnimEnd(UAnimNodeSequence* seqNode, FLOAT playedTime, FLOAT excessTime)
{
	if (bHasWeaponEquipped && seqNode == Cast<UAnimNodeSequence>(CannibalDrill->Animations))
	{		
		CannibalDrill->DetachFromAny();
		CannibalDrill->SetHiddenGame(TRUE);
		WeaponMesh->SetHiddenGame(FALSE);
	}
	else
	{
		Super::NativeOnAnimEnd(seqNode, playedTime, excessTime);
	}
}

void AOLEnemyCannibal::NativeHideWeapon()
{
	if (CannibalDrill)
	{
		CannibalDrill->DetachFromAny();
		CannibalDrill->SetHiddenGame(TRUE);
	}
}

////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////