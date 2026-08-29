/**
 * Implementation of OLBot for Outlast.
 *
 * Copyright 2012 Red Barrels, Inc. All Rights Reserved.
 */

#include "OLGame.h"

IMPLEMENT_CLASS(AOLBot);
IMPLEMENT_CLASS(AOLWaypoint);
IMPLEMENT_CLASS(UOLAISightComponent);
IMPLEMENT_CLASS(AOLScout);
IMPLEMENT_CLASS(AOLAIGroup);
IMPLEMENT_CLASS(AOLAIInvestigationVolume);
IMPLEMENT_CLASS(AOLAIInvestigationPoint);

// Commands
IMPLEMENT_CLASS(UOLAICmd_MoveAbility);
IMPLEMENT_CLASS(UOLAICmd_MoveAbility_Door);
IMPLEMENT_CLASS(UOLAICmd_MoveAbility_Bash);
IMPLEMENT_CLASS(UOLAICmd_MoveAbility_Ledge);
IMPLEMENT_CLASS(UOLAICmd_MoveAbility_Vault);

// These numbers were picked because I couldn't find them being used in the base engine. They must be unique.
enum EOLBotAIFunctions
{
	OLBotAI_PollWaitForFullBodyAnim = 529,
	OLBotAI_PollWaitForSpecialMove = 530,
	OLBotAI_PollWaitForSpecialMoveNoDelay = 541,
	OLBotAI_PollMoveAlongPath = 540,
};

#define USE_UNSTUCK		1

/*============================================================================*/

////////////////////////////////////////////////////////////////////////////////////////////
/// Updates
////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////

UBOOL AOLBot::Tick( FLOAT DeltaSeconds, ELevelTick TickType )
{
	UBOOL Ticked = TRUE;
	if (!GWorld->IsPaused())
	{
		Ticked = Super::Tick(DeltaSeconds, TickType);

		if (Ticked)
		{
			if (TickType == LEVELTICK_All)
			{
				if (EnemyPawn != NULL)
				{
					SetLocation(EnemyPawn->Location);
					SetRotation(EnemyPawn->Rotation);
				}

				// Find Player
				if (TargetPlayer == NULL || TargetPlayer->bDeleteMe
					|| (TargetPlayer->bIsDummyPawn && TargetPlayer->Health <= 0 && !bAttacking))
				{
					if (TargetPlayer != NULL && (TargetPlayer->bDeleteMe || (TargetPlayer->bIsDummyPawn && TargetPlayer->Health <= 0)))
					{
						TargetPlayer = NULL;
						bTargetInDarkness = FALSE;
						eventClearDisturbance();
					}
					eventFindPlayer();
				}
				else
				{
					// Every tick: scoring-based pick for non-attack situations.
					// Every 0.5s: force-switch to closest target in AttackRange.
					if (TargetLockTimer > 0.f)
						TargetLockTimer -= DeltaSeconds;

					TargetSwitchTimer -= DeltaSeconds;
					if (TargetSwitchTimer <= 0.f)
					{
						TargetSwitchTimer = 0.5f;
						eventPickClosestInRange();
					}
					eventPickBestTarget();
				}

				// Update Behavior
				TickBehavior(DeltaSeconds);

				if (TargetPlayer != NULL && !TargetPlayer->bDeleteMe)
				{
					if (!bAttacking && !bInvestigatingObject && !IsInState(InterruptionState))
					{
						UBOOL bStopAction = FALSE;

						// Clear Latent Command if our target has moved
						if (CurrentMove.bIsDynamic && CurrentMove.Type == MT_Actor && CurrentMove.DestinationActor != NULL && !IsPerformingMoveAbility())
						{
							MoveTimeSinceLastPath += DeltaSeconds;
							if (MoveTimeSinceLastPath >= DynamicPathCheckTime && CurrentMove.DestinationActor->Location != MoveLastLocation)
							{
								bStopAction = TRUE;
							}
						}

						if (CurrentMove.Type != MT_Invalid && CurrentMove.DestinationPoint.DistanceSquared(EnemyPawn->Location) < CurrentMove.DestinationBuffer * CurrentMove.DestinationBuffer && !IsPerformingMoveAbility())
						{
							bStopAction = TRUE;
						}

						if (NextMove.Type != MT_Invalid && !IsPerformingMoveAbility())
						{
							bStopAction = TRUE;
						}

						if (bStopAction && EnemyPawn->CanInterruptSpecialMove())
						{
							UGameAICommand* MoveCommand = FindCommandOfClass(UOLAICmd_MoveAbility::StaticClass());
							if (MoveCommand != NULL)
							{
								AbortCommand(MoveCommand);
							}

							if (EnemyPawn->IsDoingSpecialMove())
							{
								EnemyPawn->CancelSpecialMove();
							}

							StopLatentExecution();
						}
					}

					// Update Disturbances
					if (VisualDisturbance.TimeSinceUpdate >= 0.0f)
					{
						VisualDisturbance.TimeSinceUpdate += DeltaSeconds;
					}
					if (AudioDisturbance.TimeSinceUpdate >= 0.0f)
					{
						AudioDisturbance.TimeSinceUpdate += DeltaSeconds;
					}

					if (NewDisturbanceResetTimer > 0.f)
					{
						NewDisturbanceResetTimer -= DeltaSeconds;

						if (NewDisturbanceResetTimer < 0.f)
						{
							NewDisturbanceResetTimer = 0.f;
						}
					}

					if (IgnoreDisturbanceTimer > 0.f)
					{
						IgnoreDisturbanceTimer -= DeltaSeconds;

						if (IgnoreDisturbanceTimer < 0.f)
						{
							IgnoreDisturbanceTimer = 0.f;
						}
					}

					// Update Noise
					if (DelayedNoises.Num() > 0)
					{
						FDelayedNoise* Noise = NULL;
						for (INT i = 0; i < DelayedNoises.Num(); ++i)
						{
							Noise = &DelayedNoises(i);
							Noise->TimeToNoise -= DeltaSeconds;
						}

						Noise = &DelayedNoises(0);
						while (Noise != NULL && Noise->TimeToNoise <= 0.0f)
						{
							UBOOL bIsMajorNoise = TargetPlayer != NULL && (Noise->NoiseType == TargetPlayer->RunningNoise || Noise->NoiseType == TargetPlayer->DoorMajorNoise);

							TriggerAudioDisturbance(Noise->Location, FALSE, bIsMajorNoise, bIsMajorNoise && bDisturbed, bIsMajorNoise && bDisturbed);
							DelayedNoises.Remove(0);
							if (DelayedNoises.Num() > 0)
							{
								Noise = &DelayedNoises(0);
							}
							else
							{
								Noise = NULL;
							}
						}
					}

					// Check Attack Range for Behavior
					if(IsInAttackRange() && SightComponent->CanSeeTarget)
					{
						if (!InAttackRange)
						{
							InAttackRange = TRUE;
							eventRecalculate();
						}
					}
					else
					{
						InAttackRange = FALSE;
					}

					// Check in Darkness
					bInDarkness = FALSE;
					for ( INT Idx = 0; Idx < EnemyPawn->Touching.Num(); Idx++ )
					{
						AOLDarknessVolume* const DV = Cast<AOLDarknessVolume>(EnemyPawn->Touching(Idx));
						if ( DV && DV->bDark )
						{
							bInDarkness = TRUE;
							break;
						}
					}

					// Check Target in Darkness
					bTargetInDarkness = TargetPlayer->bIsDummyPawn
						? (UBOOL)TargetPlayer->bNetInDarkness
						: TargetPlayer->IsInDarkness();

					// Check EnemyPawn in Electricity
					bInElectricity = FALSE;
					for ( INT Idx = 0; Idx < EnemyPawn->Touching.Num(); Idx++ )
					{
						AOLElectrifiedWaterVolume* const EV = Cast<AOLElectrifiedWaterVolume>(EnemyPawn->Touching(Idx));
						if (EV && EV->bElectrified)
						{
							bInElectricity = TRUE;
							break;
						}
					}

					// Update the Environment
					if (bInElectricity)
					{
						CurrentEnvironment = AIE_Electricity;
					}
					else if (bInDarkness)
					{
						CurrentEnvironment = AIE_Darkness;
					}
					else
					{
						CurrentEnvironment = AIE_Normal;
					}

					if (BehaviorState == AIBS_Investigating && SightComponent->CanSeeTarget)
					{
						eventRecalculate();
					}

					if ((BehaviorState == AIBS_Chasing || (BehaviorState == AIBS_Patrolling && bPatrolToPlayer)) && IsInState(TEXT("Idle")))
					{
						// For dummy pawns velocity is interpolated and unreliable — always treat as moving.
						if (!TargetPlayer->bIsDummyPawn && TargetPlayer->Velocity.IsNearlyZero(10.0f))
						{
							AIMoveReactionTimer = EnemyPawn->MoveReactionTime;
						}
						else
						{
							AIMoveReactionTimer -= DeltaSeconds;

							if (AIMoveReactionTimer <= 0.f)
							{
								eventRecalculate();

								AIMoveReactionTimer = -1.f;
							}
						}
					}
					else
					{
						AIMoveReactionTimer = -1.f;
					}

					if (OLNavHandle->AreNewObstaclesOnPath() && CurrentMoveStatus == MS_Moving && !IsInState(InterruptionState))
					{
						RegeneratePath();
					}

#if USE_UNSTUCK
					if (StuckRepathDelayTimer > 0.f)
					{
						StuckRepathDelayTimer -= DeltaSeconds;

						if (StuckRepathDelayTimer < 0.f)
						{
							StuckRepathDelayTimer = 0.f;
						}
					}

					if (!IsPerformingMoveAbility() && IsInState(FName(TEXT("Moving"))))
					{
						LookAheadTimer += DeltaSeconds;

						if (LookAheadTimer > EnemyPawn->LookAheadUpdateTime)
						{
							LookAheadTimer = 0.f;

							// Look Ahead
							FVector CurrentPoint = EnemyPawn->CurrentMovePath.Num() > 0 ? EnemyPawn->CurrentMovePath(EnemyPawn->CurrentMovePathIdx) : FVector(0.f);
							if (!CurrentPoint.IsZero() && StuckRepathDelayTimer <= 0.f)
							{
								FVector ToCurrentPoint = (CurrentPoint - EnemyPawn->Location);
								FVector ToCurrentPointNrm = ToCurrentPoint.SafeNormal();
								FLOAT ToCurrentPointLength = ToCurrentPoint.Size();

								FVector StartTrace = EnemyPawn->Location + EnemyPawn->CylinderComponent->Translation;
								FVector EndTrace = EnemyPawn->Location + EnemyPawn->CylinderComponent->Translation + ToCurrentPointNrm * Min(ToCurrentPointLength, EnemyPawn->LookAheadDistance);

								FVector Extent = EnemyPawn->GetCylinderExtent();
								Extent.Z -= EnemyPawn->MaxStepHeight;

								FMemMark CheckMark(GMainThreadMemStack);
								FCheckResult* CheckHits = GWorld->MultiLineCheck(GMainThreadMemStack, EndTrace, StartTrace, Extent, TRACE_AllBlocking, EnemyPawn);
								for( FCheckResult* Hit = CheckHits; Hit; Hit = Hit->GetNext())
								{
									if (!Hit->Actor->IsA(AOLPawn::StaticClass()))
									{
										RegeneratePath();

										StuckRepathDelayTimer = StuckRepathDelayLength;

										AI_LOG(this, (TEXT("Repath: Caused by LookAhead.")));
										break;
									}
								}
								CheckMark.Pop();
							}
						}
					}
					else
					{
						LookAheadTimer = 0.f;
					}

					// UnStuck Timer
					if (EnemyPawn->SpecialMove == SMT_None
						&& !EnemyPawn->IsMoving() && EnemyPawn->TargetVelocity.Size2D() > CheckStuckSpeedThreshold)
					{
						CheckStuckTimer += DeltaSeconds;

						if (CheckStuckTimer > EnemyPawn->UnstuckCheckTime && StuckRepathDelayTimer <= 0.f)
						{
							TryPushBlockingPawns();

							RegeneratePath();

							StuckRepathDelayTimer = StuckRepathDelayLength;
							CheckStuckTimer = 0.f;

							AI_LOG(this, (TEXT("Repath: Caused by UnStuck.")));							
						}
					}
					else
					{
						CheckStuckTimer = 0.f;
					}
#endif

					UpdatePreferredPaths();

					EnemyPawn->UpdateLookAt(TargetPlayer->EyeLocation);

					if (AttackTimer > 0.f)
					{
						AttackTimer = Max(AttackTimer - DeltaSeconds, 0.f);

						if (AttackTimer == 0.f)
						{
							eventRecalculate();
						}
					}

					if (bPatrolToPlayer)
					{
						if (PatrolToPlayerUpdateRate == 0.0f)
						{
							PatrolToPlayerLastLocation = TargetPlayer->Location;
						}
						else
						{
							PatrolToPlayerLastUpdated += DeltaSeconds;

							if (PatrolToPlayerLastUpdated >= PatrolToPlayerUpdateRate)
							{
								PatrolToPlayerLastLocation = TargetPlayer->Location;
								PatrolToPlayerLastUpdated = 0.0f;
							}
						}
					}

					if (IgnoreTimer > 0.f)
					{
						IgnoreTimer = Max(IgnoreTimer - DeltaSeconds, 0.f);

						if (IgnoreTimer == 0.f)
						{
							eventToggleAIIgnorePlayer(false);
						}
					}

					// Handle VO when changing Behavior States
					if (BehaviorState != LastBehaviorState)
					{
						if ((LastBehaviorState == AIBS_Patrolling || LastBehaviorState == AIBS_Idle) && BehaviorState == AIBS_Investigating)
						{
							if (EnemyPawn->Modifiers.bInterruptVOOnChase && Utils::GetOLGame())
							{
								Utils::GetOLGame()->VoiceManager->ClearVOQueue(EnemyPawn);
							}

							EnemyPawn->PlayContextualVO(EVOC_PatrolToInvestigate, 1.0f);
						}
						else if (BehaviorState == AIBS_Chasing)
						{
							if (EnemyPawn->Modifiers.bInterruptVOOnChase && Utils::GetOLGame())
							{
								Utils::GetOLGame()->VoiceManager->ClearVOQueue(EnemyPawn);
							}
							
							if (Group != NULL && Group->Bots.Num() > 1)
							{
								EnemyPawn->PlayContextualVO(EVOC_SwitchToChaseMultiple);
							}
							else
							{
								EnemyPawn->PlayContextualVO(EVOC_SwitchToChase);
							}
						}
						else if (LastBehaviorState == AIBS_Chasing && BehaviorState == AIBS_Investigating)
						{
							EnemyPawn->PlayContextualVO(EVOC_ChaseToInvestigate);
						}

						LastBehaviorState = BehaviorState;
					}
				}
			}
		}
	}

#if !FINAL_RELEASE && !SHIPPING_PC_GAME
	if (bAIDrawDebug || (Utils::GetCheatManager() && Utils::GetCheatManager()->bDrawDebugAI))
	{
		DrawDebug();
	}
#endif

	return Ticked;
}

void AOLBot::TickBehavior( FLOAT DeltaSeconds )
{
	if (EnemyPawn->BehaviorTree == NULL)
		return;

	EStatus btStatus = Status_Invalid;
	if (RootBehavior == NULL)
	{
		RootBehavior = ConstructObject<UOLBTBehavior>(UOLBTBehavior::StaticClass(), this);
		RootBehavior->Setup(EnemyPawn->BehaviorTree->RootNode, this);

		btStatus = RootBehavior->Tick(DeltaSeconds);
	}
	else if (ShouldRecalculate)
	{
		if (bForceRecalculate)
		{
			RootBehavior->Setup(EnemyPawn->BehaviorTree->RootNode, this);
			btStatus = RootBehavior->Tick(DeltaSeconds);
		}
		else
		{
			TArray<FBehaviorState> RootState;
			TArray<FBehaviorState> NewState;

			RootBehavior->GetTreeState(RootState);

			// Reuse PendingBehavior to avoid allocating a new UOLBTBehavior every recalc tick.
			if (PendingBehavior == NULL)
				PendingBehavior = ConstructObject<UOLBTBehavior>(UOLBTBehavior::StaticClass(), this);
			PendingBehavior->Setup(EnemyPawn->BehaviorTree->RootNode, this);

			btStatus = PendingBehavior->Tick(DeltaSeconds, &RootState, &NewState);

			if (btStatus == Status_ReachedCurrent)
			{
				AI_LOG( this, ( TEXT("BT Log: Reached Behavior State") ) );

				btStatus = RootBehavior->Tick(DeltaSeconds);
			}
			else
			{
				// Swap: old root becomes the next pending — zero allocations.
				UOLBTBehavior* OldRoot = RootBehavior;
				OldRoot->Teardown();
				RootBehavior    = PendingBehavior;
				PendingBehavior = OldRoot;
			}
		}


		ShouldRecalculate = false;
		bForceRecalculate = false;
	}
	else
	{
		btStatus = RootBehavior->Tick(DeltaSeconds);
	}

	// Clear the Node if it finished
	if(btStatus != Status_Running)
	{
		RootBehavior->Setup(EnemyPawn->BehaviorTree->RootNode, this);
		
		// Tick one more time
		btStatus = RootBehavior->Tick(DeltaSeconds);			
		if(btStatus != Status_Running)
		{
			RootBehavior->Setup(EnemyPawn->BehaviorTree->RootNode, this);
		}
	}

}

////////////////////////////////////////////////////////////////////////////////////////////
/// Movement
////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////

/** Interface_NavigationHandle */
void AOLBot::SetupPathfindingParams( FNavMeshPathParams& out_ParamCache )
{
	VERIFY_NAVMESH_PARAMS(9);

	out_ParamCache.bAbleToSearch=TRUE;
	out_ParamCache.SearchExtent=NavigationExtent;
	out_ParamCache.SearchLaneMultiplier = 0.f;
	out_ParamCache.SearchStart=Pawn->Location;
	out_ParamCache.MaxDropHeight=GetMaxDropHeight();
	out_ParamCache.bCanMantle=FALSE;
	out_ParamCache.bNeedsMantleValidityTest = FALSE;
	out_ParamCache.MinWalkableZ = Pawn->WalkableFloorZ;
	out_ParamCache.MaxHoverDistance = -1.f;
}

/** Interface_NavigationHandle */
FVector AOLBot::GetEdgeZAdjust(struct FNavMeshEdgeBase* Edge)
{
	return FVector(0.f);
}

