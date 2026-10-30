/*=============================================================================
	OLBehaviorTree.cpp: 
	Copyright 2012 Red Barrels, Inc. All Rights Reserved.
=============================================================================*/

#include "OLGame.h"
#include "AkAudioDevice.h"

extern UBOOL GImprovedAI;

IMPLEMENT_CLASS(UOLBTBehavior);
IMPLEMENT_CLASS(UOLBTBehaviorTree);

// Nodes
IMPLEMENT_CLASS(UOLBTNode);
IMPLEMENT_CLASS(UOLBTBasicNode);
IMPLEMENT_CLASS(UOLBTDebugLogNode);
IMPLEMENT_CLASS(UOLBTWaitNode);
IMPLEMENT_CLASS(UOLBTPlayAnimNode);
IMPLEMENT_CLASS(UOLBTPlayAnimAtInvPointNode);
IMPLEMENT_CLASS(UOLBTAttackNode);
IMPLEMENT_CLASS(UOLBTSetBehaviorStateNode);
IMPLEMENT_CLASS(UOLBTPlayAkEventNode);
IMPLEMENT_CLASS(UOLBTStopPatrolNode);
IMPLEMENT_CLASS(UOLBTActivateEventNode);
IMPLEMENT_CLASS(UOLBTTargetUnreachableNode);

// Composite Nodes
IMPLEMENT_CLASS(UOLBTCompositeNode);
IMPLEMENT_CLASS(UOLBTIfNode);
IMPLEMENT_CLASS(UOLBTIfCustomNode);
IMPLEMENT_CLASS(UOLBTRepeatNode);
IMPLEMENT_CLASS(UOLBTRootNode);
IMPLEMENT_CLASS(UOLBTOptionalNode);
IMPLEMENT_CLASS(UOLBTSequenceNode);
IMPLEMENT_CLASS(UOLBTSelectorNode);
IMPLEMENT_CLASS(UOLBTGetNextInvestigationPointNode);
IMPLEMENT_CLASS(UOLBTWaitHandlerNode);

// Tasks
IMPLEMENT_CLASS(UOLBTTask);

// Basic Tasks
IMPLEMENT_CLASS(UOLBTBasicTask);
IMPLEMENT_CLASS(UOLBTMoveToTask);
IMPLEMENT_CLASS(UOLBTMoveToWaypointTask);
IMPLEMENT_CLASS(UOLBTMoveToTargetTask);
IMPLEMENT_CLASS(UOLBTMoveToDisturbanceTask);
IMPLEMENT_CLASS(UOLBTCancelMoveTask);
IMPLEMENT_CLASS(UOLBTClearDisturbanceTask);
IMPLEMENT_CLASS(UOLBTPerformWaypointActionTask);
IMPLEMENT_CLASS(UOLBTSelectNextWaypointTask);
IMPLEMENT_CLASS(UOLBTWaitForAnimTask);
IMPLEMENT_CLASS(UOLBTSetupInvestigationTask);
IMPLEMENT_CLASS(UOLBTMoveToNextInvPointTask);
IMPLEMENT_CLASS(UOLBTWaitForAttackTask);
IMPLEMENT_CLASS(UOLBTSelectClosestWaypointTask);
IMPLEMENT_CLASS(UOLBTMoveToSqueezeTask);
IMPLEMENT_CLASS(UOLBTClearSqueezeTask);
IMPLEMENT_CLASS(UOLBTMoveToBedTask);
IMPLEMENT_CLASS(UOLBTMoveToInvestigationTask);
IMPLEMENT_CLASS(UOLBTInvestigateObjectTask);
IMPLEMENT_CLASS(UOLBTMoveToLockerTask);
IMPLEMENT_CLASS(UOLBTClearInvestigationTask);
IMPLEMENT_CLASS(UOLBTMoveToWaitPointTask);
IMPLEMENT_CLASS(UOLBTRotateAtWaitPointTask);
IMPLEMENT_CLASS(UOLBTResetNewDisturbanceTask);
IMPLEMENT_CLASS(UOLBTLookAtDisturbanceTask);
IMPLEMENT_CLASS(UOLBTWaitForReactionTask);
IMPLEMENT_CLASS(UOLBTTargetUnreachableTask);

// Composite Tasks
IMPLEMENT_CLASS(UOLBTCompositeTask);
IMPLEMENT_CLASS(UOLBTSequenceTask);
IMPLEMENT_CLASS(UOLBTSelectorTask);
IMPLEMENT_CLASS(UOLBTIfTask);
IMPLEMENT_CLASS(UOLBTRepeatTask);
IMPLEMENT_CLASS(UOLBTOptionalTask);
IMPLEMENT_CLASS(UOLBTRootTask);
IMPLEMENT_CLASS(UOLBTGetNextInvestigationPointTask);
IMPLEMENT_CLASS(UOLBTWaitHandlerBaseTask);
IMPLEMENT_CLASS(UOLBTWaitHandlerBedTask);
IMPLEMENT_CLASS(UOLBTWaitHandlerLockerTask);

// Special Tasks
IMPLEMENT_CLASS(UOLBTDebugLogTask);
IMPLEMENT_CLASS(UOLBTWaitTask);
IMPLEMENT_CLASS(UOLBTPlayAnimTask);
IMPLEMENT_CLASS(UOLBTPlayAnimAtInvPointTask);
IMPLEMENT_CLASS(UOLBTAttackTask);
IMPLEMENT_CLASS(UOLBTSetBehaviorStateTask);
IMPLEMENT_CLASS(UOLBTPlayAkEventTask);
IMPLEMENT_CLASS(UOLBTStopPatrolTask);
IMPLEMENT_CLASS(UOLBTActivateEventTask);

// Other
IMPLEMENT_CLASS(UOLSeqEvent_BehaviorEvent);

/*============================================================================*/

UBOOL UOLBTNode::bNodeSearching = FALSE;
INT UOLBTNode::CurrentSearchTag = 0;

/*============================================================================*/

void UOLBTBehavior::Setup(UOLBTNode* aNode, AOLBot* aBot)
{
	Teardown();

	if (aNode != NULL)
	{
		Node = aNode;
		Task = aNode->CreateTask();
		Task->Owner = aBot;

		CurrentStatus = Status_Invalid;
	}
	else
	{
		AI_LOG(aBot, (TEXT("BT Log: Missing Child Node")));

		CurrentStatus = Status_Failure;
	}
}

void UOLBTBehavior::Teardown()
{
	if(Task == NULL)
	{
		return;
	}

	//ASSERT(CurrentStatus != Status_Running);
	if (CurrentStatus == Status_Running)
	{
		Task->OnTerminate(Status_Running);
	}

	Node->DestroyTask(Task);
	Node = NULL;
	Task = NULL;
}

EStatus UOLBTBehavior::Tick( FLOAT DeltaSeconds, TArray<FBehaviorState>* RootState /*= NULL*/, TArray<FBehaviorState>* CurrentState /*= NULL*/ )
{
	if (Task != NULL)
	{
		if (CurrentState != NULL && !Task->IsA(UOLBTCompositeTask::StaticClass()))
		{
			Task->AddTaskState(*CurrentState);
		}

		if (CurrentState != NULL && CompareTreeStates(RootState, CurrentState) && Task->IsLatentTask())
		{
			CurrentStatus = Status_ReachedCurrent;
		}
		else
		{
			if(CurrentStatus == Status_Invalid)
			{
				Task->OnInitialize();
			}

			CurrentStatus = Task->Update(DeltaSeconds, RootState, CurrentState);

			if(CurrentStatus != Status_Running)
			{
				Task->OnTerminate((EStatus)CurrentStatus);
			}

			if (CurrentState != NULL && !Task->IsA(UOLBTCompositeTask::StaticClass()))
			{
				CurrentState->Pop();
			}
		}
	}

	return (EStatus)CurrentStatus;
}

