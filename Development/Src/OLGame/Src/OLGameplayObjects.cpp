#include "OLGame.h"
#include "DebugRenderSceneProxy.h"

IMPLEMENT_CLASS(AOLGameplayMarker);
IMPLEMENT_CLASS(AOLLedgeMarker);
IMPLEMENT_CLASS(AOLHidingSpot);
IMPLEMENT_CLASS(AOLLadderMarker);
IMPLEMENT_CLASS(AOLBed);
IMPLEMENT_CLASS(AOLGameplayVolume);
IMPLEMENT_CLASS(AOLSqueezeVolume);
IMPLEMENT_CLASS(AOLAIVisionObstructionVolume);
IMPLEMENT_CLASS(AOLElectrifiedWaterVolume);
IMPLEMENT_CLASS(AOLElectrifiedVolume);
IMPLEMENT_CLASS(AOLDarknessVolume);
IMPLEMENT_CLASS(AOLHeatVolume);
IMPLEMENT_CLASS(AOLCSA);
IMPLEMENT_CLASS(AOLCameraActor);
IMPLEMENT_CLASS(AOLAIVaultMarker);
IMPLEMENT_CLASS(AOLCornerMarker);
IMPLEMENT_CLASS(UOLWaitPointComponent);
IMPLEMENT_CLASS(UOLPhysicalMaterialProperty);
IMPLEMENT_CLASS(AOLFloorMaterialVolume);
IMPLEMENT_CLASS(AOLHeatMarker);
IMPLEMENT_CLASS(AOLRecordingMarker);
IMPLEMENT_CLASS(AOLPreferredPathMarker);

////////////////////////////////////////////////////////////////////////////////////////////
// OLLedgeMarker
////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////

FMatrix AOLLedgeMarker::GetLocalToWorldForLedge()
{
	if (Next == NULL)
	{
		return FMatrix::Identity;
	}

	FVector Middle = 0.5f * (Location + Next->Location);
	FVector Dir = (Middle - Location).SafeNormal2D();
	FVector DirOne = FVector(Dir.Y, -Dir.X, 0.f);

	return FRotationTranslationMatrix(DirOne.Rotation(), Middle);
}

void AOLLedgeMarker::PostLoad()
{
	Super::PostLoad();

	if (AICanVault && Next)
	{
		if (WaitPointComponent == NULL)
		{
			WaitPointComponent = ConstructObject<UOLWaitPointComponent>( UOLWaitPointComponent::StaticClass(), this );
			WaitPointComponent->StartOffset = WaitPointStartOffset;
			WaitPointComponent->AdditionalOffset = WaitPointAdditionalOffset;

			Components.AddItem(WaitPointComponent);
		}

		WaitPointComponent->SetLocalToWorld(GetLocalToWorldForLedge());

		if (WaitPointComponent->WaitPoints.Num() == 0)
		{
			WaitPointComponent->GenerateWaitPoints(FALSE);
		}
	}
}

void AOLLedgeMarker::PostEditMove(UBOOL bFinished)
{
	Super::PostEditMove(bFinished);

	if (WaitPointComponent != NULL && bFinished)
	{
		WaitPointComponent->SetLocalToWorld(GetLocalToWorldForLedge());
	}
}

void AOLLedgeMarker::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	if (AICanVault && Next)
	{
		if (WaitPointComponent == NULL)
		{
			WaitPointComponent = ConstructObject<UOLWaitPointComponent>( UOLWaitPointComponent::StaticClass(), this );
			WaitPointComponent->StartOffset = WaitPointStartOffset;
			WaitPointComponent->AdditionalOffset = WaitPointAdditionalOffset;

			Components.AddItem(WaitPointComponent);
		}

		WaitPointComponent->SetLocalToWorld(LocalToWorld());

		if (WaitPointComponent->WaitPoints.Num() == 0)
		{
			WaitPointComponent->GenerateWaitPoints(FALSE);
		}
	}
}

UBOOL AOLLedgeMarker::Tick( FLOAT DeltaSeconds, ELevelTick TickType )
{
	if (TickType == LEVELTICK_ViewportsOnly)
	{
		return Super::Tick(DeltaSeconds, TickType);
	}

	if (Utils::GetCheatManager() && Utils::GetCheatManager()->bDebugWaitPoints && WaitPointComponent != NULL)
	{
		WaitPointComponent->DrawDebugPoints();
	}

	return Super::Tick(DeltaSeconds, TickType);
}

