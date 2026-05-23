/**
 * AI Controller for Outlast.
 *
 * Copyright 2012 Red Barrels, Inc. All Rights Reserved.
 */
class OLBot extends GameAIController
	dependson(OLEnemyPawn)
	native(AI)
	config(Enemy);

/** Our Pawn casted to OLEnemyPawn. */
var OLEnemyPawn EnemyPawn;

var OLAIGroup Group;

/** The Player */
var OLHero TargetPlayer;

var OLNavigationHandle OLNavHandle;

var bool bReGeneratePath;
var bool bRegenerateWhilePerforming;

var vector NavigationExtent;

/** The component that handles enemies seeing the player. */
var OLAISightComponent SightComponent;

/** The currently running behavior. */
var OLBTBehavior RootBehavior;

var() bool bDebugBehaviorTransitions;
var() bool bDebugThrowCalculations;

var bool ShouldRecalculate;
var bool bForceRecalculate;

var name InterruptionState;

/**
 * AI Behavior State
 */
var enum EAIBehaviorState
{
	AIBS_Idle,
	AIBS_Patrolling,
	AIBS_Investigating,
	AIBS_Chasing,
} BehaviorState;

var EAIBehaviorState LastBehaviorState;

/**
 * AI Environment
 */
var enum EAIEnvironment
{
	AIE_Normal,
	AIE_Darkness,
	AIE_Electricity,
} CurrentEnvironment;

/**
 * Movement Variables
 */
enum EMoveStatus
{
	MS_Moving,
	MS_Success,
	MS_Failed,
	MS_Pending,
};

enum EMoveType
{
	MT_Invalid,
	MT_Point,
	MT_Actor,
};

enum EMoveFailedReason
{
	MFR_Unknown,
	MFR_AINotOnNavMesh,
	MFR_TargetNotOnNavMesh,
	MFR_NoPathToTarget,
	MFR_Aborted,
};

var struct native MovementData
{
	var Actor DestinationActor;
	var vector DestinationPoint;
	var vector ValidatedMovePoint;
	var float DestinationBuffer;
	var bool bIsDynamic;
	var bool bIsInvestigation;
	var bool bFocusOnActor;
	var EMoveType Type;
} CurrentMove;

var EMoveStatus CurrentMoveStatus;
var EMoveFailedReason LastMoveFailedReason;
var MovementData NextMove;

var const float DynamicPathCheckTime;

var GameAICommand QueuedCommand;

var OLDoor				ActiveDoor;
var OLLedgeMarker		ActiveLedge;
var OLBashableObject	ActiveBashable;
var OLBed				ActiveBed;
var OLHidingSpot		ActiveLocker;
var OLAIVaultMarker		ActiveVault;

var byte CurrentBedSide; // 0 = Left, 1 = Right;

var OLWaitPointComponent ActiveWPComponent;

var bool bOpeningDoor;
var bool bBreachingDoor;
var bool bTrimmedToDoor;
var bool bFinishedDoor;
var bool bCancelBash;

var bool bWasChasing;

/** temp movement variables */
var vector MoveTempDest;
var vector MoveLastLocation;
var float MoveTimeSinceLastPath;

var bool bMoveCancelled;

var float TempTrimAmount;

var array<vector> MoveTempPath;
var transient float MoveModifiedBufferDist;

var float CheckStuckTimer;
var const float CheckStuckSpeedThreshold;
var float LookAheadTimer;
var float StuckRepathDelayTimer;
var const float StuckRepathDelayLength;
var float IgnoreTimer;
var float AIMoveReactionTimer;

var WaitPoint CurrentWaitPoint;
var float WaitForMoveTime;

var struct native AnimationData
{
	var name AnimationName;
	
	var bool bLoop;
	var bool bOnWaypoint;

	var float Rate;
	var float BlendInTime;
	var float BlendOutTime;
	var float StartTime;
	var float EndTime;

	var bool bAlign;
	var vector AlignLocation;
	var vector AlignRotation;
} CurrentAnimation;

var bool bTurning;
var rotator TurnToDirection;

var bool InAttackRange;
var bool bInDarkness;
var bool bTargetInDarkness;
var bool bInElectricity;

var transient bool bEnableHeadTracking;

var bool bAvoiding;

/**
 * Patrol Variables
 */
var bool bPatrolToPlayer;
var float PatrolToPlayerDistance;
var float PatrolToPlayerUpdateRate;

var float PatrolToPlayerLastUpdated;
var vector PatrolToPlayerLastLocation;

var Route PatrolRoute;
var int PatrolRouteIndex;
var int NextPatrolRouteIndex;
var bool bReversePatrolRoute;
var byte bPatrolComplete;
var EEnemyMode PatrolMode;

/**
 * Disturbance Variables
 */
struct native DelayedNoise
{
	var name NoiseType;
	var vector Location;
	var float TimeToNoise;
};

var init array<DelayedNoise> DelayedNoises;

struct native Disturbance
{
	var vector Location;
	var float TimeSinceUpdate;
};

var Disturbance VisualDisturbance;
var Disturbance AudioDisturbance;

var bool bNewDisturbance;
var float NewDisturbanceResetTimer;

var float IgnoreDisturbanceTimer;

var bool bNoTrimDisturbance;

var transient float DebugLastLoudness;
var transient float DebugLastOcclusion;
var transient float DebugLastDistance;

var bool bDisturbed;

/**
 * Investigation Values
 */

// Shared
var bool bInvestigationValid;
var array<OLBot> InvestigatingBots;

enum EOLInvestigationPointType
{
	OLIPT_Normal,
	OLIPT_Locker,
	OLIPT_Bed,
};

struct native InvestigationPoint
{
	var vector Location;
	var Actor InteractiveActor;
	var EOLInvestigationPointType Type;
	var bool bOccupied;
};

var array<InvestigationPoint> InvestigationPoints;
var float InvestigateTotalWeight;

var OLAIInvestigationVolume InvestigationVolume;
var vector InvestigationOrigin;

// Local
var OLBot InvestigationOwner;

var bool bIsInvestigating;
var bool bInvestigationFirstPoint;
var bool bInvestigationPointValid;

var InvestigationPoint CurrentInvestigationPoint;

var bool bInvestigatingObject;
var vector InvestigateStartLocation;
var vector InvestigateStartRotation;

/**
 * Hearing Values
 */
var float CurrentNoiseValue;
var float TimeSinceNoise;

/**
 * Attacking Values
 */
var enum AIAttackType
{
	AT_Normal,
	AT_Squeeze,
	AT_Locker,
	AT_Bed,
	AT_Grab,
	AT_GrabCrouch,
	AT_GrabUnder,
	AT_Push,
} CurrentAttackType;

var bool bAttacking;
var bool bAttackRight;
var bool bAttackCycling;
var bool bKilling;

var bool bUseQuickAttack;
var bool bDamageTargetTicking;

var bool bTookDamage;

var float AttackTimer;

var vector AttackStartLocation;
var vector AttackStartRotation;
var vector GrabTargetStartLocation;

var float ThrowPlayerRotation;

var bool bKnockedBack;

/*============================================================================*/

/*
 * Functions
 */
cpptext
{
	DECLARE_FUNCTION(execPollWaitForFullBodyAnim);
	DECLARE_FUNCTION(execPollWaitForSpecialMove);
	DECLARE_FUNCTION(execPollWaitForSpecialMoveNoDelay);
	DECLARE_FUNCTION(execPollMoveAlongPath);

	UBOOL Tick( FLOAT DeltaSeconds, ELevelTick TickType );
	virtual void UpdatePawnRotation() {} // Updating in Pawn now.

	void TickBehavior( FLOAT DeltaSeconds );

	UBOOL BotCanHear(FLOAT DistToNoise, FLOAT Loudness, FLOAT Occlusion);
	virtual void HearNoise(AActor* NoiseMake, FLOAT Loudness, FName NoiseType);

	void SetupInvestigation();
	UBOOL ChooseNextInvestigationPoint();
	FInvestigationPoint GetCurrentInvestigationPoint() const { return CurrentInvestigationPoint; }

	void SetBehaviorState(EAIBehaviorState aState) { BehaviorState = aState; UpdateAnimationMode(); }

	FVector GetLockerDestination(const class AOLHidingSpot* Locker, UBOOL bInvestigating = FALSE);
	FVector GetBestBedDestination(const class AOLBed* Bed, BYTE& BedSide, UBOOL bInvestigating = FALSE);

	/** IMPLEMENT Interface_NavigationHandle */
	virtual void SetupPathfindingParams( FNavMeshPathParams& out_ParamCache );
	virtual FVector GetEdgeZAdjust(FNavMeshEdgeBase* Edge);
	/** END */

	virtual FRotator SetRotationRate(FLOAT deltaTime);

	virtual void PollMoveComplete();

	/** base function called to kick off moveto latent action (called from execMoveTo and execMoveToDirectNonPathPos) */
	virtual void MoveTo(const FVector& Dest, AActor* ViewFocus, FLOAT DesiredOffset, UBOOL bShouldWalk);

	void TryNewDisturbance();

	void GetClosestSqueezeLocationAndRotation(const AOLSqueezeVolume* Squeeze, FVector& SqueezeLocation, FVector& SqueezeRotation, UBOOL& bRight);

	FLOAT DistToGround(const FVector& Point, const FVector& Extent = FVector(0.f));

	UBOOL CheckAttackZDiff(FVector TestLocation);

protected:

	UBOOL GetNextInvestigationPointOnOwner(AOLBot* Bot, FInvestigationPoint& out_Point);
	UBOOL ValidateInvestigationPoint(AOLBot* Bot, const FInvestigationPoint& PickedPoint);
	INT GetRandomInvestigationPointIdx();

	void TryPushBlockingPawns();

private:
	void DrawDebug();

	FLOAT DistToClosestPawn();

	UBOOL PerformThrowPositionCheck(const FVector& ThrowStartPlayerPosition);
	UBOOL CalculateThrowDirection(const FVector& ThrowStartLocation, const FVector& ThrowStartDirection, FLOAT ThrowOffset = 0.f, UBOOL bForceDirection = FALSE, UBOOL bForceClockwise = TRUE);

	void SetupInvestigationInternalWithVolume(class AOLAIInvestigationVolume* Volume);
	void SetupInvestigationInternalNoVolume();
	FLOAT GetInvestigateWeight(const FInvestigationPoint& Point) const;

	void UpdatePreferredPaths();
}

simulated function PostBeginPlay()
{
	Super.PostBeginPlay();

	FindPlayer();

	VisualDisturbance.TimeSinceUpdate = -1.0f;
	AudioDisturbance.TimeSinceUpdate = -1.0f;
}

simulated function InitNavigationHandle()
{
	Super.InitNavigationHandle();

	OLNavHandle = OLNavigationHandle(NavigationHandle);
}

event FindPlayer()
{
	local OLPlayerController PC;

	foreach WorldInfo.AllControllers(class'OLPlayerController', PC)
	{
		TargetPlayer = OLHero(PC.Pawn);
		break;
	}

	if (TargetPlayer == None)
	{
		`warn("No Player found for Bot, something is wrong.");
	}
}

event Destroyed()
{
	Super.Destroyed();

	if (Group != None)
	{
		Group.RemoveBot(self);
	}

	if (ActiveDoor != None)
	{
		ActiveDoor.DoorUser = None;
	}
}

function Possess(Pawn aPawn, bool bVehicleTransition)
{
	if (aPawn.bDeleteMe)
	{
		`Warn(self @ GetHumanReadableName() @ "attempted to possess destroyed Pawn" @ aPawn);
		ScriptTrace();
		//GotoState('Dead');
	}
	else
	{
		Super.Possess(aPawn, bVehicleTransition);
		Pawn.SetMovementPhysics();
		EnemyPawn = OLEnemyPawn(aPawn);
	}
}