FLOAT AOLBot::DistToGround(const FVector& Point, const FVector& Extent)
{
	FCheckResult Hit;
	if (!GWorld->SingleLineCheck(Hit, this, Point + FVector(0.0f, 0.0f, -300.0f), Point + FVector(0.0f, 0.0f, 200.0f), TRACE_World, Extent))
	{
		return Point.Distance(Hit.Location);
	}

	return 0.0f;
}

UBOOL BotIsValidAnchorPos(const UNavigationHandle::FFitNessFuncParams& Params)
{
	TWEAKABLE FLOAT MaxZDiff = 240.f;

	if (Abs(Params.StartPt.Z - Params.Point.Z) > MaxZDiff)
	{
		return FALSE;
	}

	// check reachability
	FVector StartCheckPos = Params.StartPt;

	INT Its=0;
	FCheckResult Hit(1.f);

	FVector CheckDir = (Params.Point - StartCheckPos).SafeNormal();

	while(Its++ < 5 && !UNavigationHandle::StaticObstacleLineCheck(Params.AskingHandle, Hit, StartCheckPos, Params.Point,Params.Extent,TRUE,NULL,Params.PylonsWeCareAbout) )
	{
		// filter backface hits
		if ( (CheckDir | Hit.Normal)  < KINDA_SMALL_NUMBER )
		{
			StartCheckPos = Hit.Location + CheckDir * FBoxPushOut(Hit.Normal,Params.Extent)*1.1f;
		}
		else
		{
			return FALSE;
		}
	}

	AOLBot* Bot = Cast<AOLBot>(Params.AskingHandle->CachedPathParams.Interface->GetUObjectInterfaceInterface_NavigationHandle());
	if (Bot && ((Params.Point - StartCheckPos).Size2D() > KINDA_SMALL_NUMBER))
	{
		FVector PointAtFloor = Params.Point;
		FVector StartAtFloor = StartCheckPos;

		if (!GWorld->SingleLineCheck(Hit, Bot, Params.Point - FVector(0.f, 0.f, Params.Extent.Z), Params.Point + FVector(0.f, 0.f, 2.f*Params.Extent.Z), TRACE_World))
		{
			PointAtFloor = Hit.Location;
		}

		if (!GWorld->SingleLineCheck(Hit, Bot, StartCheckPos - FVector(0.f, 0.f, Params.Extent.Z), StartCheckPos + FVector(0.f, 0.f, 2.f*Params.Extent.Z), TRACE_World))
		{
			StartAtFloor = Hit.Location;
		}

		if (!GWorld->SingleLineCheck(Hit, Bot, PointAtFloor + FVector(0.f, 0.f, 10.0f), StartAtFloor + FVector(0.f, 0.f, 10.0f), TRACE_World|TRACE_StopAtAnyHit))
		{
			return FALSE;
		}
	}

	if ( Params.PolyContainingPoint != NULL )
	{
		FNavMeshPolyBase* CheckablePoly = Params.PolyContainingPoint;

		if (Params.PolyContainingPoint->NumObstaclesAffectingThisPoly > 0)
		{
			FPolyObstacleInfo* Info = Params.PolyContainingPoint->GetObstacleInfo();
			FBox Box = FBox::BuildAABB(Params.Point,Params.Extent);

			checkSlowish(Info->SubMesh != NULL);
			checkSlowish(!Info->bNeedRecompute);
			CheckablePoly = Info->SubMesh->GetPolyFromBox(Box,Bot->EnemyPawn->WalkableFloorZ);

			FPolyObstacleInfo* PolyObstacleInfo = Params.PolyContainingPoint->GetObstacleInfo();
			if (PolyObstacleInfo != NULL)
			{
				for (INT i = 0; i < PolyObstacleInfo->LinkedObstacles.Num(); ++i)
				{
					AOLEnemyPawn* ePawn = Cast<AOLEnemyPawn>(PolyObstacleInfo->LinkedObstacles(i)->GetUObjectInterfaceInterface_NavMeshPathObstacle());
					if (ePawn != NULL && ePawn != Bot->EnemyPawn)
					{
						if ((Params.Point - ePawn->Location).SizeSquared2D() < Square(80.0f))
						{
							return FALSE;
						}
					}
				}
			}
		}

		if (CheckablePoly != NULL)
		{
			return CheckablePoly->IsEscapableBy(Params.AskingHandle->CachedPathParams);
		}	
	}
	return FALSE;
}

UBOOL AOLBot::GetClosestPointOnNavMesh(FVector& out_NewPoint,FVector PointToCheck)
{
	if( !NavigationHandle->PopulatePathfindingParamCache()) 
	{
		return FALSE;
	}

	static TArray<FVector> Points;
	Points.Reset();

	FVector Extent = NavigationHandle->CachedPathParams.SearchExtent;

	NavigationHandle->GetValidPositionsForBoxEx(PointToCheck, EnemyPawn->AttackRange, Extent, FALSE, Points, 1, 0.f, Extent, &BotIsValidAnchorPos);

	if (Points.Num() > 0)
	{
		out_NewPoint = Points(0);

		AScout* Scout = AScout::GetGameSpecificDefaultScoutObject();
		FCheckResult Hit(1.0f);

		//out_NewPoint.Z -= 2.0f*Scout->NavMeshGen_EntityHalfHeight;

		APylon* Pylon = NULL;
		FNavMeshPolyBase* Poly = NULL;
		NavigationHandle->GetPylonAndPolyFromPos(out_NewPoint,Scout->WalkableFloorZ, Pylon, Poly);
		if (Pylon != NULL && Pylon->FindGround(out_NewPoint, Hit, Scout))
		{
			out_NewPoint.Z = Hit.Location.Z;
		}

		return TRUE;
	}

	return FALSE;
}

FLOAT AOLBot::DistToClosestPawn()
{
	TWEAKABLE FLOAT MinDeltaZ = -100.0f;
	TWEAKABLE FLOAT MaxDeltaZ = 200.0f;

	FLOAT bestDist = FLT_MAX;

	for (APawn* pawn = GWorld->GetWorldInfo()->PawnList; pawn != NULL; pawn = pawn->NextPawn)
	{
		if (pawn && pawn != EnemyPawn && pawn->Location.Z >= (EnemyPawn->Location.Z + MinDeltaZ) && pawn->Location.Z <= (EnemyPawn->Location.Z + MaxDeltaZ))
		{			
			FLOAT distToPawn = pawn->Location.Distance(EnemyPawn->Location);
			bestDist = Min(bestDist, distToPawn);
		}
	}

	return bestDist;
}

void AOLBot::TryPushBlockingPawns()
{
	TWEAKABLE FLOAT MinDeltaZ = -100.0f;
	TWEAKABLE FLOAT MaxDeltaZ = 200.0f;
	TWEAKABLE FLOAT MinDistForPush = 80.0f;

	UGameAICommand* moveAbility = FindCommandOfClass(UOLAICmd_MoveAbility::StaticClass());
	if (!moveAbility || moveAbility->GetStateFrame()->StateNode->GetFName() != FName(TEXT("Approaching"), FNAME_Find))
	{
		// rcharpentier - currently enabled only when approaching for a move ability, but might be useful in other cases
		return;
	}

	TArray<AOLEnemyPawn*> NearbyPawns;

	for (APawn* pawn = GWorld->GetWorldInfo()->PawnList; pawn != NULL; pawn = pawn->NextPawn)
	{
		AOLEnemyPawn* otherPawn = Cast<AOLEnemyPawn>(pawn);
		if (otherPawn && otherPawn != EnemyPawn && otherPawn->Location.Z >= (EnemyPawn->Location.Z + MinDeltaZ) && otherPawn->Location.Z <= (EnemyPawn->Location.Z + MaxDeltaZ))
		{			
			FLOAT distToPawn = otherPawn->Location.Distance(EnemyPawn->Location);
			
			if (distToPawn < MinDistForPush)
			{
				otherPawn->MoveOutOfTheWay();
			}
		}
	}
}

void AOLBot::UpdatePreferredPaths()
{
	TWEAKABLE FLOAT MinDistToOtherPawn = 300.0f;
	TWEAKABLE FLOAT MinStraightPathDist = 400.0f;
	TWEAKABLE FLOAT MaxDeltaZ = 50.0f;
	TWEAKABLE FLOAT MinCosAngleToPath = 0.966f; // 15 degs
	TWEAKABLE FLOAT MaxDistToPath = 250.0f; 
	TWEAKABLE FLOAT MinDistToPath = 12.0f; 

	UBOOL bValidState = !IsPerformingMoveAbility() && IsInState(FName(TEXT("Moving")));
	UBOOL bNoNearbyPawn = DistToClosestPawn() > MinDistToOtherPawn;
	AOLHero* hero = Utils::GetHero(); // we're piggybacking on top of hero's gameplay object caching mechanism

	// Reset to invalid
	EnemyPawn->bHasPreferredPath = FALSE;
	EnemyPawn->PreferredPathAnchor = FVector(0.0f);
	EnemyPawn->PreferredPathDirection = FVector(0.0f);
	EnemyPawn->PreferredPath = NULL;

	if (EnemyPawn->bUsePreferredPaths && bValidState && bNoNearbyPawn && hero && hero->CachedPreferredPathMarkers.Num() > 0)
	{
		FVector target(0.0f);
		
		if (EnemyPawn->CurrentMovePath.Num() > 0)
		{
			target = EnemyPawn->CurrentMovePath(EnemyPawn->CurrentMovePathIdx);

			// check for multiple points forming a single straight path (broken up to prevent large paths), keep only the farthest one still forming a straight line
			FVector basePathDir = (target - EnemyPawn->Location).SafeNormal();
			for (INT i = EnemyPawn->CurrentMovePathIdx + 1; i < EnemyPawn->CurrentMovePath.Num(); i++)
			{
				FVector segmentDir = (EnemyPawn->CurrentMovePath(i) - EnemyPawn->CurrentMovePath(i - 1)).SafeNormal();
				if ((segmentDir | basePathDir) < 0.98f)
				{
					break;
				}
				target = EnemyPawn->CurrentMovePath(i);
			}
		}
		else
		{
			target = CurrentMove.DestinationPoint;
		}

		FVector toTarget = (target - EnemyPawn->Location);
		FLOAT distToTarget = 0.0f;
		FVector dirToTarget(0.0f);
		toTarget.ToDirectionAndLength(dirToTarget, distToTarget);

		if (toTarget.SizeSquared() > Square(MinStraightPathDist))
		{
			for (INT i = 0; i < hero->CachedPreferredPathMarkers.Num(); i++)
			{
				AOLPreferredPathMarker* node1 = hero->CachedPreferredPathMarkers(i);

				if (node1 && node1->IsValid() && node1->Next && node1->Next->IsValid())
				{
					AOLPreferredPathMarker* node2 = node1->Next;

					const FVector& node1Loc = node1->Location;
					const FVector& node2Loc = node2->Location;
					
					// Height check - are we on the same floor?
					if ( (Max(node1Loc.Z, node2Loc.Z) < (EnemyPawn->Location.Z - MaxDeltaZ)) || (Min(node1Loc.Z, node2Loc.Z) > (EnemyPawn->Location.Z + MaxDeltaZ)))
					{
						continue;
					}

					const FVector& path = node2Loc - node1Loc;
					FVector pathDir = path.SafeNormal();
					
					// Angle check - are we parallel to the path?
					if (Abs(pathDir | dirToTarget) < MinCosAngleToPath)
					{
						continue;
					}
					
					FVector closestPoint(0.0f);
					FLOAT distToPath = PointDistToSegment(EnemyPawn->Location, node1Loc, node2Loc, closestPoint);
					
					// Distance to path - are we even close?
					if (distToPath > MaxDistToPath)
					{
						continue;
					}

					// Are we already on the path?
					if (distToPath < MinDistToPath)
					{
						continue;
					}

					// Are we within the path?
					if (!Utils::IsBetweenMarkers(EnemyPawn->Location, node1Loc, node2Loc, 100.0f))
					{
						continue;
					}

					// Forward dist check - is one of the nodes far enough forward (or are we instead near or passed the end of the path)?
					FLOAT pureFwdDist1 = (node1Loc - EnemyPawn->Location) | dirToTarget;
					FLOAT pureFwdDist2 = (node2Loc - EnemyPawn->Location) | dirToTarget;
					if (Max(pureFwdDist1, pureFwdDist2) < MinStraightPathDist)
					{
						continue;
					}
					
					// Seems good.
					EnemyPawn->bHasPreferredPath = TRUE;
					EnemyPawn->PreferredPathAnchor = closestPoint;
					EnemyPawn->PreferredPathDirection = (pureFwdDist1 < pureFwdDist2) ? pathDir : -pathDir;
					EnemyPawn->PreferredPath = node1;
					break;
				}
			}
		}
	}
}

void AOLBot::StopMoving(UBOOL bAborted)
{
	UGameAICommand* MoveCmd = FindCommandOfClass(UOLAICmd_MoveAbility::StaticClass());
	if(MoveCmd != NULL && (MoveCmd->GetStateFrame()->StateNode->GetFName() == FName(TEXT("Approaching"), FNAME_Find) || MoveCmd->GetStateFrame()->StateNode->GetFName() == FName(TEXT("Waiting"), FNAME_Find)))
	{
		AbortCommand(MoveCmd);
	}

	if(GetStateFrame()->StateNode->GetFName() == FName(TEXT("Moving"), FNAME_Find))
	{
		if( Pawn != NULL )
		{
			Pawn->Acceleration = FVector::ZeroVector;
		}
		GetStateFrame()->LatentAction = 0;

		if(bAborted)
		{
			CurrentMoveStatus = MS_Failed;
			LastMoveFailedReason = MFR_Aborted;
		}

		EGotoState ResultState = GOTOSTATE_Success;
		ResultState = GotoState( FName( TEXT("Idle"), FNAME_Find ) );
		if ( ResultState == GOTOSTATE_Success )
		{
			GotoLabel( FName(NAME_Begin) );
		}
	}
}

FRotator AOLBot::SetRotationRate(FLOAT deltaTime)
{
	if (EnemyPawn == NULL)
	{
		return FRotator(0,0,0);
	}
	else if (EnemyPawn->Turning.bActive)
	{
		// Let the turning system do its work (but maybe eventually modify physRotation to bypass the SetRotationRate)
		return FRotator(0, DEG_TO_UNR * 720.0f, 0);
	}
	else
	{
		FLOAT deltaYaw = Abs(FRotator::NormalizeAxis(EnemyPawn->DesiredRotation.Yaw - EnemyPawn->Rotation.Yaw)) * UNR_TO_DEG;
		FLOAT pawnVel = EnemyPawn->RealVelocity.Size2D();

		TWEAKABLE FLOAT AngleForMaxRate = 90.0f;
		TWEAKABLE FLOAT VelocityForMaxAmplification = 400.0f;
		TWEAKABLE FLOAT VelocityAmplification = 1.0f; // Additional pct on angle factor
		TWEAKABLE FLOAT MinRate = 45.0f;
		TWEAKABLE FLOAT MaxRate = 720.0f;
		TWEAKABLE FLOAT BaseAngleContrib = 0.5f;
		
		FLOAT angleFactor = BaseAngleContrib * Clamp(deltaYaw / AngleForMaxRate, 0.0f, 1.0f);
		FLOAT velAmplification = 1.0f + VelocityAmplification * Clamp(pawnVel / VelocityForMaxAmplification, 0.0f, 1.0f);

		FLOAT rateFactor = Min(1.0f, velAmplification * angleFactor);
		FLOAT rotationRate = Lerp(MinRate, MaxRate, rateFactor);

		return FRotator(0, DEG_TO_UNR * rotationRate * deltaTime, 0);
	}
}

void AOLBot::MoveAlongPath(const TArray<FVector>& PathPoints, AActor* FocusTarget)
{
	if ( !Pawn || PathPoints.Num() == 0)
		return;

	FVector FinalDest = PathPoints.Top();

	Pawn->bReducedSpeed = FALSE;
	Pawn->DesiredSpeed = Pawn->MaxDesiredSpeed;
	Pawn->DestinationOffset = -20.0f;
	Pawn->NextPathRadius = 0.f;
	//Pawn->setMoveTimer(MoveDir);
	EnemyPawn->FocusTarget = FocusTarget;

	GetStateFrame()->LatentAction = OLBotAI_PollMoveAlongPath;

	UBOOL bBasedDest = FALSE;
	FCheckResult Hit;
	if( !GWorld->SingleLineCheck( Hit, Pawn, FinalDest + FVector(0,0,-100), FinalDest, TRACE_World ) )
	{
		if( Hit.Actor == Pawn->Base )
		{
			bBasedDest = TRUE;
		}
	}

	SetDestinationPosition( FinalDest, bBasedDest );
	//SetFocalPoint( PathPoints(0), bBasedDest );

	CurrentPath = NULL;
	NextRoutePath = NULL;
	Pawn->ClearSerpentine();
	SetAdjustLocation( FinalDest, FALSE );
	bAdvancedTactics = FALSE;

	EnemyPawn->moveAlongPath(PathPoints);
}

void AOLBot::MoveTo(const FVector& Dest, AActor* ViewFocus, FLOAT DesiredOffset, UBOOL bShouldWalk)
{
	EnemyPawn->CurrentMovePath.Reset();
	EnemyPawn->CurrentMovePathIdx = 0;

	EnemyPawn->FocusTarget = ViewFocus;

	Super::MoveTo(Dest, ViewFocus, DesiredOffset, bShouldWalk);
}

void AOLBot::PollMoveComplete()
{
	Super::PollMoveComplete();

	EnemyPawn->CurrentMovePath.Reset();
	EnemyPawn->CurrentMovePathIdx = 0;
}

void AOLBot::execPollMoveAlongPath( FFrame& Stack, RESULT_DECL )
{
	if( !Pawn || ((MoveTimer < 0.f) && (Pawn->Physics != PHYS_Falling)) )
	{
		PollMoveComplete();
		return;
	}

	if ( bAdjusting )
	{
		bAdjusting = !Pawn->moveToward(GetAdjustLocation(), NULL);
		if( !bAdjusting )
		{
			if( NavigationHandle != NULL && NavigationHandle->HandleFinishedAdjustMove() )
			{			
				return;
			}
		}
	}
	if (!bAdjusting)
	{
		PrePollMove();
		if( Pawn == NULL || Pawn->moveToward(GetDestinationPosition(), NULL))
		{
			PollMoveComplete();
		}
		else
		{
			PostPollMove();
		}
	}
}
IMPLEMENT_FUNCTION( AOLBot, OLBotAI_PollMoveAlongPath, execPollMoveAlongPath );

