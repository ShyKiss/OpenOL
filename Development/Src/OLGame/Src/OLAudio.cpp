#include "OLGame.h"
#include "AkAudioDevice.h"

IMPLEMENT_CLASS(AOLScareMoment);
IMPLEMENT_CLASS(AOLSoundEnvironmentVolume);
IMPLEMENT_CLASS(AOLAmbientSound);
IMPLEMENT_CLASS(AOLAmbientSoundClone);
IMPLEMENT_CLASS(UOLSoundEnvironment);
IMPLEMENT_CLASS(UOLSoundConnectorComponent);
IMPLEMENT_CLASS(AOLSoundConnector);
IMPLEMENT_CLASS(UOLSoundEnvironmentManager);
IMPLEMENT_CLASS(AOLSoundEmittingMeshActor);
IMPLEMENT_CLASS(UActorFactoryOLAmbientSound);
IMPLEMENT_CLASS(UActorFactoryOLSoundEmittingMeshActor);
IMPLEMENT_CLASS(UOLSoundEmitter);

DECLARE_STATS_GROUP(TEXT("OLAudio"),STATGROUP_OLAudio);
DECLARE_CYCLE_STAT(TEXT("SoundEnvironmentManager Tick Time"),STAT_SoundEnvironmentManagerTickTime,STATGROUP_OLAudio);
DECLARE_CYCLE_STAT(TEXT("Update Listener Volumes"),STAT_UpdateListenerVolumes,STATGROUP_OLAudio);
DECLARE_CYCLE_STAT(TEXT("Update Reverb"),STAT_UpdateReverb,STATGROUP_OLAudio);
DECLARE_CYCLE_STAT(TEXT("Update Dynamic Actor"),STAT_UpdateDynamicActor,STATGROUP_OLAudio);
DECLARE_CYCLE_STAT(TEXT("Update Static Actor"),STAT_UpdateStaticActor,STATGROUP_OLAudio);
DECLARE_CYCLE_STAT(TEXT("Update Dynamic Audio Params"),STAT_UpdateDynamicAudioParams,STATGROUP_OLAudio);
DECLARE_CYCLE_STAT(TEXT("Update Multi Position Source"),STAT_UpdateMultiPositionSources,STATGROUP_OLAudio);

//////////////////////////////////////////////////////////////////////////
// Scare Moment
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void AOLScareMoment::UpdateScareRTPC( FLOAT DistToPlayer )
{
	if (DistanceRTPC != NAME_None)
	{
		FLOAT NormalizedDistance = Clamp((DistToPlayer / Range), 0.0f, 1.0f) * 100.0f;

		SetRTPCValue( DistanceRTPC, NormalizedDistance );
	}
}

//////////////////////////////////////////////////////////////////////////
// UActorFactoryOLAmbientSound
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

UBOOL UActorFactoryOLAmbientSound::CanCreateActor(FString& OutErrorMsg, UBOOL bFromAssetOnly) 
{ 
	if( AmbientEvent )
	{
		return true;
	}
	else
	{
		OutErrorMsg = TEXT("Error_CouldNotCreateActor_NoEvent");
		return false;
	}
}

//////////////////////////////////////////////////////////////////////////
// AOLAmbientSound
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void AOLAmbientSound::SetVolumes(const TArray<class AVolume*>& Volumes)
{
	for( INT i=0; i<Volumes.Num(); i++ )
	{
		AVolume*		V = Volumes(i);

		// blocking volumes can't touch
		if( V && !V->bBlockActors )
		{
			APhysicsVolume*	P = Cast<APhysicsVolume>(V);

			if( ((bCollideActors && V->bCollideActors) || P || V->bProcessAllSkeletalMeshActors || V->bProcessAllActors) && V->Encompasses(Location + VecZ(20.0f)) )
			{
				if (( bCollideActors && V->bCollideActors ) || V->bProcessAllSkeletalMeshActors)
				{
					V->Touching.AddItem(this);
					Touching.AddItem(V);
				}
				if( P && (P->Priority > PhysicsVolume->Priority) )
				{
					PhysicsVolume = P;
				}
				if( V->bProcessAllActors )
				{
					V->eventProcessActorSetVolume( this );
				}
			}
		}
	}
}

void AOLAmbientSound::Playing( UBOOL in_IsPlaying )
{
	Super::Playing(in_IsPlaying);

	UOLSoundEnvironmentManager* sndEnvMgr = Utils::GetSoundEnvManager();

	if (sndEnvMgr && PlayEvent != NULL)
	{
		UOLSoundEmitter* sndSrc = sndEnvMgr->GetSoundEmitterForActor(this);

		if (sndSrc)
		{
			sndSrc->bActive = bIsPlaying;

			for (INT i = 0; i < Clones.Num(); i++)
			{
				AOLAmbientSoundClone* clone = Clones(i);

				if (clone)
				{
					UOLSoundEmitter* cloneEmitter = sndEnvMgr->GetSoundEmitterForActor(Clones(i));
					cloneEmitter->bActive = bIsPlaying;
				}
			}
		}
		else if (in_IsPlaying)
		{
			UOLSoundEmitter* newEmitter = sndEnvMgr->RegisterSoundEmitterForAmbientSound(this, bIsPlaying);
			
			for (INT i = 0; i < Clones.Num(); i++)
			{
				AOLAmbientSoundClone* clone = Clones(i);

				if (clone)
				{
					check(clone->Master == this);
					sndEnvMgr->RegisterSoundEmitterForAmbientSoundClone(clone, newEmitter, bIsPlaying);
				}				
			}
		}
	}
}

void AOLAmbientSound::PreEditChange(UProperty* PropertyThatWillChange)
{
	Super::PreEditChange(PropertyThatWillChange);

	// NULL out the masters, in case we remove a clone
	for (INT i = 0; i < Clones.Num(); i++)
	{
		AOLAmbientSoundClone* clone = Clones(i);

		if (clone)
		{
			clone->Master = NULL;
		}
	}
}

void AOLAmbientSound::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	for (INT i = 0; i < Clones.Num(); i++)
	{
		AOLAmbientSoundClone* clone = Clones(i);

		if (clone)
		{
			if (clone->Master && clone->Master != this)
			{
				clone->Master->Clones.RemoveItem(clone);
			}
			clone->Master = this;
		}
	}
	Super::PostEditChangeProperty(PropertyChangedEvent);
}

//////////////////////////////////////////////////////////////////////////
// AOLAmbientSoundClone
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void AOLAmbientSoundClone::SetVolumes(const TArray<class AVolume*>& Volumes)
{
	for( INT i=0; i<Volumes.Num(); i++ )
	{
		AVolume*		V = Volumes(i);

		// blocking volumes can't touch
		if( V && !V->bBlockActors )
		{
			APhysicsVolume*	P = Cast<APhysicsVolume>(V);

			if( ((bCollideActors && V->bCollideActors) || P || V->bProcessAllSkeletalMeshActors || V->bProcessAllActors) && V->Encompasses(Location + VecZ(20.0f)) )
			{
				if (( bCollideActors && V->bCollideActors ) || V->bProcessAllSkeletalMeshActors)
				{
					V->Touching.AddItem(this);
					Touching.AddItem(V);
				}
				if( P && (P->Priority > PhysicsVolume->Priority) )
				{
					PhysicsVolume = P;
				}
				if( V->bProcessAllActors )
				{
					V->eventProcessActorSetVolume( this );
				}
			}
		}
	}
}

//////////////////////////////////////////////////////////////////////////
// AOLSoundEnvironmentVolume
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void AOLSoundEnvironmentVolume::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{	
	FColor occlusionAndSE(135, 0, 200);
	FColor occlusionOnly = ((AOLSoundEnvironmentVolume*)StaticClass()->GetDefaultObject())->BrushColor;
	FColor soundEnvOnly(200, 0, 125);
	FColor nothing(200, 200, 200);
	
	if (bUseForOcclusion && SoundEnvironment)
	{
		BrushColor = occlusionAndSE;
	}
	else if (bUseForOcclusion)
	{
		BrushColor = occlusionOnly;
	}
	else if (SoundEnvironment)
	{
		BrushColor = soundEnvOnly;
	}
	else
	{
		BrushColor = nothing;
	}
	
	if (BrushComponent != NULL)
	{
		FComponentReattachContext Reattach(BrushComponent);
	}

	Super::PostEditChangeProperty(PropertyChangedEvent);
}

void AOLSoundEnvironmentVolume::RegisterOnStaticEmitters(TArray<UOLSoundEmitter*>& sndSources)
{
	// If the volume is loaded after a static emitter (e.g. ld map loaded, then snd map loaded), we must register ourselves on it
	for (INT i = 0; i < sndSources.Num(); i++)
	{
		UOLSoundEmitter* sndSource = sndSources(i);
				
		if (!sndSource->bDynamic && Encompasses(sndSource->BaseLocation + VecZ(20.0f)))
		{
			UBOOL bAdd = TRUE;

			// check already registered volumes, enforce priorities
			for (INT j = 0; j < sndSource->SoundEnvironments.Num(); j++)
			{
				AOLSoundEnvironmentVolume* otherVol = sndSource->SoundEnvironments(j).SoundEnvVolume;
				if (otherVol && otherVol->Priority < Priority)
				{
					// the existing volume has a lower priority - remove it
					sndSource->SoundEnvironments.Remove(j);
					j--;
					continue;
				}
				else if (otherVol && otherVol->Priority > Priority)
				{
					// the existing volume has a higher priority - we won't add ourselves
					bAdd = FALSE;
					break;
				}
			}

			if (bAdd)
			{
				FSoundEnvVolumeData params;
				params.SoundEnvVolume = this;
				params.bActive = TRUE;
				params.BorderPoint = FVector(FLT_MIN);
				params.bFadeFromBorder = FALSE;
				params.LastActiveEnvContrib = 0.0f;
				sndSource->SoundEnvironments.AddItem(params);
			}
		}
	}
}

//////////////////////////////////////////////////////////////////////////
// UOLSoundConnectorComponent
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void UOLSoundConnectorComponent::ConnectVolumes(const FVector& location, const TArray<class AOLSoundEnvironmentVolume*>& Volumes)
{
	ConnectedVolumes.Empty();

	UOLSoundEnvironmentManager::SndEnvVolumeArray surroundingVolumes;

	INT maxPrio = -1;

	for (INT i = 0; i < Volumes.Num(); i++)
	{
		AOLSoundEnvironmentVolume* audioVolume = Volumes(i);

		if (audioVolume && !ConnectedVolumes.ContainsItem(audioVolume) && audioVolume->bUseForOcclusion && audioVolume->Encompasses(location + VecZ(SphereRadius), FVector(SphereRadius)))
		{
			surroundingVolumes.AddItem(audioVolume);
			maxPrio = Max(maxPrio, (INT)audioVolume->Priority);
		}
	}

	if (surroundingVolumes.Num() > 2)
	{
		// special case: if more than 2 volumes, we only connect those at the max priority level

		for (INT i = 0; i < surroundingVolumes.Num(); i++)
		{
			AOLSoundEnvironmentVolume* audioVolume = surroundingVolumes(i);
			if (audioVolume->Priority < maxPrio)
			{
				// ignore - e.g. for a large volume at low-priority, encompassing a smaller set of connected rooms
				surroundingVolumes.Remove(i);
				i--;
				continue;
			}
		}
	}

	if (surroundingVolumes.Num() < 2)
	{
		debugf(TEXT("Sound connector on %s doesn't connect 2 or more volumes!"), *Owner->GetName());
		return;
	}
	else if (surroundingVolumes.Num() > 3)
	{
		debugf(TEXT("!! Sound connector on %s connects %d volumes!?!"), *Owner->GetName(), surroundingVolumes.Num());
		return;
	}

	for (INT i = 0; i < surroundingVolumes.Num(); i++)
	{
		AOLSoundEnvironmentVolume* audioVolume = surroundingVolumes(i);

		ConnectedVolumes.AddItem(audioVolume);
		audioVolume->Connections.AddUniqueItem(this);
	}
}

#if WITH_EDITOR
void UOLSoundConnectorComponent::CheckConnectorForErrors()
{
	/*if ()
	{
	GWarn->MapCheck_Add( MCTYPE_WARNING, this, *FString::Printf(  ));
	}*/
}
#endif

FVector UOLSoundConnectorComponent::GetCenterLocation()
{
	if (bSpherical)
	{
		return GetOwner()->Location + VecZ(SphereRadius);
	}
	else
	{
		return GetOwner()->Location + VecZ(100.0f); // 100 is the fixed box translation
	}
}

FVector UOLSoundConnectorComponent::GetClosestPoint(const FVector& target)
{
	FVector centerLoc = GetCenterLocation();
	if (bSpherical)
	{
		return centerLoc + SphereRadius * (target - centerLoc).SafeNormal2D();
	}
	else
	{
		FVector closestPoint;
		FVector boxAxis = GetOwner()->Rotation.Right().SafeNormal2D();
		PointDistToSegment(target, centerLoc - 0.5f * BoxWidth * boxAxis, centerLoc + 0.5f * BoxWidth * boxAxis, closestPoint);
		return closestPoint;
	}
}

//////////////////////////////////////////////////////////////////////////
// AOLSoundConnector
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void AOLSoundConnector::SetVolumes(const TArray<class AVolume*>& Volumes)
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

	SoundConnectorComp->ConnectVolumes(Location, sndEnvVolumes);

	Super::SetVolumes(Volumes);
}