void UOLBTBehavior::GetTreeState(TArray<FBehaviorState>& CurrentState)
{
	if (Task != NULL)
	{
		Task->GetTreeState(CurrentState);
	}
}

//static
UBOOL UOLBTBehavior::CompareTreeStates(const TArray<FBehaviorState>* TreeA, const TArray<FBehaviorState>* TreeB)
{
	check(TreeA && TreeB);

	if (TreeA->Num() < TreeB->Num())
	{
		return FALSE;
	}

	for (INT I = 0; I < TreeA->Num(); ++I)
	{
		if (I >= TreeB->Num())
		{
			break;
		}

		FBehaviorState StateA = (*TreeA)(I);
		FBehaviorState StateB = (*TreeB)(I);

		if (!StateA.Task->CompareTaskState(StateB.Task))
		{
			return FALSE;
		}
	}

	return TRUE;
}

/*============================================================================*/

UOLBTTask* UOLBTNode::CreateTask()
{
	check (TaskType != NULL);

	UOLBTTask* newTask = ConstructObject<UOLBTTask>(TaskType);
	newTask->Node = this;
	return newTask;
}

// Immediately destroy a UObject without waiting for a GC pass.
// UObject::~UObject() handles GObjObjects/GObjAvailable cleanup itself.
// GIsPurgingObject lets the private operator delete proceed (same as GC does).
static void DestroyNow(UObject* Obj)
{
	if (!Obj)
		return;
	Obj->ConditionalBeginDestroy();
	while (!Obj->IsReadyForFinishDestroy())
		appSleep(0);
	Obj->ConditionalFinishDestroy();
	GIsPurgingObject = TRUE;
	delete Obj;
	GIsPurgingObject = FALSE;
}

void UOLBTNode::DestroyTask(UOLBTTask* aTask)
{
	DestroyNow(aTask);
}

void UOLBTNode::GetNodes(TArray<UOLBTNode*>& Nodes)
{
	check(!UOLBTNode::bNodeSearching);

	UOLBTNode::bNodeSearching = TRUE;

	UOLBTNode::CurrentSearchTag++;
	GetNodesInternal(Nodes);

	UOLBTNode::bNodeSearching = FALSE;
}

void UOLBTNode::GetNodesInternal(TArray<UOLBTNode*>& Nodes)
{
	check(UOLBTNode::bNodeSearching);

	if (SearchTag != UOLBTNode::CurrentSearchTag)
	{
		SearchTag = UOLBTNode::CurrentSearchTag;
		Nodes.AddItem(this);
	}
}

/*============================================================================*/

UOLBTTask* UOLBTBasicNode::CreateTask()
{
	check (BasicTask != NULL);

	UOLBTTask* newTask = ConstructObject<UOLBTTask>(BasicTask);
	newTask->Node = this;
	return newTask;
}

/*============================================================================*/

void UOLBTCompositeNode::GetNodesInternal(TArray<UOLBTNode*>& Nodes)
{
	checkSlow(UOLBTNode::bNodeSearching);

	if (SearchTag != UOLBTNode::CurrentSearchTag)
	{
		SearchTag = UOLBTNode::CurrentSearchTag;
		Nodes.AddItem(this);
		for (INT i = 0; i < Children.Num(); i++)
		{
			if (Children(i).Node != NULL)
			{
				Children(i).Node->GetNodesInternal(Nodes);
			}
		}
	}
}

void UOLBTCompositeNode::RenameChildConnectors()
{
	for(INT i=0; i<Children.Num(); i++)
	{
		FName OldFName = Children(i).Name;
		FString OldStringName = Children(i).Name.ToString();
		//if it contains "child" as the first string, it more than likely isn't custom named.
		if ((OldStringName.InStr("Child")==0) || (OldFName == NAME_None))
		{
			FString NewChildName = FString::Printf( TEXT("Child%d"), i + 1 );
			Children(i).Name = FName( *NewChildName );
		}
	}
}

void UOLBTCompositeNode::OnAddChild(INT ChildNum)
{
	RenameChildConnectors();
}

void UOLBTCompositeNode::OnRemoveChild(INT ChildNum)
{
	RenameChildConnectors();
}

/*============================================================================*/

void UOLBTTask::OnTerminate(EStatus status)
{
	if (Owner->bDebugBehaviorTransitions)
	{
		AI_LOG(Owner, (TEXT("BT Log: %s - %s"), *GetName(), *Utils::GetEnumString("EStatus", status)));
	}
}

void UOLBTTask::AddTaskState(TArray<FBehaviorState>& CurrentState)
{
	INT Idx = CurrentState.AddZeroed();
	CurrentState(Idx).Task = this;
}

void UOLBTTask::GetTreeState(TArray<FBehaviorState>& CurrentState)
{
	AddTaskState(CurrentState);
}

/*============================================================================*/

UOLBTCompositeNode& UOLBTCompositeTask::GetCompositeNode()
{
	return *CastChecked<UOLBTCompositeNode>(Node);
}

UOLBTNode* UOLBTCompositeTask::GetCurrentChild()
{
	return GetCompositeNode().Children(CurrentChildIndex).Node;
}

void UOLBTCompositeTask::OnInitialize()
{
	CurrentChildIndex = 0;
	CurrentBehavior = ConstructObject<UOLBTBehavior>(UOLBTBehavior::StaticClass());
	CurrentBehavior->Setup(GetCurrentChild(), Owner);
}

void UOLBTCompositeTask::OnTerminate(EStatus status)
{
	if (CurrentBehavior != NULL)
	{
		CurrentBehavior->Teardown();
		DestroyNow(CurrentBehavior);
		CurrentBehavior = NULL;
	}

	Super::OnTerminate(status);
}

UBOOL UOLBTCompositeTask::CompareTaskState(const UOLBTTask* Other)
{
	const UOLBTCompositeTask* OtherComposite = ConstCast<UOLBTCompositeTask>(Other);

	return OtherComposite != NULL && OtherComposite->GetClass() == GetClass()
		&& OtherComposite->CurrentChildIndex == CurrentChildIndex;
}

void UOLBTCompositeTask::GetTreeState(TArray<FBehaviorState>& CurrentState)
{
	AddTaskState(CurrentState);

	if (CurrentBehavior != NULL)
	{
		CurrentBehavior->GetTreeState(CurrentState);
	}
}

/*============================================================================*/

EStatus UOLBTRootTask::Update( FLOAT DeltaSeconds, TArray<FBehaviorState>* RootState /*= NULL*/, TArray<FBehaviorState>* CurrentState /*= NULL*/ )
{
	if (CurrentState != NULL) AddTaskState(*CurrentState);

	EStatus rStatus = CurrentBehavior->Tick(DeltaSeconds, RootState, CurrentState);

	if (CurrentState != NULL) CurrentState->Pop();

	return rStatus;
}

/*============================================================================*/

EStatus UOLBTSequenceTask::Update( FLOAT DeltaSeconds, TArray<FBehaviorState>* RootState /*= NULL*/, TArray<FBehaviorState>* CurrentState /*= NULL*/ )
{
	while (true)
	{
		if (CurrentState != NULL) AddTaskState(*CurrentState);

		EStatus s = CurrentBehavior->Tick(DeltaSeconds, RootState, CurrentState);

		if (s != Status_Success)
		{
			if (CurrentState != NULL) CurrentState->Pop();
			return s;
		}

		if (++CurrentChildIndex >= GetCompositeNode().Children.Num())
		{
			if (CurrentState != NULL) CurrentState->Pop();
			return Status_Success;
		}

		CurrentBehavior->Setup(GetCurrentChild(), Owner);
		if (CurrentState != NULL) CurrentState->Pop();
	}

	return Status_Invalid;
}

