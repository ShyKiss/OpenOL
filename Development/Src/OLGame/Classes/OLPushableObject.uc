class OLPushableObject extends Actor
	native
	placeable
	implements(interface_NavMeshPathObstacle);

var() bool bEnabled;
var() float Width;
var() float MaxBackDist;
var() float MaxFwdDist;
var() bool bCanPushBack; // pushing from fwd edge in back direction
var() bool bCanPushFwd; // pushing from back edge in fwd direction
var() OLDoor LinkedDoor;
var() float MaxSpeed;
var() float BaseTranslation; // original translation of the mesh comp

enum PushableMaterial
{
	PM_Wood,
	PM_Metal,
};

var() PushableMaterial PushableType;

var() StaticMeshComponent Mesh;
var() DynamicLightEnvironmentComponent LightEnvironment;

var transient bool bPlayerLocked;
var transient bool  bNetLocked;        // remote player is currently pushing this object
var transient float NetPushLastTime;   // WorldInfo.TimeSeconds of last received push packet
var transient int   LocalPushSeq;      // monotonic sequence counter for packets we send
var transient int   RemotePushSeq;     // last received Seq (drop older packets)
var transient int   PushStopRepeat;    // remaining bPushing=0 packets to send on release
var transient bool bPushActive; // is there currently a push force moving this object
var transient bool bPushFwd; // when bPushActive, is the push forward or backward
var transient float CurrentDisplacement; // displacement, from base back edge towards fwd edge (can be negative)
var transient float CurrentVelocity; // max movement velocity, towards fwd edge (can be negative), post-modulated by phase
var transient float CurrentPhase; // positive, modulates the velocity

var const float AccelApproachCoeff;
var const float DecelApproachCoeff;

var const AkEvent SndStartPushing;
var const AkEvent SndStopPushing;

var const name RTPCPushingSpeed;
var const name SwitchPushableType;
var const name SwitchPushableTypeMetal;
var const name SwitchPushableTypeWood;

enum PushableEventType // Must match OLSeqEvent_Pushable outputs
{
	PET_StartedPushing,
	PET_StoppedPushing,
	PET_UnblockedDoor,
	PET_BlockedDoor,
};

cpptext
{
	virtual void PostBeginPlay();
	virtual void BeginDestroy();
	virtual void PostLoad();
	virtual UBOOL Tick( FLOAT DeltaSeconds, ELevelTick TickType );

	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent);

	// PathObstacle interface
	virtual UBOOL GetBoundingShape(TArray<FVector>& out_PolyShape,INT ShapeIdx);
	virtual UBOOL PreserveInternalPolys() { return FALSE; }
	virtual UBOOL Verify()
	{
		return !IsPendingKill();
	}

public:
	FVector GetMinBackEdge() const; // gets the minimum location (on the floor) where the back edge can be
	FVector GetMaxFwdEdge() const; // gets the maximum location (on the floor) where the forward edge can be
	FVector GetCurrentBackEdge() const; // gets the current location of the back edge
	FVector GetCurrentFwdEdge() const; // gets the current location of the forward edge
	FVector GetFwdDirection() const; // from back edge to fwd
	FVector GetBackDirection() const; // from back edge to fwd
	FVector GetSideDirection() const; // away from the wall

	void Reset();	
	void SetDisplacement(FLOAT newDisplacement);
	void StartPushing(); // locking the player
	void StopPushing(); // unlocking the player
	void ForcePushLeft();
	void ForcePushRight();

	UBOOL CanPush() const;
	UBOOL CanPushFwd() const;
	UBOOL CanPushBack() const;
	UBOOL TryPushFwd(); // player intent fwd
	UBOOL TryPushBack(); // player intent back
	void StopMoving(); // no player intent

public:
	void UpdateLinkedDoor();
private:
	void TriggerEvent(PushableEventType eventType, AOLPawn* instigator);

	// For PathObstacle
	void RegisterObstacle();
	void UnRegisterObstacle();
}

// Called by multiplayer to replicate pushable position to remote clients.
native function SetNetDisplacement(float NewDisplacement);
// Called after SetNetDisplacement to sync door bBlocked state on the receiving client.
native function UpdateLinkedDoorState();
// Called by multiplayer when remote player starts/stops pushing — updates NavMesh obstacle.
native function NetStartPushing();
native function NetStopPushing();

simulated function OnToggle(SeqAct_Toggle Action)
{
	// Turn ON
	if (Action.InputLinks[0].bHasImpulse)
	{
		bEnabled=true;
	}
	// Turn OFF
	else if (Action.InputLinks[1].bHasImpulse)
	{
		bEnabled=false;
	}
	// Toggle
	else if (Action.InputLinks[2].bHasImpulse)
	{
		bEnabled = !bEnabled;
	}
}

defaultproperties
{	
	bEnabled=true
	Width=136.0
	MaxBackDist=0.0
	MaxFwdDist=100.0
	bCanPushBack=true
	bCanPushFwd=true
	MaxSpeed=30.0
	AccelApproachCoeff=0.8
	DecelApproachCoeff=0.95
	BaseTranslation=68.0

	Begin Object Class=DynamicLightEnvironmentComponent Name=LightEnvironmentComp		
	End Object
	LightEnvironment=LightEnvironmentComp
	Components.Add(LightEnvironmentComp)	

	Begin Object Class=StaticMeshComponent Name=MeshComp
		StaticMesh=StaticMesh'Library.Futnitures.BibliothequeWithbooks'
		LightEnvironment=LightEnvironmentComp
		CastShadow=true
		bUsePrecomputedShadows=true
		LightingChannels=(Static=true,Dynamic=true)
		BlockActors=true
		CollideActors=true
	End Object
	Mesh=MeshComp
	CollisionComponent=MeshComp
	Components.Add(MeshComp)

	bStatic=false
	bMovable=true
	bCollideActors=true
	bBlockActors=true
	bEdShouldSnap=true

	SndStartPushing=AkEvent'Player_Sound.Miles_Push_Loop'
	SndStopPushing=AkEvent'Player_Sound.Miles_Push_Loop_Stop'

	PushableType=PM_Metal

	RTPCPushingSpeed=Vitesse_Push
	SwitchPushableType=Miles_Push_Type
	SwitchPushableTypeMetal=Metal
	SwitchPushableTypeWood=Wood

	SupportedEvents.Add(class'OLSeqEvent_Pushable')
}