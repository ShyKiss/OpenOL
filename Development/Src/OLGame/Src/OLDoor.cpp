#include "OLGame.h"
#include "AkAudioDevice.h"

// Defined in OLPlayerController.cpp — marks all Kismet actions reachable from Op
// as observer-only so they play back visually without triggering gameplay side-effects.
extern void MarkChainRemoteObserver(USequenceOp* Op, int Depth);
extern void UnmarkChainRemoteObserver(USequenceOp* Op, int Depth);
#include "DebugRenderSceneProxy.h"
#include "EngineAnimClasses.h"
#include "UDKBaseAnimationClasses.h"
#include "GameFrameworkAnimClasses.h"
#include "OLGameAnimClasses.h"

IMPLEMENT_CLASS(AOLDoor);

UBOOL GAllowGhostDoors = FALSE;
UBOOL GUnlockDoors = FALSE;


void AOLDoor::PostBeginPlay()
{
	Super::PostBeginPlay();

	UnregisterNavmeshObstacle();

	if (OpenRatio > KINDA_SMALL_NUMBERF || bUseObstacleOnClose)
	{
		RegisterNavMeshObstacle();
	}
}

void AOLDoor::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	Init();
	
	if (WaitPointComponent)
	{
		WaitPointComponent->SetLocalToWorld(LocalToWorld());
	}

	if (PropertyChangedEvent.Property != NULL)
	{
		PostEditChange();
	}
}

#if WITH_EDITOR
void AOLDoor::CheckForErrors()
{
	Super::CheckForErrors();
	SoundConnectorComp->CheckConnectorForErrors();
}
#endif

void AOLDoor::PostLoad()
{
	Super::PostLoad();
	Init();

	if (GIsGame)
	{
		bWasInitiallyLocked = bLocked;
	}

	if (WaitPointComponent != NULL)
	{
		WaitPointComponent->SetLocalToWorld(LocalToWorld());

		if (WaitPointComponent->WaitPoints.Num() == 0)
		{
			WaitPointComponent->GenerateWaitPoints(!bReverseDirection);
		}
	}
}

void AOLDoor::Reset()
{
	StopShaking();
	
	if (DoorBreakState != DBS_Normal)
	{
		DoorMainMesh->SetHiddenGame(FALSE);
		BreakingForwardMesh->SetHiddenGame(TRUE);
		BrokenForwardMesh->SetHiddenGame(TRUE);
		BreakingBackwardMesh->SetHiddenGame(TRUE);
		BrokenBackwardMesh->SetHiddenGame(TRUE);

		BrokenForwardMesh->bUpdateSkelWhenNotRendered = FALSE;
		BrokenBackwardMesh->bUpdateSkelWhenNotRendered = FALSE;

		SetCollisionType(COLLIDE_NoCollision);
		CollisionComponent = DoorMainMesh;
		SetCollisionType(COLLIDE_BlockAll);

		DoorBreakState = DBS_Normal;
	}

	bLocked = bWasInitiallyLocked;
	bAITraversing = FALSE;	
	bPlayingLockedAnim = FALSE;
	bPlayingBlockedAnim = FALSE;
	bPlayingAutoCloseAnim = FALSE;
	bBreakTriggered = FALSE;
	bInstantBreak = FALSE;
	bBlocked = FALSE;
	bNetForcedClosed = FALSE;
	DoorUser = NULL;
	bPlayingOpeningSound = FALSE;
	ProceduralAnimElapsedTime=0.0f;
	OpeningIntensity = 0.0f;	
	ActiveBot = NULL;
	bPreciseCloseTiming = FALSE;
	Bots.Empty();
	
	Init();
}

void AOLDoor::SetVolumes(const TArray<class AVolume*>& Volumes)
{
	TArray<AOLSoundEnvironmentVolume*> sndEnvVolumes;
	for (INT i = 0; i < Volumes.Num(); i++)
	{
		AOLSoundEnvironmentVolume* sndEnvVol = Cast<AOLSoundEnvironmentVolume>(Volumes(i));
		if (sndEnvVol)
		{
			sndEnvVolumes.AddItem(sndEnvVol);
		}
	}

	SoundConnectorComp->ConnectVolumes(GetCenterLocation(), sndEnvVolumes);
	Super::SetVolumes(Volumes);
}

void AOLDoor::UpdateMaterials(const TArray<UMaterialInstance*>& Materials)
{
	INT NumMaterials = Max(DoorMainMesh->Materials.Num(), Materials.Num());
	for (INT i = 0; i < NumMaterials; ++i)
	{
		if (i < Materials.Num())
		{
			DoorMainMesh->SetMaterial(i, Materials(i));
		}
		else
		{
			DoorMainMesh->SetMaterial(i, NULL);
		}
	}

	NumMaterials = Max(BreakingForwardMesh->Materials.Num(), Materials.Num());
	for (INT i = 0; i < NumMaterials; ++i)
	{
		if (i < Materials.Num())
		{
			BreakingForwardMesh->SetMaterial(i, Materials(i));
		}
		else
		{
			BreakingForwardMesh->SetMaterial(i, NULL);
		}
	}

	NumMaterials = Max(BrokenForwardMesh->Materials.Num(), Materials.Num());
	for (INT i = 0; i < NumMaterials; ++i)
	{
		if (i < Materials.Num())
		{
			BrokenForwardMesh->SetMaterial(i, Materials(i));
		}
		else
		{
			BrokenForwardMesh->SetMaterial(i, NULL);
		}
	}

	NumMaterials = Max(BreakingBackwardMesh->Materials.Num(), Materials.Num());
	for (INT i = 0; i < NumMaterials; ++i)
	{
		if (i < Materials.Num())
		{
			BreakingBackwardMesh->SetMaterial(i, Materials(i));
		}
		else
		{
			BreakingBackwardMesh->SetMaterial(i, NULL);
		}
	}

	NumMaterials = Max(BrokenBackwardMesh->Materials.Num(), Materials.Num());
	for (INT i = 0; i < NumMaterials; ++i)
	{
		if (i < Materials.Num())
		{
			BrokenBackwardMesh->SetMaterial(i, Materials(i));
		}
		else
		{
			BrokenBackwardMesh->SetMaterial(i, NULL);
		}
	}
}

void AOLDoor::UpdateMeshFromData()
{
	if (DoorMeshType == DMesh_Undefined)
	{
		if (MaterialOverrides.Num() > 0)
		{
			UpdateMaterials(MaterialOverrides);
		}
		return;
	}

	FDoorMeshDirData& Data = bReverseDirection ? DoorMeshData[DoorMeshType].ReversedData : DoorMeshData[DoorMeshType].NormalData;

	DoorMainMesh->StaticMesh = Data.MainMesh;
	
	BreakingForwardMesh->SkeletalMesh = Data.ForwardBreakingMesh;
	BreakingForwardMesh->AnimSets.Reset();
	BreakingForwardMesh->AnimSets.AddItem(Data.ForwardBreakingAnimSet);
	BreakingForwardAnim = Data.ForwardBreakingAnimation;
	
	BrokenForwardMesh->SkeletalMesh = Data.ForwardBrokenMesh;
	BrokenForwardMesh->AnimSets.Reset();
	BrokenForwardMesh->AnimSets.AddItem(Data.ForwardBrokenAnimSet);
	BrokenForwardAnim = Data.ForwardBrokenAnimation;
	
	BreakingBackwardMesh->SkeletalMesh = Data.BackwardBreakingMesh;
	BreakingBackwardMesh->AnimSets.Reset();
	BreakingBackwardMesh->AnimSets.AddItem(Data.BackwardBreakingAnimSet);
	BreakingBackwardAnim = Data.BackwardBreakingAnimation;
	
	BrokenBackwardMesh->SkeletalMesh = Data.BackwardBrokenMesh;
	BrokenBackwardMesh->AnimSets.Reset();
	BrokenBackwardMesh->AnimSets.AddItem(Data.BackwardBrokenAnimSet);
	BrokenBackwardAnim = Data.BackwardBrokenAnimation;
	
	DoorMaterial = DoorMeshData[DoorMeshType].DoorMaterialForSound;
	DefaultOcclusionFactor = DoorMeshData[DoorMeshType].OcclusionFactor;

	// Update Materials
	if (MaterialOverrides.Num() > 0)
	{
		UpdateMaterials(MaterialOverrides);
	}
	else
	{
		UpdateMaterials(DoorMeshData[DoorMeshType].Materials);
	}

	if (bReverseDirection)
	{
		BreakingForwardMesh->SetPhysicsAsset(RightPhysicsAsset);
		BrokenForwardMesh->SetPhysicsAsset(RightPhysicsAsset);
		BreakingBackwardMesh->SetPhysicsAsset(RightPhysicsAsset);
		BrokenBackwardMesh->SetPhysicsAsset(RightPhysicsAsset);
	}
	else
	{
		BreakingForwardMesh->SetPhysicsAsset(LeftPhysicsAsset);
		BrokenForwardMesh->SetPhysicsAsset(LeftPhysicsAsset);
		BreakingBackwardMesh->SetPhysicsAsset(LeftPhysicsAsset);
		BrokenBackwardMesh->SetPhysicsAsset(LeftPhysicsAsset);
	}
}

void AOLDoor::AlignMesh(UMeshComponent* Mesh, UBOOL bInvert)
{
	if (Mesh)
	{
		if (bInvert)
		{
			Mesh->Translation.Y = -Abs(Mesh->Translation.Y);
		}
		else
		{
			Mesh->Translation.Y = Abs(Mesh->Translation.Y);
		}
		Mesh->ConditionalUpdateTransform();
	}
}