/*============================================================================*/

EStatus UOLBTSelectorTask::Update( FLOAT DeltaSeconds, TArray<FBehaviorState>* RootState /*= NULL*/, TArray<FBehaviorState>* CurrentState /*= NULL*/ )
{
	while (true)
	{
		if (CurrentState != NULL) AddTaskState(*CurrentState);

		EStatus s = CurrentBehavior->Tick(DeltaSeconds, RootState, CurrentState);

		if (s != Status_Failure)
		{
			if (CurrentState != NULL) CurrentState->Pop();
			return s;
		}

		if (++CurrentChildIndex >= GetCompositeNode().Children.Num())
		{
			if (CurrentState != NULL) CurrentState->Pop();
			return Status_Failure;
		}

		CurrentBehavior->Setup(GetCurrentChild(), Owner);
		if (CurrentState != NULL) CurrentState->Pop();
	}

	return Status_Invalid;
}

/*============================================================================*/

EStatus UOLBTDebugLogTask::Update( FLOAT DeltaSeconds, TArray<FBehaviorState>* RootState /*= NULL*/, TArray<FBehaviorState>* CurrentState /*= NULL*/ )
{
	UOLBTDebugLogNode* logNode = Cast<UOLBTDebugLogNode>(Node);

	AI_LOG(Owner, (TEXT("BT Log: %s"), *logNode->DebugString));

	return (EStatus)logNode->DebugReturnValue;
}

/*============================================================================*/

UOLBTTask* UOLBTIfCustomNode::CreateTask()
{
	check (IfTask != NULL);

	UOLBTTask* newTask = ConstructObject<UOLBTTask>(IfTask);
	newTask->Node = this;
	return newTask;
}

/*============================================================================*/

EStatus UOLBTIfTask::Update( FLOAT DeltaSeconds, TArray<FBehaviorState>* RootState /*= NULL*/, TArray<FBehaviorState>* CurrentState /*= NULL*/ )
{
	UOLBTIfNode* IfNode = Cast<UOLBTIfNode>(Node);

	if(eventCondition() && !IfNode->bNot || !eventCondition() && IfNode->bNot)
	{
		if (CurrentState != NULL) AddTaskState(*CurrentState);
		
		EStatus rStatus = CurrentBehavior->Tick(DeltaSeconds, RootState, CurrentState);

		if (CurrentState != NULL) CurrentState->Pop();

		return rStatus;
	}
	else
	{
		return Status_Failure;
	}
}

/*============================================================================*/

void UOLBTWaitTask::OnInitialize()
{
	UOLBTWaitNode* waitNode = Cast<UOLBTWaitNode>(Node);
	WaitTimer = waitNode->WaitTime;
}

EStatus UOLBTWaitTask::Update( FLOAT DeltaSeconds, TArray<FBehaviorState>* RootState /*= NULL*/, TArray<FBehaviorState>* CurrentState /*= NULL*/ )
{
	if (WaitTimer != -1.f)
	{
		WaitTimer -= DeltaSeconds;

		if (WaitTimer <= 0.f)
		{
			return Status_Success;
		}
	}

	return Status_Running;
}

/*============================================================================*/

EStatus UOLBTMoveToTask::Update( FLOAT DeltaSeconds, TArray<FBehaviorState>* RootState /*= NULL*/, TArray<FBehaviorState>* CurrentState /*= NULL*/ )
{
	if (bStartedAtDestination || Owner->CurrentMoveStatus == MS_Success)
	{
		return Status_Success;
	}
	else if (Owner->CurrentMoveStatus == MS_Moving || Owner->CurrentMoveStatus == MS_Pending)
	{
		return Status_Running;
	}
	else
	{
		return Status_Failure;
	}
}

/*============================================================================*/

void UOLBTMoveToWaypointTask::OnInitialize()
{
	ARoute* Route = Owner->PatrolRoute;
	if (Route != NULL && Owner->PatrolRouteIndex >= 0 && Owner->PatrolRouteIndex < Route->RouteList.Num())
	{
		PatrolRoute = Owner->PatrolRoute;
		PatrolRouteIndex = Owner->PatrolRouteIndex;

		AI_LOG(Owner, (TEXT("BT Log: Move to Waypoint - %s - %i"), *PatrolRoute->GetName(), PatrolRouteIndex));
		
		FMovementData newMove(EC_EventParm);
		newMove.DestinationActor = Owner->PatrolRoute->RouteList(Owner->PatrolRouteIndex).Actor;
		newMove.Type = MT_Actor;
		
		bStartedAtDestination = Owner->eventStartMove(newMove);
	}
	else
	{
		Owner->CurrentMoveStatus = MS_Failed;
		Owner->LastMoveFailedReason = MFR_Unknown;
	}
}

UBOOL UOLBTMoveToWaypointTask::CompareTaskState(const UOLBTTask* Other)
{
	const UOLBTMoveToWaypointTask* OtherWaypointTask = ConstCast<UOLBTMoveToWaypointTask>(Other);

	return OtherWaypointTask != NULL && OtherWaypointTask->PatrolRoute == Owner->PatrolRoute && OtherWaypointTask->PatrolRouteIndex == Owner->PatrolRouteIndex;
}

/*============================================================================*/

void UOLBTMoveToTargetTask::OnInitialize()
{
	FMovementData newMove(EC_EventParm);
	newMove.DestinationActor = Owner->TargetPlayer;
	newMove.Type = MT_Actor;
	newMove.bIsDynamic = TRUE;
	newMove.DestinationBuffer = (Owner->bPatrolToPlayer && Owner->BehaviorState == AIBS_Patrolling) ? Owner->PatrolToPlayerDistance : 0.0f; //Owner->EnemyPawn->AttackRange - 20.0f;
	newMove.bFocusOnActor = TRUE;

	bStartedAtDestination = Owner->eventStartMove(newMove);

	if (Owner->BehaviorState == AIBS_Chasing)
		Owner->bWasChasing = TRUE;
}

/*============================================================================*/

void UOLBTMoveToDisturbanceTask::OnInitialize()
{
	FMovementData newMove(EC_EventParm);
	if ((Owner->AudioDisturbance.TimeSinceUpdate < Owner->VisualDisturbance.TimeSinceUpdate || Owner->VisualDisturbance.TimeSinceUpdate == -1.0f)  && Owner->AudioDisturbance.TimeSinceUpdate >= 0.0f)
	{
		newMove.DestinationPoint = Owner->AudioDisturbance.Location;
	}
	else
	{
		newMove.DestinationPoint = Owner->VisualDisturbance.Location;
	}
	newMove.Type = MT_Point;
	newMove.DestinationBuffer = GImprovedAI ? 0.0f : 1000.0f;
	newMove.bIsDynamic = TRUE; // To not log debug info.

	bStartedAtDestination = Owner->eventStartMove(newMove);
}

UBOOL UOLBTMoveToDisturbanceTask::CompareTaskState(const UOLBTTask* Other)
{
	FMovementData newMove(EC_EventParm);
	if ((Owner->AudioDisturbance.TimeSinceUpdate < Owner->VisualDisturbance.TimeSinceUpdate || Owner->VisualDisturbance.TimeSinceUpdate == -1.0f)  && Owner->AudioDisturbance.TimeSinceUpdate >= 0.0f)
	{
		newMove.DestinationPoint = Owner->AudioDisturbance.Location;
	}
	else
	{
		newMove.DestinationPoint = Owner->VisualDisturbance.Location;
	}
	newMove.Type = MT_Point;
	newMove.DestinationBuffer = GImprovedAI ? 0.0f : 1000.0f;
	newMove.bIsDynamic = TRUE; // To not log debug info.

	return Super::CompareTaskState(Other) && Owner->eventCompareMoves(Owner->CurrentMove, newMove);
}