void AOLBot::RegeneratePath()
{
	if (bReGeneratePath || !IsInState(FName(TEXT("Moving"))))
	{
		return;
	}

	if (!IsPerformingMoveAbility())
	{
		if (CommandList != NULL)
		{
			AbortCommand(CommandList);
		}

		StopLatentExecution();

		bRegenerateWhilePerforming = FALSE;
	}
	else
	{
		bRegenerateWhilePerforming = TRUE;
	}

	bReGeneratePath = TRUE;
}

////////////////////////////////////////////////////////////////////////////////////////////
/// Hearing
////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////

void AOLBot::TriggerAudioDisturbance(FVector DisturbanceLocation, UBOOL bFromGroup, UBOOL bNewInvestigate, UBOOL bNoTrim, UBOOL bIgnoreDisturbs)
{
	if (!EnemyPawn->Modifiers.bShouldAttack)
	{
		return;
	}

	AudioDisturbance.TimeSinceUpdate = 0.0f;
	AudioDisturbance.Location = DisturbanceLocation;

	bNoTrimDisturbance = bNoTrim;

	TryNewDisturbance();

	if (bIgnoreDisturbs && IgnoreDisturbanceTimer == 0.f)
	{
		IgnoreDisturbanceTimer = 3.0f;
	}

	if (bNewInvestigate)
	{
		ClearInvestigation();
	}

	eventRecalculate();

	if (Group != NULL && !bFromGroup)
	{
		Group->eventTriggerAudioDisturbance(this, DisturbanceLocation, bNewInvestigate, bNoTrim);
	}
}

void AOLBot::TriggerVisualDisturbance(FVector DisturbanceLocation, UBOOL bFromGroup)
{
	if (!EnemyPawn->Modifiers.bShouldAttack)
	{
		return;
	}

	VisualDisturbance.TimeSinceUpdate = 0.0f;
	VisualDisturbance.Location = DisturbanceLocation;

	bNoTrimDisturbance = FALSE;

	TryNewDisturbance();

	if (!SightComponent->CanSeeTarget)
	{
		eventRecalculate();
	}

	if (Group != NULL && !bFromGroup)
	{
		Group->eventTriggerVisualDisturbance(this, DisturbanceLocation);
	}
}

void AOLBot::TryNewDisturbance()
{
	if (NewDisturbanceResetTimer <= 0.f)
	{
		bNewDisturbance = TRUE;
	}
}

UBOOL AOLBot::BotCanHear(FLOAT DistToNoise, FLOAT Loudness, FLOAT Occlusion)
{
	DebugLastLoudness = Loudness;
	DebugLastOcclusion = Occlusion;
	DebugLastDistance = DistToNoise;

	FLOAT Perceived = (1.0f - Occlusion) * Loudness * Pawn->HearingThreshold;

	// take pawn alertness into account (it ranges from -1 to 1 normally)
	Perceived *= ::Max(0.f,(Pawn->Alertness + 1.f));

	return Perceived >= DistToNoise;
}

/**
  *  Some early outs in C++ for performance
  */
void AOLBot::HearNoise(AActor* NoiseMaker, FLOAT Loudness, FName NoiseType)
{
	AOLHero* NoiseMakerHero = Cast<AOLHero>(NoiseMaker);
	const UBOOL bIsDummy = NoiseMakerHero && NoiseMakerHero->bIsDummyPawn;
	if (bIsDummy)
	{
		// Ignore noise from dummy players when enemy sync is disabled
		if (!UOLNetworkConfig::StaticClass()->GetDefaultObject<UOLNetworkConfig>()->SyncEnemies)
			return;
	}
	if ( !bIsDummy && (!NoiseMaker->Instigator || !NoiseMaker->Instigator->Controller) )
		return;
	if ( !WorldInfo->GRI )
		return;
	if ( !bIsDummy && WorldInfo->GRI->OnSameTeam(this, NoiseMaker->Instigator) )
		return;
	if(NoiseMakerHero && !NoiseMakerHero->bIsGhost)
	{
		FLOAT Distance;
		FLOAT Occlusion;
		UBOOL bGotOcclusion;
		if (bIsDummy)
		{
			Distance    = (NoiseMaker->Location - EnemyPawn->Location).Size();
			Occlusion   = 0.f;
			bGotOcclusion = TRUE;
		}
		else
		{
			bGotOcclusion = Utils::GetSoundEnvManager()->GetPlayerOcclusionForPawn(EnemyPawn, Distance, Occlusion);
		}
		if (bGotOcclusion)
		{
			if ( BotCanHear(Distance, Loudness, Occlusion) )
			{			
				FLOAT waitTime = Distance * 0.00001f;

				if (waitTime <= KINDA_SMALL_NUMBER)
				{
					DelayedNoises.Reset();

					UBOOL bIsMajorNoise = TargetPlayer != NULL && (NoiseType == TargetPlayer->RunningNoise || NoiseType == TargetPlayer->DoorMajorNoise);

					TriggerAudioDisturbance(NoiseMakerHero->GetDisturbanceLocation(EnemyPawn), FALSE, bIsMajorNoise, bIsMajorNoise && bDisturbed, bIsMajorNoise && bDisturbed);
				}
				else
				{
					INT Idx = 0;
					FDelayedNoise* Noise = NULL;
					if (DelayedNoises.Num() > 0)
					{
						Noise = &DelayedNoises(Idx);
						while(Noise != NULL && Noise->TimeToNoise < waitTime)
						{
							++Idx;
							if (Idx < DelayedNoises.Num())
							{
								Noise = &DelayedNoises(Idx);
							}
							else
							{
								Noise = NULL;
							}
						}

						if (Noise)
						{
							DelayedNoises.Remove(Idx, DelayedNoises.Num() - Idx);
							Noise = NULL;
						}

					}

					Idx = DelayedNoises.AddZeroed();
					Noise = &DelayedNoises(Idx);

					Noise->Location = NoiseMakerHero->GetDisturbanceLocation(EnemyPawn);
					Noise->NoiseType = NoiseType;
					Noise->TimeToNoise = waitTime;
				}
			}
		}
	}
}

////////////////////////////////////////////////////////////////////////////////////////////
/// Animation/SpecialMoves
////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////

void AOLBot::WaitForFullBodyAnim()
{
	GetStateFrame()->LatentAction = OLBotAI_PollWaitForFullBodyAnim;
}

void AOLBot::execPollWaitForFullBodyAnim( FFrame& Stack, RESULT_DECL )
{
	if( !EnemyPawn->bIsAnimatingFullBody )
	{
		GetStateFrame()->LatentAction = 0;
	}
}
IMPLEMENT_FUNCTION( AOLBot, OLBotAI_PollWaitForFullBodyAnim, execPollWaitForFullBodyAnim );

void AOLBot::WaitForSpecialMove(UBOOL bNoDelay)
{
	GetStateFrame()->LatentAction = bNoDelay ? OLBotAI_PollWaitForSpecialMoveNoDelay : OLBotAI_PollWaitForSpecialMove;
}

void AOLBot::execPollWaitForSpecialMove( FFrame& Stack, RESULT_DECL )
{
	if( EnemyPawn->SpecialMove == SMT_None )
	{
		GetStateFrame()->LatentAction = 0;
	}
}
IMPLEMENT_FUNCTION( AOLBot, OLBotAI_PollWaitForSpecialMove, execPollWaitForSpecialMove );

void AOLBot::execPollWaitForSpecialMoveNoDelay( FFrame& Stack, RESULT_DECL )
{
	if (EnemyPawn->TryCompleteSpecialMove())
	{
		GetStateFrame()->LatentAction = 0;
	}
}
IMPLEMENT_FUNCTION( AOLBot, OLBotAI_PollWaitForSpecialMoveNoDelay, execPollWaitForSpecialMoveNoDelay );

UBOOL AOLBot::IsPerformingMoveAbility()
{
	UBOOL bMoveInterruptible = EnemyPawn->IsDoingSpecialMove() && EnemyPawn->CanInterruptSpecialMove();

	UGameAICommand* MoveCommand = FindCommandOfClass(UOLAICmd_MoveAbility::StaticClass());
	return !(MoveCommand == NULL || MoveCommand->IsInState(FName(TEXT("Approaching"), FNAME_Find)) || MoveCommand->IsInState(FName(TEXT("Waiting"), FNAME_Find)) || bMoveInterruptible);
}

void AOLBot::UpdateAnimationMode()
{
	if (EnemyPawn->EnemyMode != EM_SqueezeGrabLeft
		&& EnemyPawn->EnemyMode != EM_SqueezeGrabRight)
	{
		switch(BehaviorState)
		{
		case AIBS_Investigating:
			EnemyPawn->EnemyMode = EM_Investigate;
			break;
		case AIBS_Chasing:
			EnemyPawn->EnemyMode = EM_Chase;
			break;
		case AIBS_Idle: // Fall through
		case AIBS_Patrolling: // Fall through
		default:
			EnemyPawn->EnemyMode = PatrolMode;
			break;
		}
	}
}

////////////////////////////////////////////////////////////////////////////////////////////
/// Attacking
////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////

UBOOL AOLBot::CheckAttackZDiff(FVector TestLocation)
{
	return TestLocation.Z < (TargetPlayer->Location.Z + TargetPlayer->CylinderComponent->Translation.Z + TargetPlayer->CylinderComponent->CollisionHeight)
		&& TargetPlayer->Location.Z < (TestLocation.Z + 250.0f);
}

UBOOL AOLBot::IsInAttackRange()
{
	UBOOL ZHit = CheckAttackZDiff(EnemyPawn->Location);

	return ZHit && (EnemyPawn->Location - TargetPlayer->Location).SizeSquared2D() <= Square(EnemyPawn->AttackRange);
}

UBOOL AOLBot::IsInApproachAttackRange()
{
	if (!EnemyPawn || !TargetPlayer) return FALSE;
	TWEAKABLE FLOAT ExtraBuf = 1.1f;
	UBOOL ZHit = CheckAttackZDiff(EnemyPawn->Location);
	return ZHit && (EnemyPawn->Location - TargetPlayer->Location).SizeSquared2D() <= Square(ExtraBuf*EnemyPawn->AttackRange);
}


UBOOL AOLBot::IsInFinalAttackRange()
{
	if (!EnemyPawn || !TargetPlayer) return FALSE;
	TWEAKABLE FLOAT ExtraBuf = 1.25f;
	UBOOL ZHit = CheckAttackZDiff(EnemyPawn->Location);
	return ZHit && (EnemyPawn->Location - TargetPlayer->Location).SizeSquared2D() <= Square(ExtraBuf*EnemyPawn->AttackRange);
}

UBOOL AOLBot::IsInDamageRange()
{
	if (!EnemyPawn || !TargetPlayer)
		return FALSE;

	FVector CheckEnemyLocation = EnemyPawn->Location;

	if (EnemyPawn->IsA(AOLEnemyNanoCloud::StaticClass()))
	{
		CheckEnemyLocation = EnemyPawn->Mesh->GetBoneLocation(EnemyPawn->HipBone);
		CheckEnemyLocation.Z = EnemyPawn->Location.Z;
	}

	if (!TargetPlayer->CylinderComponent)
		return FALSE;

	UBOOL ZHit = CheckEnemyLocation.Z < (TargetPlayer->Location.Z + TargetPlayer->CylinderComponent->Translation.Z + TargetPlayer->CylinderComponent->CollisionHeight * 2.0f)
		&& TargetPlayer->Location.Z < (CheckEnemyLocation.Z + 250.0f);


	return (CheckEnemyLocation - TargetPlayer->Location).SizeSquared2D() <= EnemyPawn->AttackRange*EnemyPawn->AttackRange
		&& ZHit;
}

UBOOL AOLBot::IsPlayerOnBrokenSideOfDoor(AOLDoor* door)
{
	if (!door || !TargetPlayer)
	{
		return FALSE;
	}

	FVector doorBrokenSide = door->GetStaticDirection();
	
	if ((EnemyPawn->Rotation.Vector() | doorBrokenSide) < 0.0f)
	{
		doorBrokenSide = -doorBrokenSide; // bashing on the other (not natural) direction
	}

	FVector centerToHero = Vec2D(TargetPlayer->Location - door->GetCenterLocation());
	FLOAT distInFront = centerToHero | doorBrokenSide;
	FLOAT distOnSide = centerToHero | door->GetStaticPivotToEdge();

	return (distInFront > 0.0f && distInFront < 150.0f) && (Abs(distOnSide) < 100.0f);
}

UBOOL AOLBot::IsPlayerOnVaultingPath()
{
	if (!TargetPlayer)
	{
		return FALSE;
	}

	FVector vaultingDir = EnemyPawn->Rotation.Vector();

	FVector toHero = Vec2D(TargetPlayer->Location - EnemyPawn->Location);
	FLOAT distInFront = toHero | vaultingDir;
	FLOAT distOnSide = toHero | EnemyPawn->Rotation.Right();

	return (distInFront > 0.0f && distInFront < 150.0f) && (Abs(distOnSide) < 100.0f);
}

UBOOL AOLBot::PerformGrabCheck()
{
	TWEAKABLE FLOAT GrabZDiffThreshold = 15.0f;

	FLOAT ZDiff = Abs(EnemyPawn->Location.Z - TargetPlayer->Location.Z);

	if (ZDiff > GrabZDiffThreshold)
	{
		return FALSE;
	}

	FVector StartTrace = EnemyPawn->Location + FVector(0.f, 0.f, EnemyPawn->CylinderComponent->CollisionHeight + 5.0f);
	FVector EndTrace = TargetPlayer->Location + FVector(0.f, 0.f, EnemyPawn->CylinderComponent->CollisionHeight + 5.0f);

	UBOOL bSuccess = TRUE;
	FCheckResult Hit(1.f);
	if (!GWorld->SingleLineCheck(Hit, EnemyPawn, EndTrace, StartTrace, TRACE_AllBlocking, FVector(EnemyPawn->CylinderComponent->CollisionRadius, EnemyPawn->CylinderComponent->CollisionRadius, EnemyPawn->CylinderComponent->CollisionHeight)))
	{
		if (Hit.Actor != TargetPlayer)
		{
			bSuccess = FALSE;
		}
	}

	return bSuccess;
}

UBOOL AOLBot::PerformAttackCheck()
{
	TWEAKABLE FLOAT CollisionHeightMultiplier = 0.5f;

	FLOAT ZBottom = Max(EnemyPawn->Location.Z, TargetPlayer->Location.Z);
	FLOAT ZTop = Min(EnemyPawn->Location.Z + EnemyPawn->CylinderComponent->Translation.Z + EnemyPawn->CylinderComponent->CollisionHeight,
					TargetPlayer->Location.Z + TargetPlayer->CylinderComponent->Translation.Z + TargetPlayer->CylinderComponent->CollisionHeight);

	FLOAT CheckHeight = 0.f;
	FLOAT CheckZ = ZTop;
	if (ZBottom < ZTop)
	{
		CheckHeight = (ZTop - ZBottom) / 2.f;
		CheckZ = ZTop - CheckHeight;
	}
	else
	{
		CheckZ += 10.0f;
	}
	
	FVector StartTrace = EnemyPawn->Location;
	StartTrace.Z = CheckZ;
	FVector EndTrace = TargetPlayer->Location;
	EndTrace.Z = CheckZ;

	CheckHeight *= CollisionHeightMultiplier;

	UBOOL bSuccess = TRUE;
	FCheckResult Hit(1.f);
	if (!GWorld->SingleLineCheck(Hit, EnemyPawn, EndTrace, StartTrace, TRACE_AllBlocking, CheckHeight <= 0.f ? FVector(0.f) : FVector(5.0f, 5.0f, CheckHeight)))
	{
		if (Hit.Actor != TargetPlayer)
		{
			bSuccess = FALSE;
		}
	}

	return bSuccess;
}

UBOOL AOLBot::PerformThrowPositionCheck(const FVector& ThrowStartPlayerPosition)
{
	FVector CompTranslation = TargetPlayer->DefaultPawn->CylinderComponent->Translation;
	FVector Extent = TargetPlayer->DefaultPawn->GetCylinderExtent();

	FVector StartTrace = EnemyPawn->Location + CompTranslation;
	FVector EndTrace = ThrowStartPlayerPosition + CompTranslation + FVector(0.f, 0.f, EnemyPawn->ThrowStartPlayerZOffset);

	UBOOL bSuccess = TRUE;
	FMemMark CheckMark(GMainThreadMemStack);
	FCheckResult* CheckHits = GWorld->MultiLineCheck(GMainThreadMemStack, EndTrace, StartTrace, Extent, TRACE_AllBlocking, EnemyPawn);
	for( FCheckResult* Hit = CheckHits; Hit; Hit = Hit->GetNext())
	{
		if (Hit->Actor != TargetPlayer)
		{
			bSuccess = FALSE;
			break;
		}
	}
	CheckMark.Pop();

	return bSuccess;
}

void AOLBot::GetClosestSqueezeLocationAndRotation(const AOLSqueezeVolume* Squeeze, FVector& SqueezeLocation, FVector& SqueezeRotation, UBOOL& bRight)
{
	FVector CloseNode;
	FVector FarNode;
	if (EnemyPawn->Location.DistanceSquared(Squeeze->Node1->Location) < EnemyPawn->Location.DistanceSquared(Squeeze->Node2->Location))
	{
		CloseNode = Squeeze->Node1->Location;
		FarNode = Squeeze->Node2->Location;
	}
	else
	{
		CloseNode = Squeeze->Node2->Location;
		FarNode = Squeeze->Node1->Location;
	}

	FVector SqueezeDir = (FarNode - CloseNode).SafeNormal2D();
	FVector LeftDir = SqueezeDir.RotateAngleAxis(-HALF_PI*RAD_TO_UNR, FVector(0.f,0.f,1.f));

	FVector StartTrace = CloseNode - 45.0f*SqueezeDir + FVector(0, 0, 100.0f);
	FVector EndTrace = StartTrace + 100.0f*LeftDir;

	FCheckResult Hit(1.f);
	if (GWorld->SingleLineCheck(Hit, this, EndTrace, StartTrace, TRACE_AllBlocking | TRACE_StopAtAnyHit))
	{
		bRight = TRUE;
	}
	else
	{
		bRight = FALSE;
	}

	FVector SqueezeOffset = EnemyPawn->SqueezeAttackOffset;
	if (!bRight)
	{
		SqueezeOffset.Y *= -1.f;
	}

	SqueezeLocation = CloseNode - SqueezeOffset.RotateAngleAxis(SqueezeDir.Rotation().Yaw, FVector(0.f, 0.f, 1.f));
	SqueezeLocation.Z -= DistToGround(SqueezeLocation + VecZ(30.0f), FVector(5.f, 5.f, 30.0f));

	SqueezeRotation = SqueezeDir;
}