void AOLDoor::OrientSubMeshes() 
{
	// set the rotation on the submeshes to match the main one
	INT MainYaw = DoorMainMesh->Rotation.Yaw;

	BreakingForwardMesh->Rotation.Yaw = MainYaw;
	BrokenForwardMesh->Rotation.Yaw = MainYaw;
	BreakingBackwardMesh->Rotation.Yaw = MainYaw;
	BrokenBackwardMesh->Rotation.Yaw = MainYaw;

	BreakingForwardMesh->ConditionalUpdateTransform();
	BrokenForwardMesh->ConditionalUpdateTransform();
	BreakingBackwardMesh->ConditionalUpdateTransform();
	BrokenBackwardMesh->ConditionalUpdateTransform();
}

void AOLDoor::ForceInit()
{
	Init();
}

void AOLDoor::Init()
{
	if (DoorMeshType == DMesh_Undefined)
	{
		UMaterialInterface* Material = DoorMainMesh->GetMaterial(0);
		for (INT i = 0; i < DMesh_MAX; ++i)
		{
			if (DoorMeshData[i].Materials.Num() > 0 && DoorMeshData[i].Materials(0) == Material)
			{
				DoorMeshType = i;
				break;
			}
		}
	}

	UpdateMeshFromData();

	// Careful, this is also called from the editor on PostEditChangeProperty

	AlignMesh(DoorMainMesh, bReverseDirection);

	OrientSubMeshes();
	
	AlignMesh(BreakingForwardMesh, bReverseDirection);
	AlignMesh(BrokenForwardMesh, bReverseDirection);
	AlignMesh(BreakingBackwardMesh, bReverseDirection);
	AlignMesh(BrokenBackwardMesh, bReverseDirection);

	AOLDoor* defaultDoor = (AOLDoor*)(AOLDoor::StaticClass()->GetDefaultActor());
	PlayerOpenedAngle = defaultDoor->PlayerOpenedAngle; 
	MaxOpenAngle = defaultDoor->MaxOpenAngle;

	if (appIsNearlyEqual(InitialOpenAngle, 15.0f, 0.5f))
	{
		InitialOpenAngle = 15.0f;
	}
	else if (appIsNearlyEqual(InitialOpenAngle, PlayerOpenedAngle, 0.5f))
	{
		InitialOpenAngle = PlayerOpenedAngle;
	}
	else if (InitialOpenAngle > 0.0f && InitialOpenAngle < 15.0f)
	{
		InitialOpenAngle = 15.0f; // Not supporting less, for animations to match
	}
	else if (InitialOpenAngle > 15.0f && InitialOpenAngle < PlayerOpenedAngle)
	{
		InitialOpenAngle = PlayerOpenedAngle;
	}

	if (bLocked || MaxOpenAngle < 0.1f)
	{
		ForceOpenRatio(0.0f);
	}
	else
	{
		ForceOpenRatio(InitialOpenAngle/MaxOpenAngle);
	}

	if (DoorType == DT_Locker)
	{
		// lockers need to open at least to playeropenedangle
		MaxOpenAngle = Max(MaxOpenAngle, PlayerOpenedAngle);
		DoorKnobOffset.Y = -48.0f; // hardcode that shit! let's not redo all the lockers
	}

	CurrentSpeed = OpeningSpeed;
	DoorState = DS_Idle;
}

void AOLDoor::TriggerBreakDoorCameraShake()
{
	AOLHero* hero = Utils::GetHero();

	if (!hero)
	{
		return;
	}

	TWEAKABLE FLOAT BreakShakeIntensity = 0.15f;
	TWEAKABLE FLOAT BreakShakeDuration = 0.2f;
	FCameraShakeData camShakeData = hero->Camera->ShakeData;			
	camShakeData.Intensity = BreakShakeIntensity;
	camShakeData.Duration = BreakShakeDuration;

	if (camShakeData.Intensity > 0.001f)
	{
		hero->Camera->ActivateCameraShake(camShakeData, Location);
	}
}

void AOLDoor::TriggerEvent(BYTE eventType, AOLPawn* instigator)
{
	// When the door is being driven by a remote player (bNetDrivenMove), mark all
	// reachable Kismet chains as observer-only so Matinees play visually without
	// re-triggering gameplay side-effects (spawns, game state changes, etc.).
	const UBOOL bRemoteDriven = bNetDrivenMove || bRemoteTrigger;
	if (bRemoteDriven)
	{
		for (INT i = 0; i < GeneratedEvents.Num(); i++)
		{
			UOLSeqEvent_Door* doorEvent = Cast<UOLSeqEvent_Door>(GeneratedEvents(i));
			if (doorEvent && doorEvent->bEnabled)
				MarkChainRemoteObserver(doorEvent, 0);
		}
	}

	for (INT i = 0; i < GeneratedEvents.Num(); i++)
	{
		UOLSeqEvent_Door* doorEvent = Cast<UOLSeqEvent_Door>(GeneratedEvents(i));

		if (doorEvent)
		{
			TArray<INT> indices;
			indices.AddItem((DoorEventType)eventType);
			doorEvent->CheckActivate(this, instigator, FALSE, &indices);
		}
	}

	// Clear observer-only flags on the chain nodes so future local triggers
	// on this door are not affected.
	if (bRemoteDriven)
	{
		for (INT i = 0; i < GeneratedEvents.Num(); i++)
		{
			UOLSeqEvent_Door* doorEvent = Cast<UOLSeqEvent_Door>(GeneratedEvents(i));
			if (doorEvent)
				UnmarkChainRemoteObserver(doorEvent, 0);
		}
	}

	// Send door Kismet event to the other player so their OLSeqEvent_Door fires too.
	// bByPlayer=1 means the local player triggered it; bByPlayer=0 means Kismet/game triggered it.
	// The receiver uses this to decide whether to suppress OLSeqAct_Door actions.
	AOLPlayerController* OLPC = Utils::GetOLPC();
	if (OLPC && !bRemoteTrigger && GeneratedEvents.Num() > 0)
	{
		UBOOL bByPlayer = (instigator && instigator == Cast<AOLPawn>(OLPC->Pawn)) ? TRUE : FALSE;
		FString DoorPath = GetPathName();
		OLPC->eventOnDoorKismetEvent(DoorPath, (INT)eventType, bByPlayer);
	}
}