/*============================================================================*/

EStatus UOLBTCancelMoveTask::Update( FLOAT DeltaSeconds, TArray<FBehaviorState>* RootState /*= NULL*/, TArray<FBehaviorState>* CurrentState /*= NULL*/ )
{
	Owner->StopMoving(TRUE);

	return Status_Success;
}

/*============================================================================*/

EStatus UOLBTClearDisturbanceTask::Update( FLOAT DeltaSeconds, TArray<FBehaviorState>* RootState /*= NULL*/, TArray<FBehaviorState>* CurrentState /*= NULL*/ )
{
	Owner->eventClearDisturbance();
	return Status_Success;
}

/*============================================================================*/

EStatus UOLBTClearInvestigationTask::Update( FLOAT DeltaSeconds, TArray<FBehaviorState>* RootState /*= NULL*/, TArray<FBehaviorState>* CurrentState /*= NULL*/ )
{
	Owner->ClearInvestigation();
	return Status_Success;
}

/*============================================================================*/

void UOLBTPerformWaypointActionTask::OnInitialize()
{
	AOLWaypoint* waypoint = Owner->GetCurrentWaypoint();
	if (waypoint)
	{
		WaitTimer = waypoint->WaitTime;

		if (waypoint->AnimToPlay != NAME_None)
		{
			FVector AlignLocation = waypoint->Location;
			FVector Extent = Owner->EnemyPawn->GetCylinderExtent();
			AlignLocation.Z -= Owner->DistToGround(AlignLocation + VecZ(Owner->EnemyPawn->CylinderComponent->Translation.Z), Extent);

			FAnimationData BotAnimData;
			appMemZero(BotAnimData);
			BotAnimData.AnimationName = waypoint->AnimToPlay;
			BotAnimData.bLoop = waypoint->bLoopAnimation;
			BotAnimData.bOnWaypoint = TRUE;
			BotAnimData.Rate = 1.0f;
			BotAnimData.BlendInTime = 0.2f;
			BotAnimData.BlendOutTime = 0.2f;
			BotAnimData.StartTime = 0.f;
			BotAnimData.EndTime = 0.f;
			BotAnimData.bAlign = waypoint->bAlignAnimToWaypoint;
			BotAnimData.AlignLocation = AlignLocation;
			BotAnimData.AlignRotation = waypoint->Rotation.Vector();

			Owner->eventStartAnimating(BotAnimData, waypoint->bTurnToRotation ? waypoint->Rotation : Owner->EnemyPawn->Rotation);
			bPlayingAnim = TRUE;
			bLoopingAnim = waypoint->bLoopAnimation;
		}
		else if (waypoint->bTurnToRotation)
		{
			Owner->eventTurnTo(waypoint->Rotation);
		}

		waypoint->WaypointReachedEvent(Owner->EnemyPawn);
	}
}

EStatus UOLBTPerformWaypointActionTask::Update( FLOAT DeltaSeconds, TArray<FBehaviorState>* RootState /*= NULL*/, TArray<FBehaviorState>* CurrentState /*= NULL*/ )
{
	if ((!bPlayingAnim || Owner->CurrentAnimation.AnimationName == NAME_None || bLoopingAnim) && !Owner->bTurning && WaitTimer != -1.f)
	{
		WaitTimer -= DeltaSeconds;

		if (WaitTimer <= 0.f)
		{
			return Status_Success;
		}
	}

	return Status_Running;
}

/*============================================================================*/

EStatus UOLBTSelectNextWaypointTask::Update( FLOAT DeltaSeconds, TArray<FBehaviorState>* RootState /*= NULL*/, TArray<FBehaviorState>* CurrentState /*= NULL*/ )
{
	UBOOL bWaypointChosen = Owner->eventChooseNextPatrolWaypoint();
	if(bWaypointChosen)
	{
		return Status_Success;
	}
	else
	{
		return Status_Failure;
	}
}

/*============================================================================*/

const FAnimData* UOLBTPlayAnimTask::GetRandomAnimation()
{
	UOLBTPlayAnimNode* PlayAnimNode = Cast<UOLBTPlayAnimNode>(Node);

	if (PlayAnimNode->Animations.Num() <= 0)
	{
		return NULL;
	}

	FLOAT TotalWeight = 0.f;
	for(INT i = 0; i < PlayAnimNode->Animations.Num(); ++i)
	{
		TotalWeight += PlayAnimNode->Animations(i).Weight;
	}

	FLOAT RandomWeight = RandRange(0.f, TotalWeight);
	for(INT j = 0; j < PlayAnimNode->Animations.Num(); ++j)
	{
		RandomWeight -= PlayAnimNode->Animations(j).Weight;
		if(RandomWeight <= 0.f)
		{
			return &PlayAnimNode->Animations(j);
		}
	}

	return NULL;
}

void UOLBTPlayAnimTask::OnInitialize()
{
	const FAnimData* CurrentData = GetRandomAnimation();
	if (CurrentData != NULL)
	{
		UOLBTPlayAnimNode* PlayAnimNode = Cast<UOLBTPlayAnimNode>(Node);

		FAnimationData BotAnimData;
		appMemZero(BotAnimData);
		BotAnimData.AnimationName = CurrentData->AnimName;
		BotAnimData.bLoop = PlayAnimNode->bLoop;
		BotAnimData.Rate = CurrentData->Rate;
		BotAnimData.BlendInTime = CurrentData->BlendInTime;
		BotAnimData.BlendOutTime = CurrentData->BlendOutTime;
		BotAnimData.StartTime = CurrentData->StartTime;
		BotAnimData.EndTime = CurrentData->EndTime;

		Owner->eventStartAnimating(BotAnimData, Owner->EnemyPawn->Rotation);
	}
}

EStatus UOLBTPlayAnimTask::Update( FLOAT DeltaSeconds, TArray<FBehaviorState>* RootState /*= NULL*/, TArray<FBehaviorState>* CurrentState /*= NULL*/ )
{
	if (Owner->CurrentAnimation.AnimationName == NAME_None)
	{
		return Status_Success;
	}

	return Status_Running;
}

/*============================================================================*/

void UOLBTPlayAnimAtInvPointTask::OnInitialize()
{
	const FAnimData* CurrentData = GetRandomAnimation();
	if (CurrentData != NULL)
	{
		UOLBTPlayAnimAtInvPointNode* PlayAnimNode = Cast<UOLBTPlayAnimAtInvPointNode>(Node);
		FName AnimToPlay = CurrentData->AnimName;
		FRotator Rotation = Owner->EnemyPawn->Rotation;

		if (Owner->CurrentInvestigationPoint.InteractiveActor != NULL)
		{
			AOLAIInvestigationPoint* InvestigationPoint = Cast<AOLAIInvestigationPoint>(Owner->CurrentInvestigationPoint.InteractiveActor);
			if (InvestigationPoint != NULL)
			{
				if (InvestigationPoint->AnimToPlay != NAME_None)
				{
					AnimToPlay = InvestigationPoint->AnimToPlay;
				}

				if (InvestigationPoint->bUseRotation)
				{
					Rotation = InvestigationPoint->Rotation;
				}
			}
		}

		FAnimationData BotAnimData;
		appMemZero(BotAnimData);
		BotAnimData.AnimationName = AnimToPlay;
		BotAnimData.bLoop = PlayAnimNode->bLoop;
		BotAnimData.Rate = CurrentData->Rate;
		BotAnimData.BlendInTime = CurrentData->BlendInTime;
		BotAnimData.BlendOutTime = CurrentData->BlendOutTime;
		BotAnimData.StartTime = CurrentData->StartTime;
		BotAnimData.EndTime = CurrentData->EndTime;

		Owner->eventStartAnimating(BotAnimData, Rotation);
	}
}

