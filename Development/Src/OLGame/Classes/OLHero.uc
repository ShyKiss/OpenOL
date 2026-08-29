class OLHero extends OLPawn
	notplaceable
	config(Game)
	native;

////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////

var OLPlayerController OLPC;
var OLHero DefaultHero;

var vector CharForward; // cached version of Rotation.Vector()
var vector EyeForward; // cached version of EyeRotation.Vector()

// TODO: investigate potential optimisation of replacing all those by preallocated fixed arrays
var transient Array<OLLedgeMarker> CachedMarkers;
var transient Array<OLHidingSpot> CachedHidingSpots;
var transient Array<OLDoor> CachedDoors;
var transient Array<OLBed> CachedBeds;
var transient Array<OLPickableObject> CachedPickables;
var transient Array<OLLadderMarker> CachedLadders;
var transient Array<OLCSA> CachedCSAs;
var transient Array<OLScareMoment> CachedScares;
var transient Array<OLPushableObject> CachedPushables;
var transient Array<OLCornerMarker> CachedCorners;
var transient Array<OLHeatMarker> CachedHeatMarkers;
var transient Array<OLRecordingMarker> CachedRecordingMarkers;
var transient Array<OLPreferredPathMarker> CachedPreferredPathMarkers; // for AIs
var transient Array<Light> CachedLights; // for OLPC light optim
var transient vector CachedObjectsPos;
var native transient Array<pointer> CachedLevelList{ULevel};

// Components
var SkeletalMeshComponent ShadowProxy;
var StaticMeshComponent HeadMesh;
var SkeletalMeshComponent DummyHeadMesh;  // Skeletal head mesh for dummy players (WorkerHead_V2)
var int DummyHeadCamPitch;  // Last received camera pitch for dummy — reapplied every tick
var int DummyHeadCamYaw;    // Last received camera yaw for dummy — reapplied every tick
var SkeletalMeshComponent CameraMesh;
var SkeletalMeshComponent CameraMeshShadowProxy;
var ParticleSystemComponent BloodEffect;
var ParticleSystemComponent DeathParticles;
var ParticleSystemComponent DecapitatedBloodEffect;
var ParticleSystemComponent RainEffect;
var ParticleSystemComponent WaterFootstepParticlesLeft;
var ParticleSystemComponent WaterFootstepParticlesRight;
var ParticleSystemComponent WaterSplashParticles;
var ParticleSystemComponent GenericCameraEffect;
var SpotLightComponent NVLightPowered;
var SpotLightComponent NVLightDefault;
var SpotLightComponent DarkLight;
var PointLightComponent CamcorderScreenLight;
var SkeletalMesh FingerlessMesh;
var SkeletalMesh ITTechMesh;
var SkeletalMesh PrisonerMesh;
var MaterialInstanceConstant FootstepDecalMatL1;
var MaterialInstanceConstant FootstepDecalMatL2;
var MaterialInstanceConstant FootstepDecalMatR1;
var MaterialInstanceConstant FootstepDecalMatR2;
var MaterialInstanceConstant FootstepBarefeetDecalMatL1;
var MaterialInstanceConstant FootstepBarefeetDecalMatL2;
var MaterialInstanceConstant FootstepBarefeetDecalMatR1;
var MaterialInstanceConstant FootstepBarefeetDecalMatR2;
var MaterialInstanceConstant CameraScreenMat;
var name CameraScreenParamName;

// Anim nodes
var OLAnimNodeSlot ShadowProxyFullBodyAnimSlot;
var OLAnimCustomBlend ShadowProxyCustomBlendNode;
var OLAnimCustomBlend UpperBodyBlendNode;
var OLAnimCustomBlend ShadowProxyUpperBodyBlendNode;
var AnimNodeSlot RightArmAnimSlot;
var AnimNodeSlot ShadowProxyRightArmAnimSlot;
var AnimNodeSlot LeftArmAnimSlot;
var AnimNodeSlot ShadowProxyLeftArmAnimSlot;
var OLAnimBlendByPosture BlendByPostureWalkingAnimNode;
var OLAnimBlendByPosture BlendByPostureFallingAnimNode;
var OLAnimBlendByLocomotionMode LocomotionAnimNode;
var OLAnimConstrainedMovement LedgeHangAnimNode;
var OLAnimConstrainedMovement ShadowProxyLedgeHangAnimNode;
var OLAnimConstrainedMovement LedgeWalkAnimNode;
var OLAnimConstrainedMovement ShadowProxyLedgeWalkAnimNode;
var OLAnimConstrainedMovement SqueezeAnimNode;
var OLAnimConstrainedMovement ShadowProxySqueezeAnimNode;
var OLAnimMultiCycleConstrainedMovement LadderAnimNode;
var OLAnimMultiCycleConstrainedMovement ShadowProxyLadderAnimNode;
var OLAnimDoorInteraction DoorAnimNode;
var OLAnimDoorInteraction ShadowProxyDoorAnimNode;
var AnimNodeBlendPerBone LeftArmWallContactFilterNode;
var AnimNodeBlendPerBone ShadowProxyLeftArmWallContactFilterNode;
var AnimNodeSequence LeftArmWallContactAnimSequence;
var AnimNodeSequence ShadowProxyLeftArmWallContactAnimSequence;
var OLAnimPeeking PeekingAnimNode;
var OLAnimPeeking ShadowProxyPeekingAnimNode;
var AnimNodeSequence BedAnimNode;
var AnimNodeSequence ShadowProxyBedAnimNode;
var AnimNodeBlendPerBone LeftFootPlacementNode;
var AnimNodeBlendPerBone ShadowProxyLeftFootPlacementNode;
var AnimNodeBlendPerBone RightFootPlacementNode;
var AnimNodeBlendPerBone ShadowProxyRightFootPlacementNode;
var OLAnimStruggleCycle StruggleAnimNode;
var OLAnimStruggleCycle ShadowProxyStruggleAnimNode;
var OLAnimCrouchedTurnOnSpot CrouchTurnOnSpotAnimNode;
var OLAnimCrouchedTurnOnSpot ShadowProxyCrouchTurnOnSpotAnimNode;
var OLAnimParrying ParryingAnimNode;
var OLAnimParrying ShadowProxyParryingAnimNode;
var OLAnimBlendByLeaning LeanStandingAnimNode;
var OLAnimBlendByLeaning LeanCrouchedAnimNode;

// Skel controls
var SkelControlLimb ShadowProxyLeftHandIK;
var SkelControlLimb ShadowProxyRightHandIK;
var SkelControlSingleBone HiddenRightArmControl;
var SkelControlSingleBone HiddenLeftArmControl;
var SkelControlBase LeftForeTwistControl;
var SkelControlBase ShadowProxyLeftForeTwistControl;
var SkelControlBase LeftForeTwist1Control;
var SkelControlBase ShadowProxyLeftForeTwist1Control;
var SkelControlBase LeftUpArmTwistControl;
var SkelControlBase ShadowProxyLeftUpArmTwistControl;
var SkelControlBase RightForeTwistControl;
var SkelControlBase ShadowProxyRightForeTwistControl;
var SkelControlBase RightUpArmTwistControl;
var SkelControlBase ShadowProxyRightUpArmTwistControl;
var SkelControlBase ShadowProxyRightClavicleFixup;

var bool bLeftHandIKActive;
var bool bRightHandIKActive;
var bool bShouldHideLeftHandDuringSM;
var bool bShouldHideRightHandDuringSM;
var float MeshZOffset; // Moves up and down to adjust the legs for large steps
var float MeshXOffset; // Moves forward and back to adjust for camera obstruction
var float LastFrameMeshZOffset; // used to compensate mesh motion on camera lights

enum IKTargetType
{
	IKTT_DoorKnob,
	IKTT_CSAPropDestination,
	IKTT_Other
};

struct native HandIKData
{
	var bool bActive;
	var IKTargetType IKTarget;
	var float StartTime;
	var float Duration;
	var float FadeInTime;
	var float FadeOutTime;
	var vector EffectorOffset;
};
var HandIKData LeftHandIKData;

enum CornerPeekPosition
{
	CPP_Left,	// Looking from the left side (CCW side)
	CPP_Right,	// Looking from the right side (CW side)
	CPP_MiddleLeft, // On 3-sided markers, from the middle looking to the right
	CPP_MiddleRight, // On 3-sided markers, from the middle looking to the left
};

struct native CornerPeekData
{
	var OLCornerMarker CornerMarker;
	var CornerPeekPosition PeekPosition;
	var vector CornerLocation;
	var vector FwdDir;
	var vector SideDir;
	var bool bRoundedCorner;

	// IK
	var float IKStrength;
	var float LastInterpActivatedTime;
	var float LastDisconnectTime;
	var bool bDisconnecting;

	var vector AnchorPos;
	var quat AnchorRot;
	var vector JointTargetPos;
};
var CornerPeekData CornerPeek;

var float LastValidCornerPeekTime;
var OLCornerMarker LastValidCornerMarker;
var CornerPeekPosition LastValidCornerPeekPosition;

struct native AttachmentData
{
	var PrimitiveComponent AttachedComp;

	var bool bHideWhenDone;
	var float BlendInTime;
	var float BlendOutTime;
	var float Duration;
	var vector PositionOffset;
	var rotator RotationOffset;

	var bool bActive;
	var bool bAttached; // when blending out, bAttached is FALSE but bActive stays TRUE
	var float StartTimestamp;
	var vector BlendStartPos;
	var quat BlendStartRot;
};
var AttachmentData ActiveAttachment;