UBOOL AOLDoor::Tick(FLOAT deltaTime, ELevelTick tickType)
{
	if (tickType == LEVELTICK_ViewportsOnly)
	{
		return Super::Tick(deltaTime, tickType);
	}

	if (bPlayingLockedAnim)
	{
		// map to 0-1
		FLOAT phase = ProceduralAnimElapsedTime / LockedAnimTotalTime;

		phase = 2.0f*Abs(phase - 0.5f);
		phase = (phase > 0.66f) ? 1.52f*(1.33f - phase) : 1.52f*phase;		

		FLOAT ratio = (bReverseDirection ? -2.0f : 2.0f) * (phase - 0.5f) * (LockedAnimAmplitude / MaxOpenAngle);
		ForceOpenRatio(ratio);

		ProceduralAnimElapsedTime += deltaTime;
		if (ProceduralAnimElapsedTime > LockedAnimTotalTime)
		{
			ProceduralAnimElapsedTime = 0.0f;
			ForceOpenRatio(0.0f);
			bPlayingLockedAnim = FALSE;
			DoorState = DS_Idle;
		}
	}
	else if (bPlayingBlockedAnim)
	{
		FLOAT phase = ProceduralAnimElapsedTime / BlockedAnimTotalTime;
		FLOAT procAnim = 1.0f - Abs(appCos(phase * 2.0f * PI));
		FLOAT ratio = procAnim * (BlockedAnimAmplitude / MaxOpenAngle);
		ForceOpenRatio(ratio);

		ProceduralAnimElapsedTime += deltaTime;
		if (ProceduralAnimElapsedTime > BlockedAnimTotalTime)
		{
			ProceduralAnimElapsedTime = 0.0f;
			ForceOpenRatio(0.0f);
			bPlayingBlockedAnim = FALSE;
			DoorState = DS_Idle;
		}
	}
	else if (bPlayingAutoCloseAnim)
	{
		// map to 0-1
		FLOAT phase = ProceduralAnimElapsedTime / AutoCloseAnimTotalTime;

		// map to 0 - 1 - 0
		FLOAT alpha = (1.0f - 2.0f*Abs(phase - 0.5f));
		FLOAT ratio = Utils::SmootherStep(alpha) * (AutoCloseAnimAmplitude / MaxOpenAngle);
		ForceOpenRatio(ratio);

		ProceduralAnimElapsedTime += deltaTime;
		if (ProceduralAnimElapsedTime > AutoCloseAnimTotalTime)
		{
			bPlayingAutoCloseAnim = FALSE;
			ProceduralAnimElapsedTime = 0.0f;
			ForceOpenRatio(0.0f);
			DoorState = DS_Idle;
			NotifyHandlesToRepath();

			if (bUseObstacleOnClose)
			{
				RegisterNavMeshObstacle();
			}
		}
	}
	else if (ShakeData.bActive)
	{
		UpdateDoorShake();
	}
	else
	{
		FLOAT currentError = (TargetOpenRatio - OpenRatio);
		if (!appIsNearlyZero(currentError, KINDA_SMALL_NUMBERF))
		{		
			FLOAT deltaThisFrame = CurrentSpeed * deltaTime / MaxOpenAngle;
			FLOAT newRatio = OpenRatio;

			if (DoorState == DS_Closing && bPreciseCloseTiming)
			{
				if (PreciseCloseDuration <= 0.0f) { bPreciseCloseTiming = FALSE; }
				else newRatio = 1.0f - Saturate(((GWorld->GetTimeSeconds() + deltaTime) - PreciseCloseStartTime) / PreciseCloseDuration);
			}
			else
			{
				if (deltaThisFrame >= Abs(currentError))
				{
					// Reached dest
					newRatio = TargetOpenRatio;
				}
				else
				{
					newRatio = OpenRatio + (currentError > 0.0f ? deltaThisFrame : -deltaThisFrame);			
				}
			}

			FLOAT RatioDiff = newRatio - OpenRatio;

			AOLHero* hero = Utils::GetHero();

			if (RatioDiff > 0.f && hero && DoorUser != hero)
			{
				FVector doorToHero = hero->Location - GetCenterLocation();

				UBOOL closeToCenter = (doorToHero.Size2D() < 100.0f);
				UBOOL bOnStaticOpeningSide = (GetStaticDirection() | doorToHero.SafeNormal2D()) > 0.0f;
				UBOOL bOnDynamicOpeningSide = (GetDynamicDirection() | (hero->Location - GetPivotLocation())) > 0.0f;
				UBOOL bNotAfterPlayerInteract = (GWorld->GetTimeSeconds() > hero->LastCompletedDoorInteractionTime + 1.0f);

				if (closeToCenter && bOnStaticOpeningSide && bOnDynamicOpeningSide && bNotAfterPlayerInteract)
				{
					Utils::GetOLPC()->HeroPawn->ReactToHit(AIOpenDoorKnockback, GetDynamicDirection());
				}
			}
			else if (!UsedByAI())
			{
				TWEAKABLE FLOAT KnockbackDistance = 100.0f;

				for (AController* C = GWorld->GetWorldInfo()->ControllerList; C != NULL; C = C->NextController)
				{
					AOLBot* bot = Cast<AOLBot>(C);

					if (bot && bot->EnemyPawn && bot->EnemyPawn->CanBeKnockedback(TRUE) && bot->EnemyPawn->Location.DistanceSquared(GetCenterLocation()) < Square(KnockbackDistance))
					{			
						FVector toEnemy = bot->EnemyPawn->Location - GetCenterLocation();
						FLOAT dotFromOpen = (GetStaticDirection() | toEnemy.SafeNormal2D());
						
						if (Abs(toEnemy.Z) < 50.0f)
						{
							UBOOL bLocker = (DoorType == DT_Locker);
							UBOOL bClosing = (RatioDiff < 0.0f);
							UBOOL onOpenSide = (dotFromOpen > 0.f);
							if (bClosing && !onOpenSide)
							{
								bot->EnemyPawn->StartDoorKnockback(-GetStaticDirection(), bLocker); // Push away on the other side 
							}
							else if (!bClosing && onOpenSide)
							{
								bot->EnemyPawn->StartDoorKnockback(GetStaticDirection(), bLocker); // Push away on the opening side 
							}
							else if (bClosing && onOpenSide)
							{
								FVector dirKnobToEnemy = (bot->EnemyPawn->Location - GetKnobLocation()).SafeNormal2D();
								if ((dirKnobToEnemy | GetDynamicDirection()) < 0.0f) // in the way for the door to close
								{
									FVector diagDir = (GetStaticDirection() + GetStaticPivotToEdge()).SafeNormal2D();
									bot->EnemyPawn->StartDoorKnockback(diagDir, bLocker); // Push away on the diagonal
								}
							}
						}
					}
				}
			}

			CheckTriggerOpenThresholdReachedEvent(OpenRatio*MaxOpenAngle, newRatio*MaxOpenAngle);
			ApplyOpenRatio(newRatio);
		}
		else if ((DoorState == DS_Opening) || (DoorState == DS_Closing))
		{
			// done with close or open
			
			if (DoorState == DS_Closing)
			{
				// always reset max open angle to 95 when closed
				MaxOpenAngle = 95.0f;
			}

			TargetOpenRatio = OpenRatio;
			DoorState = DS_Idle;
			bPreciseCloseTiming = FALSE;
			NotifyHandlesToRepath();

			if (OpenRatio > KINDA_SMALL_NUMBERF || bUseObstacleOnClose)
			{
				RegisterNavMeshObstacle();
			}
		}

		UAkAudioDevice * AudioDevice = UAkAudioDevice::Get();
		if ( AudioDevice )
		{
			if (DoorState == DS_PlayerInteracting)
			{
				AOLHero * hero = Utils::GetHero();
				UBOOL noMove = appIsNearlyZero(currentError, KINDA_SMALL_NUMBERF) || hero->DesiredMoveDirection.IsNearlyZero();

				if (GWorld->GetTimeSeconds() > LastInteractiveSoundTime + 0.5f)
				{
					if (noMove && bPlayingOpeningSound)
					{
						AudioDevice->PostEvent(SndStopOpening, this, NAME_None);
						bPlayingOpeningSound = FALSE;
						LastInteractiveSoundTime = GWorld->GetTimeSeconds();
					}
					else if (!noMove && !bPlayingOpeningSound)
					{
						if (OpenRatio > 0.025f)
						{
							// rcharpentier - last minute hack
							if (DoorMaterial == OLDM_SecurityDoor)
							{
								AudioDevice->PostEvent(Sounds(OLDM_Metal).SndPush, this, NAME_None);
							}
							else
							{
								AudioDevice->PostEvent(Sounds(DoorMaterial).SndPush, this, NAME_None);
							}
							bPlayingOpeningSound = TRUE;
							OpeningIntensity = 0.5f;
						}
						else if (OpenRatio == 0.0f)
						{
							AudioDevice->PostEvent(Sounds(DoorMaterial).SndOpening, this, NAME_None);
							bPlayingOpeningSound = TRUE;
						}
					}
				}

				AudioDevice->SetRTPCValue(*RTPCOpeningDoorIntensity.ToString(), 100.0f*OpeningIntensity, this);
			}
			else if (bPlayingOpeningSound && currentError <= 0.05f)
			{
				AudioDevice->PostEvent(SndStopOpening, this, NAME_None);
				bPlayingOpeningSound = FALSE;
			}
		}
	}

	if (Utils::GetCheatManager() && Utils::GetCheatManager()->bDebugWaitPoints)
	{
		WaitPointComponent->DrawDebugPoints();
	}

	return Super::Tick(deltaTime, tickType);
}

void AOLDoor::UpdateDoorShake()
{
	FLOAT currentTime = GWorld->GetTimeSeconds();
	FLOAT totalElapsedTime = currentTime - ShakeData.GlobalStartedTime;
	if (ShakeData.TotalDuration > 0.0f && totalElapsedTime > ShakeData.TotalDuration)
	{
		// Done
		StopShaking();
		return;
	}

	if (currentTime >= ShakeData.NextShakeStartTime)
	{
		// start new shake
		FLOAT effectiveRate = RandRange((1.0f - ShakeData.RateVariation)*ShakeData.Rate, (1.0f + ShakeData.RateVariation)*ShakeData.Rate);
		FLOAT delay = effectiveRate > 0.0f ? (1.0f / effectiveRate) : 0.0f;
		ShakeData.ShakeStartedTime = ShakeData.NextShakeStartTime;
		ShakeData.NextShakeStartTime += delay;

		AOLHero* hero = Utils::GetHero();
		if (ShakeData.bShakeCamera && hero && hero->Camera)
		{
			TWEAKABLE FLOAT MaxShakeIntensity = 0.06f;
			FCameraShakeData camShakeData = hero->Camera->ShakeData;			
			camShakeData.Intensity = MaxShakeIntensity * ShakeData.Intensity;
			camShakeData.Duration = ShakeData.ShakeDuration;

			if (camShakeData.Intensity > 0.001f)
			{
				hero->Camera->ActivateCameraShake(camShakeData, Location);
			}
		}

		UAkAudioDevice * AudioDevice = UAkAudioDevice::Get();
		if( AudioDevice )
		{
			Utils::GetSoundEnvManager()->ConditionalRegisterSoundEmitterForDoor(this);

			if (DoorType == DT_Locker)
			{
				AudioDevice->PostEvent(SndLockerBash, this, NAME_None);
			}
			else
			{
				AudioDevice->PostEvent(Sounds(DoorMaterial).SndBash, this, NAME_None);
			}
		}
	}

	FLOAT amplitudeYaw = 0.0f;
	FLOAT amplitudeTrans = 0.0f;

	FLOAT thisShakeElapsedTime = currentTime - ShakeData.ShakeStartedTime;
	if (thisShakeElapsedTime < ShakeData.ShakeDuration)
	{
		amplitudeYaw = ShakeData.AmplitudeYaw * appSin(ShakeData.FrequencyYaw * 2.0f * PI * thisShakeElapsedTime);
		amplitudeTrans = ShakeData.AmplitudeTrans * appSin(ShakeData.FrequencyTrans * 2.0f * PI * thisShakeElapsedTime);
	}

	FLOAT effectiveIntensity = RandRange((1.0f - ShakeData.IntensityVariation)*ShakeData.Intensity, (1.0f + ShakeData.IntensityVariation)*ShakeData.Intensity);
	FLOAT alpha = Clamp(1.0f - (thisShakeElapsedTime / ShakeData.ShakeDuration), 0.0f, 1.0f);
	alpha = appPow(alpha, ShakeData.FadeExp);
	effectiveIntensity *= alpha;

	FLOAT translation = effectiveIntensity * amplitudeTrans;
	UBOOL bTranslationChanged = !appIsNearlyEqual(translation, DoorMainMesh->Translation.X, 0.1f);
	DoorMainMesh->Translation.X = ShakeData.OriginalTranslationX + translation;
	BreakingForwardMesh->Translation.X = ShakeData.OriginalTranslationX + translation;
	BreakingBackwardMesh->Translation.X = ShakeData.OriginalTranslationX + translation;

	FLOAT angleRatio = effectiveIntensity * (amplitudeYaw / MaxOpenAngle);
	if (!appIsNearlyEqual(angleRatio, OpenRatio, 0.001f))
	{
		ForceOpenRatio(angleRatio);
	}
	else if (bTranslationChanged)
	{
		DoorMainMesh->ConditionalUpdateTransform();
		BreakingForwardMesh->ConditionalUpdateTransform();		
		BreakingBackwardMesh->ConditionalUpdateTransform();
	}
}