/*============================================================================*/

void UOLBTAttackTask::OnInitialize()
{
	AttackSucceeded = Owner->AttackTarget(Cast<UOLBTAttackNode>(Node)->Type);
}

EStatus UOLBTAttackTask::Update( FLOAT DeltaSeconds, TArray<FBehaviorState>* RootState /*= NULL*/, TArray<FBehaviorState>* CurrentState /*= NULL*/ )
{
	if (!AttackSucceeded)
	{
		return Status_Failure;
	}

	if (!Owner->bAttacking)
	{
		return Status_Success;
	}

	return Status_Running;
}

/*============================================================================*/

EStatus UOLBTWaitForAnimTask::Update( FLOAT DeltaSeconds, TArray<FBehaviorState>* RootState /*= NULL*/, TArray<FBehaviorState>* CurrentState /*= NULL*/ )
{
	if (Owner->CurrentAnimation.AnimationName == NAME_None)
	{
		return Status_Success;
	}

	return Status_Running;
}

/*============================================================================*/

void UOLBTRepeatTask::OnInitialize()
{
	Super::OnInitialize();

	CurrRepetition = 0;
}

EStatus UOLBTRepeatTask::Update( FLOAT DeltaSeconds, TArray<FBehaviorState>* RootState /*= NULL*/, TArray<FBehaviorState>* CurrentState /*= NULL*/ )
{
	while (true)
	{
		if (CurrentState != NULL) AddTaskState(*CurrentState);
		EStatus s = CurrentBehavior->Tick(DeltaSeconds, RootState, CurrentState);

		if (s != Status_Success)
		{
			if (CurrentState != NULL) CurrentState->Pop();
			return s;
		}

		if (++CurrRepetition >= Cast<UOLBTRepeatNode>(&GetCompositeNode())->NumRepetitions)
		{
			if (CurrentState != NULL) CurrentState->Pop();
			return Status_Success;	
		}

		CurrentBehavior->Setup(GetCurrentChild(), Owner);
		if (CurrentState != NULL) CurrentState->Pop();
	}

	return Status_Invalid;
}

/*============================================================================*/

EStatus UOLBTSetupInvestigationTask::Update( FLOAT DeltaSeconds, TArray<FBehaviorState>* RootState /*= NULL*/, TArray<FBehaviorState>* CurrentState /*= NULL*/ )
{
	if (!Owner->bIsInvestigating)
	{
		Owner->SetupInvestigation();
	}

	return Status_Success;
}

/*============================================================================*/

void UOLBTMoveToNextInvPointTask::OnInitialize()
{
	FVector Point = Owner->GetCurrentInvestigationPoint().Location;

	if( !Point.IsZero())
	{
		FMovementData newMove(EC_EventParm);
		newMove.DestinationPoint = Point;
		newMove.Type = MT_Point;
		
		bStartedAtDestination = Owner->eventStartMove(newMove);
	}
	else
	{
		Owner->CurrentMoveStatus = MS_Failed;
		Owner->LastMoveFailedReason = MFR_Unknown;
	}
}

EStatus UOLBTMoveToNextInvPointTask::Update( FLOAT DeltaSeconds, TArray<FBehaviorState>* RootState, TArray<FBehaviorState>* CurrentState )
{
	EStatus CurrentStatus = Super::Update(DeltaSeconds, RootState, CurrentState);

	if (CurrentStatus != Status_Running)
	{
		Owner->bInvestigationPointValid = FALSE;
	}

	return CurrentStatus;
}

/*============================================================================*/

EStatus UOLBTSetBehaviorStateTask::Update( FLOAT DeltaSeconds, TArray<FBehaviorState>* RootState /*= NULL*/, TArray<FBehaviorState>* CurrentState /*= NULL*/ )
{
	UOLBTSetBehaviorStateNode* BSNode = CastChecked<UOLBTSetBehaviorStateNode>(Node);
	Owner->SetBehaviorState((EAIBehaviorState)BSNode->BehaviorState);

	return Status_Success;
}

/*============================================================================*/

EStatus UOLBTOptionalTask::Update( FLOAT DeltaSeconds, TArray<FBehaviorState>* RootState /*= NULL*/, TArray<FBehaviorState>* CurrentState /*= NULL*/ )
{
	while (true)
	{
		if (CurrentState != NULL) AddTaskState(*CurrentState);

		EStatus s = CurrentBehavior->Tick(DeltaSeconds, RootState, CurrentState);

		if (s == Status_Running || s == Status_ReachedCurrent)
		{
			if (CurrentState != NULL) CurrentState->Pop();
			return s;
		}

		if (++CurrentChildIndex >= GetCompositeNode().Children.Num())
		{
			if (CurrentState != NULL) CurrentState->Pop();
			return Status_Success;
		}

		CurrentBehavior->Setup(GetCurrentChild(), Owner);
		if (CurrentState != NULL) CurrentState->Pop();
	}

	return Status_Invalid;
}

/*============================================================================*/

EStatus UOLBTWaitForAttackTask::Update( FLOAT DeltaSeconds, TArray<FBehaviorState>* RootState /*= NULL*/, TArray<FBehaviorState>* CurrentState /*= NULL*/ )
{
	if (!Owner->bAttacking)
	{
		return Status_Success;
	}

	return Status_Running;
}

/*============================================================================*/

EStatus UOLBTPlayAkEventTask::Update( FLOAT DeltaSeconds, TArray<FBehaviorState>* RootState /*= NULL*/, TArray<FBehaviorState>* CurrentState /*= NULL*/ )
{
	UOLBTPlayAkEventNode* AkNode = CastChecked<UOLBTPlayAkEventNode>(Node);
	if (AkNode->Event)
	{
		UAkAudioDevice* AudioDevice = UAkAudioDevice::Get();

		if ( AudioDevice )
		{
			AudioDevice->PostEvent( AkNode->Event, Owner->EnemyPawn, Owner->EnemyPawn->HeadBoneName );
		}
	}

	return Status_Success;
}

/*============================================================================*/

EStatus UOLBTSelectClosestWaypointTask::Update( FLOAT DeltaSeconds, TArray<FBehaviorState>* RootState /*= NULL*/, TArray<FBehaviorState>* CurrentState /*= NULL*/ )
{
	UBOOL bWaypointChosen = Owner->eventChooseClosestPatrolWaypoint();
	if(bWaypointChosen)
	{
		return Status_Success;
	}
	else
	{
		return Status_Failure;
	}
}

/*============================================================================*/

void UOLBTMoveToSqueezeTask::OnInitialize()
{
	if (Owner->SightComponent->bSawPlayerInSqueeze)
	{
		AOLSqueezeVolume* Squeeze = Owner->SightComponent->LastSqueeze;

		AOLGameplayMarker* ClosestNode = NULL;
		if (Owner->EnemyPawn->Location.DistanceSquared(Squeeze->Node1->Location) < Owner->EnemyPawn->Location.DistanceSquared(Squeeze->Node2->Location))
		{
			ClosestNode = Squeeze->Node1;
		}
		else
		{
			ClosestNode = Squeeze->Node2;
		}

		FMovementData newMove(EC_EventParm);

		FVector SqueezeLocation;
		FVector SqueezeRotation;
		UBOOL bRight = FALSE;
		Owner->GetClosestSqueezeLocationAndRotation(Squeeze, SqueezeLocation, SqueezeRotation, bRight);

		newMove.DestinationPoint = SqueezeLocation;
		newMove.Type = MT_Point;
		newMove.DestinationBuffer = 0.0f;

		bStartedAtDestination = Owner->eventStartMove(newMove);
	}
	else
	{
		Owner->CurrentMoveStatus = MS_Failed;
		Owner->LastMoveFailedReason = MFR_Unknown;
	}
}

