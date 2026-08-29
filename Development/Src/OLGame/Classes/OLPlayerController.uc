class OLPlayerController extends UDKPlayerController
	dependson(OLPawn)
	dependson(OLRecordingMarker)
	config(Game)
	native;

////////////////////////////////////////////////////////////////////////////////////////////
// Variables
////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////

var OLHero HeroPawn;
var OLHUD HUD;
var OLInventoryManager InventoryManager;
var OLTutorialManager TutorialManager;
var OLSoundEnvironmentManager SoundEnvManager;
var OLFXManager FXManager;

var OLSeqAct_SplashScreen ActiveSplashScreen;
var bool bFlushingStreaming;

var name CurrentObjective; // localized
var array<name> CompletedObjectives; // localized

var bool bHasCamcorder; // For the sequence where you lose the camcorder
var int NumBatteries; // in inventory, not counting the current set
var int MaxNumBatteries;

var config int NrmMaxNumBatteries;
var config int HardMaxNumBatteries;
var config int NightmareMaxNumBatteries;
var config int DefaultNumBatteries;

var float GameOverActivatedTimestamp;
var bool bBlockedOnLoading;
var bool bShowingKismetControlledLoadingScreen;
var bool bShowingLoadingOverlay;

enum EPlayerInteractionType
{
	PIT_CSA,
	PIT_PickupObject,
	PIT_BatteriesFull,
	PIT_EnterBed,
	PIT_EnterLocker,
	PIT_ExitLocker,
	PIT_OpenDoor,
	PIT_OpenPartiallyOpenDoor,
	PIT_CloseDoor,
	PIT_LockedDoor,
	PIT_AutoCloseDoor,	
	PIT_ClimbUpLedge,
	PIT_ReloadBatteries,	
	PIT_Struggle,
	PIT_PushObject,	
};

var array<EPlayerInteractionType> AvailableInteractions;
var string CSAPrompt; // localized
var string PickupTargetName; // localized

/** Whether we need to update the profile settings or not */
var bool bProfileSettingsUpdated;
var bool bTriggerActObserver;   // true during ObserverActivateTrigger to suppress local echo

var OLProfileSettings ProfileSettings;

//
// Input
//

var struct native DigitalInputs
{
	var bool bPerformedUseAction;
	var bool bPressedUseButton;
	var bool bReleasedUseButton;
	var bool bPressedToggleCamcorder;
	var bool bPressedToggleNightVision;
	var bool bPressedZoomIn;
	var bool bPressedZoomOut;
	var bool bPressedJump;
	var bool bPressedReloadBatteries;
	var bool bStartedActiveZoom;
	var bool bPressedPrototypeActionA;
	var bool bPressedPrototypeActionB;
	var bool bPressedPrototypeActionC;
	var bool bPressedPrototypeActionD;
	var bool bPressedPrototypeActionE;
} Inputs;

var input byte bLeanInputLeft;
var input byte bLeanInputRight;
var input byte bRunInput;
var input byte bUseButtonDown;
var input float PureMouseX;
var input float AnalogLeanInputLeft;
var input float AnalogLeanInputRight;
var int ZoomInput;

var bool bValidDoorHold;
var bool bInvalidateLeanInput;
var bool bInvalidateReleasedUse; // set when the interact input has been used on hold (so invalidate the release)
var float UsePressedTime;

var vector LastPlayerInputIntent; // world space intent

var bool bToggleCrouch; // user-config

enum ZoomMovementType 
{ 
	Zoom_Undefined,
	Zoom_MovingLeft, 
	Zoom_NotMoving, 
	Zoom_MovingRight 
};

//
// PS4 Controller
//

struct native TouchZoomData
{
	var float bActive;
	var float SmoothedPosition; // [0,1]
	var ZoomMovementType CurrentDirection; 

	// vars below are set directly by OLPlayerInput
	var float LastPosition; // [0,1]
	var float LastInputTime;
};

var TouchZoomData TouchZoom;

var LinearColor LightBarColor;
var float LightBarPulsePhase;

//
// Recording
//
var array<name> CompletedRecordingMoments;
var array<name> UnreadRecordingMoments;
var OLRecordingMarker PendingRecordingMarker;
var float RecordingCompletedTime;

//
// Struggle
//

enum StruggleInputDirection
{
	SID_Undefined,
	SID_Left,
	SID_Right,
};

struct native StruggleData
{
	var OLSeqAct_Struggle StruggleAct; // keeping it only to activate its output links when done
	var SkeletalMeshActor Enemy;
	var vector RefLocation;
	var vector RefDirection;
	var StruggleConfig Config;
	var OLAnimEnemyStruggle EnemyAnimNode;

	var bool bActiveStrugging; // set during the cycle phase
	var bool bSucceeded; // set when starting a success exit
	var float CycleStartedTime;
	var float NbShakes; // either direction. float for cooldown in no-fails
	var StruggleInputDirection CurrentInputDirection; // -1: left, 0: undefined, 1: right
	var float TotalDeltas;
	var float LastMouseX; // absolute (same units as PureMouseX)
	var float SmoothedDirection; // 1.0 - right, -1.0 - left
	var float SmoothedAnimPlayRate;
};

 var transient StruggleData Struggle;

 var config float StruggleInputThresholdForWin; // win points per second
 var config float StruggleShakesThresholdForWin; // shakes per second
 var config float StruggleInputThresholdForWinNoFail;
 var config float StruggleShakesThresholdForWinNoFail;

//
// Music
//

var enum EAIMusicState
{
	EAIMS_None,
	EAIMS_Patrol,
	EAIMS_Investigate,
	EAIMS_Chase,
} AIMusic;

var float AIDistance;

var bool bOverriddenMusic;
var EAIMusicState OverriddenMusicState;
var float OverriddenMusicDistance;

var float AIChaseMusicTimer;
var config float AIChaseMusicTimeDelay;

var const name MusicAIStateGroup;
var const name MusicAIStateNone;
var const name MusicAIStatePatrol;
var const name MusicAIStateInvestigate;
var const name MusicAIStateChase;

var const name LoadingStateGroup;
var const name LoadingStateOn;
var const name LoadingStateOff;

var const name AIDistanceRTPC;

var const name AstoundSoundGroup;
var const name AstoundSoundOn;
var const name AstoundSoundOff;

var const AkEvent SndResetMixStates;

//
// Checkpoint
//

var int NumDeathsSinceLastCheckpoint;
var bool bTravellingToCheckpoint;
var bool bTravelCheckPersistent;
var bool bForceLevelReset;
var float StableLevelsTimestamp;
var int NumBatteriesAtLastCheckpoint; // when saving upon Save & Quit, or dying in nightmare, reset the number of batteries to that of the last checkpoint
var float BatteryEnergyAtLastCheckpoint; // saved battery energy at the last checkpoint, used when spawning in nightmare

struct native CheckpointRecord
{
	var int CheckpointRecordVersion;

	// Version 1 or above

	var int NumBatteries;

	var name CurrentObjective; // localized
	var array<name> CompletedObjectives; // localized

	var array<name> CompletedRecordingMoments;
	var array<name> UnreadRecordingMoments;

	var array<name> ActivatedGameState;

	var array<name> CollectedDocuments;
	var array<name> UnreadDocuments;
	var array<vector> CollectedBatteries; // Prevent batteries from respawning if collected. Unfortunately we have no unique ID, so we identify them by world position.

	var bool bBatteryTutorialComplete;
	var bool bClimbUpTutorialComplete;
	var bool bDocumentTutorialComplete;

	var bool bFoundBySoldierWhileHidden;
	var bool bFoundBySurgeonWhileHidden;

	// Camera recording time (the "00:00:00:00" display in the top left)
	var float RecordingTimeSeconds;

	var int DifficultyMode;

	// Version 2 or above

	var float BatteryEnergy; // [0.0, 1.0]

	// Version 3 or above

	var bool bUsedHidingSpot; // Whether a bed or locker has been used
};

// rcharpentier - CheckpointRecord we shipped with on Steam, before adding the CheckpointRecordVersion field
struct native DeprecatedCheckpointRecord
{
	var int NumBatteries;