simulated function AddToAIGroup()
{
	local OLBot Bot;
	local OLAIGroup NewGroup;

	foreach WorldInfo.AllControllers(class'OLBot', Bot)
	{
		if (Bot != self && Bot.Group != None)
		{
			NewGroup = Bot.Group;
			break;
		}
	}

	if (NewGroup == None)
	{
		NewGroup = Spawn(class'OLAIGroup');
	}

	NewGroup.AddBot(self);
}

function UnPossess()
{
	Super.UnPossess();

	Destroy();
}

event bool QueueAICommand( GameAICommand Cmd )
{
	if (QueuedCommand == None)
	{
		QueuedCommand = Cmd;

		return true;
	}

	return false;
}

function PushQueuedCommand()
{
	if (QueuedCommand != None)
	{
		PushCommand(QueuedCommand);
		QueuedCommand = None;
	}
}

function PlayFullBodyAnim
(
	name	AnimName,
	float	Rate,
	optional	float	BlendInTime,
	optional	float	BlendOutTime,
	optional	bool	bLooping,
	optional	float	StartTime,
	optional	float	EndTime
)
{
	if (EnemyPawn != None)
	{
		EnemyPawn.PlayFullBodyAnim(AnimName, Rate, BlendInTime, BlendOutTime, bLooping, StartTime, EndTime);
	}
}

native final latent function WaitForFullBodyAnim();

native final latent function WaitForSpecialMove(optional bool bNoDelay);

native function bool IsPerformingMoveAbility();

native function bool PerformGrabCheck();
native function bool PerformAttackCheck();

event TurnTo( rotator Direction )
{
	local GameAICommand MoveCmd;

	if (!IsInState('Turning'))
	{
		MoveCmd = FindCommandOfClass(class'OLAICmd_MoveAbility');
		if (MoveCmd != None)
		{
			if (MoveCmd.IsInState('Approaching') || MoveCmd.IsInState('Waiting'))
			{
				AbortCommand(MoveCmd);
			}
			else
			{
				return;
			}
		}

		TurnToDirection = Direction;

		GotoState('Turning', , true);
	}
}

function bool CompareAnimations(AnimationData AnimDataOne, AnimationData AnimDataTwo)
{
	if (AnimDataOne.AnimationName == AnimDataTwo.AnimationName
		&& AnimDataOne.bLoop == AnimDataTwo.bLoop
		&& AnimDataOne.bOnWaypoint == AnimDataTwo.bOnWaypoint
		&& AnimDataOne.Rate == AnimDataTwo.Rate
		&& AnimDataOne.BlendInTime == AnimDataTwo.BlendInTime
		&& AnimDataOne.BlendOutTime == AnimDataTwo.BlendOutTime
		&& AnimDataOne.StartTime == AnimDataTwo.StartTime
		&& AnimDataOne.EndTime == AnimDataTwo.EndTime
		&& AnimDataOne.bAlign == AnimDataTwo.bAlign
		&& AnimDataOne.AlignLocation == AnimDataTwo.AlignLocation
		&& AnimDataOne.AlignRotation == AnimDataTwo.AlignRotation)
	{
		return true;
	}

	return false;
}

function ClearAnimation()
{
	CurrentAnimation.AnimationName = '';
}

event StartAnimating(	AnimationData aAnim,
						rotator Direction )
{
	local GameAICommand MoveCmd;

	if (aAnim.AnimationName == '' || CompareAnimations(CurrentAnimation, aAnim))
	{
		return;
	}

	if (bInvestigatingObject || bAttacking)
	{
		return;
	}

	MoveCmd = FindCommandOfClass(class'OLAICmd_MoveAbility');
	if (MoveCmd != None)
	{
		if (MoveCmd.IsInState('Approaching') || MoveCmd.IsInState('Waiting'))
		{
			AbortCommand(MoveCmd);
		}
		else
		{
			return;
		}
	}

	CurrentAnimation = aAnim;
	TurnToDirection = Direction;

	GotoState('Animating', , true);
}

event TriggerDisturbed()
{
	local GameAICommand MoveCmd;

	MoveCmd = FindCommandOfClass(class'OLAICmd_MoveAbility');
	if (MoveCmd != None)
	{
		if (MoveCmd.IsInState('Approaching') || MoveCmd.IsInState('Waiting'))
		{
			AbortCommand(MoveCmd);
		}
		else
		{
			return;
		}
	}
	
	if (bInvestigatingObject || bAttacking || bDisturbed)
	{
		return;
	}

	GotoState('Disturbed', , true);
}

native function UpdateAnimationMode();

native function bool AttackTarget(AIAttackType aType);

function AttackCycleEnd()
{
	bAttackCycling = false;
}

native function bool TryGrabNormal(bool bCrouched);

native function bool TryGrabUnder();

native function bool InvestigateObject(Actor anActor);

native function bool IsInAttackRange();
native function bool IsInApproachAttackRange(); // rcharpentier - IsInAttackRange with some extra padding
native function bool IsInFinalAttackRange(); // rcharpentier - same but with more padding, to account for moving targets

native function bool IsInDamageRange();

event DamageTarget()
{
	if (bTookDamage)
	{
		return;
	}

	if (bKilling)
	{
		TargetPlayer.TakeDamage(TargetPlayer.PreciseHealth + 1.f, self, EnemyPawn.Location, vect(0.f, 0.f, 0.f), EnemyPawn.InstantKillDmgType);
		
		bTookDamage = true;
	}
	else if (CurrentAttackType == AT_Normal)
	{
		if (IsInDamageRange())
		{
			TargetPlayer.TakeDamage(EnemyPawn.AttackNormalDamage, self, EnemyPawn.Location, vect(0.f, 0.f, 0.f), EnemyPawn.AttackNormalDmgType);
			TargetPlayer.ReactToHit(EnemyPawn.AttackNormalKnockbackPower, vector(EnemyPawn.Rotation)); // TODO: The attack direction should be the 2d vector from the enemy fist bone position to the hero root

			bTookDamage = true;
		}
	}
	else
	{
		TargetPlayer.TakeDamage(EnemyPawn.AttackThrowDamage, self, EnemyPawn.Location, vect(0.f, 0.f, 0.f), EnemyPawn.AttackThrowDmgType);

		bTookDamage = true;
	}
}

event DamageTargetRangeStartNotify()
{
	bDamageTargetTicking = true;
}

event DamageTargetRangeTickNotify()
{
	if (bDamageTargetTicking && IsInDamageRange() && !bTookDamage)
	{
		TargetPlayer.TakeDamage(EnemyPawn.AttackNormalDamage, self, EnemyPawn.Location, vect(0.f, 0.f, 0.f), EnemyPawn.AttackNormalDmgType);
		TargetPlayer.ReactToHit(EnemyPawn.AttackNormalKnockbackPower, vector(EnemyPawn.Rotation)); // TODO: The attack direction should be the 2d vector from the enemy fist bone position to the hero root
		
		bDamageTargetTicking = false;
		bTookDamage = true;
	}
}

event BashDoorNotify()
{
	local OLAICmd_MoveAbility_Door DoorCmd;

	DoorCmd = OLAICmd_MoveAbility_Door(GetActiveCommand());

	if (ActiveDoor != None && DoorCmd != None)
	{
		ActiveDoor.BashDoor(DoorCmd.bReversed);
	}
}

event BreakDoorNotify()
{
	local OLAICmd_MoveAbility_Door DoorCmd;

	DoorCmd = OLAICmd_MoveAbility_Door(GetActiveCommand());

	if (ActiveDoor != None && DoorCmd != None)
	{
		ActiveDoor.BreakDoor(EnemyPawn, DoorCmd.bReversed);
	}
}

event KnockbackStartNotify()
{
	bKnockedBack = false;
}

native function bool IsPlayerOnBrokenSideOfDoor(OLDoor Door);
native function bool IsPlayerOnVaultingPath();

event KnockbackTickNotify()
{
	local float Damage;
	local float KnockbackPower;
	local bool bForceFullPower;

	if (!bKnockedBack && IsInDamageRange())
	{
		bForceFullPower = false;

		Damage = EnemyPawn.AttackNormalDamage;
		KnockbackPower = EnemyPawn.AttackNormalKnockbackPower;

		if (GetActiveCommand().IsA('OLAICmd_MoveAbility_Door'))
		{
			if (!IsPlayerOnBrokenSideOfDoor(ActiveDoor))
			{
				return;
			}

			Damage = EnemyPawn.DoorBashDamage;
			KnockbackPower = EnemyPawn.DoorBashKnockbackPower;
		}
		else if (GetActiveCommand().IsA('OLAICmd_MoveAbility_Vault'))
		{
			if (!PerformAttackCheck() || !IsPlayerOnVaultingPath())
			{
				return;
			}

			bForceFullPower = true;
			Damage = EnemyPawn.VaultDamage;
			KnockbackPower = EnemyPawn.VaultKnockbackPower;
		}
		else if (EnemyPawn.bUsedByMatinee)
		{
			// rcharpentier - using the vault number, as the current case if a matinee-driven vault
			bForceFullPower = true;
			Damage = EnemyPawn.VaultDamage;
			KnockbackPower = EnemyPawn.VaultKnockbackPower;
		}

		TargetPlayer.TakeDamage(Damage, self, EnemyPawn.Location, vect(0.f, 0.f, 0.f), EnemyPawn.AttackNormalDmgType);
		TargetPlayer.ReactToHit(KnockbackPower, vector(EnemyPawn.Rotation), bForceFullPower); // TODO: The attack direction should be the 2d vector from the enemy fist bone position to the hero root
		bKnockedBack = true;
	}
}

event PushNotify()
{
	TargetPlayer.ReactToHit(EnemyPawn.AttackPushKnockbackPower, Normal2D(TargetPlayer.Location - EnemyPawn.Location));
}