FVector AOLBot::GetSqueezePoint(AOLSqueezeVolume* Squeeze)
{
	FVector SqueezeLocation;
	FVector SqueezeRotation;
	UBOOL bRight;
	GetClosestSqueezeLocationAndRotation(Squeeze, SqueezeLocation, SqueezeRotation, bRight);

	return SqueezeLocation;
}

UBOOL AOLBot::AttackTarget(BYTE aType)
{
	if (TargetPlayer == NULL || TargetPlayer->IsPendingKill() || !TargetPlayer->CanBeAttacked() || IsPerformingMoveAbility() || IsInState(InterruptionState) || bInvestigatingObject)
	{
		return FALSE;
	}

	if (Group != NULL)
	{
		if (!Group->CanAttack(this))
		{
			return FALSE;
		}
	}

	if (AttackTimer > 0.f)
	{
		return FALSE;
	}

	if (bAttacking)
	{
		return FALSE;
	}

	UBOOL Success = TRUE;
	FName NewState;
	switch (aType)
	{
	case AT_Normal:
		if(InAttackRange 
			&& SightComponent->CanSeeTarget
			//&& !TargetPlayer->CanBeGrabbedUnder()
			&& PerformAttackCheck())
		{
			if (TargetPlayer->PreciseHealth <= EnemyPawn->AttackNormalDamage && TargetPlayer->CanBeFatalitized() /* && EnemyPawn->Modifiers.bUseKillingBlow*/)
			{
				FVector EnemyToPlayer = (TargetPlayer->Location - EnemyPawn->Location).SafeNormal2D();

				bKilling = TRUE;

				if (EnemyPawn->IsA(AOLEnemyGenericPatient::StaticClass()) || EnemyPawn->IsA(AOLEnemySurgeon::StaticClass()) || EnemyPawn->IsA(AOLEnemyNanoCloud::StaticClass()))
				{
					FVector PlayerFacing = TargetPlayer->Rotation.Vector().SafeNormal2D();
					UBOOL IsPlayerFacingEnemy = (EnemyToPlayer | PlayerFacing) < 0.f;

					if (EnemyPawn->IsA(AOLEnemyNanoCloud::StaticClass()))
					{
						if (IsPlayerFacingEnemy)
						{
							NewState = FName( TEXT("AttackKill"), FNAME_Find);

							AttackStartLocation = Location;
							AttackStartRotation = EnemyToPlayer;

							EnemyPawn->bBackAnim = FALSE;
						}
						else
						{
							NewState = FName( TEXT("AttackKill"), FNAME_Find);

							AttackStartLocation = TargetPlayer->Location - EnemyPawn->SwarmXplodBackOffset*TargetPlayer->CharForward;
							AttackStartRotation = TargetPlayer->CharForward;

							EnemyPawn->bBackAnim = TRUE;
						}
					}
					else if (EnemyPawn->bUsingWeapon)
					{
						if (IsPlayerFacingEnemy)
						{
							NewState = FName( TEXT("AttackKill"), FNAME_Find);

							AttackStartLocation = Location;
							AttackStartRotation = EnemyToPlayer;

							EnemyPawn->bBackAnim = FALSE;
						}
						else
						{
							NewState = FName( TEXT("AttackKill"), FNAME_Find);

							AttackStartLocation = Location;
							AttackStartRotation = EnemyToPlayer;

							EnemyPawn->bBackAnim = TRUE;
						}
					}
					else
					{
						if (IsPlayerFacingEnemy)
						{
							NewState = FName( TEXT("AttackKill"), FNAME_Find);

							AttackStartLocation = Location;
							AttackStartRotation = EnemyToPlayer;

							EnemyPawn->bBackAnim = FALSE;
						}
						else
						{
							NewState = FName( TEXT("AttackNormal"), FNAME_Find);

							AttackStartLocation = Location;
							AttackStartRotation = EnemyToPlayer;
						}
					}
				}
				else if (EnemyPawn->bCanThrow && PerformGrabCheck())
				{
					NewState = FName( TEXT("AttackGrab"), FNAME_Find);

					FVector TargetToEnemy = (EnemyPawn->Location - TargetPlayer->Location).SafeNormal2D();
					FVector TargetStart = EnemyPawn->Location - TargetToEnemy * 150.0f;

					FVector StartTrace = TargetPlayer->Location + TargetPlayer->CylinderComponent->Translation;
					FVector EndTrace = TargetStart + TargetPlayer->CylinderComponent->Translation;

					FCheckResult Hit(1.f);
					if (!GWorld->SingleLineCheck(Hit, this, EndTrace, StartTrace, TRACE_World, TargetPlayer->GetCylinderExtent()))
					{
						GrabTargetStartLocation = Hit.Location - TargetPlayer->CylinderComponent->Translation;
						AttackStartLocation = EnemyPawn->Location + (TargetToEnemy) * GrabTargetStartLocation.Distance(TargetStart);
					}
					else
					{
						GrabTargetStartLocation = TargetStart;
						AttackStartLocation = EnemyPawn->Location;
					}

					AttackStartRotation = (GrabTargetStartLocation - AttackStartLocation).SafeNormal2D();
					
					if (!PerformThrowPositionCheck(GrabTargetStartLocation) || !CalculateThrowDirection(GrabTargetStartLocation - AttackStartRotation * EnemyPawn->ThrowStartPlayerDistance, AttackStartRotation))
					{
						NewState = FName( TEXT("AttackNormal"), FNAME_Find);

						AttackStartLocation = Location;
						AttackStartRotation = EnemyToPlayer;
					}
				}
				else
				{
					NewState = FName( TEXT("AttackNormal"), FNAME_Find);

					AttackStartLocation = Location;
					AttackStartRotation = EnemyToPlayer;
				}
			}
			else
			{
				NewState = FName( TEXT("AttackNormal"), FNAME_Find);

				FVector ToPlayer = (TargetPlayer->Location - EnemyPawn->Location).SafeNormal2D();
				FVector NormVel = TargetPlayer->Velocity.SafeNormal2D();

				if (EnemyPawn->bAttackUseQuickAttack
					&& TargetPlayer->Velocity.SizeSquared2D() > Square(EnemyPawn->AttackQuickSpeedThreshold)  // Speed
					&& (ToPlayer | NormVel) < -1.0 * appCos(EnemyPawn->AttackQuickAngleThreshold * DEG_TO_RAD) // Angle
					&& (EnemyPawn->Rotation.Vector() | ToPlayer) > 0.f // In Front of Enemy
					&& (TargetPlayer->Location - EnemyPawn->Location).SizeSquared2D() <= EnemyPawn->AttackQuickDistanceThreshold) // Distance
				{
					bUseQuickAttack = TRUE;
				}

				AttackStartRotation = (TargetPlayer->Location - EnemyPawn->Location).SafeNormal2D();
			}
		}
		else
		{
			Success = FALSE;
		}
		break;
	case AT_Squeeze:
		if (TargetPlayer->LocomotionMode == LM_Squeeze || SightComponent->bSawPlayerInSqueeze)
		{
			FVector CloseNode;
			FVector FarNode;
			if (EnemyPawn->Location.DistanceSquared(SightComponent->LastSqueeze->Node1->Location) < EnemyPawn->Location.DistanceSquared(SightComponent->LastSqueeze->Node2->Location))
			{
				CloseNode = SightComponent->LastSqueeze->Node1->Location;
				FarNode = SightComponent->LastSqueeze->Node2->Location;
			}
			else
			{
				CloseNode = SightComponent->LastSqueeze->Node2->Location;
				FarNode = SightComponent->LastSqueeze->Node1->Location;
			}

			if ((EnemyPawn->Location.DistanceSquared(CloseNode) < EnemyPawn->AttackSqueezeVisibleRangeToNode * EnemyPawn->AttackSqueezeVisibleRangeToNode && (SightComponent->CanSeeTarget || SightComponent->bSawPlayerInSqueeze))  || (EnemyPawn->Location.DistanceSquared(TargetPlayer->Location) < EnemyPawn->AttackSqueezeHiddenRangeToTarget*EnemyPawn->AttackSqueezeHiddenRangeToTarget))
			{
				NewState = FName( TEXT("AttackSqueeze"), FNAME_Find);

				UBOOL bRight = FALSE;
				GetClosestSqueezeLocationAndRotation(SightComponent->LastSqueeze, AttackStartLocation, AttackStartRotation, bRight);
				bAttackRight = bRight;

				CalculateThrowDirection(AttackStartLocation, -AttackStartRotation);

				if (!PerformThrowPositionCheck(AttackStartLocation - AttackStartRotation * EnemyPawn->ThrowStartPlayerDistance))
				{
					Success = FALSE;
				}
				else if (TargetPlayer->PreciseHealth <= EnemyPawn->AttackThrowDamage)
				{
					bKilling = TRUE;
				}
			}
			else
			{
				Success = FALSE;
			}
		}
		else
		{
			Success = FALSE;
		}
		break;
	case AT_Locker:
		if (SightComponent->CanSeeTarget && TargetPlayer->LocomotionMode == LM_Locker || TargetPlayer->SpecialMove == SMT_EnterLocker)
		{
			NewState = FName( TEXT("AttackLocker"), FNAME_Find);
						
			AttackStartLocation = GetLockerDestination(TargetPlayer->ActiveLocker);
			AttackStartLocation.Z = EnemyPawn->Location.Z;
			AttackStartRotation = TargetPlayer->ActiveLocker->AssociatedDoor->GetStaticDirection()*-1.f;
			
			if (EnemyPawn->bCanThrow)
			{
				CalculateThrowDirection(AttackStartLocation, -AttackStartRotation);
			}

			if (EnemyPawn->bCanThrow && !PerformThrowPositionCheck(AttackStartLocation - AttackStartRotation * EnemyPawn->ThrowStartPlayerDistance))
			{
				Success = FALSE;
			}
			else if (TargetPlayer->PreciseHealth <= EnemyPawn->AttackThrowDamage)
			{
				bKilling = TRUE;
			}
		}
		else
		{
			Success = FALSE;
		}
		break;
	case AT_Bed:
		if (SightComponent->CanSeeTarget && (TargetPlayer->LocomotionMode == LM_Bed || TargetPlayer->SpecialMove == SMT_EnterBed))
		{
			NewState = FName( TEXT("AttackBed"), FNAME_Find);

			AOLBed* Bed = TargetPlayer->ActiveBed;

			if (Bed != NULL)
			{
				AttackStartLocation = GetBestBedDestination(Bed, CurrentBedSide);
				AttackStartRotation = (Bed->Location - Bed->LocalToWorld().TransformFVector(FVector(0.f, CurrentBedSide ? 1.f : -1.f, 0.f))).SafeNormal2D();

				if (EnemyPawn->bCanThrow)
				{
					CalculateThrowDirection(AttackStartLocation, -AttackStartRotation, CurrentBedSide == 0 ? Bed->ThrowOffsetLeft : Bed->ThrowOffsetRight, TRUE, CurrentBedSide == 0 ? FALSE : TRUE );
				}

				if (EnemyPawn->bCanThrow && !PerformThrowPositionCheck(AttackStartLocation - AttackStartRotation * EnemyPawn->ThrowStartPlayerDistance))
				{
					Success = FALSE;
				}
				else if (TargetPlayer->PreciseHealth <= EnemyPawn->AttackThrowDamage)
				{
					bKilling = TRUE;
				}
			}
			else
			{
				Success = FALSE;
			}
		}
		else
		{
			Success = FALSE;
		}
		break;
	case AT_Grab:
	case AT_GrabCrouch:
		{
			UBOOL bCrouchGrab = (aType == AT_GrabCrouch);

			if (SightComponent->CanSeeTarget 
				&& InAttackRange
				&& TargetPlayer->Velocity.Size2D() < EnemyPawn->GrabTargetVelocity
				&& TargetPlayer->IsInGrabbableState()
				&& (!bCrouchGrab || TargetPlayer->bIsCrouched)
				&& PerformGrabCheck())
			{
				NewState = bCrouchGrab ? FName( TEXT("AttackCrouch"), FNAME_Find) : FName( TEXT("AttackGrab"), FNAME_Find);

				FVector TargetToEnemy = (EnemyPawn->Location - TargetPlayer->Location).SafeNormal2D();
				FVector TargetStart = EnemyPawn->Location - TargetToEnemy * 150.0f;

				FVector StartTrace = TargetPlayer->Location + TargetPlayer->CylinderComponent->Translation;
				FVector EndTrace = TargetStart + TargetPlayer->CylinderComponent->Translation;

				FCheckResult Hit(1.f);
				if (!GWorld->SingleLineCheck(Hit, this, EndTrace, StartTrace, TRACE_World, TargetPlayer->GetCylinderExtent()))
				{
					GrabTargetStartLocation = Hit.Location - TargetPlayer->CylinderComponent->Translation;
					AttackStartLocation = EnemyPawn->Location + (TargetToEnemy) * GrabTargetStartLocation.Distance(TargetStart);
				}
				else
				{
					GrabTargetStartLocation = TargetStart;
					AttackStartLocation = EnemyPawn->Location;
				}

				AttackStartRotation = (GrabTargetStartLocation - AttackStartLocation).SafeNormal2D();

				FLOAT GrabRand = appFrand();

				if (!PerformThrowPositionCheck(GrabTargetStartLocation))
				{
					Success = FALSE;
				}
				else if (!CalculateThrowDirection(GrabTargetStartLocation - AttackStartRotation * EnemyPawn->ThrowStartPlayerDistance, AttackStartRotation))
				{
					Success = FALSE;
				}
				else  if (TargetPlayer->PreciseHealth <= EnemyPawn->AttackThrowDamage)
				{
					bKilling = TRUE;
				}
				else if (GrabRand > EnemyPawn->AttackGrabChance)
				{
					Success = FALSE;
				}
			}
			else
			{
				Success = FALSE;
			}
		}
		break;
	case AT_GrabUnder:
		if (SightComponent->CanSeeTarget 
			&& InAttackRange
			//&& TargetPlayer->Velocity.Size2D() < EnemyPawn->GrabTargetVelocity
			&& TargetPlayer->CanBeGrabbedUnder())
		{
			FVector Extent(10.0f, 10.0f, 35.0f);

			FVector StartTrace = TargetPlayer->Location + VecZ(Extent.Z + 5.0f);
			FVector EndTrace = EnemyPawn->Location + VecZ(Extent.Z + 5.0f);

			UBOOL CheckSuccess = TRUE;
			FCheckResult Hit(1.f);
			if (!GWorld->SingleLineCheck(Hit, EnemyPawn, EndTrace, StartTrace, TRACE_AllBlocking, Extent))
			{
				if (Hit.Actor != TargetPlayer)
				{
					CheckSuccess = FALSE;
				}
			}

			if (CheckSuccess)
			{
				NewState = FName( TEXT("AttackGrabUnder"), FNAME_Find);

				FVector TargetToEnemy = (EnemyPawn->Location - TargetPlayer->Location);
				FVector TargetToEnemyNrm = TargetToEnemy.SafeNormal2D();
				FLOAT TargetToEnemyLength = TargetToEnemy.Size2D();

				FVector TargetStart;
				if (TargetToEnemyLength < 100.0f)
				{
					TargetStart = TargetPlayer->Location;
					AttackStartLocation = TargetPlayer->Location + TargetToEnemyNrm * 100.0f; 
				}
				else
				{
					TargetStart = EnemyPawn->Location - TargetToEnemyNrm * 100.0f;
					AttackStartLocation = EnemyPawn->Location; 
				}

				GrabTargetStartLocation = TargetStart;
				AttackStartRotation = -TargetToEnemyNrm;

				if (EnemyPawn->bCanThrow)
				{
					CalculateThrowDirection(AttackStartLocation, TargetToEnemyNrm);
				}

				if (EnemyPawn->bCanThrow && !PerformThrowPositionCheck(AttackStartLocation - AttackStartRotation * EnemyPawn->ThrowStartPlayerDistance))
				{
					Success = FALSE;
				}
				else if (TargetPlayer->PreciseHealth <= EnemyPawn->AttackThrowDamage)
				{
					bKilling = TRUE;
				}

			}
			else
			{
				Success = FALSE;
			}
		}
		else
		{
			Success = FALSE;
		}
		break;
	case AT_Push:
		if (SightComponent->CanSeeTarget
			&& InAttackRange
			&& PerformAttackCheck())
		{
			FVector ToTarget = (TargetPlayer->Location - Location).SafeNormal2D();

			FLOAT AngleToTarget = UNR_TO_DEG * FRotator::NormalizeAxis(Rotation.Yaw - ToTarget.Rotation().Yaw);
			
			if (Abs(AngleToTarget) <= 90.0f)
			{
				NewState = FName( TEXT("AttackPush"), FNAME_Find);
			}
			else
			{
				Success = FALSE;
			}
		}
		else
		{
			Success = FALSE;
		}
		break;
	}

	if (Success && NewState != NAME_None)
	{
		SetBehaviorState(AIBS_Chasing);

		ClearInvestigation();

		if (EnemyPawn->SpecialMove == SMT_Disturbed)
		{
			EnemyPawn->CancelSpecialMove();
		}

		CurrentAttackType = aType;

		EGotoState ResultState = GOTOSTATE_Success;
		ResultState = GotoState( NewState );
		if ( ResultState == GOTOSTATE_Success )
		{
			GotoLabel( FName(NAME_Begin) );
		}
	}

	return Success;
}

