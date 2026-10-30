class OLDoor extends Actor
	ClassGroup(OL,Ingredient)
	native
	placeable
	implements(Interface_NavMeshPathObject)
	implements(Interface_NavMeshPathObstacle);

////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////

//
// Components
//

var StaticMeshComponent Mesh;	// The door mesh.
var() StaticMeshComponent DoorMainMesh;

var() SkeletalMeshComponent BreakingForwardMesh;
var() name BreakingForwardAnim;
var() SkeletalMeshComponent BrokenForwardMesh;
var() name BrokenForwardAnim;

var() SkeletalMeshComponent BreakingBackwardMesh;
var() name BreakingBackwardAnim;
var() SkeletalMeshComponent BrokenBackwardMesh;
var() name BrokenBackwardAnim;

var() DynamicLightEnvironmentComponent DoorMainLightEnvironment;
var() OLSoundConnectorComponent SoundConnectorComp;
var() OLWaitPointComponent WaitPointComponent;

//
// Config
//

var() float InitialOpenAngle; //degs
var() editconst float MaxOpenAngle; // degs
var() editconst float PlayerOpenedAngle; // degs
var() bool bReverseDirection;
var() bool bLocked;
var() bool bFakeUnlocked; // when a door is locked, but we still want the player prompt to appear
var() bool bNoLockedInteraction;
var() bool bAutoClose;
var() bool bCantClose;
var() bool bNoPushKnob;
var() float OpeningSpeed; // degs/s
var() float ClosingSpeed; // degs/s
var() float ExplicitOcclusionFactor;
var() editconst float DefaultOcclusionFactor;

/** Disable interaction if the player owns the required item for the AssociatedCSA */
var() OLCSA UnlockingCSA;
var const float LockerOpeningSpeed; // degs/s
var const float LockerClosingSpeed; // degs/s
var const float StaticOffsetFwdToCenterNormal; // there unfortunately exists a small offset from the actor location to the middle of the frame (in the traversal direction)
var const float StaticOffsetFwdToCenterLocker; 
var const float LockedAnimTotalTime;
var const float LockedAnimAmplitude; // degs
var const float BlockedAnimTotalTime;
var const float BlockedAnimAmplitude; // degs
var const float AutoCloseAnimTotalTime;
var const float AutoCloseAnimAmplitude; // degs

var() enum EOLDoorType
{
	DT_Normal,
	DT_Locker,
} DoorType;

var enum EDoorMaterial
{
	OLDM_Wood,
	OLDM_Metal,
	OLDM_SecurityDoor,
	OLDM_BigPrisonDoor,
	OLDM_BigWoodenDoor,
} DoorMaterial; // for sound

var() enum EOLDoorMeshType
{
	DMesh_Undefined,
	DMesh_Wooden,
	DMesh_WoodenOld,
	DMesh_WoodenWindow,
	DMesh_WoodenWindowSmall,
	DMesh_WoodenWindowOld,
	DMesh_WoodenWindowOldSmall,
	DMesh_WoodenWindowBig,
	DMesh_Metal,
	DMesh_MetalWindow,
	DMesh_MetalWindowSmall,
	DMesh_Enforced,
	DMesh_Grid,
	DMesh_Prison,
	DMesh_Entrance,
	DMesh_Bathroom,
	DMesh_IsolatedCell,
	DMesh_Locker,
	DMesh_LockerRusted,
	DMesh_LockerBeige,
	DMesh_LockerGreen,
	DMesh_Glass,
	DMesh_Fence,
	DMesh_LockerHole
} DoorMeshType;

struct native DoorMeshDirData
{
	var StaticMesh					MainMesh;

	var SkeletalMesh	ForwardBreakingMesh;
	var AnimSet			ForwardBreakingAnimSet;
	var name			ForwardBreakingAnimation;

	var SkeletalMesh	ForwardBrokenMesh;
	var AnimSet			ForwardBrokenAnimSet;
	var name			ForwardBrokenAnimation;

	var SkeletalMesh	BackwardBreakingMesh;
	var AnimSet			BackwardBreakingAnimSet;
	var name			BackwardBreakingAnimation;

	var SkeletalMesh	BackwardBrokenMesh;
	var AnimSet			BackwardBrokenAnimSet;
	var name			BackwardBrokenAnimation;
};