	var name CurrentObjective; // localized
	var array<name> CompletedObjectives; // localized

	var array<name> CompletedRecordingMoments;
	var array<name> UnreadRecordingMoments;

	var array<name> ActivatedGameState;

	var array<name> CollectedDocuments;
	var array<name> UnreadDocuments;
	var array<vector> CollectedBatteries; // Prevent batteries from respawning if collected. Unfortunately we have no unique ID, so we identify them by world position.

	var bool bBatteryTutorialComplete;
	var bool bClimbUpTutorialComplete;
	var bool bDocumentTutorialComplete;

	var bool bFoundBySoldierWhileHidden;
	var bool bFoundBySurgeonWhileHidden;

	// Camera recording time (the "00:00:00:00" display in the top left)
	var float RecordingTimeSeconds;
};

var bool bFoundBySoldierWhileHidden;
var bool bFoundBySurgeonWhileHidden;

var bool bUsedHidingSpot;
var bool bReloadedBatteries; // used in insane only, not saved in save games

var config name FirstSoldierFindableCheckpoint;
var config name FirstSurgeonFindableCheckpoint;

var int FindHiddenPlayerSkipCount; // when >0, must not find a hidden player (avoids finding him e.g. twice in a row)

//
// Systems
//
var vector LastLightOptimCamPos;
var config bool bEnableShadowOptimisation; // switch shadow type between modulate and normal
var config bool bEnableLightOptimisation; // enable to disable lights


var array<name> LevelsResetAfterPlayerDeath;

//
// Legacy UTPlayerController variables (needed?)
//

var vector DesiredLocation;

var float LastCameraTimeStamp; /** Used during matinee sequences */
var class<Camera> MatineeCameraClass;

var Actor CalcViewActor;
var vector CalcViewActorLocation;
var rotator CalcViewActorRotation;
var vector CalcViewLocation;
var rotator CalcViewRotation;

var globalconfig float	OnFootDefaultFOV;

//
// Achievements
//
enum EOutlastAchievement
{
	OA_000_StartedGenerator,
	OA_001_DrainedSewer,
	OA_002_StartedSprinklers,
	OA_003_FoundFemaleWardKey,
	OA_004_MilestoneCollectibles,
	OA_005_FinishedGame,
	OA_006_AllCollectibles,
	OA_007_FinishedNightmare,

	// DLC

	OA_008_TurnOffGas,
	OA_009_TurnOffPower,
	OA_010_FinishDLC,
	OA_011_FinishDLCInsane,
	OA_012_AllDLCDocs,
	OA_013_AllDLCRecordings,

	// Xbox extra

	OA_014_DropElevatorCorpse,
	OA_015_NoBedNoLocker,
	OA_016_InsaneNoBatteries,
};

//
// Debug
//

var bool		bBehindView;
var bool		bDebugFixedCam;
var bool		bDebugFreeCam;
var bool		bDebugGhost;
var bool		bBunnyHop;
var rotator	DebugCamRot;
var vector	DebugCamPos;
var float	DebugFreeCamSpeed;
var float	DebugFreeCamFOV;
var int		InspectorHitIndex;  // Current selection index for Smart Trace cycling

struct native InspectorHit
{
	var Actor                  HitActor;
	var StaticMeshComponent    HitSMC;
	var SkeletalMeshComponent  HitSKMC;
	var vector                 HitLoc;
	var float                  Dist;
};
var array<InspectorHit> InspectorHits;  // All hits along the ray this frame
var bool		bSlowDownFPS;
var float	SlowDownFactor;
// Set by MultiplayerController while applying a remote door event — suppresses OLSeqAct_Door actions.
var bool bApplyingRemoteDoorEvent;

// Placement mode — ghost-based object placement editor.
var bool   bPlacementMode;
var bool   bPlacementConfirmPending;  // set by C++ InputKey on LMB; consumed by caller each tick
var Actor  PlacementGhost;
var float  PlacementYaw;    // current yaw offset, rotated by mouse wheel


////////////////////////////////////////////////////////////////////////////////////////////
// Native functions
////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////

cpptext
{
	virtual void PostBeginPlay();
	virtual void BeginDestroy();
	virtual UBOOL HearSound(UAkBaseSoundObject* InSoundCue, AActor* SoundPlayer, const FVector& SoundLocation, UBOOL bStopWhenOwnerDestroyed);

	virtual UBOOL Tick( FLOAT DeltaSeconds, ELevelTick TickType );
	virtual UBOOL TickPaused(FLOAT DeltaSeconds);

	void StartTravelToCheckpoint();
	void ResetWorldState();
	void ApplyCheckpoint(const FName& checkpointName);
	void InitializeKismetSequence(USequence* levelSequence);
	void GetLocationsForStreamingVolumes(TArray<FVector>& viewLocations);
	virtual void ModifyPostProcessSettings(FPostProcessSettings& PPSettings) const;
	virtual void OverrideListenerLocation(FVector& inout_Location, FVector& inout_ProjUp, FVector& inout_ProjRight, FVector& inout_ProjFront) const;
	void StartBlockOnLoading();
	void StopBlockOnLoading();
	void KismetRequestedShowLoadingScreen();
	void KismetRequestedHideLoadingScreen();
	void LoadedLevelListChanged();
	void OnLevelBecomingVisible(ULevelStreaming* streamingLevel);
	void ResetLevelState(ULevel* level);

	void SetNewObjective(FName newObjective, UBOOL bForce=FALSE);
	void ShowSplashScreen(UOLSeqAct_SplashScreen* splashScreenAction);
	void HideSplashScreen();
	void AddAvailableInteraction(EPlayerInteractionType interactionType);
	void RecordingCompleted(AOLRecordingMarker* recordingMarker);
	void PauseGameOnLostFocus();

	void InitStruggleOnEnemy(UOLSeqAct_Struggle* struggleAct); // starts the idle on the enemy, does not activate any struggle sequence
	void ActivateStruggle(UOLSeqAct_Struggle* struggleAct);
	void StruggleEntryCompleted();
	void StruggleExitCompleted();

	void GameOver(); // temp for demo / vertical slice
	void CheckCollectibleAchievement(UBOOL bFoundDocument, const FString& collectibleName);
	void CheckForAchievementOnRemoteEvent(const FName& eventName);
	void TryDingoUnlockAchievement(EOutlastAchievement achievement);

	UBOOL IsUsingGamepad() { return PlayerInput->bUsingGamepad; }

protected:
	void NativeUpdateRotation(FLOAT deltaTime);

private:
	UBOOL ProcessPlayerActions(FLOAT deltaSeconds);
	void ProcessFreeCam(FLOAT deltaSeconds);

	void DrawDebug();

	void UpdateOverlay(FLOAT deltaSeconds);	
	void UpdateFade(FLOAT deltaSeconds);
	void UpdateTravel(FLOAT deltaSeconds);
	void UpdateMusic(FLOAT deltaSeconds);
	void UpdateOcclusion(FLOAT deltaSeconds);
	void UpdateLightingOptimisation(FLOAT deltaSeconds);
	void UpdateStruggle(FLOAT deltaSeconds);
	void UpdateOrbisController(FLOAT deltaSeconds);
	void UpdateTouchZoom(FLOAT deltaSeconds);
	void TickPlacementMode();

	void ProcessCompletedRecording(AOLRecordingMarker* recordingMarker);

	void TravelComplete();
}

event OnTravelComplete()
{
}

native function NativePlayerMove(float deltaTime);

native function float GetGamma();
native function SetGamma(float GammaValue);

native function SetVolume(float VolumeLevel);

native function ShowLoadingOverlay();
native function HideLoadingOverlay();

////////////////////////////////////////////////////////////////////////////////////////////
// PlayerController overrides
////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////

simulated event PostBeginPlay()
{
	Super.PostBeginPlay();

	if (TutorialManager != None)
	{
		TutorialManager.Clear();
	}

	// Ensure the profile settings are initially set
	bProfileSettingsUpdated = TRUE;

	UpdateDifficultyBasedValues();
}