#if WITH_EDITOR
void AOLSoundConnector::PostLoad()
{
	Super::PostLoad();
	UpdatePreviewShapes();
}

void AOLSoundConnector::UpdatePreviewShapes()
{
	if (GIsEditor)
	{
		if (SoundConnectorComp->bSpherical)
		{
			BoxPreviewComp->SetHiddenEditor(TRUE);

			SpherePreviewComp->SphereRadius = SoundConnectorComp->SphereRadius;
			SpherePreviewComp->Translation.Z = SoundConnectorComp->SphereRadius;
			SpherePreviewComp->ConditionalUpdateTransform();		
			SpherePreviewComp->SetHiddenEditor(FALSE);
		}
		else
		{
			SpherePreviewComp->SetHiddenEditor(TRUE);

			BoxPreviewComp->BoxExtent = FVector(25.0f, 0.5f*SoundConnectorComp->BoxWidth, 50.0f);
			BoxPreviewComp->ConditionalUpdateTransform();
			BoxPreviewComp->SetHiddenEditor(FALSE);
		}
	}
}

void AOLSoundConnector::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	UpdatePreviewShapes();
	Super::PostEditChangeProperty(PropertyChangedEvent);
}

void AOLSoundConnector::CheckForErrors()
{
	Super::CheckForErrors();
	SoundConnectorComp->CheckConnectorForErrors();
}
#endif

//////////////////////////////////////////////////////////////////////////
// OLSoundEnvironmentManager - Exterior interface
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

UOLSoundEnvironmentManager* UOLSoundEnvironmentManager::GetSoundEnvManager()
{
	return Utils::GetSoundEnvManager();
}

UOLSoundEmitter* UOLSoundEnvironmentManager::GetSoundEmitterForActor(AActor* actor)
{
	for (INT i = 0; i < ActiveSources.Num(); i++)
	{
		if (ActiveSources(i)->Actor == actor)
		{
			return ActiveSources(i);
		}
	}
	return NULL;
}

void RegisterSoundEmitterForActor(AActor* actor)
{
	if (Utils::GetSoundEnvManager() && !Utils::GetSoundEnvManager()->bPendingDestroy)
	{
		ASkeletalMeshActor* skelActor = Cast<ASkeletalMeshActor>(actor); // other sound sources (ambient sounds, pawns) are currently handled separately

		if (skelActor && skelActor->SkeletalMeshComponent)
		{
			Utils::GetSoundEnvManager()->RegisterSoundEmitterForSkeletalMeshActor(skelActor);
		}
		else
		{
			Utils::GetSoundEnvManager()->UpdateSoundEmitterForActor(actor);
		}
	}
}

void UOLSoundEnvironmentManager::ConditionalRegisterSoundEmitterForDoor(AOLDoor* door)
{
	if (bPendingDestroy)
	{
		return;
	}

	UOLSoundEmitter* exisingSrc = GetSoundEmitterForActor(door);

	if (exisingSrc)
	{
		return;
	}

	UOLSoundEmitter* sndSrc = RegisterSoundEmitterForActor(door, TRUE, FALSE, TRUE);

	sndSrc->BaseLocation = door->GetCenterLocation() + VecZ(135.0f);
	sndSrc->VirtualizedLocation = sndSrc->BaseLocation;

	// Add the volumes from the existing connector
	for (INT i = 0; i < door->SoundConnectorComp->ConnectedVolumes.Num(); i++)
	{
		AOLSoundEnvironmentVolume* audioVolume = door->SoundConnectorComp->ConnectedVolumes(i);

		if (audioVolume)
		{
			FSoundEnvVolumeData params;
			params.SoundEnvVolume = audioVolume;
			params.bActive = TRUE;
			params.BorderPoint = FVector(FLT_MIN);
			params.bFadeFromBorder = FALSE;
			params.LastActiveEnvContrib = 0.0f;
			sndSrc->SoundEnvironments.AddItem(params);
		}
	}

	InitializeSoundEmitter(sndSrc);

	return;
}

UOLSoundEmitter* UOLSoundEnvironmentManager::RegisterSoundEmitterForAmbientSound(AOLAmbientSound* ambientSound, UBOOL bIsActive)
{
	if (bPendingDestroy)
	{
		return NULL;
	}

	UOLSoundEmitter* sndSrc = RegisterSoundEmitterForActor(ambientSound, bIsActive, FALSE, ambientSound->bAllowVirtualization);

	SndEnvVolumeArray touchingVolumes;

	for (INT i=0; i < ambientSound->Touching.Num(); i++)
	{
		AOLSoundEnvironmentVolume* audioVolume = Cast<AOLSoundEnvironmentVolume>(ambientSound->Touching(i));

		if (audioVolume)
		{
			touchingVolumes.AddItem(audioVolume);
		}
	}

	FilterVolumesByPriority(touchingVolumes);
	
	sndSrc->SoundEnvironments.Empty();
	for (INT i=0; i < touchingVolumes.Num(); i++)
	{		
		FSoundEnvVolumeData params;
		params.SoundEnvVolume = touchingVolumes(i);
		params.bActive = bIsActive;
		params.BorderPoint = FVector(FLT_MIN);
		params.bFadeFromBorder = FALSE;
		params.LastActiveEnvContrib = 0.0f;
		sndSrc->SoundEnvironments.AddItem(params);
	}

	if (ambientSound->Clones.Num() > 0)
	{
		sndSrc->bInMultiPositionGroup = TRUE;
		sndSrc->bGroupMaster = TRUE;
		sndSrc->GroupEventName = ambientSound->PlayEvent->GetName();

		FMultiPositionGroup newGroup;
		newGroup.EventName = ambientSound->PlayEvent->GetName();
		newGroup.Members.AddItem(sndSrc);
		ActiveGroups.AddItem(newGroup);
	}

	InitializeSoundEmitter(sndSrc);

	return sndSrc;
}

UOLSoundEmitter* UOLSoundEnvironmentManager::RegisterSoundEmitterForAmbientSoundClone(AOLAmbientSoundClone* clone, UOLSoundEmitter* masterEmitter, UBOOL bIsActive)
{
	if (bPendingDestroy)
	{
		return NULL;
	}

	check(clone->Master);

	UOLSoundEmitter* sndSrc = RegisterSoundEmitterForActor(clone, bIsActive, FALSE, clone->bAllowVirtualization);

	SndEnvVolumeArray touchingVolumes;

	sndSrc->SoundEnvironments.Empty();
	for (INT i=0; i < clone->Touching.Num(); i++)
	{
		AOLSoundEnvironmentVolume* audioVolume = Cast<AOLSoundEnvironmentVolume>(clone->Touching(i));

		if (audioVolume)
		{
			touchingVolumes.AddItem(audioVolume);
		}
	}

	FilterVolumesByPriority(touchingVolumes);

	for (INT i=0; i < touchingVolumes.Num(); i++)
	{
		FSoundEnvVolumeData params;
		params.SoundEnvVolume = touchingVolumes(i);
		params.bActive = bIsActive;
		params.BorderPoint = FVector(FLT_MIN);
		params.bFadeFromBorder = FALSE;
		params.LastActiveEnvContrib = 0.0f;
		sndSrc->SoundEnvironments.AddItem(params);
	}

	FMultiPositionGroup* group = GetGroupForEmitter(masterEmitter);
	check(group);

	sndSrc->bInMultiPositionGroup = TRUE;
	sndSrc->bGroupMaster = FALSE;
	sndSrc->GroupEventName = group->EventName;
		
	group->Members.AddItem(sndSrc);

	//debugf(TEXT("### Adding %s to group %s"), *clone->GetName(), *sndSrc->GroupEventName);
	
	InitializeSoundEmitter(sndSrc);

	return sndSrc;
}

UOLSoundEmitter* UOLSoundEnvironmentManager::RegisterSoundEmitterForSoundEmittingMeshActor(AOLSoundEmittingMeshActor* emittingMesh)
{
	if (bPendingDestroy)
	{
		return NULL;
	}

	UOLSoundEmitter* sndSrc = RegisterSoundEmitterForActor(emittingMesh, TRUE, FALSE, emittingMesh->bAllowVirtualization);

	SndEnvVolumeArray touchingVolumes;

	sndSrc->SoundEnvironments.Empty();
	for (INT i=0; i < emittingMesh->Touching.Num(); i++)
	{
		AOLSoundEnvironmentVolume* audioVolume = Cast<AOLSoundEnvironmentVolume>(emittingMesh->Touching(i));

		if (audioVolume)
		{
			touchingVolumes.AddItem(audioVolume);
		}
	}

	FilterVolumesByPriority(touchingVolumes);

	for (INT i=0; i < touchingVolumes.Num(); i++)
	{
		FSoundEnvVolumeData params;
		params.SoundEnvVolume = touchingVolumes(i);
		params.bActive = TRUE;
		params.BorderPoint = FVector(FLT_MIN);
		params.bFadeFromBorder = FALSE;
		params.LastActiveEnvContrib = 1.0f;
		sndSrc->SoundEnvironments.AddItem(params);
	}

	if (emittingMesh->bEnableMultiPosition)
	{
		AddSoundEmittingMeshToMultiPositionGroup(sndSrc, emittingMesh->PlayEvent->GetName());
	}

	InitializeSoundEmitter(sndSrc);

	return sndSrc;
}

UOLSoundEmitter* UOLSoundEnvironmentManager::RegisterSoundEmitterForPawn(APawn* pawn)
{
	if (bPendingDestroy)
	{
		return NULL;
	}

	UOLSoundEmitter* sndSrc = RegisterSoundEmitterForActor(pawn, TRUE, TRUE, TRUE);
	UpdateSoundEnvironmentForDynamicActor(sndSrc, 0.0f, TRUE);

	if (pawn->IsA(AOLHero::StaticClass()))
	{
		sndSrc->TargetObstruction = 0.0f;
		sndSrc->TargetOcclusion = 0.0f;
		sndSrc->CurrentObstruction = 0.0f;
		sndSrc->CurrentOcclusion = 0.0f;
	}
	else
	{
		InitializeSoundEmitter(sndSrc);
	}

	return sndSrc;
}

UOLSoundEmitter* UOLSoundEnvironmentManager::RegisterSoundEmitterForSkeletalMeshActor(ASkeletalMeshActor* skelActor)
{
	UOLSoundEmitter* exisingSrc = GetSoundEmitterForActor(skelActor);

	if (exisingSrc)
	{
		exisingSrc->bActive = TRUE;		
		return exisingSrc;
	}

	UOLSoundEmitter* sndSrc = RegisterSoundEmitterForActor(skelActor, TRUE, FALSE, TRUE);

	SndEnvVolumeArray touchingVolumes;

	// Add the touching volumes
	for (INT i = 0; i < AllVolumes.Num(); i++)
	{
		AOLSoundEnvironmentVolume* audioVolume = AllVolumes(i);

		if (audioVolume && audioVolume->Encompasses(sndSrc->BaseLocation + VecZ(20.0f)))
		{
			touchingVolumes.AddItem(audioVolume);
		}
	}

	FilterVolumesByPriority(touchingVolumes);

	for (INT i = 0; i < touchingVolumes.Num(); i++)
	{
		FSoundEnvVolumeData params;
		params.SoundEnvVolume = touchingVolumes(i);
		params.bActive = TRUE;
		params.BorderPoint = FVector(FLT_MIN);
		params.bFadeFromBorder = FALSE;
		params.LastActiveEnvContrib = 0.0f;
		sndSrc->SoundEnvironments.AddItem(params);
	}

	InitializeSoundEmitter(sndSrc);

	return sndSrc;
}

UOLSoundEmitter* UOLSoundEnvironmentManager::RegisterSoundEmitterForActor(AActor* actor, UBOOL bActive, UBOOL bDynamic, UBOOL bAllowVirtualization)
{
	UOLSoundEmitter* exisingSrc = GetSoundEmitterForActor(actor);

	if (exisingSrc)
	{
		exisingSrc->bActive = bActive;
		return exisingSrc;
	}

	// Construct a sound environment	
	UOLSoundEmitter* sndSource = ConstructObject<UOLSoundEmitter>(UOLSoundEmitter::StaticClass(), this);
	sndSource->Actor = actor;
	sndSource->bActive = bActive;
	sndSource->bDynamic = bDynamic;
	sndSource->bAllowVirtualization = bAllowVirtualization;
	sndSource->bDirty = TRUE;
	sndSource->CurrentOcclusion = 1.0f;
	sndSource->CurrentObstruction = 1.0f;
	sndSource->TargetObstruction = 1.0f;
	sndSource->TargetOcclusion = 1.0f;
	sndSource->LastObstructionCheckTime = -1.0f;
	sndSource->LastOcclusionCheckTime = -1.0f;
	sndSource->SoundEnvironments.Empty();
	sndSource->BaseLocation = actor->Location;
	sndSource->bVirtualized = FALSE;
	sndSource->VirtualizedLocation = sndSource->BaseLocation;
	appMemZero(sndSource->DebugInfo);

	ActiveSources.AddItem(sndSource); 

	return sndSource;
}

void UOLSoundEnvironmentManager::UnregisterSoundEmitterForActor(AActor* actor)
{
	for (INT i = 0; i < ActiveSources.Num(); i++)
	{
		UOLSoundEmitter* sndSrc = ActiveSources(i);
		if (sndSrc->Actor == actor)
		{
			if (sndSrc->bInMultiPositionGroup)
			{
				RemoveFromMultiPositionGroup(sndSrc);
			}

			ActiveSources.Remove(i);
			return;
		}
	}

	warnf(TEXT("UnregisterSoundEmitterForActor for no existing emitter for %s"), *actor->GetName());
}