var bool bMustCrouchAfterSpecialMove;
var bool bForcedCrouch;
var float CurrentLean; // 0.0f - no lean, 1.0f - fully right, -1.0f - fully left. ONLY FOR PROCEDURAL LEAN, not contextuals.
var bool bLeaningLeftPushing;
var bool bLeaningRightPushing;
var bool bWantToRun;
var bool bIsGhost;
var float LastLeanSndTime;
var float LastFootstepTime;
var int LastFootDown;
var float LargeSlopeStartedTime;
var bool bPlayingRunSnd;
var int RemainingBloodyFootsteps;

//
// Camera
//

var OLHeroCamera Camera;
var name LeftHandBoneName;
var name LeftHandAuxBoneName;
var name RightHandAuxBoneName;

var vector	EyeLocation;
var rotator EyeRotation;
var float	CurrentFOV;
var matrix	CameraCompSpace; // used for camera-space anim, to get around a frame lag between camera, physics and mesh updates

struct native CameraParameters
{
	var float	MinYaw;
	var float	MaxYaw;
	var float	MinPitchWS; // world-space
	var float	MaxPitchWS;
	var float	MinPitchCS; // cam-space
	var float	MaxPitchCS;
};

var CameraParameters CamParams;

//
// Health
//

var float LastDamageTime;
var class<OLDmgType> LastDamageType;
var float LastScriptedDamageEffectTime; // used to prevent overlapping NonFatalDamage notifies to step on top of each other
var float PreciseHealth; // same as Health, except in float for e.g. slow regen

var float ElectricEffectStartTime;
var float NextElectricHurtSoundTime;
var float ElectricGlitchStartTime;
var float ElectricGlitchMaxIntensity;
var float ElectricGlitchDuration;
var float ElectricGlitchFrequency;
var bool bElectricGlitchActive;

var float TimeOfDeath;
var bool bElectrified;
var bool bHeatShielding;
var float HeatDistance;
var float LastHeatDamageTime;
var float LastElectricDamageTime;
var float CurrentHeatBlur;
var bool bParrying;
var bool bLimping;
var bool bHobbling;
var float HobblingIntensity; // 1 - full hobble, 0 - no hobble
var float TargetHobblingIntensity;

//
// Camcorder
//

enum ECamcorderMode
{
	CCM_Default,	
	CCM_PoweredNightVision,
	CCM_NightVision,
};

enum ECamcorderState
{
	CCS_Inactive,
	CCS_Active,
	CCS_Raising,
	CCS_Lowering,
	CCS_ReloadingActive,
	CCS_ReloadingInactive,
};

var ECamcorderMode CamcorderMode;
var ECamcorderState CamcorderState;
var float CamcorderDisabledEndTime; // Time at which we can raise the camcorder again, if still valid
var float LastCamcorderSwitchTime; // Time at which we last raised or lowered the camcorder, as failsafe against bad state
var bool bCamcorderDesired;
var bool bLastCinematicModeDisabledCamcorder;
var bool bBothHandsNeeded; // Set during a special move while both hands are needed
var float CurrentBatterySetEnergy; // 0.5: half a battery left, 2.0: 2 full batteries left, max = 4.0 : fully recharged
var float CurrentCamcorderZoomFactor; // 0 - fully zoomed out, 1 - fully zoomed in
var float TargetCamcorderZoomFactor;
var float NVLightInterpFactor; // follows CurrentCamcorderZoomFactor with some lag
var bool bPlayingNVGlitchSound;
var bool bRainEffectDesired;
var float LastRainEffectActiveTime;
var float CurrentDarkLightRadius;
var float CurrentDarkLightBrightness;
var bool bCameraCracked;
var bool bOverrideDarkLight;
var float DarkLightOverrideBrightness;
var float DarkLightOverrideRadius;
var float CameraEffectEndTime;
var bool bCameraEffectActive;
var float CameraEffectPlaneDist;
var float OverriddenMinCamcorderFOV;

enum NVGlitchType
{
	NVGT_SuddenDrop,
	NVGT_SlowDrop,
	NVGT_Buzz,
	NVGT_LastBreath,
};

var struct native NVGlitchData
{
	var bool bGlitching;
	var float CurrentLevel; // 0,1
	var float NextGlitchTime;
	var float StartTime;	
	
	var NVGlitchType GlitchType;
	var float TargetLevel;	// how low can it go
	var float Duration;
	
} NVGlitch;

//
// Locomotion
//

var vector DesiredMoveDirection; // Where the player wants to go, normalized world space
var vector ExternalImpulse;
var bool bBlockConstrainedMovement; // Used to prevent starting another movement cycle in constrained modes (e.g. arriving at end of a ledge) 
var bool bKillConstrainedMovement; // Force constrained movement to idle
var float LedgeHangIdleStartTime; // to force idle pose if the player stops moving
var float WalkSpeed;
var float RunSpeed;
var float CurrentRunSpeed;
var float InputMovementScaling; // [0,1] - For gamepads, scales the input intention when the thumbstick isn't fully pushed
var bool bWantLookBack;
var float CurrentPlayerSpeedRTPC;
var float TargetPlayerSpeedRTPC; // for on-screen debug info only

var struct native LocomotionModeParameters
{
	var GameplayParams GP;

	var float	NeckOffsetSide;
	var float	NeckOffsetFwd;	

} LocomotionModeParams[ELocomotionMode]; 

var OLSeqAct_HeroControl HeroControl;

var OLSqueezeVolume	ActiveSqueeze;
var OLLedgeMarker		ActiveLedge; // used by both ledge walk and ledge hang
var OLDoor				ActiveDoor;
var OLLadderMarker	ActiveLadder; // the bottom marker
var OLPickableObject ActivePickup;
var OLHidingSpot		ActiveLocker;
var OLBed				ActiveBed;
var OLCSA				ActiveCSA;
var OLPushableObject	ActivePushable;

enum EHeroWalkingStyle
{
	HWS_Default,
	HWS_Water,
	HWS_Limping,
	HWS_Hobbling,
	HWS_PrototypeStyleA,
	HWS_PrototypeStyleB,
	HWS_PrototypeStyleC,
	HWS_PrototypeStyleD,
	HWS_PrototypeStyleE,
};

var EHeroWalkingStyle ForcedWalkingStyle;
var float WalkSpeedOverride;
var float RunSpeedOverride;
var float AccelApproachFactor;
var float DecelApproachFactor;

//
// Special Moves control flags
//

// Enter ledge: 0.0 - perpendicular, 1.0 - parallel
// Enter squeeze, ladder, door: 0.0 - 45 deg angle, 1.0 - straight
var float SpecialMoveBlendAlpha;
// Set by StartSpecialMove for moves with AdjustPosition so the network layer can
// read the computed animation start position and direction without recalculating.
var vector LastGrabPos;
var vector LastGrabDir;
// Target position root motion drives toward (e.g. ledge top for ClimbUpLedge).
var vector LastGrabTargetPos;
// Playback length of the anim started by the last StartSpecialMove call.
var float  LastGrabLength;
var name   LastPickupAnimName;  // dominant anim name chosen for SMT_PickupObject, for network replication
var bool   bPickupNotifyFired;  // set in PickupNotify(); cleared by network layer after sending PICKUP_ATTACH
var bool bLeftAnim; // multipurpose
var bool bBackAnim;
var bool bExitLadderLeftHand;
var bool bLookingBackLeftSide;
var bool bRunningTraversalMove; // for jump over, climb over and other traversal: are we using the running or walking version
var bool bApplyLandingPenalty; // after jumping but not moves such as slide over
var bool bJumping; // Set when jumping deliberately, not when LM_Fall/PHYS_Falling has been triggering automatically
var float SpecialMoveTargetYaw; // ws yaw where the special move should end at (e.g. during an exit squeeze), to dynamically adjust camera limits
var bool bPushingFromBackEdge;
var bool bEnterLedgeWalkAfterStruggle;
var bool bStartingSpecialMove; // used to control some flag handling in FinishSpecialMove
var bool bJumpFromLedgeWalkWithVelocity;
var bool bPickupCrouched;
var float DummyPickupDistHorz;
var float DummyPickupDistVert;
var bool  bDummyPickupIsDocument;
// Struggle anim play rate received from the remote player (dummy only).
var float DummyStrugglePlayRate;
// Struggle anim names received from the remote player via SMT_EnterStruggle packet (dummy only).
var name  DummyStruggleEntryAnimPlayer;
var name  DummyStruggleCycleAnimPlayer;
var name  DummyStruggleCycleAnimEnemy;
var float EnterBedZ;
var float LastCompletedDoorInteractionTime;

var float LastActiveLedgeTimestamp; // prevent grabbing ledge right after leaving it
var float LastActiveLedgeZ; // location.Z, not ledge itself. differentiator to allow dropping to a lower ledge
var float LastActiveLadderTimestamp;

var float LastSpecialMoveFinishedTime;
var float LastClimbUpObstacleFinishedTime;
var float RunStartedTime;
var float LastLandingTimestamp; // used for movement speed modifier
var float SpecialMoveStalledTimestamp; // Used as failsafe to unstuck from a special move; set when the pawn stops moving and the locomotion mode anim node has full weight
var float FallingStartedTime; // Warning! Similar but not quite the same as UDKPawn::StartedFallingTime
var float LastEnterLookBackTime;