//////////////////////////////////////////////////////////////////////////
// Interactions
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void AOLDoor::Open(AOLPawn* instigator, FLOAT rotationSpeed, FLOAT targetAngle, UBOOL bNoSound)
{
	if (IsBroken())
	{
		return;
	}

	StopShaking();

	UBOOL bByPlayer = (instigator && instigator->IsA(AOLHero::StaticClass()));

	if (bByPlayer)
	{
		if (!bNetDrivenMove && (bLocked || bBlocked) && !GUnlockDoors)
		{
			return;
		}

		SetTargetOpenRatio( Min(1.0f, PlayerOpenedAngle / MaxOpenAngle) );

		if (DoorType == DT_Locker)
		{
			CurrentSpeed = LockerOpeningSpeed;
		}
		else if (rotationSpeed > 0.0f)
		{
			CurrentSpeed = rotationSpeed;
		}
		else
		{
			CurrentSpeed = OpeningSpeed;
		}
	}
	else
	{
		SetTargetOpenRatio(targetAngle > 0.0f ? targetAngle/MaxOpenAngle : 1.0f);
		CurrentSpeed = rotationSpeed > 0.0f ? rotationSpeed : OpeningSpeed;
	}

	UnregisterNavmeshObstacle();

	UAkAudioDevice * audioDevice = UAkAudioDevice::Get();
	if (!bNoSound && audioDevice && (!instigator || !instigator->IsA(AOLEnemyPawn::StaticClass())) && DoorType != DT_Locker) // temp for now, as sound is 2d-only
	{
		audioDevice->PostEvent(Sounds(DoorMaterial).SndPush, this, NAME_None);
		bPlayingOpeningSound = TRUE;
		OpeningIntensity = 1.0f;
	}

	if (DoorState == DS_Idle)
	{
		TriggerEvent(DET_StartOpening, instigator);
	}

	DoorState = DS_Opening;

	TriggerEvent(DET_Opened, instigator);

	// Notify the local player controller so multiplayer can broadcast this to peers.
	if (instigator && instigator->IsA(AOLHero::StaticClass()))
	{
		AOLPlayerController* OLPC = Utils::GetOLPC();
		if (OLPC && OLPC->HeroPawn == instigator)
			OLPC->eventOnLocalDoorOpen(this);
	}
}

void AOLDoor::CloseQuiet(class AOLPawn* instigator, FLOAT closeStartTime, FLOAT closeDuration)
{
	if (closeDuration <= 0.0f)
		return;

	Close(instigator, (95.0f / closeDuration), CST_Quiet);

	if (DoorState == DS_Closing)
	{
		bPreciseCloseTiming = TRUE;
		PreciseCloseStartTime = closeStartTime;
		PreciseCloseDuration = closeDuration;
	}
}

void AOLDoor::Close(AOLPawn* instigator, FLOAT rotationSpeed, EClosingSoundType closingSoundType)
{
	if (IsBroken())
	{
		return;
	}

	if (UsedByAI() && (instigator == NULL || !instigator->IsA(AOLEnemyPawn::StaticClass())))
	{
		return;
	}

	SetTargetOpenRatio(0.0f);
	CurrentSpeed = rotationSpeed > 0.0f ? rotationSpeed : ((DoorType == DT_Locker) ? LockerClosingSpeed : ClosingSpeed);

	if (instigator == NULL)
	{
		NotifyHandlesToWait();
	}

	UnregisterNavmeshObstacle();

	UAkAudioDevice * audioDevice = UAkAudioDevice::Get();

	UBOOL bPlaySound = (closingSoundType != CST_NoSound) && (!instigator || !instigator->IsA(AOLEnemyPawn::StaticClass())) && DoorType != DT_Locker;
		
	if (bPlaySound && audioDevice) // currently only 2d sound, so player-only
	{	
		if (bPlayingOpeningSound)
		{
			audioDevice->PostEvent(SndStopOpening, this, NAME_None);
			bPlayingOpeningSound = FALSE;
		}

		if (OpenRatio > 0.05f)
		{
			if (closingSoundType == CST_Quiet)
			{
				audioDevice->PostEvent(Sounds(DoorMaterial).SndSlowClose, this, NAME_None);
			}
			else
			{
				audioDevice->PostEvent(Sounds(DoorMaterial).SndClosing, this, NAME_None);
			}
		}
	}

	TriggerEvent(DET_Closed, instigator);
	DoorState = DS_Closing;
	bPreciseCloseTiming = FALSE;

	// Notify the local player controller so multiplayer can broadcast this to peers.
	if (instigator && instigator->IsA(AOLHero::StaticClass()))
	{
		AOLPlayerController* OLPC = Utils::GetOLPC();
		if (OLPC && OLPC->HeroPawn == instigator)
			OLPC->eventOnLocalDoorClose(this);
	}

	// If this door has a non-zero default angle, mark it as intentionally closed
	// so Kismet "Open" / "Force Open" actions don't re-open it on checkpoint restore.
	if (InitialOpenAngle > 0.5f)
		bNetForcedClosed = TRUE;
}

void AOLDoor::StartedInteractiveOpening(AOLPawn* instigator)
{
	StopShaking();

	UnregisterNavmeshObstacle();

	DoorState = DS_PlayerInteracting;
	bPlayingOpeningSound = FALSE;
	OpeningIntensity = 1.0f;

	TriggerEvent(DET_StartOpening, instigator);
}

void AOLDoor::TriedOpening(AOLPawn* instigator)
{
	if (bBlocked)
	{
		PlayBlockedAnimation();

		UAkAudioDevice * AudioDevice = UAkAudioDevice::Get();
		if( AudioDevice )
		{
			AudioDevice->PostEvent(Sounds(DoorMaterial).SndLocked, this, NAME_None); // TEMP - should be another sound!
		}
	}
	else
	{
		// should be bLocked, but beware of kismet that may have unlocked the door during the animation
		PlayLockedAnimation();
		TriggerEvent(DET_TriedOnLocked, instigator);

		UAkAudioDevice * AudioDevice = UAkAudioDevice::Get();
		if( AudioDevice )
		{
			AudioDevice->PostEvent(Sounds(DoorMaterial).SndLocked, this, NAME_None);
		}
	}
}

void AOLDoor::CheckTriggerOpenThresholdReachedEvent(FLOAT prevAngle, FLOAT newAngle)
{
	if (newAngle > prevAngle)
	{
		for (INT i = 0; i < GeneratedEvents.Num(); i++)
		{
			UOLSeqEvent_Door* doorEvent = Cast<UOLSeqEvent_Door>(GeneratedEvents(i));

			if (doorEvent && (!doorEvent->bOpenThresholdOnlyForInteractiveOpen || DoorState == DS_PlayerInteracting || DoorState == DS_Opening))
			{
				if (doorEvent->OpenThreshold > prevAngle && doorEvent->OpenThreshold <= newAngle)
				{
					TArray<INT> indices;
					indices.AddItem(4); // "Open Threshold Reached"
					doorEvent->CheckActivate(this, Utils::GetHero(), FALSE, &indices);
				}
			}
		}
	}
}

void AOLDoor::PlayLockedAnimation()
{
	bPlayingLockedAnim = TRUE;
	ProceduralAnimElapsedTime = 0.0f;
	DoorState = DS_Animating;
}

void AOLDoor::PlayBlockedAnimation()
{
	bPlayingBlockedAnim = TRUE;
	ProceduralAnimElapsedTime = 0.0f;
	DoorState = DS_Animating;
}

void AOLDoor::PlayAutoCloseAnim()
{
	bPlayingAutoCloseAnim = TRUE;
	ProceduralAnimElapsedTime = 0.0f;
	DoorState = DS_Animating;

	NotifyHandlesToWait();
}

void AOLDoor::StartShaking(FDoorShakeData shakeParams, UBOOL bSwitchToBreakingMesh, UBOOL bReversed, UBOOL bFromAI)
{
	if ((!bFromAI && UsedByAI()) || !DoorMainMesh || IsBroken() || (DoorState != DS_Animating && DoorState != DS_Idle) || !IsClosed())
	{
		return;
	}

	appMemZero(ShakeData);
	ShakeData = shakeParams;

	USkeletalMeshComponent* breakingMesh = bReversed ? BreakingBackwardMesh : BreakingForwardMesh;
	
	if (bSwitchToBreakingMesh && (DoorBreakState == DBS_Normal) && breakingMesh && breakingMesh->SkeletalMesh && breakingMesh->IsAttached())
	{
		DoorBreakState = DBS_Breaking;
		DoorMainMesh->SetHiddenGame(TRUE);
		breakingMesh->SetHiddenGame(FALSE);
	}

	DoorState = DS_Animating;

	ShakeData.bActive = TRUE;
	ShakeData.GlobalStartedTime = GWorld->GetTimeSeconds();
	ShakeData.ShakeStartedTime = ShakeData.GlobalStartedTime;
	ShakeData.OriginalTranslationX = DoorMainMesh->Translation.X;
	ShakeData.NextShakeStartTime = ShakeData.ShakeStartedTime; // start a new shake now
}