var struct native DoorMeshTypeData
{
	var DoorMeshDirData NormalData;
	var DoorMeshDirData ReversedData;
	var array<MaterialInstance>	Materials;
	var EDoorMaterial DoorMaterialForSound;
	var float OcclusionFactor;
} DoorMeshData[EOLDoorMeshType];

var() array<MaterialInstance>	MaterialOverrides;

var const PhysicsAsset LeftPhysicsAsset;
var const PhysicsAsset RightPhysicsAsset;

var() vector DoorKnobOffset;

var() float PathPointOffset;
var() bool bAICanUseDoor;
var bool bAITraversing; // Can no longer block the door
var() bool bDontBreak;
var() bool bAlwaysBreak;
var() bool bWaitForTriggerToBreak;
var() int NumBashesAfterBlocked;
var() bool bTrimToDoorForInvestigate;
var() bool bSplitNavMesh;
var() bool bAIAlwaysCloses;

var() float AIOpenDoorKnockback;
var() bool bUseObstacleOnClose;

//
// Dynamic state
//

var float OpenRatio; // normalized [0,1]
var transient float TargetOpenRatio; // normalized [0,1]
var const float CurrentSpeed; // degs/s

enum EDoorState
{
	DS_Idle, // may be closed, open or partially open - query the related function
	DS_Opening,
	DS_Closing,
	DS_PlayerInteracting,
	DS_Animating,
};	

var EDoorState DoorState;

enum DoorEventType // Must match OLSeqEvent_Door outputs!
{
	DET_StartOpening,
	DET_Opened,
	DET_Closed,
	DET_TriedOnLocked,
	DET_OpenThresholdReached,
	DET_Bashed,
	DET_StartedBashing,
	DET_StartedClosing,
};

enum EDoorBreakState
{
	DBS_Normal,
	DBS_Breaking,
	DBS_Broken,
};

var EDoorBreakState DoorBreakState;

enum ECancelBashDirection
{
	ECBD_Both,
	ECBD_Forward,
	ECBD_Backward,
};

var float ProceduralAnimElapsedTime;
var bool bPlayingLockedAnim;
var bool bPlayingBlockedAnim;
var bool bPlayingAutoCloseAnim;
var bool bBreakTriggered;
var bool bBlocked;
var bool bWasInitiallyLocked;
var bool bInstantBreak;
var bool bPreciseCloseTiming;
var float PreciseCloseDuration;
var float PreciseCloseStartTime;

struct native DoorShakeData
{	
	/** shakes / sec */
	var() float Rate; 
	/** the ponctual effective rate is random in min/max range around the nominal rate, expressed as a fraction of the rate (i.e. 0.3: delay until next shake is 70% to 130% of average) */
	var() float RateVariation; 
	/** simple scaling factor: 1.0 = normal */
	var() float Intensity; 
	/** same principle as RateVariation (% of random range on either side of nominal intensity) */
	var() float IntensityVariation; 
	/** if > 0.0, automatically stop the shakes at that duration */
	var() float TotalDuration; 

	var() float AmplitudeYaw;
	var() float AmplitudeTrans;
	var() float FrequencyYaw;
	var() float FrequencyTrans;
	var() float ShakeDuration;
	var() float FadeExp;
	var() bool bShakeCamera;

	var transient bool bActive;
	var transient float GlobalStartedTime; // timestamp when the whole sequence started
	var transient float ShakeStartedTime; // when this particular shake started
	var transient float NextShakeStartTime;
	var transient float OriginalTranslationX;

	structdefaultproperties
	{
		Rate=0.6
		RateVariation=0.75
		Intensity=1.0
		IntensityVariation=0.5
		TotalDuration=-1.0
		AmplitudeYaw=7.5
		AmplitudeTrans=2.5
		FrequencyYaw=9.5
		FrequencyTrans=3.0
		ShakeDuration=0.4
		FadeExp=3.0
		bShakeCamera=true
	}
};

var DoorShakeData ShakeData;

var() DoorShakeData BashShakeData;

// Person interacting with the door
var OLPawn DoorUser;

// Set while TriggerEventByInt runs to prevent TriggerEvent from echoing DOOR_EVENT back.
var bool bRemoteTrigger;