var enum ELedgeTransitionType // Used for both ledge hang and ledge walk
{
	LTT_LeftInside,
	LTT_LeftOutside,
	LTT_RightInside,
	LTT_RightOutside

} ActiveLedgeTransitionType;

var enum ELedgeClimbType
{
	LCT_ClimbUpToStand,
	LCT_ClimbUpToCrouch,
	LCT_ClimbOverToFalling,
	LCT_ClimbOverToStand

} LedgeClimbType;

var enum ESqueezeTransitionType
{
	STT_Left_Facing,
	STT_Left_Back,
	STT_Right_Facing,
	STT_Right_Back,

} SqueezeTransitionType;

var enum EDoorOpeningType
{
	DOT_LeftPush,
	DOT_LeftPull,
	DOT_RightPush,
	DOT_RightPull,

} DoorOpeningType;

var enum EDoorPartialOpenType
{
	DPOT_LeftPush,
	DPOT_LeftPull,
	DPOT_LeftSwipe,
	DPOT_RightPush,
	DPOT_RightPull,
	DPOT_RightSwipe,

} DoorPartialOpenType;

var enum EDoorClosingType
{
	DCT_LeftFront,
	DCT_LeftSide,
	DCT_LeftBack,
	DCT_LeftInside,
	DCT_RightFront,
	DCT_RightSide,
	DCT_RightBack,
	DCT_RightInside,

} DoorClosingType;

// Interactive door opening - reach at least DoorInteractionStartingRatio before interactive open
var float DoorInteractionStartingRatio; // ratio relative to the anim node
var bool bDoorStartingRatioReached;
var bool bQuietDoorInteraction;
var float DoorSlowClosingAnimStartTime;

var float MovingNoiseTimer;
var bool MovingNoiseActive;

var enum EEnemyType
{
	ET_Soldier,
	ET_Generic,
	ET_Surgeon,
	ET_Swarm,
	ET_Other,
	ET_Groom,
	ET_Cannibal
} EnemyType;

var EWeaponType EnemyWeapon;

//
// Body Setup
//

enum EHeroBodySetup
{
	HBS_Normal, // shadow on proxy, no cam
	HBS_NoProxy, // shadow on main mesh, no cam
	HBS_CamcorderRaised, // shadow on proxy, cam invisible but projects shadow, no right arm
	HBS_CamcorderRaisedNoShadow, // no shadow, no right arm
	HBS_CamcorderVisible, // shadow on proxy, cam visible and projecting shadow, right arm visible
};

var EHeroBodySetup BodySetup;

//
// AI Positioning
//

struct native AIPositionPoint
{
	var vector Location;
	var vector LocationOnNavMesh;
	var array<OLEnemyPawn> PawnsOccupying;
	var bool bValid;
};

var array<AIPositionPoint> AIPositionPoints;

struct native AIPositionPawnInfo
{
	var OLEnemyPawn Enemy;
	var int PointIndex;
	var float TimeToNextChange;
};

var array<AIPositionPawnInfo> AIPositionPawns;

var int LastAIPositionChecked;

var const float NumAIPositions;
var const float AIPositionDistance;
var const float AIPositionLayerBuffer;
var const int NumAIPositionsPerSidePerPawn;
var const int NumAIPositionsToCheckPerFrame;

var vector AvgVelocity;
var array<vector2d> AvgVelPoints;
var float AvgVelocityTimer;

var const float NumAvgVelPoints;
var const float AvgVelPointLength;

var bool bWasUnder;

//
// Exposed config
//

var config name FirstFingerlessCheckpoint;
var config name ShatteredCameraGlassCheckpoint;
var config name ITUniformCheckpoint;
var config name PrisonerUniformCheckpoint;

var config float NormalWalkSpeed;
var config float NormalRunSpeed;
var config float CrouchedSpeed;
var config float ElectrifiedSpeed;
var config float WaterWalkSpeed;
var config float WaterRunSpeed;
var config float LimpingWalkSpeed;
var config float HobblingWalkSpeed;
var config float HobblingRunSpeed;
var config float SpeedPenaltyBackwards; // pct of speed lost
var config float SpeedPenaltyStrafe; // pct of speed lost
var config float ForwardSpeedForJumpWalking;
var config float ForwardSpeedForJumpRunning;
var config float JumpClearanceWalking;
var config float JumpClearanceRunning;
var config float LandingPenaltyDuration;
var config float LandingSpeedModifier;
var config float ElectrifiedJumpDelay;
var config float ExternalImpulseDecelCoeff;
var config float ExternalImpulseMinVel;
var config float ExternalImpulseMaxVel;
var config float ExternalImpulseMaxVelCrouched;
var config float FallSpeedForDamage;
var config float FallSpeedForDeath;
var config float FallDamageExponent;

var config float DefaultFOV;
var config float RunningFOV;
var config float FOVApproachCoeffOpening;
var config float FOVApproachCoeffClosing;
var config float FOVApproachCoeffRun;
var config float FOVApproachCoeffWalk;

var float BatteryDuration;
var config float NrmBatteryDuration;
var config float HardBatteryDuration;

var config float CamcorderMinFOV;
var config float CamcorderMaxFOV;
var config float CamcorderNVMaxFOV;
var config float NVLightZoomedInInnerAngle;
var config float NVLightZoomedInOuterAngle;
var config float NVLightZoomedInRadius;
var config float NVLightZoomedInBrightness;
var config float DarkLightBrightnessDefault;
var config float DarkLightRadiusDefault;
var config float DarkLightBrightnessNoCamcorder;
var config float DarkLightRadiusNoCamcorder;
var config float DarkLightBrightnessBothHandsNeeded;
var config float DarkLightRadiusBothHandsNeeded;
var config float DarkLightBrightnessAttacked;
var config float DarkLightRadiusAttacked;
var config float DarkLightBrightnessParrying;
var config float DarkLightRadiusParrying;

var config float NVGlitchTimeThreshold;
var config float NVGlitchMaxDelayStart;
var config float NVGlitchMaxDelayEnd;
var config float NVGlitchMinDuration;
var config float NVGlitchMaxDuration;
var config float NVGlitchMaxLevel;

var config float HealthRegenRate; // HP/sec
var config float HealthRegenDelay; // delay after last damage before regen starts
var config float HobbleApproachRate; // pct / sec
var config float ElectricEffectPeriod;
var config float ElectricEffectBase;
var config int ElectricEffectMode;
var config float ElectricHurtSoundInterval;
var config float DeathScreenDuration;
var config int NumBloodyFootsteps;

var config float HeatDamageDist;
var config float HeatDamageInterval;
var config float HeatDamagePerSec;
var config float HeatMaxBlurDist;
var config float HeatMinBlurDist;
var config float HeatMinBlurAmount;
var config float HeatBlurApproachCoeffIn;
var config float HeatBlurApproachCoeffOut;

var config float MinCosAngleForPickup; // with the mouse, aim precisely
var config float PickupInteractRadius; // with a gamepad, must aim within this radius to interact

var config float JumpForwardFromLedgeWalkXYSpeed;
var config float JumpForwardFromLedgeWalkZSpeed;
var config float DropFromLedgeWalkXYSpeed;
var config float DropFromLedgeWalkZSpeed;

var config float LookBackCamRotOffset;
var config float LookBackCamBackOffset;
var config float LookBackCamSideOffset;

var config float LeanSpeedThreshold;

// Loudness
var config float WalkingLoudness;
var config float CrouchLoudness;
var config float RunningLoudness;
var config float WalkingWaterLoudness;
var config float CrouchWaterLoudness;
var config float HobblingWalkLoudness;
var config float HobblingRunLoudness;

var config float LandingBigLoudness;
var config float LandingSmallLoudness;

var config float LandingBigWaterLoudness;
var config float LandingSmallWaterLoudness;

var config float DoorOpenInstantLoudness;
var config float DoorOpenPartialLoudness;
var config float DoorCloseFastLoudness;
var config float DoorEnterLockerLoudness;
var config float DoorExitLockerLoudness;
var config float DoorRunThroughLoudness;

var config float MovingNoiseStartTime;
var config float MovingNoiseClearTime;

var config float PlayerSpeedRTPCApproachUp;
var config float PlayerSpeedRTPCApproachDown;

//
// Static config
//

var const name CrouchNoise;
var const name WalkingNoise;
var const name RunningNoise;

var const name DoorMajorNoise;

var const float MovingNoiseRate;

var const name WaterMaterial;
var const name BloodMaterial;
// Surface material detected at the last footstep (updated each step, read by multiplayer to sync dummy decals).
var name LastFootstepSurface;

var const ForceFeedbackWaveform BigLandingFFWaveform;
var const ForceFeedbackWaveform SmallLandingFFWaveform;
var const ForceFeedbackWaveform PickupFFWaveform;
var const ForceFeedbackWaveform DoorInteractionFFWaveform;
var const ForceFeedbackWaveform RunThroughDoorFFWaveform;

//
// Sound
//