void AOLDoor::StopShaking()
{
	if (ShakeData.bActive)
	{
		DoorMainMesh->Translation.X = ShakeData.OriginalTranslationX;
		BreakingForwardMesh->Translation.X = ShakeData.OriginalTranslationX;
		BreakingBackwardMesh->Translation.X = ShakeData.OriginalTranslationX;
		ForceOpenRatio(0.0f);
		appMemZero(ShakeData);

		DoorState = DS_Idle;
	}
}

void AOLDoor::InstantBreak()
{
	bInstantBreak = TRUE;

	if (ActiveBot != NULL)
	{
		if (ActiveBot->bBreachingDoor && ActiveBot->EnemyPawn->bUsesDoorBashLoop)
		{
			ActiveBot->EnemyPawn->CancelSpecialMove();
		}
	}
}

void AOLDoor::CancelBash(ECancelBashDirection CancelDirection)
{
	if (ActiveBot != NULL && !ActiveBot->IsPendingKill() && ActiveBot->bBreachingDoor && ActiveBot->EnemyPawn->bUsesDoorBashLoop)
	{
		UOLAICmd_MoveAbility_Door* DoorMoveCommand = Cast<UOLAICmd_MoveAbility_Door>(ActiveBot->FindCommandOfClass(UOLAICmd_MoveAbility_Door::StaticClass()));

		switch(CancelDirection)
		{
		case ECBD_Both:
			ActiveBot->bCancelBash = TRUE;
			break;
		case ECBD_Forward:
			if (DoorMoveCommand != NULL && !DoorMoveCommand->bReversed)
			{
				ActiveBot->bCancelBash = TRUE;
			}
			break;
		case ECBD_Backward:
			if (DoorMoveCommand != NULL && DoorMoveCommand->bReversed)
			{
				ActiveBot->bCancelBash = TRUE;
			}
			break;
		}
	}
}

//////////////////////////////////////////////////////////////////////////
// Control
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void AOLDoor::ForceOpen()
{
	StopShaking();
	bNetForcedClosed = FALSE;
	ForceOpenRatio(1.0f);
	DoorState = DS_Idle;
	NotifyHandlesToRepath();
}

void AOLDoor::ForceClose()
{
	if (!UsedByAI())
	{
		ForceOpenRatio(0.0f);
	}
	if (InitialOpenAngle > 0.5f)
		bNetForcedClosed = TRUE;
	DoorState = DS_Idle;
	NotifyHandlesToRepath();
}

void AOLDoor::SetTargetOpenAngle(FLOAT newAngle)
{
	SetTargetOpenRatio(Clamp(newAngle/MaxOpenAngle, 0.0f, 1.0f));
}

void AOLDoor::SetTargetOpenRatio(FLOAT newTarget)
{
	TargetOpenRatio = newTarget;	
}

void AOLDoor::ForceOpenAngle(FLOAT newAngle)
{
	ForceOpenRatio(Clamp(newAngle/MaxOpenAngle, 0.0f, 1.0f));
}

void AOLDoor::ForceOpenRatio(FLOAT newOpenRatio)
{
	if (IsBroken())
	{
		return;
	}

	TargetOpenRatio = newOpenRatio;
	ApplyOpenRatio(newOpenRatio);

	// Don't modify this if shaking since its expensive and shaking calls this function every frame.
	if (!ShakeData.bActive && !bPlayingLockedAnim && !bPlayingBlockedAnim)
	{
		UnregisterNavmeshObstacle();

		if (GWorld && GWorld->HasBegunPlay() && (OpenRatio > KINDA_SMALL_NUMBERF || bUseObstacleOnClose))
		{
			RegisterNavMeshObstacle();
		}
	}
}


void AOLDoor::NetOpen(FLOAT RotationSpeed, FLOAT TargetAngle)
{
	if (IsBroken())
		return;
	UnregisterNavmeshObstacle();
	// Always open to MaxOpenAngle — TargetAngle may be a mid-move snapshot from the sender.
	SetTargetOpenRatio(1.0f);
	CurrentSpeed = RotationSpeed > 0.0f ? RotationSpeed : OpeningSpeed;
	if (DoorState != DS_Opening)
	{
		TriggerEvent(DET_StartOpening, Utils::GetHero());
		DoorState = DS_Opening;
	}
}

void AOLDoor::NetClose(FLOAT RotationSpeed)
{
	if (IsBroken())
		return;
	UnregisterNavmeshObstacle();
	SetTargetOpenRatio(0.0f);
	CurrentSpeed = RotationSpeed > 0.0f ? RotationSpeed : ClosingSpeed;
	if (DoorState != DS_Closing)
		DoorState = DS_Closing;
}

void AOLDoor::NetReplicateOpen(FLOAT RotationSpeed)
{
	AOLHero* hero = Utils::GetHero();
	Open(hero, RotationSpeed > 0.0f ? RotationSpeed : OpeningSpeed, MaxOpenAngle, TRUE);
}

void AOLDoor::NetReplicateClose(FLOAT RotationSpeed)
{
	AOLHero* hero = Utils::GetHero();
	Close(hero, RotationSpeed > 0.0f ? RotationSpeed : ClosingSpeed, CST_NoSound);
}

void AOLDoor::NetReplicateInteractStart()
{
	AOLHero* hero = Utils::GetHero();
	StartedInteractiveOpening(hero);
}

void AOLDoor::SetNetTargetOpenRatio(FLOAT newOpenRatio)
{
	if (IsBroken())
	{
		return;
	}

	// Mirror the bookkeeping Open()/Close() do (DoorState/CurrentSpeed/navmesh obstacle),
	// minus their side effects (sounds, AI knockback, instigator-specific logic), which
	// don't apply to a remote player's action. Without DoorState being set here, Tick()'s
	// "reached target" branch (which calls NotifyHandlesToRepath and re-registers the
	// navmesh obstacle) never runs, leaving the door's interaction/pathing state stale
	// after a net-driven open/close.
	if (appIsNearlyEqual(newOpenRatio, OpenRatio, 0.01f))
		return;

	TargetOpenRatio = newOpenRatio;
	CurrentSpeed = (newOpenRatio > OpenRatio) ? OpeningSpeed : ClosingSpeed;
	DoorState = (newOpenRatio > OpenRatio) ? DS_Opening : DS_Closing;
	UnregisterNavmeshObstacle();
}

void AOLDoor::SetNetInteractiveAngle(FLOAT newAngle, FLOAT speed)
{
	if (IsBroken())
		return;
	TargetOpenRatio = Clamp(newAngle / MaxOpenAngle, 0.0f, 1.0f);
	CurrentSpeed = speed > 0.0f ? speed : OpeningSpeed;
}

void AOLDoor::ApplyOpenRatio(FLOAT newOpenRatio)
{
	if (newOpenRatio != OpenRatio)
	{
		OpenRatio = newOpenRatio;
		FLOAT doorRelativeYaw = OpenRatio * DEG_TO_UNR * MaxOpenAngle;
		DoorMainMesh->Rotation.Yaw = bReverseDirection ? doorRelativeYaw : -doorRelativeYaw;
		DoorMainMesh->ConditionalUpdateTransform();

		OrientSubMeshes();
	}
}

void AOLDoor::SoftDestroy()
{
	StopShaking();
	ForceOpenRatio(0.0f);
	DoorMainMesh->SetHiddenGame(TRUE);
	SetCollisionType(COLLIDE_NoCollision);
	DoorState = DS_Idle;
	DoorBreakState = DBS_Broken;

	BreakingForwardMesh->SetHiddenGame(TRUE);
	BreakingBackwardMesh->SetHiddenGame(TRUE);
	BrokenForwardMesh->SetHiddenGame(TRUE);
	BrokenBackwardMesh->SetHiddenGame(TRUE);
	
	UnregisterNavmeshObstacle();
}

void AOLDoor::ForceBreakDoor(UBOOL bForward)
{
	StopShaking();
	ForceOpenRatio(0.0f);
	DoorMainMesh->SetHiddenGame(TRUE);
	SetCollisionType(COLLIDE_NoCollision);
	DoorState = DS_Idle;
	DoorBreakState = DBS_Broken;

	BreakingForwardMesh->SetHiddenGame(TRUE);
	BreakingBackwardMesh->SetHiddenGame(TRUE);

	if (bForward)
	{
		BrokenForwardMesh->SetHiddenGame(FALSE);
		BrokenBackwardMesh->SetHiddenGame(TRUE);

		UAnimNodeSequence* animSeq = Cast<UAnimNodeSequence>(BrokenForwardMesh->Animations);
		if (animSeq)
		{
			animSeq->SetAnim(BrokenForwardAnim);
			animSeq->PlayAnim(FALSE, 20.0f);
		}
	}
	else
	{
		BrokenForwardMesh->SetHiddenGame(TRUE);
		BrokenBackwardMesh->SetHiddenGame(FALSE);

		UAnimNodeSequence* animSeq = Cast<UAnimNodeSequence>(BrokenBackwardMesh->Animations);
		if (animSeq)
		{
			animSeq->SetAnim(BrokenBackwardAnim);
			animSeq->PlayAnim(FALSE, 20.0f);
		}
	}
	
	UnregisterNavmeshObstacle();
}

//////////////////////////////////////////////////////////////////////////
// Location and setup info
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

FVector AOLDoor::GetStaticDirection() const
{
	return -Rotation.Vector();
}

