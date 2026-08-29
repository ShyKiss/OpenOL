#include "OLGame.h"
#include "AkAudioDevice.h"
#include "DebugRenderSceneProxy.h"

IMPLEMENT_CLASS(AOLPushableObject);

////////////////////////////////////////////////////////////////////////////////////////////
// Dynamics
////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////

void AOLPushableObject::PostBeginPlay()
{
	Super::PostBeginPlay();
	RegisterObstacle();
	UpdateLinkedDoor();
}

void AOLPushableObject::BeginDestroy()
{
	Super::BeginDestroy();
	UnRegisterObstacle();
}

void AOLPushableObject::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	Reset();
}

void AOLPushableObject::PostLoad()
{
	Super::PostLoad();
	Reset();
}

UBOOL AOLPushableObject::Tick(FLOAT deltaTime, ELevelTick tickType)
{
	if (tickType == LEVELTICK_ViewportsOnly)
	{
		return Super::Tick(deltaTime, tickType);
	}

	// Fallback: stop net-locked sound if no push packet received for > 0.5s
	// (handles case where all bPushing=0 redundant packets were lost)
	if (bNetLocked && GWorld && (GWorld->GetTimeSeconds() - NetPushLastTime) > 0.5f)
	{
		bNetLocked    = FALSE;
		RemotePushSeq = 0;
		NetStopPushing();
		PostAkEvent(SndStopPushing);
		StopMoving();
	}

	FLOAT targetVel = bPushActive ? (bPushFwd ? MaxSpeed : -MaxSpeed) : 0.0f;
	CurrentVelocity = Utils::Approach(CurrentVelocity, targetVel, Abs(targetVel) > Abs(CurrentVelocity) ? AccelApproachCoeff : DecelApproachCoeff, deltaTime);

	FLOAT phaseModulator = 0.5f - 0.5f*appCos(CurrentPhase * 2.0f * PI);
	TWEAKABLE FLOAT MinVelComp = 0.2f;
	FLOAT effectiveVel = (MinVelComp + (1.0f - MinVelComp)*phaseModulator) * CurrentVelocity;
	FLOAT delta = effectiveVel * deltaTime;
	SetDisplacement(CurrentDisplacement + delta);
	
	FLOAT rtpcValue = 0.0f;
	if (CurrentPhase < 0.5f)
	{
		rtpcValue = MapClamped(CurrentPhase, 0.0f, 0.5f, 0.0f, 100.0f);
	}
	else if (CurrentPhase < 0.75f)
	{
		rtpcValue = 100.0f;
	}
	else
	{
		rtpcValue = MapClamped(CurrentPhase, 0.75f, 1.0f, 100.0f, 0.0f);
	}
	SetRTPCValue(RTPCPushingSpeed, 100.0f - rtpcValue);

	return Super::Tick(deltaTime, tickType);
}

void AOLPushableObject::Reset()
{
	// Careful - Called from editor as well as game
	bPlayerLocked = FALSE;
	bPushActive   = FALSE;
	CurrentVelocity = 0.0f;
	if (bNetLocked)
	{
		bNetLocked    = FALSE;
		RemotePushSeq = 0;
		NetStopPushing();
		PostAkEvent(SndStopPushing);
	}

	if (GWorld && GWorld->HasBegunPlay())
	{
		UnRegisterObstacle();
		SetDisplacement(0.0f);	
		RegisterObstacle();

		UpdateLinkedDoor();
	}
	else
	{
		SetDisplacement(0.0f);
	}
}

void AOLPushableObject::SetDisplacement(FLOAT newDisplacement)
{
	CurrentDisplacement = Clamp(newDisplacement, -MaxBackDist, MaxFwdDist);

	FLOAT targetTranslation = BaseTranslation + CurrentDisplacement;
	if (!appIsNearlyEqual(targetTranslation, Mesh->Translation.X))
	{
		Mesh->Translation.X = targetTranslation;
		Mesh->ConditionalUpdateTransform();
		UpdateLinkedDoor();
	}
}

void AOLPushableObject::SetNetDisplacement(FLOAT newDisplacement)
{
	// Remote replication: only move the mesh visually, skip physics/door/obstacle logic.
	// bPlayerLocked means the local player is pushing this object — never override that.
	if (bPlayerLocked)
		return;
	CurrentDisplacement = Clamp(newDisplacement, -MaxBackDist, MaxFwdDist);
	if (Mesh)
	{
		FLOAT targetTranslation = BaseTranslation + CurrentDisplacement;
		if (!appIsNearlyEqual(targetTranslation, Mesh->Translation.X))
		{
			Mesh->Translation.X = targetTranslation;
			Mesh->ConditionalUpdateTransform();
		}
	}
}