var const AkEvent FootStepSound_Run;
var const AKEvent SndCamStart;
var const AKEvent SndCamStop;
var const AKEvent SndCamOnNVOn;
var const AKEvent SndCamOnNVOff;
var const AKEvent SndCamOffNVOn;
var const AKEvent SndCamOffNVOff;
var const AKEvent SndReloadBatteries;
var const AKEvent SndPickupDocument;
var const AKEvent SndPickupBatteries;
var const AKEvent SndZoomIn;
var const AKEvent SndZoomOut;
var const AKEvent SndSoldierHit;
var const AkEvent SndDie;
var const AkEvent SndHit;
var const AkEvent SndHitElectrified;
var const AkEvent SndStartPeek;
var const AkEvent SndStopPeek;
var const AkEvent SndStartDamage;
var const AkEvent SndStopDamage;
var const AkEvent SndElectricHitStart;
var const AkEvent SndElectricHitStop;
var const AkEvent SndStartRun;
var const AkEvent SndStopRun;
var const AkEvent SndStartLookBack;
var const AkEvent SndStopLookBack;
var const AkEvent SndSmallLanding;
var const AkEvent SndBigLanding;
var const AkEvent SndDieMusicEvent;
var const AkEvent SndNewObjective;
var const AkEvent SndRecordingCompleted;
var const AkEvent SndLowBatteryStart;
var const AkEvent SndLowBatteryStop;

var const string StateHitIntensityGroup;
var const string StateHitIntensityLow;
var const string StateHitIntensityMed;
var const string StateHitIntensityHigh;

var const string RTPCHealth;
var const string RTPCZoom;
var const string RTPCBatteryIntensity;
var const string RTPCPlayerSpeed;

//
// Includes
//

`define HERO_ANIMS_DECL
`include(OLHeroAnims.uci)
`undefine(HERO_ANIMS_DECL)

`define HERO_CONST_DECL
`include(OLHeroConstants.uci)
`undefine(HERO_CONST_DECL)

////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////

