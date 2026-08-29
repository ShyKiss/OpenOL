class OLCheatManager extends GameCheatManager within OLPlayerController
	config(Game)
	native;

var int GameSpeedLevel;
var config bool bCheatsEnabled;
var config bool bUnlimitedBatteries;
var bool bDebugGameplay;
var bool bDebugSoundEnvironment;
var bool bDebugGameState;
var bool bDebugCamera;
var bool bDebugTrajectory;
var bool bDrawDebugAI; // similar to the AI bAIDrawDebug, but non-persistent
var bool bEnableAstoundSound;
var config bool bDebugWaitPoints;
var config bool bMuted;
var config bool bDebugAIPositions;
var config bool bSuppressAllMessages;

var bool bPausedForFreeCam;
var bool bFreeCamInspector;  // Draw actor inspector overlay while in FreeCam
var string DebugSoundEnvFilter;

var float NextSpikeTime;
var float AutoSpikeDelay;

enum EDebugTrajectoryType
{
	DTT_Walking,
	DTT_Falling,
	DTT_AdjustPosition,
	DTT_ProceduralAnim,
	DTT_SpecialMove,
	DTT_Camera,
	DTT_Other,
};

struct native DebugTrajectoryPoint
{
	var vector Position;
	var float Speed;
	var EDebugTrajectoryType PointType;
	var float TimeStamp;
};

var array<DebugTrajectoryPoint> DebugTrajectoryPoints;

native function AddDebugTrajectoryPoint(DebugTrajectoryPoint point);

native static function OLCheatManager GetCheatManager();

exec function ToggleCheats()
{
	bCheatsEnabled = !bCheatsEnabled;

	if (bCheatsEnabled)
	{
		ClientMessage("Cheats are ENABLED");
	}
	else
	{
		ClientMessage("Cheats are DISABLED");
	}
}

exec function KillAll(class<actor> aClass)
{
	if (!bCheatsEnabled)
	{
		return;
	}

	super.KillAll(aClass);
}

exec function Reload()
{
	if (!bCheatsEnabled)
		return;
	if (HeroPawn != None)
		HeroPawn.RespawnHero();
}

exec native function DeleteAllSaves();
exec native function SaveAllCheckpoints();

exec native function DebugGameplay();
exec native function DebugSoundEnvironment(optional string filter);

exec function DebugCamera()
{
	if (!bCheatsEnabled)
	{
		return;
	}

	bDebugCamera = !bDebugCamera;
}

exec function DebugTrajectory()
{
	if (!bCheatsEnabled)
	{
		return;
	}

	bDebugTrajectory = !bDebugTrajectory;

	DebugTrajectoryPoints.Length = 0;
}

exec function DebugWaitPoints()
{
	if (!bCheatsEnabled)
	{
		return;
	}

	bDebugWaitPoints = !bDebugWaitPoints;
}

exec function ShowStatLevels()
{
	if (!bCheatsEnabled)
	{
		return;
	}

	ConsoleCommand("stat levels");
}

exec function ShowPaths()
{
	if (!bCheatsEnabled)
	{
		return;
	}

	ConsoleCommand("show paths");
}

exec function ShowHeroDebug()
{
	if (!bCheatsEnabled)
	{
		return;
	}

	ConsoleCommand("showdebug");
}

exec function DebugAI()
{
	if (!bCheatsEnabled)
	{
		return;
	}

	if (bDrawDebugAI)
	{
		bDrawDebugAI = false;
	}
	else
	{
		bDrawDebugAI = true;
	}

	ConsoleCommand("showdebug OLAI");
}

exec function DebugAIPositions()
{
	if (!bCheatsEnabled)
	{
		return;
	}

	bDebugAIPositions = !bDebugAIPositions;
}

exec function BadFPS(optional float slowdown)
{
	if (!bCheatsEnabled)
	{
		return;
	}

	if (bSlowDownFPS && (slowdown == 0.0 || slowdown == SlowDownFactor))
	{
		bSlowDownFPS = false;
		ClientMessage("Bad FPS OFF");
	}
	else
	{
		bSlowDownFPS = true;
		
		if (slowdown > 0.0)
		{
			SlowDownFactor = slowdown;
		}
		else
		{
			SlowDownFactor = 1.0;
		}

		ClientMessage("Bad FPS ON (" $ slowdown $ ")");
	}
}