void UOLSoundEnvironmentManager::UpdateSoundEmitterForActor(AActor* actor)
{
	UOLSoundEmitter* existingEmitter = GetSoundEmitterForActor(actor);

	if (existingEmitter)
	{
		existingEmitter->bActive = TRUE;
		return;
	}

	AOLAmbientSound* ambSound = Cast<AOLAmbientSound>(actor);
	if (ambSound)
	{
		// Consider any event as possibly activating the ambient sound - wasteful but safer
		UOLSoundEmitter* newEmitter = RegisterSoundEmitterForAmbientSound(ambSound, TRUE);

		for (INT i = 0; i < ambSound->Clones.Num(); i++)
		{
			AOLAmbientSoundClone* clone = ambSound->Clones(i);

			if (clone)
			{
				check(clone->Master == ambSound);
				RegisterSoundEmitterForAmbientSoundClone(ambSound->Clones(i), newEmitter, TRUE);
			}
		}
	}
}

void UpdateSoundPosition(AActor* actor, FVector& inout_Position, FVector& inout_Direction)
{
	if (Utils::GetSoundEnvManager())
	{
		Utils::GetSoundEnvManager()->UpdateSoundPosition(actor, inout_Position, inout_Direction);
	}
}

void UOLSoundEnvironmentManager::UpdateSoundPosition(AActor* actor, FVector& inout_Position, FVector& inout_Direction)
{
	UOLSoundEmitter* exisingSrc = GetSoundEmitterForActor(actor);

	if (exisingSrc)
	{
		exisingSrc->BaseLocation = inout_Position;

		if (exisingSrc->bVirtualized)
		{
			inout_Position = exisingSrc->VirtualizedLocation;
		}
	}

#if !SHIPPING_PC_GAME && !FINAL_RELEASE
	if (Utils::GetCheatManager() && Utils::GetCheatManager()->bDebugSoundEnvironment)
	{
		FVector headPt = inout_Position + 25.0f * inout_Direction;
		const FLOAT arrowSz = 3.0f;
		FVector rightV = inout_Direction.Rotation().Right();
		actor->DrawDebugLine(inout_Position, headPt, 0, 255, 0 );
		actor->DrawDebugLine(headPt, headPt - arrowSz * inout_Direction + arrowSz * rightV, 0, 255, 0 );
		actor->DrawDebugLine(headPt, headPt - arrowSz * inout_Direction - arrowSz * rightV, 0, 255, 0 );
	}
#endif
}

UBOOL TryGetMultipleSourcePositions(AActor* actor, TArray<FVector>& out_Positions, TArray<FVector>& out_Directions)
{
	if (Utils::GetSoundEnvManager())
	{
		return Utils::GetSoundEnvManager()->TryGetMultipleSourcePositions(actor, out_Positions, out_Directions);
	}

	return FALSE;
}

UBOOL UOLSoundEnvironmentManager::TryGetMultipleSourcePositions(AActor* actor, TArray<FVector>& out_Positions, TArray<FVector>& out_Directions)
{
	UOLSoundEmitter* exisingSrc = GetSoundEmitterForActor(actor);

	if (exisingSrc && exisingSrc->bInMultiPositionGroup)
	{
		check(exisingSrc->bGroupMaster); // Slaves should not have active AkComponents!

		FMultiPositionGroup* group = GetGroupForEmitter(exisingSrc);
		check(group);

		for (INT j = 0; j < group->Members.Num(); j++)
		{
			UOLSoundEmitter* groupMember = group->Members(j);
			check(groupMember);

			if (groupMember->bVirtualized)
			{
				out_Positions.AddItem(groupMember->VirtualizedLocation);
			}
			else
			{
				out_Positions.AddItem(groupMember->BaseLocation);				
			}					

			out_Directions.AddItem(groupMember->Actor->Rotation.Vector());
		}

		return TRUE;
	}

	return FALSE;
}

//////////////////////////////////////////////////////////////////////////
// Internal logic
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

UBOOL UOLSoundEnvironmentManager::FindPathToListener(	UOLSoundConnectorComponent* connector, 
																	  FLOAT totalDistance,
																	  const SoundPathingListenerInfo& listenerInfo, 
																	  FLOAT& out_occlusion, 
																	  INT& out_NbSubNodesToListener,											 											
																	  SndEnvVolumeArray& visitedVolumes, 
																	  VirtNodeArray& virtualizationNodes)
{
	FLOAT occlusion = 1.0f;
	INT minNbSubNodes = -1;
		
	const FVector& connectorLoc = connector->GetClosestPoint(*listenerInfo.ListenerLocation);

	FLOAT distToListener = listenerInfo.ListenerLocation->Distance(connectorLoc);

	if (totalDistance + distToListener - listenerInfo.DirectDistToListener > MaxPathingDist)
	{
		// too far away - stop the search
		return FALSE;
	}

	for (INT i = 0; i < connector->ConnectedVolumes.Num(); i++)
	{
		AOLSoundEnvironmentVolume* testVolume = connector->ConnectedVolumes(i);

		if (!testVolume || !testVolume->bUseForOcclusion || visitedVolumes.ContainsItem(testVolume))
		{
			// invalid volume
			continue;
		}

		UBOOL bConnected = FALSE;
		FLOAT thisLinkOcclusion = 1.0f;
		INT subLevel = -1;

		if (ListenerVolumes.ContainsItem(testVolume))
		{
			// we've found a connection
			bConnected = TRUE;
			FLOAT pathDistance = (totalDistance + distToListener);		
			FLOAT occlusiveDist = (pathDistance - listenerInfo.DirectDistToListener - FreePathingDist);
			FLOAT distOcclusion = Saturate(occlusiveDist / (MaxPathingDist - FreePathingDist)); 
			thisLinkOcclusion = distOcclusion;
			subLevel = 0;
		}
		else
		{
			visitedVolumes.AddItem(testVolume);
			
			for (INT j = 0; j < testVolume->Connections.Num(); j++)
			{
				// Try traversing this volume to other connections
				UOLSoundConnectorComponent* subConnector = testVolume->Connections(j);

				if (subConnector == connector)
				{
					continue;
				}

				if (!subConnector || subConnector->GetOwner() == NULL)
				{
					// Unstreamed owner
					testVolume->Connections.Remove(j);
					j--;
					continue;
				}

				if (!subConnector->bEnabled)
				{
					continue;
				}

				FLOAT linkDist = subConnector->GetOwner()->Location.Distance(connectorLoc);
				FLOAT connectedOcclusion = 0.0f;
				INT nbSubNodes = -1;

				UBOOL connected = FindPathToListener(subConnector, totalDistance + linkDist, listenerInfo, connectedOcclusion, nbSubNodes, visitedVolumes, virtualizationNodes);

				if (connected)
				{
					bConnected = TRUE;
					thisLinkOcclusion = Min(thisLinkOcclusion, connectedOcclusion);
					subLevel = nbSubNodes+1;					
				}
			}

			visitedVolumes.Pop();
		}

		if (bConnected)
		{
			AOLDoor* connectedDoor = Cast<AOLDoor>(connector->GetOwner());
			if (connectedDoor && connectedDoor->IsClosed() && !connectedDoor->IsBroken())
			{
				// but through a closed door
				thisLinkOcclusion = Min(1.0f, connectedDoor->GetOcclusionFactor() + thisLinkOcclusion);
			}
			else
			{
				AOLSoundConnector* soundConnector = Cast<AOLSoundConnector>(connector->GetOwner());
				if (soundConnector)
				{
					thisLinkOcclusion = Saturate(soundConnector->OcclusionFactor + thisLinkOcclusion);
				}
			}

			occlusion = Min(occlusion, thisLinkOcclusion);
			
			minNbSubNodes = (minNbSubNodes < 0) ? subLevel : Min(minNbSubNodes, subLevel);

			// Add the virtualization node
			UBOOL bExistingNode = FALSE;
			for (INT j = 0; j < virtualizationNodes.Num(); j++)
			{
				FVirtualizationNode& virtualizationNode = virtualizationNodes(j);
				if (virtualizationNode.Connector == connector)
				{
					// another way reaching the listener through the same end connector
					virtualizationNode.VirtualizableDistance = Min(virtualizationNode.VirtualizableDistance, totalDistance);
					virtualizationNode.DistToListener = Min(virtualizationNode.DistToListener, distToListener);
					virtualizationNode.Level = Min(subLevel, virtualizationNode.Level);
					bExistingNode = TRUE;
					break;
				}
			}

			if (!bExistingNode)
			{
				FVirtualizationNode virtualizationNode;
				virtualizationNode.Connector = connector;
				virtualizationNode.VirtualizableDistance = totalDistance;
				virtualizationNode.DistToListener = distToListener;
				virtualizationNode.Level = subLevel;

				virtualizationNodes.AddItem(virtualizationNode);
			}
		}
	}

	out_NbSubNodesToListener = minNbSubNodes;
	out_occlusion = Min(1.0f, occlusion);
	return occlusion < 1.0f;
}

UBOOL UOLSoundEnvironmentManager::GetPathedAudioParams(	FLOAT& out_occlusion, 
																			const FVector& srcLocation, 
																			const SndEnvVolumeArray& srcVolumes, 
																			const FVector& listenerLocation, 
																			VirtNodeArray& virtualizationNodes)
{
	FLOAT occlusion = (srcVolumes.Num() > 0) ? 1.0f : 0.0f; // a sound is fully occluded by default unless it has no occlusion volumes
	FLOAT directDistToListener = srcLocation.Distance(listenerLocation);

	FLOAT minEnvOcclusion = 1.0f;
	INT soundEnvironmentPriority = -1;

	UBOOL bConnected = FALSE;

	for (INT i = 0; i < srcVolumes.Num(); i++)
	{
		AOLSoundEnvironmentVolume* audioVolume = srcVolumes(i);

		if (!audioVolume->bUseForOcclusion)
		{
			continue;
		}

		if (ListenerVolumes.ContainsItem(audioVolume))
		{
			occlusion = 0.0f; // same volume, no occlusion
			bConnected = TRUE;
			break;
		}
		
		if (ListenerVolumes.Num() > 0)
		{
			SndEnvVolumeArray visitedVolumes;
			visitedVolumes.AddItem(audioVolume);

			for (INT j = 0; j < audioVolume->Connections.Num(); j++)
			{
				UOLSoundConnectorComponent* connector = audioVolume->Connections(j);

				if (!connector || connector->GetOwner() == NULL)
				{
					// Unstreamed owner
					audioVolume->Connections.Remove(j);
					j--;
					continue;
				}

				if (!connector->bEnabled)
				{
					continue;
				}

				FLOAT connectedOcclusion = 0.0f;
				FLOAT startDistance = connector->GetOwner()->Location.Distance(srcLocation);
				INT nbSubLevels = -1;

				SoundPathingListenerInfo listenerInfo;
				listenerInfo.DirectDistToListener = directDistToListener;
				listenerInfo.ListenerLocation = &listenerLocation;

				UBOOL connected = FindPathToListener(connector, startDistance, listenerInfo, connectedOcclusion, nbSubLevels, visitedVolumes, virtualizationNodes);

				if (connected)
				{
					occlusion = Min(occlusion, connectedOcclusion);
					bConnected = TRUE;
				}
			}
		}

		if (audioVolume->SoundEnvironment)
		{
			minEnvOcclusion = Min(minEnvOcclusion, audioVolume->SoundEnvironment->OcclusionFactor);
		}
	}

	// The maximum occlusion factor is the one from the sound environment
	out_occlusion = Min(occlusion, minEnvOcclusion);

	return bConnected;
}

FLOAT UOLSoundEnvironmentManager::GetObstructionFactor(UOLSoundEmitter* sndSrc, const SndEnvVolumeArray& srcVolumes)
{
	// We may eventually do complex raycasts, testing materials and obstruction ratio (distance between first and last hit)
	// For now, just a simple yes/no based on a single raycast from hero to sound source

	FCheckResult Hit(1.f);
	
	FVector startTrace = Utils::GetHero()->EyeLocation;
	FVector endTrace = sndSrc->BaseLocation;

	FVector toSrc = sndSrc->BaseLocation - startTrace;
	
	TWEAKABLE FLOAT SrcOffset = 25.0f; // small offset towards the listener, to clear the sound out of the ground or other object it's embedded into
	if (toSrc.SizeSquared() > Square(SrcOffset))
	{
		endTrace -= SrcOffset*toSrc.SafeNormal();
	}
	
	UBOOL allClear = GWorld->SingleLineCheck( Hit, Utils::GetHero(), endTrace, startTrace, TRACE_AllBlocking | TRACE_ComplexCollision | TRACE_SoundOcclusion, FVector(0.0f));

	if (allClear || Hit.Actor == sndSrc->Actor)
	{
		return 0.0f;
	}
	
	FLOAT obstruction = 1.0f;

	for (INT i = 0; i < srcVolumes.Num(); i++)
	{
		AOLSoundEnvironmentVolume* audioVolume = srcVolumes(i);

		FLOAT thisVolumeObstruction = (audioVolume->SoundEnvironment ? audioVolume->SoundEnvironment->ObstructionFactor : 1.0f);

		if (ListenerVolumes.ContainsItem(audioVolume))
		{
			thisVolumeObstruction *= ObstructionRatioInSameVolume;
		}

		obstruction = Min(obstruction, thisVolumeObstruction);
	}	

	return obstruction;
}