cpptext
{
	void ApplyNeckBoneRotation(INT CamPitchUNR);
	virtual FName GetMaterialBelowFeet();
	virtual void performPhysics(FLOAT deltaSeconds);
	virtual void SetBase(AActor *NewBase, FVector NewFloor = FVector(0,0,1), INT bNotifyActor=1, USkeletalMeshComponent* SkelComp=NULL, FName BoneName=NAME_None );
	virtual void stepUp(const FVector& GravDir, const FVector& DesiredDir, const FVector& Delta, FCheckResult &Hit);
	virtual void processLanded(FVector const& HitNormal, AActor *HitActor, FLOAT remainingTime, INT Iterations);
	virtual void physWalking(FLOAT deltaTime, INT Iterations);
	virtual void physCustom(FLOAT deltaTime, INT iterations);
	virtual void NotifyBump(AActor *Other, UPrimitiveComponent* OtherComp, const FVector &HitNormal);
	virtual void Crouch(INT bClientSimulation=0);
	virtual void UnCrouch(INT bClientSimulation=0);
	
	UBOOL TryPassObstacle(const FVector& playerIntentDirection=FVector(0.0f));
	UBOOL TryGrabLedge(const FVector& playerIntentVelocity);
	UBOOL TryGrabAndClimb(const FVector& playerIntentVelocity);
	UBOOL TryClimbUpLedge(UBOOL playerInteraction, const FVector& playerIntentVelocity);
	UBOOL TryEnterLedgeWalk(const FVector& playerIntentDirection);
	UBOOL TryEnterSqueeze(const FVector& playerIntentDirection, AOLGameplayVolume* gameplayVolume);
	UBOOL TryJump(const FVector& playerIntentVelocity);
	UBOOL TryCrouch();
	UBOOL TryUncrouch();
	UBOOL	TryDropFromLedge();
	UBOOL TryLean(FLOAT leanInput); // -1: left, 0: none, 1: right
	UBOOL TryLookBack(UBOOL bLeftSide);
	UBOOL TryStopLookBack();
	UBOOL TryLeanLeftPushing();
	UBOOL TryLeanRightPushing();	
	UBOOL TryExitSqueeze(const FVector& playerIntentDirection, UBOOL bForce = FALSE);
	UBOOL TryExitLedgeWalk();
	UBOOL TryLedgeTransition(const FVector& playerIntentDirection);
	UBOOL TryJumpFromLedgeWalk(UBOOL withForwardVelocity);
	UBOOL TryDoorInstantInteraction(UBOOL playerInteraction, const FVector& playerIntentDirection);
	UBOOL TryDoorInteractiveOpen();
	UBOOL TryToggleCamcorder();
	UBOOL TryObjectPickup(UBOOL playerInteraction);
	UBOOL TryCSA(UBOOL playerInteraction);
	UBOOL TryOpenAndEnterLocker(UBOOL playerInteraction);
	UBOOL TryOpenAndExitLocker(UBOOL playerInteraction);
	UBOOL TryEnterLocker(UBOOL playerInteraction, const AOLDoor* lockerDoor);	
	UBOOL TryEnterLadder(const FVector& playerIntentDirection);
	UBOOL TryExitLadder();
	UBOOL TryDropFromLadder();
	UBOOL TryGrabLadder(const FVector& playerIntentVelocity);
	UBOOL TryEnterBed(UBOOL playerInteraction);
	UBOOL TryPushObject(UBOOL playerInteraction);
	UBOOL TryPushFromLedge(const FVector& playerIntentVelocity);
	UBOOL TryEnterContextualLean(UBOOL wantsToLeanLeft, UBOOL wantsToLeanRight);
	
	void UpdateContextualLean(FLOAT leanInput, const FVector& playerIntentVelocity);
	void UpdateLookBackIntent(UBOOL bleanInput);

	void StopInteractiveOpen();
	void StopLeaning();	
	void StopPushing();
	void StopLeanPushing();
	void ActivateNightVision();
	void ZoomImpulse(FLOAT impulse);
	void StartedActiveZoom(UBOOL bZoomingIn);
	void SetTargetZoom(FLOAT newTarget);
	void TryToggleNightVision();
	void ForceActivateNightVision();
	void ForceDeactivateNightVision();
	void ActivateRainEffect();
	void DeactivateRainEffect();
	
	void NotifyDoorClosing(AOLDoor* door); // called when a kismet action closes a door
	void StartStruggle(const FStruggleConfig& struggleConfig, const FVector& refLocation, const FVector& refDirection);
	void FinishStruggle(UBOOL bSucceeded);
	void SetHealth(FLOAT newHealth);
	void ActivateLeftHandIK(IKTargetType targetType, FLOAT duration, FLOAT fadeInTime, FLOAT fadeOutTime, const FVector& knobOffset);
	void AttachCSAProp(FLOAT duration, FLOAT blendInTime, FLOAT blendOutTime, const FVector& positionOffset, const FRotator& orientationOffset, UBOOL bDiscardCurrentOffsets, UBOOL bHideWhenDone);
	void GiveCamcorder(UBOOL andUseRightAway, UBOOL bWithNV);
	void RemoveCamcorder();
	void CrackCameraGlass();
	void StartElectricGlitch(FLOAT glitchIntensity, FLOAT glitchFrequency, FLOAT glitchDuration);
	void SetDarkLightOverride(FLOAT brightness, FLOAT radius);
	void ResetDarkLightOverride();	
		
	void ApplyCheckpointState(const FName checkpointName);
	void UpdateCamera(FLOAT deltaTime);
	virtual void UpdateEyeHeight(FLOAT deltaTime) {}
	
	UBOOL IsPlayerInputEnabled() const;
	UBOOL IsCamcorderActive() const { return CamcorderState == CCS_Active; }
	UBOOL IsCamcorderInactive() const { return CamcorderState == CCS_Inactive; }
	UBOOL IsInCamcorderTransition() const { return CamcorderState != CCS_Active && CamcorderState != CCS_Inactive; }
	UBOOL IsInNightVision() const { return CamcorderState == CCS_Active && (CamcorderMode == CCM_NightVision || CamcorderMode == CCM_PoweredNightVision); }
	UBOOL IsReloading() const { return CamcorderState == CCS_ReloadingActive || CamcorderState == CCS_ReloadingInactive; }
	UBOOL IsPeeking() const;
	UBOOL IsInGrabbableState() const { return LocomotionMode == LM_Walk; }
	UBOOL IsCornerPeeking() const;
	UBOOL IsInWaterVolume();
	UBOOL IsInHeatVolume();
	UBOOL IsInDarkness();
	UBOOL IsConsideredLookingBack(); // LM_LookBack plus some smoothing around small steps
	UBOOL IsBeingChased();

	UBOOL CanRun() const;
	UBOOL CanCrouch() const;
	virtual UBOOL CanUncrouch();
	UBOOL CanReloadBatteries() const;
	UBOOL CanLookBack();
	UBOOL CanZoom();
	
	void TriggerSoundEvent(UAkEvent* sndEvent);
	void SetAudioState(const FString& stateName, const FString& newState);
	void SetAudioValue(const FString& valueName, FLOAT newValue);
	AOLLedgeMarker* FindClosestLedge(FLOAT minRelZ, FLOAT maxRelZ, FLOAT maxFwdDist);	
	ELocomotionMode GetNextLocomotionMode(ESpecialMoveType moveType);

	FVector GetAIPosition(AOLEnemyPawn* EnemyPawn);
	void DrawDebugAIPositions();

	FVector GetDisturbanceLocation(AOLEnemyPawn* Enemy);

protected:
	virtual void TickPrePhysics(FLOAT deltaTime);
	virtual void TickPostPhysics(FLOAT deltaTime);
	virtual void CalcVelocity(FVector &AccelDir, FLOAT DeltaTime, FLOAT MaxSpeed, FLOAT Friction, INT bFluid, INT bBrake, INT bBuoyant);
	virtual void BuildAnimSetList();
	virtual void AddAnimSets(const TArray<UAnimSet*>& CustomAnimSets);

	virtual FLOAT PlayFullBodyAnim(const FName& animName, FLOAT playRate=1.0f, FLOAT blendInTime=0.25f, FLOAT blendOutTime=0.25f, FLOAT startTime=0.0f, FLOAT endTime=0.0f);
	virtual FLOAT PlayBlendedAnim(const FName& animNameA, const FName& animNameB, FLOAT weightA, FLOAT blendInTime=0.0f, FLOAT blendOutTime=0, FLOAT rate=1.0f, FLOAT startRatio=0.0f);
	virtual FLOAT PlayBlendedAnim3(const FName& animNameA, const FName& animNameB, const FName& animNameC, FLOAT blendWeightA, FLOAT blendWeightB, FLOAT blendInTime=0.0f, FLOAT blendOutTime=0, FLOAT rate=1.0f, FLOAT startRatio=0.0f);
	virtual FLOAT PlayBlendSpace(const FBlendSpaceNode nodes[], INT numNodes, const INT blendSpaces[][3], INT numSpaces, const FVector2D& coord, FLOAT blendInTime=0.0f, FLOAT blendOutTime=0.0f, FLOAT rate=1.0f, FLOAT startRatio=0.0f);
	virtual void PlayRightArmAnim(const FName& animName, FLOAT playRate=1.0f, FLOAT blendInTime=0.25f, FLOAT blendOutTime=0.25f, FLOAT startTime=0.0f, FLOAT endTime=0.0f);
	virtual void PlayLeftArmAnim(const FName& animName, FLOAT playRate=1.0f, FLOAT blendInTime=0.25f, FLOAT blendOutTime=0.25f, FLOAT startTime=0.0f, FLOAT endTime=0.0f);
	virtual void PlayShadowedLeftArmAnim(const FName& animNameCamSpace, const FName& animNameShadow, FLOAT playRate=1.0f, FLOAT blendInTime=0.25f, FLOAT blendOutTime=0.25f, FLOAT startTime=0.0f, FLOAT endTime=0.0f);
	virtual void PlayCamSpaceAnim(const FName& animName, FLOAT playRate=1.0f, FLOAT blendInTime=0.25f, FLOAT blendOutTime=0.25f, FLOAT startRatio=0.0f);
	virtual void PlayUpperBodyAnim(const FName& animName, FLOAT blendInTime=0.0f, FLOAT blendOutTime=0, FLOAT rate=1.0f, FLOAT startRatio=0.0f);
	virtual void PlayUpperBodyBlendedAnim(const FName& animNameA, const FName& animNameB, FLOAT weightA, FLOAT blendInTime=0.0f, FLOAT blendOutTime=0, FLOAT rate=1.0f, FLOAT startRatio=0.0f);
	
	// Verify if the pawn state allows the specified move
	virtual UBOOL CouldStartSpecialMove(ESpecialMoveType moveType);
	virtual void ApplySpecialMoveParams(ESpecialMoveType moveType);
	virtual void PlayAnimForSpecialMove(ESpecialMoveType moveType);
	virtual UBOOL IsSpecialMoveCompleted();
	virtual void SpecialMoveCompleted();
	virtual void SpecialMovePhaseCompleted();
	virtual void CancelSpecialMove();
	virtual void UpdateSpecialMove(FLOAT deltaTime);

	virtual void OnCrouch();
	virtual void OnUncrouch();

	virtual UBOOL UseFootPlacementThisTick() { return FALSE; }

private:
	// Internal updates
	void UpdateHealth(FLOAT deltaTime);
	void UpdateCachedObjects();
	void UpdateCamcorder(FLOAT deltaTime);
	void UpdateNightVision(FLOAT deltaTime);
	void UpdateRainEffect(FLOAT deltaTime);
	void UpdateCameraEffect(FLOAT deltaTime);
	void UpdateElectricGlitch(FLOAT deltaTime);
	void UpdateHeroControl(FLOAT deltaTime);
	void UpdateDoorInteraction();
	void UpdateBedAnimation(FLOAT deltaTime);
	void UpdateNVGlitch(FLOAT deltaTime);
	void UpdateFootPlacement(FLOAT deltaTime);
	void UpdateMeshOffset(FLOAT deltaTime);
	void UpdateCornerPeek(FLOAT deltaTime);
	void UpdateHandIK(FLOAT deltaTime);
	void UpdateAttachments(FLOAT deltaTime);
	void UpdateGroundSpeed(FLOAT deltaTime, const FVector &AccelDir);
	void UpdateBreath(FLOAT deltaTime);
	void UpdateHeatShielding(FLOAT deltaTime);
	void UpdateParrying(FLOAT deltaTime);
	void UpdateMovingNoise(FLOAT deltaTime);
	void UpdateRecording(FLOAT deltaTime);
	void UpdateAvgVelocity(FLOAT deltaTime);
	void UpdateRTPCs(FLOAT deltaTime);
	
	// Camcorder
	void ActivateCamcorder();
	void DeactivateCamcorder();	
	void ActivateCamcorderMode(ECamcorderMode prevCamcorderMode, ECamcorderMode newCamcorderMode);
	void DeactivateNightVision();
	void TriggerNVEvent(INT eventIdx);
	void CancelReload();
	void DeactivateCameraEffect();
	
	// Locomotion	
	UBOOL EnterLocomotionMode(ELocomotionMode newMode);
	UBOOL TryAdjustCollisionSizeForLocomotionMode(ELocomotionMode newMode);
	FLOAT GetCollisionRadiusForLocomotionMode(ELocomotionMode locoMode) const;
	FLOAT GetCollisionHeightForLocomotionMode(ELocomotionMode locoMode) const;
	void ApplyPositioningCorrection(FLOAT deltaTime);
	void ApplyExternalImpulse(FLOAT deltaTime);	
	void ApplyObstacleAvoidance(FLOAT deltaTime, const FVector& AccelDir);

	// Camera
	void ProcessRotation(FLOAT deltaTime);
	void ApplyDynamicCameraLimits(const FRotator& baseRot, const FCamView& viewWS, FViewLimits& CurrentLimits, FLOAT& minYawCS, FLOAT& maxYawCS, FLOAT& minPitchCS, FLOAT& maxPitchCS, FLOAT& minPitchWS, FLOAT& maxPitchWS); // all in degrees, except baseRot
	void UpdateFOV(FLOAT deltaTime);
	
	// Misc
	void SetBodySetup(EHeroBodySetup shadowMode);
	void Die();
	void SyncShadowProxy();
	void AttachProp(UPrimitiveComponent* comp, FLOAT duration, FLOAT blendInTime, FLOAT blendOutTime, const FVector& positionOffset, const FRotator& orientationOffset, UBOOL bDiscardCurrentOffsets, UBOOL bHideWhenDone);
			
	void FinishSpecialMove(ESpecialMoveType completedMove, UBOOL bCancelling=FALSE);
	UBOOL TryPassObstacle(AOLLedgeMarker* node1, const FVector& playerIntentDirection);
	UBOOL TryGrabLedge(AOLLedgeMarker* node1, const FVector& playerIntentVelocity);
	UBOOL TryGrabAndClimb(AOLLedgeMarker* node1, const FVector& playerIntentVelocity);
	UBOOL TryPushObject(UBOOL playerInteraction, AOLPushableObject* pushable);	
	UBOOL TryLeanPushing(UBOOL bRight);	
	UBOOL AdjustJumpTargetToClearObstacle(FVector& out_JumpTarget, AOLLedgeMarker* node1, const FVector& startPoint, const FVector& endPoint);	
	void PlayGrabFromSqueezePhaseTwo();
	
	void GetSqueezeExitParams(const FVector& baseLoc, FVector& closestMarkerLoc, FVector& exitDir);
	ESqueezeTransitionType GetSqueezeExitType(UBOOL bAIAttackRight, const FVector& exitDir);
	AOLDoor* FindTargetDoor();	
	AOLLedgeMarker* FindFarEdge(FVector& farEdge, const FVector& targetPoint, const FVector& crossingDirection, AOLLedgeMarker* node1, AOLLedgeMarker* node2, FLOAT maxWidth);
	
	UBOOL IsRecordingMarkerInSight(AOLRecordingMarker* recordingMarker);
	
	void SetEnemyType(AOLEnemyPawn* attacker);

	void GenerateAIPositions();
	void UpdateAIPositions(FLOAT deltaTime);
	void UpdateAIPositionPawn(FLOAT deltaTime, INT PawnIndex, UBOOL bForce);
	void AddEnemyToPoint(AOLEnemyPawn* Enemy, INT PointIndex, INT Layer);
	void RemoveEnemyFromPoint(AOLEnemyPawn* Enemy, INT PointIndex);
	UBOOL CanEnemyOccupy(AOLEnemyPawn* Enemy, INT PointIndex, INT Layer);

	void UpdateCornerHandIK(FLOAT deltaTime);
	void SetCamParams(const FGameplayParams& gameplayParams);
	UBOOL HeadingLockedToCamera() const;

	const FGameplayParams& GP() const { return (SpecialMove != SMT_None) ? SpecialMoveParams[SpecialMove].GP : LocomotionModeParams[LocomotionMode].GP; }

	friend class UOLHeroCamera;
}