exec function Spike(optional float spike)
{
	if (!bCheatsEnabled)
	{
		return;
	}

	NextSpikeTime = WorldInfo.TimeSeconds + 1.0;
	
	if (spike > 0)
	{
		SlowDownFactor = spike;
	}
	else
	{
		SlowDownFactor = 10.0;
	}

	AutoSpikeDelay = -1.0;
}

exec function AutoSpike(optional float spike, optional float delay)
{
	if (!bCheatsEnabled)
	{
		return;
	}

	if (AutoSpikeDelay > 0.0)
	{
		ClientMessage("Auto Spike OFF");
		AutoSpikeDelay = -1.0;
		NextSpikeTime = -1.0;

		return;
	}

	NextSpikeTime = WorldInfo.TimeSeconds + 1.0;

	if (spike > 0)
	{
		SlowDownFactor = spike;
	}
	else
	{
		SlowDownFactor = 10.0;
	}

	if (delay > 0)
	{
		AutoSpikeDelay = delay;
	}
	else
	{
		AutoSpikeDelay = 2.0;
	}

	ClientMessage("Auto Spike ON (spike " $ SlowDownFactor $ " every " $ AutoSpikeDelay $ "s)");
}

exec function ToggleFreeCam()
{
	if (!bCheatsEnabled)
	{
		return;
	}

	if (bDebugGhost)
	{
		bDebugGhost = false;
		GhostPawn(false);
		if (HeroPawn != None)
			HeroPawn.ResetAfterTeleport();
	}

	if (bDebugFreeCam)
	{
		SetCameraMode('FirstPerson');
		SetPause(false);
		ClientMessage("Default camera");
		bPausedForFreeCam = false;
	}
	else
	{
		SetCameraMode('FreeCam');
		ClientMessage("Free Cam");

		if (WorldInfo.Pauser == None)
		{
			SetPause(true);
			bPausedForFreeCam = true;
		}
	}
}

exec function ToggleFreeCamNoPause()
{
	if (!bCheatsEnabled)
	{
		return;
	}

	if (bDebugGhost)
	{
		bDebugGhost = false;
		GhostPawn(false);
		if (HeroPawn != None)
			HeroPawn.ResetAfterTeleport();
	}
	SetPause(false);
	bPausedForFreeCam = false;

	if (bDebugFreeCam)
	{
		SetCameraMode('FirstPerson');
		ClientMessage("Default camera");
	}
	else
	{
		SetCameraMode('FreeCam');
		ClientMessage("Free Cam (unpaused)");
	}
}

exec function ToggleFixedCam()
{
	if (!bCheatsEnabled)
	{
		return;
	}

	if (bDebugGhost)
	{
		bDebugGhost = false;
		GhostPawn(false);
	}

	if (bPausedForFreeCam)
	{
		SetPause(false);
		bPausedForFreeCam = false;
	}

	if (bDebugFixedCam)
	{
		SetCameraMode('FirstPerson');
		ClientMessage("Default camera");
	}
	else
	{
		SetCameraMode('FixedCam');		
		ClientMessage("Fixed Cam");
	}
}

exec function TeleportToFreeCam()
{
	if (!bCheatsEnabled)
	{
		return;
	}

	if (bDebugGhost)
	{
		bDebugGhost = false;
		GhostPawn(false);
	}

	if (bDebugFreeCam || bDebugFixedCam)
	{
		if (HeroPawn != None)
		{
			HeroPawn.SetLocation(DebugCamPos);
		}

		SetPause(false);
		bPausedForFreeCam = false;
		SetLocation(DebugCamPos);
		SetCameraMode('FirstPerson');
		HeroPawn.SetRotation(DebugCamRot);
		HeroPawn.EyeRotation = DebugCamRot;

		HeroPawn.ResetAfterTeleport();

		ClientMessage("Teleported");
	}
}