event Possess(Pawn inPawn, bool bVehicleTransition)
{
	Super.Possess(inPawn, bVehicleTransition);
	HeroPawn = OLHero(inPawn);	

	bDuck = 0; // for toggle crouch
}

event UnPossess()
{
	HeroPawn = None;
	Super.UnPossess();
}

function Reset()
{
	Super.Reset();
	if ( PlayerCamera != None )
	{
		PlayerCamera.Destroy();
	}
	TutorialManager.Clear();
}

// ---------------------------------------------------------------------------
// Placement mode
// ---------------------------------------------------------------------------

event BeginPlacementMode(Actor Ghost)
{
	EndPlacementMode();

	PlacementGhost            = Ghost;
	PlacementYaw              = Rotation.Yaw;
	bPlacementConfirmPending  = false;
	bPlacementMode            = true;
}

event EndPlacementMode()
{
	if (PlacementGhost != None)
	{
		PlacementGhost.Destroy();
		PlacementGhost = None;
	}
	bPlacementConfirmPending = false;
	bPlacementMode           = false;
}

reliable client function ClientRestart(Pawn NewPawn)
{
	Super.ClientRestart(NewPawn);
}

event OnKismetRemoteEvent(name EventName);
event OnCheckpointReached(name CheckpointName);
event OnCSAActivated(OLCSA CSA);
event OnDoorKismetEvent(string DoorPath, int EventType, bool bByPlayer);
event OnLocalDoorOpen(OLDoor D);
event OnLocalDoorClose(OLDoor D);
event OnPickupKismetEvent(OLPickableObject Pickup) {}

event Destroyed()
{
	if (SoundEnvManager != None)
	{
		SoundEnvManager.Cleanup();
	}
}

function PlayerDied()
{
	NumDeathsSinceLastCheckpoint++;
	InventoryManager.ClearGameplayItems();
	InventoryManager.ClearUnsavedBatteries();
	CurrentObjective = '';
	PendingRecordingMarker = None;
}

native function bool IsPlayerFindableWhileHidden(OLEnemyPawn SearchingEnemy);
native function SetPlayerFoundWhileHidden(OLEnemyPawn SearchingEnemy);

native function ForceActivateTouchEvent(SeqEvent_Touch TouchEvent);
native function ObserverActivateCSA(OLCSA CSA, bool bConsumeActivation);

////////////////////////////////////////////////////////////////////////////////////////////
// Control helpers
////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////

simulated function bool PerformedUseAction()
{
	if (Super.PerformedUseAction())
	{
		return true;		
	}

	Inputs.bPerformedUseAction = true;
	return FALSE;
}

exec function PressedUseButton()
{
	Inputs.bPressedUseButton = true;
	bInvalidateReleasedUse = false;
}

exec function ReleasedUseButton()
{
	if (!bInvalidateReleasedUse)
	{
		Inputs.bReleasedUseButton = true;
	}

	bInvalidateReleasedUse = false;
}

exec function ToggleNightVision()
{
	Inputs.bPressedToggleNightVision = true;
}

exec function ToggleCamcorder()
{
	Inputs.bPressedToggleCamcorder = true;
}

exec function ZoomIn()
{
	Inputs.bPressedZoomIn = true;
}

exec function ZoomOut()
{
	Inputs.bPressedZoomOut = true;
}

simulated exec function StartZoomIn()
{
	ZoomInput = 1;
	Inputs.bStartedActiveZoom = true;
}

simulated exec function StartZoomOut()
{
	ZoomInput = -1;
	Inputs.bStartedActiveZoom = true;
}

simulated exec function StopZoom()
{
	ZoomInput = 0;
	Inputs.bStartedActiveZoom = false;
}

exec function InspectorScrollUp()
{
    InspectorHitIndex++;
    ClientMessage("Inspector idx: " $ InspectorHitIndex $ " / hits: " $ InspectorHits.Length);
}

exec function InspectorScrollDown()
{
    InspectorHitIndex--;
    ClientMessage("Inspector idx: " $ InspectorHitIndex $ " / hits: " $ InspectorHits.Length);
}

exec function FreeCamSpeedUp()
{
    if (!bDebugFreeCam)
        return;
    DebugFreeCamSpeed *= 1.25;
}

exec function FreeCamSpeedDown()
{
    if (!bDebugFreeCam)
        return;
    DebugFreeCamSpeed *= 0.8;
}

exec function FreeCamSpeedReset()
{
    DebugFreeCamSpeed = default.DebugFreeCamSpeed;
}

exec function PressedReloadBatteries()
{
	Inputs.bPressedReloadBatteries = true;
}

exec function PressedJump()
{
	Inputs.bPressedJump = true;
}

exec function GammaCalibration()
{
	HUD.SetGammaCalibrationActive(!HUD.bGammaCalibrationOpen);
	FullScreenOverlayChanged();
}

exec function PrototypeActionA()
{
	Inputs.bPressedPrototypeActionA = true;
}

exec function PrototypeActionB()
{
	Inputs.bPressedPrototypeActionB = true;
}

exec function PrototypeActionC()
{
	Inputs.bPressedPrototypeActionC = true;
}

exec function PrototypeActionD()
{
	Inputs.bPressedPrototypeActionD = true;
}

exec function PrototypeActionE()
{
	Inputs.bPressedPrototypeActionE = true;
}

////////////////////////////////////////////////////////////////////////////////////////////
// Camera
////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////


exec function BehindView()
{
	if ( WorldInfo.NetMode == NM_Standalone )
		SetBehindView(!bBehindView);
}


function SetBehindView(bool bNewBehindView)
{
	bBehindView = bNewBehindView;

	if (LocalPlayer(Player) == None)
	{
		SetBehindView(bNewBehindView);
	}

	// make sure we recalculate camera position for this frame
	LastCameraTimeStamp = WorldInfo.TimeSeconds - 1.0;
}

/**
* return whether viewing in first person mode
*/
function bool UsingFirstPersonCamera()
{
	return !bBehindView;
}

/**
 * Set new camera mode
 *
 * @param	NewCamMode, new camera mode.
 */
function SetCameraMode( name NewCamMode )
{
	// will get set back to true below, if necessary
	bDebugFreeCam = false;
	bDebugFixedCam = false;

	if ( NewCamMode == 'FreeCam' )
	{
		if ( !bBehindView )
		{
			SetBehindView(true);
		}
		bDebugFreeCam = true;
		DebugCamRot = CalcViewRotation;
		DebugCamRot.Roll = 0.0;
		DebugCamPos = CalcViewLocation;
	}
	else if ( NewCamMode == 'FixedCam' )
	{
		if ( !bBehindView )
		{
			SetBehindView(true);
		}
		bDebugFixedCam = true;
		DebugCamRot = CalcViewRotation;
		DebugCamPos = CalcViewLocation;
	}
	else 
	{
		if ( bBehindView )
			SetBehindView(false);
	}
}