event Recalculate(optional bool bForce)
{
	`AILog("OLBT: Recalculate");

	ShouldRecalculate = true;
	bForceRecalculate = bForce;
}

/**
 * Pathfinding Methods
 */
function AddBasePathContraints()
{
	// Clear cache and constraints (ignore recycling for the moment)
	NavigationHandle.ClearConstraints();
}

event bool GeneratePathToActor( Actor Goal, optional float WithinDistance, optional bool bAllowPartialPath )
{
	if (NavigationHandle == None)
	{
		return false;
	}

	AddBasePathContraints();

	class'NavMeshPath_Toward'.static.TowardGoal(NavigationHandle, Goal);
	class'OLNavMeshPath_SimilarToLastPath'.static.SimilarToLastPath(NavigationHandle);

	class'NavMeshGoal_At'.static.AtActor(NavigationHandle, Goal, WithinDistance, bAllowPartialPath);

	return NavigationHandle.FindPath();
}

event bool GeneratePathToLocation( Vector Goal, optional float WithinDistance, optional bool bAllowPartialPath )
{
	if (NavigationHandle == None)
	{
		return false;
	}

	AddBasePathContraints();

	class'NavMeshPath_Toward'.static.TowardPoint(NavigationHandle, Goal);
	class'OLNavMeshPath_SimilarToLastPath'.static.SimilarToLastPath(NavigationHandle);

	class'NavMeshGoal_At'.static.AtLocation(NavigationHandle, Goal, WithinDistance, bAllowPartialPath);

	return NavigationHandle.FindPath();
}

event NotifyPathChanged()
{
	local OLAICmd_MoveAbility MoveCmd;

	if (IsInState('Moving'))
	{
		MoveCmd = FindCommandOfClass(class'OLAICmd_MoveAbility_Door');
		if (MoveCmd != None)
		{
			if (MoveCmd.IsInState('Waiting') && MoveCmd.bAtWaitPoint)
			{	
				WaitForMoveTime = MoveCmd.GetWaitIndex() * (BehaviorState == AIBS_Chasing ? EnemyPawn.WaitLeaveChaseMultiplier : EnemyPawn.WaitLeaveNormalMultiplier);

				AbortCommand(MoveCmd);

				GotoState('WaitForMove', , true);

				return;
			}
		}

		RegeneratePath();
	}
}

native function RegeneratePath();

event bool NotifyBump(Actor Other, Vector HitNormal)
{
	local bool bIsPasser;
	local OLEnemyPawn otherPawn;

	otherPawn = OLEnemyPawn(Other);
	if (otherPawn != None)
	{
		if (!EnemyPawn.IsInState('Avoiding') && !otherPawn.IsInState('Avoiding') && NoZDot(EnemyPawn.Velocity, otherPawn.Velocity) < 0.f)
		{
			bIsPasser = true;
			if (BehaviorState < otherPawn.Bot.BehaviorState)
			{
				bIsPasser = false;
			}
			else if (BehaviorState == otherPawn.Bot.BehaviorState)
			{
				if (NoZDot(EnemyPawn.Velocity, Normal2D(TargetPlayer.Location - EnemyPawn.Location)) < 
					NoZDot(otherPawn.Velocity, Normal2D(TargetPlayer.Location - otherPawn.Location)))
				{
					bIsPasser = false;
				}
			}

			if (bIsPasser && otherPawn.bUseAvoidSystem)
			{
				otherPawn.StartAvoid(EnemyPawn);
			}
			else if (EnemyPawn.bUseAvoidSystem)
			{
				EnemyPawn.StartAvoid(otherPawn);
			}
		}
		
		if( !EnemyPawn.HasRegisteredNavMeshRecently() &&
			IsZero(Pawn.Velocity) && IsZero(Pawn.Acceleration)
			&& !bAvoiding && !otherPawn.Bot.bAvoiding )
		{
			`AILog("Registering obstacle around ourselves so "@otherPawn@"can get by");
			EnemyPawn.RegisterNavMeshObstacle();

			if (Group != None)
			{
				Group.NotifyOthersPathChanged(self);
			}
			else
			{
				otherPawn.Bot.NotifyPathChanged();
			}

			return true;
		}
		// if we're trying to stepaside for a stationary infantry pawn register an obstacle around them instead
		else if( !otherPawn.HasRegisteredNavMeshRecently() && 
			IsZero(otherPawn.Velocity) && IsZero(otherPawn.Acceleration)
			&& !bAvoiding && !otherPawn.Bot.bAvoiding )
		{
			`AILog("Registering obstacle around "$otherPawn$" because they are not moving.");
			otherPawn.RegisterNavMeshObstacle();

			if (Group != None)
			{
				Group.NotifyOthersPathChanged(otherPawn.Bot);
			}
			else
			{
				NotifyPathChanged();
			}

			return true;
		}
	}

	return false;
}

/**
 * Picks the next waypoint or if it finishes then clears the latent action.
 */
event bool ChooseNextPatrolWaypoint()
{
	local byte bReverseDirection;
	local int NewIndex;

	local SeqAct_Latent Action;

	if (NextPatrolRouteIndex != -1)
	{
		PatrolRouteIndex = NextPatrolRouteIndex;
		NextPatrolRouteIndex = -1;
	}
	else if (PatrolRoute.RouteType == ERT_Random)
	{
		NewIndex = Rand(PatrolRoute.RouteList.length-1);

		if (NewIndex >= PatrolRouteIndex)
		{
			NewIndex++;
		}

		PatrolRouteIndex = NewIndex;
	}
	else
	{
		if (bReversePatrolRoute)
		{
			--PatrolRouteIndex;
		}
		else
		{
			++PatrolRouteIndex;
		}

		bReverseDirection = 0;
		PatrolRouteIndex = PatrolRoute.ResolveRouteIndex(PatrolRouteIndex, bReversePatrolRoute ? ERD_Reverse : ERD_Forward, bPatrolComplete, bReverseDirection );

		if (bReverseDirection != 0)
		{
			bReversePatrolRoute = !bReversePatrolRoute;
		}

		if (bPatrolComplete != 0)
		{
			PatrolRoute = None;
			PatrolRouteIndex = 0;
			bPatrolComplete = 0;
			bReversePatrolRoute = false;

			`AILog("Patrol Completed", 'Patrol');
			`AILog("Actions: ", 'Patrol');

			foreach LatentActions(Action)
			{
				if (Action.IsA('OLSeqAct_AIStartPatrol'))
				{
					`AILog("  " $ Action.Name, 'Patrol');
				}
			}

			ClearLatentAction(class'OLSeqAct_AIStartPatrol');

			return false;
		}
	}

	return true;
}

event bool ChooseClosestPatrolWaypoint()
{
	if (PatrolRoute == None)
	{
		return false;
	}

	PatrolRouteIndex = PatrolRoute.MoveOntoRoutePath( EnemyPawn, bReversePatrolRoute ? ERD_Reverse : ERD_Forward );
	
	return true;
}

native function OLWaypoint GetCurrentWaypoint();

event bool CompareMoves(MovementData data1, MovementData data2)
{
	local bool Success;
	Success = false;

	if ((data1.Type == data2.Type)
		&& (data1.bIsDynamic == data2.bIsDynamic)
		&& (data1.DestinationBuffer == data2.DestinationBuffer))
	{
		switch(data1.Type)
		{
		case MT_Point:
			Success = (data1.DestinationPoint == data2.DestinationPoint);
			break;
		case MT_Actor:
			Success = (data1.DestinationActor == data2.DestinationActor);
			break;
		}
	}

	return Success;
}

function bool IsAlreadyAtDestination(MovementData aData)
{
	local vector Destination;

	if (aData.Type == MT_Actor)
	{
		Destination = aData.DestinationActor.Location;
	}
	else
	{
		Destination = aData.DestinationPoint;
	}
	
	return (VSizeSq2D(Destination - EnemyPawn.Location) <= Square(aData.DestinationBuffer + 10.0f)) && Abs(Destination.Z - EnemyPawn.Location.Z) < EnemyPawn.CylinderComponent.CollisionHeight;
}