exec native function SingleFrame();

function UpdateGameSpeed()
{
	if (!bCheatsEnabled)
	{
		return;
	}

	if (GameSpeedLevel == -3)			{	WorldInfo.Game.SetGameSpeed(0.1); }
	else if (GameSpeedLevel == -2)	{	WorldInfo.Game.SetGameSpeed(0.25); }
	else if (GameSpeedLevel == -1)	{	WorldInfo.Game.SetGameSpeed(0.5); }
	else if (GameSpeedLevel == 0)		{	WorldInfo.Game.SetGameSpeed(1.0); }
	else if (GameSpeedLevel == 1)		{	WorldInfo.Game.SetGameSpeed(2.0); }
	else if (GameSpeedLevel == 2)		{	WorldInfo.Game.SetGameSpeed(4.0); }
	else if (GameSpeedLevel == 3)		{	WorldInfo.Game.SetGameSpeed(10.0); }

	ClientMessage("Game speed: " $ WorldInfo.TimeDilation);
}

exec function NormalGameSpeed()
{
	if (!bCheatsEnabled)
	{
		return;

	}
	GameSpeedLevel = 0;
	UpdateGameSpeed();
}

exec function SlowerGameSpeed()
{
	if (!bCheatsEnabled)
	{
		return;

	}
	GameSpeedLevel--;
	if (GameSpeedLevel < -3)
	{
		GameSpeedLevel = -3;
	}

	UpdateGameSpeed();
}

exec function FasterGameSpeed()
{
	if (!bCheatsEnabled)
	{
		return;
	}

	GameSpeedLevel++;
	if (GameSpeedLevel > 3)
	{
		GameSpeedLevel = 3;
	}

	UpdateGameSpeed();
}

native function GhostPawn(bool ghosting);


exec function Ghost()
{
	if (!bCheatsEnabled)
	{
		return;
	}

	bDebugGhost = !bDebugGhost;

	bDebugFreeCam = false;
	bDebugFixedCam = false;
	SetPause(false);
	bPausedForFreeCam = false;

	if (bDebugGhost)
	{
		SetCameraMode('FreeCam');		
		ClientMessage("Ghost ON!");
	}
	else
	{
		SetCameraMode('FirstPerson');
		ClientMessage("Ghost OFF!");	

		HeroPawn.ResetAfterTeleport();
	}

	GhostPawn(bDebugGhost);	
}

exec function RechargeNightVision()
{
	local int nbBatAdded;

	if (!bCheatsEnabled)
	{
		return;
	}

	if (NumBatteries + 4 > MaxNumBatteries)
	{
		nbBatAdded = MaxNumBatteries - NumBatteries;
	}
	else
	{
		nbBatAdded = 4;
	}
	NumBatteries += nbBatAdded;
	ClientMessage("" $ nbBatAdded $ " batteries added. New battery count: " $ NumBatteries);
}

exec function ToggleUnlimitedBatteries()
{
	if (!bCheatsEnabled)
	{
		return;
	}

	bUnlimitedBatteries = !bUnlimitedBatteries;

	if (bUnlimitedBatteries)
	{
		ClientMessage("Batteries are UNLIMITED.");
	}
	else
	{
		ClientMessage("Batteries are NOT unlimited.");
	}
}

exec function KillFade()
{
	ClientSetCameraFade(false);
}

exec function Checkpoint(string CPName)
{
	StartNewGameAtCheckpoint(CPName, false);
}

exec function cp(string CPName)
{
	StartNewGameAtCheckpoint(CPName, false);
}


exec native function cplist();

exec native function ApplyCP(string CPName);

exec native function GiveItem(string ItemName);

//>> FOR DEBUGGING
exec function SaveGame(string Filename)
{
	local OLEngine Engine;
	Engine = OLEngine(class'Engine'.static.GetEngine());
	Engine.DebugSaveGame(Filename);
}

exec function LoadGame(string Filename)
{
	local OLEngine Engine;
	Engine = OLEngine(class'Engine'.static.GetEngine());
	Engine.DebugLoadGame(Filename);
}
//<<