FVector AOLDoor::GetDynamicDirection() const
{
	return -DoorMainMesh->LocalToWorld.GetAxis(0);
}

FVector AOLDoor::GetCenterLocation() const
{
	return Location + (DoorType == DT_Locker ? StaticOffsetFwdToCenterLocker : StaticOffsetFwdToCenterNormal)*GetStaticDirection(); // location on the ground in the middle of the door frame
}

FVector AOLDoor::ScriptGetCenterLocation()
{
	return GetCenterLocation();
}

FVector AOLDoor::GetStaticPivotToEdge() const
{
	return bReverseDirection ? Rotation.Right() : -Rotation.Right();
}

FLOAT AOLDoor::GetDoorWidth() const
{
	return 2.0f*DoorMainMesh->StaticMesh->Bounds.BoxExtent.Y;
}

FVector AOLDoor::GetKnobLocation() const
{
	FVector doorKnobOffset = DoorKnobOffset;
	if (bReverseDirection)
	{
		doorKnobOffset.Y *= -1.0f;
	}
	return DoorMainMesh->LocalToWorld.TransformFVector(doorKnobOffset);
}

FVector AOLDoor::GetEdgeLocation() const
{
	const FMatrix& doorMeshTM = DoorMainMesh->LocalToWorld;
	FVector doorEdge = GetPivotLocation() + (bReverseDirection ? 2.0f : -2.0f)*DoorMainMesh->StaticMesh->Bounds.BoxExtent.Y*doorMeshTM.GetAxis(1);
	return doorEdge;
}

FVector AOLDoor::GetStaticEdgeLocation() const
{
	return GetCenterLocation() + 0.5f*GetDoorWidth()*GetStaticPivotToEdge();
}

FVector AOLDoor::GetPivotLocation() const
{
	return GetCenterLocation() - 0.5f*GetDoorWidth()*GetStaticPivotToEdge();
}

FVector AOLDoor::GetStaticKnobLocation() const
{
	return GetCenterLocation() - 0.5f*DoorKnobOffset.Y*GetStaticPivotToEdge();
}

FLOAT AOLDoor::GetOpenAngle() 
{ 
	return OpenRatio*MaxOpenAngle; 
}

FLOAT AOLDoor::GetOcclusionFactor() const
{
	return Saturate((ExplicitOcclusionFactor >= 0.0f) ? ExplicitOcclusionFactor : DefaultOcclusionFactor);
}

UBOOL AOLDoor::CanClose(AOLPawn* instigator)
{
	if (bCantClose)
	{
		return FALSE;
	}

	TArray<AOLPawn*> pawns;
	GetEncroachingPawns(pawns);

	for (INT i = 0; i < pawns.Num(); i++)
	{
		if (pawns(i) != instigator)
		{
			return FALSE;
		}
	}

	return TRUE;
}

UBOOL AOLDoor::UsedByAI() const 
{ 
	return DoorUser && DoorUser->IsA(AOLEnemyPawn::StaticClass()); 
}

INT AOLDoor::GetEncroachingPawns(TArray<AOLPawn*>& pawns)
{
	TWEAKABLE FLOAT SmallBufferRoom = 20.0f;
	TWEAKABLE FLOAT LargeBufferRoom = 50.0f;

	FVector pivot = GetPivotLocation();
	FVector openDir = GetStaticDirection();
	FVector pivotToEdge = GetStaticPivotToEdge();

	FLOAT minPrlDist = -SmallBufferRoom;
	FLOAT maxPrlDist = GetDoorWidth() + LargeBufferRoom;
	FLOAT minPerpDist = -LargeBufferRoom;
	FLOAT maxPerpDist = GetDoorWidth() + SmallBufferRoom;

	pawns.Empty();

	for (AController* controller = GWorld->GetWorldInfo()->ControllerList; controller != NULL; controller = controller->NextController)
	{
		AOLPawn* pawn = Cast<AOLPawn>(controller->Pawn);

		if (pawn)
		{
			AOLHero* hero = Cast<AOLHero>(pawn);
			if (hero && hero->bIsDummyPawn)
				continue;

			FVector pivotToPawn = pawn->Location - pivot;

			if (Abs(pivotToPawn.Z) > 30.0f)
			{
				continue;
			}

			FLOAT distPrl = (pivotToPawn | pivotToEdge);
			FLOAT distPerp = (pivotToPawn | openDir);

			if ((distPrl >= minPrlDist) && (distPrl <= maxPrlDist) && (distPerp >= minPerpDist) && (distPerp <= maxPerpDist))
			{
				// inside the "zone"
				pawns.AddItem(pawn);
			}
		}
	}

	return pawns.Num();
}

//////////////////////////////////////////////////////////////////////////
// Navmesh stuff
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////


void AOLDoor::RegisterNavMeshObstacle()
{
	if (!bObstacleRegistered)
	{
		RegisterObstacleWithNavMesh();
		bObstacleRegistered = TRUE;
	}
}

void AOLDoor::UnregisterNavmeshObstacle()
{
	if (bObstacleRegistered)
	{
		UnregisterObstacleWithNavMesh();
		bObstacleRegistered = FALSE;
	}
}


void AOLDoor::PostSubMeshUpdateForOwningPoly(FNavMeshPathObjectEdge* Edge, FNavMeshPolyBase* Poly, UNavigationMeshBase* New_SubMesh)
{
	FVector StartVert, EndVert;
	FNavMeshPolyBase *StartPoly=NULL, *EndPoly=NULL, *StartBasePoly=NULL, *EndBasePoly=NULL;
	APylon* Py =NULL;
	BOOL OneWayEdge = TRUE;

	if (Edge->InternalPathObjectID == 0)
	{
		 StartVert = Edge->GetVertLocation(0);
		 EndVert = Edge->GetVertLocation(1);

		 UNavigationHandle::GetPylonAndPolyFromPos(Edge->GetPoly0()->GetPolyCenter(),AScout::GetGameSpecificDefaultScoutObject()->WalkableFloorZ,Py,StartPoly);
		 UNavigationHandle::GetPylonAndPolyFromPos(Edge->GetPoly1()->GetPolyCenter(),AScout::GetGameSpecificDefaultScoutObject()->WalkableFloorZ,Py,EndPoly);
	}
	else
	{
		StartVert = Edge->GetEdgeCenter();

		if (Edge->InternalPathObjectID == 1)
		{
			EndVert = Edge0Dest;
		}
		else
		{
			EndVert = Edge1Dest;
		}

		StartPoly = New_SubMesh->GetPolyFromPoint(StartVert,AScout::GetGameSpecificDefaultScoutObject()->WalkableFloorZ);
		if (StartPoly == NULL)
		{
			UNavigationHandle::GetPylonAndPolyFromPos(StartVert,AScout::GetGameSpecificDefaultScoutObject()->WalkableFloorZ,Py,StartPoly);

			if (StartPoly == NULL)
			{
				StartVert = EndVert + (StartVert - EndVert) * 0.75f;
				UNavigationHandle::GetPylonAndPolyFromPos(StartVert,AScout::GetGameSpecificDefaultScoutObject()->WalkableFloorZ,Py,StartPoly);
			}

			if (StartPoly != NULL && StartPoly->IsSubMeshPoly())
			{
				StartBasePoly = StartPoly->GetParentPoly();

				check(StartBasePoly != StartPoly);
			}
		}

		EndPoly = New_SubMesh->GetPolyFromPoint(EndVert,AScout::GetGameSpecificDefaultScoutObject()->WalkableFloorZ);
		if (EndPoly == NULL)
		{
			UNavigationHandle::GetPylonAndPolyFromPos(EndVert,AScout::GetGameSpecificDefaultScoutObject()->WalkableFloorZ,Py,EndPoly);

			if (EndPoly == NULL)
			{
				EndVert = StartVert + (EndVert - StartVert) * 0.75f;
				UNavigationHandle::GetPylonAndPolyFromPos(EndVert,AScout::GetGameSpecificDefaultScoutObject()->WalkableFloorZ,Py,EndPoly);
			}

			if (EndPoly != NULL && EndPoly->IsSubMeshPoly())
			{
				EndBasePoly = EndPoly->GetParentPoly();

				check(EndBasePoly != EndPoly);
			}
		}

		EndVert = StartVert;
	}

	if( StartPoly != NULL && EndPoly != NULL && StartPoly != EndPoly )
	{
		static TArray<FNavMeshPolyBase*> ConnectedPolys;
		ConnectedPolys.Reset(2);
		ConnectedPolys.AddItem(StartPoly);
		ConnectedPolys.AddItem(EndPoly);

		static TArray<FNavMeshPathObjectEdge*> ReturnEdges;
		ReturnEdges.Reset(ReturnEdges.Num());

		StartPoly->NavMesh->AddDynamicCrossPylonEdge<FNavMeshPathObjectEdge>(StartVert, EndVert, ConnectedPolys, Edge->EffectiveEdgeLength, Edge->EdgeGroupID, OneWayEdge, &ReturnEdges);

		FNavMeshPathObjectEdge* NewEdge = NULL;

		for (INT i = 0; i < ReturnEdges.Num(); ++i)
		{
			NewEdge = ReturnEdges(i);

			if (NewEdge != NULL)
			{
				NewEdge->PathObject = Edge->PathObject;
				NewEdge->InternalPathObjectID = Edge->InternalPathObjectID;

				StartPoly->NavMesh->SetNeedsRecompute(TRUE);
			}
		}

		// Add Edge to base poly too. If both are non-NULL (can happen in multiplayer when
		// navmesh state differs), StartBasePoly takes priority — EndBasePoly is ignored.
		if (StartBasePoly != NULL || EndBasePoly != NULL)
		{
			if (StartBasePoly != NULL)
			{
				StartPoly = StartBasePoly;
			}
			else if (EndBasePoly != NULL)
			{
				EndPoly = EndBasePoly;
			}

			ConnectedPolys.Reset(2);
			ConnectedPolys.AddItem(StartPoly);
			ConnectedPolys.AddItem(EndPoly);

			static TArray<FNavMeshPathObjectEdge*> ReturnEdges;
			ReturnEdges.Reset(ReturnEdges.Num());

			StartPoly->NavMesh->AddDynamicCrossPylonEdge<FNavMeshPathObjectEdge>(StartVert, EndVert, ConnectedPolys, Edge->EffectiveEdgeLength, Edge->EdgeGroupID, OneWayEdge, &ReturnEdges);

			FNavMeshPathObjectEdge* NewEdge = NULL;

			for (INT i = 0; i < ReturnEdges.Num(); ++i)
			{
				NewEdge = ReturnEdges(i);

				if (NewEdge != NULL)
				{
					NewEdge->PathObject = Edge->PathObject;
					NewEdge->InternalPathObjectID = Edge->InternalPathObjectID;

					StartPoly->NavMesh->SetNeedsRecompute(TRUE);
				}
			}
		}
	}
}