function SpawnCamera()
{
	local Actor OldViewTarget;

	// Associate Camera with PlayerController
	PlayerCamera = Spawn(MatineeCameraClass, self);
	if (PlayerCamera != None)
	{
		OldViewTarget = ViewTarget;
		PlayerCamera.InitializeFor(self);
		PlayerCamera.SetViewTarget(OldViewTarget);
	}
	else
	{
		`Log("Couldn't Spawn Camera Actor for Player!!");
	}
}

event float GetFOVAngle()
{
	local CameraActor camActor;

	if (bDebugFreeCam || bDebugFixedCam)
		return DebugFreeCamFOV;

	camActor = CameraActor(ViewTarget);
	if (camActor != None )
	{
		return camActor.FOVAngle;
	}
	return FOVAngle;
}

/* GetPlayerViewPoint: Returns Player's Point of View
	For the AI this means the Pawn's Eyes ViewPoint
	For a Human player, this means the Camera's ViewPoint */
simulated event GetPlayerViewPoint( out vector POVLocation, out Rotator POVRotation )
{
	if (bDebugFreeCam || bDebugFixedCam)
	{
		POVRotation = DebugCamRot;
		POVLocation = DebugCamPos;
	}	
	else if ( CameraActor(ViewTarget) != None )
	{
		if ( PlayerCamera == None )
		{
			super.ResetCameraMode();
			SpawnCamera();
		}
		super.GetPlayerViewPoint( POVLocation, POVRotation );
	}
	else
	{
		if ( PlayerCamera != None )
		{
			PlayerCamera.Destroy();
			PlayerCamera = None;
		}
					
		if (HeroPawn != None)
		{
			HeroPawn.GetCamera(POVLocation, POVRotation, FOVAngle);
		}
		else
		{
			POVRotation = Rotation;
			POVLocation = Location;
		}
	}

	// cache result
	CalcViewActor = ViewTarget;
	CalcViewActorLocation = ViewTarget.Location;
	CalcViewActorRotation = ViewTarget.Rotation;
	CalcViewLocation = POVLocation;
	CalcViewRotation = POVRotation;
}

////////////////////////////////////////////////////////////////////////////////////////////
// Cinematics
////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////

function SetCinematicMode( SeqAct_ToggleCinematicMode Action, bool bInCinematicMode, bool bHidePlayer, bool bAffectsHUD, bool bAffectsMovement, bool bAffectsTurning, bool bAffectsButtons )
{
	if (bExcludeFromKismetPlayer)
		return;

	Super.SetCinematicMode(Action, bInCinematicMode, bHidePlayer, bAffectsHUD, bAffectsMovement, bAffectsTurning, bAffectsButtons);

	if (!bInCinematicMode)
	{
		bIgnoreMoveInput = 0;
		bIgnoreLookInput = 0;
	}

	if (Action != None && HeroPawn != None)
	{
		if (bInCinematicMode)
		{
			HeroPawn.EnterCinematicMode(Action);		
		}
		else
		{
			HeroPawn.ExitCinematicMode(Action);
		}
	}
}

reliable client event ClientSetCameraFade(bool _enableFading, optional color _fadeColor, optional vector2d _fadeAlpha, optional float _fadeTime, optional bool _fadeAudio)
{
	bEnableFading = _enableFading;
	FadeColor = _fadeColor;

	if (_enableFading)
	{
		FadeAlpha = _fadeAlpha;
		FadeTime = _fadeTime;
		FadeTimeRemaining = _fadeTime;
		bFadeAudio = _fadeAudio;
		FadeAmount = _fadeAlpha.X;
	}
	else
	{
		// Make sure FadeAmount finishes at the correct value
		FadeAmount = _fadeAlpha.Y;
	}
}

event OnSetMesh(SeqAct_SetMesh Action)
{
	if (Action.MeshType == MeshType_SkeletalMesh)
	{
		`log("### OnSetMesh: HeroPawn=" $ HeroPawn $ " NewMesh=" $ Action.NewSkeletalMesh $ " CurrentMesh=" $ (HeroPawn != None ? string(HeroPawn.Mesh.SkeletalMesh) : "N/A"));
		if (HeroPawn != None && Action.NewSkeletalMesh != None && Action.NewSkeletalMesh != HeroPawn.Mesh.SkeletalMesh)
		{
			HeroPawn.Mesh.SetSkeletalMesh(Action.NewSkeletalMesh);
		}
	}
}

////////////////////////////////////////////////////////////////////////////////////////////
// HUD
////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////

reliable client function ClientSetHUD(class<HUD> newHUDType)
{
	Super.ClientSetHUD(newHUDType);
	HUD = OLHUD(myHUD);
}

event ForcePause(bool bPause)
{
	SetPause(bPause);

	if (bPause && HUD != None)
	{
		// If the HUD paused because of loss of focus, make sure that it won't unpause automatically when focus gets back on
		HUD.bLostFocusPaused = false;
	}
}

event FullScreenOverlayChanged()
{
	if (HUD != None && (HUD.bGammaCalibrationOpen || HUD.bGameOver || (HUD.bSplashScreenOpen && ActiveSplashScreen.bPauseGame)))
	{
		if (!HUD.bGameOver)
		{
			SetPause(true);
		}
	}
	else
	{
		SetPause(false);
	}
}

function DrawHUD( HUD H )
{
	local OLHUD olHUD;

	Super.DrawHUD(H);

	olHUD = OLHUD(H);
	olHUD.Draw();

	if (bDebugFreeCam && OLCheatManager(CheatManager) != None && OLCheatManager(CheatManager).bFreeCamInspector)
		DrawInspectorTriggerLabels(H);

	if (bDebugFreeCam && OLCheatManager(CheatManager) != None && OLCheatManager(CheatManager).bFreeCamInspector)
		DrawFreeCamInspector(H);
}

// Returns the full path of an object including the top-level package: "Package.Group.Name".
// PathName() alone omits the package when called from script, so we walk Outer manually.
function string GetObjectFullPath(Object Obj)
{
	local string Path;
	local Object O;

	if (Obj == None)
		return "None";

	Path = string(Obj.Name);
	O = Obj.Outer;
	while (O != None)
	{
		Path = string(O.Name) $ "." $ Path;
		O = O.Outer;
	}
	return Path;
}

// Ray vs OBB intersection using the component's LocalToWorld matrix.
// Transforms the ray into local mesh space and does a slab test against
// the static mesh's local-space box extents (unaffected by rotation/scale).
native function bool RayIntersectsOBB(vector RayOrigin, vector RayDir, StaticMeshComponent SMC, out vector HitPos);

// Draw name labels at the screen-projected location of all Triggers and TriggerVolumes.
function DrawInspectorTriggerLabels(HUD H)
{
    local Actor A;
    local vector ScreenPos;
    local Canvas C;

    C = H.Canvas;
    C.Font = H.GetFontSizeIndex(0);

    foreach AllActors(class'Actor', A)
    {
        if (!A.IsA('Trigger') && !A.IsA('TriggerVolume'))
            continue;

        ScreenPos = C.Project(A.Location);
        if (ScreenPos.Z <= 0)
            continue;

        C.SetDrawColor(50, 255, 100, 200);
        C.SetPos(ScreenPos.X, ScreenPos.Y);
        C.DrawText(string(A.Name));
    }
}