void AOLLedgeMarker::PostSubMeshUpdateForOwningPoly(FNavMeshPathObjectEdge* Edge, FNavMeshPolyBase* Poly, UNavigationMeshBase* New_SubMesh)
{
	FVector StartVert = Edge->GetVertLocation(0);

	FVector EndVert;

	if (StartVert == TopPoint)
	{
		EndVert = BottomPoint;
	}
	else
	{
		EndVert = TopPoint;
	}

	FNavMeshPolyBase *StartPoly=NULL, *EndPoly=NULL;

	APylon* Py =NULL;
	UNavigationHandle::GetPylonAndPolyFromPos(StartVert,AScout::GetGameSpecificDefaultScoutObject()->WalkableFloorZ,Py,StartPoly);
	UNavigationHandle::GetPylonAndPolyFromPos(EndVert,AScout::GetGameSpecificDefaultScoutObject()->WalkableFloorZ,Py,EndPoly);

	if (StartPoly == NULL)
	{
		StartVert = EndVert + (StartVert - EndVert) * 1.25f;
		UNavigationHandle::GetPylonAndPolyFromPos(StartVert,AScout::GetGameSpecificDefaultScoutObject()->WalkableFloorZ,Py,StartPoly);
	}
	else if (EndPoly == NULL)
	{
		EndVert = StartVert + (EndVert - StartVert) * 1.25f;
		UNavigationHandle::GetPylonAndPolyFromPos(EndVert,AScout::GetGameSpecificDefaultScoutObject()->WalkableFloorZ,Py,EndPoly);
	}

	if( StartPoly != NULL && EndPoly != NULL && StartPoly != EndPoly )
	{
		static TArray<FNavMeshPolyBase*> ConnectedPolys;
		ConnectedPolys.Reset(2);
		ConnectedPolys.AddItem(StartPoly);
		ConnectedPolys.AddItem(EndPoly);

		static TArray<FNavMeshPathObjectEdge*> ReturnEdges;
		ReturnEdges.Reset(ReturnEdges.Num());

		StartPoly->NavMesh->AddDynamicCrossPylonEdge<FNavMeshPathObjectEdge>(StartVert, StartVert, ConnectedPolys, Edge->EffectiveEdgeLength, Edge->EdgeGroupID, TRUE, &ReturnEdges);

		FNavMeshPathObjectEdge* NewEdge = NULL;
		if( ReturnEdges.Num() > 0)
		{
			NewEdge = ReturnEdges(0);
		}

		if(NewEdge != NULL)
		{
			NewEdge->PathObject = Edge->PathObject;
			NewEdge->InternalPathObjectID = Edge->InternalPathObjectID;
			NewEdge->EffectiveEdgeLength = Edge->EffectiveEdgeLength;
			NewEdge->EdgeGroupID = Edge->EdgeGroupID;

			StartPoly->NavMesh->SetNeedsRecompute(TRUE);
		}
	}
}

INT AOLLedgeMarker::CostFor( const FNavMeshPathParams& PathParams, const FVector& PreviousPoint, FVector& out_PathEdgePoint, FNavMeshPathObjectEdge* Edge, FNavMeshPolyBase* SourcePoly )
{
	out_PathEdgePoint = Edge->GetEdgeCenter();

	FVector OtherDest;
	if (out_PathEdgePoint == BottomPoint)
	{
		OtherDest = TopPoint;
	}
	else
	{
		OtherDest = BottomPoint;
	}

	FLOAT Multiplier = 1.2f;
	AOLBot* Bot = Cast<AOLBot>(PathParams.Interface->GetUObjectInterfaceInterface_NavigationHandle());
	if (Bot != NULL)
	{
		if (Bot->EnemyPawn->EnemyMode != EM_Chase)
		{
			Multiplier *= NonChaseCostMultiplier;
		}
	}

	return appTrunc((out_PathEdgePoint - PreviousPoint).Size() + (out_PathEdgePoint - OtherDest).Size() * Multiplier);
}

UBOOL AOLLedgeMarker::Supports( const FNavMeshPathParams& PathParams, FNavMeshPolyBase* CurPoly, FNavMeshPathObjectEdge* Edge, FNavMeshEdgeBase* PredecessorEdge)
{
	AOLBot* Bot = Cast<AOLBot>(PathParams.Interface->GetUObjectInterfaceInterface_NavigationHandle());
	if(Bot != NULL)
	{
		return TRUE;
	}

	return FALSE;
}

UBOOL AOLLedgeMarker::PrepareMoveThru( class IInterface_NavigationHandle* Interface, FVector& out_MovePt, FNavMeshPathObjectEdge* Edge )
{
	AOLBot* Bot = Cast<AOLBot>(Interface->GetUObjectInterfaceInterface_NavigationHandle());
	if(Bot != NULL)
	{
		UBOOL Chasing = (Bot->EnemyPawn->EnemyMode == EM_Chase);

		UBOOL bReversed = Edge->GetEdgeCenter() == TopPoint;
		FVector Destination = FVector(0.f);
		if (bReversed)
		{
			FVector ToBottom = BottomPoint - TopPoint;
			ToBottom.Z = 0.f;
			ToBottom.Normalize();

			Destination = TopPoint + ToBottom * (PathOffset - (Chasing ? Bot->EnemyPawn->ChasingDropDownDistance : Bot->EnemyPawn->NormalDropDownDistance));
		}
		else
		{
			FVector ToTop = TopPoint - BottomPoint;
			ToTop.Z = 0.f;
			ToTop.Normalize();

			Destination = BottomPoint + ToTop * (PathOffset - (Chasing ? Bot->EnemyPawn->ChasingClimbUpDistance : Bot->EnemyPawn->NormalClimbUpDistance));
		}

		UOLAICmd_MoveAbility_Ledge* Cmd = UOLAICmd_MoveAbility_Ledge::StaticClass()->GetDefaultObject<UOLAICmd_MoveAbility_Ledge>()->eventMoveThruLedge(Bot, this, Destination, bReversed);
		Bot->eventQueueAICommand(Cmd);
	}
	
	return TRUE;
}

UBOOL AOLLedgeMarker::GetEdgeDestination( const FNavMeshPathParams& PathParams, FLOAT EntityRadius, const FVector& InfluencePosition, const FVector& EntityPosition, FVector& out_EdgeDest,	FNavMeshPathObjectEdge* Edge, UNavigationHandle* Handle)
{
	out_EdgeDest = Edge->GetEdgeCenter();

	return TRUE;
}

UBOOL AOLLedgeMarker::GetFinalEdgeDestination( FVector& out_EdgeDest, FNavMeshPathObjectEdge* Edge )
{
	FVector Start = Edge->GetEdgeCenter();
	if (Start == TopPoint)
	{
		out_EdgeDest = BottomPoint;
	}
	else
	{
		out_EdgeDest = TopPoint;
	}

	return TRUE;
}

UBOOL AOLLedgeMarker::DrawEdge( FDebugRenderSceneProxy* DRSP, FColor C, FVector DrawOffset, FNavMeshPathObjectEdge* Edge )
{
	FVector Start = Edge->GetEdgeCenter();
	if (Start == TopPoint)
	{
		new(DRSP->ArrowLines) FDebugRenderSceneProxy::FArrowLine(Start,BottomPoint,FColor(255,0,128));
	}
	else
	{
		new(DRSP->ArrowLines) FDebugRenderSceneProxy::FArrowLine(Start,TopPoint,FColor(255,0,128));
	}

	return TRUE;
}