event bool StartMove(MovementData aData)
{
	local OLAICmd_MoveAbility MoveCmd;

	if (!CompareMoves(CurrentMove, aData))
	{
		if (IsAlreadyAtDestination(aData))
		{
			`AILog("Already At Destination", 'Movement');

			return true;
		}
		else
		{
			MoveCmd = FindCommandOfClass(class'OLAICmd_MoveAbility');
			if (MoveCmd != None)
			{

				if (MoveCmd.IsInState('Waiting'))
				{	
					WaitForMoveTime = MoveCmd.GetWaitIndex() * (BehaviorState == AIBS_Chasing ? EnemyPawn.WaitLeaveChaseMultiplier : EnemyPawn.WaitLeaveNormalMultiplier);

					AbortCommand(MoveCmd);

					GotoState('WaitForMove', , true);
				}
				else if (MoveCmd.IsInState('Approaching'))
				{
					AbortCommand(MoveCmd);
				}
				else
				{
					NextMove = aData;
					return false;
				}
			}

			if (bInvestigatingObject)
			{
				NextMove = aData;
				CurrentMoveStatus = MS_Pending;
				return false;
			}

			if (bDisturbed)
			{
				EnemyPawn.CancelSpecialMove();
			}

			GotoState('Moving', , true);
			CurrentMove = aData;			
		}
	}
	else
	{
		ClearNextMove();
	}

	return false;
}

function bool GetClosestPointToActor(out vector NewPoint, Actor ActorToCheck, out float NewBuffer)
{
	local bool Success;
	local OLHero HeroActor;
	local OLPawn PawnActor;

	Success = true;
	HeroActor = OLHero(ActorToCheck);
	PawnActor = OLPawn(ActorToCheck);
	if (HeroActor != None)
	{
		if (HeroActor.LocomotionMode == LM_Squeeze)
		{
			NewPoint = GetSqueezePoint(HeroActor.ActiveSqueeze);

			NewPoint.Z = HeroActor.Location.Z;
			NewBuffer = 0.0f;
		}
		else if (((HeroActor.LocomotionMode == LM_SpecialMove && HeroActor.SpecialMove == SMT_EnterLocker) || HeroActor.LocomotionMode == LM_Locker) && HeroActor.ActiveLocker != None)
		{
			NewPoint = HeroActor.ActiveLocker.AssociatedDoor.Location - vector(HeroActor.ActiveLocker.AssociatedDoor.Rotation)*120.0;
			NewBuffer = 0.0f;
		}
		else if (HeroActor.CanBeGrabbedUnder())
		{
			NewPoint = HeroActor.GetGrabUnderDestination(EnemyPawn);
		}
		else
		{
			NewPoint = HeroActor.GetFutureDestination(EnemyPawn);
		}
	}
	else if (PawnActor != None)
	{
		NewPoint = PawnActor.GetFutureDestination(EnemyPawn);
	}
	else
	{
		NewPoint = ActorToCheck.Location;
	}

	if (!NavigationHandle.PointReachable(NewPoint))
	{
		if (!GetClosestPointOnNavMesh(NewPoint, NewPoint))
		{
			Success = false;
		}
	}

	return Success;
}

function bool TargetReachable()
{
	local vector Point;
	local float Buffer;

	Point = TargetPlayer.Location;
	Buffer = 0.f;

	return GetClosestPointToActor(Point, TargetPlayer, Buffer);
}

/** Stops the player immediately, breaking out of the Moving state. */
native function StopMoving(optional bool bAborted);

event StartWaitForDoor()
{
	GotoState('WaitForDoor', , true);
}

/** Clears the CurrentMove values */
event ClearCurrentMove()
{
	local OLHero HeroActor;

	HeroActor = OLHero(CurrentMove.DestinationActor);
	if (CurrentMove.Type == MT_Actor && HeroActor != None)
	{
		HeroActor.RemoveEnemyFromAIPositionPawns(EnemyPawn);
	}

	CurrentMove.DestinationActor = None;
	CurrentMove.DestinationPoint = vect(0,0,0);
	CurrentMove.ValidatedMovePoint = vect(0,0,0);
	CurrentMove.Type = MT_Invalid;
	CurrentMove.DestinationBuffer = 0.f;
	CurrentMove.bIsDynamic = false;
	CurrentMove.bIsInvestigation = false;
}

function ClearNextMove()
{
	NextMove.DestinationActor = None;
	NextMove.DestinationPoint = vect(0,0,0);
	NextMove.ValidatedMovePoint = vect(0,0,0);
	NextMove.Type = MT_Invalid;
	NextMove.bIsDynamic = false;
	NextMove.DestinationBuffer = 0.f;
	NextMove.bIsInvestigation = false;
}

native latent function MoveAlongPath(array<vector> PathPoints, optional Actor FocusTarget);

native function bool GetClosestPointOnNavMesh(out vector out_NewPoint, Vector PointToCheck);

native function vector GetSqueezePoint(OLSqueezeVolume Squeeze);

native function StartDoorTraversal( bool bReversed );

native function EndDoorTraversal();

native function bool GetDoorApproachPoint(OLDoor aDoor, out vector ApproachPoint);

native function bool IsAtTrimmedDoor();

native function LostTarget(bool bAfterChase);

native function TriggerAudioDisturbance(vector DisturbanceLocation, optional bool bFromGroup, optional bool bNewInvestigate, optional bool bNoTrim, optional bool bIgnoreDisturbs);
native function TriggerVisualDisturbance(vector DisturbanceLocation, optional bool bFromGroup);

native function ClearInvestigation();

event ClearDisturbance()
{
	ClearInvestigation();

	VisualDisturbance.TimeSinceUpdate = -1.0f;
	AudioDisturbance.TimeSinceUpdate = -1.0f;
	bNewDisturbance = false;

	LostTarget(bWasChasing);

	bWasChasing = false;
}

function ClearDestination()
{
	SetDestinationPosition(vect(0,0,0));
}

/** Debug Info */

event DebugMessagePlayer( coerce String Msg )
{
`if(`notdefined(FINAL_RELEASE))
	local PlayerController PC;
	local OLGame TheGame;

	TheGame = OLGame(class'WorldInfo'.static.GetWorldInfo().Game);

	if (TheGame != None && !TheGame.IsDemo())
	{
		`log( "!!!!"@Msg@"!!!!" );
		ScriptTrace();

		foreach LocalPlayerControllers(class'PlayerController', PC)
		{
			PC.ClientMessage( Msg );
		}
	}
`endif
}

static native function DrawDebugStates(Object anObject, Canvas aCanvas, float XL, float XPos, out float out_YL, out float out_YPos);

simulated function DisplayDebug(HUD HUD, out float out_YL, out float out_YPos)
{
	local GameAICommand ActiveCmd;
	local int I;
	local array<AnimNode> visitedAnimNodes;
	local string buffer;

	// Pawn
	HUD.Canvas.SetDrawColor(255,86,26);
	HUD.Canvas.SetPos(8,out_YPos);
	HUD.Canvas.DrawText("Enemy Mode: " $ EnemyPawn.EnemyMode);
	out_YPos += out_YL;
	HUD.Canvas.DrawText("Location: " $ EnemyPawn.Location $ ", Rotation: " $ EnemyPawn.Rotation);
	out_YPos += out_YL;
	HUD.Canvas.DrawText("Vel: " $ VSize(EnemyPawn.Velocity) $ ", Acc: " $ VSize(EnemyPawn.Acceleration) $ ", GndSpd: " $ EnemyPawn.GroundSpeed $ ", (tgt: " $ EnemyPawn.MoveSpeed_Target $ " - " $ EnemyPawn.MoveSpeedMode $ ")");
	out_YPos += out_YL;
	HUD.Canvas.DrawText("SpecialMove: " $ EnemyPawn.SpecialMove $ ", In Matinee: " $ EnemyPawn.bUsedByMatinee);
	out_YPos += out_YL;	
	HUD.Canvas.DrawText("RotMode: " $ EnemyPawn.RotationMode $ ", DRot.Locked: " $ EnemyPawn.IsDesiredRotationLocked() $ ", DRot.Used: " $ EnemyPawn.IsDesiredRotationInUse() $ ", TYaw: " $ EnemyPawn.TargetYaw $ ", TVel.Yaw: " $ (rotator(EnemyPawn.TargetVelocity).Yaw * 45.0/8192.0));
	out_YPos += out_YL;	


	// Bot
	HUD.Canvas.SetDrawColor(26,86,255);
	HUD.Canvas.SetPos(8,out_YPos);
	HUD.Canvas.DrawText("Behavior State: " $ BehaviorState);
	out_YPos += out_YL;
	HUD.Canvas.DrawText("Attack Timer: " $ (Group != None) ? Group.AttackTimer : AttackTimer);
	out_YPos += out_YL;
	HUD.Canvas.DrawText("In Darkness - self: " $ bInDarkness $ ", target: " $ bTargetInDarkness $ ", In Electricity: " $ bInElectricity);
	out_YPos += out_YL;	

	if (PatrolRoute != None)
	{
		HUD.Canvas.DrawText("Next Patrol Point: " $ (PatrolRoute.RouteList[PatrolRouteIndex].Actor.Location));
		out_YPos += out_YL;
	}

	// Disturbances
	HUD.Canvas.SetDrawColor(0,255,255);
	HUD.Canvas.SetPos(8,out_YPos);
	HUD.Canvas.DrawText("Disturbances: Audio " $ AudioDisturbance.TimeSinceUpdate $ " [" $ AudioDisturbance.Location $ "], Vis " $ VisualDisturbance.TimeSinceUpdate $ " [" $ VisualDisturbance.Location $ "]");
	out_YPos += out_YL;	

	// Sight
	SightComponent.DisplayDebug(HUD, out_YL, out_YPos);
	out_YPos += out_YL;

	HUD.Canvas.SetDrawColor(0,100,255);

	// Hearing
	HUD.Canvas.SetPos(8,out_YPos);
	HUD.Canvas.DrawText("Hearing - Noise Value: " $ CurrentNoiseValue $ ", Time Since Noise: " $ TimeSinceNoise);
	out_YPos += out_YL;

	HUD.Canvas.SetPos(8,out_YPos);
	HUD.Canvas.DrawText("        - Distance: " $ DebugLastDistance $ ", Loudness: " $ DebugLastLoudness $ ", Occlusion: " $ DebugLastOcclusion);
	out_YPos += out_YL;

	// Bot State
	HUD.Canvas.SetDrawColor(128, 232, 255);
	DrawDebugStates(self, HUD.Canvas, 4.0f, 8.0f, out_YL, out_YPos);

	// Command
	HUD.Canvas.SetDrawColor(238,114,238);
	HUD.Canvas.SetPos(8,out_YPos);
	
	ActiveCmd = GetActiveCommand();
	if (ActiveCmd != None)
	{
		HUD.Canvas.DrawText("Command: " $ string(ActiveCmd.Class));
		out_YPos += out_YL;
		HUD.Canvas.SetPos(12, out_YPos);
		DrawDebugStates(ActiveCmd, HUD.Canvas, 4.0f, 16.0f, out_YL, out_YPos);
	}
	else
	{
		HUD.Canvas.DrawText("Command: None");
		out_YPos += out_YL;
	}

	// Group
	if (Group != None)
	{
		buffer = "Group: ";

		for (I = 0; I < Group.Bots.length; ++I)
		{
			if (I == 0)
			{
				buffer = buffer $ Group.Bots[I].EnemyPawn.GetDebugName();
			}
			else
			{
				buffer = buffer $ ", " $ Group.Bots[I].EnemyPawn.GetDebugName();
			}
		}

		HUD.Canvas.SetPos(8, out_YPos);
		HUD.Canvas.DrawText(buffer);
		out_YPos += out_YL;
	}

	// Behaviors
	HUD.Canvas.SetDrawColor(242, 73, 95);
	HUD.Canvas.SetPos(8, out_YPos);
	RootBehavior.Task.DisplayDebug(HUD.Canvas, 4.0f, 12.0f, out_YL, out_YPos);
	out_YPos += out_YL;

	// Animation
	visitedAnimNodes.Length = 0;
	EnemyPawn.DrawDebugAnimNode('', EnemyPawn.Mesh.Animations, visitedAnimNodes, HUD.Canvas, out_YL, out_YPos, 13.0f, 8.0f, 0.0f);
}

/**
 * Scripting hook to start a patrol route.
 */
function OnAIStartPatrol(OLSeqAct_AIStartPatrol Action)
{
	// Start Patrol
	if (Action.InputLinks[0].bHasImpulse)
	{
		//// abort any previous latent moves
		ClearLatentAction(class'OLSeqAct_AIStartPatrol',true,Action);

		`AILog("Patrol Started: " $ Action.Name, 'Patrol');

		bPatrolToPlayer = Action.FollowPlayer;
		PatrolToPlayerDistance = Action.FollowDistance;
		PatrolToPlayerUpdateRate = Action.FollowUpdateRate;
		PatrolToPlayerLastLocation = TargetPlayer.Location;
		PatrolToPlayerLastUpdated = 0.0f;

		PatrolRoute = Action.PatrolRoute;
		PatrolMode = Action.PatrolMode;

		NextPatrolRouteIndex = -1;

		// Make sure no one accidentally sets it to Squeeze Left or Right.
		if (PatrolMode == EM_SqueezeGrabRight || PatrolMode == EM_SqueezeGrabLeft)
		{
			`warn("Bat PatrolMode on StartPatrol.");
			PatrolMode = EM_Patrol;
		}

		if (!bPatrolToPlayer)
		{
			if (PatrolRoute != None)
			{
				if (PatrolRoute.RouteList.length == 0)
				{
					`warn("Invalid route with empty MoveList for patrol");
					PatrolRoute = None;
				}
				else
				{
					if (Action.bStartAtClosestPoint)
					{
						PatrolRouteIndex = PatrolRoute.MoveOntoRoutePath(Pawn);
					}
					else if (PatrolRoute.RouteType == ERT_Random)
					{
						PatrolRouteIndex = Rand(PatrolRoute.RouteList.length-1);
					}
					else
					{
						PatrolRouteIndex = 0;
					}

					bReversePatrolRoute=false;
					bPatrolComplete=0;

					Recalculate();
				}
			}
			else
			{
				`warn("No route set for patrol.");
			}
		}
	}
	// Goto Target
	else if (Action.InputLinks[1].bHasImpulse)
	{
		NextPatrolRouteIndex = PatrolRoute.MoveOntoRoutePath(TargetPlayer);
	}
}

event StopPatrol( optional bool bAbort )
{
	`AILog("Stopping Patrol: Aborted? " $ bAbort, 'Patrol');

	StopMoving(true);

	PatrolRoute = None;
	PatrolRouteIndex = 0;
	bPatrolComplete = 0;
	bReversePatrolRoute = false;
	bPatrolToPlayer = false;

	ClearLatentAction(class'OLSeqAct_AIStartPatrol', bAbort);
}

function OnAIAbortPatrol(OLSeqAct_AIAbortPatrol Action)
{
	StopPatrol(false);
}

function OnToggleAIIgnorePlayer(OLSeqAct_ToggleAIIgnorePlayer Action)
{
	if (Group != None && Action.bSetOnGroup)
	{
		Group.ToggleAIIgnorePlayer(Action.InputLinks[0].bHasImpulse);
	}
	else
	{
		ToggleAIIgnorePlayer(Action.InputLinks[0].bHasImpulse);
	}
}

event ToggleAIIgnorePlayer(bool bEnable)
{
	if (bEnable)
	{
		EnemyPawn.Modifiers.bShouldAttack = false;

		VisualDisturbance.TimeSinceUpdate = -1.0f;
		AudioDisturbance.TimeSinceUpdate = -1.0f;

		SightComponent.bIgnoreTarget = true;
	}
	else
	{
		EnemyPawn.Modifiers.bShouldAttack = true;

		SightComponent.bIgnoreTarget = false;
	}

	Recalculate();
}

event IgnoreTarget(FLOAT Time)
{
	ToggleAIIgnorePlayer(true);

	IgnoreTimer = Time;
}

function OnToggleAIAlwaysSeePlayer(OLSeqAct_ToggleAIAlwaysSeePlayer Action)
{
	if (Group != None)
	{
		Group.ToggleAIAlwaysSeePlayer(Action.InputLinks[0].bHasImpulse);
	}
	else
	{
		ToggleAIAlwaysSeePlayer(Action.InputLinks[0].bHasImpulse);
	}
}

function ToggleAIAlwaysSeePlayer(bool bEnable)
{
	if (bEnable)
	{
		SightComponent.bAlwaysSeeTarget = true;
	}
	else
	{
		SightComponent.bAlwaysSeeTarget = false;
		SightComponent.bWasAlwaysSeeTarget = true;
	}

	Recalculate();
}

function OnToggleAIAttackOnProximity(OLSeqAct_ToggleAIAttackOnProximity Action)
{
	ToggleAIAttackOnProximity(Action.InputLinks[0].bHasImpulse);
}

event ToggleAIAttackOnProximity(bool bEnable)
{
	if (bEnable)
	{
		EnemyPawn.Modifiers.bAttackOnProximity = true;
		SightComponent.bIgnoreTarget = false;
	}
	else
	{
		EnemyPawn.Modifiers.bAttackOnProximity = false;
	}

	Recalculate();
}


function OnAIInvestigatePoint(OLSeqAct_AIInvestigatePoint Action)
{
	if (Action.Point != None && EnemyPawn.Modifiers.bShouldAttack)
	{
		ClearInvestigation();
		
		TriggerAudioDisturbance(Action.Point.Location, false, true, true);

		if (Action.bStartChasing)
		{
			BehaviorState = AIBS_Chasing;
			UpdateAnimationMode();
		}

		Recalculate();
	}
}

//function OnAIGroup(OLSeqAct_AIGroup Action)
//{
//	if (Action.InputLinks[0].bHasImpulse)
//	{
//		if (Action.AIGroup != None)
//		{
//			Action.AIGroup.AddBot(self);
//		}
//	}
//	else
//	{
//		if (Action.AIGroup != None)
//		{
//			Action.AIGroup.RemoveBot(self);
//		}
//	}
//}

simulated function BeginMatinee()
{
	Recalculate();
}

simulated function FinishMatinee()
{
	Recalculate();
}

/*============================================================================*/

/**
 * State Code
 */

/** AI standing still waiting for orders. */
auto state Idle
{
	function BeginState(Name PreviousStateName)
	{
		OLNavHandle.ClearPath();
		EnemyPawn.StartIdleSound();
	}

Begin:
	`AILog("Idle", 'AIState');

	Sleep(0.5);
	Goto('Begin');
}

/** Moves AI to DestinationPoint or DestinationActor then returns to Idle */
state Moving
{
	function BeginState(Name PreviousStateName)
	{
		CurrentMoveStatus = MS_Moving;
		LastMoveFailedReason = MFR_Unknown;
		EnemyPawn.StartIdleSound();
	}

	function EndState(Name NextStateName)
	{
		bReGeneratePath = FALSE;
		bFinishedDoor = false;
		ClearCurrentMove();
	}

	function bool GeneratePath()
	{
		local bool bGenerateSucceeded;

		NavigationHandle.SetFinalDestination(CurrentMove.ValidatedMovePoint);
		MoveLastLocation = CurrentMove.DestinationPoint;
		MoveTimeSinceLastPath = 0.0f;

		if (!NavigationHandle.PointReachable(CurrentMove.ValidatedMovePoint/*, Anchor*/) && VSizeSq(CurrentMove.ValidatedMovePoint - EnemyPawn.Location) > 30.f * 30.f)
		{
			bGenerateSucceeded = GeneratePathToLocation(CurrentMove.ValidatedMovePoint, CurrentMove.DestinationBuffer);

			if (bGenerateSucceeded)
			{
				`AILog("FindNavMeshPath returned TRUE", 'Movement');

				bTrimmedToDoor = false;
				// Trim the Path for the Current Situation
				if (CurrentMove.bIsInvestigation)
				{
					if (OLNavHandle.TrimPathToLastDoor( CurrentMove.ValidatedMovePoint ))
					{
						bTrimmedToDoor = true;
						`AILog("Investigation Trimmed to Door", 'Movement');
						return true;
					}
					else if (bFinishedDoor)
					{
						`AILog("Investigation Just FinishedDoor, Path Finished", 'Movement');
						CurrentMoveStatus = MS_Success;
						GotoState('Idle');
						return false;
					}
					else if (OLNavHandle.TrimPathByDistance( 500.0f, CurrentMove.ValidatedMovePoint ))
					{
						`AILog("Investigation Trimmed by Distance", 'Movement');
						return true;
					}
					else
					{
						`AILog("Investigation Failed to Trim, Path Finished", 'Movement');
						CurrentMoveStatus = MS_Success;
						GotoState('Idle');
						return false;
					}
				}
				else
				{
					MoveModifiedBufferDist = CurrentMove.DestinationBuffer - VSize(CurrentMove.DestinationPoint - CurrentMove.ValidatedMovePoint);
					if (MoveModifiedBufferDist < 0.f)
					{
						MoveModifiedBufferDist = 0.f;
					}

					OLNavHandle.TrimPathByDistance( MoveModifiedBufferDist, CurrentMove.ValidatedMovePoint, true );
					`AILog("Path Trimmed by " $ MoveModifiedBufferDist, 'Movement');
				}

				return true;
			}
			else
			{
				//give up because the nav mesh failed to find a path
				`AILog("FindNavMeshPath failed to find a path to" @ CurrentMove.ValidatedMovePoint, 'Movement');
				CurrentMoveStatus = MS_Failed;
				LastMoveFailedReason = MFR_NoPathToTarget;
				GotoState('Idle');

				return false;
			}
		}
		else if (CurrentMove.bIsInvestigation && bFinishedDoor)
		{
			`AILog("Investigation Just FinishedDoor, Path Finished", 'Movement');
			CurrentMoveStatus = MS_Success;
			GotoState('Idle');
			return false;
		}
		else
		{
			// Trim the Direct Line for the buffer
			TempTrimAmount = CurrentMove.DestinationBuffer;

			if (CurrentMove.bIsInvestigation)
			{
				TempTrimAmount = Max(TempTrimAmount, 500.0f);
			}

			MoveModifiedBufferDist = TempTrimAmount - VSize(CurrentMove.DestinationPoint - CurrentMove.ValidatedMovePoint);
			if (MoveModifiedBufferDist < 0.f)
			{
				MoveModifiedBufferDist = 0.f;
			}

			if (Square(MoveModifiedBufferDist) > VSizeSq(CurrentMove.ValidatedMovePoint - EnemyPawn.Location))
			{
				CurrentMove.ValidatedMovePoint = EnemyPawn.Location;
			}
			else
			{
				CurrentMove.ValidatedMovePoint -= Normal(CurrentMove.DestinationPoint - EnemyPawn.Location) * MoveModifiedBufferDist;
				`AILog("Path Trimmed by " $ MoveModifiedBufferDist, 'Movement');
			}
			
			return false;
		}
	}

	function bool ReachedDestination()
	{
		local bool bReachedDestination;

		if ( VSizeSq(CurrentMove.DestinationPoint - EnemyPawn.Location) < Square(CurrentMove.DestinationBuffer) )
		{
			bReachedDestination = true;
		}
		else
		{
			bReachedDestination = Pawn.ReachedPoint(CurrentMove.ValidatedMovePoint, None);
		}

		return bReachedDestination;
	}