void UOLSoundEnvironmentManager::GetEnvironmentVolumes(const UOLSoundEmitter* sndSrc, SndEnvVolumeArray& sndVolumes)
{
	AOLSoundEnvironmentVolume* backupVolume = NULL; 
	for (INT i = 0; i < sndSrc->SoundEnvironments.Num(); i++)
	{
		if (sndSrc->SoundEnvironments(i).SoundEnvVolume->bUseForOcclusion)
		{
			if (sndSrc->SoundEnvironments(i).bActive)
			{
				sndVolumes.AddItem(sndSrc->SoundEnvironments(i).SoundEnvVolume);
			}
			backupVolume = sndSrc->SoundEnvironments(i).SoundEnvVolume;
		}
	}

	if (sndVolumes.Num() == 0 && backupVolume)
	{
		sndVolumes.AddItem(backupVolume);
	}
}

void UOLSoundEnvironmentManager::UpdateDynamicAudioParams(UOLSoundEmitter* sndSrc, FLOAT deltaTime, UBOOL bInitialUpdate)
{
	SCOPE_CYCLE_COUNTER(STAT_UpdateDynamicAudioParams);

	TWEAKABLE FLOAT MinDelayForObstructionCheckDynamic = 0.25f;
	TWEAKABLE FLOAT MinDelayForObstructionCheckStatic = 0.75f;
	TWEAKABLE FLOAT MinDelayForObstructionCheckDynamicLowSpec = 0.75f;
	TWEAKABLE FLOAT MinDelayForObstructionCheckStaticLowSpec = 1.5f;
	TWEAKABLE FLOAT MinDelayForOcclusionCheckDynamic = 0.25f;
	TWEAKABLE FLOAT MinDelayForOcclusionCheckStatic = 0.75f;
	TWEAKABLE FLOAT MinDelayForOcclusionCheckDynamicLowSpec = 1.0f;
	TWEAKABLE FLOAT MinDelayForOcclusionCheckStaticLowSpec = 2.0f;

	SndEnvVolumeArray srcVolumes;
	GetEnvironmentVolumes(sndSrc, srcVolumes);

	// Get the obstruction factor
	{	
		if (bInitialUpdate)
		{
			sndSrc->TargetObstruction = GetObstructionFactor(sndSrc, srcVolumes);
			sndSrc->CurrentObstruction = sndSrc->TargetObstruction;
			sndSrc->LastObstructionCheckTime = GWorld->GetTimeSeconds();			
		}
		else
		{
			if (sndSrc->CurrentOcclusion > 0.99f)
			{
				// optim - assume obstructed as well - skip the costly raycast
				sndSrc->TargetObstruction = 1.0f;
			}
			else
			{			
				FLOAT checkDelay = 1.0f;
			
				if (GSystemSettings.DetailMode == DM_Low)
				{
					checkDelay = sndSrc->bDynamic ? MinDelayForObstructionCheckDynamicLowSpec : MinDelayForObstructionCheckStaticLowSpec;
				}
				else
				{
					checkDelay = sndSrc->bDynamic ? MinDelayForObstructionCheckDynamic : MinDelayForObstructionCheckStatic;
				}

				if (GWorld->GetTimeSeconds() > sndSrc->LastObstructionCheckTime + checkDelay)
				{
					sndSrc->TargetObstruction = GetObstructionFactor(sndSrc, srcVolumes);
					sndSrc->LastObstructionCheckTime = GWorld->GetTimeSeconds();
				}
			}
			sndSrc->CurrentObstruction = Utils::Approach(sndSrc->CurrentObstruction, sndSrc->TargetObstruction, ObstructionApproachCoeff, deltaTime);
		}
	}
	
	// Get occlusion and virtualization params
	{	
		UBOOL bUpdateOcclusion = FALSE;
		VirtNodeArray virtualizationNodes;

		if (Utils::GetHero()->IsInLocker())
		{
			sndSrc->TargetOcclusion = LockerOcclusion;
		}
		else if (sndSrc->TargetObstruction < 0.01f) 
		{	
			sndSrc->TargetOcclusion = 0.0f; // no obstruction overrides occlusion calculations

			sndSrc->DebugInfo.bConnected = TRUE;
		}
		else
		{
			if (bInitialUpdate || sndSrc->bVirtualized)
			{
				bUpdateOcclusion = TRUE; // update every frame when virtualized
			}
			else if (Utils::GetHero()->ActiveDoor != NULL)
			{
				// Interacting with a door - update every frame
				bUpdateOcclusion = TRUE;
			}
			else 
			{
				FLOAT checkDelay = 1.0f;

				if (GSystemSettings.DetailMode == DM_Low)
				{
					checkDelay = sndSrc->bDynamic ? MinDelayForOcclusionCheckDynamicLowSpec : MinDelayForOcclusionCheckStaticLowSpec;
				}
				else
				{
					checkDelay = sndSrc->bDynamic ? MinDelayForOcclusionCheckDynamic : MinDelayForOcclusionCheckStatic;
				}

				bUpdateOcclusion = (GWorld->GetTimeSeconds() > (sndSrc->LastOcclusionCheckTime + checkDelay));
			}

			if (bUpdateOcclusion)
			{
#if !SHIPPING_PC_GAME && !FINAL_RELEASE
				DOUBLE timeStart = appSeconds();
#endif

				UBOOL bConnected = GetPathedAudioParams(sndSrc->TargetOcclusion, sndSrc->BaseLocation, srcVolumes, Utils::GetHero()->EyeLocation, virtualizationNodes);

#if !SHIPPING_PC_GAME && !FINAL_RELEASE
				DOUBLE timeEnd = appSeconds();
				FLOAT timeDiffMs = ((FLOAT)(timeEnd - timeStart) * 1000.0f);

				if (timeDiffMs >= 0.5f)
				{
					warnf(TEXT("PERF ISSUE! GetPathedAudioParams for %s took %.2f ms"), *sndSrc->Actor->GetName(), timeDiffMs);
				}
#endif

				sndSrc->DebugInfo.bConnected = bConnected;

				sndSrc->LastOcclusionCheckTime = GWorld->GetTimeSeconds();

				UpdateVirtualization(sndSrc, virtualizationNodes);
			}
		}

		if (!bUpdateOcclusion && sndSrc->bVirtualized)
		{
			sndSrc->bVirtualized = FALSE; // make sure we're not virtualized if we skipped the occlusion update
		}

		if (bInitialUpdate)
		{
			sndSrc->CurrentOcclusion = sndSrc->TargetOcclusion;
		}
		else
		{
			sndSrc->CurrentOcclusion = Utils::Approach(sndSrc->CurrentOcclusion, sndSrc->TargetOcclusion, OcclusionApproachCoeff, deltaTime);		
		}
	}

	if (!sndSrc->bInMultiPositionGroup)
	{
		UAkAudioDevice::Get()->SetObstructionAndOcclusionOnAllComponents(sndSrc->CurrentObstruction, sndSrc->CurrentOcclusion, sndSrc->Actor);
	}
}

void UOLSoundEnvironmentManager::FilterVirtualizationNodes(const VirtNodeArray& virtualizationNodes, const FVirtualizationNode*& level0Node, const FVirtualizationNode*& level1Node, FLOAT& baseWeight0, FLOAT& baseWeight1)
{
	TWEAKABLE FLOAT BlendStartConnectorWeight = 0.6f;
	TWEAKABLE FLOAT BlendStopConnectorWeight = 0.85f;
	
	const INT NbRelevantLevels = 2;

	const FVirtualizationNode* nodes[NbRelevantLevels] = { NULL };
	FLOAT baseWeights[2] = { 1.0f, 1.0f };
	FLOAT sumDistances[NbRelevantLevels] = { 0.0f };
	INT nbNodes[NbRelevantLevels] = { 0 };
	
	for (INT i = 0; i < virtualizationNodes.Num(); i++)
	{
		INT level = virtualizationNodes(i).Level;
		if (virtualizationNodes(i).Level < NbRelevantLevels)
		{
			sumDistances[level] += virtualizationNodes(i).DistToListener;
			nbNodes[level]++;
		}
	}

	for (INT i = 0; i < virtualizationNodes.Num(); i++)
	{
		INT level = virtualizationNodes(i).Level;
		if (virtualizationNodes(i).Level < NbRelevantLevels)
		{
			if (nbNodes[level] == 1)
			{
				nodes[level] = &virtualizationNodes(i);
			}
			else
			{
				FLOAT weight = 1.0f - virtualizationNodes(i).DistToListener / sumDistances[level];

				if (weight >= BlendStartConnectorWeight)
				{
					nodes[level] = &virtualizationNodes(i);
					baseWeights[level] = Saturate((weight - BlendStartConnectorWeight) / (BlendStopConnectorWeight - BlendStartConnectorWeight));
				}
			}
		}
	}

	level0Node = nodes[0];
	level1Node = nodes[1];
	baseWeight0 = baseWeights[0];
	baseWeight1 = baseWeights[1];
}

FVector UOLSoundEnvironmentManager::GetVirtualizedLocation(UOLSoundEmitter* sndSrc, const FVirtualizationNode* virtualizationNode, FLOAT baseWeight, FVector& out_Pivot)
{
	TWEAKABLE FLOAT MinVirtualizableDist = 350.0f;

	const FVector& listenerLocation = Utils::GetHero()->EyeLocation;
	
	FVector connectorLoc = virtualizationNode->Connector->GetCenterLocation();
	FVector connectorToListener = (listenerLocation - connectorLoc);
	
	FLOAT virtualizationWeight = Min(virtualizationNode->VirtualizableDistance / MinVirtualizableDist, baseWeight);
	virtualizationWeight = Min(virtualizationWeight, 1.0f - sndSrc->CurrentOcclusion); // occlusion goes through the walls, not open connectors

	// Find the pivot
	FVector closestToListener = virtualizationNode->Connector->GetClosestPoint(listenerLocation);
	FVector closestToSnd = virtualizationNode->Connector->GetClosestPoint(sndSrc->BaseLocation);
	FVector pivot = 0.5f * (closestToListener + closestToSnd);
	pivot.Z = listenerLocation.Z; // force pivot to be level with us; we do 2D virtualization (nobody has sound systems with up/down speakers so why bother)

	FVector toPivot = (pivot - listenerLocation);
	FVector toPivotDir = toPivot.SafeNormal2D();

	FVector virtualizedLoc = pivot + virtualizationNode->VirtualizableDistance * toPivotDir;

	if (virtualizationWeight < 1.0f)
	{
		// ideally we should slerp the rotation around the pivot, but just lerping the vectors and renormalizing does the job fine
		FVector interpLoc = Lerp(sndSrc->BaseLocation, virtualizedLoc, virtualizationWeight);
		FVector dirToInterpLoc = (interpLoc - pivot).SafeNormal2D();
		FLOAT actualDist = sndSrc->BaseLocation.Distance(pivot);
		FLOAT effectiveDist = Lerp(actualDist, virtualizationNode->VirtualizableDistance, virtualizationWeight); // lerp the distance as well
		virtualizedLoc = pivot + effectiveDist * dirToInterpLoc;
	}

	virtualizedLoc.Z = sndSrc->BaseLocation.Z;

	out_Pivot = pivot;

	return virtualizedLoc;
}

void UOLSoundEnvironmentManager::UpdateVirtualization(UOLSoundEmitter* sndSrc, const VirtNodeArray& virtualizationNodes)
{
	sndSrc->bVirtualized = FALSE;

	if (!bEnableSoundVirtualization || !sndSrc->bAllowVirtualization)
	{
		return;
	}

	if (sndSrc->TargetObstruction < 0.5f)
	{
		return; // no virtualization if we can see our target
	}

	TWEAKABLE FLOAT MinDistToConnector = 100.0f;
	TWEAKABLE FLOAT BlendDistToConnector = 300.0f;

	const FVirtualizationNode* virtualizationNode = NULL;
	const FVirtualizationNode* backupLevelOneNode = NULL;

	FLOAT baseWeight0 = 1.0f;
	FLOAT baseWeight1 = 1.0f;

	FilterVirtualizationNodes(virtualizationNodes, virtualizationNode, backupLevelOneNode, baseWeight0, baseWeight1);

	if (!virtualizationNode || !virtualizationNode->Connector || !virtualizationNode->Connector->bAllowSourceVirtualization)
	{
		return;
	}

	const FVector& listenerLocation = Utils::GetHero()->EyeLocation;
	FVector connectorLoc = virtualizationNode->Connector->GetCenterLocation();
	FVector connectorToListener = (listenerLocation - connectorLoc);

	UBOOL hasValidBackup = (backupLevelOneNode && backupLevelOneNode->Connector && backupLevelOneNode->Connector->bAllowSourceVirtualization);

	if (connectorToListener.SizeSquared2D() < Square(MinDistToConnector))
	{
		// Too close to this connector - use next level if available
		if (hasValidBackup)
		{
			FVector pivot;
			sndSrc->VirtualizedLocation = GetVirtualizedLocation(sndSrc, backupLevelOneNode, baseWeight1, pivot);
			sndSrc->DebugInfo.VirtualizationPivot = pivot;
		}
		else
		{
			return; // can't virtualize
		}		
	}
	else if (hasValidBackup && connectorToListener.SizeSquared2D() < Square(BlendDistToConnector))
	{
		// blend levels 0 and 1
		FVector pivot0;
		FVector pivot1;
		FVector level0 = GetVirtualizedLocation(sndSrc, virtualizationNode, baseWeight0, pivot0);
		FVector level1 = GetVirtualizedLocation(sndSrc, backupLevelOneNode, baseWeight1, pivot1);

		FLOAT distToConnector = connectorToListener.Size2D();
		FLOAT alpha = (distToConnector - MinDistToConnector) / (BlendDistToConnector - MinDistToConnector);

		sndSrc->VirtualizedLocation = Lerp(level1, level0, alpha);

		sndSrc->DebugInfo.VirtualizationPivot = Lerp(pivot1, pivot0, alpha);
	}
	else
	{
		// normal case - 1st level connector
		FVector pivot;
		sndSrc->VirtualizedLocation = GetVirtualizedLocation(sndSrc, virtualizationNode, baseWeight0, pivot);
		sndSrc->DebugInfo.VirtualizationPivot = pivot;
	}
		
	sndSrc->bVirtualized = TRUE;
}