void AOLLedgeMarker::CreateEdgesForPathObject( APylon* Py )
{
	if (Next == NULL || !AICanVault)
		return;

	FVector Middle = (Location + Next->Location) / 2;

	FVector Dir = (Middle - Location).SafeNormal2D();

	FVector DirOne = FVector(Dir.Y, -Dir.X, 0.f);
	FVector DirTwo = FVector(-Dir.Y, Dir.X, 0.f);

	static float HeightOffset = 10.0f;
	FVector One = Middle + DirOne*PathOffset + FVector(0.f, 0.f, HeightOffset);
	FVector Two = Middle + DirTwo*PathOffset + FVector(0.f, 0.f, HeightOffset);

	AOLScout* Scout = Cast<AOLScout>(FPathBuilder::GetScout());
	if (Scout != NULL)
	{
		FVector HumanSize = Scout->GetSize(TEXT("Human"));
		if( Scout->FindSpot( FVector(HumanSize.X,HumanSize.X,HumanSize.Y), One ) && 
			Scout->FindSpot( FVector(HumanSize.X,HumanSize.X,HumanSize.Y), Two ) )
		{
			One.Z -= 2.0f*Scout->NavMeshGen_EntityHalfHeight;
			Two.Z -= 2.0f*Scout->NavMeshGen_EntityHalfHeight;

			FNavMeshPolyBase *StartPoly=NULL, *EndPoly=NULL;
			APylon *StartPylon=NULL, *EndPylon=NULL;

			FCheckResult Hit(1.0f);

			if(Py->FindGround(One, Hit, Scout))
			{
				One = Hit.Location;// + FVector(0.f, 0.f, HeightOffset);
				UNavigationHandle::GetPylonAndPolyFromPos(One, Scout->WalkableFloorZ, StartPylon, StartPoly);
			}
			else
			{
				GWorld->GetWorldInfo()->DrawDebugLine(One,One+FVector(0,0,50),255,0,255,TRUE);
			}

			if(Py->FindGround(Two, Hit, Scout))
			{
				Two = Hit.Location;// + FVector(0.f, 0.f, HeightOffset);
				UNavigationHandle::GetPylonAndPolyFromPos(Two, Scout->WalkableFloorZ, EndPylon, EndPoly);
			}
			else
			{
				GWorld->GetWorldInfo()->DrawDebugLine(Two,Two+FVector(0,0,50),255,0,255,TRUE);
			}


			if( StartPoly != NULL && EndPoly != NULL && 
				(StartPylon == Py || EndPylon == Py))
			{
				AddEdgeForThisPO(this, StartPylon, StartPoly, EndPoly, One, One, 0);
				AddEdgeForThisPO(this, EndPylon, EndPoly, StartPoly, Two, Two, 1);
			}

			if (One.Z > Two.Z)
			{
				TopPoint = One;
				BottomPoint = Two;
			}
			else
			{
				TopPoint = Two;
				BottomPoint = One;
			}
		}
	}
}

void AOLLedgeMarker::AddAuxSeedPoints( APylon* Py )
{
	if (Next == NULL || !AICanVault)
		return;

	FVector Middle = (Location + Next->Location) / 2;

	FVector Dir = (Middle - Location).SafeNormal2D();

	FVector DirOne = FVector(Dir.Y, -Dir.X, 0.f);
	FVector DirTwo = FVector(-Dir.Y, Dir.X, 0.f);

	static float HeightOffset = 10.0f;
	FVector One = Middle + DirOne*PathOffset + FVector(0.f, 0.f, HeightOffset);
	FVector Two = Middle + DirTwo*PathOffset + FVector(0.f, 0.f, HeightOffset);

	AOLScout* Scout = Cast<AOLScout>(FPathBuilder::GetScout());
	if (Scout != NULL)
	{
		FVector HumanSize = Scout->GetSize(TEXT("Human"));
		if( Scout->FindSpot( FVector(HumanSize.X,HumanSize.X,HumanSize.Y), One ) && 
			Scout->FindSpot( FVector(HumanSize.X,HumanSize.X,HumanSize.Y), Two ) )
		{
			One.Z -= 2.0f*Scout->NavMeshGen_EntityHalfHeight;
			Two.Z -= 2.0f*Scout->NavMeshGen_EntityHalfHeight;

			FNavMeshPolyBase *StartPoly=NULL, *EndPoly=NULL;
			APylon *StartPylon=NULL, *EndPylon=NULL;

			FCheckResult Hit(1.0f);
			if(Py->FindGround(One, Hit, Scout))
			{
				One = Hit.Location;// + FVector(0.f, 0.f, HeightOffset);
				UNavigationHandle::GetPylonAndPolyFromPos(One, Scout->WalkableFloorZ, StartPylon, StartPoly);
			}
			else
			{
				GWorld->GetWorldInfo()->DrawDebugLine(One,One+FVector(0,0,50),255,0,255,TRUE);
			}

			if(Py->FindGround(Two, Hit, Scout))
			{
				Two = Hit.Location;// + FVector(0.f, 0.f, HeightOffset);
				UNavigationHandle::GetPylonAndPolyFromPos(Two, Scout->WalkableFloorZ, EndPylon, EndPoly);
			}
			else
			{
				GWorld->GetWorldInfo()->DrawDebugLine(Two,Two+FVector(0,0,50),255,0,255,TRUE);
			}

			Py->NextPassSeedList.AddItem(One);
			Py->NextPassSeedList.AddItem(Two);
		}
	}
}

UBOOL AOLLedgeMarker::Verify()
{
	return IsValid();
}