exec function DemoMode()
{
	local OLGame olgame;
	olgame = OLGame(WorldInfo.Game);

	if (olgame != None)
	{
		olgame.bIsDemo = !olgame.bIsDemo;

		if (olgame.bIsDemo)
		{
			ClientMessage("Demo mode is ON");
		}
		else
		{
			ClientMessage("Demo mode is OFF");
		}
	}
}

exec native function SetGamma(float newGamma);

exec native function ResetDoors();
exec native function ResetPushables();
exec native function ResetWorldState();

exec function InflictDamage(optional float amount)
{
	if (!bCheatsEnabled)
	{
		return;
	}

	HeroPawn.TakeDamage(amount, None, HeroPawn.Location, vect(0, 0, 0), None);
}

exec function DamagePlayer(float Damage)
{
	if (HeroPawn != None)
		HeroPawn.TakeDamage(int(Damage), None, HeroPawn.Location, vect(0, 0, 0), None);
}

exec function KillPlayer()
{
	if (HeroPawn != None)
		HeroPawn.TakeDamage(HeroPawn.Health + 1, None, HeroPawn.Location, vect(0, 0, 0), None);
}

exec native function ToggleMute();

native function DrawDebug();

exec function SuppressAllMessages()
{
	if (!bCheatsEnabled)
	{
		return;
	}

	bSuppressAllMessages = !bSuppressAllMessages;

	if (bSuppressAllMessages)
	{
		ClientMessage("Messages are SUPPRESSED");
	}
	else
	{
		ClientMessage("Messages are NOT SUPPRESSED");
	}
}

exec function DebugGameState()
{
	if (!bCheatsEnabled)
	{
		return;
	}

	bDebugGameState = !bDebugGameState;
}

exec function DumpGS()
{
	if (!bCheatsEnabled)
	{
		return;
	}

	class'OLGameStateList'.static.DumpGameState();
}

exec native function ActivateGS(name gsName);

exec native function ResetGS();

exec function OutlastPause()
{
	if (!bCheatsEnabled)
	{
		return;
	}

	Pause();
}

exec function ToggleAstoundSound()
{
	if (!bCheatsEnabled)
	{
		return;
	}

	bEnableAstoundSound = !bEnableAstoundSound;

	SetAstoundSoundEnabled(bEnableAstoundSound);

	if (bEnableAstoundSound)
	{
		ClientMessage("AstoundSound is ENABLED");
	}
	else
	{
		ClientMessage("AstoundSound is DISABLED");
	}
}


exec function SetFinishedGame(bool hasFinishedGame, bool hasFinishedDLC)
{
	if (!bCheatsEnabled)
	{
		return;
	}

	if (hasFinishedGame)
	{
		ClientMessage("Setting FinishedGame to TRUE");
		ProfileSettings.SetProfileSettingValueId(PSI_FinishedGame, 1);
	}
	else
	{
		ClientMessage("Setting FinishedGame to FALSE");
		ProfileSettings.SetProfileSettingValueId(PSI_FinishedGame, 0);
	}

	if (hasFinishedDLC)
	{
		ClientMessage("Setting FinishedDLC to TRUE");
		ProfileSettings.SetProfileSettingValueId(PSI_FinishedDLC, 1);
	}
	else
	{
		ClientMessage("Setting FinishedDLC to FALSE");
		ProfileSettings.SetProfileSettingValueId(PSI_FinishedDLC, 0);
	}

	ClientSaveAllPlayerData();
}

native function bool IsViewModeUnlit();