UBOOL AOLBot::TryGrabNormal(UBOOL bCrouched)
{
	FVector TargetToEnemy = (EnemyPawn->Location - TargetPlayer->Location).SafeNormal2D();
	FVector TargetStart = EnemyPawn->Location - TargetToEnemy * 150.0f;

	FVector StartTrace = TargetPlayer->Location + TargetPlayer->CylinderComponent->Translation;
	FVector EndTrace = TargetStart + TargetPlayer->CylinderComponent->Translation;

	FCheckResult Hit(1.f);
	if (!GWorld->SingleLineCheck(Hit, this, EndTrace, StartTrace, TRACE_World, TargetPlayer->GetCylinderExtent()))
	{
		GrabTargetStartLocation = Hit.Location - TargetPlayer->CylinderComponent->Translation;
		AttackStartLocation = EnemyPawn->Location + (TargetToEnemy) * GrabTargetStartLocation.Distance(TargetStart);
	}
	else
	{
		GrabTargetStartLocation = TargetStart;
		AttackStartLocation = EnemyPawn->Location;
	}

	AttackStartRotation = (GrabTargetStartLocation - AttackStartLocation).SafeNormal2D();

	if (!TargetPlayer->bIsDummyPawn)
	{
		if (!PerformThrowPositionCheck(GrabTargetStartLocation))
		{
			return FALSE;
		}
		else if (!CalculateThrowDirection(GrabTargetStartLocation - AttackStartRotation * EnemyPawn->ThrowStartPlayerDistance, AttackStartRotation))
		{
			return FALSE;
		}
	}

	if (TargetPlayer->bIsDummyPawn)
	{
		// Mirror TryGrabNormal: pass CharForward (victim's facing) as targetDirection so
		// ActivatePositionAdjustment on the client computes headingError correctly.
		// bLeftAnim/blendAlpha mirror TryGrabNormal's lookAngleToAttacker computation.
		FVector toEnemy = (AttackStartLocation - GrabTargetStartLocation).SafeNormal2D();
		FVector charFwd = TargetPlayer->Rotation.Vector(); charFwd.Z = 0.f; charFwd = charFwd.SafeNormal();
		FLOAT lookAngle = UNR_TO_DEG * FRotator::NormalizeAxis(TargetPlayer->EyeRotation.Yaw - toEnemy.Rotation().Yaw);
		FLOAT blendAlpha = Clamp(Abs(lookAngle / 180.0f), 0.0f, 1.0f);
		UBOOL bLeftAnimFlag = lookAngle > 0.0f;
		// GrabType=0 (Normal)
		eventNotifyDummyGrab(GrabTargetStartLocation, charFwd, bCrouched, blendAlpha, bLeftAnimFlag, 0);
	}
	else if (!TargetPlayer->TryGrabNormal(EnemyPawn, GrabTargetStartLocation, AttackStartLocation))
	{
		return FALSE;
	}

	EnemyPawn->StartSpecialMove(bCrouched ? SMT_AttackCrouch : SMT_AttackGrab, AttackStartLocation, AttackStartRotation, APTT_TargetAtStart);
	
	return TRUE;
}

UBOOL AOLBot::TryGrabUnder()
{
	FVector Extent(10.0f, 10.0f, 35.0f);

	FVector StartTrace = TargetPlayer->Location + VecZ(Extent.Z + 5.0f);
	FVector EndTrace = EnemyPawn->Location + VecZ(Extent.Z + 5.0f);

	UBOOL CheckSuccess = TRUE;
	FCheckResult Hit(1.f);
	if (!GWorld->SingleLineCheck(Hit, EnemyPawn, EndTrace, StartTrace, TRACE_AllBlocking, Extent))
	{
		if (Hit.Actor != TargetPlayer)
		{
			CheckSuccess = FALSE;
		}
	}

	if (CheckSuccess)
	{
		FVector TargetToEnemy = (EnemyPawn->Location - TargetPlayer->Location);
		FVector TargetToEnemyNrm = TargetToEnemy.SafeNormal2D();
		FLOAT TargetToEnemyLength = TargetToEnemy.Size2D();

		FVector TargetStart;
		if (TargetToEnemyLength < 100.0f)
		{
			TargetStart = TargetPlayer->Location;
			AttackStartLocation = TargetPlayer->Location + TargetToEnemyNrm * 100.0f; 
		}
		else
		{
			TargetStart = EnemyPawn->Location - TargetToEnemyNrm * 100.0f;
			AttackStartLocation = EnemyPawn->Location; 
		}

		GrabTargetStartLocation = TargetStart;
		AttackStartRotation = (GrabTargetStartLocation - AttackStartLocation).SafeNormal2D();

		CalculateThrowDirection(AttackStartLocation, -AttackStartRotation);

		if (!Utils::IsPlayingDLC() || !EnemyPawn->IsA(AOLEnemyCannibal::StaticClass())) // never fail if cannibal - too many tight spaces where you could get stuck
		{
			if (!PerformThrowPositionCheck(AttackStartLocation - AttackStartRotation * EnemyPawn->ThrowStartPlayerDistance))
			{
				return FALSE;
			}
		}
	}
	else
	{
		return FALSE;
	}

	if (TargetPlayer->bIsDummyPawn)
	{
		FVector toEnemy = (AttackStartLocation - GrabTargetStartLocation).SafeNormal2D();
		FVector charFwd = TargetPlayer->Rotation.Vector(); charFwd.Z = 0.f; charFwd = charFwd.SafeNormal();
		FLOAT lookAngle = UNR_TO_DEG * FRotator::NormalizeAxis(TargetPlayer->EyeRotation.Yaw - toEnemy.Rotation().Yaw);
		FLOAT blendAlpha = Saturate(Abs(lookAngle / 180.0f));
		UBOOL bLeftAnimFlag = lookAngle > 0.0f;
		// GrabType=4 (Under)
		eventNotifyDummyGrab(GrabTargetStartLocation, charFwd, FALSE, blendAlpha, bLeftAnimFlag, 4);
	}
	else if (!TargetPlayer->TryGrabFromUnder(EnemyPawn, GrabTargetStartLocation, AttackStartLocation))
	{
		return FALSE;
	}

	EnemyPawn->StartSpecialMove(SMT_AttackGrabUnder, AttackStartLocation, AttackStartRotation, APTT_TargetAtStart);

	return TRUE;
}

UBOOL AOLBot::CalculateThrowDirection(const FVector& ThrowStartLocation, const FVector& ThrowStartDirection, FLOAT ThrowOffset /*= 0.f*/, UBOOL bForceDirection /*= FALSE*/, UBOOL bForceClockwise /*= TRUE*/)
{
	static const FLOAT THROW_DISTANCE = 280.0f;
	static const INT NUM_DIRECTIONS = 8;
	static const FLOAT DIR_INCREMENT = 360.0f / NUM_DIRECTIONS;
	
	if (bDebugThrowCalculations)
	{
		FlushPersistentDebugLines();
	}

	UBOOL bCheckClockwise;
	if (bForceDirection)
	{
		bCheckClockwise = bForceClockwise;
	}
	else
	{
		bCheckClockwise = (appFrand() >= 0.5f);
	}

	FLOAT DirectionIncrement = (bCheckClockwise ? 1.0f : -1.0f) * DIR_INCREMENT;
	FVector Direction = ThrowStartDirection.SafeNormal2D();

	Direction = Direction.RotateAngleAxis(ThrowOffset * DEG_TO_UNR, FVector(0.f, 0.f, 1.f));

	FLOAT ThrowRotation = ThrowOffset;

	UBOOL bPassTwo = FALSE;

	for (INT i = 0; i < NUM_DIRECTIONS; )
	{
		FVector StartPoint = ThrowStartLocation + TargetPlayer->CylinderComponent->Translation;
		StartPoint.Z += EnemyPawn->ThrowStartPlayerZOffset;
		FVector EndPoint = StartPoint + Direction*THROW_DISTANCE;

		FVector Extent = TargetPlayer->DefaultPawn->GetCylinderExtent();

		UBOOL bSuccess = TRUE;
		FMemMark CheckMark(GMainThreadMemStack);
		FCheckResult* CheckHits = GWorld->MultiLineCheck(GMainThreadMemStack, EndPoint, StartPoint, Extent, TRACE_AllBlocking, EnemyPawn);
		for( FCheckResult* Hit = CheckHits; Hit; Hit = Hit->GetNext())
		{
			if (Hit->Actor != TargetPlayer)
			{
				bSuccess = FALSE;
				break;
			}
		}
		CheckMark.Pop();

		if (bSuccess)
		{
			if (bDebugThrowCalculations)
			{
				DrawDebugLine(StartPoint, EndPoint, 0, 255, 0, TRUE);
			}

			ThrowPlayerRotation = DEG_TO_RAD * Utils::NormalizeRotAngle(ThrowRotation);
			
			FMemMark CheckMark(GMainThreadMemStack);
			FCheckResult* CheckHits;

			FVector ThrowStartNrm = ThrowStartDirection.SafeNormal2D();
			if (Abs(ThrowPlayerRotation) <= HALF_PI)
			{
				// Check One Side
				StartPoint = ThrowStartLocation + ThrowStartNrm * EnemyPawn->ThrowStartPlayerDistance + TargetPlayer->CylinderComponent->Translation;
				StartPoint.Z += EnemyPawn->ThrowStartPlayerZOffset;
				EndPoint = ThrowStartLocation + ThrowStartNrm.RotateAngleAxis(ThrowPlayerRotation * RAD_TO_UNR, FVector(0.f, 0.f, 1.f)) * EnemyPawn->ThrowStartPlayerDistance + TargetPlayer->CylinderComponent->Translation;
				EndPoint.Z += EnemyPawn->ThrowStartPlayerZOffset;
				
				if (bDebugThrowCalculations)
				{
					DrawDebugLine(StartPoint, EndPoint, 255, 255, 0, TRUE);
				}

				CheckHits = GWorld->MultiLineCheck(GMainThreadMemStack, EndPoint, StartPoint, Extent, TRACE_AllBlocking, EnemyPawn);
				for( FCheckResult* Hit = CheckHits; Hit; Hit = Hit->GetNext())
				{
					if (Hit->Actor != TargetPlayer)
					{
						bSuccess = FALSE;
						break;
					}
				}
			}
			else 
			{
				// Check Two Sides
				StartPoint = ThrowStartLocation + ThrowStartNrm * EnemyPawn->ThrowStartPlayerDistance + TargetPlayer->CylinderComponent->Translation;
				StartPoint.Z += EnemyPawn->ThrowStartPlayerZOffset;
				EndPoint = ThrowStartLocation + ThrowStartNrm.RotateAngleAxis(Sgn(ThrowPlayerRotation) * HALF_PI * RAD_TO_UNR, FVector(0.f, 0.f, 1.f)) * EnemyPawn->ThrowStartPlayerDistance + TargetPlayer->CylinderComponent->Translation;
				EndPoint.Z += EnemyPawn->ThrowStartPlayerZOffset;

				if (bDebugThrowCalculations)
				{
					DrawDebugLine(StartPoint, EndPoint, 255, 255, 0, TRUE);
				}

				CheckHits = GWorld->MultiLineCheck(GMainThreadMemStack, EndPoint, StartPoint, Extent, TRACE_AllBlocking, EnemyPawn);
				for( FCheckResult* Hit = CheckHits; Hit; Hit = Hit->GetNext())
				{
					if (Hit->Actor != TargetPlayer)
					{
						bSuccess = FALSE;
						break;
					}
				}

				if (bSuccess)
				{
					StartPoint = EndPoint;
					EndPoint = ThrowStartLocation + ThrowStartNrm.RotateAngleAxis(ThrowPlayerRotation * RAD_TO_UNR, FVector(0.f, 0.f, 1.f)) * EnemyPawn->ThrowStartPlayerDistance + TargetPlayer->CylinderComponent->Translation;
					EndPoint.Z += EnemyPawn->ThrowStartPlayerZOffset;

					if (bDebugThrowCalculations)
					{
						DrawDebugLine(StartPoint, EndPoint, 255, 255, 0, TRUE);
					}

					CheckHits = GWorld->MultiLineCheck(GMainThreadMemStack, EndPoint, StartPoint, Extent, TRACE_AllBlocking, EnemyPawn);
					for( FCheckResult* Hit = CheckHits; Hit; Hit = Hit->GetNext())
					{
						if (Hit->Actor != TargetPlayer)
						{
							bSuccess = FALSE;
							break;
						}
					}
				}
			}

			CheckMark.Pop();
			
			if (bSuccess)
			{
				return TRUE;
			}
		}

		if (!bSuccess)
		{
			if (bDebugThrowCalculations)
			{
				DrawDebugLine(StartPoint, EndPoint, 255, 0, 0, TRUE);
			}

			warnf(TEXT("Throw: Skipping Direction %i"), bPassTwo ? i+8 : i);
		}
		
		if (!bPassTwo && i == 7)
		{
			warnf(TEXT("All directions failed, trying more refined search"));

			FLOAT halfIncrement = 0.5f * DIR_INCREMENT;
			ThrowRotation = ThrowOffset + halfIncrement;

			Direction = ThrowStartDirection.SafeNormal2D();
			Direction = Direction.RotateAngleAxis(ThrowRotation * DEG_TO_UNR, FVector(0.f, 0.f, 1.f));

			bPassTwo = TRUE;
			i = 0;
		}
		else
		{
			ThrowRotation += DirectionIncrement;
			Direction = Direction.RotateAngleAxis( DirectionIncrement * DEG_TO_UNR, FVector(0.f, 0.f, 1.f) );

			++i;
		}
	}

	ThrowPlayerRotation = DEG_TO_RAD * Utils::NormalizeRotAngle(ThrowRotation);

	return FALSE;
}

////////////////////////////////////////////////////////////////////////////////////////////
/// Object Interaction
////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////

void AOLBot::StartDoorTraversal( UBOOL bReversed )
{
	if (ActiveDoor != NULL)
	{
		ActiveDoor->DoorUser = EnemyPawn;

		if (ActiveDoor->GetOpenAngle() <= 60.0f)
		{
			bOpeningDoor = TRUE;
		}

		UAkAudioDevice * AudioDevice = UAkAudioDevice::Get();
		if ( AudioDevice )
		{
			FString DoorMaterial;

			switch (ActiveDoor->DoorMaterial)
			{			
			case OLDM_Metal:
				DoorMaterial = EnemyPawn->SoundSwitchParamDMMetal;
				break;
			case OLDM_SecurityDoor:
				DoorMaterial = EnemyPawn->SoundSwitchParamDMSecurity;
				break;
			case OLDM_BigPrisonDoor:
				DoorMaterial = EnemyPawn->SoundSwitchParamDMBigPrison;
				break;			
			default: // OLDM_Wood, OLDM_BigWoodenDoor
				DoorMaterial = EnemyPawn->SoundSwitchParamDMWood;
				break;
			}

			AudioDevice->SetSwitch(*EnemyPawn->SoundSwitchDoorMaterial, *DoorMaterial, EnemyPawn);
			AudioDevice->SetSwitch(*EnemyPawn->SoundSwitchDoorMaterial, *DoorMaterial, EnemyPawn, EnemyPawn->HeadBoneName);
		}
	}
}

void AOLBot::EndDoorTraversal()
{
	if (ActiveDoor != NULL)
	{
		FLOAT CloseSpeed = 0.f;
		if (bOpeningDoor)
		{
			bOpeningDoor = FALSE;

			if (ActiveDoor->bAutoClose)
			{
				ActiveDoor->Close(EnemyPawn);
				CloseSpeed = (ActiveDoor->DoorType == DT_Locker)
					? ActiveDoor->LockerClosingSpeed : ActiveDoor->ClosingSpeed;
			}
		}

		bBreachingDoor = FALSE;

		AOLDoor* DoneDoor = ActiveDoor;
		ActiveDoor->DoorUser = NULL;

		AOLPlayerController* OLPC = Utils::GetOLPC();
		if (OLPC)
			OLPC->eventNotifyEnemyDoorDone(EnemyPawn, DoneDoor, CloseSpeed);
	}
}

UBOOL AOLBot::GetDoorApproachPoint(AOLDoor* aDoor, FVector& ApproachPoint)
{
	FVector Edge0ToLocation = EnemyPawn->Location - aDoor->Edge0Dest;
	FVector Edge1ToLocation = EnemyPawn->Location - aDoor->Edge1Dest;
	FVector Edge0ToEdge1 = aDoor->Edge1Dest - aDoor->Edge0Dest;
	FVector Edge1ToEdge0 = -Edge0ToEdge1;

	if ((Edge0ToLocation | Edge0ToEdge1) > 0.f && (Edge1ToLocation | Edge1ToEdge0) > 0.f)
	{
		FVector Edge0ToLocProj = Edge0ToLocation.ProjectOnTo(Edge0ToEdge1);
		FVector ProjToLoc = Edge0ToLocation - Edge0ToLocProj;

		if (ProjToLoc.SizeSquared2D() < 20.0f )
		{
			return TRUE;
		}
		else
		{
			if (ApproachPoint == aDoor->Edge0Dest)
			{
				ApproachPoint = aDoor->Edge0Dest + Edge0ToLocProj;
			}
			else
			{
				FVector Edge1ToLocProj = Edge1ToLocation.ProjectOnTo(Edge1ToEdge0);

				ApproachPoint = aDoor->Edge1Dest + Edge1ToLocProj;
			}

			return FALSE;
		}
	}

	return FALSE;
}

FVector AOLBot::GetLockerDestination(const class AOLHidingSpot* Locker, UBOOL bInvestigating)
{
	AOLDoor* door = Locker->AssociatedDoor;

	const FVector& Offset = bInvestigating ? EnemyPawn->LockerInvestigateOffset : EnemyPawn->LockerAttackOffset;

	return door->LocalToWorld().TransformFVector(-1.0f * Offset);
}

FVector AOLBot::GetBestBedDestination(const AOLBed* Bed, BYTE& BedSide, UBOOL bInvestigating)
{
	static const FLOAT BED_HEIGHT_DIFF = 15.0f;

	BedSide = 0;

	if (Bed != NULL)
	{
		FVector Left = Bed->LocalToWorld().TransformFVector(bInvestigating ? EnemyPawn->BedInvestigateOffsetLeft : EnemyPawn->BedAttackOffsetLeft);
		FVector Right = Bed->LocalToWorld().TransformFVector(bInvestigating ? EnemyPawn->BedInvestigateOffsetRight : EnemyPawn->BedAttackOffsetRight);

		APylon* Pylon = NULL;
		FNavMeshPolyBase* Poly = NULL;
		FCheckResult Hit(1.f);

		if (EnemyPawn->Location.DistanceSquared(Left) < EnemyPawn->Location.DistanceSquared(Right))
		{
			NavigationHandle->GetPylonAndPolyFromPos(Left, EnemyPawn->WalkableFloorZ, Pylon, Poly);

			GWorld->SingleLineCheck(Hit, EnemyPawn, Bed->Location + VecZ(10.0f), Left + VecZ(10.0f), TRACE_AllBlocking);

			if (Poly == NULL || DistToGround(Left) > BED_HEIGHT_DIFF || (Hit.Actor != NULL && Hit.Actor != TargetPlayer))
			{
				NavigationHandle->GetPylonAndPolyFromPos(Right, EnemyPawn->WalkableFloorZ, Pylon, Poly);

				GWorld->SingleLineCheck(Hit, EnemyPawn, Bed->Location + VecZ(10.0f), Right + VecZ(10.0f), TRACE_AllBlocking);

				if (Poly == NULL || DistToGround(Right) > BED_HEIGHT_DIFF || (Hit.Actor != NULL && Hit.Actor != TargetPlayer))
				{
					return FVector(0.f);
				}
				else
				{
					BedSide = 1;
					return Right;
				}
			}
			else
			{
				return Left;
			}
		}
		else
		{
			NavigationHandle->GetPylonAndPolyFromPos(Right, EnemyPawn->WalkableFloorZ, Pylon, Poly);

			GWorld->SingleLineCheck(Hit, EnemyPawn, Bed->Location + VecZ(10.0f), Right + VecZ(10.0f), TRACE_AllBlocking);

			if (Poly == NULL || DistToGround(Right) > BED_HEIGHT_DIFF || (Hit.Actor != NULL && Hit.Actor != TargetPlayer))
			{
				NavigationHandle->GetPylonAndPolyFromPos(Left, EnemyPawn->WalkableFloorZ, Pylon, Poly);

				GWorld->SingleLineCheck(Hit, EnemyPawn, Bed->Location + VecZ(10.0f), Left + VecZ(10.0f), TRACE_AllBlocking);

				if (Poly == NULL || DistToGround(Left) > BED_HEIGHT_DIFF || (Hit.Actor != NULL && Hit.Actor != TargetPlayer))
				{
					return FVector(0.f);
				}
				else
				{
					return Left;
				}
			}
			else
			{
				BedSide = 1;
				return Right;
			}
		}
	}

	return FVector(0.f);
}