//////////////////////////////////////////////////////////////////////////
// CSA
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void AOLCSA::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	PreviewComp->SphereRadius = InteractRadius;
	PreviewComp->ConditionalUpdateTransform();

	Super::PostEditChangeProperty(PropertyChangedEvent);
}

UBOOL AOLCSA::TryActivate(AOLHero* hero, UBOOL playerInteraction)
{
	if (!bEnabled)
	{
		return FALSE;
	}

	if (MaxTriggerCount > 0 && TriggerCount >= MaxTriggerCount)
	{
		return FALSE;
	}

	if (LastValidCheckpoint != NAME_None && Utils::IsCheckpointCompleted(LastValidCheckpoint))
	{
		// We've completed the associated checkpoint
		return FALSE;
	}

	if ((Location - hero->EyeLocation).SizeSquared2D() > Square(InteractDistHorz) || Abs(Location.Z - hero->EyeLocation.Z) > InteractDistVert)
	{
		// Too far or incorrect height
		return FALSE;
	}

	check(InteractRadius > 0.0f);
	if (PointDistToLine(Location, hero->EyeForward, hero->EyeLocation) > InteractRadius)
	{
		// Not looking within the interact radius
		return FALSE;
	}

	if (((Location - hero->EyeLocation) | hero->EyeForward) < 0.0f)
	{
		// looking backwards
		return FALSE;
	}

	if (MaxPlayerAngle > 0.0f)
	{
		// Check whether we're facing the right side of the CSA
		FVector csaRight = Rotation.Right();
		FVector interactRight2d = Vec2D(Location + InteractRadius*csaRight);
		FVector interactLeft2d = Vec2D(Location - InteractRadius*csaRight);
		FVector closestPoint(0.0f);

		// find the closest interaction point (allow a radius)
		PointDistToSegment(Vec2D(hero->EyeLocation), interactLeft2d, interactRight2d, closestPoint);
		
		FVector toCSADir = (closestPoint - hero->Location).SafeNormal2D();
		if ((toCSADir | -Rotation.Vector()) < appCos(DEG_TO_RAD * MaxPlayerAngle))
		{
			// Not oriented correctly relative to the CSA
			return FALSE;
		}
	}

	if (bCheckLOS)
	{
		FCheckResult Hit(1.f);
		FVector startTrace = hero->EyeLocation;
		FVector endTrace = Location + 10.0f*(hero->EyeLocation - Location).SafeNormal(); // 10 cm closer to the eye, to clear whatever collision we may be in

		// check that we have LoS to the CSA
		if (!GWorld->SingleLineCheck( Hit, this, endTrace, startTrace, TRACE_AllBlocking | TRACE_ComplexCollision | TRACE_StopAtAnyHit, FVector(0.0f)))
		{
			return FALSE;
		}
	}

	if (RequiredItem != NAME_None && (!hero->OLPC->InventoryManager || !hero->OLPC->InventoryManager->OwnsItem(RequiredItem)))
	{
		// player doesn't have the required item

		if (!bNoPrompt && RequiredItemPromptTextId != NAME_None)
		{
			FString text = Localize(TEXT("Messages"), *RequiredItemPromptTextId.ToString(), TEXT("OLGame"));
			hero->OLPC->HUD->AddMessage(EHMT_Objective, text, 0.1f);
		}

		return FALSE;
	}

	if (!playerInteraction && !bAutomatic)
	{
		if (!bNoPrompt)
		{
			hero->OLPC->AddAvailableInteraction(PIT_CSA);

			if (ActivationPromptTextId != NAME_None)
			{
				hero->OLPC->CSAPrompt = Localize(TEXT("Messages"), *ActivationPromptTextId.ToString(), TEXT("OLGame"));
			}
			else
			{
				hero->OLPC->CSAPrompt = FString("[TEMP] Press USE to interact");
			}
		}
		return FALSE;
	}	

	// In multiplayer the server grants the CSA first; TryCSA runs again once it does.
	extern UBOOL MpWorld_ClaimCSA(AOLCSA*);
	if (MpWorld_ClaimCSA(this))
	{
		return FALSE;
	}

	if (bConsumeItem && RequiredItem != NAME_None)
	{
		hero->OLPC->InventoryManager->ConsumeItem(RequiredItem);
	}

	TriggerCount++;

	return TRUE;
}

void AOLCSA::Completed(AOLHero* hero)
{
	for (INT i = 0; i < GeneratedEvents.Num(); i++)
	{
		UOLSeqEvent_CSAActivated* ev = Cast<UOLSeqEvent_CSAActivated>(GeneratedEvents(i));

		if (ev)
		{
			ev->CheckActivate(this, hero);
		}
	}
}

void AOLCSA::RemoteActivate(UBOOL bConsumeActivation)
{
	// bExcludeFromKismetPlayer on the PC (set by ApplyRemoteCSA) blocks PC binding in
	// InitInterp and CinematicMode in OnToggleCinematicMode for the full cutscene duration.
	if (bConsumeActivation)
		TriggerCount++;
	Completed(NULL);
}

void AOLCSA::Reset()
{
	Super::Reset();
	TriggerCount = 0;

	if (bShowPropAfterLastValidCheckpoint && AnimatedProp && AnimatedProp->StaticMeshComponent && LastValidCheckpoint != NAME_None)
	{
		if (Utils::IsCheckpointCompleted(LastValidCheckpoint))
		{
			AnimatedProp->StaticMeshComponent->SetHiddenGame(FALSE);
		}
		else
		{
			AnimatedProp->StaticMeshComponent->SetHiddenGame(TRUE); // Reset as hidden, as this is presumably an objective that the player must go through again
		}
	}
}

////////////////////////////////////////////////////////////////////////////////////////////
// OLCameraActor
////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////

void AOLCameraActor::NativeGetCameraView(FTPOV& outTPOV)
{
	outTPOV.Location = GetViewLocation();
	outTPOV.Rotation = GetViewRotation();

	outTPOV.FOV = FOVAngle;
}