event bool ProcessCheatInput(OLPlayerInput InputMgr, name Key)
{
	if (bCheatsEnabled && InputMgr.bUsingGamepad && InputMgr.IsKeyPressed('XboxTypeS_LeftThumbstick'))
	{
		if (Key == 'XboxTypeS_RightThumbstick')
		{
			Ghost();
			return true;
		}
		else if (Key == 'XboxTypeS_A')
		{
			RechargeNightVision();
			return true;
		}
		else if (Key == 'XboxTypeS_B')
		{
			ToggleFreeCam();
			return true;
		}
		else if (Key == 'XboxTypeS_X')
		{
			KillAll(class'OLEnemyPawn');
			return true;
		}
		else if (Key == 'XboxTypeS_Y')
		{
			God();
			return true;
		}
		else if (Key == 'XboxTypeS_DPad_Up')
		{
			FasterGameSpeed();
			return true;
		}
		else if (Key == 'XboxTypeS_DPad_Down')
		{
			SlowerGameSpeed();
			return true;
		}
		else if (Key == 'XboxTypeS_RightShoulder')
		{
			TeleportToFreeCam();
			return true;
		}
		else if (Key == 'XboxTypeS_LeftShoulder')
		{
			if (IsViewModeUnlit())
			{
				ConsoleCommand("viewmode lit");
			}
			else
			{
				ConsoleCommand("viewmode unlit");
			}
			return true;
		}
	}

	return false;
}

exec function Unlit()
{
	ConsoleCommand("Viewmode Unlit");
	ConsoleCommand("Show POSTPROCESS 0");
}

exec function Lit()
{
	ConsoleCommand("Viewmode Lit");
	ConsoleCommand("Show POSTPROCESS 1");
}

exec function ToggleUnlit()
{
	if (IsViewModeUnlit())
		Lit();
	else
		Unlit();
}

exec function ClearView()
{
	if (HeroPawn != None && HeroPawn.Health <= 0) {
		HeroPawn.Destroy();
		PlayerDied();
		WorldInfo.Game.RestartPlayer(Outer);
	}
	ClientSetCameraFade(false);
	ShowLoadingMovie(false);
}

exec function SpawnEnemy(string EnemyType, optional EWeapon WeaponToUse=0, optional bool ShouldAttack=true)
{
	local class<OLEnemyPawn> PawnClass;
	local OLEnemyPawn NewPawn;
	local OLBot NewBot;
	local vector SpawnLocation;
	local rotator SpawnRotation;

	if (!bCheatsEnabled)
	{
		return;
	}

	if (EnemyType ~= "Cannibal")
		PawnClass = class'OLEnemyCannibal';
	else if (EnemyType ~= "Soldier")
		PawnClass = class'OLEnemySoldier';
	else if (EnemyType ~= "Groom")
		PawnClass = class'OLEnemyGroom';
	else if (EnemyType ~= "Priest")
		PawnClass = class'OLEnemyPriest';
	else if (EnemyType ~= "Surgeon")
		PawnClass = class'OLEnemySurgeon';
	else if (EnemyType ~= "NanoCloud")
		PawnClass = class'OLEnemyNanoCloud';
	else
	{
		ClientMessage("SpawnEnemy: unknown type '" $ EnemyType $ "'. Use: Cannibal, Soldier, Groom, Priest, Surgeon, NanoCloud");
		return;
	}

	GetPlayerViewPoint(SpawnLocation, SpawnRotation);
	NewPawn = Spawn(PawnClass,, 'CustomEnemy', SpawnLocation, SpawnRotation,,true,true);
	if (NewPawn == None)
	{
		return;
	}

	NewBot = Spawn(class'OLBot');
	if (NewBot == None)
	{
		NewPawn.Destroy();
		return;
	}

	NewBot.Possess(NewPawn, false);

	NewPawn.Modifiers.bShouldAttack = ShouldAttack;
    NewPawn.Modifiers.bUseForMusic 	= true;
    NewPawn.Modifiers.WeaponToUse 	= WeaponToUse;
	NewPawn.ApplyModifiers(NewPawn.Modifiers);

	ApplySpawnedEnemySetup(NewPawn, EnemyType);
}