Begin:
	`AILog("Moving", 'AIState');

	bReGeneratePath = FALSE;
	bMoveCancelled = FALSE;

	if (NextMove.Type != MT_Invalid)
	{
		CurrentMove = NextMove;
		ClearNextMove();
	}

	if (Pawn != None && CurrentMove.Type != MT_Invalid)
	{
		// Make sure the Bot is on the NavMesh
		if(!NavigationHandle.FindPylon())
		{
			if(!GetClosestPointOnNavMesh(MoveTempDest, EnemyPawn.Location))
			{
				DebugMessagePlayer(EnemyPawn $ " not on NavMesh!");

				CurrentMoveStatus = MS_Failed;
				LastMoveFailedReason = MFR_AINotOnNavMesh;

				GotoState('Idle');
			}
			else
			{
				MoveTo(MoveTempDest, None, -25.0f );

				if (bReGeneratePath)
				{
					bReGeneratePath = false;
					Goto('Begin');
				}
				else if (VSize2D(EnemyPawn.Location - MoveTempDest) > 10.0f)
				{
					DebugMessagePlayer(EnemyPawn $ " can't move to NavMesh. Teleporting.");

					EnemyPawn.StartSpecialMove(SMT_SlideToNavMesh, MoveTempDest, vect(0,0,0), APTT_TargetAtStart);
					WaitForSpecialMove();
					//EnemyPawn.SetLocation(MoveTempDest);
				}
			}
		}

		// Get the destination on the NavMesh
		if(CurrentMove.Type == MT_Actor)
		{
			if (bPatrolToPlayer && BehaviorState == AIBS_Patrolling)
			{
				CurrentMove.DestinationPoint = PatrolToPlayerLastLocation;
				if (!GetClosestPointOnNavMesh(CurrentMove.ValidatedMovePoint, CurrentMove.DestinationPoint))
				{
					`AILog("Failed to find End Position on NavMesh.", 'Movement');

					CurrentMoveStatus = MS_Failed;
					LastMoveFailedReason = MFR_TargetNotOnNavMesh;
					GotoState('Idle');
				}
			}
			else
			{
				CurrentMove.DestinationPoint = CurrentMove.DestinationActor.Location;
				if (!GetClosestPointToActor(CurrentMove.ValidatedMovePoint, CurrentMove.DestinationActor, CurrentMove.DestinationBuffer))
				{
					`AILog("Failed to find End Position on NavMesh.", 'Movement');

					if (!CurrentMove.bIsDynamic)
					{
						DebugMessagePlayer(EnemyPawn $ " destination not on NavMesh!");
					}

					CurrentMoveStatus = MS_Failed;
					LastMoveFailedReason = MFR_TargetNotOnNavMesh;
					GotoState('Idle');
				}
			}
		}
		else
		{
			if (NavigationHandle.PointReachable(CurrentMove.DestinationPoint))
			{
				CurrentMove.ValidatedMovePoint = CurrentMove.DestinationPoint; 
			}
			else if (!GetClosestPointOnNavMesh(CurrentMove.ValidatedMovePoint, CurrentMove.DestinationPoint))
			{
				`AILog("Failed to find End Position on NavMesh.", 'Movement');

				if (!CurrentMove.bIsDynamic)
				{
					DebugMessagePlayer(EnemyPawn $ " destination not on NavMesh!");
				}

				CurrentMoveStatus = MS_Failed;
				LastMoveFailedReason = MFR_TargetNotOnNavMesh;
				GotoState('Idle');
			}
		}

		// Generate the Path to the point.
		if (GeneratePath())
		{
			bFinishedDoor = false;

			while (Pawn != None && !ReachedDestination() && !bMoveCancelled)
			{
				// move to the first node on the path
				if( OLNavHandle.GetNextMovePath( MoveTempPath, 10.0f/*Pawn.GetCollisionRadius()*/) )
				{
					MoveTempDest = MoveTempPath[MoveTempPath.length-1];

					// suggest move preparation will return TRUE when the edge's
					// logic is getting the bot to the edge point
					// FALSE if we should run there ourselves
					if (!NavigationHandle.SuggestMovePreparation( MoveTempDest, self ))
					{
						if (MoveTempDest == CurrentMove.ValidatedMovePoint && CurrentMove.bFocusOnActor && CurrentMove.Type == MT_Actor)
						{
							MoveAlongPath(MoveTempPath, CurrentMove.DestinationActor);
						}
						else
						{
							MoveAlongPath(MoveTempPath);
						}
					}
					else if (QueuedCommand != None)
					{
						PushQueuedCommand();

						if (bReGeneratePath && bRegenerateWhilePerforming && IsAtTrimmedDoor())
						{
							bReGeneratePath = FALSE;
						}

						if (NextMove.Type != MT_Invalid)
						{
							CurrentMove = NextMove;
							ClearNextMove();
							Goto('Begin');
						}
					}
					else
					{
						`AILog("Bad Nav Point", 'Movement');

						MoveLastLocation = vect(0,0,0);
						Sleep(0.1f);
						Goto('Begin');
					}
				}
				else
				{
					`AILog("Bad Move Location.", 'Movement');

					MoveLastLocation = vect(0,0,0); // Force a Repath.
					Sleep(0.1f);
					Goto('Begin');
				}

				if (!ReachedDestination())
				{
					if (CurrentMove.bIsDynamic && CurrentMove.Type == MT_Actor && CurrentMove.DestinationActor.Location != MoveLastLocation)
					{
						Goto('Begin');
					}

					if (bReGeneratePath)
					{
						bReGeneratePath = false;
						Goto('Begin');
					}
				}
			}
		}
		else
		{
			bFinishedDoor = false;

			OLNavHandle.ClearPath();

			if (CurrentMove.bFocusOnActor && CurrentMove.Type == MT_Actor)
			{
				MoveTo(CurrentMove.ValidatedMovePoint, CurrentMove.DestinationActor, -20.0f);
			}
			else
			{
				MoveTo(CurrentMove.ValidatedMovePoint, , -20.0f);
			}
		}

		if (!ReachedDestination() && CurrentMove.bIsDynamic && CurrentMove.Type == MT_Actor && CurrentMove.DestinationActor.Location != MoveLastLocation)
		{
			Goto('Begin');
		}
	}

	if (ReachedDestination())
	{
		CurrentMoveStatus = MS_Success;
	}
	else
	{
		CurrentMoveStatus = MS_Failed;
		LastMoveFailedReason = MFR_Unknown;
	}

	GotoState('Idle');
}