/*============================================================================*/

EStatus UOLBTClearSqueezeTask::Update( FLOAT DeltaSeconds, TArray<FBehaviorState>* RootState /*= NULL*/, TArray<FBehaviorState>* CurrentState /*= NULL*/ )
{
	Owner->SightComponent->bSawPlayerInSqueeze = FALSE;
	Owner->SightComponent->LastSqueeze = NULL;

	return Status_Success;
}

/*============================================================================*/

void UOLBTMoveToBedTask::OnInitialize()
{
	AOLBed* Bed = Owner->TargetPlayer->ActiveBed;

	if (Owner->SightComponent->bSawPlayerEnterBed && Bed != NULL)
	{
		BYTE Side = 0;
		FVector Destination = Owner->GetBestBedDestination(Bed, Side);

		if (!Destination.IsZero())
		{
			FMovementData newMove(EC_EventParm);
			newMove.DestinationPoint = Destination;
			newMove.Type = MT_Point;

			bStartedAtDestination = Owner->eventStartMove(newMove);
		}
		else
		{
			Owner->CurrentMoveStatus = MS_Failed;
			Owner->LastMoveFailedReason = MFR_Unknown;
		}
	}
	else
	{
		Owner->CurrentMoveStatus = MS_Failed;
		Owner->LastMoveFailedReason = MFR_Unknown;
	}
}

/*============================================================================*/

EStatus UOLBTStopPatrolTask::Update( FLOAT DeltaSeconds, TArray<FBehaviorState>* RootState /*= NULL*/, TArray<FBehaviorState>* CurrentState /*= NULL*/ )
{
	UOLBTStopPatrolNode* StopNode = CastChecked<UOLBTStopPatrolNode>(Node);
	Owner->eventStopPatrol(StopNode->bAbort);

	return Status_Success;
}

/*============================================================================*/

void UOLBTMoveToInvestigationTask::OnInitialize()
{
	if (Owner->bIsInvestigating)
	{
		bStartedAtDestination = TRUE;
		return;
	}

	FMovementData newMove(EC_EventParm);

	if ((Owner->AudioDisturbance.TimeSinceUpdate < Owner->VisualDisturbance.TimeSinceUpdate || Owner->VisualDisturbance.TimeSinceUpdate == -1.0f) && Owner->AudioDisturbance.TimeSinceUpdate != -1.0f)
	{
		newMove.DestinationPoint = Owner->AudioDisturbance.Location;
		newMove.DestinationBuffer = 0.0f;
		newMove.bIsInvestigation = !Owner->bNoTrimDisturbance;
	}
	else if (Owner->VisualDisturbance.TimeSinceUpdate != -1.0f)
	{
		newMove.DestinationPoint = Owner->VisualDisturbance.Location;
		newMove.DestinationBuffer = 0.0f;
		newMove.bIsInvestigation = !Owner->bNoTrimDisturbance;
	}
	else
	{
		newMove.DestinationPoint = Owner->TargetPlayer->Location;
		newMove.DestinationBuffer = 150.0f;
		newMove.bIsInvestigation = TRUE;
	}

	newMove.Type = MT_Point;
	newMove.bIsDynamic = TRUE;

	bStartedAtDestination = Owner->eventStartMove(newMove);
}

UBOOL UOLBTMoveToInvestigationTask::CompareTaskState(const UOLBTTask* Other)
{
	FMovementData newMove(EC_EventParm);

	if ((Owner->AudioDisturbance.TimeSinceUpdate < Owner->VisualDisturbance.TimeSinceUpdate || Owner->VisualDisturbance.TimeSinceUpdate == -1.0f) && Owner->AudioDisturbance.TimeSinceUpdate != -1.0f)
	{
		newMove.DestinationPoint = Owner->AudioDisturbance.Location;
		newMove.DestinationBuffer = 0.0f;
		newMove.bIsInvestigation = !Owner->bNoTrimDisturbance;
	}
	else if (Owner->VisualDisturbance.TimeSinceUpdate != -1.0f)
	{
		newMove.DestinationPoint = Owner->VisualDisturbance.Location;
		newMove.DestinationBuffer = 0.0f;
		newMove.bIsInvestigation = !Owner->bNoTrimDisturbance;
	}
	else
	{
		newMove.DestinationPoint = Owner->TargetPlayer->Location;
		newMove.DestinationBuffer = 150.0f;
		newMove.bIsInvestigation = TRUE;
	}

	return Super::CompareTaskState(Other) && Owner->eventCompareMoves(Owner->CurrentMove, newMove);
}

/*============================================================================*/

void UOLBTGetNextInvestigationPointTask::OnInitialize()
{
	if (Owner->ChooseNextInvestigationPoint())
	{
		CurrentChildIndex = Owner->GetCurrentInvestigationPoint().Type;

		CurrentBehavior = ConstructObject<UOLBTBehavior>(UOLBTBehavior::StaticClass());
		CurrentBehavior->Setup(GetCurrentChild(), Owner);
	}
	else
	{
		CurrentChildIndex = -1;
	}
}

EStatus UOLBTGetNextInvestigationPointTask::Update( FLOAT DeltaSeconds, TArray<FBehaviorState>* RootState /*= NULL*/, TArray<FBehaviorState>* CurrentState /*= NULL*/ )
{
	if (CurrentBehavior != NULL)
	{
		return CurrentBehavior->Tick(DeltaSeconds);
	}
	else
	{
		return Status_Failure;
	}
}

/*============================================================================*/

void UOLBTInvestigateObjectTask::OnInitialize()
{
	FInvestigationPoint* Point = &Owner->CurrentInvestigationPoint;

	bInvestigateSuccessful = FALSE;
	if (Point->Type == OLIPT_Locker)
	{
		AOLHidingSpot* Locker = Cast<AOLHidingSpot>(Point->InteractiveActor);
		if (Locker != NULL)
		{
			if (Owner->TargetPlayer != NULL && Owner->TargetPlayer->ActiveLocker == Locker)
			{
				Owner->SightComponent->CanSeeTarget = TRUE;
				bInvestigateSuccessful = Owner->AttackTarget(AT_Locker);
			}
			else
			{
				bInvestigateSuccessful = Owner->InvestigateObject(Locker);
			}
		}
	}
	else if (Point->Type == OLIPT_Bed)
	{
		AOLBed* Bed = Cast<AOLBed>(Point->InteractiveActor);
		if (Bed != NULL)
		{
			if (Owner->TargetPlayer != NULL && Owner->TargetPlayer->ActiveBed == Bed)
			{
				Owner->SightComponent->CanSeeTarget = TRUE;
				bInvestigateSuccessful = Owner->AttackTarget(AT_Bed);
			}
			else
			{
				bInvestigateSuccessful = Owner->InvestigateObject(Bed);
			}
		}
	}
}

EStatus UOLBTInvestigateObjectTask::Update( FLOAT DeltaSeconds, TArray<FBehaviorState>* RootState /*= NULL*/, TArray<FBehaviorState>* CurrentState /*= NULL*/ )
{
	if (!bInvestigateSuccessful)
	{
		return Status_Failure;
	}

	if (!Owner->bAttacking && !Owner->bInvestigatingObject)
	{
		return Status_Success;
	}

	return Status_Running;
}

/*============================================================================*/