// Draw an actor inspector overlay in the top-right corner while FreeCam is active.
// Traces from the camera position along its view direction and reports the hit actor.
// Uses per-component complex collision so meshes without physics collision are hit too.
function DrawFreeCamInspector(HUD H)
{
	local vector TraceEnd, HitLoc, HitNorm;
	local TraceHitInfo HitInfo;
	local Actor A, HitActor, TriggerHit;
	local StaticMeshComponent SMC, HitSMC;
	local SkeletalMeshComponent SKMC, HitSKMC;
	local MaterialInterface Mat;
	local MaterialInstanceConstant MIC;
	local int MatIdx, TexIdx, i, j, SelIdx;
	local Box ActorBox;
	local vector BoxCenter, BoxExtent, TriggerHitLoc, TriggerHitNorm;
	local float X, Y, LineH;
	local Canvas C;
	local string Line;
	local InspectorHit Entry;

	C = H.Canvas;

	// Collect all hits along the ray into InspectorHits array.
	TraceEnd = DebugCamPos + vector(DebugCamRot) * 50000.0;
	InspectorHits.Length = 0;

	foreach AllActors(class'Actor', A)
	{
		foreach A.ComponentList(class'StaticMeshComponent', SMC)
		{
			if (SMC.StaticMesh == None || SMC.HiddenGame)
				continue;
			if (TraceComponent(HitLoc, HitNorm, SMC, TraceEnd, DebugCamPos,, HitInfo, true))
			{
				Entry.HitActor = A;
				Entry.HitSMC   = SMC;
				Entry.HitSKMC  = None;
				Entry.HitLoc   = HitLoc;
				Entry.Dist     = VSize(HitLoc - DebugCamPos);
				InspectorHits.AddItem(Entry);
			}
			// Fallback for components without collision (e.g. StaticMeshCollectionActor):
			// test ray against the component's OBB using LocalToWorld axes.
			else if (!SMC.BlockActors && !SMC.CollideActors)
			{
				if (RayIntersectsOBB(DebugCamPos, vector(DebugCamRot), SMC, HitLoc))
				{
					Entry.HitActor = A;
					Entry.HitSMC   = SMC;
					Entry.HitSKMC  = None;
					Entry.HitLoc   = HitLoc;
					Entry.Dist     = VSize(HitLoc - DebugCamPos);
					InspectorHits.AddItem(Entry);
				}
			}
		}
		foreach A.ComponentList(class'SkeletalMeshComponent', SKMC)
		{
			if (SKMC.SkeletalMesh == None || SKMC.HiddenGame)
				continue;
			if (TraceComponent(HitLoc, HitNorm, SKMC, TraceEnd, DebugCamPos,, HitInfo, true))
			{
				Entry.HitActor = A;
				Entry.HitSMC   = None;
				Entry.HitSKMC  = SKMC;
				Entry.HitLoc   = HitLoc;
				Entry.Dist     = VSize(HitLoc - DebugCamPos);
				InspectorHits.AddItem(Entry);
			}
		}
	}

	// Collision trace for Triggers/TriggerVolumes (invisible, no mesh).
	TriggerHit = Trace(TriggerHitLoc, TriggerHitNorm, TraceEnd, DebugCamPos, true);
	if (TriggerHit != None && (TriggerHit.IsA('Trigger') || TriggerHit.IsA('TriggerVolume')))
	{
		Entry.HitActor = TriggerHit;
		Entry.HitSMC   = None;
		Entry.HitSKMC  = None;
		Entry.HitLoc   = TriggerHitLoc;
		Entry.Dist     = VSize(TriggerHitLoc - DebugCamPos);
		InspectorHits.AddItem(Entry);
	}

	// Sort by distance ascending (bubble sort — list is small).
	for (i = 0; i < InspectorHits.Length - 1; i++)
	{
		for (j = 0; j < InspectorHits.Length - 1 - i; j++)
		{
			if (InspectorHits[j].Dist > InspectorHits[j + 1].Dist)
			{
				Entry                = InspectorHits[j];
				InspectorHits[j]     = InspectorHits[j + 1];
				InspectorHits[j + 1] = Entry;
			}
		}
	}

	// Clamp index and pick selected hit.
	if (InspectorHits.Length == 0)
	{
		// nothing hit
		HitActor = None;
		HitSMC   = None;
		HitSKMC  = None;
		HitLoc   = vect(0,0,0);
	}
	else
	{
		if (InspectorHitIndex < 0) InspectorHitIndex = 0;
		SelIdx   = InspectorHitIndex % InspectorHits.Length;
		HitActor = InspectorHits[SelIdx].HitActor;
		HitSMC   = InspectorHits[SelIdx].HitSMC;
		HitSKMC  = InspectorHits[SelIdx].HitSKMC;
		HitLoc   = InspectorHits[SelIdx].HitLoc;
	}

	// Overlay box: top-right corner, 460px wide
	LineH = 16.0;
	X = C.ClipX - 470.0;
	Y = 10.0;

	C.Font = H.GetFontSizeIndex(0);

	// Helper macro: draw one text line and advance Y
	// (UnrealScript has no macros; we write it inline below)

	if (HitActor == None)
	{
		C.SetDrawColor(200, 200, 200, 200);
		C.SetPos(X, Y);
		C.DrawText("[FreeCam Inspector] No hit");
		return;
	}

	// ---- Header ----
	C.SetDrawColor(255, 220, 80, 230);
	C.SetPos(X, Y);
	C.DrawText("[FreeCam Inspector]  " $ (SelIdx + 1) $ "/" $ InspectorHits.Length $ "  (scroll to cycle)");
	Y += LineH;

	// ---- Class ----
	C.SetDrawColor(180, 255, 180, 220);
	C.SetPos(X, Y);
	C.DrawText("Class:    " $ HitActor.Class);
	Y += LineH;

	// ---- Name / Tag ----
	C.SetDrawColor(220, 220, 220, 220);
	C.SetPos(X, Y);
	C.DrawText("Name:     " $ HitActor.Name);
	Y += LineH;

	// ---- Hit Component ----
	if (HitSMC != None)
	{
		C.SetDrawColor(255, 220, 100, 220);
		C.SetPos(X, Y);
		C.DrawText("Component: " $ HitSMC.Name);
		Y += LineH;
	}
	else if (HitSKMC != None)
	{
		C.SetDrawColor(100, 220, 255, 220);
		C.SetPos(X, Y);
		C.DrawText("Component: " $ HitSKMC.Name);
		Y += LineH;
	}

	C.SetPos(X, Y);
	C.DrawText("Tag:      " $ HitActor.Tag);
	Y += LineH;

	// ---- Location / Distance ----
	C.SetPos(X, Y);
	C.DrawText("Location: " $ int(HitActor.Location.X) $ ", " $ int(HitActor.Location.Y) $ ", " $ int(HitActor.Location.Z));
	Y += LineH;

	C.SetPos(X, Y);
	C.DrawText("Distance: " $ int(VSize(HitLoc - DebugCamPos)));
	Y += LineH;

	// ---- Hit StaticMeshComponent ----
	if (HitSMC != None && HitSMC.StaticMesh != None)
	{
		Y += 4.0;
		C.SetDrawColor(130, 200, 255, 220);
		C.SetPos(X, Y);
		C.DrawText("Mesh: " $ GetObjectFullPath(HitSMC.StaticMesh));
		Y += LineH;

		for (MatIdx = 0; MatIdx < 16; MatIdx++)
		{
			Mat = HitSMC.GetMaterial(MatIdx);
			if (Mat == None)
				continue;

			C.SetDrawColor(200, 200, 200, 200);
			C.SetPos(X + 12, Y);
			C.DrawText("Mat[" $ MatIdx $ "]: " $ GetObjectFullPath(Mat));
			Y += LineH;

			MIC = MaterialInstanceConstant(Mat);
			if (MIC != None)
			{
				for (TexIdx = 0; TexIdx < MIC.TextureParameterValues.Length; TexIdx++)
				{
					if (MIC.TextureParameterValues[TexIdx].ParameterValue != None)
					{
						C.SetDrawColor(160, 160, 200, 200);
						C.SetPos(X + 24, Y);
						Line = "  " $ MIC.TextureParameterValues[TexIdx].ParameterName
							$ " = " $ MIC.TextureParameterValues[TexIdx].ParameterValue.Name;
						C.DrawText(Line);
						Y += LineH;
					}
				}
			}
		}
	}

	// ---- Hit SkeletalMeshComponent ----
	if (HitSKMC != None && HitSKMC.SkeletalMesh != None)
	{
		Y += 4.0;
		C.SetDrawColor(255, 180, 130, 220);
		C.SetPos(X, Y);
		C.DrawText("Mesh: " $ GetObjectFullPath(HitSKMC.SkeletalMesh));
		Y += LineH;

		for (MatIdx = 0; MatIdx < 16; MatIdx++)
		{
			Mat = HitSKMC.GetMaterial(MatIdx);
			if (Mat == None)
				continue;

			C.SetDrawColor(200, 200, 200, 200);
			C.SetPos(X + 12, Y);
			C.DrawText("Mat[" $ MatIdx $ "]: " $ GetObjectFullPath(Mat));
			Y += LineH;

			MIC = MaterialInstanceConstant(Mat);
			if (MIC != None)
			{
				for (TexIdx = 0; TexIdx < MIC.TextureParameterValues.Length; TexIdx++)
				{
					if (MIC.TextureParameterValues[TexIdx].ParameterValue != None)
					{
						C.SetDrawColor(160, 160, 200, 200);
						C.SetPos(X + 24, Y);
						Line = "  " $ MIC.TextureParameterValues[TexIdx].ParameterName
							$ " = " $ MIC.TextureParameterValues[TexIdx].ParameterValue.Name;
						C.DrawText(Line);
						Y += LineH;
					}
				}
			}
		}
	}

	// Draw the hit component's bounding box in the world.
	if (HitSMC != None)
	{
		BoxCenter = HitSMC.Bounds.Origin;
		BoxExtent = HitSMC.Bounds.BoxExtent;
		DrawDebugBox(BoxCenter, BoxExtent, 255, 200, 50, false);
	}
	else if (HitSKMC != None)
	{
		BoxCenter = HitSKMC.Bounds.Origin;
		BoxExtent = HitSKMC.Bounds.BoxExtent;
		DrawDebugBox(BoxCenter, BoxExtent, 100, 200, 255, false);
	}
	else if (HitActor != None && (HitActor.IsA('Trigger') || HitActor.IsA('TriggerVolume')))
	{
		HitActor.GetComponentsBoundingBox(ActorBox);
		BoxCenter = (ActorBox.Min + ActorBox.Max) * 0.5;
		BoxExtent = (ActorBox.Max - ActorBox.Min) * 0.5;
		DrawDebugBox(BoxCenter, BoxExtent, 50, 255, 100, false);
	}
}