void AOLPushableObject::UpdateLinkedDoorState()
{
	UpdateLinkedDoor();
}

void AOLPushableObject::NetStartPushing()
{
	UnRegisterObstacle();
}

void AOLPushableObject::NetStopPushing()
{
	RegisterObstacle();
}

void AOLPushableObject::ForcePushLeft()
{
	if (!bPlayerLocked)
	{
		UnRegisterObstacle();

		bPushActive = FALSE;
		CurrentVelocity = 0.0f;
		SetDisplacement(-MaxBackDist);

		RegisterObstacle();
	}
}

void AOLPushableObject::ForcePushRight()
{
	if (!bPlayerLocked)
	{
		UnRegisterObstacle();

		bPushActive = FALSE;
		CurrentVelocity = 0.0f;
		SetDisplacement(MaxFwdDist);

		RegisterObstacle();
	}
}

void AOLPushableObject::StartPushing()
{
	bPlayerLocked = TRUE;
	bPushActive = FALSE;
	CurrentVelocity = 0.0f;

	UnRegisterObstacle();

	SetRTPCValue(RTPCPushingSpeed, 100.0f);

	TriggerEvent(PET_StartedPushing, Utils::GetHero());
}

void AOLPushableObject::StopPushing()
{
	if (bPushActive)
	{
		PostAkEvent(SndStopPushing);
	}

	bPlayerLocked = FALSE;
	bPushActive = FALSE;
	CurrentVelocity = 0.0f;

	RegisterObstacle();

	TriggerEvent(PET_StoppedPushing, Utils::GetHero());
}

UBOOL AOLPushableObject::TryPushFwd()
{
	if (CanPushFwd())
	{
		if (!bPushActive)
		{
			SetSwitch(SwitchPushableType, PushableType == PM_Wood ? SwitchPushableTypeWood : SwitchPushableTypeMetal);
			PostAkEvent(SndStartPushing);
			SetRTPCValue(RTPCPushingSpeed, 100.0f);
		}

		bPushActive = TRUE;
		bPushFwd = TRUE;
		return TRUE;
	}

	bPushActive = FALSE;
	return FALSE;
}

UBOOL AOLPushableObject::TryPushBack()
{
	if (CanPushBack())
	{
		if (!bPushActive)
		{
			SetSwitch(SwitchPushableType, PushableType == PM_Wood ? SwitchPushableTypeWood : SwitchPushableTypeMetal);
			PostAkEvent(SndStartPushing);
			SetRTPCValue(RTPCPushingSpeed, 100.0f);
		}

		bPushActive = TRUE;
		bPushFwd = FALSE;
		return TRUE;
	}

	bPushActive = FALSE;
	return FALSE;
}

void AOLPushableObject::StopMoving()
{
	if (bPushActive)
	{
		PostAkEvent(SndStopPushing);
		SetRTPCValue(RTPCPushingSpeed, 100.0f);
	}

	bPushActive = FALSE;
}

UBOOL AOLPushableObject::CanPush() const
{
	return (!bNetLocked && bEnabled && (!LinkedDoor || (LinkedDoor->IsClosed() && !LinkedDoor->IsBroken() && (LinkedDoor->DoorUser == NULL || !LinkedDoor->bAITraversing))));
}

UBOOL AOLPushableObject::CanPushFwd() const
{
	return (CanPush() && bCanPushFwd && !appIsNearlyEqual(CurrentDisplacement, MaxFwdDist));
}

UBOOL AOLPushableObject::CanPushBack() const
{
	return (CanPush() && bCanPushBack && !appIsNearlyEqual(CurrentDisplacement, -MaxBackDist));
}