AOLWaypoint* AOLBot::GetCurrentWaypoint()
{
	if (PatrolRoute && PatrolRouteIndex >= 0 && PatrolRouteIndex < PatrolRoute->RouteList.Num())
	{
		return Cast<AOLWaypoint>(PatrolRoute->RouteList(PatrolRouteIndex).Actor);
	}

	return NULL;
}

////////////////////////////////////////////////////////////////////////////////////////////
/// Investigation
////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////

FLOAT AOLBot::GetInvestigateWeight(const FInvestigationPoint& Point) const
{
	if (EnemyPawn == NULL)
	{
		return 0.f;
	}

	switch(Point.Type)
	{
	case OLIPT_Locker:
		return EnemyPawn->InvestigationLockerWeight;
	case OLIPT_Bed:
		return EnemyPawn->InvestigationBedWeight;
	case OLIPT_Normal:
	default:
		{
			AOLAIInvestigationPoint* AIPoint = Cast<AOLAIInvestigationPoint>(Point.InteractiveActor);
			if (AIPoint != NULL && AIPoint->WeightOverride > 0.f)
			{
				return AIPoint->WeightOverride;
			}
			else
			{
				return EnemyPawn->InvestigationNormalWeight;
			}
		}
	}
}

void AOLBot::SetupInvestigation()
{
	if (TargetPlayer == NULL)
	{
		return;
	}

	bIsInvestigating = TRUE;
	bInvestigationPointValid = FALSE;

	AI_LOG(this, (TEXT("Investigate: Started")));

	AOLAIInvestigationVolume* IV = NULL;
	for ( INT Idx = 0; Idx < EnemyPawn->Touching.Num(); Idx++ )
	{
		IV = Cast<AOLAIInvestigationVolume>(EnemyPawn->Touching(Idx));
		if ( IV )
		{
			break;
		}
	}

	UBOOL bSetupInvestigation = TRUE;

	if (Group != NULL)
	{
		for (INT i = 0; i < Group->Bots.Num(); ++i)
		{
			AOLBot* Bot = Group->Bots(i);

			if (Bot->bInvestigationValid)
			{
				if (IV == Bot->InvestigationVolume
					|| EnemyPawn->Location.DistanceSquared(Bot->InvestigationOrigin) < Square(100.0f))
				{
					InvestigationOwner = Bot;
					Bot->InvestigatingBots.AddItem(this);

					bSetupInvestigation = FALSE;
					break;
				}
			}
		}
	}

	if (bSetupInvestigation)
	{
		if (IV != NULL)
		{
			SetupInvestigationInternalWithVolume(IV);
		}
		else
		{
			SetupInvestigationInternalNoVolume();
		}
	}

	bInvestigationFirstPoint = EnemyPawn->bUseFirstInvestigationInFront;
}

void AOLBot::SetupInvestigationInternalNoVolume()
{
	InvestigationPoints.Reset();
	InvestigateTotalWeight = 0.f;
	bInvestigationValid = TRUE;

	InvestigationOwner = this;
	InvestigatingBots.AddItem(this);

	InvestigationOrigin = Location;

	INT Idx = 0;
	FInvestigationPoint* Point = NULL;

	if (EnemyPawn->bInvestigateLockers)
	{
		// Find all Lockers and Beds in Radius Using Player's Cached list.
		for (INT i = 0; i < TargetPlayer->CachedHidingSpots.Num(); ++i)
		{
			AOLHidingSpot* Locker = TargetPlayer->CachedHidingSpots(i);
			if (Locker != NULL && Locker->AssociatedDoor && (Locker->Location - InvestigationOrigin).SizeSquared2D() <= Square(EnemyPawn->InvestigationMaxRadius) && !Locker->AssociatedDoor->IsBroken())
			{
				INT PtIdx = InvestigationPoints.AddZeroed();
				Point = &InvestigationPoints(PtIdx);
				Point->Type = OLIPT_Locker;
				Point->Location = GetLockerDestination(Locker, TRUE);
				Point->InteractiveActor = Locker;
				Point->bOccupied = (TargetPlayer->ActiveLocker == Locker);

				++Idx;
				InvestigateTotalWeight += GetInvestigateWeight(*Point);
			}
		}
	}

	if (EnemyPawn->bInvestigateBeds)
	{
		for (INT i = 0; i < TargetPlayer->CachedBeds.Num(); ++i)
		{
			AOLBed* Bed = TargetPlayer->CachedBeds(i);
			if (Bed != NULL && (Bed->Location - InvestigationOrigin).SizeSquared2D() <= Square(EnemyPawn->InvestigationMaxRadius))
			{
				INT PtIdx = InvestigationPoints.AddZeroed();
				Point = &InvestigationPoints(PtIdx);
				Point->Type = OLIPT_Bed;

				BYTE BedSide = 0;
				Point->Location = GetBestBedDestination(Bed, BedSide, TRUE);
				Point->InteractiveActor = Bed;
				Point->bOccupied = (TargetPlayer->ActiveBed == Bed);

				++Idx;
				InvestigateTotalWeight += GetInvestigateWeight(*Point);
			}
		}
	}
	
	INT NumObjects = Idx;

	// Fill up list with points
	FLOAT Incr = (2.f*PI/EnemyPawn->InvestigationNumPointsGenerated);
	FLOAT CurrentAngle = 0.f;
	while(Idx < EnemyPawn->InvestigationNumPointsGenerated)
	{
		FVector Direction(appCos(CurrentAngle), appSin(CurrentAngle), 0.f);

		UBOOL ObjInAngle = FALSE;
		for (INT ObjIdx = 0; ObjIdx < NumObjects; ++ObjIdx)
		{
			FVector ToObj = (InvestigationPoints(ObjIdx).Location - InvestigationOrigin).SafeNormal2D();

			FLOAT Angle = appAcos(Direction.Dot2d(ToObj));

			if (Angle < Incr*0.5f)
			{
				ObjInAngle = TRUE;
				break;
			}
		}

		if (!ObjInAngle)
		{
			FLOAT Length = EnemyPawn->InvestigationMinRadius + (EnemyPawn->InvestigationMaxRadius - EnemyPawn->InvestigationMinRadius) * appFrand();

			INT PtIdx = InvestigationPoints.AddZeroed();
			Point = &InvestigationPoints(PtIdx);
			Point->Type = OLIPT_Normal;
			Point->Location = FVector(appCos(CurrentAngle), appSin(CurrentAngle), 0.f);
			Point->Location *= Length;
			Point->Location += InvestigationOrigin;

			++Idx;
			InvestigateTotalWeight += GetInvestigateWeight(*Point);
		}

		CurrentAngle += Incr;
	}
}

void AOLBot::SetupInvestigationInternalWithVolume( AOLAIInvestigationVolume* Volume )
{
	InvestigationPoints.Reset();
	InvestigateTotalWeight = 0.f;
	bInvestigationValid = TRUE;

	InvestigationOwner = this;
	InvestigatingBots.AddItem(this);

	InvestigationVolume = Volume;

	FInvestigationPoint* Point = NULL;
	if (EnemyPawn->bInvestigateLockers)
	{
		for (INT i = 0; i < Volume->Lockers.Num(); ++i)
		{
			AOLHidingSpot* Locker = Volume->Lockers(i);
			if (Locker != NULL && !Locker->AssociatedDoor->IsBroken())
			{
				INT PtIdx = InvestigationPoints.AddZeroed();
				Point = &InvestigationPoints(PtIdx);
				Point->Type = OLIPT_Locker;
				Point->Location = GetLockerDestination(Locker, TRUE);
				Point->InteractiveActor = Locker;
				Point->bOccupied = (TargetPlayer->ActiveLocker == Locker);

				InvestigateTotalWeight += GetInvestigateWeight(*Point);
			}
		}
	}

	if (EnemyPawn->bInvestigateBeds)
	{
		for (INT i = 0; i < Volume->Beds.Num(); ++i)
		{
			AOLBed* Bed = Volume->Beds(i);
			if (Bed != NULL)
			{
				INT PtIdx = InvestigationPoints.AddZeroed();
				Point = &InvestigationPoints(PtIdx);
				Point->Type = OLIPT_Bed;

				BYTE BedSide = 0;
				Point->Location = GetBestBedDestination(Bed, BedSide, TRUE);
				Point->InteractiveActor = Bed;
				Point->bOccupied = (TargetPlayer->ActiveBed == Bed);

				InvestigateTotalWeight += GetInvestigateWeight(*Point);
			}
		}
	}

	for (INT i = 0; i < Volume->Points.Num(); ++i)
	{
		AOLAIInvestigationPoint* AIPoint = Volume->Points(i);
		if (AIPoint != NULL)
		{
			INT PtIdx = InvestigationPoints.AddZeroed();
			Point = &InvestigationPoints(PtIdx);
			Point->Type = OLIPT_Normal;

			Point->Location = AIPoint->Location;
			Point->InteractiveActor = AIPoint;

			InvestigateTotalWeight += GetInvestigateWeight(*Point);
		}
	}
}

UBOOL AOLBot::ChooseNextInvestigationPoint()
{
	if (InvestigationOwner == NULL)
	{
		return FALSE;
	}

	if (!bInvestigationPointValid)
	{
		bInvestigationPointValid = InvestigationOwner->GetNextInvestigationPointOnOwner(this, this->CurrentInvestigationPoint);

		return bInvestigationPointValid;
	}
	else
	{
		return TRUE;
	}
}

UBOOL AOLBot::GetNextInvestigationPointOnOwner(AOLBot* Bot, FInvestigationPoint& out_Point)
{
	if (InvestigationPoints.Num() == 0)
	{
		return FALSE;
	}

	AOLPlayerController* OLPC = Utils::GetOLPC();

	if (OLPC == NULL)
	{
		return FALSE;
	}
		
	FInvestigationPoint* playerPoint = NULL;
	INT pointIdx = 0;
	FLOAT overrideProbability = -1.0f;

	for (INT I = 0; I < InvestigationPoints.Num(); ++I)
	{
		FInvestigationPoint* Point = &InvestigationPoints(I);
		if (Point->bOccupied && (Point->Type == OLIPT_Locker || Point->Type == OLIPT_Bed))
		{
			playerPoint = Point;
			pointIdx = I;

			if (Point->Type == OLIPT_Locker)
			{
				AOLHidingSpot* hidingSpot = Cast<AOLHidingSpot>(Point->InteractiveActor);
				if (hidingSpot)
				{
					overrideProbability = hidingSpot->FindHiddenPlayerProbability;
				}
			}
			else
			{
				AOLBed* bed = Cast<AOLBed>(Point->InteractiveActor);
				if (bed)
				{
					overrideProbability = bed->FindHiddenPlayerProbability;
				}
			}

			break;
		}
	}

	if (playerPoint)
	{
		// Player is hidding. Should we find him?
			
		if (OLPC->FindHiddenPlayerSkipCount > 0)
		{
			OLPC->FindHiddenPlayerSkipCount--;
			AI_LOG(Bot, (TEXT("Investigate: Hidden player, skipped (%d skips remaining)"), OLPC->FindHiddenPlayerSkipCount));
		}
		else
		{
			FLOAT probability = overrideProbability >= 0.0f ? overrideProbability : Bot->EnemyPawn->InvestigationFindHiddenPlayerProbability;
			FLOAT diceRoll = appFrand();

			AI_LOG(Bot, (TEXT("Investigate: Hidden player, rolled %f, probability %f"), diceRoll, probability));

			if (diceRoll < probability)
			{
				// Player's found

				AI_LOG(Bot, (TEXT("Investigate: Picked player hiding spot")));

				FInvestigationPoint PickedPoint = *playerPoint;
				InvestigationPoints.RemoveSwap(pointIdx);
				FLOAT CurrentWeight = GetInvestigateWeight(PickedPoint);
				InvestigateTotalWeight -= CurrentWeight;

				if (CurrentWeight > 0.f && ValidateInvestigationPoint(Bot, PickedPoint))
				{
					TWEAKABLE INT NumSkipInvestigations = 2;

					OLPC->FindHiddenPlayerSkipCount = NumSkipInvestigations;

					out_Point = PickedPoint;
					return TRUE;
				}
			}
		}
	}
	
	INT NumChecked = 0;
	while (Bot->bInvestigationFirstPoint && NumChecked < InvestigationPoints.Num())
	{
		INT Idx = GetRandomInvestigationPointIdx();
		FInvestigationPoint* Point = &InvestigationPoints(Idx);

		++NumChecked;

		FVector PawnDir = Bot->EnemyPawn->Rotation.Vector().SafeNormal2D();
		FVector PointDir = (Point->Location - Bot->EnemyPawn->Location).SafeNormal2D();
		FLOAT PointAngle = appAcos(PawnDir|PointDir);

		if (!Point->bOccupied && PointAngle < Bot->EnemyPawn->InvestigationFirstPointAngle * DEG_TO_RAD)
		{
			FInvestigationPoint PickedPoint = InvestigationPoints(Idx);
			InvestigationPoints.RemoveSwap(Idx);
			FLOAT CurrentWeight = GetInvestigateWeight(PickedPoint);
			InvestigateTotalWeight -= CurrentWeight;

			if (CurrentWeight > 0.f && ValidateInvestigationPoint(Bot, PickedPoint))
			{
				Bot->bInvestigationFirstPoint = FALSE;
				out_Point = PickedPoint;
				return TRUE;
			}
		}
	}

	if (Bot->bInvestigationFirstPoint)
	{
		AI_LOG(Bot, (TEXT("Investigate: No First Point Found")));
		Bot->bInvestigationFirstPoint = FALSE;
	}

	while (InvestigationPoints.Num() > 0)
	{
		INT Idx = GetRandomInvestigationPointIdx();
		
		FInvestigationPoint PickedPoint = InvestigationPoints(Idx);

		if (!PickedPoint.bOccupied || InvestigationPoints.Num() == 1)
		{
			InvestigationPoints.RemoveSwap(Idx);
			FLOAT CurrentWeight = GetInvestigateWeight(PickedPoint);
			InvestigateTotalWeight -= CurrentWeight;

			if (!PickedPoint.bOccupied && CurrentWeight > 0.f && ValidateInvestigationPoint(Bot, PickedPoint))
			{
				out_Point = PickedPoint;
				return TRUE;
			}		
		}
	}

	AI_LOG(Bot, (TEXT("Investigate: Point Not Found")));
	return FALSE;
}

INT AOLBot::GetRandomInvestigationPointIdx()
{
	FLOAT AccumulatedWeight = 0.f;
	FLOAT WeightedRand = appFrand() * InvestigateTotalWeight;

	INT Idx = 0;
	while (Idx < InvestigationPoints.Num())
	{
		FInvestigationPoint* Point = &InvestigationPoints(Idx);

		AccumulatedWeight += GetInvestigateWeight(*Point);

		if (WeightedRand <= AccumulatedWeight)
		{
			break;
		}

		++Idx;
	}

	return Min(Idx, InvestigationPoints.Num() - 1);
}

UBOOL AOLBot::ValidateInvestigationPoint(AOLBot* Bot, const FInvestigationPoint& PickedPoint)
{
	if (!PickedPoint.bOccupied || Utils::GetOLPC()->IsPlayerFindableWhileHidden(EnemyPawn))
	{
		if (Bot->NavigationHandle->GetPylonFromPos(PickedPoint.Location) != NULL && Bot->eventGeneratePathToLocation(PickedPoint.Location))
		{
			if (InvestigationVolume != NULL || (Bot->NavigationHandle->CalculatePathDistance() <= Bot->EnemyPawn->InvestigationMaxPathDistance && !Bot->OLNavHandle->PathContainsDoor()))
			{
				AI_LOG(Bot, (TEXT("Investigate: Point Found")));

				if (PickedPoint.bOccupied)
				{
					Utils::GetOLPC()->SetPlayerFoundWhileHidden(EnemyPawn);
				}

				return TRUE;
			}
		}
	}
	AI_LOG(Bot, (TEXT("Investigate: Point Skipped")));

	return FALSE;
}

UBOOL AOLBot::InvestigateObject(AActor* anActor)
{
	FName NewState;
	bool Success = FALSE;

	if (anActor->IsA(AOLHidingSpot::StaticClass()))
	{
		NewState = FName( TEXT("InvestigatingLocker"), FNAME_Find);

		ActiveLocker = static_cast<AOLHidingSpot*>(anActor);

		InvestigateStartLocation = GetLockerDestination(ActiveLocker, TRUE);
		InvestigateStartRotation = ActiveLocker->AssociatedDoor->GetStaticDirection()*-1.0f;

		Success = TRUE;
	}
	else if (anActor->IsA(AOLBed::StaticClass()))
	{
		NewState = FName( TEXT("InvestigatingBed"), FNAME_Find);

		ActiveBed = static_cast<AOLBed*>(anActor);
		
		InvestigateStartLocation = GetBestBedDestination(ActiveBed, CurrentBedSide, TRUE);
		InvestigateStartRotation = ActiveBed->Location - ActiveBed->LocalToWorld().TransformFVector(FVector(0.f, CurrentBedSide ? 1.f : -1.f, 0.f));

		Success = TRUE;
	}

	if (Success && NewState != NAME_None)
	{
		EGotoState ResultState = GOTOSTATE_Success;
		ResultState = GotoState( NewState );
		if ( ResultState == GOTOSTATE_Success )
		{
			GotoLabel( FName(NAME_Begin) );
		}
	}

	InvestigateStartLocation.Z = EnemyPawn->Location.Z;

	return Success;
}