// Set by multiplayer when this door's angle is driven by a remote DOOR_STATE packet.
// Prevents the local player from echoing it back as a Kismet-driven move.
var bool bNetDrivenMove;

// Set when this door was opened by local Kismet (bByPlayer=false DOOR_EVENT received).
// Prevents P2 from sending DOOR_STATE for a door already driven by P1's Kismet.
var bool bRemoteKismetDriven;

// Used for Waiting mechanic, ActiveBot is the one who can use the door when he gets there.
var OLBot ActiveBot;
var array<OLBot> Bots;

//
// Navmesh stuff
//

var vector Edge0Dest;
var vector Edge1Dest;

var bool bObstacleRegistered;

//
// Sounds
//

var bool bPlayingOpeningSound;
var bool bNetForcedClosed;
var float LastInteractiveSoundTime;
var float OpeningIntensity;

struct native DoorSoundEvents
{
	var const AKEvent SndLocked;
	var const AKEvent SndPush;
	var const AKEvent SndOpening;
	var const AKEvent SndClosing;
	var const AKEvent SndSlowClose;
	var const AKEvent SndBash;
};

var const array<DoorSoundEvents> Sounds; // indexed by material type

var const AKEvent SndStopOpening;
var const AKEvent SndLockerBash;
var const name RTPCOpeningDoorIntensity;

////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////

cpptext
{
	//
	// System
	//
	virtual void PostBeginPlay();
	virtual UBOOL Tick( FLOAT DeltaSeconds, ELevelTick TickType );
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent);
	virtual void SetVolumes(const TArray<class AVolume*>& Volumes);
	virtual void PostLoad();
	virtual void Reset();
	
#if WITH_EDITOR
	virtual void CheckForErrors();