void UOLSoundEnvironmentManager::UpdateReverb(UOLSoundEmitter* sndSrc)
{
	SCOPE_CYCLE_COUNTER(STAT_UpdateReverb);

	FLOAT totalContrib = 0.0f;

	sndSrc->ReverbBusInfos.Empty();

	if (sndSrc->Actor == Utils::GetHero() && Utils::GetHero()->IsInLocker())
	{
		// special case
		FAuxBusInfo auxBus;
		auxBus.BusName = LockerReverbEnvironment;
		auxBus.BusValue = 1.0f;
		sndSrc->ReverbBusInfos.AddItem(auxBus);
		totalContrib = 1.0f;
	}
	else 
	{
		UBOOL bHasValidSndEnv = FALSE;

		for (INT i = 0; i < sndSrc->SoundEnvironments.Num(); i++)
		{
			FSoundEnvVolumeData& params = sndSrc->SoundEnvironments(i);

			if (!params.SoundEnvVolume)
			{
				// failsafe in case it's been streamed out or something
				sndSrc->SoundEnvironments.Remove(i);
				i--;
				continue;
			}

			UOLSoundEnvironment* sndEnv = params.SoundEnvVolume->SoundEnvironment;
			if (sndEnv && !sndEnv->ReverbEnvironmentType.IsEmpty())
			{
				bHasValidSndEnv = TRUE;

				FLOAT contrib = 1.0f;

				if (params.bFadeFromBorder)
				{
					FLOAT pctAwayFromBorderPoint = Utils::SmootherStep(sndSrc->Actor->Location.Distance(params.BorderPoint), 0.0f, params.SoundEnvVolume->FadeDistance);

					if (params.bActive)
					{
						contrib = Max(params.LastActiveEnvContrib, pctAwayFromBorderPoint);
						params.LastActiveEnvContrib = contrib;
					}
					else
					{
						contrib = Saturate(params.LastActiveEnvContrib - pctAwayFromBorderPoint);
					}
				}

				totalContrib += contrib;

				UBOOL bFound = FALSE;
				for (INT j = 0; j < sndSrc->ReverbBusInfos.Num(); j++)
				{
					FAuxBusInfo& auxBus = sndSrc->ReverbBusInfos(j);
					if (auxBus.BusName == sndEnv->ReverbEnvironmentType)
					{
						auxBus.BusValue += contrib;
						bFound = TRUE;
						break;
					}
				}

				if (!bFound)
				{
					FAuxBusInfo auxBus;
					auxBus.BusName = sndEnv->ReverbEnvironmentType;
					auxBus.BusValue = contrib;
					sndSrc->ReverbBusInfos.AddItem(auxBus);
				}
			}
		}

		// Normalize contributions if they add up to more than 100%
		if (totalContrib > 1.0f)
		{
			for (INT i = 0; i < sndSrc->ReverbBusInfos.Num(); i++)
			{
				FAuxBusInfo& auxValue = sndSrc->ReverbBusInfos(i);
				auxValue.BusValue /= totalContrib;
			}
		}

		if (!bHasValidSndEnv && DefaultSoundEnvironment && !DefaultSoundEnvironment->ReverbEnvironmentType.IsEmpty())
		{
			// hook in the default reverb type
			FAuxBusInfo auxBus;
			auxBus.BusName = DefaultSoundEnvironment->ReverbEnvironmentType;
			auxBus.BusValue = 1.0f;
			sndSrc->ReverbBusInfos.AddItem(auxBus);
		}
	}

	if (!sndSrc->bInMultiPositionGroup)
	{
		TArray< UAkAudioDevice::AkAuxBusValue > values;
		values.Reserve(sndSrc->ReverbBusInfos.Num());

		for (INT i = 0; i < sndSrc->ReverbBusInfos.Num(); i++)
		{
			UAkAudioDevice::AkAuxBusValue busValue;
			busValue.AuxBus = *sndSrc->ReverbBusInfos(i).BusName;
			busValue.ControlValue = sndSrc->ReverbBusInfos(i).BusValue;
			values.AddItem(busValue);
		}

		UAkAudioDevice::Get()->SetGameObjectAuxSendValuesOnAllComponents(values, sndSrc->Actor);
	}

	UpdateDebugReverb(sndSrc);
}

void UOLSoundEnvironmentManager::UpdateSoundEnvironmentForStaticActor(UOLSoundEmitter* sndSrc)
{
	SCOPE_CYCLE_COUNTER(STAT_UpdateStaticActor);

	// We're static, we just have to remove dead volumes

	for (INT i = 0; i < sndSrc->SoundEnvironments.Num(); i++)
	{
		FSoundEnvVolumeData& params = sndSrc->SoundEnvironments(i);

		if (!params.SoundEnvVolume)
		{
			// Streamed out
			sndSrc->SoundEnvironments.Remove(i);
			i--;
			continue;
		}
	}
}

void UOLSoundEnvironmentManager::UpdateSoundEnvironmentForDynamicActor(UOLSoundEmitter* sndSrc, FLOAT deltaTime, UBOOL initialUpdate)
{
	SCOPE_CYCLE_COUNTER(STAT_UpdateDynamicActor);

	AActor* actor = sndSrc->Actor;

	UBOOL bIsListener = actor->IsA(AOLHero::StaticClass());

	SndEnvVolumeArray activeVolumes;

	// Find the active volumes
	if (actor->bCollideWorld)
	{
		for (INT i = 0; i < actor->Touching.Num(); i++)
		{
			AOLSoundEnvironmentVolume* audioVolume = Cast<AOLSoundEnvironmentVolume>(actor->Touching(i));

			if (audioVolume)
			{
				activeVolumes.AddItem(audioVolume);
			}
		}
	}
	else
	{
		for (INT i = 0; i < AllVolumes.Num(); ++i)
		{
			if (AllVolumes(i) != NULL && AllVolumes(i)->Encompasses(actor->Location + VecZ(20.0f)))
			{
				activeVolumes.AddItem(AllVolumes(i));
			}
		}
	}

	FilterVolumesByPriority(activeVolumes);
	
	// Iterate over the current environments
	for (INT i = 0; i < sndSrc->SoundEnvironments.Num(); i++)
	{
		FSoundEnvVolumeData& params = sndSrc->SoundEnvironments(i);

		if (!params.SoundEnvVolume)
		{
			// Streamed out
			sndSrc->SoundEnvironments.Remove(i);
			i--;
			continue;
		}

		if (activeVolumes.ContainsItem(params.SoundEnvVolume))
		{
			if (!params.bActive)
			{
				// Entering
				params.bActive = TRUE;
				params.LastActiveEnvContrib = 0.0f;
				params.BorderPoint = actor->Location;
				params.bFadeFromBorder = TRUE;

				if (bIsListener)
				{
					ListenerEnteredSoundEnvironment(sndSrc, params);
				}
				
			}

			activeVolumes.RemoveItem(params.SoundEnvVolume); // Trim the list gradually to only new volumes
		}
		else
		{
			if (params.bActive)
			{
				// Leaving
				params.bActive = FALSE;
				params.BorderPoint = actor->Location;
				params.bFadeFromBorder = TRUE;
			}
			else if (actor->Location.DistanceSquared(params.BorderPoint) > Square(params.SoundEnvVolume->FadeDistance))
			{
				if (bIsListener)
				{
					ListenerLeftSoundEnvironment(sndSrc, params);
				}

				// Dead - we're too far away
				sndSrc->SoundEnvironments.Remove(i);
				i--;
				continue;
			}
		}
	}

	// Add the new volumes
	for (INT i = 0; i < activeVolumes.Num(); i++)
	{
		FSoundEnvVolumeData params;
		params.SoundEnvVolume = activeVolumes(i);
		params.bActive = TRUE;
		params.BorderPoint = (initialUpdate ? FVector(FLT_MIN) : actor->Location);
		params.bFadeFromBorder = !initialUpdate;
		params.LastActiveEnvContrib = 0.0f;
		sndSrc->SoundEnvironments.AddItem(params);

		if (bIsListener)
		{
			ListenerEnteredSoundEnvironment(sndSrc, params);
		}
	}
}

void UOLSoundEnvironmentManager::ListenerEnteredSoundEnvironment(UOLSoundEmitter* sndSrc, FSoundEnvVolumeData& params)
{
	AOLHero* hero = Cast<AOLHero>(sndSrc->Actor);

	for (INT i = 0; i < params.SoundEnvVolume->OnEnterEvents.Num(); i++)
	{
		UAkEvent* evt = params.SoundEnvVolume->OnEnterEvents(i);

		UBOOL bSkip = FALSE;

		// Don't start a sound that's already been started by an active volume
		for (INT j = 0; !bSkip && j < sndSrc->SoundEnvironments.Num(); j++)
		{
			FSoundEnvVolumeData& otherParams = sndSrc->SoundEnvironments(j);

			if (otherParams.SoundEnvVolume == params.SoundEnvVolume)
			{
				continue;
			}

			if (!otherParams.bActive)
			{
				continue;
			}

			for (INT k = 0; k < otherParams.SoundEnvVolume->OnEnterEvents.Num(); k++)
			{
				UAkEvent* otherEvt = otherParams.SoundEnvVolume->OnEnterEvents(k);

				if (otherEvt == evt)
				{
					bSkip = TRUE; 
					break;
				}
			}
		}

		if (!bSkip)
		{
			warnf(TEXT("*** ENTER : %s -> %s"), *params.SoundEnvVolume->GetPathName(), *evt->GetPathName());
			hero->TriggerSoundEvent(evt);
		}
	}
}

void UOLSoundEnvironmentManager::ListenerLeftSoundEnvironment(UOLSoundEmitter* sndSrc, FSoundEnvVolumeData& params)
{
	AOLHero* hero = Utils::GetHero();

	for (INT i = 0; i < params.SoundEnvVolume->OnExitEvents.Num(); i++)
	{
		UAkEvent* evt = params.SoundEnvVolume->OnExitEvents(i);

		UBOOL bSkip = FALSE;

		// Don't stop a sound that's pending stop by another active volume
		for (INT j = 0; !bSkip && j < sndSrc->SoundEnvironments.Num(); j++)
		{
			FSoundEnvVolumeData& otherParams = sndSrc->SoundEnvironments(j);

			if (otherParams.SoundEnvVolume == params.SoundEnvVolume)
			{
				continue;
			}

			if (!otherParams.bActive)
			{
				continue;
			}

			for (INT k = 0; k < otherParams.SoundEnvVolume->OnExitEvents.Num(); k++)
			{
				UAkEvent* otherEvt = otherParams.SoundEnvVolume->OnExitEvents(k);

				if (otherEvt == evt)
				{
					bSkip = TRUE; 
					break;
				}
			}
		}

		if (!bSkip)
		{
			warnf(TEXT("*** EXIT : %s -> %s"), *params.SoundEnvVolume->GetPathName(), *evt->GetPathName());
			hero->TriggerSoundEvent(evt);
		}
	}
}

void UOLSoundEnvironmentManager::UpdateListenerVolumes()
{
	SCOPE_CYCLE_COUNTER(STAT_UpdateListenerVolumes);

	AOLHero* hero = Utils::GetHero();
	UOLSoundEmitter* heroSndEmitter = GetSoundEmitterForActor(hero);
	check(heroSndEmitter);

	AOLSoundEnvironmentVolume* backupListenerVolume = NULL; // if we have no active volume, choose an inactive one as backup (to forgive small cracks between volumes)
	ListenerVolumes.Empty();
	for (INT i = 0; i < heroSndEmitter->SoundEnvironments.Num(); i++)
	{
		if (!heroSndEmitter->SoundEnvironments(i).SoundEnvVolume)
		{
			// likely streamed out, make sure not to crash
			heroSndEmitter->SoundEnvironments.Remove(i);
			i--;
			continue; 
		}

		if (heroSndEmitter->SoundEnvironments(i).SoundEnvVolume->bUseForOcclusion)
		{
			if (heroSndEmitter->SoundEnvironments(i).bActive)
			{
				ListenerVolumes.AddItem(heroSndEmitter->SoundEnvironments(i).SoundEnvVolume);
			}
			backupListenerVolume = heroSndEmitter->SoundEnvironments(i).SoundEnvVolume; // arbitrarily choose the last - shouldn't happen often
		}
	}

	if (ListenerVolumes.Num() == 0 && backupListenerVolume)
	{
		ListenerVolumes.AddItem(backupListenerVolume);
	}
}