simulated function DisplayDebug(HUD H, out float out_YL, out float out_YPos)
{
	local OLGame CurrentGame;

	H.Canvas.SetDrawColor(255,255,200);
	H.Canvas.SetPos(4,out_YPos);
		
	CurrentGame = OLGame(WorldInfo.Game);
	if (CurrentGame.CurrentCheckpointName != '')
	{
		if (CurrentGame.bIsPlayingDLC)
		{
			H.Canvas.DrawText("Checkpoint: " $ CurrentGame.CurrentCheckpointName $ " [dlc]");
		}
		else
		{
			H.Canvas.DrawText("Checkpoint: " $ CurrentGame.CurrentCheckpointName);
		}
	}
	else
	{
		H.Canvas.DrawText("Checkpoint: [none]");
	}
	out_YPos += out_YL;	

	if (bDebugFreeCam || bDebugFixedCam)
	{
		H.Canvas.SetDrawColor(255,255,200);
		H.Canvas.SetPos(4,out_YPos);
		H.Canvas.DrawText("Camera pos: " $ DebugCamPos $ ", rot: " $ DebugCamRot);
		out_YPos += out_YL;
	}

	H.Canvas.SetPos(4,out_YPos);
	H.Canvas.DrawText("Has Reloaded Batteries: " $ bReloadedBatteries $ ", Used Hiding Spot: " $ bUsedHidingSpot);
	out_YPos += out_YL;

	if (IsPaused())
	{
		H.Canvas.SetDrawColor(240,20,20);
		H.Canvas.SetPos(0.8*HUD.Canvas.ClipX,20);
		if (bDebugFreeCam)
		{
			H.Canvas.DrawText("PAUSED - FREE CAMERA");
		}
		else
		{
			H.Canvas.DrawText("PAUSED");
		}
	}
}

////////////////////////////////////////////////////////////////////////////////////////////
// Sound
////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////

native function StopAllSounds();

function OnOverrideAIMusic(OLSeqAct_OverrideAIMusic Action)
{
	if (Action.InputLinks[0].bHasImpulse)
	{
		bOverriddenMusic = TRUE;
		OverriddenMusicState = Action.MusicState;
		OverriddenMusicDistance = Action.MusicDistance;
	}
	else if (Action.InputLinks[1].bHasImpulse)
	{
		bOverriddenMusic = FALSE;
	}
}

////////////////////////////////////////////////////////////////////////////////////////////
// Checkpoint
////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////

native function SavePersistentState();

native function NativeCreateCheckpointRecord(out CheckpointRecord Record);
native function NativeApplyCheckpointRecord(const out CheckpointRecord Record);

function CreateCheckpointRecord( out CheckpointRecord Record )
{
	NativeCreateCheckpointRecord(Record);
}

function ApplyCheckpointRecord(const out CheckpointRecord Record)
{
	NativeApplyCheckpointRecord(Record);
	
	if (HeroPawn != None)
	{
		HeroPawn.LifeSpan = 0.1;
	}
	UnPossess();
	
	// restart this player to create the pawn
	WorldInfo.Game.RestartPlayer(self);
}

function ApplyDeprecatedCheckpointRecord(const out DeprecatedCheckpointRecord OldRecord)
{
	// Convert initial shipped version record to a new one.
	local OLGame CurrentGame;
	local CheckpointRecord NewRecord;

	NewRecord.NumBatteries = OldRecord.NumBatteries;

	NewRecord.CurrentObjective = OldRecord.CurrentObjective; 
	NewRecord.CompletedObjectives = OldRecord.CompletedObjectives; 

	NewRecord.CompletedRecordingMoments = OldRecord.CompletedRecordingMoments;
	NewRecord.UnreadRecordingMoments = OldRecord.UnreadRecordingMoments;

	NewRecord.ActivatedGameState = OldRecord.ActivatedGameState;

	NewRecord.CollectedDocuments = OldRecord.CollectedDocuments;
	NewRecord.UnreadDocuments = OldRecord.UnreadDocuments;
	NewRecord.CollectedBatteries = OldRecord.CollectedBatteries; 

	NewRecord.bBatteryTutorialComplete = OldRecord.bBatteryTutorialComplete;
	NewRecord.bClimbUpTutorialComplete = OldRecord.bClimbUpTutorialComplete;
	NewRecord.bDocumentTutorialComplete = OldRecord.bDocumentTutorialComplete;

	NewRecord.bFoundBySoldierWhileHidden = OldRecord.bFoundBySoldierWhileHidden;
	NewRecord.bFoundBySurgeonWhileHidden = OldRecord.bFoundBySurgeonWhileHidden;

	NewRecord.RecordingTimeSeconds = OldRecord.RecordingTimeSeconds;

	// new fields
	NewRecord.CheckpointRecordVersion = 2;
	NewRecord.BatteryEnergy = 1.0;

	CurrentGame = OLGame(WorldInfo.Game);

	if (CurrentGame != None && CurrentGame.DifficultyMode != EDM_Insane)
	{	
		NewRecord.DifficultyMode = CurrentGame.DifficultyMode;
	}
	else
	{
		NewRecord.DifficultyMode = EDM_Normal;
	}

	ApplyCheckpointRecord(NewRecord);	
}

native event StartNewGameAtCheckpoint(string CheckpointStr, bool bSaveToDisk);

reliable client event ClientCommitMapChange()
{
	if (SoundEnvManager != None)
	{
		SoundEnvManager.ExitAllTouchingVolumes();
	}	
	ShowLoadingOverlay();
	Super.ClientCommitMapChange();
	HideLoadingOverlay();	
}

native function SaveBeforeQuitting();
native function ClearAllProgress();

native function bool ShippingCheat_GiveAllCheckpoints();
native function CheatGiveAllCollectibles();
native function GiveCollectibleToPlayer(name CollectibleName);
native function GiveGameplayItemToPlayer(name ItemName);
native function float GetHighResTimeSeconds();
native function int GetHighResTimeMs();

native function SetAstoundSoundEnabled(bool bEnabled);

////////////////////////////////////////////////////////////////////////////////////////////
// Profile
////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////

/**
* Creates and initializes the "PlayerOwner" and "PlayerSettings" data stores.  This function assumes that the PlayerReplicationInfo
* for this player has not yet been created, and that the PlayerOwner data store's PlayerDataProvider will be set when the PRI is registered.
*
* Overloaded so we can initialize game specific player data stores.
*/
simulated protected function RegisterCustomPlayerDataStores()
{
	local LocalPlayer LP;

	// only create player data store for local players
	LP = LocalPlayer(Player);

	Super.RegisterCustomPlayerDataStores();

	if (LP != None)
	{
		ProfileSettings = OLProfileSettings(OnlinePlayerData.ProfileProvider.Profile);
	}
}

/**
* Unregisters the "PlayerOwner" data store for this player.  Called when this PlayerController is destroyed.
*
* Overloaded so we can unregister game specific player data stores.
*/
simulated function UnregisterPlayerDataStores()
{
	local LocalPlayer LP;

	// only execute for local players
	LP = LocalPlayer(Player);
	if ( LP != None )
	{
		ProfileSettings = None;
	}

	Super.UnregisterPlayerDataStores();
}