#endif

	virtual void BeginDestroy()
	{
		Super::BeginDestroy();
		UnregisterObstacleWithNavMesh();
	}

	enum EClosingSoundType
	{
		CST_Normal,
		CST_Quiet,
		CST_NoSound,
	};

	//
	// Interactions
	//	
	void Open(class AOLPawn* instigator, FLOAT rotationSpeed = 0.0f, FLOAT targetAngle = 0.0f, UBOOL bNoSound = FALSE); // rotationSpeed - 0.0f means default opening speed, targetAngle - 0.0f means max angle
	void Close(class AOLPawn* instigator, FLOAT rotationSpeed = 0.0f, EClosingSoundType closingSoundType = CST_Normal); // 0.0f means default closing speed
	void CloseQuiet(class AOLPawn* instigator, FLOAT closeStartTime, FLOAT closeDuration);
	void StartedInteractiveOpening(class AOLPawn* instigator);	// slow player interaction
	void TriedOpening(class AOLPawn* instigator); // tried but locked
	void CheckTriggerOpenThresholdReachedEvent(FLOAT prevAngle, FLOAT newAngle);
	void PlayAutoCloseAnim(); // when trying an instant interact on an auto-close door
	void StartNewShake();
	void InstantBreak();
	void CancelBash(ECancelBashDirection CancelDirection);
			
	//
	// Location and setup info
	//
	FVector GetStaticDirection() const; // towards the direction the door opens, relative to door frame (not using current open angle)
	FVector GetDynamicDirection() const; // towards the direction the door opens, relative to door mesh (accounts for current open angle)
	FVector GetCenterLocation() const; // in the middle of the frame (independent of openness), on the ground
	FVector GetKnobLocation() const; // location of the knob in the door plane (with height but no side offset), updated with open angle
	FVector GetPivotLocation() const; // on the ground, at the pivot point of the door
	FVector GetEdgeLocation() const; // on the ground, on the moving edge of the door, updated with actual rotation
	FVector GetStaticKnobLocation() const;
	FVector GetStaticPivotToEdge() const;
	FVector GetStaticEdgeLocation() const; // on the ground, on the moving edge of the door, not using current angle
	FLOAT GetDoorWidth() const;	
	FLOAT GetOcclusionFactor() const;

	//
	// Dynamic queries
	//
	UBOOL IsOpened() const { return OpenRatio*MaxOpenAngle >= 30.0f; }
	UBOOL IsClosed() const { return appIsNearlyZero(OpenRatio, KINDA_SMALL_NUMBERF) || ShakeData.bActive; }
	UBOOL IsPartiallyOpened() const { return OpenRatio > KINDA_SMALL_NUMBERF && OpenRatio < 0.95f*Min(PlayerOpenedAngle, MaxOpenAngle)/MaxOpenAngle; }
	UBOOL IsClosing() const { return DoorState == DS_Closing; } // False for slow player interactions
	UBOOL IsOpening() const { return DoorState == DS_Opening; } // False for slow player interactions
	UBOOL IsBroken() const { return DoorBreakState == DBS_Broken; }
	UBOOL CanClose(AOLPawn* instigator);
	INT GetEncroachingPawns(TArray<AOLPawn*>& pawns);
	UBOOL UsedByAI() const;
		
	//
	// Direct control (no sounds or events are triggered)
	//
	void ForceOpen();
	void ForceClose();	
	void SetTargetOpenAngle(FLOAT newAngle);
	void SetTargetOpenRatio(FLOAT newTarget);
	void ForceOpenAngle(FLOAT newAngle);
	void ForceBreakDoor(UBOOL bForward);

	//
	// Interface_NavMeshPathObject
	//
	virtual void PostSubMeshUpdateForOwningPoly(FNavMeshPathObjectEdge* Edge, FNavMeshPolyBase* Poly, UNavigationMeshBase* New_SubMesh);
	virtual INT CostFor( const FNavMeshPathParams& PathParams, const FVector& PreviousPoint, FVector& out_PathEdgePoint, FNavMeshPathObjectEdge* Edge, FNavMeshPolyBase* SourcePoly );
	virtual UBOOL Supports( const FNavMeshPathParams& PathParams, FNavMeshPolyBase* CurPoly, FNavMeshPathObjectEdge* Edge, FNavMeshEdgeBase* PredecessorEdge);
	virtual UBOOL PrepareMoveThru( class IInterface_NavigationHandle* Interface, FVector& out_MovePt, FNavMeshPathObjectEdge* Edge );
	virtual UBOOL GetEdgeDestination( const FNavMeshPathParams& PathParams, FLOAT EntityRadius, const FVector& InfluencePosition, const FVector& EntityPosition, FVector& out_EdgeDest,	FNavMeshPathObjectEdge* Edge, UNavigationHandle* Handle);
	virtual UBOOL GetFinalEdgeDestination( FVector& out_EdgeDest, FNavMeshPathObjectEdge* Edge );
	virtual UBOOL DrawEdge( FDebugRenderSceneProxy* DRSP, FColor C, FVector DrawOffset, FNavMeshPathObjectEdge* Edge );
	virtual UBOOL AllowMoveToNextEdge(FNavMeshPathParams& PathParams, UBOOL bInPoly0, UBOOL bInPoly1);
	virtual UBOOL GetMeshSplittingPoly( TArray<FVector>& Poly, FLOAT& PolyHeight );
	virtual void CreateEdgesForPathObject( APylon* Py );
	virtual EEdgeHandlingStatus AddStaticEdgeIntoThisPO( EEdgeHandlingStatus Status, const FVector& inV1, const FVector& inV2, TArray<FNavMeshPolyBase*>& ConnectedPolys, INT PolyAssocatedWithThisPO, FLOAT SupportedEdgeWidth=-1.0f, BYTE EdgeGroupID=MAXBYTE);
	virtual UBOOL ModifyFinalPath( UNavigationHandle* Handle, INT Idx );
	virtual UBOOL Verify();

	//
	// Interface_NavMeshPathObstacle
	//
	virtual UBOOL GetBoundingShape(TArray<FVector>& out_PolyShape,INT ShapeIdx);
	virtual UBOOL VerifyObstacle() { return Verify(); }

	virtual EEdgeHandlingStatus AddObstacleEdge( EEdgeHandlingStatus Status, const FVector& inV1, const FVector& inV2, TArray<FNavMeshPolyBase*>& ConnectedPolys, UBOOL bEdgesNeedToBeDynamic, INT PolyAssocatedWithThisPO, FLOAT SupportedEdgeWidth=-1.0f, BYTE EdgeGroupID=MAXBYTE)
	{
		return AddObstacleEdgeForObstacle(Status,inV1,inV2,ConnectedPolys,bEdgesNeedToBeDynamic,PolyAssocatedWithThisPO,this,SupportedEdgeWidth,EdgeGroupID);
	}

	FVector GetWaitPointForwardVector(INT Idx);