FVector AOLCameraActor::GetViewLocation()
{
	AOLHero* hero = Utils::GetHero();
	return hero ? hero->EyeLocation : Location;
}

FRotator AOLCameraActor::GetViewRotation()
{
	AOLHero* hero = Utils::GetHero();
	return hero ? hero->EyeRotation : Rotation;
}

void AOLCameraActor::GetViewPointForMatineePreview(APawn* previewPawn, FVector& out_Loc, FRotator& out_Rot)
{
	if (previewPawn && previewPawn->Mesh)
	{
		const FName cameraBoneName = Utils::GetCameraBoneName();

		if (previewPawn->Mesh->MatchRefBone(cameraBoneName) != INDEX_NONE)
		{
			out_Loc = previewPawn->Mesh->GetBoneLocation(cameraBoneName);
			out_Rot = previewPawn->Mesh->GetBoneQuaternion(cameraBoneName).Rotator();

			return;
		}
	}

	return Super::GetViewPointForMatineePreview(previewPawn, out_Loc, out_Rot);
}

////////////////////////////////////////////////////////////////////////////////////////////
// OLAIVaultMarker
////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////

void AOLAIVaultMarker::PostLoad()
{
	Super::PostLoad();

	if (WaitPointComponent != NULL)
	{
		WaitPointComponent->SetLocalToWorld(LocalToWorld());

		if (WaitPointComponent->WaitPoints.Num() == 0)
		{
			WaitPointComponent->GenerateWaitPoints(false);
		}
	}
}

void AOLAIVaultMarker::PostEditMove(UBOOL bFinished)
{
	Super::PostEditMove(bFinished);

	if (bFinished)
	{
		WaitPointComponent->SetLocalToWorld(LocalToWorld());
	}
}

UBOOL AOLAIVaultMarker::Tick(FLOAT deltaTime, ELevelTick tickType)
{
	if (tickType == LEVELTICK_ViewportsOnly)
	{
		return Super::Tick(deltaTime, tickType);
	}
	
	if (Utils::GetCheatManager() && Utils::GetCheatManager()->bDebugWaitPoints)
	{
		WaitPointComponent->DrawDebugPoints();
	}

	return Super::Tick(deltaTime, tickType);
}

void AOLAIVaultMarker::PostSubMeshUpdateForOwningPoly(FNavMeshPathObjectEdge* Edge, FNavMeshPolyBase* Poly, UNavigationMeshBase* New_SubMesh)
{
	FVector StartVert = Edge->GetVertLocation(0);

	FVector EndVert;

	if (Edge->InternalPathObjectID == 1)
	{
		EndVert = EndPointOne;
	}
	else
	{
		EndVert = EndPointTwo;
	}

	FNavMeshPolyBase *StartPoly=NULL, *EndPoly=NULL;

	APylon* Py =NULL;
	UNavigationHandle::GetPylonAndPolyFromPos(StartVert,AScout::GetGameSpecificDefaultScoutObject()->WalkableFloorZ,Py,StartPoly);
	UNavigationHandle::GetPylonAndPolyFromPos(EndVert,AScout::GetGameSpecificDefaultScoutObject()->WalkableFloorZ,Py,EndPoly);

	if (StartPoly == NULL)
	{
		StartVert = EndVert + (StartVert - EndVert) * 1.25f;
		UNavigationHandle::GetPylonAndPolyFromPos(StartVert,AScout::GetGameSpecificDefaultScoutObject()->WalkableFloorZ,Py,StartPoly);
	}
	else if (EndPoly == NULL)
	{
		EndVert = StartVert + (EndVert - StartVert) * 1.25f;
		UNavigationHandle::GetPylonAndPolyFromPos(EndVert,AScout::GetGameSpecificDefaultScoutObject()->WalkableFloorZ,Py,EndPoly);
	}

	if( StartPoly != NULL && EndPoly != NULL && StartPoly != EndPoly )
	{
		static TArray<FNavMeshPolyBase*> ConnectedPolys;
		ConnectedPolys.Reset(2);
		ConnectedPolys.AddItem(StartPoly);
		ConnectedPolys.AddItem(EndPoly);

		static TArray<FNavMeshPathObjectEdge*> ReturnEdges;
		ReturnEdges.Reset(ReturnEdges.Num());

		StartPoly->NavMesh->AddDynamicCrossPylonEdge<FNavMeshPathObjectEdge>(StartVert, StartVert, ConnectedPolys, Edge->EffectiveEdgeLength, Edge->EdgeGroupID, TRUE, &ReturnEdges);

		FNavMeshPathObjectEdge* NewEdge = NULL;
		if( ReturnEdges.Num() > 0)
		{
			NewEdge = ReturnEdges(0);
		}

		if(NewEdge != NULL)
		{
			NewEdge->PathObject = Edge->PathObject;
			NewEdge->InternalPathObjectID = Edge->InternalPathObjectID;
			NewEdge->EffectiveEdgeLength = Edge->EffectiveEdgeLength;
			NewEdge->EdgeGroupID = Edge->EdgeGroupID;

			StartPoly->NavMesh->SetNeedsRecompute(TRUE);
		}
	}
}

INT AOLAIVaultMarker::CostFor( const FNavMeshPathParams& PathParams, const FVector& PreviousPoint, FVector& out_PathEdgePoint, FNavMeshPathObjectEdge* Edge, FNavMeshPolyBase* SourcePoly )
{
	out_PathEdgePoint = Edge->GetEdgeCenter();
	
	FVector OtherDest;
	if (Edge->InternalPathObjectID == 1)
	{
		OtherDest = EndPointOne;
	}
	else
	{
		OtherDest = EndPointTwo;
	}

	FLOAT Multiplier = 1.2f;
	AOLBot* Bot = Cast<AOLBot>(PathParams.Interface->GetUObjectInterfaceInterface_NavigationHandle());
	if (Bot != NULL)
	{
		if (Bot->EnemyPawn->EnemyMode != EM_Chase)
		{
			Multiplier *= NonChaseCostMultiplier;
		}
	}

	return appTrunc((out_PathEdgePoint - PreviousPoint).Size() + (out_PathEdgePoint - OtherDest).Size() * Multiplier);
}