native function NativeOnControllerChanged(int ControllerId,bool bIsConnected);
function OnControllerChanged(int ControllerId,bool bIsConnected)
{
	`log("### OLPC OnControllerChanged");
	NativeOnControllerChanged(ControllerId, bIsConnected);
}

/**
 * Save the profile
 */
private event SaveProfile()
{
	local int ControllerId;

	ControllerId = LocalPlayer(Player).ControllerId;
	
	if (OnlineSub != None &&
		OnlineSub.PlayerInterface != None)
	{
		if ( ControllerId != INDEX_NONE )
		{
			OnlineSub.PlayerInterface.WriteProfileSettings(ControllerId, ProfileSettings);
		}
	}
}

/**
 * Clear a SaveProfile delegate from the online player interface
 *
 * @param WriteProfileSettingsCompleteDelegate delegate function to clear
 */
function ClearSaveProfileDelegate(int ControllerId,delegate<OnlinePlayerInterface.OnWriteProfileSettingsComplete> WriteProfileSettingsCompleteDelegate )
{
	local OnlinePlayerInterface PlayerInt;

	if ( WriteProfileSettingsCompleteDelegate != None )
	{
		if (OnlineSub != None)
		{
			PlayerInt = OnlineSub.PlayerInterface;
			if (PlayerInt != None)
			{
				if ( ControllerId != INDEX_NONE )
				{
					PlayerInt.ClearWriteProfileSettingsCompleteDelegate(ControllerId,WriteProfileSettingsCompleteDelegate);
				}
			}
		}
	}
}

/**
 * Called when the profile is done writing
 *
 * @param LocalUserNum User whose data was written.
 * @param bWasSuccessful Whether or not the write was successful
 */
function OnProfileWriteComplete(byte LocalUserNum,bool bWasSuccessful)
{
	//`LogOnline(`showvar(LocalUserNum) @ `showvar(bWasSuccessful));
	ClearSaveProfileDelegate(LocalUserNum,OnProfileWriteComplete);
}

/**
 * Client function so that the server can tell this player to save it's profile
 *
 * @param bShouldForce whether to force or not (almost never use this TCR violation)
 * @param bSkipCacheUpdate whether to skip the call to UpdateLocalCacheOfProfileSettings() (useful when we know we're not in the menus and know nothing needs update, e.g. checkpoints)
 */
reliable client function ClientSaveAllPlayerData(optional bool bShouldForce, optional bool bSkipCacheUpdate)
{
	local LocalPlayer LP;

	LP = LocalPlayer(Player);
	if (LP != None)
	{
		SaveProfile();
	}

	if (!bSkipCacheUpdate)
	{
		// so this saveprofile gets called when things have been updated.  When that occurs we need to call this function
		// which will set all of the correct runtime settings.
		UpdateLocalCacheOfProfileSettings(ProfileSettings);
		bProfileSettingsUpdated = FALSE;
	}
}

/**
 * Copies profile settings from the datastore and stores them in local variables.
 */
event UpdateLocalCacheOfProfileSettings(OLProfileSettings EffectiveProfileSettings)
{
	local float FloatValue;
	local int IntValue;
	local OLEngine TheEngine;

	if (EffectiveProfileSettings != None)
	{
		if (PlayerInput != None)
		{
			EffectiveProfileSettings.GetProfileSettingValueId(PSI_YInversion, IntValue);
			PlayerInput.bInvertMouse = (IntValue == PSB_On);

			EffectiveProfileSettings.GetProfileSettingValueFloat(PSI_ControllerSensitivity, FloatValue);
			PlayerInput.MouseSensitivity = FloatValue;
		}

		EffectiveProfileSettings.GetProfileSettingValueFloat(PSI_GammaSetting, FloatValue);
		SetGamma(FloatValue);

		EffectiveProfileSettings.GetProfileSettingValueFloat(PSI_Volume, FloatValue);
		SetVolume(FloatValue);

		if (HUD != None)
		{
			EffectiveProfileSettings.GetProfileSettingValueId(PSI_Subtitles, IntValue);
			HUD.bShowSubtitles = (IntValue == PSB_On);

			EffectiveProfileSettings.GetProfileSettingValueId(PSI_ShowCrosshair, IntValue);
			HUD.bShowCrosshair = (IntValue == PSB_On);

			EffectiveProfileSettings.GetProfileSettingValueId(PSI_ShowPrompts, IntValue);
			HUD.bAlwaysShowPrompts = (IntValue == PSB_On);
		}

		EffectiveProfileSettings.GetProfileSettingValueId(PSI_Tutorials, IntValue);
		TutorialManager.bTutorialsEnabled = (IntValue == PSB_On);
			
		if (class'OLUtils'.static.IsConsole())
		{
			bToggleCrouch = true; // no choice on console
		}
		else
		{
			EffectiveProfileSettings.GetProfileSettingValueId(PSI_ToggleCrouch, IntValue);
			bToggleCrouch = (IntValue == PSB_On);		
		}

		if (ForceFeedbackManager != None)
		{
			if (class'OLUtils'.static.IsConsole())
			{
				ForceFeedbackManager.bAllowsForceFeedback = true; // let the system disable it if the user wants
			}
			else
			{
				EffectiveProfileSettings.GetProfileSettingValueId(PSI_ControllerVibration, IntValue);
				ForceFeedbackManager.bAllowsForceFeedback = (IntValue == PSB_On);
			}
		}

		TheEngine = OLEngine(class'Engine'.static.GetEngine());

		if (TheEngine != None)
		{
			TheEngine.ApplySystemSettings(EffectiveProfileSettings);
			TheEngine.ApplyKeyBindings(EffectiveProfileSettings);
		}
	}
}

event CheckForProfileUpdate()
{
	if ( bProfileSettingsUpdated && ProfileSettings != None && (ProfileSettings.ProfileSettings.length) > 0 )
	{
		UpdateLocalCacheOfProfileSettings(ProfileSettings);
		bProfileSettingsUpdated = FALSE;
	}
}

function UpdateDifficultyBasedValues()
{
	local OLGame TheGame;

	TheGame = OLGame(WorldInfo.Game);
	if (TheGame == None || TheGame.DifficultyMode == EDM_Normal)
	{
		MaxNumBatteries = NrmMaxNumBatteries;
	}
	else if (TheGame.DifficultyMode == EDM_Hard)
	{
		MaxNumBatteries = HardMaxNumBatteries;
	}
	else
	{
		MaxNumBatteries = NightmareMaxNumBatteries;
	}

	if (NumBatteries > MaxNumBatteries)
	{
		NumBatteries = MaxNumBatteries;
	}
}

function NotifyDifficultyChanged()
{
	local OLBot Bot;

	// Update Self
	UpdateDifficultyBasedValues();

	// Update Hero
	if (HeroPawn != None)
	{
		HeroPawn.UpdateDifficultyBasedValues();
	}
	
	// Update All Active Bots
	foreach WorldInfo.AllControllers(class'OLBot', Bot)
	{
		if (Bot.EnemyPawn != None)
		{
			Bot.EnemyPawn.UpdateDifficultyBasedValues();
		}
	}
}

event UnlockAchievement(EOutlastAchievement achievement)
{
	local int ControllerId;

	ControllerId = LocalPlayer(Player).ControllerId;

	if (OnlineSub != None && OnlineSub.PlayerInterface != None && ControllerId != INDEX_NONE)
	{
		if (OnlineSub.PlayerInterface.UnlockAchievement(ControllerId, int(achievement)))
		{
			`log("-- Unlocked achievement #" $ achievement);
		}
		else
		{
			`log("-- FAILED to unlock achievement #" $ achievement);
		}
	}
}


////////////////////////////////////////////////////////////////////////////////////////////
// States
////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////

state PlayerWalking
{
ignores SeePlayer, HearNoise, Bump;

	function PlayerMove( float DeltaTime )
	{
		local rotator DeltaRot, DummyViewRotation;
		
		if( Pawn == None )
		{
			return;
		}

		if (!HUD.ShowingFullScreenOverlay())
		{
			NativePlayerMove(DeltaTime);

			if (PlayerCamera != None)
			{
				DummyViewRotation = Rotation;
				DeltaRot.Yaw = PlayerInput.aTurn;
				DeltaRot.Pitch	= PlayerInput.aLookUp;

				PlayerCamera.ProcessViewRotation( DeltaTime, DummyViewRotation, DeltaRot );
			}
		}
	}
}

