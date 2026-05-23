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
var rotator	DebugCamRot;
var vector	DebugCamPos;
var float	DebugFreeCamSpeed;
var float	DebugFreeCamFOV;
var bool		bSlowDownFPS;
var float	SlowDownFactor;


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

	void ProcessCompletedRecording(AOLRecordingMarker* recordingMarker);

	void TravelComplete();		
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

reliable client function ClientRestart(Pawn NewPawn)
{
	Super.ClientRestart(NewPawn);
}

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

event StartNewGameAtCheckpoint(string CheckpointStr, bool bSaveToDisk)
{
	local OLCheckpoint CheckCP;
	local OLCheckpoint StartCP;
	local OLHero Hero;
	local OLGame CurrentGame;
	local OLEngine Engine;

	// just to check that it exists
	foreach AllActors(class'OLCheckpoint', CheckCP)
	{
		if (Caps(CheckCP.CheckpointName) == Caps(CheckpointStr))
		{
			StartCP = CheckCP;
			break;
		}
	}

	if (StartCP != None) 
	{
		if (HUD.IsMainMenuOpen())
		{
			StopAllSounds();
		}

		HUD.HideMenu();

		Hero = HeroPawn;
		UnPossess();

		if (Hero != None)
		{
			Hero.Destroy();
		}

		ClearAllProgress();

		Engine = OLEngine(class'Engine'.static.GetEngine());
		CurrentGame = OLGame(WorldInfo.Game);

		if (CurrentGame != None)
		{	
			if (Engine != None && !class'OLUtils'.static.IsConsole()) // already saved on consoles
			{		
				Engine.SaveCheckpoint(StartCP.CheckpointName, bSaveToDisk);
			}

			CurrentGame.CurrentCheckpointName = StartCP.CheckpointName;
			CurrentGame.RestartPlayer(self);
		}
	}
}

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