state Turning
{
	function BeginState(Name PreviousStateName)
	{
		EnemyPawn.ZeroMovementVariables();
		ClearCurrentMove();
		OLNavHandle.ClearPath();
		ClearDestination();

		EnemyPawn.StartIdleSound();

		bTurning = true;
	}

	function EndState(Name NextStateName)
	{
		bTurning = false;
	}

Begin:
	`AILog("Turning", 'AIState');

	// Clear any rotation before starting attack.
	EnemyPawn.ResetDesiredRotation();
	SetFocalPoint(vect(0,0,0));

	EnemyPawn.TurnOnSpot(TurnToDirection);
	WaitForSpecialMove();

	EnemyPawn.SetDesiredRotation(TurnToDirection);
	if (!EnemyPawn.ReachedDesiredRotation())
	{
		FinishRotation();
	}

	GotoState('Idle');
}

state Animating
{
	function BeginState(Name PreviousStateName)
	{
		EnemyPawn.ZeroMovementVariables();
		ClearCurrentMove();
		OLNavHandle.ClearPath();
		ClearDestination();

		EnemyPawn.StartIdleSound();
	}

	function EndState(Name NextStateName)
	{
		local AnimNodeSequence CustomSequence;

		EnemyPawn.DisableRootMotion();

		CustomSequence = EnemyPawn.FullBodyAnimSlot.GetCustomAnimNodeSeq();

		if (CustomSequence != None && CustomSequence.AnimSeqName == CurrentAnimation.AnimationName)
		{
			EnemyPawn.FullBodyAnimSlot.StopCustomAnim(0.1f);
		}

		ClearAnimation();

		if (EnemyPawn.SpecialMove == SMT_AlignAnim)
		{
			EnemyPawn.CancelSpecialMove();
		}
		
		EnemyPawn.ResetDesiredRotation();
	}

Begin:
	`AILog("Animating", 'AIState');

	// Clear any rotation before starting attack.
	EnemyPawn.ResetDesiredRotation();
	SetFocalPoint(vect(0,0,0));

	if (RDiff(TurnToDirection, EnemyPawn.Rotation) > 0.001f)
	{
		EnemyPawn.TurnOnSpot(TurnToDirection);
		WaitForSpecialMove();

		EnemyPawn.SetDesiredRotation(TurnToDirection);
		if (!EnemyPawn.ReachedDesiredRotation())
		{
			FinishRotation();
		}
	}

	if (CurrentAnimation.bOnWaypoint)
	{
		GetCurrentWaypoint().AnimStartedEvent(EnemyPawn);
	}

	if (CurrentAnimation.bAlign)
	{
		EnemyPawn.StartSpecialMove(SMT_AlignAnim, CurrentAnimation.AlignLocation, CurrentAnimation.AlignRotation, APTT_TargetAtStart);
		WaitForSpecialMove();
	}

	EnemyPawn.ResetDesiredRotation();

	PlayFullBodyAnim(CurrentAnimation.AnimationName, 
						CurrentAnimation.Rate, 
						CurrentAnimation.BlendInTime, 
						CurrentAnimation.BlendOutTime, 
						CurrentAnimation.bLoop,
						CurrentAnimation.StartTime, 
						CurrentAnimation.EndTime);

	EnemyPawn.EnableRootMotion();

	WaitForFullBodyAnim();

	GotoState('Idle');
}