void UOLBTMoveToLockerTask::OnInitialize()
{
	AOLHidingSpot* Locker = Owner->TargetPlayer->ActiveLocker;

	if (Owner->SightComponent->bSawPlayerEnterHidingSpot && Locker != NULL)
	{
		FVector Destination = Owner->GetLockerDestination(Locker);

		if (!Destination.IsZero())
		{
			FMovementData newMove(EC_EventParm);
			newMove.DestinationPoint = Destination;
			newMove.Type = MT_Point;

			bStartedAtDestination = Owner->eventStartMove(newMove);
		}
		else
		{
			Owner->CurrentMoveStatus = MS_Failed;
			Owner->LastMoveFailedReason = MFR_Unknown;
		}
	}
	else
	{
		Owner->CurrentMoveStatus = MS_Failed;
		Owner->LastMoveFailedReason = MFR_Unknown;
	}
}

/*============================================================================*/

void UOLBTActivateEventTask::OnInitialize()
{
	UOLBTActivateEventNode* EventNode = Cast<UOLBTActivateEventNode>(Node);

	if (GWorld->GetGameSequence())
	{
		TArray<USequenceObject*> AllBehaviorEvents;

		// Find all SeqEvent_Console objects anywhere.
		GWorld->GetGameSequence()->FindSeqObjectsByClass(UOLSeqEvent_BehaviorEvent::StaticClass(), AllBehaviorEvents);

		// Iterate over them, seeing if the name is the one we typed in.
		for (INT Idx = 0; Idx < AllBehaviorEvents.Num(); Idx++)
		{
			UOLSeqEvent_BehaviorEvent* BehaviorEvent = Cast<UOLSeqEvent_BehaviorEvent>(AllBehaviorEvents(Idx));
			if (BehaviorEvent != NULL)
			{
				TArray<UObject**> ObjVars;
				BehaviorEvent->GetObjectVars(ObjVars, TEXT("Targets"));

				for (INT EvtIdx = 0; EvtIdx < ObjVars.Num(); EvtIdx++)
				{
					AOLEnemyPawn* EventPawn = Cast<AOLEnemyPawn>(*(ObjVars(EvtIdx)));
					if(EventPawn == Owner->EnemyPawn && BehaviorEvent->EventName == EventNode->EventName)
					{
						BehaviorEvent->CheckActivate(Owner, Owner);
					}
				}
			}
		}
	}
}

/*============================================================================*/

UOLBTTask* UOLBTWaitHandlerNode::CreateTask()
{
	check (WaitHandlerTask != NULL);

	UOLBTTask* newTask = ConstructObject<UOLBTTask>(WaitHandlerTask);
	newTask->Node = this;
	return newTask;
}

/*============================================================================*/

void UOLBTWaitHandlerBaseTask::OnInitialize()
{
	AddOwnerToObject();

	if (IsObjectInUse())
	{
		AOLBot* BestBot = GetBestBotForObject();
		if (BestBot == Owner)
		{
			CurrentChildIndex = EWHS_Active;
			RecalculateActiveBot();
			SetOwnerAsActive();
		}
		else
		{
			CurrentChildIndex = EWHS_Waiting;
			GrabWaitPointOnOwner();
		}
	}
	else
	{
		CurrentChildIndex = EWHS_Active;
		SetOwnerAsActive();
	}

	CurrentBehavior = ConstructObject<UOLBTBehavior>(UOLBTBehavior::StaticClass());
	CurrentBehavior->Setup(GetCurrentChild(), Owner);
}

EStatus UOLBTWaitHandlerBaseTask::Update( FLOAT DeltaSeconds, TArray<FBehaviorState>* RootState /*= NULL*/, TArray<FBehaviorState>* CurrentState /*= NULL*/ )
{
	if (CurrentBehavior != NULL)
	{
		return CurrentBehavior->Tick(DeltaSeconds);
	}
	else
	{
		return Status_Failure;
	}
}

void UOLBTWaitHandlerBaseTask::OnTerminate(EStatus status)
{
	if (IsOwnerActive())
	{
		RemoveOwnerFromObject();
		RecalculateAllBots();
	}
	else
	{
		ReturnWaitPointOnOwner();
		RemoveOwnerFromObject();
	}

	Owner->ActiveWPComponent = NULL;

	Super::OnTerminate(status);
}

/*============================================================================*/

void UOLBTWaitHandlerLockerTask::OnInitialize()
{
	Locker = Owner->TargetPlayer->ActiveLocker;
	if (Locker != NULL)
	{
		Owner->ActiveWPComponent = Locker->WaitPointComponent;

		Super::OnInitialize();
	}
}

void UOLBTWaitHandlerLockerTask::OnTerminate(EStatus status)
{
	if (Locker != NULL)
	{
		Super::OnTerminate(status);
	}
	else
	{
		UOLBTCompositeTask::OnTerminate(status);
	}
}

void UOLBTWaitHandlerLockerTask::AddOwnerToObject()
{
	if (Locker != NULL)
	{
		Locker->Bots.AddUniqueItem(Owner);
	}
}

void UOLBTWaitHandlerLockerTask::RemoveOwnerFromObject()
{
	if (Locker != NULL)
	{
		Locker->Bots.RemoveItem(Owner);

		if (Locker->ActiveBot == Owner)
		{
			Locker->ActiveBot = NULL;
		}
	}
}

UBOOL UOLBTWaitHandlerLockerTask::IsObjectInUse()
{
	return Locker != NULL && Locker->Bots.Num() > 0;
}

UBOOL UOLBTWaitHandlerLockerTask::IsOwnerActive()
{
	return Locker != NULL && Locker->ActiveBot == Owner;
}

void UOLBTWaitHandlerLockerTask::SetOwnerAsActive()
{
	if (Locker != NULL)
	{
		Locker->ActiveBot = Owner;
	}
}

AOLBot* UOLBTWaitHandlerLockerTask::GetBestBotForObject()
{
	if (Locker != NULL)
	{
		// ActiveBot is using the Locker so don't replace it.
		if (Locker->ActiveBot != NULL && Locker->ActiveBot->ActiveLocker == Locker)
		{
			return Locker->ActiveBot;
		}

		// Otherwise Find the Closest Bot
		AOLBot* ClosestBot = NULL;
		FLOAT ClosestDistanceSq = 0.f;

		for (INT i = 0; i < Locker->Bots.Num(); ++i)
		{
			AOLBot* CurrentBot = Locker->Bots(i);
			FLOAT CurrentDistanceSq = CurrentBot->Location.DistanceSquared(Locker->Location);

			if (ClosestBot == NULL || ClosestDistanceSq > CurrentDistanceSq)
			{
				ClosestBot = CurrentBot;
				ClosestDistanceSq = CurrentDistanceSq;
			}
		}

		return ClosestBot;
	}

	return NULL;
}

void UOLBTWaitHandlerLockerTask::RecalculateActiveBot()
{
	if (Locker != NULL && Locker->ActiveBot != NULL)
	{
		Locker->ActiveBot->eventRecalculate();
	}
}

void UOLBTWaitHandlerLockerTask::RecalculateAllBots()
{
	if (Locker != NULL)
	{
		for (INT i = 0; i < Locker->Bots.Num(); ++i)
		{
			Locker->Bots(i)->eventRecalculate();
		}
	}
}

void UOLBTWaitHandlerLockerTask::GrabWaitPointOnOwner()
{
	if (Locker != NULL)
	{
		if (Owner->CurrentWaitPoint.Point.IsNearlyZero())
		{
			Owner->CurrentWaitPoint = Locker->WaitPointComponent->GrabBestWaitPoint(FALSE);
		}
		else
		{
			UBOOL bFound = FALSE;
			for (INT i = 0; i < Locker->WaitPointComponent->WaitPoints.Num(); ++i)
			{
				if (Owner->CurrentWaitPoint.Point == Locker->WaitPointComponent->WaitPoints(i).Point)
				{
					bFound = TRUE;

					break;
				}
			}

			check(bFound);
		}

	}
}