void UOLSoundEnvironmentManager::UpdateAllVolumes()
{
	TArray<AOLSoundConnector*> allConnectors;
	TArray<AOLDoor*> allDoors;

	AllVolumes.Empty();

	// Gather all relevant actors
	for (FActorIterator It; It; ++It)
	{
		AActor* actor = *It;
		if (actor && actor->IsAStaticMeshActor())
		{
			// early out of the most common case
			continue;
		}

		AOLSoundEnvironmentVolume* vol = Cast<AOLSoundEnvironmentVolume>(*It);
		if (vol)
		{
			AllVolumes.AddItem(vol);
			vol->Connections.Empty();
			continue;
		}

		AOLDoor* door = Cast<AOLDoor>(*It);
		if (door)
		{
			allDoors.AddItem(door);
			continue;
		}

		AOLSoundConnector* connector = Cast<AOLSoundConnector>(*It);
		if (connector)
		{
			allConnectors.AddItem(connector);
		}
	}

	// Connect through all doors
	for (INT i = 0; i < allDoors.Num(); i++)
	{
		AOLDoor* door = allDoors(i);
		door->SoundConnectorComp->ConnectVolumes(door->GetCenterLocation(), AllVolumes);
	}

	// Connect through all SoundConnectors
	for (INT i = 0; i < allConnectors.Num(); i++)
	{
		AOLSoundConnector* connector = allConnectors(i);
		connector->SoundConnectorComp->ConnectVolumes(connector->Location, AllVolumes);
	}

	// Run through all the uninitialized volumes and add them to existing actors
	for (INT i = 0; i < AllVolumes.Num(); i++)
	{
		AOLSoundEnvironmentVolume* vol = AllVolumes(i);
		if (!vol->bInitialized)
		{
			vol->RegisterOnStaticEmitters(ActiveSources);
			vol->bInitialized = TRUE;
		}
	}
}

UBOOL UOLSoundEnvironmentManager::GetPlayerOcclusionForPawn(APawn* pawn, FLOAT& out_Distance, FLOAT& out_Occlusion)
{
	AActor* listener = Utils::GetHero();
	UOLSoundEmitter* listenerSndEmitter = listener ? GetSoundEmitterForActor(listener) : NULL;

	if (!listenerSndEmitter || !listenerSndEmitter->Actor)
	{
		// hmm... maybe we just died
		out_Occlusion = 1.0f;
		out_Distance = 100000.0f;
		return FALSE;
	}

	const FVector& listenerLoc = listenerSndEmitter->Actor->Location;

	for (INT i = 0; i < ActiveSources.Num(); i++)
	{
		UOLSoundEmitter* sndSource = ActiveSources(i);

		if (sndSource->Actor == pawn)
		{
			out_Occlusion = sndSource->CurrentOcclusion;

			if (sndSource->bVirtualized)
			{
				out_Distance = sndSource->VirtualizedLocation.Distance(listenerLoc);
			}
			else
			{
				out_Distance = sndSource->BaseLocation.Distance(listenerLoc);
			}

			return TRUE;
		}		
	}

	// can't find the pawn in the active list (bug?), default to fully occluded
	out_Occlusion = 1.0f;
	out_Distance = 100000.0f;

	return FALSE;
}

UBOOL UOLSoundEnvironmentManager::GetPlayerOcclusionForActor(AActor* actor, FLOAT& out_Distance, FLOAT& out_Occlusion, FLOAT& out_Obstruction)
{
	AActor* listener = Utils::GetHero();
	UOLSoundEmitter* listenerSndEmitter = listener ? GetSoundEmitterForActor(listener) : NULL;

	if (!listenerSndEmitter || !listenerSndEmitter->Actor)
	{
		out_Occlusion = 1.0f;
		out_Obstruction = 1.0f;
		out_Distance = 100000.0f;
		return FALSE;
	}

	const FVector& listenerLoc = listenerSndEmitter->Actor->Location;

	for (INT i = 0; i < ActiveSources.Num(); i++)
	{
		UOLSoundEmitter* sndSource = ActiveSources(i);

		if (sndSource->Actor == actor)
		{
			out_Occlusion = sndSource->CurrentOcclusion;
			out_Obstruction = sndSource->CurrentObstruction;

			if (sndSource->bVirtualized)
			{
				out_Distance = sndSource->VirtualizedLocation.Distance(listenerLoc);
			}
			else
			{
				out_Distance = sndSource->BaseLocation.Distance(listenerLoc);
			}

			return TRUE;
		}		
	}

	out_Occlusion = 1.0f;
	out_Obstruction = 1.0f;
	out_Distance = 100000.0f;
	return FALSE;
}

void UOLSoundEnvironmentManager::AddSoundEmittingMeshToMultiPositionGroup(UOLSoundEmitter* sndSrc, const FString& EventName)
{
	check(!sndSrc->bInMultiPositionGroup);

	sndSrc->bInMultiPositionGroup = TRUE;
	sndSrc->GroupEventName = EventName;

	// Add to existing group if present
	for (INT i = 0; i < ActiveGroups.Num(); i++)
	{
		FMultiPositionGroup& group = ActiveGroups(i);
		check(group.Members.Num() > 0);
		check(group.Members(0));
		
		if (!group.Members(0)->Actor)
		{
			// During checkpoint transitions, we may have invalid groups (all actors NULLed out). Destroy them.			

			for (INT j = 0; j < group.Members.Num(); j++)
			{
				// For sanity's sake
				UOLSoundEmitter* slaveEmitter = group.Members(j);
				check(slaveEmitter);
				slaveEmitter->bInMultiPositionGroup = FALSE;
			}

			group.Members(0)->bGroupMaster = FALSE;
			ActiveGroups.Remove(i);
			i--;
			continue;
		}
		
		UBOOL sameEvent = (group.EventName == EventName);
		UBOOL sameLevel = (group.Members(0)->Actor->GetLevel() == sndSrc->Actor->GetLevel());
		UBOOL soundEmittingMeshGroup = group.Members(0)->Actor->IsA(AOLSoundEmittingMeshActor::StaticClass());

		if (sameEvent && sameLevel && soundEmittingMeshGroup)
		{
			if (group.Members.Num() >= MaxGroupMembers) // enforcing a soft limit, to simplify/optimise processing. Limit can be increased as needed.
			{
				// over the limit, this emitter won't be in the group
				sndSrc->bInMultiPositionGroup = FALSE;
				sndSrc->GroupEventName.Empty();
				return;
			}

			group.Members.AddItem(sndSrc);
			sndSrc->bGroupMaster = FALSE;

			//debugf(TEXT("### Adding %s to group %s"), *sndSrc->Actor->GetName(), *sndSrc->GroupEventName);
			return;
		}
	}

	// if not found, create, add self as master
	FMultiPositionGroup newGroup;
	newGroup.EventName = EventName;
	newGroup.Members.AddItem(sndSrc);
	sndSrc->bGroupMaster = TRUE;
	ActiveGroups.AddItem(newGroup);

	//debugf(TEXT("### %s master of new group %s"), *sndSrc->Actor->GetName(), *sndSrc->GroupEventName);
}

void UOLSoundEnvironmentManager::RemoveFromMultiPositionGroup(UOLSoundEmitter* sndSrc)
{
	check(sndSrc->bInMultiPositionGroup);
	sndSrc->bInMultiPositionGroup = FALSE;
	
	// find group
	for (INT i = 0; i < ActiveGroups.Num(); i++)
	{
		FMultiPositionGroup& group = ActiveGroups(i);
		if (group.Members.ContainsItem(sndSrc))
		{
			if (sndSrc->bGroupMaster)
			{
				for (INT j = 0; j < group.Members.Num(); j++)
				{
					// Remove slaves from group. Perhaps we should explicitely flag them as ghosts, but they should be removed from world very soon anyway
					UOLSoundEmitter* slaveEmitter = group.Members(j);
					check(slaveEmitter);
					slaveEmitter->bInMultiPositionGroup = FALSE;
				}

				sndSrc->bGroupMaster = FALSE;

				//debugf(TEXT("### Removing master %s along with group %s"), *sndSrc->Actor->GetName(), *sndSrc->GroupEventName);

				ActiveGroups.Remove(i);
			}
			else 
			{
				INT removedIdx = group.Members.RemoveItem(sndSrc);
				check(removedIdx >= 0);

				//debugf(TEXT("### Removing %s from group %s"), *sndSrc->Actor->GetName(), *sndSrc->GroupEventName);
			}

			return;
		}
	}	

	warnf(TEXT("UOLSoundEnvironmentManager::RemoveFromMultiPositionGroup - Owner group not found for %s!"), *sndSrc->GroupEventName);
}

FMultiPositionGroup* UOLSoundEnvironmentManager::GetGroupForEmitter(UOLSoundEmitter* sndSrc)
{
	for (INT i = 0; i < ActiveGroups.Num(); i++)
	{
		FMultiPositionGroup& group = ActiveGroups(i);
		if (group.Members.ContainsItem(sndSrc))
		{
			return &group;
		}
	}

	return NULL;
}

void UOLSoundEnvironmentManager::FilterVolumesByPriority(SndEnvVolumeArray& sndVolumes)
{
	INT highestPrio = -1;

	for (INT i = 0; i < sndVolumes.Num(); i++)
	{
		highestPrio = Max(highestPrio, (INT)sndVolumes(i)->Priority);
	}

	for (INT i = 0; i < sndVolumes.Num(); i++)
	{
		if (sndVolumes(i)->Priority < highestPrio)
		{
			// Remove lower priorities
			sndVolumes.Remove(i);
			i--;
			continue;
		}
	}
}

//////////////////////////////////////////////////////////////////////////
// Update
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void UOLSoundEnvironmentManager::Tick(FLOAT deltaTime)
{
	SCOPE_CYCLE_COUNTER(STAT_SoundEnvironmentManagerTickTime);

	AActor* listener = Utils::GetHero();
	
	if (!listener || !UAkAudioDevice::Get())
	{
		return;
	}

	UOLSoundEmitter* listenerSndEmitter = GetSoundEmitterForActor(listener);
	if (!listenerSndEmitter)
	{
		return;
	}

	if (bEnvironmentsDirty)
	{
		UpdateAllVolumes();
	}

	UpdateListenerVolumes();

	UpdateReverb(listenerSndEmitter);

	for (INT i = 0; i < ActiveSources.Num(); i++)
	{
		UOLSoundEmitter* sndSource = ActiveSources(i);

		if (sndSource->Actor == NULL || sndSource->Actor->IsPendingKill())
		{
			// Clear deleted / unstreamed / pendingkill
			if (sndSource->bInMultiPositionGroup)
			{
				RemoveFromMultiPositionGroup(sndSource);
			}

			ActiveSources.Remove(i);
			i--;
			continue;
		}

		if (!sndSource->bActive)
		{
			continue;
		}

		if (sndSource->bDynamic)
		{
			UpdateSoundEnvironmentForDynamicActor(sndSource, deltaTime, sndSource->bDirty);
		}
		else
		{
			UpdateSoundEnvironmentForStaticActor(sndSource);
		}

		if (sndSource->Actor != listener)
		{
			if (bEnvironmentsDirty || sndSource->bDynamic || sndSource->bDirty)
			{
				UpdateReverb(sndSource);
			}

			UpdateDynamicAudioParams(sndSource, deltaTime);
		}

		sndSource->bDirty = FALSE;
	}

	UpdateMultiPositionSources(deltaTime);
	UpdateFadeRTPC(deltaTime);

	bEnvironmentsDirty = FALSE;
}