private function ApplySpawnedEnemySetup(OLEnemyPawn NewPawn, string EnemyType)
{
	if (EnemyType ~= "Soldier")
	{
		NewPawn.BehaviorTree = OLBTBehaviorTree(class'OLUtils'.static.LoadObjectFromModPackage(
			"", "02_AI_Behaviors.Soldier_BT", class'OLBTBehaviorTree'));
		NewPawn.VOAsset = OLAIContextualVOAsset(class'OLUtils'.static.LoadObjectFromModPackage(
			"Sewers_LD", "02_AI_Behaviors.Soldier", class'OLAIContextualVOAsset'));
	}
	else if (EnemyType ~= "Groom")
	{
		NewPawn.NetMeshPath = "DLC_Build2Exterior-01_LD|02_Groom.Groom_Shirt";
		NewPawn.Mesh.SetSkeletalMesh(SkeletalMesh(class'OLUtils'.static.LoadObjectFromModPackage(
			"DLC_Build2Exterior-01_LD", "02_Groom.Groom_Shirt", class'SkeletalMesh')));
		NewPawn.BehaviorTree = OLBTBehaviorTree(class'OLUtils'.static.LoadObjectFromModPackage(
			"DLC_Build2Floor3-01_LD", "02_AI_Behaviors.Generic_FullLoop_BT", class'OLBTBehaviorTree'));
		NewPawn.VOAsset = OLAIContextualVOAsset(class'OLUtils'.static.LoadObjectFromModPackage(
			"DLC_Build2Floor3-01_LD", "02_AI_Behaviors.Groom_soft", class'OLAIContextualVOAsset'));
		LoadDLCSoundBank("SB_DLC_VO_AI_Groom");
	}
	else if (EnemyType ~= "NanoCloud")
	{
		NewPawn.BehaviorTree = OLBTBehaviorTree(class'OLUtils'.static.LoadObjectFromModPackage(
			"", "02_AI_Behaviors.NanoCloud_BT", class'OLBTBehaviorTree'));
	}
	else if (EnemyType ~= "Surgeon")
	{
		NewPawn.NetMeshPath = "Male_ward_SE|02_Surgeon.Mesh.Surgeon";
		NewPawn.Mesh.SetSkeletalMesh(SkeletalMesh(class'OLUtils'.static.LoadObjectFromModPackage(
			"Male_ward_SE", "02_Surgeon.Mesh.Surgeon", class'SkeletalMesh')));
		NewPawn.BehaviorTree = OLBTBehaviorTree(class'OLUtils'.static.LoadObjectFromModPackage(
			"Male_ward_03_LD", "02_AI_Behaviors.Surgeon_FullLoop_BT", class'OLBTBehaviorTree'));
		NewPawn.VOAsset = OLAIContextualVOAsset(class'OLUtils'.static.LoadObjectFromModPackage(
			"Male_ward_03_LD", "02_AI_Behaviors.Surgeon", class'OLAIContextualVOAsset'));
	}
	else if (EnemyType ~= "Cannibal")
	{
		NewPawn.BehaviorTree = OLBTBehaviorTree(class'OLUtils'.static.LoadObjectFromModPackage(
			"DLC_Build1Floor2-01_LD", "02_AI_Behaviors.Surgeon_FullLoop_BT", class'OLBTBehaviorTree'));
		NewPawn.VOAsset = OLAIContextualVOAsset(class'OLUtils'.static.LoadObjectFromModPackage(
			"DLC_Build1Floor2-01_LD", "02_AI_Behaviors.Cannibal", class'OLAIContextualVOAsset'));
		LoadDLCSoundBank("SB_DLC_VO_AI_Cannibal");
	}
	else if (EnemyType ~= "Priest")
	{
		NewPawn.NetMeshPath = "|02_Priest.Pawn.Priest-01";
		NewPawn.Mesh.SetSkeletalMesh(SkeletalMesh(class'OLUtils'.static.LoadObjectFromModPackage(
			"", "02_Priest.Pawn.Priest-01", class'SkeletalMesh')));
		NewPawn.BehaviorTree = OLBTBehaviorTree(class'OLUtils'.static.LoadObjectFromModPackage(
			"Male_ward_LD", "02_AI_Behaviors.Generic_FullLoop_BT", class'OLBTBehaviorTree'));
	}

	NewPawn.InitContextualVO();
}