private:
	void Init();
	void ApplyOpenRatio(FLOAT newOpenRatio);
	void PlayLockedAnimation();
	void PlayBlockedAnimation();
	void UpdateDoorShake();
	
	void UpdateMeshFromData();
	void UpdateMaterials(const TArray<class UMaterialInstance*>& Materials);
	void AlignMesh(UMeshComponent* Mesh, UBOOL bInvert);
	void OrientSubMeshes(); // set the rotation on the submeshes to match the main one
}

////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////

native function vector ScriptGetCenterLocation();

native final function RegisterNavMeshObstacle();
native final function UnregisterNavmeshObstacle();

function bool IsBroken() { return DoorBreakState == DBS_Broken; }

native function StartShaking(DoorShakeData shakeParams, optional bool bSwitchToBreakingMesh = true, optional bool bReversed, optional bool bFromAI);
native function StopShaking();

native function TriggerBreakDoorCameraShake();
native function TriggerEvent(DoorEventType EventType, OLPawn Triggerer);
function TriggerEventByInt(int EventType)
{
    local PlayerController PC;
    local OLPawn P;
    PC = GetALocalPlayerController();
    if (PC != None)
        P = OLPawn(PC.Pawn);
    `log("TRIG_BY_INT: door=" $ PathName(self) $ " EventType=" $ EventType $ " P=" $ (P != None ? PathName(P) : "None") $ " GenEvents=" $ GeneratedEvents.Length);
    bRemoteTrigger = true;
    TriggerEvent(DoorEventType(EventType), P);
    bRemoteTrigger = false;
}

native function float GetOpenAngle();
native function ForceOpenRatio(float NewRatio);
native function SetNetTargetOpenRatio(float NewRatio);
native function SetNetInteractiveAngle(float NewAngle, float Speed);
native function NetOpen(float RotationSpeed, float TargetAngle);
native function NetClose(float RotationSpeed);
// Called on receiving client to replicate Open()/Close() without sound and without NotifyHandlesToWait.
native function NetReplicateOpen(float RotationSpeed);
native function NetReplicateClose(float RotationSpeed);
native function NetReplicateInteractStart();

native function NotifyHandlesToRepath();
native function NotifyHandlesToWait();

native function SoftDestroy(); // Similar to what a Destroy would do, but the actor needs to stay alive for navmesh pathing

// Forces DoorMeshType to be re-derived from the currently assigned mesh material and
// refreshes the meshes/collision that depend on it.  Used by editor tooling after a
// door's mesh data is changed at runtime (e.g. the Spawns editor tab).
native function ForceInit();

simulated function OnDestroy(SeqAct_Destroy Action)
{
	SoftDestroy();
}

event bool ShouldBreak(OLBot Bot)
{
	return bAlwaysBreak || ((bLocked || Bot.EnemyPawn.EnemyMode == EM_Chase) && !bDontBreak);
}

event BashDoor(bool bReversed)
{
	local name BreakAnim;

	StopShaking();

	if (!bReversed)
	{
		BreakAnim = BreakingForwardAnim;
	}
	else
	{
		BreakAnim = BreakingBackwardAnim;
	}

	if (BreakAnim == '')
	{
		StartShaking(BashShakeData, true, bReversed, true);
	}
	else
	{
		if (DoorBreakState == DBS_Normal)
		{
			DoorBreakState = DBS_Breaking;

			DoorMainMesh.SetHidden(true);

			if (!bReversed)
			{
				BreakingForwardMesh.SetHidden(false);
			}
			else
			{
				BreakingBackwardMesh.SetHidden(false);
			}
		}

		BreakingForwardMesh.PlayAnim(BreakAnim);
	}

	DoorState = DS_Idle;
}

event BreakDoor(OLPawn Breaker, bool bReversed)
{
	StopShaking();

	SetCollisionType(COLLIDE_NoCollision);

	if (!bReversed)
	{
		CollisionComponent = BrokenForwardMesh;
	}
	else
	{
		CollisionComponent = BrokenBackwardMesh;
	}

	SetCollisionType(COLLIDE_BlockAll);

	if (DoorBreakState == DBS_Normal)
	{
		DoorMainMesh.SetHidden(true);
	}
	else if (DoorBreakState == DBS_Breaking)
	{
		if (!bReversed)
		{
			BreakingForwardMesh.SetHidden(true);
		}
		else
		{
			BreakingBackwardMesh.SetHidden(true);
		}
	}

	if (!bReversed)
	{
		BrokenForwardMesh.SetHidden(false);
		BrokenForwardMesh.bUpdateSkelWhenNotRendered = true; // force update even if the player's not looking

		BrokenForwardMesh.PlayAnim(BrokenForwardAnim);
	}
	else
	{
		BrokenBackwardMesh.SetHidden(false);
		BrokenBackwardMesh.bUpdateSkelWhenNotRendered = true; // force update even if the player's not looking

		BrokenBackwardMesh.PlayAnim(BrokenBackwardAnim);
	}

	DoorBreakState = DBS_Broken;

	TriggerEvent(DET_Bashed, Breaker);
	TriggerBreakDoorCameraShake();
	NotifyHandlesToRepath();
	UnregisterNavmeshObstacle();
}

////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////

defaultproperties
{
	`include(OLDoorData.uci)

	Begin Object Class=DynamicLightEnvironmentComponent Name=DoorMainLightEnvironmentComp		
	End Object
	DoorMainLightEnvironment=DoorMainLightEnvironmentComp
	Components.Add(DoorMainLightEnvironmentComp)	

	Begin Object Class=StaticMeshComponent Name=DoorMesh
		BlockActors=true
		CollideActors=true
		StaticMesh=StaticMesh'door.Standard.Door_L'
		LightEnvironment=DoorMainLightEnvironmentComp
		CastShadow=true
		bUsePrecomputedShadows=true
		LightingChannels=(Static=TRUE,Dynamic=FALSE)
		Translation=(X=-7.6,Y=50.0,Z=107.5)
	End Object
	Mesh=DoorMesh
	DoorMainMesh=DoorMesh
	CollisionComponent=DoorMesh
	Components.Add(DoorMesh)

	Begin Object Class=SkeletalMeshComponent Name=SkeletalMeshComponent_0
		SkeletalMesh=SkeletalMesh'01_Animated_Props.Doors.WoodenDoor_R_Front_State1'
		AnimSets(0)=AnimSet'01_Animated_Props.Doors.Door_R_Front_State1'
		Begin Object Class=AnimNodeSequence Name=AnimSeq
		End Object
		Animations=AnimSeq
		ReplacementPrimitive=None
		LightEnvironment=DoorMainLightEnvironmentComp
		HiddenGame=true
		HiddenEditor=true
		RBChannel=RBCC_GameplayPhysics
		bUpdateSkelWhenNotRendered=False
		LightingChannels=(bInitialized=True,Dynamic=True)
		RBCollideWithChannels=(Default=True,GameplayPhysics=True,EffectPhysics=True,BlockingVolume=True)
		Translation=(X=-7.6,Y=50.0,Z=107.5)
	End Object
	BreakingForwardMesh=SkeletalMeshComponent_0
	Components.Add(SkeletalMeshComponent_0)

	BreakingForwardAnim=WoodenDoor_R_Front_State1

	Begin Object Class=SkeletalMeshComponent Name=SkeletalMeshComponent_1
		SkeletalMesh=SkeletalMesh'01_Animated_Props.Doors.WoodenDoor_R_Front_State2'
		AnimSets(0)=AnimSet'01_Animated_Props.Doors.Door_R_Front_State2'
		Begin Object Class=AnimNodeSequence Name=AnimSeq
		End Object
		Animations=AnimSeq
		ReplacementPrimitive=None
		LightEnvironment=DoorMainLightEnvironmentComp
		HiddenGame=true
		HiddenEditor=true
		RBChannel=RBCC_GameplayPhysics
		bUpdateSkelWhenNotRendered=False
		LightingChannels=(bInitialized=True,Dynamic=True)
		RBCollideWithChannels=(Default=True,GameplayPhysics=True,EffectPhysics=True,BlockingVolume=True)
		Translation=(X=-7.6,Y=50.0,Z=107.5)
	End Object
	BrokenForwardMesh=SkeletalMeshComponent_1
	Components.Add(SkeletalMeshComponent_1)

	BrokenForwardAnim=WoodenDoor_R_Front_State2

	Begin Object Class=SkeletalMeshComponent Name=SkeletalMeshComponent_2
		SkeletalMesh=SkeletalMesh'01_Animated_Props.Doors.WoodenDoor_L_Front_State1'
		AnimSets(0)=AnimSet'01_Animated_Props.Doors.Door_L_Front_State1'
		Begin Object Class=AnimNodeSequence Name=AnimSeq
		End Object
		Animations=AnimSeq
		ReplacementPrimitive=None
		LightEnvironment=DoorMainLightEnvironmentComp
		HiddenGame=true
		HiddenEditor=true
		RBChannel=RBCC_GameplayPhysics
		bUpdateSkelWhenNotRendered=False
		LightingChannels=(bInitialized=True,Dynamic=True)
		RBCollideWithChannels=(Default=True,GameplayPhysics=True,EffectPhysics=True,BlockingVolume=True)
		Translation=(X=-7.6,Y=50.0,Z=107.5)
	End Object
	BreakingBackwardMesh=SkeletalMeshComponent_2
	Components.Add(SkeletalMeshComponent_2)

	BreakingBackwardAnim=WoodenDoor_L_Front_State1

	Begin Object Class=SkeletalMeshComponent Name=SkeletalMeshComponent_3
		SkeletalMesh=SkeletalMesh'01_Animated_Props.Doors.WoodenDoor_L_Front_State2'
		AnimSets(0)=AnimSet'01_Animated_Props.Doors.Door_L_Front_State2'
		Begin Object Class=AnimNodeSequence Name=AnimSeq
		End Object
		Animations=AnimSeq
		ReplacementPrimitive=None
		LightEnvironment=DoorMainLightEnvironmentComp
		HiddenGame=true
		HiddenEditor=true
		RBChannel=RBCC_GameplayPhysics
		bUpdateSkelWhenNotRendered=False
		LightingChannels=(bInitialized=True,Dynamic=True)
		RBCollideWithChannels=(Default=True,GameplayPhysics=True,EffectPhysics=True,BlockingVolume=True)
		Translation=(X=-7.6,Y=50.0,Z=107.5)
	End Object
	BrokenBackwardMesh=SkeletalMeshComponent_3
	Components.Add(SkeletalMeshComponent_3)

	BrokenBackwardAnim=WoodenDoor_L_Front_State2

	Begin Object Class=OLSoundConnectorComponent Name=SoundConnectorComponent
		SphereRadius=50.0
		bSpherical=true
	End Object
	SoundConnectorComp=SoundConnectorComponent
	Components.Add(SoundConnectorComponent)

	Begin Object Class=OLWaitPointComponent Name=WPC
	End Object
	WaitPointComponent=WPC
	Components.Add(WPC)

	SupportedEvents.Add(class'OLSeqEvent_Door')

	LeftPhysicsAsset=PhysicsAsset'01_Animated_Props.Doors.Doors_Physics'
	RightPhysicsAsset=PhysicsAsset'01_Animated_Props.Doors.Doors_Physics_R'

	DoorType=DT_Normal

	OpenRatio=0.0f
	OpeningSpeed=150.0
	ClosingSpeed=240.0
	LockerOpeningSpeed=150.0
	LockerClosingSpeed=175.0
	MaxOpenAngle=95.0
	PlayerOpenedAngle=95.0

	LockedAnimTotalTime=0.25
	LockedAnimAmplitude=0.6
	BlockedAnimTotalTime=0.6
	BlockedAnimAmplitude=8.0
	AutoCloseAnimTotalTime=0.3
	AutoCloseAnimAmplitude=25.0

	DoorKnobOffset=(X=3.0,Y=-95.0,Z=-10.0)
	StaticOffsetFwdToCenterNormal=3.0
	StaticOffsetFwdToCenterLocker=25.0

	ExplicitOcclusionFactor=-1.0

	bEdShouldSnap=True

	NumBashesAfterBlocked=10 
	bAICanUseDoor=true
	bSplitNavMesh=true
	bObstacleRegistered=false
	bTrimToDoorForInvestigate=true
	bUseObstacleOnClose=false
	AIOpenDoorKnockback=10.0
	PathPointOffset=60.0
	Edge0Dest=(X=0.0,Y=0.0,Z=0.0)
	Edge1Dest=(X=0.0,Y=0.0,Z=0.0)
	
	bStatic=false
	bCollideActors=true
	bBlockActors=true
	Physics=PHYS_None

	BashShakeData={(
		TotalDuration=0.2f
	)}

	Begin Object Class=ArrowComponent Name=ArrowComponent0
		ArrowColor=(R=245,G=0,B=0)
		bTreatAsASprite=True
		SpriteCategoryName="OutlastGameplay"
		Translation=(Z=135.0)
		Rotation=(Yaw=32768)
	End Object
	Components.Add(ArrowComponent0)
	
	Sounds(OLDM_Wood)={(
		SndLocked=AkEvent'General_Sound.EV_Closed_Wood_Door',
		SndPush=AkEvent'General_Sound.EV_Push_Wood_Door',
		SndOpening=AkEvent'General_Sound.EV_Opening_Wood_Door',
		SndClosing=AkEvent'General_Sound.EV_Closing_Wood_Door',
		SndSlowClose=AkEvent'DLC_SFX_generic.SFX_Door_wood_Close_Slow',
		SndBash=AkEvent'NPCs_Sounds.NPC_BashingDoor',
	)}

	Sounds(OLDM_Metal)={(
		SndLocked=AkEvent'General_Sound.EV_Closed_Metal_Door',
		SndPush=AkEvent'General_Sound.EV_Push_Metal_Door',
		SndOpening=AkEvent'General_Sound.EV_Push_Metal_Door',
		SndClosing=AkEvent'General_Sound.EV_Closing_Metal_Door',
		SndSlowClose=AkEvent'DLC_SFX_generic.SFX_Door_metal_Close_Slow',
		SndBash=AkEvent'NPCs_Sounds.NPC_BashingDoor',
	)}

	Sounds(OLDM_SecurityDoor)={(
		SndLocked=AkEvent'General_Sound.EV_Closed_Security_Door',
		SndPush=AkEvent'General_Sound.EV_Opening_Security_Door',
		SndOpening=AkEvent'General_Sound.EV_Opening_Security_Door',
		SndClosing=AkEvent'General_Sound.EV_Closing_Security_Door',
		SndSlowClose=AkEvent'DLC_SFX_generic.SFX_Closing_Security_Door_slow',
		SndBash=AkEvent'NPCs_Sounds.NPC_BashingDoor',
	)}

	Sounds(OLDM_BigPrisonDoor)={(
		SndLocked=AkEvent'General_Sound.EV_Closed_Metal_Door',
		SndPush=AkEvent'General_Sound.EV_Push_Metal_Door',
		SndOpening=AkEvent'General_Sound.EV_Push_Metal_Door',
		SndClosing=AkEvent'General_Sound.EV_Closing_Metal_Door',
		SndSlowClose=AkEvent'DLC_SFX_generic.SFX_Door_metal_Close_Slow',
		SndBash=AkEvent'NPCs_Sounds.NPC_BashingDoor',
	)}

	Sounds(OLDM_BigWoodenDoor)={(
		SndLocked=AkEvent'General_Sound.EV_Closed_Wood_Door',
		SndPush=AkEvent'General_Sound.EV_Chapel_door_opening',
		SndOpening=AkEvent'General_Sound.EV_Chapel_door_opening',
		SndClosing=AkEvent'General_Sound.EV_Chapel_door_closing',
		SndSlowClose=AkEvent'DLC_SFX_generic.SFX_Door_wood_Close_Slow',
		SndBash=AkEvent'NPCs_Sounds.NPC_BashingDoor',
	)}

	SndStopOpening=AkEvent'General_Sound.EV_Stop_Opening'
	SndLockerBash=AkEvent'NPCs_Sounds.NPC_Locked_In_Locker_Hit'
	RTPCOpeningDoorIntensity=OpeningDoorIntensity
}