UBOOL AOLAIVaultMarker::Supports( const FNavMeshPathParams& PathParams, FNavMeshPolyBase* CurPoly, FNavMeshPathObjectEdge* Edge, FNavMeshEdgeBase* PredecessorEdge)
{
	AOLBot* Bot = Cast<AOLBot>(PathParams.Interface->GetUObjectInterfaceInterface_NavigationHandle());
	if(Bot != NULL && Bot->EnemyPawn != NULL && Bot->EnemyPawn->bCanVault)
	{
		return TRUE;
	}

	return FALSE;
}

UBOOL AOLAIVaultMarker::PrepareMoveThru( class IInterface_NavigationHandle* Interface, FVector& out_MovePt, FNavMeshPathObjectEdge* Edge )
{
	AOLBot* Bot = Cast<AOLBot>(Interface->GetUObjectInterfaceInterface_NavigationHandle());
	if(Bot != NULL)
	{
		UBOOL bReversed = Edge->InternalPathObjectID == 1;
		FVector Destination = FVector(0.f);
		if (bReversed)
		{
			Destination = EndPointTwo;
		}
		else
		{
			Destination = EndPointOne;
		}

		UOLAICmd_MoveAbility_Vault* Cmd = UOLAICmd_MoveAbility_Vault::StaticClass()->GetDefaultObject<UOLAICmd_MoveAbility_Vault>()->eventMoveThruLedge(Bot, this, Destination, bReversed);
		Bot->eventQueueAICommand(Cmd);
	}

	return TRUE;
}

UBOOL AOLAIVaultMarker::GetEdgeDestination( const FNavMeshPathParams& PathParams, FLOAT EntityRadius, const FVector& InfluencePosition, const FVector& EntityPosition, FVector& out_EdgeDest,	FNavMeshPathObjectEdge* Edge, UNavigationHandle* Handle)
{
	out_EdgeDest = Edge->GetEdgeCenter();

	return TRUE;
}

UBOOL AOLAIVaultMarker::GetFinalEdgeDestination( FVector& out_EdgeDest, FNavMeshPathObjectEdge* Edge )
{
	if (Edge->InternalPathObjectID == 1)
	{
		out_EdgeDest = EndPointOne;
	}
	else
	{
		out_EdgeDest = EndPointTwo;
	}

	return TRUE;
}

UBOOL AOLAIVaultMarker::DrawEdge( FDebugRenderSceneProxy* DRSP, FColor C, FVector DrawOffset, FNavMeshPathObjectEdge* Edge )
{
	FVector Start = Edge->GetEdgeCenter();
	if (Edge->InternalPathObjectID == 1)
	{
		new(DRSP->ArrowLines) FDebugRenderSceneProxy::FArrowLine(Start,EndPointOne,FColor(255,0,128));
	}
	else
	{
		new(DRSP->ArrowLines) FDebugRenderSceneProxy::FArrowLine(Start,EndPointTwo,FColor(255,0,128));
	}

	return TRUE;
}

void AOLAIVaultMarker::CreateEdgesForPathObject( APylon* Py )
{	
	FVector Dir = (Rotation.Vector()).SafeNormal2D();

	FVector DirOne = Dir;
	FVector DirTwo = -Dir;

	FVector One = Location + DirOne*(VaultWidth*0.5f);
	FVector Two = Location + DirTwo*(VaultWidth*0.5f);

	AOLScout* Scout = Cast<AOLScout>(FPathBuilder::GetScout());
	if (Scout != NULL)
	{
		FVector HumanSize = Scout->GetSize(TEXT("Human"));
		if( Scout->FindSpot( FVector(HumanSize.X,HumanSize.X,HumanSize.Y), One ) && 
			Scout->FindSpot( FVector(HumanSize.X,HumanSize.X,HumanSize.Y), Two ) )
		{
			One.Z -= 2.0f*Scout->NavMeshGen_EntityHalfHeight;
			Two.Z -= 2.0f*Scout->NavMeshGen_EntityHalfHeight;

			FNavMeshPolyBase *StartPoly=NULL, *EndPoly=NULL;
			APylon *StartPylon=NULL, *EndPylon=NULL;

			FCheckResult Hit(1.0f);

			if(Py->FindGround(One, Hit, Scout))
			{
				One = Hit.Location;// + FVector(0.f, 0.f, HeightOffset);
				UNavigationHandle::GetPylonAndPolyFromPos(One, Scout->WalkableFloorZ, StartPylon, StartPoly);
			}
			else
			{
				GWorld->GetWorldInfo()->DrawDebugLine(One,One+FVector(0,0,50),255,0,255,TRUE);
			}

			if(Py->FindGround(Two, Hit, Scout))
			{
				Two = Hit.Location;// + FVector(0.f, 0.f, HeightOffset);
				UNavigationHandle::GetPylonAndPolyFromPos(Two, Scout->WalkableFloorZ, EndPylon, EndPoly);
			}
			else
			{
				GWorld->GetWorldInfo()->DrawDebugLine(Two,Two+FVector(0,0,50),255,0,255,TRUE);
			}


			if( StartPoly != NULL && EndPoly != NULL && 
				(StartPylon == Py || EndPylon == Py))
			{
				AddEdgeForThisPO(this, StartPylon, StartPoly, EndPoly, One, One, 0);
				AddEdgeForThisPO(this, EndPylon, EndPoly, StartPoly, Two, Two, 1);
			}

			EndPointOne = One;
			EndPointTwo = Two;
		}
	}
}