function SpawnEnemyAt(string EnemyType, byte WeaponToUse, bool ShouldAttack, vector SpawnLoc, rotator SpawnRot)
{
	local class<OLEnemyPawn> PawnClass;
	local OLEnemyPawn NewPawn;
	local OLBot NewBot;

	if (!bCheatsEnabled)
		return;

	if (EnemyType ~= "Cannibal")
		PawnClass = class'OLEnemyCannibal';
	else if (EnemyType ~= "Soldier")
		PawnClass = class'OLEnemySoldier';
	else if (EnemyType ~= "Groom")
		PawnClass = class'OLEnemyGroom';
	else if (EnemyType ~= "Priest")
		PawnClass = class'OLEnemyPriest';
	else if (EnemyType ~= "Surgeon")
		PawnClass = class'OLEnemySurgeon';
	else if (EnemyType ~= "NanoCloud")
		PawnClass = class'OLEnemyNanoCloud';
	else
		return;

	NewPawn = Spawn(PawnClass,, 'CustomEnemy', SpawnLoc, SpawnRot,, true, true);
	if (NewPawn == None)
		return;

	NewBot = Spawn(class'OLBot');
	if (NewBot == None)
	{
		NewPawn.Destroy();
		return;
	}

	NewBot.Possess(NewPawn, false);

	NewPawn.Modifiers.bShouldAttack = ShouldAttack;
	NewPawn.Modifiers.bUseForMusic  = true;
	NewPawn.Modifiers.WeaponToUse   = EWeapon(WeaponToUse);
	NewPawn.ApplyModifiers(NewPawn.Modifiers);

	ApplySpawnedEnemySetup(NewPawn, EnemyType);
}

exec function MakeFullProfile()
{
	SetFinishedGame(true, true);
	NativeMakeFullProfile();
}

exec native function NativeMakeFullProfile();

exec native function ReloadSoundBanks(bool bDLC);
exec native function LoadDLCSoundBank(string BankName);

exec native function SetLanguage(string LanguageCode);

exec native function DingoTest(int id);

exec function KillAllEnemies()
{
    local OLEnemyPawn E;

    if (!bCheatsEnabled)
        return;

    foreach AllActors(class'OLEnemyPawn', E)
    {
        if (E.Tag == 'MultiplayerDummyEnemy')
            continue;

        E.Destroy();
    }
}

exec function SetGameSpeed(float Speed)
{
    WorldInfo.Game.SetGameSpeed(Speed);
}

native function SetShowEditorSprites(bool bShow);

exec function ToggleFreeCamInspector()
{
    bFreeCamInspector = !bFreeCamInspector;
    SetShowEditorSprites(bFreeCamInspector);
}

exec function ToggleGrain()
{
    local OLFXManager FX;
    local OLUberPostProcessEffect Effect;
    FX = class'OLFXManager'.static.GetFXManager();
    if (FX == None || FX.CurrentUberPostEffect == None)
        return;
    Effect = FX.CurrentUberPostEffect;
    FX.bGrainDisabled = Effect.GrainOpacity > 0.0;
    if (FX.bGrainDisabled)
        Effect.GrainOpacity = 0.0;
    else
        Effect.GrainOpacity = Effect.Default.GrainOpacity;
}

// Stream a YouTube (or any) video URL via yt-dlp + ffmpeg into a Texture2DDynamic on a screen plane.
// Usage: playvideo <url>
// Usage: playvideo <YouTubeVideoID>  (e.g. playvideo dQw4w9WgXcQ)
// Full URLs are not supported in exec — pass only the 11-char video ID.
exec function PlayVideo(string VideoID)
{
    local OLBikScreen Screen;
    local OLVideoPlayer VP;
    local vector CamLoc, SpawnLoc;
    local rotator CamRot, SpawnRot;
    local string URL;

    if (VideoID == "")
        return;

    // Reconstruct the full YouTube URL from the video ID
    URL = "https://www.youtube.com/watch?v=" $ VideoID;

    Outer.GetPlayerViewPoint(CamLoc, CamRot);
    SpawnRot.Pitch = 0;
    SpawnRot.Yaw   = CamRot.Yaw;
    SpawnRot.Roll  = 0;
    SpawnLoc = CamLoc + vector(SpawnRot) * 300.0;

    Screen = Outer.Spawn(class'OLBikScreen', Outer,, SpawnLoc, SpawnRot);
    VP = Outer.Spawn(class'OLVideoPlayer', Outer,, SpawnLoc, SpawnRot);

    if (Screen != None && VP != None)
    {
        // Init screen material so VP can assign texture to it later
        Screen.Init("");
        VP.TargetScreen = Screen;
        VP.Play(URL);
    }
}