void AOLBot::LostTarget(UBOOL bAfterChase)
{
	EnemyPawn->PlayContextualVO(EVOC_LostTarget);

	if (GWorld->GetGameSequence())
	{
		TArray<USequenceObject*> AllLostTargetEvents;

		// Find all SeqEvent_Console objects anywhere.
		GWorld->GetGameSequence()->FindSeqObjectsByClass(UOLSeqEvent_AILostTarget::StaticClass(), AllLostTargetEvents);

		// Iterate over them, seeing if the name is the one we typed in.
		for (INT Idx = 0; Idx < AllLostTargetEvents.Num(); Idx++)
		{
			UOLSeqEvent_AILostTarget* LostTargetEvent = Cast<UOLSeqEvent_AILostTarget>(AllLostTargetEvents(Idx));
			if (LostTargetEvent != NULL)
			{
				TArray<UObject**> ObjVars;
				LostTargetEvent->GetObjectVars(ObjVars, TEXT("Targets"));

				for (INT EvtIdx = 0; EvtIdx < ObjVars.Num(); EvtIdx++)
				{
					AOLEnemyPawn* EventPawn = Cast<AOLEnemyPawn>(*(ObjVars(EvtIdx)));
					if(EventPawn == EnemyPawn)
					{
						if (bAfterChase || !LostTargetEvent->bOnlyAfterChase)
						{
							LostTargetEvent->CheckActivate(this, this);
						}
					}
				}
			}
		}
	}
}

void AOLBot::ClearInvestigation()
{
	bIsInvestigating = FALSE;

	if (InvestigationOwner != NULL)
	{
		InvestigationOwner->InvestigatingBots.RemoveItem(this);
		
		if (InvestigationOwner->InvestigatingBots.Num() == 0)
		{
			InvestigationOwner->bInvestigationValid = FALSE;

			InvestigationOwner->InvestigationVolume = NULL;
			InvestigationOwner->InvestigationOrigin = FVector(0.f);
			InvestigationOwner->InvestigationPoints.Reset();
		}

		InvestigationOwner = NULL;
	}
}

UBOOL AOLBot::IsAtTrimmedDoor()
{
	if (bTrimmedToDoor && OLNavHandle->MovePoints.Num() == 1 && (OLNavHandle->MovePoints(0).Destination - EnemyPawn->Location).Size2D() < 10.0f)
	{
		return TRUE;
	}

	return FALSE;
}

////////////////////////////////////////////////////////////////////////////////////////////
/// Debug
////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////

void AOLBot::DrawDebug()
{
	if (CurrentMoveStatus == MS_Moving)
	{
		if (OLNavHandle->MovePoints.Num() > 0)
		{
			static const UBOOL bCatmullRomCurve = TRUE;
			static const UBOOL bDebugBezierCurve = FALSE;
			static const FLOAT ControlPointScale = 0.5f;
			static const INT PointsPerSegment = 10;
			if (bCatmullRomCurve && EnemyPawn->CurrentMovePath.Num() > 1)
			{
				FLOAT CurrZ = EnemyPawn->CurrentMovePathStart.Z;
				FLOAT NextZ = EnemyPawn->CurrentMovePath(0).Z;

				for (INT i = 0; i < EnemyPawn->CurrentMovePath.Num(); ++i)
				{
					if (i == EnemyPawn->CurrentMovePathIdx)
					{
						DrawDebugSphere(EnemyPawn->CurrentMovePath(i) + FVector(0.0f, 0.0f, 10.0f), 5.0f, 10, 230, 24, 230);
					}
					else
					{
						DrawDebugSphere(EnemyPawn->CurrentMovePath(i) + FVector(0.0f, 0.0f, 10.0f), 5.0f, 10, 0, 24, 230);
					}
				}

				for (INT i = 2; i < EnemyPawn->CRPathSegments.Num()-1; ++i)
				{
					NextZ = EnemyPawn->CurrentMovePath(i - 2).Z;

					if (i - 2 != EnemyPawn->CurrentMovePathIdx)
					{
						DrawDebugLine(FVector(EnemyPawn->CRPathSegments(i-1), CurrZ) + FVector(0.0f, 0.0f, 10.0f), FVector(EnemyPawn->CRPathSegments(i), NextZ) + FVector(0.0f, 0.0f, 10.0f), 255, 255, 0,FALSE);
					}

					CurrZ = NextZ;
				}

				if (EnemyPawn->CurrentMovePathIdx == 0)
				{
					CurrZ = EnemyPawn->CurrentMovePathStart.Z;
					NextZ = EnemyPawn->CurrentMovePath(0).Z;
				}
				else
				{
					CurrZ = EnemyPawn->CurrentMovePath(EnemyPawn->CurrentMovePathIdx-1).Z;
					NextZ = EnemyPawn->CurrentMovePath(EnemyPawn->CurrentMovePathIdx).Z;
				}

				FLOAT ZIncr = (NextZ - CurrZ) / (EnemyPawn->CRPathSubSegments.Num() - 1.f);

				for (INT j = 0; j < EnemyPawn->CRPathSubSegments.Num()-1; ++j)
				{
					DrawDebugLine(FVector(EnemyPawn->CRPathSubSegments(j), CurrZ) + FVector(0.0f, 0.0f, 10.0f), FVector(EnemyPawn->CRPathSubSegments(j+1), CurrZ + ZIncr) + FVector(0.0f, 0.0f, 10.0f), 255, 255, 0,FALSE);

					CurrZ += ZIncr;
				}

				if (OLNavHandle->LastMovePointIdxForPath > -1)
				{
					for (INT k = OLNavHandle->LastMovePointIdxForPath; k < OLNavHandle->MovePoints.Num()-1; ++k)
					{
						DrawDebugLine(OLNavHandle->MovePoints(k).Destination + FVector(0.0f, 0.0f, 10.0f), OLNavHandle->MovePoints(k+1).Destination + FVector(0.0f, 0.0f, 10.0f), 255, 255, 0,FALSE);

						if (Abs(DistToGround(OLNavHandle->MovePoints(k+1).Destination)) > 20.0f)
						{
							//warnf(TEXT("Point %d too far above ground."), i);
						}
					}
				}
			}
			else if (bDebugBezierCurve)
			{
				TArray<FVector> SegmentPoints;

				SegmentPoints.AddItem(EnemyPawn->Location);

				for (INT i = 0; i < OLNavHandle->MovePoints.Num(); ++i)
				{
					SegmentPoints.AddItem(OLNavHandle->MovePoints(i).Destination);
				}

				TArray<FVector> ControlPoints = OLNavHandle->CalculateBezierControlPoints(SegmentPoints, ControlPointScale);

				INT Point1Idx = 0;
				for (INT i = 0; i < OLNavHandle->MovePoints.Num(); ++i)
				{
					TArray<FVector> IndControlPoints;
					TArray<FVector> OutDrawPoints;

					IndControlPoints.AddItem(ControlPoints(Point1Idx));
					IndControlPoints.AddItem(ControlPoints(Point1Idx+1));
					IndControlPoints.AddItem(ControlPoints(Point1Idx+2));
					IndControlPoints.AddItem(ControlPoints(Point1Idx+3));

					EvaluateBezier(IndControlPoints.GetData(), PointsPerSegment, OutDrawPoints);

					for (INT j = 0; j < OutDrawPoints.Num()-1; ++j)
					{
						DrawDebugLine(OutDrawPoints(j) + FVector(0.0f, 0.0f, 10.0f), OutDrawPoints(j+1) + FVector(0.0f, 0.0f, 10.0f), 255, 255, 0,FALSE);
					}

					Point1Idx += 3;
				}

			}
			else
			{
				DrawDebugLine(EnemyPawn->Location + FVector(0.0f, 0.0f, 10.0f), OLNavHandle->MovePoints(0).Destination + FVector(0.0f, 0.0f, 10.0f), 255, 255, 0,FALSE);

				if (Abs(DistToGround(OLNavHandle->MovePoints(0).Destination)) > 20.0f)
				{
					//warnf(TEXT("Point 0 too far above ground."));
				}

				for (INT i = 0; i < OLNavHandle->MovePoints.Num()-1; ++i)
				{
					DrawDebugLine(OLNavHandle->MovePoints(i).Destination + FVector(0.0f, 0.0f, 10.0f), OLNavHandle->MovePoints(i+1).Destination + FVector(0.0f, 0.0f, 10.0f), 255, 255, 0,FALSE);

					if (Abs(DistToGround(OLNavHandle->MovePoints(i+1).Destination)) > 20.0f)
					{
						//warnf(TEXT("Point %d too far above ground."), i);
					}
				}
			}
		}
		else if (!CurrentMove.ValidatedMovePoint.IsNearlyZero())
		{
			DrawDebugLine(EnemyPawn->Location + FVector(0.0f, 0.0f, 10.0f), CurrentMove.ValidatedMovePoint + FVector(0.0f, 0.0f, 10.0f), 255, 255, 0,FALSE);
		}

		DrawDebugCone(MoveTempDest + FVector(0.0f, 0.0f, 10.0f), FVector(0.0f, 0.0f, -1.0f), 10.0f, PI/4, PI/4, 8, FColor(0, 0, 255), FALSE);

		if (!CurrentMove.ValidatedMovePoint.IsNearlyZero())
		{
			DrawDebugCone(CurrentMove.ValidatedMovePoint + FVector(0.0f, 0.0f, 10.0f), FVector(0.0f, 0.0f, -1.0f), 10.0f, PI/4, PI/4, 8, FColor(255, 0, 255), FALSE);
		}

		NavigationHandle->DrawPathCache();
	}

	if (bIsInvestigating)
	{
		for (INT i = 0; i < InvestigationPoints.Num(); ++i)
		{
			DrawDebugSphere(InvestigationPoints(i).Location, 5.0f, 10, 255, 50, 100);
		}
	}

	if (Utils::GetHUD() && Utils::GetHUD()->CurrentDebugBot == this)
	{
		DrawDebugCylinder( EnemyPawn->CylinderComponent->Bounds.Origin - FVector(0, 0, EnemyPawn->CylinderComponent->CollisionHeight), EnemyPawn->CylinderComponent->Bounds.Origin + FVector(0, 0, EnemyPawn->CylinderComponent->CollisionHeight), EnemyPawn->CylinderComponent->CollisionRadius, 10, 0, 190, 0);
	}
	else
	{
		DrawDebugCylinder( EnemyPawn->CylinderComponent->Bounds.Origin - FVector(0, 0, EnemyPawn->CylinderComponent->CollisionHeight), EnemyPawn->CylinderComponent->Bounds.Origin + FVector(0, 0, EnemyPawn->CylinderComponent->CollisionHeight), EnemyPawn->CylinderComponent->CollisionRadius, 10, 0, 60, 60);
	}

	DrawDebugCoordinateSystem( EnemyPawn->Location + FVector(0.f,0.f,10.0f), EnemyPawn->Rotation, 10.0f);

	if (!EnemyPawn->CurrentRepulsion.IsNearlyZero())
	{
		DrawDebugLine(EnemyPawn->Location + FVector(0.f,0.f,10.0f), EnemyPawn->Location + EnemyPawn->CurrentRepulsion*100.0f + FVector(0.f,0.f,10.0f), 255, 0, 0, FALSE);
	}

	if (!GetDestinationPosition().IsNearlyZero())
	{
		DrawDebugCone(GetDestinationPosition() + FVector(0.0f, 0.0f, 10.0f), FVector(0.0f, 0.0f, -1.0f), 10.0f, PI/4, PI/4, 8, FColor(175, 17, 214), FALSE);
	}
}

void AOLBot::DrawDebugStates(UObject* anObject,  UCanvas* aCanvas,FLOAT XL,FLOAT XPos,FLOAT& out_YL,FLOAT& out_YPos)
{
	aCanvas->SetPos(XPos, out_YPos);

	FString text(TEXT("State: "));
	
	if ( anObject->GetStateFrame() != NULL )
	{
		text += *anObject->GetStateFrame()->StateNode->GetName();
		
		for (INT idx = anObject->GetStateFrame()->StateStack.Num()-1; idx >= 0 ; idx--)
		{
			text += FString::Printf(TEXT(" <= [%d] %s"), idx, *anObject->GetStateFrame()->StateStack(idx).State->GetName());				
		}
	}
	else
	{
		text += FString(TEXT("[none]"));
	}

	aCanvas->DrawText(text);
	out_YPos += out_YL;
}

/*============================================================================*/

////////////////////////////////////////////////////////////////////////////////////////////
/// OLAISightComponent
////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////

inline FLOAT FDistSquared2D( const FVector2D &V1, const FVector2D &V2 )
{
	return Square(V2.X-V1.X) + Square(V2.Y-V1.Y);
}

FVisionParameters* UOLAISightComponent::GetCurrentVisionParameters()
{
	AOLEnemyPawn* Pawn = Bot->EnemyPawn;

	UBOOL bTargetUsingNightvision = Bot->TargetPlayer->IsInNightVision();
	
	FVisionParameters* CurrentVisionParameters = CanSeeTarget ? &Pawn->LightAwareVisionParameters : &Pawn->LightUnAwareVisionParameters;
	
	if (Bot->bTargetInDarkness)
	{
		if (bTargetUsingNightvision)
		{
			CurrentVisionParameters = CanSeeTarget ? &Pawn->NightvisionAwareVisionParameters : &Pawn->NightvisionUnAwareVisionParameters;
		}
		else
		{
			CurrentVisionParameters = CanSeeTarget ? &Pawn->DarkAwareVisionParameters : &Pawn->DarkUnAwareVisionParameters;
		}
	}

	return CurrentVisionParameters;
}

void UOLAISightComponent::Tick( FLOAT DeltaSeconds )
{
	if (Bot == NULL)
	{
		Bot = Cast<AOLBot>(Owner);
	}

	CouldSeeTarget = CanSeeTarget;

	if (Bot->TargetPlayer == NULL || Bot->TargetPlayer->bIsGhost || Bot->TargetPlayer->bDeleteMe)
	{
		if (CouldSeeTarget)
		{
			CanSeeTarget = FALSE;
			if (Bot->Group != NULL)
			{
				Bot->Group->eventRecalculate();
			}
			else
			{
				Bot->eventRecalculate();
			}
		}

		IsInClose = FALSE;
		IsInNarrowCone = FALSE;
		IsInWideCone = FALSE;

		WideConeTimer = -1.0f;
		LoseTimer = 0.0f;
	}
	else
	{
		FVisionParameters* CurrentVisionParameters = GetCurrentVisionParameters();
		
		if (bAlwaysSeeTarget)
		{
			CanSeeTarget = TRUE;
			LoseTimer = -1.0f;
		}
		else if (bIgnoreTarget)
		{
			CanSeeTarget = FALSE;
			LoseTimer = 0.0f;
			bSawPlayerEnterHidingSpot = FALSE;
			bSawPlayerEnterBed = FALSE;
			bSawPlayerInSqueeze = FALSE;
			bSawPlayerGoUnder = FALSE;
		}
		else
		{
			UpdateTargetVisibility();
	
			if (IsInNarrowCone || IsInClose)
			{
				CanSeeTarget = TRUE;
				LoseTimer = -1.0f;
			}
			else if (IsInWideCone)
			{
				if (!CouldSeeTarget)
				{
					if (WideConeTimer == -1.0f)
					{
						WideConeTimer = CurrentVisionParameters->WideConeReactionTime;
					}
					else
					{
						WideConeTimer -= DeltaSeconds;

						if (WideConeTimer <= 0.0f)
						{
							CanSeeTarget = TRUE;
							LoseTimer = -1.0f;
						}
					}
				}
			}
			else if (CouldSeeTarget)
			{
				if (LoseTimer == -1.0f && !bWasAlwaysSeeTarget)
				{
					WideConeTimer = -1.0f;
					LoseTimer = CurrentVisionParameters->LoseTargetTime;
				}

				LoseTimer -= DeltaSeconds;
				if (LoseTimer <= 0.0f)
				{
					CanSeeTarget = FALSE;
				}
			}
		}

		if (Bot->TargetPlayer->LocomotionMode == LM_Locker 
			|| (Bot->TargetPlayer->LocomotionMode == LM_SpecialMove 
				&& (Bot->TargetPlayer->SpecialMove == SMT_EnterLocker 
					|| Bot->TargetPlayer->SpecialMove == SMT_ExitLocker
					|| Bot->TargetPlayer->SpecialMove == SMT_HeroGrabbedFromLocker)))
		{
			if (!CanSeeTarget && bSawPlayerEnterHidingSpot)
			{
				CanSeeTarget = TRUE;
				LoseTimer = -1.f;
			}
			else if (CanSeeTarget && LoseTimer == -1.0f)
			{
				if (Bot->TargetPlayer->LocomotionMode == LM_Locker && !bSawPlayerEnterHidingSpot)
				{
					CanSeeTarget = FALSE;
				}
				else
				{
					bSawPlayerEnterHidingSpot = TRUE;
				}
			}
		}
		else
		{
			bSawPlayerEnterHidingSpot = FALSE;
		}

		if (Bot->TargetPlayer->LocomotionMode == LM_Bed
			|| (Bot->TargetPlayer->LocomotionMode == LM_SpecialMove
				&& (Bot->TargetPlayer->SpecialMove == SMT_EnterBed 
				|| Bot->TargetPlayer->SpecialMove == SMT_ExitBed
				|| Bot->TargetPlayer->SpecialMove == SMT_HeroGrabbedFromBed)))
		{
			if (!CanSeeTarget && bSawPlayerEnterBed)
			{
				CanSeeTarget = TRUE;
				LoseTimer = -1.f;
			}
			else if (CanSeeTarget && LoseTimer == -1.0f)
			{
				if (Bot->TargetPlayer->LocomotionMode == LM_Bed && !bSawPlayerEnterBed)
				{
					CanSeeTarget = FALSE;
				}
				else
				{
					bSawPlayerEnterBed = TRUE;
				}
			}
		}
		else
		{
			bSawPlayerEnterBed = FALSE;
		}

		UBOOL bPlayerInSqueeze = (Bot->TargetPlayer->ActiveSqueeze && !Bot->TargetPlayer->ActiveSqueeze->bNoHands) && (Bot->TargetPlayer->LocomotionMode == LM_Squeeze || Bot->TargetPlayer->SpecialMove == SMT_EnterSqueeze);
		if (bPlayerInSqueeze)
		{
			if (!CanSeeTarget && bSawPlayerInSqueeze)
			{
				CanSeeTarget = TRUE;
				LoseTimer = -1.f;
			}
			else if (CanSeeTarget && LoseTimer == -1.0f)
			{
				bSawPlayerInSqueeze = TRUE;
				LastSqueeze = Bot->TargetPlayer->ActiveSqueeze;
			}
		}
		else
		{
			bSawPlayerInSqueeze = FALSE;
		}

		if (Bot->TargetPlayer->CanBeGrabbedUnder())
		{
			if (!CanSeeTarget && bSawPlayerGoUnder)
			{
				if (Bot->TargetPlayer->Location.DistanceSquared(SawPlayerGoUnderLastPosition) < Square(SawPlayerGoUnderMaxDistance))
				{
					CanSeeTarget = TRUE;
					LoseTimer = -1.f;
				}
				else
				{
					bSawPlayerGoUnder = FALSE;
					SawPlayerGoUnderLastPosition = FVector(0.f);
				}
			}
			else if (CanSeeTarget && LoseTimer == -1.0f)
			{
				bSawPlayerGoUnder = TRUE;
				SawPlayerGoUnderLastPosition = Bot->TargetPlayer->Location;
			}
		}
		else
		{
			bSawPlayerGoUnder = FALSE;
			SawPlayerGoUnderLastPosition = FVector(0.f);
		}

		if (Bot->bAttacking || IsInState(Bot->InterruptionState))
		{
			CanSeeTarget = CouldSeeTarget;
		}

		if (CanSeeTarget != CouldSeeTarget)
		{
			if (Bot->Group != NULL)
			{
				Bot->Group->eventRecalculate();
			}
			else
			{
				Bot->eventRecalculate();
			}
		}

		if (CanSeeTarget)
		{
			WideConeTimer = 0.0f;
		
			Bot->TriggerVisualDisturbance(Bot->TargetPlayer->GetDisturbanceLocation(Bot->EnemyPawn));
		}
		else
		{
			LoseTimer = 0.0f;
			bSawPlayerEnterHidingSpot = FALSE;
			bSawPlayerEnterBed = FALSE;
			bSawPlayerInSqueeze = FALSE;
			bSawPlayerGoUnder = FALSE;
		}
	}

	if (bWasAlwaysSeeTarget)
	{
		bWasAlwaysSeeTarget = FALSE;
	}

	if (bDebugSight)
	{
		DrawDebug();
	}
}