void AOLAIVaultMarker::AddAuxSeedPoints( APylon* Py )
{
	FVector Dir = (Rotation.Vector()).SafeNormal2D();

	FVector DirOne = Dir;
	FVector DirTwo = -Dir;

	FVector One = Location + DirOne*(VaultWidth*0.5f);
	FVector Two = Location + DirTwo*(VaultWidth*0.5f);

	AOLScout* Scout = Cast<AOLScout>(FPathBuilder::GetScout());
	if (Scout != NULL)
	{
		FVector HumanSize = Scout->GetSize(TEXT("Human"));
		if( Scout->FindSpot( FVector(HumanSize.X,HumanSize.X,HumanSize.Y), One ) && 
			Scout->FindSpot( FVector(HumanSize.X,HumanSize.X,HumanSize.Y), Two ) )
		{
			One.Z -= 2.0f*Scout->NavMeshGen_EntityHalfHeight;
			Two.Z -= 2.0f*Scout->NavMeshGen_EntityHalfHeight;

			FNavMeshPolyBase *StartPoly=NULL, *EndPoly=NULL;
			APylon *StartPylon=NULL, *EndPylon=NULL;

			FCheckResult Hit(1.0f);
			if(Py->FindGround(One, Hit, Scout))
			{
				One = Hit.Location;// + FVector(0.f, 0.f, HeightOffset);
				UNavigationHandle::GetPylonAndPolyFromPos(One, Scout->WalkableFloorZ, StartPylon, StartPoly);
			}
			else
			{
				GWorld->GetWorldInfo()->DrawDebugLine(One,One+FVector(0,0,50),255,0,255,TRUE);
			}

			if(Py->FindGround(Two, Hit, Scout))
			{
				Two = Hit.Location;// + FVector(0.f, 0.f, HeightOffset);
				UNavigationHandle::GetPylonAndPolyFromPos(Two, Scout->WalkableFloorZ, EndPylon, EndPoly);
			}
			else
			{
				GWorld->GetWorldInfo()->DrawDebugLine(Two,Two+FVector(0,0,50),255,0,255,TRUE);
			}

			Py->NextPassSeedList.AddItem(One);
			Py->NextPassSeedList.AddItem(Two);
		}
	}
}

UBOOL AOLAIVaultMarker::Verify()
{
	return IsValid();
}

////////////////////////////////////////////////////////////////////////////////////////////
// OLWaitPointComponent
////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////

void UOLWaitPointComponent::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	ForceUpdateAllPoints();
}

void UOLWaitPointComponent::SetLocalToWorld(const FMatrix& NewLocalToWorld)
{
	LocalToWorld = NewLocalToWorld;

	if (GWorld != NULL)
	{
		ForceUpdateAllPoints();
	}
}

void UOLWaitPointComponent::GenerateWaitPoints(UBOOL InvertY)
{
	WaitPoints.Reset();

	FVector AddOff = AdditionalOffset;
	FVector StartOff = StartOffset;

	if (InvertY)
	{
		StartOff.Y = -StartOff.Y;
		AddOff.Y = -AddOff.Y;
	}

	FVector LastOffset = StartOff;
	for (INT j = 0; j < NumWaitPointsPerSide; ++j)
	{
		WaitPoints.AddZeroed();
		WaitPoints(j).Offset = LastOffset;
		WaitPoints(j).bForReversed = FALSE;

		LastOffset += AddOff;
	}

	if (!bOneSideOnly)
	{
		LastOffset = StartOff;
		if (bFlipOnYAxis)
		{
			LastOffset.Y = -LastOffset.Y;
			AddOff.Y = -AddOff.Y;
		}
		else
		{
			LastOffset.X = -LastOffset.X;
			AddOff.X = -AddOff.X;
		}

		for (INT j = NumWaitPointsPerSide; j < NumWaitPointsPerSide*2; ++j)
		{
			WaitPoints.AddZeroed();
			WaitPoints(j).Offset = LastOffset;
			WaitPoints(j).bForReversed = TRUE;

			LastOffset += AddOff;
		}
	}

	ForceUpdateAllPoints();
}

void UOLWaitPointComponent::ForceUpdateAllPoints()
{
	for (INT i = 0; i < WaitPoints.Num(); ++i)
	{
		UpdateWaitPoint(i, TRUE);
	}
}

void UOLWaitPointComponent::UpdateWaitPoint(INT Idx, UBOOL bForce /*= FALSE*/)
{
	FWaitPoint& WPoint = WaitPoints(Idx);

	if (bForce || !WPoint.bIsValid)
	{
		WPoint.Point = LocalToWorld.TransformFVector(WPoint.Offset);

		FCheckResult Hit;
		if (GWorld != NULL && !GWorld->SingleLineCheck(Hit, NULL, WPoint.Point + FVector(0.0f, 0.0f, -300.0f), WPoint.Point + FVector(0.0f, 0.0f, 200.0f), TRACE_World ))
		{
			WPoint.Point = Hit.Location;
			WPoint.bIsValid = TRUE;
		}
	}
}

FVector UOLWaitPointComponent::GetWaitPointForwardVector(FWaitPoint Point)
{
	FVector Fwd = FVector(0.f);
	
	if (bFlipOnYAxis)
	{
		Fwd = LocalToWorld.TransformFVector4(FVector4(0.0f, Point.bForReversed ? 1.0f : -1.0f, 0.0f, 0.f));
	}
	else
	{
		Fwd = LocalToWorld.TransformFVector4(FVector4(Point.bForReversed ? 1.0f : -1.0f, 0.0f, 0.0f, 0.f));
	}

	Fwd = Fwd.RotateAngleAxis(Point.ForwardYaw * DEG_TO_UNR, FVector(0.f, 0.f, 1.f));

	return Fwd;
}