state Attacking
{
	function BeginState(Name PreviousStateName)
	{
		`AILog("Attacking", 'AIState');

		EnemyPawn.ZeroMovementVariables();
		ClearCurrentMove();

		OLNavHandle.ClearPath();
		ClearDestination();

		bAttacking = true;

		bTookDamage = false;

		EnemyPawn.StopIdleSound();

		if (Group != None)
		{
			Group.TakeAttackToken(self);
		}

		bEnableHeadTracking = false;
	}

	function EndState(Name NextStateName)
	{
		EnemyPawn.LockDesiredRotation(false);
		EnemyPawn.ResetDesiredRotation();

		bAttacking = false;
		bKilling = false;

		if (Group != None)
		{
			Group.ReturnAttackToken(self);
		}

		bEnableHeadTracking = true;
	}

	function ThrowPlayer()
	{
		EnemyPawn.ThrowRotation = ThrowPlayerRotation;
		EnemyPawn.StartSpecialMove(SMT_ThrowHero);
		TargetPlayer.TryThrowPlayer(EnemyPawn, ThrowPlayerRotation);
	}

	function bool TargetInSpecialLocation()
	{
		return TargetPlayer != None && (TargetPlayer.IsInLocker() || TargetPlayer.LocomotionMode == LM_Bed || TargetPlayer.LocomotionMode == LM_Squeeze);
	}
}

state AttackNormal extends Attacking
{
	function EndState(Name NextStateName)
	{
		Super.EndState(NextStateName);
		bUseQuickAttack = false;
	}

Begin:
	WaitForSpecialMove();

	if (EnemyPawn.bUsingWeapon && !EnemyPawn.bHasWeaponEquipped)
	{
		PlayFullBodyAnim(EnemyPawn.AnimNameEquipWeapon, 1.0f, 0.1f, 0.1f);
		WaitForFullBodyAnim();
	}

	if (TargetInSpecialLocation() || !IsInApproachAttackRange())
	{
		GotoState('Idle');
	}

	// Clear any rotation before starting attack.
	EnemyPawn.ResetDesiredRotation();
	SetFocalPoint(vect(0,0,0));

	EnemyPawn.TurnOnSpot(rotator(AttackStartRotation));
	WaitForSpecialMove();

	EnemyPawn.SetDesiredRotation(rotator(AttackStartRotation), true);
	if (!EnemyPawn.ReachedDesiredRotation())
	{
		FinishRotation();
	}

	while (TargetPlayer != None && TargetPlayer.SpecialMove != SMT_None)
	{
		Sleep(0.1f);
	}

	if (TargetInSpecialLocation() || !PerformAttackCheck() || !IsInFinalAttackRange())
	{
		GotoState('Idle');
	}

	if (bUseQuickAttack)
	{
		EnemyPawn.StartSpecialMove(SMT_AttackQuick, EnemyPawn.Location, Normal2D(TargetPlayer.Location - EnemyPawn.Location), APTT_TargetAtStart);
	}
	else
	{
		EnemyPawn.StartNormalAttack();
	}
	
	WaitForSpecialMove();

	EnemyPawn.LockDesiredRotation(false);
	EnemyPawn.ZeroMovementVariables();
	ClearCurrentMove();

	EnemyPawn.StartIdleSound();

	if (Group != None)
	{
		Group.ReturnAttackToken(self);
	}
	else
	{
		AttackTimer = EnemyPawn.AttackNormalRecovery;
	}

	GotoState('Idle');
}

state AttackSqueeze extends Attacking
{
	function EndState(Name NextStateName)
	{
		Super.EndState(NextStateName);

		SightComponent.bSawPlayerInSqueeze = false;
		SightComponent.LastSqueeze = None;
	}

Begin:
	WaitForSpecialMove();

	// Clear any rotation before starting attack.
	EnemyPawn.ResetDesiredRotation();
	SetFocalPoint(vect(0,0,0));

	EnemyPawn.TurnOnSpot(rotator(AttackStartRotation));
	WaitForSpecialMove();

	EnemyPawn.SetDesiredRotation(rotator(AttackStartRotation), true);
	if (!EnemyPawn.ReachedDesiredRotation())
	{
		FinishRotation();
	}
	
	if (bAttackRight)
	{
		EnemyPawn.EnemyMode = EM_SqueezeGrabRight;
	}
	else
	{
		EnemyPawn.EnemyMode = EM_SqueezeGrabLeft;
	}

	EnemyPawn.StartSpecialMove(SMT_AttackSqueezeStart, AttackStartLocation, AttackStartRotation, APTT_TargetAtStart);

	WaitForSpecialMove(true);

	if (VSizeSq(EnemyPawn.Location - TargetPlayer.Location) < EnemyPawn.AttackSqueezeRange * EnemyPawn.AttackSqueezeRange && TargetPlayer.TryGrabFromSqueeze(EnemyPawn))
	{
		EnemyPawn.LockDesiredRotation(false);
		EnemyPawn.StartSpecialMove(SMT_AttackSqueezeSuccess);
		bAttackCycling = false;
		
		if (EnemyPawn.bCanThrow)
		{
			WaitForSpecialMove(true);
			ThrowPlayer();
			WaitForSpecialMove();
		}
		else
		{
			WaitForSpecialMove();
		}
		
		if (Group != None)
		{
			Group.ReturnAttackToken(self);
		}
		else
		{
			AttackTimer = EnemyPawn.AttackThrowRecovery;
		}

		EnemyPawn.StartIdleSound();

		GotoState('Idle');
	}
	else
	{
		EnemyPawn.StartSpecialMove(SMT_AttackSqueezeStartToWait);
		WaitForSpecialMove();
	}

	bAttackCycling = true;
	SetTimer(EnemyPawn.AttackSqueezeCycleTime, false, 'AttackCycleEnd');

	while(bAttackCycling)
	{
		if (VSizeSq(EnemyPawn.Location - TargetPlayer.Location) < EnemyPawn.AttackSqueezeRange * EnemyPawn.AttackSqueezeRange && TargetPlayer.CanGrabFromSqueeze())
		{
			EnemyPawn.StartSpecialMove(SMT_AttackSqueezeWaitToSuccess);
			WaitForSpecialMove(true);

			if (TargetPlayer.TryGrabFromSqueeze(EnemyPawn))
			{
				EnemyPawn.LockDesiredRotation(false);
				EnemyPawn.StartSpecialMove(SMT_AttackSqueezeSuccess);
				bAttackCycling = false;
				ClearTimer('AttackCycleEnd');

				if (EnemyPawn.bCanThrow)
				{
					WaitForSpecialMove(true);
					ThrowPlayer();
					WaitForSpecialMove();
				}
				else
				{
					WaitForSpecialMove();
				}

				if (Group != None)
				{
					Group.ReturnAttackToken(self);
				}
				else
				{
					AttackTimer = EnemyPawn.AttackThrowRecovery;
				}

				EnemyPawn.StartIdleSound();

				Sleep(EnemyPawn.AttackIdleTimeAfterGrab);

				GotoState('Idle');
			}
			else
			{
				EnemyPawn.StartSpecialMove(SMT_AttackSqueezeStartToWait);
				WaitForSpecialMove();
			}
		}
		else
		{
			Sleep(0.1);
		}
	}

	EnemyPawn.StartSpecialMove(SMT_AttackSqueezeWaitToFail);
	WaitForSpecialMove();

	IgnoreTarget(5.0f);

	EnemyPawn.LockDesiredRotation(false);

	GotoState('Idle');
}

state AttackLocker extends Attacking
{
	function EndState(Name NextStateName)
	{
		Super.EndState(NextStateName);

		ActiveDoor = None;
	}
Begin:
	if (TargetPlayer.ActiveLocker != None)
	{
		WaitForSpecialMove();

		// Clear any rotation before starting attack.
		EnemyPawn.ResetDesiredRotation();
		SetFocalPoint(vect(0,0,0));

		EnemyPawn.TurnOnSpot(rotator(AttackStartRotation));
		WaitForSpecialMove();

		EnemyPawn.SetDesiredRotation(rotator(AttackStartRotation));
		if (!EnemyPawn.ReachedDesiredRotation())
		{
			FinishRotation();
		}

		while (TargetPlayer.SpecialMove == SMT_EnterLocker)
		{
			Sleep(0.1f);
		}

		// Initiate Locker Grab
		if (TargetPlayer.LocomotionMode == LM_Locker && EnemyPawn.bUsingWeapon && EnemyPawn.bHasWeaponEquipped)
		{
			PlayFullBodyAnim(EnemyPawn.AnimNameUnequipWeapon, 1.0f, 0.2f, 0.2f);
			WaitForFullBodyAnim();
		}

		if (TargetPlayer.LocomotionMode == LM_Locker) // check that the target is still in the locker
		{
			EnemyPawn.StartSpecialMove(SMT_AttackLocker, AttackStartLocation, AttackStartRotation, APTT_TargetAtStart);

			ActiveDoor = TargetPlayer.ActiveLocker.AssociatedDoor;

			StartDoorTraversal( false );

			if (EnemyPawn.IsA('OLEnemyNanoCloud'))
			{
				TargetPlayer.TryKillInLocker(EnemyPawn);
			}
			else
			{
				TargetPlayer.TryGrabFromLocker(EnemyPawn);
			}
			
			if (EnemyPawn.bCanThrow)
			{
				WaitForSpecialMove(true);
				ThrowPlayer();
				WaitForSpecialMove();
			}
			else
			{
				WaitForSpecialMove();
			}

			EndDoorTraversal();

			if (EnemyPawn.bUsingWeapon && !EnemyPawn.bHasWeaponEquipped)
			{
				PlayFullBodyAnim(EnemyPawn.AnimNameEquipWeapon, 1.0f, 0.2f, 0.2f);
				WaitForFullBodyAnim();
			}

			EnemyPawn.ZeroMovementVariables();

			if (Group != None)
			{
				Group.ReturnAttackToken(self);
			}
			else
			{
				AttackTimer = EnemyPawn.AttackThrowRecovery;
			}

			EnemyPawn.StartIdleSound();

			Sleep(EnemyPawn.AttackIdleTimeAfterGrab);
		}
		else if (EnemyPawn.bUsingWeapon && !EnemyPawn.bHasWeaponEquipped)
		{
			PlayFullBodyAnim(EnemyPawn.AnimNameEquipWeapon, 1.0f, 0.2f, 0.2f);
			WaitForFullBodyAnim();
		}
	}

	GotoState('Idle');
}

state AttackBed extends Attacking
{
Begin:
	if (TargetPlayer.ActiveBed != None)
	{
		WaitForSpecialMove();
		ClearDestination();

		// Clear any rotation before starting attack.
		EnemyPawn.ResetDesiredRotation();
		SetFocalPoint(vect(0,0,0));

		EnemyPawn.TurnOnSpot(rotator(AttackStartRotation));
		WaitForSpecialMove();

		EnemyPawn.SetDesiredRotation(rotator(AttackStartRotation));
		if (!EnemyPawn.ReachedDesiredRotation())
		{
			FinishRotation();
		}

		while (TargetPlayer.SpecialMove == SMT_EnterBed)
		{
			Sleep(0.1f);
		}

		if (TargetPlayer.LocomotionMode == LM_Bed)
		{
			if (EnemyPawn.bUsingWeapon && EnemyPawn.bHasWeaponEquipped)
			{
				PlayFullBodyAnim(EnemyPawn.AnimNameUnequipWeapon, 1.0f, 0.2f, 0.2f);
				WaitForFullBodyAnim();
			}

			EnemyPawn.StartSpecialMove(SMT_AttackBed, AttackStartLocation, AttackStartRotation, APTT_TargetAtStart);

			ActiveBed = TargetPlayer.ActiveBed;

			TargetPlayer.TryGrabFromBed(EnemyPawn);

			if (EnemyPawn.bCanThrow)
			{
				WaitForSpecialMove(true);
				ThrowPlayer();
				WaitForSpecialMove();
			}
			else
			{
				WaitForSpecialMove();
			}
			
			if (EnemyPawn.bUsingWeapon && !EnemyPawn.bHasWeaponEquipped)
			{
				PlayFullBodyAnim(EnemyPawn.AnimNameEquipWeapon, 1.0f, 0.2f, 0.2f);
				WaitForFullBodyAnim();
			}

			if (Group != None)
			{
				Group.ReturnAttackToken(self);
			}
			else
			{
				AttackTimer = EnemyPawn.AttackThrowRecovery;
			}

			EnemyPawn.StartIdleSound();

			Sleep(EnemyPawn.AttackIdleTimeAfterGrab);
		}
	}

	GotoState('Idle');
}

state AttackGrab extends Attacking
{
Begin:
	WaitForSpecialMove();

	// Clear any rotation before starting attack.
	EnemyPawn.ResetDesiredRotation();
	SetFocalPoint(vect(0,0,0));

	EnemyPawn.TurnOnSpot(rotator(AttackStartRotation));
	WaitForSpecialMove();

	EnemyPawn.SetDesiredRotation(rotator(AttackStartRotation));
	if (!EnemyPawn.ReachedDesiredRotation())
	{
		FinishRotation();
	}

	while (TargetPlayer != None && TargetPlayer.SpecialMove != SMT_None)
	{
		Sleep(0.1f);
	}

	if (TargetInSpecialLocation() || !PerformGrabCheck())
	{
		GotoState('Idle');
	}

	if (!TryGrabNormal(false))
	{
		GotoState('Idle');
	}

	if (bKilling)
	{
		WaitForSpecialMove(true);
		GotoState('KillPlayer');
	}
	else if (EnemyPawn.bCanThrow)
	{
		WaitForSpecialMove(true);
		ThrowPlayer();
		WaitForSpecialMove();
	}
	else
	{
		WaitForSpecialMove();
	}

	EnemyPawn.ZeroMovementVariables();
	ClearCurrentMove();

	if (Group != None)
	{
		Group.ReturnAttackToken(self);
	}
	else
	{
		AttackTimer = EnemyPawn.AttackThrowRecovery;
	}

	Sleep(EnemyPawn.AttackIdleTimeAfterGrab);

	EnemyPawn.StartIdleSound();

	GotoState('Idle');
}

state AttackGrabUnder extends Attacking
{
Begin:
	WaitForSpecialMove();

	EnemyPawn.RegisterNavMeshObstacle();

	if (Group != None)
	{
		Group.NotifyOthersPathChanged(self);
	}

	// Clear any rotation before starting attack.
	EnemyPawn.ResetDesiredRotation();
	SetFocalPoint(vect(0,0,0));

	EnemyPawn.TurnOnSpot(rotator(AttackStartRotation));
	WaitForSpecialMove();

	EnemyPawn.SetDesiredRotation(rotator(AttackStartRotation));
	if (!EnemyPawn.ReachedDesiredRotation())
	{
		FinishRotation();
	}

	while (TargetPlayer != None && TargetPlayer.SpecialMove != SMT_None)
	{
		Sleep(0.1f);
	}

	if (TargetInSpecialLocation())
	{
		GotoState('Idle');
	}

	if (EnemyPawn.bUsingWeapon && EnemyPawn.bHasWeaponEquipped)
	{
		PlayFullBodyAnim(EnemyPawn.AnimNameUnequipWeapon, 1.0f, 0.2f, 0.2f);
		WaitForFullBodyAnim();
	}

	if (!TryGrabUnder())
	{
		if (EnemyPawn.bUsingWeapon && !EnemyPawn.bHasWeaponEquipped)
		{
			PlayFullBodyAnim(EnemyPawn.AnimNameEquipWeapon, 1.0f, 0.2f, 0.2f);
			WaitForFullBodyAnim();
		}

		GotoState('Idle');
	}

	if (bKilling)
	{
		WaitForSpecialMove(true);
		GotoState('KillPlayer');
	}
	else if (EnemyPawn.bCanThrow)
	{
		WaitForSpecialMove(true);
		ThrowPlayer();
		WaitForSpecialMove();
	}
	else
	{
		WaitForSpecialMove();
	}

	if (EnemyPawn.bUsingWeapon && !EnemyPawn.bHasWeaponEquipped)
	{
		PlayFullBodyAnim(EnemyPawn.AnimNameEquipWeapon, 1.0f, 0.2f, 0.2f);
		WaitForFullBodyAnim();
	}

	EnemyPawn.ZeroMovementVariables();
	ClearCurrentMove();

	if (Group != None)
	{
		Group.ReturnAttackToken(self);
	}
	else
	{
		AttackTimer = EnemyPawn.AttackThrowRecovery;
	}

	Sleep(EnemyPawn.AttackIdleTimeAfterGrab);

	EnemyPawn.StartIdleSound();

	GotoState('Idle');
}

state AttackCrouch extends Attacking
{
Begin:
	WaitForSpecialMove();

	// Clear any rotation before starting attack.
	EnemyPawn.ResetDesiredRotation();
	SetFocalPoint(vect(0,0,0));

	EnemyPawn.TurnOnSpot(rotator(AttackStartRotation));
	WaitForSpecialMove();

	EnemyPawn.SetDesiredRotation(rotator(AttackStartRotation));
	if (!EnemyPawn.ReachedDesiredRotation())
	{
		FinishRotation();
	}

	while (TargetPlayer != None && TargetPlayer.SpecialMove != SMT_None)
	{
		Sleep(0.1f);
	}

	if (TargetInSpecialLocation())
	{
		GotoState('Idle');
	}

	if (!TryGrabNormal(true))
	{
		GotoState('Idle');
	}

	if (bKilling)
	{
		WaitForSpecialMove(true);
		GotoState('KillPlayer');
	}
	else if (EnemyPawn.bCanThrow)
	{
		WaitForSpecialMove(true);
		ThrowPlayer();
		WaitForSpecialMove();
	}
	else
	{
		WaitForSpecialMove();
	}

	EnemyPawn.ZeroMovementVariables();
	ClearCurrentMove();

	if (Group != None)
	{
		Group.ReturnAttackToken(self);
	}
	else
	{
		AttackTimer = EnemyPawn.AttackThrowRecovery;
	}

	EnemyPawn.StartIdleSound();

	GotoState('Idle');
}

state AttackPush extends Attacking
{
Begin:
	EnemyPawn.StartSpecialMove(SMT_AttackPush);

	WaitForSpecialMove();

	EnemyPawn.StartIdleSound();
	
	GotoState('Idle');
}

state AttackKill extends Attacking
{
Begin:
	// Clear any rotation before starting attack.
	EnemyPawn.ResetDesiredRotation();
	SetFocalPoint(vect(0,0,0));

	EnemyPawn.TurnOnSpot(rotator(AttackStartRotation));
	WaitForSpecialMove();

	EnemyPawn.SetDesiredRotation(rotator(AttackStartRotation));
	if (!EnemyPawn.ReachedDesiredRotation())
	{
		FinishRotation();
	}

	if (TargetPlayer.TryKillHero(EnemyPawn, AttackStartLocation, AttackStartRotation))
	{
		EnemyPawn.StartSpecialMove(SMT_KillHero, AttackStartLocation, AttackStartRotation, APTT_TargetAtStart);
	}
	else
	{
		EnemyPawn.StartNormalAttack();
	}

	WaitForSpecialMove();

	EnemyPawn.StartIdleSound();

	GotoState('Idle');
}

state KillPlayer
{	
	function BeginState(Name PreviousStateName)
	{
		bAttacking = true;
	}

	function EndState(Name NextStateName)
	{
		bAttacking = false;
	}
Begin:
	EnemyPawn.StartSpecialMove(SMT_KillHero);
	TargetPlayer.TryDecapitate(EnemyPawn);

	WaitForSpecialMove();

	GotoState('Idle');
}

state InvestigatingObject
{
	function BeginState(Name PreviousStateName)
	{
		`AILog("Investigating Object", 'AIState');

		EnemyPawn.ZeroMovementVariables();
		ClearCurrentMove();

		OLNavHandle.ClearPath();
		ClearDestination();

		bInvestigatingObject = true;

		EnemyPawn.StopIdleSound();
	}

	function EndState(Name NextStateName)
	{
		bInvestigatingObject = false;

		if (NextMove.Type != MT_Invalid)
		{
			StartMove(NextMove);
		}

		EnemyPawn.ResetDesiredRotation();
	}
}