void UOLAISightComponent::GetVisionBoneLocationAndRotation(FVector& out_BoneLocation, FRotator& out_BoneRotation)
{
	out_BoneLocation = Bot->EnemyPawn->Mesh->GetBoneLocation(Bot->EnemyPawn->VisionBone);

	if (out_BoneLocation.IsZero())
	{
		out_BoneLocation = Bot->EnemyPawn->Location + FVector(0.f, 0.f, Bot->EnemyPawn->EyeHeight);
	}

	FQuat BoneQuat = Bot->EnemyPawn->Mesh->GetBoneQuaternion(Bot->EnemyPawn->VisionBone);

	BoneQuat *= FQuat(FVector(1.f, 0.f, 0.f), -HALF_PI);
	BoneQuat *= FQuat(FVector(0.f, 1.f, 0.f), HALF_PI);

	out_BoneRotation = BoneQuat.Rotator();
	if (out_BoneRotation.IsZero())
	{
		out_BoneRotation = Bot->EnemyPawn->Rotation;
	}
}

void UOLAISightComponent::UpdateTargetVisibility()
{
	FVisionParameters* CurrentVisionParameters = GetCurrentVisionParameters();

	FVector BoneLocation;
	FRotator BoneRotation;
	GetVisionBoneLocationAndRotation(BoneLocation, BoneRotation);

	FVector TargetHead = Bot->TargetPlayer->EyeLocation;
		
	UBOOL InClose2D = (BoneLocation - TargetHead).SizeSquared2D() < Square(CurrentVisionParameters->CloseDistance);
	
	FLOAT BottomZ = Bot->TargetPlayer->Location.Z;
	FLOAT TopZ = BottomZ + Bot->TargetPlayer->CylinderComponent->Translation.Z + Bot->TargetPlayer->CylinderComponent->CollisionHeight;

	UBOOL InCloseZ = (BoneLocation.Z + CurrentVisionParameters->CloseDistance > BottomZ)
					&& (BoneLocation.Z - CurrentVisionParameters->CloseDistance < TopZ);

	IsInClose = InClose2D && InCloseZ;

	FLOAT DistToPlayerSqr = (BoneLocation - TargetHead).SizeSquared();

	// For Debug Only
	DistanceToPlayer = appSqrt(DistToPlayerSqr);

	// Horizontal Angle
	FVector2D targetVec2 = FVector2D(TargetHead.X, TargetHead.Y);
	FVector2D botVec2 = FVector2D(BoneLocation.X, BoneLocation.Y);
	FLOAT aSqr = 1;
	FLOAT bSqr = FDistSquared2D(botVec2, targetVec2);
	FVector2D cVec2 = FVector2D(BoneRotation.Vector().X, BoneRotation.Vector().Y).SafeNormal() + botVec2;
	FLOAT cSqr = FDistSquared2D(targetVec2, cVec2);

	FLOAT a = appSqrt(aSqr);
	FLOAT b = appSqrt(bSqr);

	HorizontalAngleToPlayer = appAcos((aSqr + bSqr - cSqr)/(2*a*b)) * (180/(FLOAT)PI);

	// Vertical Angle
	FVector targetFlat = FVector(TargetHead.X, TargetHead.Y, BoneLocation.Z);
	aSqr = FDistSquared(BoneLocation, targetFlat);
	bSqr = FDistSquared(BoneLocation, TargetHead);
	cSqr = FDistSquared(targetFlat, TargetHead);

	a = appSqrt(aSqr);
	b = appSqrt(bSqr);

	VerticalAngleToPlayer = appAcos((aSqr + bSqr - cSqr)/(2*a*b)) * (180/(FLOAT)PI);

	bPeeking = FALSE;
	if (Bot->TargetPlayer->IsPeeking())
	{
		FVector StartPoint = BoneLocation;
		FVector EndPoint = Bot->TargetPlayer->Location;
		EndPoint.Z = TargetHead.Z;

		FCheckResult Hit;
		if (!GWorld->SingleLineCheck(Hit, Bot->EnemyPawn, EndPoint, StartPoint, TRACE_AllBlocking|TRACE_ComplexCollision|TRACE_AISight ))
		{
			if (Hit.Actor != Bot->TargetPlayer)
			{
				bPeeking = TRUE;
			}
		}
	}

	bCrouched = Bot->TargetPlayer->bIsCrouched;
	IsInNarrowCone = IsTargetInCone(CurrentVisionParameters->NarrowCone, DistToPlayerSqr, HorizontalAngleToPlayer, VerticalAngleToPlayer, bPeeking, bCrouched, CurrentVisionParameters->CrouchMultiplier);
	IsInWideCone = IsTargetInCone(CurrentVisionParameters->WideCone, DistToPlayerSqr, HorizontalAngleToPlayer, VerticalAngleToPlayer, bPeeking, bCrouched, CurrentVisionParameters->CrouchMultiplier);

	if (IsInNarrowCone || IsInWideCone || IsInClose)
	{			
		FVector StartPoint = BoneLocation;
		FVector EndPoint = TargetHead;

		FCheckResult Hit;
		GWorld->SingleLineCheck(Hit, Bot, EndPoint, StartPoint, TRACE_World|TRACE_StopAtAnyHit|TRACE_ComplexCollision|TRACE_AISight );

		if(bDebugSight)
		{
			Bot->DrawDebugLine(StartPoint, Hit.Actor ? Hit.Location : EndPoint, 255, 0, 0, FALSE);
		}

		if(Hit.Actor)
		{
			IsInNarrowCone = FALSE;
			IsInWideCone = FALSE;
			IsInClose = FALSE;
			return;
		}

		FMemMark Mark(GMainThreadMemStack);
		FCheckResult* FirstHit = GWorld->MultiLineCheck
			(
			GMainThreadMemStack,
			EndPoint,
			StartPoint,
			FVector(0,0,0),
			TRACE_Others|TRACE_AISight,
			Bot
			);
		
		for( FCheckResult* Check = FirstHit; Check != NULL; Check = Check->GetNext() )
		{
			if (Check->Actor && Check->Actor->IsA(AOLDoor::StaticClass()))
			{
				IsInNarrowCone = FALSE;
				IsInWideCone = FALSE;
				IsInClose = FALSE;
				break;
			}
		}

		UBOOL bTargetUsingNightvision = (Bot->TargetPlayer->IsInNightVision());
		if(TRUE/*!bTargetUsingNightvision*/)
		{
			FirstHit = GWorld->MultiLineCheck
				(
				GMainThreadMemStack,
				EndPoint,
				StartPoint,
				FVector(0,0,0),
				TRACE_Volumes|TRACE_PhysicsVolumes|TRACE_AISight,
				Bot
				);

			AOLAIVisionObstructionVolume* Volume = NULL;
			for( FCheckResult* Check = FirstHit; Check != NULL; Check = Check->GetNext() )
			{
				Volume = Cast<AOLAIVisionObstructionVolume>(Check->Actor);
				if (Volume && Volume->bEnabled)
				{
					IsInNarrowCone = FALSE;
					IsInWideCone = FALSE;
					IsInClose = FALSE;
					break;
				}
			}
		}

		Mark.Pop();
	}
}

UBOOL UOLAISightComponent::IsTargetInCone(FVisionCone& aCone, FLOAT DistToPlayerSqr, FLOAT HorzAngleToPlayer, FLOAT VertAngleToPlayer, UBOOL bPeeking, UBOOL bCrouching, FLOAT CrouchMultiplier)
{
	FLOAT DistMultipler = bCrouching ? CrouchMultiplier : 1.0f;
	FLOAT Distance = bPeeking ? aCone.PeekingDistance : aCone.Distance;

	return ((DistToPlayerSqr <  Square(Distance * DistMultipler)) && HorzAngleToPlayer < aCone.HorizontalAngle && VertAngleToPlayer < aCone.VerticalAngle);
}

void UOLAISightComponent::DrawDebugVisionCone(const FVisionCone& Cone, BYTE R, BYTE G, BYTE B, UBOOL bPersistentLines)
{
	FVector EyePosition;
	FRotator BoneRotation;
	GetVisionBoneLocationAndRotation(EyePosition, BoneRotation);
	
	BoneRotation.Pitch = 0;
	
	FRotator BottomLeftR = BoneRotation + FRotator((INT)-Cone.VerticalAngle*DEG_TO_UNR, (INT)-Cone.HorizontalAngle*DEG_TO_UNR, 0);
	FVector BottomLeft = EyePosition + BottomLeftR.Vector()*Cone.Distance;

	FRotator BottomRightR = BoneRotation + FRotator((INT)-Cone.VerticalAngle*DEG_TO_UNR, (INT)Cone.HorizontalAngle*DEG_TO_UNR, 0);
	FVector BottomRight = EyePosition + BottomRightR.Vector()*Cone.Distance;

	FRotator TopLeftR = BoneRotation + FRotator((INT)Cone.VerticalAngle*DEG_TO_UNR, (INT)-Cone.HorizontalAngle*DEG_TO_UNR, 0);
	FVector TopLeft = EyePosition + TopLeftR.Vector()*Cone.Distance;

	FRotator TopRightR = BoneRotation + FRotator((INT)Cone.VerticalAngle*DEG_TO_UNR, (INT)Cone.HorizontalAngle*DEG_TO_UNR, 0);
	FVector TopRight = EyePosition + TopRightR.Vector()*Cone.Distance;

	Bot->DrawDebugLine(EyePosition, BottomLeft, R, G, B, bPersistentLines);
	Bot->DrawDebugLine(EyePosition, BottomRight, R, G, B, bPersistentLines);
	Bot->DrawDebugLine(EyePosition, TopLeft, R, G, B, bPersistentLines);
	Bot->DrawDebugLine(EyePosition, TopRight, R, G, B, bPersistentLines);

	static const INT Segments = 6;

	FVector Vertex1, Vertex2, Vertex3, Vertex4;
	const INT AngleIncY = Cone.VerticalAngle*DEG_TO_UNR*2/Segments;
	const INT AngleIncX = Cone.HorizontalAngle*DEG_TO_UNR*2/Segments;
	INT NumSegmentsY = Segments, Latitude = BoneRotation.Pitch + 16384 - Cone.VerticalAngle*DEG_TO_UNR;
	INT NumSegmentsX, Longitude;

	FLOAT SinY1 = GMath.SinTab(Latitude);
	FLOAT CosY1 = GMath.CosTab(Latitude);

	FLOAT SinY2, CosY2;
	FLOAT SinX, CosX;

	while( NumSegmentsY-- )
	{
		Latitude += AngleIncY;
		SinY2 = GMath.SinTab(Latitude);
		CosY2 = GMath.CosTab(Latitude);

		Longitude = BoneRotation.Yaw - Cone.HorizontalAngle*DEG_TO_UNR;
		SinX = GMath.SinTab(Longitude);
		CosX = GMath.CosTab(Longitude);

		Vertex1 = FVector((CosX * SinY1), (SinX * SinY1), CosY1) * Cone.Distance + EyePosition;
		Vertex3 = FVector((CosX * SinY2), (SinX * SinY2), CosY2) * Cone.Distance + EyePosition;

		NumSegmentsX = Segments;
		while( NumSegmentsX-- )
		{
			Longitude += AngleIncX;
			SinX = GMath.SinTab(Longitude);
			CosX = GMath.CosTab(Longitude);

			Vertex2 = FVector((CosX * SinY1), (SinX * SinY1), CosY1) * Cone.Distance + EyePosition;
			Vertex4 = FVector((CosX * SinY2), (SinX * SinY2), CosY2) * Cone.Distance + EyePosition;

			Bot->DrawDebugLine(Vertex1, Vertex2, R, G, B, bPersistentLines);
			Bot->DrawDebugLine(Vertex1, Vertex3, R, G, B, bPersistentLines);

			if (NumSegmentsX == 0) // Last XSegment
			{
				Bot->DrawDebugLine(Vertex2, Vertex4, R, G, B, bPersistentLines);
			}

			if (NumSegmentsY == 0) // Last YSegment
			{
				Bot->DrawDebugLine(Vertex3, Vertex4, R, G, B, bPersistentLines);
			}

			Vertex1 = Vertex2;
			Vertex3 = Vertex4;
		}

		SinY1 = SinY2;
		CosY1 = CosY2;
	}
}

void UOLAISightComponent::DrawDebug()
{
	FVisionParameters* CurrentVisionParameters = GetCurrentVisionParameters();

	FVector EyePosition;
	FRotator BoneRotation;
	GetVisionBoneLocationAndRotation(EyePosition, BoneRotation);

	Bot->DrawDebugCoordinateSystem(EyePosition, BoneRotation, 100.0f, FALSE);
	Bot->DrawDebugCoordinateSystem(Bot->EnemyPawn->Location, Bot->EnemyPawn->Rotation, 100.0f, FALSE);

	// Close Distance
	Bot->DrawDebugSphere(EyePosition, CurrentVisionParameters->CloseDistance, 12, 255, 0, 0, FALSE);

	// Narrow Cone
	DrawDebugVisionCone(CurrentVisionParameters->NarrowCone, 0, 255, 0, FALSE);
	DrawDebugVisionCone(CurrentVisionParameters->WideCone, 0, 0, 255, FALSE);
}

/*============================================================================*/

////////////////////////////////////////////////////////////////////////////////////////////
/// OLAIGroup
////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////

UBOOL AOLAIGroup::CanAttack(AOLBot* Attacker)
{
	return (AttackingBot == NULL && bTokenAvailable) || (AttackingBot == Attacker);
}

void AOLAIGroup::TakeAttackToken(AOLBot* Attacker)
{
	AttackingBot = Attacker;
	bTokenAvailable = FALSE;
}

void AOLAIGroup::ReturnAttackToken(AOLBot* Attacker)
{
	if (AttackingBot == Attacker)
	{
		AttackingBot = NULL;

		AttackTimer = MinTimeBetweenAttacks + (appFrand() * (MaxTimeBetweenAttacks - MinTimeBetweenAttacks));

		if (Bots.Num() > 1)
		{
			Attacker->AttackTimer = AttackTimer + 0.5f;
		}
	}
}

void AOLAIGroup::TickSpecial( FLOAT DeltaSeconds )
{
	if (AttackTimer > 0.f)
	{
		AttackTimer -= DeltaSeconds;

		if (AttackTimer <= 0.f)
		{
			bTokenAvailable=true;
			AttackTimer = 0.f;
		}
	}
}

/*============================================================================*/

////////////////////////////////////////////////////////////////////////////////////////////
/// OLAICmd_MoveAbility
////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////

void UOLAICmd_MoveAbility::ModifyPath(FVector NewDestination, TArray<FVector>& NewPath)
{
	AOLBot* BotOwner = Cast<AOLBot>(GameAIOwner);

	NewPath.Reset();

	if (BotOwner->EnemyPawn->CurrentMovePath.Num() > 0)
	{
		NewPath.Append(BotOwner->EnemyPawn->CurrentMovePath);

		if (BotOwner->EnemyPawn->CurrentMovePathIdx > 0)
		{
			NewPath.Remove(0, BotOwner->EnemyPawn->CurrentMovePathIdx);
		}

		NewPath.Pop();
	}

	NewPath.Push(NewDestination);
}

/*============================================================================*/

////////////////////////////////////////////////////////////////////////////////////////////
/// OLAICmd_MoveAbility
////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////

void AOLWaypoint::WaypointReachedEvent(AActor* InInstigator)
{
	TArray<INT> ActivateIndices;
	ActivateIndices.AddItem(0);
	
	UOLSeqEvent_AIWaypointAction* WaypointEvent;
	for (INT i = 0; i < GeneratedEvents.Num(); ++i)
	{
		WaypointEvent = Cast<UOLSeqEvent_AIWaypointAction>(GeneratedEvents(i));
		if (WaypointEvent != NULL)
		{
			WaypointEvent->CheckActivate(this, InInstigator, FALSE, &ActivateIndices);
		}
	}
}

void AOLWaypoint::AnimStartedEvent(AActor* InInstigator)
{
	TArray<INT> ActivateIndices;
	ActivateIndices.AddItem(1);

	UOLSeqEvent_AIWaypointAction* WaypointEvent;
	for (INT i = 0; i < GeneratedEvents.Num(); ++i)
	{
		WaypointEvent = Cast<UOLSeqEvent_AIWaypointAction>(GeneratedEvents(i));
		if (WaypointEvent != NULL)
		{
			WaypointEvent->CheckActivate(this, InInstigator, FALSE, &ActivateIndices);
		}
	}
}

/*============================================================================*/