void UOLBTWaitHandlerLockerTask::ReturnWaitPointOnOwner()
{
	if (Locker != NULL)
	{
		Locker->WaitPointComponent->ReturnWaitPoint(Owner->CurrentWaitPoint);
		Owner->CurrentWaitPoint.Point = FVector(0.f);
	}
}

/*============================================================================*/

void UOLBTMoveToWaitPointTask::OnInitialize()
{
	if (!Owner->CurrentWaitPoint.Point.IsNearlyZero())
	{
		FMovementData newMove(EC_EventParm);
		newMove.DestinationPoint = Owner->CurrentWaitPoint.Point;
		newMove.Type = MT_Point;
		newMove.DestinationBuffer = 0.f;

		bStartedAtDestination = Owner->eventStartMove(newMove);
	}
	else
	{
		Owner->CurrentMoveStatus = MS_Failed;
		Owner->LastMoveFailedReason = MFR_Unknown;
	}
}

/*============================================================================*/

void UOLBTWaitHandlerBedTask::OnInitialize()
{
	Bed = Owner->TargetPlayer->ActiveBed;
	if (Bed != NULL)
	{
		Owner->ActiveWPComponent = Bed->WaitPointComponent;

		Super::OnInitialize();
	}
}

void UOLBTWaitHandlerBedTask::OnTerminate(EStatus status)
{
	if (Bed != NULL)
	{
		Super::OnTerminate(status);
	}
	else
	{
		UOLBTCompositeTask::OnTerminate(status);
	}
}

void UOLBTWaitHandlerBedTask::AddOwnerToObject()
{
	if (Bed != NULL)
	{
		Bed->Bots.AddUniqueItem(Owner);
	}
}

void UOLBTWaitHandlerBedTask::RemoveOwnerFromObject()
{
	if (Bed != NULL)
	{
		Bed->Bots.RemoveItem(Owner);

		if (Bed->ActiveBot == Owner)
		{
			Bed->ActiveBot = NULL;
		}
	}
}

UBOOL UOLBTWaitHandlerBedTask::IsObjectInUse()
{
	return Bed != NULL && Bed->Bots.Num() > 0;
}

UBOOL UOLBTWaitHandlerBedTask::IsOwnerActive()
{
	return Bed != NULL && Bed->ActiveBot == Owner;
}

void UOLBTWaitHandlerBedTask::SetOwnerAsActive()
{
	if (Bed != NULL)
	{
		Bed->ActiveBot = Owner;
	}
}

AOLBot* UOLBTWaitHandlerBedTask::GetBestBotForObject()
{
	if (Bed != NULL)
	{
		// ActiveBot is using the Locker so don't replace it.
		if (Bed->ActiveBot != NULL && Bed->ActiveBot->ActiveBed == Bed)
		{
			return Bed->ActiveBot;
		}

		// Otherwise Find the Closest Bot
		AOLBot* ClosestBot = NULL;
		FLOAT ClosestDistanceSq = 0.f;

		for (INT i = 0; i < Bed->Bots.Num(); ++i)
		{
			AOLBot* CurrentBot = Bed->Bots(i);
			FLOAT CurrentDistanceSq = CurrentBot->Location.DistanceSquared(Bed->Location);

			if (ClosestBot == NULL || ClosestDistanceSq > CurrentDistanceSq)
			{
				ClosestBot = CurrentBot;
				ClosestDistanceSq = CurrentDistanceSq;
			}
		}

		return ClosestBot;
	}

	return NULL;
}

void UOLBTWaitHandlerBedTask::RecalculateActiveBot()
{
	if (Bed != NULL && Bed->ActiveBot != NULL)
	{
		Bed->ActiveBot->eventRecalculate();
	}
}

void UOLBTWaitHandlerBedTask::RecalculateAllBots()
{
	if (Bed != NULL)
	{
		for (INT i = 0; i < Bed->Bots.Num(); ++i)
		{
			Bed->Bots(i)->eventRecalculate();
		}
	}
}

void UOLBTWaitHandlerBedTask::GrabWaitPointOnOwner()
{
	if (Bed != NULL)
	{
		if (Owner->CurrentWaitPoint.Point.IsNearlyZero())
		{
			BYTE Side = 0;
			FVector Destination = Owner->GetBestBedDestination(Bed, Side);

			Owner->CurrentWaitPoint = Bed->WaitPointComponent->GrabBestWaitPoint(Side != 0);
		}
		else
		{
			UBOOL bFound = FALSE;
			for (INT i = 0; i < Bed->WaitPointComponent->WaitPoints.Num(); ++i)
			{
				if (Owner->CurrentWaitPoint.Point == Bed->WaitPointComponent->WaitPoints(i).Point)
				{
					bFound = TRUE;

					break;
				}
			}

			check(bFound);
		}

	}
}

void UOLBTWaitHandlerBedTask::ReturnWaitPointOnOwner()
{
	if (Bed != NULL)
	{
		Bed->WaitPointComponent->ReturnWaitPoint(Owner->CurrentWaitPoint);
		Owner->CurrentWaitPoint.Point = FVector(0.f);
	}
}

/*============================================================================*/

void UOLBTRotateAtWaitPointTask::OnInitialize()
{
	if (!Owner->CurrentWaitPoint.Point.IsNearlyZero() && Owner->ActiveWPComponent != NULL)
	{
		Owner->eventTurnTo(Owner->ActiveWPComponent->GetWaitPointForwardVector(Owner->CurrentWaitPoint).Rotation());
	}
}

EStatus UOLBTRotateAtWaitPointTask::Update( FLOAT DeltaSeconds, TArray<FBehaviorState>* RootState, TArray<FBehaviorState>* CurrentState)
{
	if (!Owner->bTurning)
	{
		return Status_Success;
	}

	return Status_Running;
}

/*============================================================================*/

EStatus UOLBTResetNewDisturbanceTask::Update( FLOAT DeltaSeconds, TArray<FBehaviorState>* RootState, TArray<FBehaviorState>* CurrentState)
{
	Owner->bNewDisturbance = FALSE;
	Owner->NewDisturbanceResetTimer = 1.0f;

	return Status_Success;
}

/*============================================================================*/

void UOLBTLookAtDisturbanceTask::OnInitialize()
{
	Owner->eventTriggerDisturbed();
}

EStatus UOLBTLookAtDisturbanceTask::Update( FLOAT DeltaSeconds, TArray<FBehaviorState>* RootState, TArray<FBehaviorState>* CurrentState)
{
	if (!Owner->bDisturbed)
	{
		return Status_Success;
	}

	return Status_Running;
}

/*============================================================================*/

EStatus UOLBTWaitForReactionTask::Update( FLOAT DeltaSeconds, TArray<FBehaviorState>* RootState, TArray<FBehaviorState>* CurrentState)
{
	if (Owner->AIMoveReactionTimer > 0.f && !Owner->TargetPlayer->Velocity.IsNearlyZero(10.0f))
	{
		return Status_Running;
	}

	return Status_Success;
}

/*============================================================================*/

void UOLBTTargetUnreachableTask::OnTerminate(EStatus status)
{
	if (status == Status_Success)
	{
		UOLBTTargetUnreachableNode* TUNode = Cast<UOLBTTargetUnreachableNode>(Node);

		Owner->eventIgnoreTarget(TUNode->IgnoreTime);
	}
}

/*============================================================================*/