state InvestigatingLocker extends InvestigatingObject
{
	function EndState(Name NextStateName)
	{
		Super.EndState(NextStateName);

		ActiveDoor = None;
		ActiveLocker = None;
	}

Begin:
	if (ActiveLocker != None)
	{
		WaitForSpecialMove();

		// Clear any rotation before starting attack.
		EnemyPawn.ResetDesiredRotation();
		SetFocalPoint(vect(0,0,0));

		EnemyPawn.TurnOnSpot(rotator(InvestigateStartRotation));
		WaitForSpecialMove();

		EnemyPawn.SetDesiredRotation(rotator(InvestigateStartRotation));
		if (!EnemyPawn.ReachedDesiredRotation())
		{
			FinishRotation();
		}

		ActiveDoor = ActiveLocker.AssociatedDoor;

		StartDoorTraversal(false);

		if (EnemyPawn.bUsingWeapon && EnemyPawn.bHasWeaponEquipped)
		{
			PlayFullBodyAnim(EnemyPawn.AnimNameUnequipWeapon, 1.0f, 0.2f, 0.2f);
			WaitForFullBodyAnim();
		}

		EnemyPawn.StartSpecialMove(SMT_InvestigateLocker, InvestigateStartLocation, InvestigateStartRotation, APTT_TargetAtStart);

		WaitForSpecialMove();

		if (EnemyPawn.bUsingWeapon && !EnemyPawn.bHasWeaponEquipped)
		{
			PlayFullBodyAnim(EnemyPawn.AnimNameEquipWeapon, 1.0f, 0.2f, 0.2f);
			WaitForFullBodyAnim();
		}

		EndDoorTraversal();

		EnemyPawn.ZeroMovementVariables();

		EnemyPawn.StartIdleSound();
		//Sleep(1.0f);
	}

	GotoState('Idle');
}

state InvestigatingBed extends InvestigatingObject
{
Begin:
	if (ActiveBed != None)
	{
		WaitForSpecialMove();

		// Clear any rotation before starting attack.
		EnemyPawn.ResetDesiredRotation();
		SetFocalPoint(vect(0,0,0));

		EnemyPawn.TurnOnSpot(rotator(InvestigateStartRotation));
		WaitForSpecialMove();

		EnemyPawn.SetDesiredRotation(rotator(InvestigateStartRotation));
		if (!EnemyPawn.ReachedDesiredRotation())
		{
			FinishRotation();
		}
		
		if (EnemyPawn.bUsingWeapon && EnemyPawn.bHasWeaponEquipped)
		{
			PlayFullBodyAnim(EnemyPawn.AnimNameUnequipWeapon, 1.0f, 0.2f, 0.2f);
			WaitForFullBodyAnim();
		}

		EnemyPawn.StartSpecialMove(SMT_InvestigateBed, InvestigateStartLocation, InvestigateStartRotation, APTT_TargetAtStart);

		WaitForSpecialMove();

		if (EnemyPawn.bUsingWeapon && !EnemyPawn.bHasWeaponEquipped)
		{
			PlayFullBodyAnim(EnemyPawn.AnimNameEquipWeapon, 1.0f, 0.2f, 0.2f);
			WaitForFullBodyAnim();
		}

		EnemyPawn.ZeroMovementVariables();

		EnemyPawn.StartIdleSound();
		//Sleep(1.0f);
	}

	GotoState('Idle');
}

state Interruption
{
	function BeginState(Name PreviousStateName)
	{
		EnemyPawn.ZeroMovementVariables();
		ClearCurrentMove();
		ClearNextMove();
		ClearDestination();
	}
}

state Avoiding extends Interruption
{
	function EndState(Name NextStateName)
	{
		bAvoiding = false;
	}
Begin:
	`AILog("Avoiding", 'AIState');

	EnemyPawn.StartSpecialMove(SMT_Avoiding);
	WaitForSpecialMove();

	Recalculate(true);
	GotoState('Idle');
}

state Knockback extends Interruption
{
Begin:
	`AILog("Knockback", 'AIState');

	EnemyPawn.StartSpecialMove(SMT_Knockedback);
	WaitForSpecialMove();

	Recalculate(true);
	GotoState('Idle');
}

state WaitForMove extends Interruption
{
Begin:
	`AILog("WaitForMove - " $ WaitForMoveTime, 'AIState');

	Sleep(WaitForMoveTime);

	Recalculate(true);
	GotoState('Idle');
}

state Disturbed
{
	function BeginState(Name PreviouStateName)
	{
		EnemyPawn.ZeroMovementVariables();
		ClearCurrentMove();
		ClearNextMove();
		ClearDestination();

		bDisturbed = true;
	}

	function EndState(Name NextStateName)
	{
		bDisturbed = false;
	}
Begin:
	`AILog("Disturbed", 'AIState');

	EnemyPawn.StartDisturbed();
	WaitForSpecialMove();

	GotoState('Idle');
}

state WaitForDoor extends Interruption
{
Begin:
	`AILog("Pause", 'AIState');

	Sleep(2.f);

	Recalculate(true);
	GotoState('Idle');
}

/*============================================================================*/

/**
 * Scripting hook to move this AI to a specific actor using NavMesh.
 */
function OnAIMoveToActor(SeqAct_AIMoveToActor Action)
{
	local Actor DestActor;
	local SeqVar_Object ObjVar;

	// abort any previous latent moves
	ClearLatentAction(class'SeqAct_AIMoveToActor',true,Action);
	// pick a destination
	DestActor = Action.PickDestination(Pawn);
	// if we found a valid destination
	if (DestActor != None)
	{
		// set the target and push our movement state
		ScriptedRoute = Route(DestActor);
		if (ScriptedRoute != None)
		{
			if (ScriptedRoute.RouteList.length == 0)
			{
				`warn("Invalid route with empty MoveList for scripted move");
			}
			else
			{
				ScriptedRouteIndex = 0;
				if (!IsInState('ScriptedRouteMove'))
				{
					PushState('ScriptedRouteMove');
				}
			}
		}
		else
		{
			// MT->pop existing state if there is one
			if(IsInState('ScriptedMove'))
			{
				PopState(true);
			}
			ScriptedMoveTarget = DestActor;
			PushState('ScriptedMove');
// 			if (!IsInState('ScriptedMove'))
// 			{
// 				PushState('ScriptedMove');
// 			}
		}
		// set AI focus, if one was specified
		ScriptedFocus = None;
		foreach Action.LinkedVariables(class'SeqVar_Object', ObjVar, "Look At")
		{
			ScriptedFocus = Actor(ObjVar.GetObjectValue());
			if (ScriptedFocus != None)
			{
				break;
			}
		}
	}
	else
	{
		`warn("Invalid destination for scripted move");
	}
}

//Overwrite AIController's ScriptedMove state to make use of the NavigationHandle instead of the old way
state ScriptedMove
{
	function bool FindNavMeshPath()
	{
		// Clear cache and constraints (ignore recycling for the moment)
		NavigationHandle.PathConstraintList = none;
		NavigationHandle.PathGoalList = none;

		// Create constraints
		class'NavMeshPath_Toward'.static.TowardGoal( NavigationHandle,ScriptedMoveTarget );
		class'NavMeshGoal_At'.static.AtActor( NavigationHandle, ScriptedMoveTarget );

		// Find path
		return NavigationHandle.FindPath();
	}

	Begin:
		`log("BEGIN STATE SCRIPTEDMOVE");
		// while we have a valid pawn and move target, and
		// we haven't reached the target yet
		NavigationHandle.SetFinalDestination(ScriptedMoveTarget.Location);

		
		if( !NavigationHandle.ActorReachable( ScriptedMoveTarget) )
		{
			if( FindNavMeshPath() )
			{
				`log("FindNavMeshPath returned TRUE");
				FlushPersistentDebugLines();
				NavigationHandle.DrawPathCache(,TRUE);
			}
			else
			{
				//give up because the nav mesh failed to find a path
				`warn("FindNavMeshPath failed to find a path to"@ScriptedMoveTarget);
				ScriptedMoveTarget = None;
			}   
		}
		else
		{
			// then move directly to the actor
			MoveTo( ScriptedMoveTarget.Location,ScriptedFocus );
		}

		while( Pawn != None && ScriptedMoveTarget != None && !Pawn.ReachedDestination(ScriptedMoveTarget) )
		{				
			// move to the first node on the path
			if( NavigationHandle.GetNextMoveLocation( MoveTempDest, Pawn.GetCollisionRadius()) )
			{
				// suggest move preparation will return TRUE when the edge's
			    // logic is getting the bot to the edge point
					// FALSE if we should run there ourselves
				if (!NavigationHandle.SuggestMovePreparation( MoveTempDest,self))
				{
					MoveTo( MoveTempDest, ScriptedFocus );						
				}
			}
		}

	`log("POPPING STATE!");
	Pawn.ZeroMovementVariables();
	// return to the previous state
	PopState();
}

/*============================================================================*/

defaultproperties
{
	bIsPlayer=false
	
	NavigationHandleClass=class'OLNavigationHandle'

	InterruptionState=Interruption

	Begin Object Class=OLAISightComponent Name=TheSeer
	End Object
	SightComponent=TheSeer
	Components.Add(TheSeer)

	DynamicPathCheckTime=0.3333f

	NavigationExtent=(X=5.f,Y=5.f,Z=90.f)

	CurrentNoiseValue=0.0f
	TimeSinceNoise=-1.0f

	bWasChasing=false
	bInDarkness=false

	bAlwaysTick=true

	BehaviorState=AIBS_Idle
	PatrolMode=EM_Patrol
	NextPatrolRouteIndex=-1

	bEnableHeadTracking=true

	CheckStuckSpeedThreshold=25.0f
	StuckRepathDelayLength=1.0f
}