void UOLSoundEnvironmentManager::UpdateMultiPositionSources(FLOAT deltaTime)
{
	SCOPE_CYCLE_COUNTER(STAT_UpdateMultiPositionSources);

	AActor* listener = Utils::GetHero();
	const FVector& listenerLoc = listener->Location;

	for (INT i = 0; i < ActiveGroups.Num(); i++)
	{
		FMultiPositionGroup& group = ActiveGroups(i);
		check(group.Members.Num() > 0);
		check(group.Members(0));

		UOLSoundEmitter* master = group.Members(0);
		check(master);

		if (!master->bActive)
		{
			continue;
		}

		AActor* masterActor = group.Members(0)->Actor;
		check(masterActor);

		if (group.Members.Num() == 1) // early out
		{
			UAkAudioDevice::Get()->SetObstructionAndOcclusionOnAllComponents(master->CurrentObstruction, master->CurrentOcclusion, masterActor);

			TArray<UAkAudioDevice::AkAuxBusValue> values;
			for (INT i = 0; i < master->ReverbBusInfos.Num(); i++)
			{
				UAkAudioDevice::AkAuxBusValue busValue;
				busValue.AuxBus = *master->ReverbBusInfos(i).BusName;
				busValue.ControlValue = master->ReverbBusInfos(i).BusValue;
				values.AddItem(busValue);
			}

			UAkAudioDevice::Get()->SetGameObjectAuxSendValuesOnAllComponents(values, masterActor);

			continue;
		}

		FLOAT totalContrib = 0.0f;
		FLOAT contribs[MaxGroupMembers] = {0.0f};

		// calc individual contribs
		for (INT j = 0; j < group.Members.Num(); j++)
		{
			UOLSoundEmitter* groupMember = group.Members(j);
			check(groupMember);
			FLOAT distToListener = group.Members(j)->Actor->Location.Distance(listenerLoc);
			contribs[j] = (1.0f - groupMember->CurrentOcclusion) / distToListener;
			totalContrib += contribs[j];
		}

		TArray<FAuxBusInfo> aggregatedBusInfos;
		FLOAT effectiveObstruction = 0.0f;
		FLOAT effectiveOcclusion = 0.0f;
		
		// Aggregate contribs
		if (totalContrib < 0.001f)
		{
			effectiveObstruction = master->CurrentObstruction;
			effectiveOcclusion = master->CurrentOcclusion;
			aggregatedBusInfos = master->ReverbBusInfos;
		}
		else
		{
			FLOAT totalReverbContrib = 0.0f;

			for (INT j = 0; j < group.Members.Num(); j++)
			{
				UOLSoundEmitter* groupMember = group.Members(j);
				check(groupMember);

				FLOAT nrmContrib = contribs[j] / totalContrib;
				effectiveObstruction += nrmContrib * groupMember->CurrentObstruction;
				effectiveOcclusion += nrmContrib * groupMember->CurrentOcclusion;

				FAuxBusInfo majorityBus;
				majorityBus.BusValue = 0.0f;

				for (INT k = 0; k < groupMember->ReverbBusInfos.Num(); k++)
				{
					FAuxBusInfo& busInfo = groupMember->ReverbBusInfos(k);
					if (busInfo.BusValue > majorityBus.BusValue)
					{
						majorityBus.BusValue = busInfo.BusValue;
						majorityBus.BusName = busInfo.BusName;
					}
				}

				FLOAT busContrib = nrmContrib * majorityBus.BusValue;

				if (busContrib <= 0.05f)
				{
					continue; // skip a negligeable contrib
				}

				UBOOL bFoundBus = FALSE;

				for (INT k = 0; k < aggregatedBusInfos.Num(); k++)
				{
					FAuxBusInfo& busInfo = aggregatedBusInfos(k);
					if (busInfo.BusName == majorityBus.BusName)
					{
						busInfo.BusValue += busContrib;
						totalReverbContrib += busContrib;
						bFoundBus = TRUE;
						break;
					}
				}

				if (!bFoundBus)
				{
					totalReverbContrib += busContrib;
					majorityBus.BusValue = busContrib;
					aggregatedBusInfos.AddItem(majorityBus);
				}
			}

			// Normalize contributions if they add up to more than 100%
			if (totalReverbContrib > 1.0f)
			{
				for (INT k = 0; k < aggregatedBusInfos.Num(); k++)
				{
					FAuxBusInfo& busInfo = aggregatedBusInfos(k);
					busInfo.BusValue /= totalReverbContrib;
				}
			}
		}

		effectiveObstruction = Saturate(effectiveObstruction);
		effectiveOcclusion = Saturate(effectiveOcclusion);
		
		UAkAudioDevice::Get()->SetObstructionAndOcclusionOnAllComponents(effectiveObstruction, effectiveOcclusion, masterActor);

		TArray<UAkAudioDevice::AkAuxBusValue> values;
		values.Reserve(aggregatedBusInfos.Num());

		for (INT i = 0; i < aggregatedBusInfos.Num(); i++)
		{
			UAkAudioDevice::AkAuxBusValue busValue;
			busValue.AuxBus = *aggregatedBusInfos(i).BusName;
			busValue.ControlValue = Saturate(aggregatedBusInfos(i).BusValue);
			values.AddItem(busValue);
		}

		UAkAudioDevice::Get()->SetGameObjectAuxSendValuesOnAllComponents(values, masterActor);
	}
}

void UOLSoundEnvironmentManager::InitializeSoundEmitter(UOLSoundEmitter* sndSrc)
{
	UpdateReverb(sndSrc);
	UpdateDynamicAudioParams(sndSrc, 0.0f, TRUE);
}

void UOLSoundEnvironmentManager::UpdateFadeRTPC(FLOAT deltaTime)
{
	AOLPlayerController* OLPC = Utils::GetOLPC();

	FLOAT effectiveFadeAmount = 0.0f;

	if (OLPC && !Utils::IsInMainMenu())
	{		
		if (OLPC->PlayerCamera && OLPC->PlayerCamera->bEnableFading)
		{
			effectiveFadeAmount = OLPC->PlayerCamera->FadeAmount;
		}
		else if (OLPC->bEnableFading)
		{
			effectiveFadeAmount = OLPC->FadeAmount;
		}
	}

	UAkAudioDevice* AudioDevice = UAkAudioDevice::Get();
	if (AudioDevice && FadeRTPC != NAME_None)
	{
		AudioDevice->SetRTPCValue(*FadeRTPC.ToString(), 100.0f*Saturate(effectiveFadeAmount), NULL);
	}
}

void UOLSoundEnvironmentManager::Cleanup()
{
	ListenerVolumes.Empty();
	ActiveSources.Empty();
	ActiveGroups.Empty();
	bPendingDestroy = TRUE;
}

void UOLSoundEnvironmentManager::ExitAllTouchingVolumes()
{
	AOLHero* Hero = Utils::GetHero();

	if (Hero)
	{
		for (INT TouchingIdx = 0; TouchingIdx < Hero->Touching.Num(); TouchingIdx++)
		{
			AOLSoundEnvironmentVolume* AudioVolume = Cast<AOLSoundEnvironmentVolume>(Hero->Touching(TouchingIdx));

			if (AudioVolume)
			{
				for (INT EventIdx = 0; EventIdx < AudioVolume->OnExitEvents.Num(); EventIdx++)
				{
					UAkEvent* Event = AudioVolume->OnExitEvents(EventIdx);
					Hero->TriggerSoundEvent(Event);
					warnf(TEXT("*** EXIT ALL : %s -> %s"), *GetPathName(), *Event->GetPathName());
				}	
			}
		}
	}
}

//////////////////////////////////////////////////////////////////////////
// AOLSoundEmittingMeshActor
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void AOLSoundEmittingMeshActor::PostBeginPlay()
{
	Super::PostBeginPlay();

	UAkAudioDevice * AudioDevice = UAkAudioDevice::Get();

	if (GIsGame && AudioDevice && PlayEvent)
	{
		UOLSoundEnvironmentManager* sndEnvMgr = Utils::GetSoundEnvManager();
		if (sndEnvMgr)
		{
			UOLSoundEmitter* sndSrc = sndEnvMgr->RegisterSoundEmitterForSoundEmittingMeshActor(this);
			check(sndSrc);

			if (!sndSrc->bInMultiPositionGroup || sndSrc->bGroupMaster)
			{
				AudioDevice->PostEvent(PlayEvent, this, NAME_None);
			}
		}
		else
		{
			warnf(TEXT("### - %s PostBeginPlay() - No Sound Env Manager, can't initialize"), *GetName());
		}
	}
}
	
void AOLSoundEmittingMeshActor::OnRemoveFromWorld()
{
	UAkAudioDevice * AudioDevice = UAkAudioDevice::Get();

	if (GIsGame && AudioDevice && PlayEvent)
	{
		UOLSoundEmitter* sndSrc = Utils::GetSoundEnvManager()->GetSoundEmitterForActor(this);
		if (!sndSrc || !sndSrc->bInMultiPositionGroup || sndSrc->bGroupMaster)
		{
			for (INT i = 0; i < AllComponents.Num(); i++)
			{
				UAkComponent * pComponent = Cast<UAkComponent>(AllComponents(i));
				if ( pComponent) 
				{
					pComponent->Stop();					
				}
			}
		}
		
		check(Utils::GetSoundEnvManager());
		Utils::GetSoundEnvManager()->UnregisterSoundEmitterForActor(this);
	}

	Super::OnRemoveFromWorld();
}

//////////////////////////////////////////////////////////////////////////
// Debug
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void UOLSoundEnvironmentManager::UpdateDebugReverb(UOLSoundEmitter* sndSrc)
{
	FLOAT reverbA = 0.0f;
	FLOAT reverbB = 0.0f;
	FString revNameA;
	FString revNameB;

	for (INT i = 0; i < sndSrc->ReverbBusInfos.Num(); i++)
	{
		const FAuxBusInfo& busInfo = sndSrc->ReverbBusInfos(i);
		
		if (busInfo.BusName == revNameA)
		{
			reverbA += busInfo.BusValue; // shouldn't happen
		}
		else if (busInfo.BusName == revNameB)
		{
			reverbB += busInfo.BusValue; // shouldn't happen
		}
		else if (busInfo.BusValue > reverbA)
		{
			// Push back - A is always stronger than B, B gets thrown away is required
			revNameB = revNameA;
			reverbB = reverbA;			
			revNameA = busInfo.BusName;
			reverbA = busInfo.BusValue;
		}
		else if (busInfo.BusValue > reverbB)
		{
			revNameB = busInfo.BusName;
			reverbB = busInfo.BusValue;
		}
	}

	sndSrc->DebugInfo.ReverbA = revNameA;
	sndSrc->DebugInfo.ReverbAContrib = reverbA;
	sndSrc->DebugInfo.ReverbB = revNameB;
	sndSrc->DebugInfo.ReverbBContrib = reverbB;
}

IMPLEMENT_COMPARE_CONSTPOINTER( UOLSoundEmitter, OLAudio, { return (A->DebugInfo.DirectDistToListener < B->DebugInfo.DirectDistToListener) ? -1 : 1; } )