simulated native function Vector GetPawnViewLocation();
simulated native event rotator GetViewRotation();
native function Walk();
native function bool TryRun();
native function bool IsRunning();
native function ResetAfterTeleport();
native function NativePostBeginPlay();
native function GetCamera(out vector out_CamLoc, out rotator out_CamRot, out float out_FOV);
native function NativeDisplayDebug(Canvas canvas, out float out_YL, out float out_YPos);
native function NativeTakeDamage(int Damage, Controller InstigatedBy, vector HitLocation, class<DamageType> DamageType);
native function NativeTakeFallingDamage();
native function TakeElectricDamage(int Damage, float Knockback, vector hitNormal, AkEvent ElectricSoundEvent);
native function ActivateWaterFootstepParticles(bool bRightFoot);
native function OnPossess();
native function SetDoorAnimRatio(float Ratio, int OpeningType);
native function InitDummyDoorAnim(int OpeningType, float Ratio);
native function SyncCrouchPosture();
native function PlayShadowOnlyAnim(name AnimName, float Rate, float BlendIn, float BlendOut, optional bool bLooping);
native function PlayCinematicDummyAnim(name AnimName, float Rate, float BlendIn, float BlendOut);
native function SetFullBodyIdleAnim(name AnimName, float Rate);
native function ClearFullBodyIdleAnim();
native function ResetDummyAnimState();
native function SetDummyLocomotionMode(int NewMode);
native function PlayDummySMTAnim(int SMTType);
native function InitDummyMesh();
native function SetDummyCrouched(bool bCrouched);
native function SetDummyBedRelYaw(int RelYaw);
native function PlayDummyUpperBodyAnim(name AnimName, float Rate, float BlendIn, float BlendOut);
native function PlayDummyReloadAnim(name AnimName, float Rate, float BlendIn, float BlendOut);
native function StopDummyUpperBodyAnim(float BlendOut);
native function SetDummyUpperBodyIdleAnim(name AnimName, float Rate);
native function ClearDummyUpperBodyIdleAnim();
native function SetDummyLadderDelta(float Delta);
native function SetDummyLedgeWalkDelta(float Delta);
native function SetDummyLedgeHangDelta(float Delta);
native function float GetLedgeHangSignedDelta();
native function float GetLedgeWalkSignedDelta();
native function float GetSqueezeSignedDelta();
native function SetDummySqueezeDelta(float Delta);
native function int GetMeshPresetIndex();
native function name GetCurrentFullBodyAnimName();
native function SetDummyMeshPreset(int PresetIndex);
native function SetDummyHobblingState(bool bNewHobbling, float Intensity, float TargetIntensity);
native function float GetPeekingRatio();
native function SetDummyActiveDoor(OLDoor D);
native function SetDummyDoorOpeningType(int OpeningType);
native function SetDummyDoorParams(int OpeningType, int PartialOpenType, int ClosingType, bool bQuiet);
native function SetDummyPickupParams(float DistHorz, float DistVert, bool bCrouched, bool bDocument);
native function SetDummyActiveLocker(vector AnimPos);
native function SetDummyActiveBed(vector Pos);
native function SetDummyActivePushable(vector Pos);
native function SetDummyActiveSqueeze(vector AnimPos);
native function StartDummyGrabbedFromSqueeze(bool bAIAttackRight);
native function SetDummyKillParams(int InEnemyType, int InEnemyWeapon, bool bInBackAnim, bool bInLeftAnim, float InBlendAlpha);
native function vector GetRootBoneWorldLocation();
native function SetDummyRootMotionMode(bool bTranslate);
native function SetDummyHeadPitch(int CamPitchUNR, int CamYawUNR);

native function StartSpecialMove(ESpecialMoveType moveType, optional vector targetPosition, optional vector targetDirection, optional EAdjustPositionTargetType targetType);
native function EnterCinematicMode(SeqAct_ToggleCinematicMode seqAction);
native function ExitCinematicMode(SeqAct_ToggleCinematicMode seqAction);
native function bool HeroControlActivated(OLSeqAct_HeroControl heroControlAction);
native function bool TryGrabFromSqueeze(OLEnemyPawn attacker);
native function bool CanGrabFromSqueeze();

native function bool TryGrabFromLocker(OLEnemyPawn attacker);
native function bool TryExitLocker();
native function bool TryExitBed(vector playerIntentDirection);
native function bool TryGrabFromBed(OLEnemyPawn attacker);
native function bool TryGrabNormal(OLEnemyPawn attacker, vector startLocation, vector AttackStartLocation);
native function bool TryThrowPlayer(OLEnemyPawn attacker, float throwRotation);
native function bool TryGrabFromUnder(OLEnemyPawn attacker, vector startLocation, vector AttackStartLocation);
native function bool TryDecapitate(OLEnemyPawn attacker);
native function bool TryKillHero(OLEnemyPawn attacker, vector enemyStartLocation, vector direction);
native function bool TryKillInLocker(OLEnemyPawn attacker);
native function ReactToHit(float hitStrength, vector hitDirection, optional bool bForceFullPower);

// Notifies
native function PickupNotify();
native function AttachPickupMeshToDummyHand(OLPickableObject Pickup);
native function DoorAnimNotify();
native function CamcorderAvailableNotify();
native function CamcorderRaisedNotify();
native function CamcorderLoweredNotify();
native function bool TryRaiseCamcorder();
native function LowerCamcorder();
native function BatteriesReloadedNotify();
native function ReloadBatteries();
native function OverrideCameraSettingsNotify(OLAnimNotify_OverrideCameraParams camParamsNotify);
native function DieNotify();
native function DecapitatedNotify();
native function BloodOnScreenNotify();
native function ResetNeckOffsetNotify(); // only valid in cinematic mode
native function SetCamcorderVisibleNotify(); // for reload inactive in squeeze and bed
native function AttachCameraEffect(ParticleSystem ParticleEffectTemplate, float Duration, float PlaneDist);

native function bool IsInLocker();
native function bool IsBarefeet();

native function bool CanBeAttacked();
native function bool CanBeFatalitized();
native function bool CanBeGrabbedUnder();

exec native function QuickHeroTest();

native function vector GetFutureDestination(OLPawn Agent);
native function vector GetGrabUnderDestination(OLEnemyPawn Enemy);
native function RemoveEnemyFromAIPositionPawns(OLEnemyPawn EnemyPawn);

////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////

function PossessedBy(Controller C, bool bVehicleTransition)
{
	Super.PossessedBy(C, bVehicleTransition);
	OLPC = OLPlayerController(C);

	if (OLPC == None)
		return;

	UpdateDifficultyBasedValues();
	OLPC.UpdateDifficultyBasedValues();

	OnPossess();
}