void AOLPushableObject::UpdateLinkedDoor()
{
	if (LinkedDoor)
	{
		if (LinkedDoor->IsClosed())
		{
			FVector backEdge = GetCurrentBackEdge();
			FVector fwdEdge = GetCurrentFwdEdge();
			FVector fwdDir = (fwdEdge - backEdge).SafeNormal2D();

			TWEAKABLE FLOAT Buffer = 5.0f;
			FVector effectiveBackEdge = backEdge - Buffer*fwdDir;
			FVector effectiveFwdEdge = fwdEdge + Buffer*fwdDir;

			FVector doorPivot = LinkedDoor->GetPivotLocation();
			FVector doorEdge = LinkedDoor->GetEdgeLocation();
			FVector pivotToEdge = (doorEdge - doorPivot).SafeNormal2D();

			UBOOL allClear = FALSE;

			if ((pivotToEdge | fwdDir) > 0.0f)
			{
				allClear = (((doorPivot - fwdEdge) | fwdDir) > 0.0f) || (((doorEdge - backEdge) | fwdDir) < 0.0f);
			}
			else
			{
				allClear = (((doorPivot - backEdge) | fwdDir) < 0.0f) || (((doorEdge - fwdEdge) | fwdDir) > 0.0f);
			}

			if (LinkedDoor->bBlocked == allClear)
			{
				LinkedDoor->bBlocked = !allClear;

				TriggerEvent(LinkedDoor->bBlocked ? PET_BlockedDoor : PET_UnblockedDoor, bPlayerLocked ? Utils::GetHero() : NULL);
			}
		}
		else
		{
			LinkedDoor->bBlocked = FALSE;
		}
	}
}

void AOLPushableObject::TriggerEvent(PushableEventType eventType, AOLPawn* instigator)
{
	for (INT i = 0; i < GeneratedEvents.Num(); i++)
	{
		UOLSeqEvent_Pushable* evt = Cast<UOLSeqEvent_Pushable>(GeneratedEvents(i));

		if (evt)
		{
			TArray<INT> indices;
			indices.AddItem(eventType);
			evt->CheckActivate(this, instigator, FALSE, &indices);
		}
	}
}

////////////////////////////////////////////////////////////////////////////////////////////
// Queries
////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////

FVector AOLPushableObject::GetMinBackEdge() const
{
	return Location - MaxBackDist*Rotation.Vector();
}

FVector AOLPushableObject::GetMaxFwdEdge() const
{
	return Location + (MaxFwdDist + Width)*Rotation.Vector();
}

FVector AOLPushableObject::GetCurrentBackEdge() const
{
	return Location + CurrentDisplacement*Rotation.Vector();
}

FVector AOLPushableObject::GetCurrentFwdEdge() const
{
	return Location + (CurrentDisplacement + Width)*Rotation.Vector();
}

FVector AOLPushableObject::GetFwdDirection() const
{
	return Rotation.Vector();
}

FVector AOLPushableObject::GetBackDirection() const
{
	return -Rotation.Vector();
}

FVector AOLPushableObject::GetSideDirection() const
{
	return Rotation.Right();
}

////////////////////////////////////////////////////////////////////////////////////////////
// Path Obstacle
////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////

UBOOL AOLPushableObject::GetBoundingShape(TArray<FVector>& out_PolyShape,INT ShapeIdx)
{
	static const FLOAT ExtraSpacing = 25.0f;

	FBoxSphereBounds& MeshBounds = Mesh->StaticMesh->Bounds;

	FVector newVert = Mesh->LocalToWorld.TransformFVector(MeshBounds.Origin + FVector(MeshBounds.BoxExtent.X + ExtraSpacing, MeshBounds.BoxExtent.Y + ExtraSpacing, -MeshBounds.BoxExtent.Z));
	out_PolyShape.AddItem(newVert);
	newVert = Mesh->LocalToWorld.TransformFVector(MeshBounds.Origin + FVector(-(MeshBounds.BoxExtent.X + ExtraSpacing), MeshBounds.BoxExtent.Y + ExtraSpacing, -MeshBounds.BoxExtent.Z));
	out_PolyShape.AddItem(newVert);
	newVert = Mesh->LocalToWorld.TransformFVector(MeshBounds.Origin + FVector(-(MeshBounds.BoxExtent.X + ExtraSpacing), -(MeshBounds.BoxExtent.Y + ExtraSpacing), -MeshBounds.BoxExtent.Z));
	out_PolyShape.AddItem(newVert);
	newVert = Mesh->LocalToWorld.TransformFVector(MeshBounds.Origin + FVector(MeshBounds.BoxExtent.X + ExtraSpacing, -(MeshBounds.BoxExtent.Y + ExtraSpacing), -MeshBounds.BoxExtent.Z));
	out_PolyShape.AddItem(newVert);

	return TRUE;
}

void AOLPushableObject::RegisterObstacle()
{
	RegisterObstacleWithNavMesh();
}

void AOLPushableObject::UnRegisterObstacle()
{
	UnregisterObstacleWithNavMesh();
}