void UOLSoundEnvironmentManager::DisplayDebug(UCanvas* canvas)
{
	if (!Utils::GetCheatManager())
	{
		return;
	}

#if !SHIPPING_PC_GAME && !FINAL_RELEASE
	
	AActor* listener = Utils::GetHero();
	const FVector& listenerLoc = listener->Location;

	TArray<UOLSoundEmitter*> SortedSources = ActiveSources;
	
	INT nbStatic = 0;
	INT nbDynamic = 0;

	// Gather some stats
	for (INT i = 0; i < SortedSources.Num(); i++)
	{
		UOLSoundEmitter* sndSrc = SortedSources(i);

		if (!sndSrc->Actor)
		{
			continue;
		}

		if (sndSrc->Actor == listener)
		{
			sndSrc->DebugInfo.DirectDistToListener = 0.0f;
			sndSrc->DebugInfo.VirtualizedDist = 0.0f;
			continue;
		}

		if (sndSrc->bActive)
		{
			if (sndSrc->bDynamic)
			{
				nbDynamic++;
			}
			else
			{
				nbStatic++;
			}
		}

		sndSrc->DebugInfo.DirectDistToListener = sndSrc->BaseLocation.Distance(listenerLoc);

		if (sndSrc->bVirtualized)
		{
			sndSrc->DebugInfo.VirtualizedDist = sndSrc->VirtualizedLocation.Distance(sndSrc->DebugInfo.VirtualizationPivot) + sndSrc->DebugInfo.VirtualizationPivot.Distance(listenerLoc);
		}
		else
		{
			sndSrc->DebugInfo.VirtualizedDist = 0.0f;
		}
	}

	// Sort by distance (closest first)
	Sort<USE_COMPARE_CONSTPOINTER( UOLSoundEmitter, OLAudio )>( SortedSources.GetTypedData(), SortedSources.Num() );

	TWEAKABLE FLOAT PosY = 15.0f;
	TWEAKABLE FLOAT YL = 18.0f;
	TWEAKABLE FLOAT BottomYSpacing = 40.0f;
	
	FLOAT posY = PosY;

	canvas->SetDrawColor(0, 82, 153);

	TWEAKABLE FLOAT col0 = 10.0f;	// dist
	TWEAKABLE FLOAT col1 = col0 + 100.0f;	// source
	TWEAKABLE FLOAT col2 = col1 + 460.0f; // occlusion
	TWEAKABLE FLOAT col3 = col2 + 100.0f; // obstruction
	TWEAKABLE FLOAT col4 = col3 + 100.0f; // virt?
	TWEAKABLE FLOAT col5 = col3 + 60.0f; // group?
	TWEAKABLE FLOAT col6 = col4 + 60.0f; // reverb A
	TWEAKABLE FLOAT col7 = col5 + 180.0f; // reverb B

	TWEAKABLE FLOAT heroCol0 = 10.0f;	// # sources (static / dynamic)
	TWEAKABLE FLOAT heroCol1 = heroCol0 + 170.0f;	// # groups
	TWEAKABLE FLOAT heroCol2 = heroCol1 + 50.0f; // # volumes
	TWEAKABLE FLOAT heroCol3 = heroCol2 + 50.0f; // Listener volumes

	// Listener info

	UOLSoundEmitter* listenerSndEmitter = GetSoundEmitterForActor(listener);
	if (listenerSndEmitter)
	{
		canvas->SetPos(heroCol0, posY);
		canvas->DrawText(TEXT("Sources (sta / dyn)"));
		canvas->SetPos(heroCol1, posY);
		canvas->DrawText(TEXT("# Gr"));
		canvas->SetPos(heroCol2, posY);
		canvas->DrawText(TEXT("# Vol"));
		canvas->SetPos(heroCol3, posY);
		canvas->DrawText(TEXT("Listener Volumes"));
		posY += YL;	

		DrawLine2D(canvas->Canvas, FVector2D(heroCol0-5.0f, posY), FVector2D(col7+100.0f, posY), canvas->DrawColor);
		posY += YL;	

		canvas->SetDrawColor(32, 150, 255);

		canvas->SetPos(heroCol0, posY);
		canvas->DrawText(FString::Printf(TEXT("%d (%d / %d)"), nbDynamic + nbStatic, nbStatic, nbDynamic));

		canvas->SetPos(heroCol1, posY);
		canvas->DrawText(FString::Printf(TEXT("%d"), ActiveGroups.Num()));

		canvas->SetPos(heroCol2, posY);
		canvas->DrawText(FString::Printf(TEXT("%d"), AllVolumes.Num()));

		canvas->SetPos(heroCol3, posY);
		{
			FString listenerVolumesStr = TEXT("[none]");
			
			for (INT i = 0; i < ListenerVolumes.Num(); i++)
			{
				INT nbConnections = 0;
				INT nbConnectors = 0;
				for (INT j = 0; j < ListenerVolumes(i)->Connections.Num(); j++)
				{
					UOLSoundConnectorComponent* connector = ListenerVolumes(i)->Connections(j);
					if (connector)
					{
						nbConnectors++;
						nbConnections += connector->ConnectedVolumes.Num() - 1;
					}
				}

				FString prioStr = TEXT("");

				switch (ListenerVolumes(i)->Priority)
				{
				case EVP_VeryLow:
					prioStr = TEXT("VeryLow, ");
					break;
				case EVP_Low:
					prioStr = TEXT("Low, ");
					break;
				case EVP_High:
					prioStr = TEXT("High, ");
					break;
				case EVP_VeryHigh:
					prioStr = TEXT("VeryHigh, ");
					break;
				}

				if (i == 0)
				{
					listenerVolumesStr = FString::Printf(TEXT("%s [%s%d connectors to %d volumes]"), *ListenerVolumes(i)->GetName(), *prioStr, nbConnectors, nbConnections);
				}
				else
				{
					listenerVolumesStr = listenerVolumesStr + FString::Printf(TEXT(", %s [%s%d -> %d]"), *ListenerVolumes(i)->GetName(), *prioStr, nbConnectors, nbConnections);
				}
			}
			canvas->DrawText(listenerVolumesStr);
		}

		posY += YL;	
		posY += YL;	

	}

	// Audio sources

	canvas->SetDrawColor(0, 82, 153);
	
	canvas->SetPos(col0, posY);
	canvas->DrawText(TEXT("Dist (m)"));
	canvas->SetPos(col1, posY);
	canvas->DrawText(TEXT("Source"));
	canvas->SetPos(col2, posY);
	canvas->DrawText(TEXT("Occl."));
	canvas->SetPos(col3, posY);
	canvas->DrawText(TEXT("Obst."));
	canvas->SetPos(col4, posY);
	canvas->DrawText(TEXT("Vir"));
	canvas->SetPos(col5, posY);
	canvas->DrawText(TEXT("Gr"));
	canvas->SetPos(col6, posY);
	canvas->DrawText(TEXT("Reverb"));
	posY += YL;	

	DrawLine2D(canvas->Canvas, FVector2D(col0-5.0f, posY), FVector2D(col7+100.0f, posY), canvas->DrawColor);
	posY += YL;	

	canvas->SetDrawColor(32, 150, 255);
	INT nbFiltered = 0;
	FString debugFilter = Utils::GetCheatManager()->DebugSoundEnvFilter;

	INT nbShown = 0;
	INT nbHidden = 0;

	for (INT i = 0; i < SortedSources.Num(); i++)
	{
		UOLSoundEmitter* sndSrc = SortedSources(i);

		if (!sndSrc->Actor)
		{
			continue;
		}

		FString audioSrcName = sndSrc->Actor->GetName();

		AOLAmbientSound* ambientSound = Cast<AOLAmbientSound>(sndSrc->Actor);
		if (ambientSound && ambientSound->PlayEvent)
		{
			audioSrcName = FString::Printf(TEXT("%s [%s]"), *audioSrcName, *ambientSound->PlayEvent->GetName());
		}

		AOLAmbientSoundClone* ambientSoundClone = Cast<AOLAmbientSoundClone>(sndSrc->Actor);
		if (ambientSoundClone && ambientSoundClone->Master && ambientSoundClone->Master->PlayEvent)
		{
			audioSrcName = FString::Printf(TEXT("%s [%s]"), *audioSrcName, *ambientSoundClone->Master->PlayEvent->GetName());
		}

		AOLSoundEmittingMeshActor* emittingMesh = Cast<AOLSoundEmittingMeshActor>(sndSrc->Actor);
		if (emittingMesh && emittingMesh->PlayEvent)
		{
			audioSrcName = FString::Printf(TEXT("%s [%s]"), *audioSrcName, *emittingMesh->PlayEvent->GetName());
		}

		if (!debugFilter.IsEmpty())
		{
			if (debugFilter.StartsWith(TEXT("-")))
			{
				// negative testing - reject if matched
				if (audioSrcName.InStr(debugFilter.RightChop(1), FALSE, TRUE) != INDEX_NONE)
				{
					nbFiltered++;
					continue;
				}
			}
			else
			{
				// match the filter
				if (audioSrcName.InStr(debugFilter, FALSE, TRUE) == INDEX_NONE)
				{
					nbFiltered++;
					continue;
				}
			}
		}

		if (posY > canvas->SizeY - BottomYSpacing)
		{
			nbHidden++;
			continue;
		}

		nbShown++;

		canvas->SetDrawColor(32, 150, 255);

		canvas->SetPos(col0, posY);
		canvas->DrawText(FString::Printf(TEXT("%3.1f"), 0.01f*sndSrc->DebugInfo.DirectDistToListener));
		
		canvas->SetPos(col1, posY);

		if (!sndSrc->bActive)
		{
			canvas->SetDrawColor(128, 128, 128);
		}
		else if (sndSrc->DebugInfo.bConnected)
		{
			canvas->SetDrawColor(191, 239, 255);
		}
		else
		{
			canvas->SetDrawColor(32, 150, 255);
		}
		
		canvas->DrawText(audioSrcName);
		canvas->SetDrawColor(32, 150, 255);

		canvas->SetPos(col2, posY);

		if (appIsNearlyEqual(sndSrc->CurrentOcclusion, sndSrc->TargetOcclusion, KINDA_SMALL_NUMBERF))
		{
			canvas->DrawText(FString::Printf(TEXT("%.0f%%"), 100.0f*sndSrc->CurrentOcclusion));
		}
		else
		{
			canvas->SetDrawColor(255, 150, 32);
			canvas->DrawText(FString::Printf(TEXT("%.0f%%"), 100.0f*sndSrc->CurrentOcclusion));
			canvas->SetDrawColor(32, 150, 255);
		}
		
		canvas->SetPos(col3, posY);

		if (appIsNearlyEqual(sndSrc->CurrentObstruction, sndSrc->TargetObstruction, KINDA_SMALL_NUMBERF))
		{
			canvas->DrawText(FString::Printf(TEXT("%.0f%%"), 100.0f*sndSrc->CurrentObstruction));
		}
		else
		{
			canvas->SetDrawColor(255, 150, 32);
			canvas->DrawText(FString::Printf(TEXT("%.0f%%"), 100.0f*sndSrc->CurrentObstruction));
			canvas->SetDrawColor(32, 150, 255);
		}		

		canvas->SetPos(col4, posY);
		if (sndSrc->bVirtualized)
		{
			canvas->SetDrawColor(102, 217, 255);
			canvas->DrawText(TEXT("Y"));
			canvas->SetDrawColor(32, 150, 255);
		}
		else if (sndSrc->bAllowVirtualization)
		{
			canvas->DrawText(TEXT("N"));
		}
		else
		{
			canvas->DrawText(TEXT("-"));
		}

		canvas->SetPos(col5, posY);
		if (sndSrc->bInMultiPositionGroup)
		{
			INT groupIdx = -1;

			for (INT j = 0; j < ActiveGroups.Num(); j++)
			{
				FMultiPositionGroup& group = ActiveGroups(j);
				if (group.Members.ContainsItem(sndSrc))
				{
					groupIdx = j;
					break;
				}
			}

			if (sndSrc->bGroupMaster)
			{
				canvas->DrawText(FString::Printf(TEXT("M%d"), groupIdx+1));
			}
			else if (sndSrc->bInMultiPositionGroup)
			{
				canvas->SetDrawColor(242, 255, 102);
				canvas->DrawText(FString::Printf(TEXT("C%d"), groupIdx+1));
				canvas->SetDrawColor(32, 150, 255);
			}
		}
		else
		{
			canvas->DrawText(TEXT("-"));
		}

		if (sndSrc->DebugInfo.ReverbAContrib > 0.01f)
		{
			if (sndSrc->DebugInfo.ReverbAContrib < 0.99f)
			{
				canvas->SetPos(col6, posY);
				canvas->DrawText(FString::Printf(TEXT("%s [%.0f%%]"), *sndSrc->DebugInfo.ReverbA, 100.0f*sndSrc->DebugInfo.ReverbAContrib));
			}
			else
			{
				canvas->SetPos(col6, posY);
				canvas->DrawText(FString::Printf(TEXT("%s"), *sndSrc->DebugInfo.ReverbA));
			}

			if (sndSrc->DebugInfo.ReverbBContrib > 0.01f)
			{
				canvas->SetPos(col7, posY);
				canvas->DrawText(FString::Printf(TEXT("%s [%.0f%%]"), *sndSrc->DebugInfo.ReverbB, 100.0f*sndSrc->DebugInfo.ReverbBContrib));
			}
		}
		else
		{
			canvas->SetPos(col6, posY);
			canvas->DrawText(TEXT("[none]"));
		}

		posY += YL;	
	}

	canvas->SetDrawColor(0, 82, 153);

	if (nbHidden > 0)
	{
		canvas->SetPos(col0, posY);

		if (nbFiltered > 0)
		{
			canvas->DrawText(FString::Printf(TEXT("[%d not shown, %d filtered with '%s']"), nbHidden, nbFiltered, *debugFilter));
		}
		else
		{
			canvas->DrawText(FString::Printf(TEXT("[%d not shown]"), nbHidden));
		}
	}
	else if (nbFiltered > 0)
	{
		canvas->SetPos(col0, posY);
		canvas->DrawText(FString::Printf(TEXT("[%d filtered with '%s']"), nbFiltered, *debugFilter));
	}
#endif
}

void UOLSoundEnvironmentManager::DrawDebug(const FVector& baseLocation)
{
#if !SHIPPING_PC_GAME && !FINAL_RELEASE
	AOLPlayerController* OLPC = Utils::GetOLPC();

	// connectors
	for (FActorIterator It; It; ++It)
	{
		TWEAKABLE FLOAT MaxDist = 2000.0f;

		AOLSoundConnector* sndConnector = Cast<AOLSoundConnector>(*It);
		if (sndConnector && sndConnector->SoundConnectorComp->bEnabled && sndConnector->Location.DistanceSquared(baseLocation) < Square(MaxDist))
		{
			if (sndConnector->SoundConnectorComp->bSpherical)
			{
				FLOAT radius = sndConnector->SoundConnectorComp->SphereRadius;
				if (sndConnector->SoundConnectorComp->bAllowSourceVirtualization)
				{
					OLPC->DrawDebugSphere(sndConnector->Location + VecZ(radius), radius, 8, 204, 41, 41);
				}
				else
				{
					OLPC->DrawDebugSphere(sndConnector->Location + VecZ(radius), radius, 8, 204, 82, 41);
				}
			}
			else
			{				
				if (sndConnector->SoundConnectorComp->bAllowSourceVirtualization)
				{
					OLPC->DrawDebugBoxOriented(sndConnector->LocalToWorld().ConcatTranslation(sndConnector->BoxPreviewComp->Translation), sndConnector->BoxPreviewComp->BoxExtent, 204, 41, 41);
				}
				else
				{
					OLPC->DrawDebugBoxOriented(sndConnector->LocalToWorld().ConcatTranslation(sndConnector->BoxPreviewComp->Translation), sndConnector->BoxPreviewComp->BoxExtent, 204, 82, 41);
				}
			}
		}
	}

	// sources
	for (INT i = 0; i < ActiveSources.Num(); i++)
	{
		const UOLSoundEmitter* sndSrc = ActiveSources(i);

		if (!sndSrc->Actor)
		{
			continue;
		}

		TWEAKABLE FLOAT MaxDist = 2000.0f;
		if (sndSrc->BaseLocation.DistanceSquared(baseLocation) < Square(MaxDist))
		{
			if (sndSrc->bVirtualized)
			{
				OLPC->DrawDebugSphere(sndSrc->BaseLocation, 15.0f, 8, 163, 204, 166);
				OLPC->DrawDebugSphere(sndSrc->VirtualizedLocation, 15.0f, 8, 41, 204, 190);
				OLPC->DrawDebugLine( sndSrc->BaseLocation, sndSrc->VirtualizedLocation, 41, 204, 190);
				OLPC->DrawDebugLine( sndSrc->BaseLocation, sndSrc->DebugInfo.VirtualizationPivot, 184, 204, 202);
				OLPC->DrawDebugLine( sndSrc->VirtualizedLocation, sndSrc->DebugInfo.VirtualizationPivot, 184, 204, 202);
			}
			else if (sndSrc->bInMultiPositionGroup && !sndSrc->bGroupMaster)
			{
				OLPC->DrawDebugSphere(sndSrc->BaseLocation, 15.0f, 8, 255, 229, 0);
			}
			else
			{
				OLPC->DrawDebugSphere(sndSrc->BaseLocation, 15.0f, 8, 41, 204, 52);
			}
		}
	}
#endif
}