INT AOLDoor::CostFor( const FNavMeshPathParams& PathParams, const FVector& PreviousPoint, FVector& out_PathEdgePoint, FNavMeshPathObjectEdge* Edge, FNavMeshPolyBase* SourcePoly )
{
	if (Edge->InternalPathObjectID < 1 || Edge->InternalPathObjectID > 2)
	{
		return Edge->FNavMeshEdgeBase::CostFor(PathParams, PreviousPoint, out_PathEdgePoint, SourcePoly);
	}

	FVector OtherDest;
	if (Edge->InternalPathObjectID == 1)
	{
		out_PathEdgePoint = Edge1Dest;
		OtherDest = Edge0Dest;
	}
	else
	{
		out_PathEdgePoint = Edge0Dest;
		OtherDest = Edge1Dest;
	}

	FLOAT Multiplier = 1.2f;
	AOLBot* Bot = Cast<AOLBot>(PathParams.Interface->GetUObjectInterfaceInterface_NavigationHandle());
	if (Bot != NULL && Edge->InternalPathObjectID != 0)
	{
		if (GetOpenAngle() <= 80.0f && !IsBroken())
		{
			Multiplier *= Bot->EnemyPawn->DoorClosedPathMultiplier;
		}
	}

	return appTrunc((out_PathEdgePoint - PreviousPoint).Size() + (out_PathEdgePoint - OtherDest).Size() * Multiplier);
}

UBOOL AOLDoor::Supports( const FNavMeshPathParams& PathParams, FNavMeshPolyBase* CurPoly, FNavMeshPathObjectEdge* Edge, FNavMeshEdgeBase* PredecessorEdge)
{
	AOLBot* Bot = Cast<AOLBot>(PathParams.Interface->GetUObjectInterfaceInterface_NavigationHandle());
	if(Bot != NULL && (!bBlocked || Edge->InternalPathObjectID == 0))
	{
		return TRUE;
	}

	return FALSE;
}

UBOOL AOLDoor::GetEdgeDestination( const FNavMeshPathParams& PathParams, FLOAT EntityRadius, const FVector& InfluencePosition, const FVector& EntityPosition, FVector& out_EdgeDest,	FNavMeshPathObjectEdge* Edge, UNavigationHandle* Handle)
{
	if (Edge->InternalPathObjectID < 1 || Edge->InternalPathObjectID > 2)
	{
		return FALSE;
	}

	if (Edge->InternalPathObjectID == 1)
	{
		out_EdgeDest = Edge1Dest;
	}
	else
	{
		out_EdgeDest = Edge0Dest;
	}

	return TRUE;
}

UBOOL AOLDoor::GetFinalEdgeDestination( FVector& out_EdgeDest, FNavMeshPathObjectEdge* Edge )
{
	if (Edge->InternalPathObjectID < 1 || Edge->InternalPathObjectID > 2)
	{
		return FALSE;
	}

	if (Edge->InternalPathObjectID == 1)
	{
		out_EdgeDest = Edge0Dest;
	}
	else
	{
		out_EdgeDest = Edge1Dest;
	}

	return TRUE;
}

UBOOL AOLDoor::PrepareMoveThru( class IInterface_NavigationHandle* Interface, FVector& out_MovePt, FNavMeshPathObjectEdge* Edge )
{
	if (Edge->InternalPathObjectID == 0)
	{
		return FALSE;
	}

	AOLBot* Bot = Cast<AOLBot>(Interface->GetUObjectInterfaceInterface_NavigationHandle());
	if(Bot != NULL)
	{
		UBOOL bReversed = Edge->InternalPathObjectID == 2;
		FVector Destination = Edge->GetEdgeCenter();
		if (!IsOpened())
		{
			if (eventShouldBreak(Bot))
			{
				FLOAT Distance = Bot->EnemyPawn->EnemyMode == EM_Chase ? Bot->EnemyPawn->DoorBreakDistance : Bot->EnemyPawn->DoorChasingBreakDistance;

				if (bReversed)
				{
					Distance = (Distance + 15.0f) * -1.f;
				}

				Destination = LocalToWorld().TransformFVector(FVector(Distance, 0.0f, 0.0f));
			}
			else if (bReversed)
			{
				Destination = LocalToWorld().TransformFVector(FVector(-Bot->EnemyPawn->DoorOpenDistancePull, 0.0f, 0.0f));
			}
			else
			{
				Destination = LocalToWorld().TransformFVector(FVector(Bot->EnemyPawn->DoorOpenDistancePush, 0.0f, 0.0f));
			}
		}

		UOLAICmd_MoveAbility_Door* Cmd = UOLAICmd_MoveAbility_Door::StaticClass()->GetDefaultObject<UOLAICmd_MoveAbility_Door>()->eventMoveThruDoor(Bot, this, Destination, bReversed);
		Bot->eventQueueAICommand(Cmd);
	}

	return TRUE;
}

UBOOL AOLDoor::DrawEdge( FDebugRenderSceneProxy* DRSP, FColor C, FVector DrawOffset, FNavMeshPathObjectEdge* Edge )
{
	if(Edge->InternalPathObjectID < 1 || Edge->InternalPathObjectID > 2)
	{
		return FALSE;
	}

	FVector Start = Edge->GetEdgeCenter();

	FBoxSphereBounds& MeshBounds = DoorMainMesh->StaticMesh->Bounds;
	FVector End;
	if (Edge->InternalPathObjectID == 1)
	{
		End = LocalToWorld().TransformFVector(FVector(-PathPointOffset, 0, 0));
		End.Z = Location.Z;
		new(DRSP->ArrowLines) FDebugRenderSceneProxy::FArrowLine(Start,End,FColor(0,255,128));
	}
	else
	{
		End = LocalToWorld().TransformFVector(FVector(PathPointOffset, 0, 0));
		End.Z = Location.Z;
		new(DRSP->ArrowLines) FDebugRenderSceneProxy::FArrowLine(Start,End,FColor(0,255,128));
	}

	return TRUE;
}

UBOOL AOLDoor::AllowMoveToNextEdge(FNavMeshPathParams& PathParams, UBOOL bInPoly0, UBOOL bInPoly1)
{
	if (IsBroken() || GetOpenAngle() > 80.0f)
	{
		return TRUE;
	}

	return FALSE;
}

UBOOL AOLDoor::GetMeshSplittingPoly( TArray<FVector>& Poly, FLOAT& PolyHeight )
{
	if (!DoorMainMesh->StaticMesh || !bSplitNavMesh)
	{
		return FALSE;
	}

	FBoxSphereBounds& MeshBounds = DoorMainMesh->StaticMesh->Bounds;
		
	FVector newVert = LocalToWorld().TransformFVector(FVector(MeshBounds.BoxExtent.X, MeshBounds.BoxExtent.Y, 0));
	Poly.AddItem(newVert);
	newVert = LocalToWorld().TransformFVector(FVector(-MeshBounds.BoxExtent.X, MeshBounds.BoxExtent.Y, 0));
	Poly.AddItem(newVert);
	newVert = LocalToWorld().TransformFVector(FVector(-MeshBounds.BoxExtent.X, -MeshBounds.BoxExtent.Y, 0));
	Poly.AddItem(newVert);
	newVert = LocalToWorld().TransformFVector(FVector(MeshBounds.BoxExtent.X, -MeshBounds.BoxExtent.Y, 0));
	Poly.AddItem(newVert);

	PolyHeight = MeshBounds.BoxExtent.Z *2.0f;

	return TRUE;
}

void AOLDoor::CreateEdgesForPathObject( APylon* Py )
{
	if (!bAICanUseDoor)
	{
		return;
	}

	FVector Start = LocalToWorld().TransformFVector(FVector(PathPointOffset, 0, 0));
	Start.Z = Location.Z;
	FVector End = LocalToWorld().TransformFVector(FVector(-PathPointOffset, 0, 0));
	End.Z = Location.Z;

	FNavMeshPolyBase *StartPoly=NULL, *EndPoly=NULL;
	APylon *StartPylon=NULL, *EndPylon=NULL;

	UNavigationHandle::GetPylonAndPolyFromPos(Start, AScout::GetGameSpecificDefaultScoutObject()->WalkableFloorZ, StartPylon, StartPoly);
	UNavigationHandle::GetPylonAndPolyFromPos(End, AScout::GetGameSpecificDefaultScoutObject()->WalkableFloorZ, EndPylon, EndPoly);

	if( StartPoly != NULL && EndPoly != NULL && 
		(StartPylon == Py || EndPylon == Py))
	{
		AddEdgeForThisPO(this, StartPylon, StartPoly, EndPoly, Start, Start, 1);
		AddEdgeForThisPO(this, EndPylon, EndPoly, StartPoly, End, End, 2);
	}

	Edge0Dest = End;
	Edge1Dest = Start;
}