// Spawn a CEF browser on a screen plane 300 units ahead of the player.
// Usage: playbrowser [url]  (default: about:blank)
exec function PlayBrowser(string URL)
{
    local OLBikScreen Screen;
    local OLBrowser Browser;
    local vector CamLoc, SpawnLoc;
    local rotator CamRot, SpawnRot;

    Outer.GetPlayerViewPoint(CamLoc, CamRot);
    SpawnRot.Pitch = 0;
    SpawnRot.Yaw   = CamRot.Yaw;
    SpawnRot.Roll  = 0;
    SpawnLoc = CamLoc + vector(SpawnRot) * 300.0;

    Screen = Outer.Spawn(class'OLBikScreen', Outer,, SpawnLoc, SpawnRot);
    Browser = Outer.Spawn(class'OLBrowser', Outer,, SpawnLoc, SpawnRot);

    if (Screen != None && Browser != None)
    {
        Screen.Init("");
        Browser.TargetScreen = Screen;
        if (URL != "")
            Browser.StartURL = URL;
        Browser.Open();
    }
}

// Spawn a video plane 300 units in front of the player playing <Filename>.bik from OLGame/Movies/.
// Usage: playbik <filename>  (no extension)
exec function PlayBik(string Filename)
{
    local OLBikScreen Screen;
    local vector CamLoc, SpawnLoc;
    local rotator CamRot, SpawnRot;

    if (Filename == "")
        return;

    // Get camera position and facing direction
    Outer.GetPlayerViewPoint(CamLoc, CamRot);

    // Place screen 300 units ahead of camera, yaw only so it stands vertically
    SpawnRot.Pitch = 0;
    SpawnRot.Yaw   = CamRot.Yaw;
    SpawnRot.Roll  = 0;
    SpawnLoc = CamLoc + vector(SpawnRot) * 300.0;

    Screen = Outer.Spawn(class'OLBikScreen', Outer,, SpawnLoc, SpawnRot);
    if (Screen != None)
        Screen.Init(Filename);
}

exec function ScaleHero(float Scale)
{
    local OLHero Hero;
    local float  S, Ratio;

    Hero = OLHero(Outer.Pawn);
    if (Hero == None)
        return;

    S = FMax(Scale, 0.01);

    // Ratio relative to current draw scale so repeated calls work correctly
    Ratio = S / FMax(Hero.DrawScale, 0.001);

    // Visual scale
    Hero.SetDrawScale(S);

    // Collision
    Hero.SetCollisionSize(Hero.CylinderComponent.CollisionRadius * Ratio, Hero.CylinderComponent.CollisionHeight * Ratio);
    Hero.CrouchHeight *= Ratio;
    Hero.CrouchRadius *= Ratio;

    // Camera eye height
    Hero.BaseEyeHeight *= Ratio;
    Hero.EyeHeight      = Hero.BaseEyeHeight;

    // Mesh offset so feet stay on ground
    Hero.Mesh.SetTranslation(Hero.Mesh.Translation * Ratio);

    // Movement speeds — multiply current values so repeated calls compose
    Hero.NormalWalkSpeed   *= Ratio;
    Hero.NormalRunSpeed    *= Ratio;
    Hero.WaterWalkSpeed    *= Ratio;
    Hero.WaterRunSpeed     *= Ratio;
    Hero.LimpingWalkSpeed  *= Ratio;
    Hero.HobblingWalkSpeed *= Ratio;
    Hero.HobblingRunSpeed  *= Ratio;
    Hero.JumpZ             *= Ratio;
    Hero.AccelRate         *= Ratio;
}