FWaitPoint UOLWaitPointComponent::GrabBestWaitPoint(UBOOL bReversed)
{
	FWaitPoint ReturnPoint(EC_EventParm);
	for (INT Idx = 0; Idx < WaitPoints.Num(); ++Idx)
	{
		if (bReversed == WaitPoints(Idx).bForReversed && !WaitPoints(Idx).bInUse)
		{
			UpdateWaitPoint(Idx);

			ReturnPoint = WaitPoints(Idx);
			WaitPoints(Idx).bInUse = TRUE;
			break;
		}
	}

	return ReturnPoint;
}

void UOLWaitPointComponent::ReturnWaitPoint(FWaitPoint Point)
{
	UBOOL bFound = FALSE;
	for (INT Idx = 0; Idx < WaitPoints.Num(); ++Idx)
	{
		if (WaitPoints(Idx).Point == Point.Point)
		{
			WaitPoints(Idx).bInUse = FALSE;
			bFound = TRUE;

			break;
		}
	}

	if (!bFound)
	{
		warnf(TEXT("WaitPoint returned that doesn't exist."));
	}
}

void UOLWaitPointComponent::DrawDebugPoints()
{
	for(INT i = 0; i < WaitPoints.Num(); ++i)
	{
		UpdateWaitPoint(i);
		Owner->DrawDebugCone(WaitPoints(i).Point, FVector(0.f, 0.f, 1.f), 5.0f, PI/4, PI/4, 8, WaitPoints(i).bInUse ? FColor(255, 0, 0) : FColor(0,255,0), FALSE);
	}
}

void UOLWaitPointComponent::DrawDebugPointsEditor(FPrimitiveDrawInterface* PDI)
{
	for(INT i = 0; i < WaitPoints.Num(); ++i)
	{
		UpdateWaitPoint(i);
		PDI->DrawPoint(WaitPoints(i).Point, FColor(0,255,0), 10.0f, SDPG_UnrealEdForeground);
		PDI->DrawLine(WaitPoints(i).Point, WaitPoints(i).Point + 15.0f * GetWaitPointForwardVector(WaitPoints(i)), FColor(0, 255, 0), SDPG_UnrealEdForeground, 1.0f);
	}
}

////////////////////////////////////////////////////////////////////////////////////////////
// AOLHeatMarker
////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////

void AOLHeatMarker::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	PreviewComp->SphereRadius = Radius;
	PreviewComp->ConditionalUpdateTransform();

	Super::PostEditChangeProperty(PropertyChangedEvent);
}

////////////////////////////////////////////////////////////////////////////////////////////
// OLRecordingMarker
////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////

UBOOL AOLRecordingMarker::IsValid() const 
{ 
	AOLPlayerController* OLPC = Utils::GetOLPC();
	return bEnabled && MomentName != NAME_None && !bRecorded && (!OLPC || !OLPC->CompletedRecordingMoments.ContainsItem(MomentName));
}

void AOLRecordingMarker::RecordingComplete()
{
	bRecorded = TRUE;

	for (INT i = 0; i < GeneratedEvents.Num(); i++)
	{
		UOLSeqEvent_RecordingComplete* ev = Cast<UOLSeqEvent_RecordingComplete>(GeneratedEvents(i));

		if (ev)
		{			
			ev->CheckActivate(this, Utils::GetHero());
		}
	}
}

void AOLRecordingMarker::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	PreviewComp->SphereRadius = Radius;
	PreviewComp->ConditionalUpdateTransform();

	Super::PostEditChangeProperty(PropertyChangedEvent);
}

////////////////////////////////////////////////////////////////////////////////////////////
// OLHidingSpot
////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////

void AOLHidingSpot::PostLoad()
{
	Super::PostLoad();

	if (WaitPointComponent != NULL)
	{
		WaitPointComponent->SetLocalToWorld(LocalToWorld());

		if (WaitPointComponent->WaitPoints.Num() == 0)
		{
			WaitPointComponent->GenerateWaitPoints(FALSE);
		}
	}
}

void AOLHidingSpot::PostEditMove(UBOOL bFinished)
{
	Super::PostEditMove(bFinished);

	if (bFinished && WaitPointComponent != NULL)
	{
		WaitPointComponent->SetLocalToWorld(LocalToWorld());
	}
}

void AOLHidingSpot::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	if (WaitPointComponent != NULL)
	{
		WaitPointComponent->SetLocalToWorld(LocalToWorld());
	}
}

UBOOL AOLHidingSpot::Tick(FLOAT deltaTime, ELevelTick tickType)
{
	if (Utils::GetCheatManager() && Utils::GetCheatManager()->bDebugWaitPoints)
	{
		WaitPointComponent->DrawDebugPoints();
	}

	return Super::Tick(deltaTime, tickType);
}

////////////////////////////////////////////////////////////////////////////////////////////
// OLBed
////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////

void AOLBed::PostLoad()
{
	Super::PostLoad();

	if (WaitPointComponent != NULL)
	{
		WaitPointComponent->SetLocalToWorld(LocalToWorld());

		if (WaitPointComponent->WaitPoints.Num() == 0)
		{
			WaitPointComponent->GenerateWaitPoints(FALSE);
		}
	}
}

void AOLBed::PostEditMove(UBOOL bFinished)
{
	Super::PostEditMove(bFinished);

	if (bFinished && WaitPointComponent != NULL)
	{
		WaitPointComponent->SetLocalToWorld(LocalToWorld());
	}
}

void AOLBed::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	if (WaitPointComponent != NULL)
	{
		WaitPointComponent->SetLocalToWorld(LocalToWorld());
	}
}

UBOOL AOLBed::Tick(FLOAT deltaTime, ELevelTick tickType)
{
	if (Utils::GetCheatManager() && Utils::GetCheatManager()->bDebugWaitPoints)
	{
		WaitPointComponent->DrawDebugPoints();
	}

	return Super::Tick(deltaTime, tickType);
}