// Called by OLBot.DamageTarget when the target is a dummy player pawn.
// Overridden in MultiplayerController to route the hit to the dummy's owner via PLAYER_HIT.
function NotifyDummyPlayerHit(OLHero DummyTarget, float Damage, float KnockbackPower, vector HitDir) {}

// Called by OLBot.NotifyDummyGrab when the enemy grabs a dummy player pawn.
// Overridden in MultiplayerController to route the grab to the dummy's owner via PLAYER_GRAB.
function NotifyDummyPlayerGrab(int TargetPlayerID, vector GrabTargetLoc, vector CharDir, bool bCrouched, int EnemyTypeInt, float BlendAlpha, bool bLeftAnim, int GrabType) {}

// Called by OLBot.ThrowPlayer when the target is a dummy player pawn.
// Overridden in MultiplayerController to route the throw to the dummy's owner via PLAYER_THROW.
function NotifyDummyPlayerThrow(int TargetPlayerID, float ThrowRotation) {}

// Overridden in MultiplayerController to route kill animation to the dummy's owner via PLAYER_KILL.
// KillType: 0=SMT_HeroKilled, 1=SMT_HeroDecapitate
function NotifyDummyPlayerKill(int TargetPlayerID, int EnemyTypeInt, int WeaponType, bool bBackAnim, bool bLeftAnim, float BlendAlpha, vector AnimStart, vector CharDir, int KillType, int VictimYaw) {}

function NotifyDummyEnemySMT(OLEnemyPawn EnemyPawn, int SMTType, int Param1, int Param2) {}

event NotifyEnemyDoorOpen(OLEnemyPawn EnemyPawn, OLDoor D, float Speed, float Angle) {}
event NotifyEnemyDoorDone(OLEnemyPawn EnemyPawn, OLDoor D, float CloseSpeed) {}
event NotifyEnemyDoorBash(OLEnemyPawn EnemyPawn, OLDoor D, bool bReversed) {}
event NotifyEnemyDoorBreak(OLEnemyPawn EnemyPawn, OLDoor D, bool bReversed) {}

// Called by OLPawn when the local player physically touches an actor with Kismet touch events.
// Overridden in MultiplayerController to send TRIGGER_ACT to remote.
function NotifyPawnTouchedTrigger(Actor TriggerActor) {}

// Called when local player completes a recording marker.
// Overridden in MultiplayerController to sync bRecorded state to all peers.
function NotifyRecordingCompleted(OLRecordingMarker Marker) {}

// Called on remote peer: marks marker recorded and shows HUD note + sound.
native function NativeApplyRemoteRecording(OLRecordingMarker Marker);

function SaveNetworkSettings(string NewIP, string NewPort, string NewUsername, bool bSyncInteractable, bool bSyncEnemies, bool bSyncMatinees, bool bSyncPickups, bool bSpeedrunMode, string NewRoomCode, string NewPassword)
{
    class'OLNetworkConfig'.static.Save(NewIP, NewPort, NewUsername,
        bSyncInteractable, bSyncEnemies, bSyncMatinees, bSyncPickups, bSpeedrunMode,
        NewRoomCode, NewPassword);
}

function OnInventoryItemConsumed(name ItemName) {}

function string GetNetIP()               { return Len(class'OLNetworkConfig'.default.IP)       > 0 ? class'OLNetworkConfig'.default.IP       : "127.0.0.1"; }
function string GetNetPort()             { return Len(class'OLNetworkConfig'.default.UdpPort)   > 0 ? class'OLNetworkConfig'.default.UdpPort   : "7777"; }
function string GetNetUsername()         { return Len(class'OLNetworkConfig'.default.Username)  > 0 ? class'OLNetworkConfig'.default.Username  : "Player"; }
function string GetNetRoomCode()         { return class'OLNetworkConfig'.default.RoomCode; }
function string GetNetPassword()         { return class'OLNetworkConfig'.default.Password; }
function string GetNetHostSteamID()      { return class'OLNetworkConfig'.default.HostSteamID; }
function bool   GetNetSyncInteractable() { return class'OLNetworkConfig'.default.SyncInteractable; }
function bool   GetNetSyncEnemies()      { return class'OLNetworkConfig'.default.SyncEnemies; }
function bool   GetNetSyncMatinees()     { return class'OLNetworkConfig'.default.SyncMatinees; }
function bool   GetNetSyncPickups()      { return class'OLNetworkConfig'.default.SyncPickups; }
function bool   GetNetSpeedrunMode()     { return class'OLNetworkConfig'.default.SpeedrunMode; }

// Builds openol://ip:port?room=CODE[&password=SECRET]
native function string NativeBuildInviteLink(string IP, string Port, string Room, string Pass);
// Parses openol://ip:port?room=CODE[&password=SECRET]; returns false if not a valid link
native function bool NativeParseInviteLink(string Link, out string OutIP, out string OutPort, out string OutRoom, out string OutPass);
// Connect to a host via Steam P2P tunnel (FP2PBridge client mode).
native function NativeConnectP2P(string HostSteamID, int RelayPort, string RoomCode, string Password);
// Returns the local user's 64-bit SteamID as a string, or "" if not available.
native function string NativeGetMySteamID();
// Opens the Steam overlay to the friends list (so the user can invite via Steam UI).
native function NativeOpenSteamFriendsOverlay();
// Set Rich Presence so friends see "Join Game" button pointing to our relay room.
native function NativeSetRelayRichPresence(string RoomCode);
// Clear Rich Presence when relay stops.
native function NativeClearRelayRichPresence();
// Start the embedded relay server on the given port.
native function NativeStartRelay(int Port);
// Stop the embedded relay server.
native function NativeStopRelay();
// Returns true if the embedded relay is currently running.
native function bool NativeIsRelayRunning();
// Connect to a local relay server on 127.0.0.1 (used when hosting).
native function NativeConnectLocal(int Port);

event OnLevelBecameVisible(string PackageName) {}

////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////


defaultproperties
{
	CheatClass=class'OLCheatManager'
	CameraClass=None
	Begin Object Name=CollisionCylinder
		CollisionRadius=22.0
		CollisionHeight=22.0
	End Object
	CollisionComponent=CollisionCylinder
	CylinderComponent=CollisionCylinder
	Components.Add(CollisionCylinder)

	FOVAngle=80.000
	NetPriority=3
	bIsPlayer=true
	bCanDoSpecial=true
	Physics=PHYS_None
	InputClass=class'OLGame.OLPlayerInput'

	bHasCamcorder=true

	MaxResponseTime=0.125
	ClientCap=0
	LastSpeedHackLog=-100.0
	SavedMoveClass=class'SavedMove'

	DesiredFOV=80.0
	DefaultFOV=80.0
	LODDistanceFactor=1.0

	bCinemaDisableInputMove=false
	bCinemaDisableInputLook=false

	bIsUsingStreamingVolumes=true
	
	DebugFreeCamSpeed=0.004
	DebugFreeCamFOV=90.0

	MatineeCameraClass=class'OLGame.OLCamera'

	MusicAIStateGroup=AI_State
	MusicAIStateNone=Z_None
	MusicAIStatePatrol=A_Patrol
	MusicAIStateInvestigate=B_Investigate
	MusicAIStateChase=C_Chase
	AIDistanceRTPC=Soldier_distance

	LoadingStateGroup=Loading_State
	LoadingStateOn=LOADING_MODE
	LoadingStateOff=GAMEPLAY_MODE

	AstoundSoundGroup=ST_Astound
	AstoundSoundOn=Astound_ON
	AstoundSoundOff=Astound_Off

	SndResetMixStates=AkEvent'General_Sound.MIX_STATES_RESET'

	LightBarColor=(R=0.41,G=0.63,B=0.26)

}