simulated event PostInitAnimTree(SkeletalMeshComponent SkelComp)
{
	if (SkelComp == Mesh)
	{
		UpperBodyBlendNode = OLAnimCustomBlend(Mesh.FindAnimNode('UpperBodyBlend'));		
		RightArmAnimSlot = AnimNodeSlot(Mesh.FindAnimNode('RightArmAnimSlot'));
		LeftArmAnimSlot = AnimNodeSlot(Mesh.FindAnimNode('LeftArmAnimSlot'));
		SqueezeAnimNode = OLAnimConstrainedMovement(Mesh.FindAnimNode('Squeeze'));
		LedgeWalkAnimNode = OLAnimConstrainedMovement(Mesh.FindAnimNode('LedgeWalk'));
		LedgeHangAnimNode = OLAnimConstrainedMovement(Mesh.FindAnimNode('LedgeHang'));
		LadderAnimNode = OLAnimMultiCycleConstrainedMovement(Mesh.FindAnimNode('Ladder'));
		DoorAnimNode = OLAnimDoorInteraction(Mesh.FindAnimNode('Door'));
		PeekingAnimNode = OLAnimPeeking(Mesh.FindAnimNode('Peeking'));
		LocomotionAnimNode = OLAnimBlendByLocomotionMode(Mesh.FindAnimNode('LocomotionMode'));
		BlendByPostureWalkingAnimNode = OLAnimBlendByPosture(Mesh.FindAnimNode('BlendByPostureWalking'));
		BlendByPostureFallingAnimNode = OLAnimBlendByPosture(Mesh.FindAnimNode('BlendByPostureFalling'));
		LeftArmWallContactFilterNode = AnimNodeBlendPerBone(Mesh.FindAnimNode('LeftArmWallContact'));
		LeftArmWallContactAnimSequence = AnimNodeSequence(Mesh.FindAnimNode('WallContactSlot'));
		BedAnimNode = AnimNodeSequence(Mesh.FindAnimNode('Bed'));
		RightHandIK = SkelControlLimb(Mesh.FindSkelControl('RightHandIK'));
		LeftHandIK = SkelControlLimb(Mesh.FindSkelControl('LeftHandIK'));
		LeftFootPlacementNode = AnimNodeBlendPerBone(Mesh.FindAnimNode('LeftFootPlacement'));
		RightFootPlacementNode = AnimNodeBlendPerBone(Mesh.FindAnimNode('RightFootPlacement'));
		HiddenRightArmControl = SkelControlSingleBone(Mesh.FindSkelControl('HiddenRightArm'));
		HiddenLeftArmControl = SkelControlSingleBone(Mesh.FindSkelControl('HiddenLeftArm'));
		if (OLPC == None)
		{
			// IK controls for the dummy are driven by UpdateHandIK (called in TickPostPhysics).
			if (RightHandIK != None)
				RightHandIK.SetSkelControlActive(false);
			if (LeftHandIK != None)
				LeftHandIK.SetSkelControlActive(false);
		}
		StruggleAnimNode = OLAnimStruggleCycle(Mesh.FindAnimNode('Struggle'));
		CrouchTurnOnSpotAnimNode = OLAnimCrouchedTurnOnSpot(Mesh.FindAnimNode('CrouchedTurnOnSpot'));
		ParryingAnimNode = OLAnimParrying(Mesh.FindAnimNode('Parrying'));
		LeanStandingAnimNode = OLAnimBlendByLeaning(Mesh.FindAnimNode('LeanStanding'));
		LeanCrouchedAnimNode = OLAnimBlendByLeaning(Mesh.FindAnimNode('LeanCrouched'));
		LeftForeTwistControl = (Mesh.FindSkelControl('LeftForeTwist'));
		LeftForeTwist1Control = (Mesh.FindSkelControl('LeftForeTwist1'));
		LeftUpArmTwistControl = (Mesh.FindSkelControl('LeftUpArmTwist'));		
		LeftHandIK.ControlStrength = 0.0;
		LeftForeTwistControl.ControlStrength = 0.0;
		LeftForeTwist1Control.ControlStrength = 0.0;
		LeftUpArmTwistControl.ControlStrength = 0.0;
		RightForeTwistControl = (Mesh.FindSkelControl('RightForeTwist'));
		RightUpArmTwistControl = (Mesh.FindSkelControl('RightUpArmTwist'));		
		RightHandIK.ControlStrength = 0.0;
		RightForeTwistControl.ControlStrength = 0.0;
		RightUpArmTwistControl.ControlStrength = 0.0;
	}
	else if (SkelComp == ShadowProxy)
	{
		ShadowProxyFullBodyAnimSlot = OLAnimNodeSlot(ShadowProxy.FindAnimNode('FullBodySlot'));
		ShadowProxyCustomBlendNode = OLAnimCustomBlend(ShadowProxy.FindAnimNode('CustomBlend'));
		ShadowProxyUpperBodyBlendNode = OLAnimCustomBlend(ShadowProxy.FindAnimNode('UpperBodyBlend'));		
		ShadowProxyRightArmAnimSlot = AnimNodeSlot(ShadowProxy.FindAnimNode('RightArmAnimSlot'));
		ShadowProxyLeftArmAnimSlot = AnimNodeSlot(ShadowProxy.FindAnimNode('LeftArmAnimSlot'));	
		ShadowProxySqueezeAnimNode = OLAnimConstrainedMovement(ShadowProxy.FindAnimNode('Squeeze'));
		ShadowProxyLedgeWalkAnimNode = OLAnimConstrainedMovement(ShadowProxy.FindAnimNode('LedgeWalk'));
		ShadowProxyLedgeHangAnimNode = OLAnimConstrainedMovement(ShadowProxy.FindAnimNode('LedgeHang'));
		ShadowProxyLadderAnimNode = OLAnimMultiCycleConstrainedMovement(ShadowProxy.FindAnimNode('Ladder'));
		ShadowProxyDoorAnimNode = OLAnimDoorInteraction(ShadowProxy.FindAnimNode('Door'));
		ShadowProxyPeekingAnimNode = OLAnimPeeking(ShadowProxy.FindAnimNode('Peeking'));
		ShadowProxyLeftArmWallContactFilterNode = AnimNodeBlendPerBone(ShadowProxy.FindAnimNode('LeftArmWallContact'));
		ShadowProxyLeftArmWallContactAnimSequence = AnimNodeSequence(ShadowProxy.FindAnimNode('WallContactSlot'));
		ShadowProxyBedAnimNode = AnimNodeSequence(ShadowProxy.FindAnimNode('Bed'));
		ShadowProxyLeftHandIK = SkelControlLimb(ShadowProxy.FindSkelControl('LeftHandIK'));
		ShadowProxyLeftFootPlacementNode = AnimNodeBlendPerBone(ShadowProxy.FindAnimNode('LeftFootPlacement'));
		ShadowProxyRightFootPlacementNode = AnimNodeBlendPerBone(ShadowProxy.FindAnimNode('RightFootPlacement'));
		ShadowProxyStruggleAnimNode = OLAnimStruggleCycle(ShadowProxy.FindAnimNode('Struggle'));
		ShadowProxyCrouchTurnOnSpotAnimNode = OLAnimCrouchedTurnOnSpot(ShadowProxy.FindAnimNode('CrouchedTurnOnSpot'));
		ShadowProxyParryingAnimNode = OLAnimParrying(ShadowProxy.FindAnimNode('Parrying'));
		ShadowProxyLeftForeTwistControl = (ShadowProxy.FindSkelControl('LeftForeTwist'));
		ShadowProxyLeftForeTwist1Control = (ShadowProxy.FindSkelControl('LeftForeTwist1'));
		ShadowProxyLeftUpArmTwistControl = (ShadowProxy.FindSkelControl('LeftUpArmTwist'));
		ShadowProxyRightHandIK = SkelControlLimb(ShadowProxy.FindSkelControl('ShadowProxyRightHandIK'));
		ShadowProxyRightForeTwistControl = (ShadowProxy.FindSkelControl('ShadowProxyRightForeTwist'));
		ShadowProxyRightUpArmTwistControl = (ShadowProxy.FindSkelControl('ShadowProxyRightUpArmTwist'));
		ShadowProxyRightClavicleFixup = (ShadowProxy.FindSkelControl('ShadowProxyRightClavicleFixup'));

		ShadowProxyLeftHandIK.ControlStrength = 0.0;		
		ShadowProxyLeftForeTwistControl.ControlStrength = 0.0;
		ShadowProxyLeftForeTwist1Control.ControlStrength = 0.0;
		ShadowProxyLeftUpArmTwistControl.ControlStrength = 0.0;
		ShadowProxyRightHandIK.ControlStrength = 0.0;		
		ShadowProxyRightForeTwistControl.ControlStrength = 0.0;
		ShadowProxyRightUpArmTwistControl.ControlStrength = 0.0;
		ShadowProxyRightClavicleFixup.ControlStrength = 0.0;
	}

	Super.PostInitAnimTree(SkelComp);
}

simulated function DisplayDebug(HUD HUD, out float out_YL, out float out_YPos)
{
	if (OLPC.HUD.ShowingFullScreenOverlay())
	{
		return;
	}

	Super.DisplayDebug(HUD, out_YL, out_YPos);
	NativeDisplayDebug(HUD.Canvas, out_YL, out_YPos);
}

event TakeDamage(int Damage, Controller InstigatedBy, vector HitLocation, vector Momentum, class<DamageType> DamageType, optional TraceHitInfo HitInfo, optional Actor DamageCauser)
{
	if (bIsDummyPawn)
		return;
	NativeTakeDamage(Damage, InstigatedBy, HitLocation, DamageType);
}

event bool HealDamage(int Amount, Controller Healer, class<DamageType> DamageType)
{
	// not if already dead or already at full
	if (Health > 0 && Health < 100)
	{
		Health = Min(100, Health + Amount);
		PreciseHealth = Health;
		return true;
	}
	else
	{
		return false;
	}
}

function TakeFallingDamage()
{
	NativeTakeFallingDamage();
}

native function NativePlayLanded(float impactVel);
function PlayLanded(float impactVel)
{
	Super.PlayLanded(impactVel);
	NativePlayLanded(impactVel);
}

native function OnEnterDeepWater(PhysicsVolume NewVolume);
event PhysicsVolumeChange(PhysicsVolume NewVolume)
{
	if (NewVolume != None && NewVolume.bWaistDeepWater)
	{
		OnEnterDeepWater(NewVolume);
	}
}

/* BecomeViewTarget
	Called by Camera when this actor becomes its ViewTarget */
simulated event BecomeViewTarget( PlayerController PC )
{
	Super.BecomeViewTarget(PC);

	if (LocalPlayer(PC.Player) != None)
	{
		PawnAmbientSound.bAllowSpatialization = false;		
	}
}

/* EndViewTarget
	Called by Camera when this actor becomes its ViewTarget */
simulated event EndViewTarget( PlayerController PC )
{
	PawnAmbientSound.bAllowSpatialization = true;
}

/** sets whether or not the owner of this pawn can see it */
simulated function SetMeshVisibility(bool bVisible)
{
	// Handle the main player mesh
	if (Mesh != None)
	{
		Mesh.SetOwnerNoSee(!bVisible);
	}
}

native function GetFootstepDecalTransform(bool bLeftFoot, out vector DecalLocation, out rotator DecalRotation);

function SpawnBloodFootstepDecal(bool bLeftFoot, float Alpha)
{
	local MaterialInstanceConstant ParentMI;
	local MaterialInstanceConstant DecalMI;
	local vector DecalLocation;
	local rotator DecalRotation;

	GetFootstepDecalTransform(bLeftFoot, DecalLocation, DecalRotation);

	if (IsBarefeet())
	{
		if (bLeftFoot)
		{
			if (Rand(2) == 0)
			{
				ParentMI = FootstepBarefeetDecalMatL1;
			}
			else
			{
				ParentMI = FootstepBarefeetDecalMatL2;
			}
		}
		else
		{
			if (Rand(2) == 0)
			{
				ParentMI = FootstepBarefeetDecalMatR1;
			}
			else
			{
				ParentMI = FootstepBarefeetDecalMatR2;
			}
		}
	}
	else
	{
		if (bLeftFoot)
		{
			if (Rand(2) == 0)
			{
				ParentMI = FootstepDecalMatL1;
			}
			else
			{
				ParentMI = FootstepDecalMatL2;
			}
		}
		else
		{
			if (Rand(2) == 0)
			{
				ParentMI = FootstepDecalMatR1;
			}
			else
			{
				ParentMI = FootstepDecalMatR2;
			}
		}
	}

	DecalMI = new(self) class'MaterialInstanceConstant';
	DecalMI.SetParent(ParentMI);	
	DecalMI.SetScalarParameterValue('Opacity', Alpha);

	WorldInfo.MyDecalManager.SpawnDecal(DecalMI, DecalLocation, DecalRotation, 18.75, 37.5, 5.0, false, 0, None, false, false, , , , 90.0);
}