EEdgeHandlingStatus AOLDoor::AddStaticEdgeIntoThisPO( EEdgeHandlingStatus Status, const FVector& inV1, const FVector& inV2, TArray<FNavMeshPolyBase*>& ConnectedPolys, INT PolyAssocatedWithThisPO, FLOAT SupportedEdgeWidth, BYTE EdgeGroupID)
{
	if (ConnectedPolys.Num() < 2)
	{
		return EHS_AddedNone;
	}

	FNavMeshPolyBase* Poly0 = ConnectedPolys(0);
	FNavMeshPolyBase* Poly1 = ConnectedPolys(1);

	if (Poly0 == NULL || Poly1 == NULL)
	{
		return EHS_AddedNone;
	}

	APylon* Pylon0 = UNavigationHandle::StaticGetPylonFromPos(Poly0->GetPolyCenter());
	APylon* Pylon1 = UNavigationHandle::StaticGetPylonFromPos(Poly1->GetPolyCenter());
	
	if (Pylon0 == NULL || Pylon1 == NULL)
	{
		return EHS_AddedNone;
	}

	if (PolyAssocatedWithThisPO == 0)
	{
		AddEdgeForThisPO(this, Pylon0, Poly0, Poly1, inV1, inV2, 0);
	}
	else if (PolyAssocatedWithThisPO == 1)
	{
		AddEdgeForThisPO(this, Pylon1, Poly1, Poly0, inV1, inV2, 0);
	}

	return EHS_AddedBothDirs;
}

UBOOL AOLDoor::ModifyFinalPath( UNavigationHandle* Handle, INT Idx )
{
	TWEAKABLE FLOAT InnerBoxHalfWidth = 30.f;

	UBOOL bModified = FALSE;

	FNavMeshEdgeBase* MyEdge = Handle->PathCache(Idx);
	FVector MyEdgePoint = MyEdge->GetEdgeCenter();
	FVector StartPoint = Handle->CachedPathParams.SearchStart;

	FVector StartPointToEdgeEndNrm = (MyEdge->GetFinalEdgeDestination() - StartPoint).SafeNormal2D();

	FVector EdgeStartToEndNrm = (MyEdge->GetFinalEdgeDestination() - MyEdgePoint).SafeNormal2D();
	FVector Side = EdgeStartToEndNrm.RotateAngleAxis(90.0f * DEG_TO_UNR, FVector(0.f, 0.f, 1.f)) * InnerBoxHalfWidth;

	TArray<FVector> Points;
	Points.AddItem(MyEdgePoint + Side);
	Points.AddItem(MyEdgePoint - Side);
	Points.AddItem(MyEdge->GetFinalEdgeDestination() + Side);
	Points.AddItem(MyEdge->GetFinalEdgeDestination() - Side);
	Points.AddItem(MyEdgePoint + FVector(0.f, 0.f, 50.f));
	Points.AddItem(MyEdgePoint - FVector(0.f, 0.f, 50.f));
	FBox InnerBox(Points);

	FNavMeshEdgeBase* PrevEdge;
	FVector PrevEdgePoint;
	FVector StartPointToPrevEdgePointNrm;
	INT PrevIndex = Idx - 1;
	while (PrevIndex >= 0)
	{
		PrevEdge = Handle->PathCache(PrevIndex);
		PrevEdgePoint = PrevEdge->GetEdgeCenter();
		StartPointToPrevEdgePointNrm = (PrevEdgePoint - StartPoint).SafeNormal2D();

		if (InnerBox.IsInside(PrevEdgePoint) && InnerBox.IsInside(StartPoint) && (StartPointToEdgeEndNrm | StartPointToPrevEdgePointNrm) < 0.f)
		{
			Handle->PathCache_RemoveIndex(PrevIndex, 1);

			--PrevIndex;

			bModified = TRUE;
		}
		else
		{
			break;
		}
	}

	return bModified;
}

UBOOL AOLDoor::Verify()
{
	return !IsPendingKill();
}

UBOOL AOLDoor::GetBoundingShape(TArray<FVector>& out_PolyShape,INT ShapeIdx)
{
	TWEAKABLE FLOAT ExtraSpacingX = 15.0f;
	TWEAKABLE FLOAT ExtraSpacingY = 25.0f;

	FBoxSphereBounds& MeshBounds = DoorMainMesh->StaticMesh->Bounds;

	FVector newVert = DoorMainMesh->LocalToWorld.TransformFVector(MeshBounds.Origin + FVector(MeshBounds.BoxExtent.X + ExtraSpacingX, MeshBounds.BoxExtent.Y + ExtraSpacingY, -MeshBounds.BoxExtent.Z));
	out_PolyShape.AddItem(newVert);
	newVert = DoorMainMesh->LocalToWorld.TransformFVector(MeshBounds.Origin + FVector(-(MeshBounds.BoxExtent.X + ExtraSpacingX), MeshBounds.BoxExtent.Y + ExtraSpacingY, -MeshBounds.BoxExtent.Z));
	out_PolyShape.AddItem(newVert);
	newVert = DoorMainMesh->LocalToWorld.TransformFVector(MeshBounds.Origin + FVector(-(MeshBounds.BoxExtent.X + ExtraSpacingX), -(MeshBounds.BoxExtent.Y + ExtraSpacingY), -MeshBounds.BoxExtent.Z));
	out_PolyShape.AddItem(newVert);
	newVert = DoorMainMesh->LocalToWorld.TransformFVector(MeshBounds.Origin + FVector(MeshBounds.BoxExtent.X + ExtraSpacingX, -(MeshBounds.BoxExtent.Y + ExtraSpacingY), -MeshBounds.BoxExtent.Z));
	out_PolyShape.AddItem(newVert);

	return TRUE;
}

void AOLDoor::NotifyHandlesToRepath()
{
	FNavMeshWorld* World = FNavMeshWorld::GetNavMeshWorld();

	if(World != NULL)
	{
		UNavigationHandle* CurrentHandle = NULL;
		FNavMeshEdgeBase* CurrentEdge = NULL;
		FNavMeshPathObjectEdge* CurrPathEdge = NULL;

		for (INT HandleIdx = 0; HandleIdx < World->ActiveHandles.Num(); ++HandleIdx)
		{
			CurrentHandle = World->ActiveHandles(HandleIdx);

			for (INT HandleEdgeIdx = 0; HandleEdgeIdx < CurrentHandle->PathCache.Num(); ++HandleEdgeIdx)
			{
				CurrentEdge = CurrentHandle->PathCache(HandleEdgeIdx);

				if (CurrentEdge != NULL && CurrentEdge->GetEdgeType()==NAVEDGE_PathObject)
				{
					CurrPathEdge = static_cast<FNavMeshPathObjectEdge*>(CurrentEdge);

					AActor* PO = *CurrPathEdge->PathObject;

					if (PO == this)
					{
						IInterface_NavigationHandle* Interface = InterfaceCast<IInterface_NavigationHandle>(CurrentHandle->GetOuter());	
						if(Interface != NULL)
						{
							UObject* InterfaceImplementor = Interface->GetUObjectInterfaceInterface_NavigationHandle();
							if(InterfaceImplementor != NULL && !InterfaceImplementor->HasAnyFlags(RF_Unreachable))
							{
								Interface->eventNotifyPathChanged();
							}
						}
					}
				}
			}
		}
	}
}

void AOLDoor::NotifyHandlesToWait()
{
	FNavMeshWorld* World = FNavMeshWorld::GetNavMeshWorld();

	if(World != NULL)
	{
		UNavigationHandle* CurrentHandle = NULL;
		FNavMeshEdgeBase* CurrentEdge = NULL;
		FNavMeshPathObjectEdge* CurrPathEdge = NULL;

		for (INT HandleIdx = 0; HandleIdx < World->ActiveHandles.Num(); ++HandleIdx)
		{
			CurrentHandle = World->ActiveHandles(HandleIdx);

			for (INT HandleEdgeIdx = 0; HandleEdgeIdx < CurrentHandle->PathCache.Num(); ++HandleEdgeIdx)
			{
				CurrentEdge = CurrentHandle->PathCache(HandleEdgeIdx);

				if (CurrentEdge != NULL && CurrentEdge->GetEdgeType()==NAVEDGE_PathObject)
				{
					CurrPathEdge = static_cast<FNavMeshPathObjectEdge*>(CurrentEdge);

					AActor* PO = *CurrPathEdge->PathObject;

					if (PO == this)
					{
						IInterface_NavigationHandle* Interface = InterfaceCast<IInterface_NavigationHandle>(CurrentHandle->GetOuter());	
						if(Interface != NULL)
						{
							UObject* InterfaceImplementor = Interface->GetUObjectInterfaceInterface_NavigationHandle();
							if(InterfaceImplementor != NULL && !InterfaceImplementor->HasAnyFlags(RF_Unreachable))
							{
								AOLBot* Bot = Cast<AOLBot>(InterfaceImplementor);
								if (Bot && Bot->Location.DistanceSquared(GetCenterLocation()) < Square(300.0f))
								{
									Bot->eventStartWaitForDoor();
								}
							}
						}
					}
				}
			}
		}
	}
}