native function bool IsInMainMenu();
simulated event PlayFootStepSound(int FootDown, AnimNotify_Footstep footstepNotify)
{
	local name SurfaceMat;
	local float DecalAlpha;

	if (!bIsDummyPawn && OLPC == None)
		return;

	if (WorldInfo.TimeSeconds < (LastFootstepTime + 0.2) || WorldInfo.TimeSeconds < (SpawnTime + 0.75) || (!bIsDummyPawn && IsInMainMenu()))
	{
		// Ignore - we may be getting multiple footsteps due to anim blending
		return;
	}

	if (bIsDummyPawn || VSizeSq2D(OLPC.LastPlayerInputIntent) > 25.0 || footstepNotify.bNoVelocityRequired)
	{
		LastFootstepTime = WorldInfo.TimeSeconds;
		LastFootDown = FootDown;

		SurfaceMat = GetMaterialBelowFeet();
		LastFootstepSurface = SurfaceMat;
		SetSwitch(FloorMaterialGroup, SurfaceMat);

		if (footstepNotify.bForceRunEvent || (IsRunning() && !footstepNotify.bForceWalkEvent))
		{
			PlayAkEvent(FootStepSound_Run);
		}
		else
		{
			PlayAkEvent(FootStepSound_Walk);
		}

		if (WorldInfo.GetDetailMode() != DM_Low)
		{
			if (SurfaceMat == WaterMaterial)
			{
				ActivateWaterFootstepParticles(FootDown == 1);
			}
			else if (SurfaceMat == BloodMaterial)
			{
				RemainingBloodyFootsteps = NumBloodyFootsteps;
			}

			if (RemainingBloodyFootsteps > 0)
			{
				if (RemainingBloodyFootsteps > 0.75*NumBloodyFootsteps)
				{
					DecalAlpha = 1.0;
				}
				else
				{
					DecalAlpha = RemainingBloodyFootsteps / (0.75*NumBloodyFootsteps);
				}
				SpawnBloodFootstepDecal(FootDown == 0, DecalAlpha);
				RemainingBloodyFootsteps--;
			}
		}
	}
}

event RespawnHero()
{
	local OLGame TheGame;
	local OLEngine Engine;

	if (OLPC != None)
	{
		TheGame = OLGame(WorldInfo.Game);

		if (TheGame != None && TheGame.DifficultyMode == EDM_Insane)
		{
			// start at the beginning
			if (class'OLUtils'.static.IsPlayingDLC())
			{
				OLPC.StartNewGameAtCheckpoint("Hospital_Start", false);
			}
			else
			{				
				OLPC.StartNewGameAtCheckpoint("Admin_Gates", false);
			}
			OLPC.bForceLevelReset = true;
		}
		else
		{
			OLPC.UnPossess();

			Destroy();

			OLPC.PlayerDied();

			class'OLGameStateList'.static.OnPlayerDeath();

			Engine = OLEngine(class'Engine'.static.GetEngine());
			TheGame = OLGame(WorldInfo.Game);
					
			if (Engine != None && Engine.CurrentCheckpointSave != None)
			{
				Engine.StartCurrentCheckpoint();
			}
			else
			{
				WorldInfo.Game.RestartPlayer( OLPC );
				OLPC.ClientSetCameraFade(false);
			}
		}
	}
}

simulated event FellOutOfWorld(class<DamageType> dmgType)
{
	RespawnHero();
}

simulated singular event OutsideWorldBounds()
{
	RespawnHero();
}

function UpdateDifficultyBasedValues()
{
	local OLGame TheGame;

	TheGame = OLGame(WorldInfo.Game);
	if (TheGame == None || TheGame.DifficultyMode == EDM_Normal)
	{
		BatteryDuration = NrmBatteryDuration;		
	}
	else
	{
		BatteryDuration = HardBatteryDuration;
	}
}

event Landed(Vector HitNormal, Actor FloorActor)
{
    if (OLPC != None && !OLPC.bBunnyHop)
    {
        Super.Landed(HitNormal, FloorActor);
    }
	TakeFallingDamage();
}

////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////

defaultproperties
{	
	`include(OLHeroComponents.uci)
	`include(OLHeroData.uci)
	`include(OLHeroAnims.uci)
	`include(OLHeroConstants.uci)
	
	
	Begin Object Name=CollisionCylinder
		CollisionRadius=30.0
		CollisionHeight=82.5
		Translation=(Z=87.5)
	End Object


////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////

	HeadBoneName=Hero-Head
	LeftHandBoneName=Hero-L-Hand
	LeftHandAuxBoneName=Hero-L-Hand_aux
	RightHandAuxBoneName=Hero-R-Hand_aux
		
	CrouchHeight=37.5
	CrouchRadius=30.0
	CrouchedPct=1.0
	AirControl=0.0
	DefaultAirControl=0.0

	ViewPitchMin=-13800
	ViewPitchMax=10000

	MaxStepHeight=30.0
	MaxJumpHeight=110.0
		
	CrouchNoise=Crouch
	WalkingNoise=Walking
	RunningNoise=Running
	DoorMajorNoise=DoorMajor

	MovingNoiseRate=0.033f

	WaterMaterial=Water
	BloodMaterial=Blood

	NumAIPositions=64
	AIPositionDistance=130.0f
	AIPositionLayerBuffer=70.0f
	NumAIPositionsPerSidePerPawn=3
	NumAIPositionsToCheckPerFrame=8

	NumAvgVelPoints=10
	AvgVelPointLength=1.0f

	AccelApproachFactor=0.85
	DecelApproachFactor=0.85

	bFilterAnimEndNotifies=true

	OverriddenMinCamcorderFOV=-1.0
	
	SndCamStart=AkEvent'Player_Sound.CAM_Start_Using'
	SndCamStop=AkEvent'Player_Sound.CAM_Stop_Using'
	SndCamOnNVOn=AkEvent'Player_Sound.CAM_On_SwitchON_NV'
	SndCamOnNVOff=AkEvent'Player_Sound.CAM_On_SwitchOFF_NV'
	SndCamOffNVOn=AkEvent'Player_Sound.CAM_Off_SwitchON_NV'
	SndCamOffNVOff=AkEvent'Player_Sound.CAM_Off_SwitchOFF_NV'	
	SndReloadBatteries=AkEvent'Player_Sound.CAM_Reload_Battery'
	SndPickupDocument=AkEvent'Player_Sound.Pick_Up_Document'
	SndPickupBatteries=AkEvent'Player_Sound.Pick_Up_Battery'
	SndZoomIn=AkEvent'General_Sound.EV_Zoom_IN_Cam'
	SndZoomOut=AkEvent'General_Sound.EV_Zoom_OUT_Cam'
	SndSoldierHit=AkEvent'General_Sound.EV_Soldier_Hit'
	SndHit=AkEvent'VO_Miles.VO_MILES_HIT_INTENSITY'
	SndHitElectrified=AkEvent'VO_Miles.VO_MILES_HIT_INTENSITY'
	FootStepSound_Run=AkEvent'Player_Sound.Footsteps_Run_Miles'
	SndDie=AkEvent'VO_Miles.VO_MILES_DIE'
	SndStartPeek=AkEvent'VO_Miles.VO_MILES_PEK_INSPI'
	SndStopPeek=AkEvent'VO_Miles.VO_MILES_PEK_EXPI'
	SndStartDamage=AkEvent'Player_Sound.DAMAGES_Health_Play'
	SndStopDamage=AkEvent'Player_Sound.DAMAGES_Health_Stop'
	SndElectricHitStart=AkEvent'SFX.Play_Loop_Electric'
	SndElectricHitStop=AkEvent'SFX.Stop_Loop_Electric'
	SndStartRun=AkEvent'VO_Miles.VO_MILES_RUN_BREATH'
	SndStopRun=AkEvent'VO_Miles.VO_MILES_RUN_STOP'
	SndDieMusicEvent=AkEvent'General_Sound.MUS_MILES_DEATH'
	SndStartLookBack=AkEvent'SFX.Play_WHOOSH_LOOKBACK'
	SndStopLookBack=AkEvent'SFX.Play_WHOOSH_LOOKFRONT'
	SndSmallLanding=AkEvent'Player_Sound.Landing_Soft_Miles'
	SndBigLanding=AkEvent'Player_Sound.Landing_Strong_Miles'
	SndNewObjective=AkEvent'General_Sound.SFX_SetObjectif'
	SndRecordingCompleted=AkEvent'General_Sound.SFX_RecordingMoment_Done'
	SndLowBatteryStart=AkEvent'Player_Sound.CAM_Light_Intensity'
	SndLowBatteryStop=AkEvent'Player_Sound.CAM_Light_Intensity_Stop'

	StateHitIntensityGroup="HIT_Intensity"
	StateHitIntensityLow="HIT_Low"
	StateHitIntensityMed="HIT_Medium"
	StateHitIntensityHigh="HIT_Hight"

	RTPCHealth="Health"
	RTPCZoom="Zoom"
	RTPCBatteryIntensity="Light_Cam_Intensity"
	RTPCPlayerSpeed="RTPC_PlayerSpeed"
}
