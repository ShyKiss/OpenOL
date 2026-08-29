class MultiplayerController extends OLPlayerController
    config(Multiplayer)
    native;

// =============================================================================
// OWNED OBJECTS
// =============================================================================

var HeroChannel         HeroChannel;
var DoorChannel         DoorChannel;
var PushableChannel     PushableChannel;
var EnemyChannel        EnemyChannel;
var WorldChannel        WorldChannel;

// =============================================================================
// SESSION STATE
// =============================================================================

var int         OnlineCount;
var float       CurrentPingMs;
var float       LastPongTime;
var string      ServerName;
var bool        bSentPlayerDied;

// =============================================================================
// REMOTE PLAYERS
// =============================================================================

var array<RemotePlayer> RemotePlayers;

// =============================================================================
// WORLD CACHES  (doors, pushables, enemies)
// =============================================================================

// --- Doors ---
struct PendingDoorState { var int KeyX, KeyY, KeyZ; var float Angle; };
var array<Actor>            CachedDoors;
var bool                    bDoorsIndexed;
var array<PendingDoorState> PendingDoorStates;
var array<float>            LastSentDoorAngle;
var array<float>            RemoteDoorLockExpiry;

// --- Pushables ---
struct PendingPushState { var int KeyX, KeyY, KeyZ; var float Displacement; };
var array<OLPushableObject> CachedPushables;
var bool                    bPushablesIndexed;
var array<PendingPushState> PendingPushStates;
var array<float>            LastSentPushDisplacement;

// --- Enemies ---
struct RemoteEnemyState
{
    var string      NetName;
    var OLEnemyPawn DummyEnemy;
    var int         OwnerID;
    var vector      TargetLoc;
    var rotator     TargetRot;
    var vector      TargetVel;
    var bool        bAnimating;
};
var array<OLEnemyPawn>      CachedEnemies;
var array<bool>             LastEnemyAlive;
var array<int>              LastEnemySMT;
var array<float>            LastBashLoopSentTime;
var array<RemoteEnemyState> RemoteEnemies;
var array<string>           SentEnemySpawns;

// =============================================================================
// KISMET / WORLD FLAGS
// =============================================================================

var array<string>   TriggerActBlacklist;
var array<string>   CSAActBlacklist;

// =============================================================================
// NATIVE INTERFACE
// =============================================================================

function string GetNetUsername()      { return NativeGetUsername(); }
function bool   GetNetSyncEnemies()   { return true; }
function bool   GetNetSyncInteractable() { return true; }

native function string NativeGetUsername();
native function NativeInit();
native function bool IsConnected();
native function bool IsReady();
native function int  FindRemoteIndex(int PlayerID);
native function int  RegisterRemotePlayer(int PlayerID, string Nick);
native function NativeSetHero(OLHero Hero);
native function OnReceiveData(string Data);
native function OnReceiveBinaryData(byte PktType, int SenderID, byte Data[255], int DataLen);
native function OnConnected();
native function OnDisconnected();
native function NativeRemoveRemotePlayer(int PlayerID);
native function NativeDestroyRemoteEnemies();
native function bool IndexDoors();
native function IndexEnemies();
native function bool IndexPushables();
native function ApplyPendingPushStates();
native function RemoveRemotePlayer(int PlayerID);
native function DestroyRemoteEnemies();
native function NotifyDummyPlayerHit(OLHero DummyTarget, float Damage, float KnockbackPower, vector HitDir);
native function NotifyDummyPlayerGrab(int TargetPlayerID, vector GrabTargetLoc, vector CharDir, bool bCrouched, int EnemyTypeInt, float BlendAlpha, bool bLeftAnim, int GrabType);
native function NotifyDummyPlayerThrow(int TargetPlayerID, float ThrowRotation);
native function NotifyDummyPlayerKill(int TargetPlayerID, int EnemyTypeInt, int WeaponType, bool bBackAnim, bool bLeftAnim, float BlendAlpha, vector AnimStart, vector CharDir, int KillType, int VictimYaw);
native function NotifyDummyEnemySMT(OLEnemyPawn EnemyPawn, int SMTType, int Param1, int Param2);
native function NativeNotifyEnemyDoorOpen(OLEnemyPawn EnemyPawn, OLDoor D, float Speed, float Angle);
native function NativeNotifyEnemyDoorDone(OLEnemyPawn EnemyPawn, OLDoor D, float CloseSpeed);
native function NotifyEnemyDoorBash(OLEnemyPawn EnemyPawn, OLDoor D, bool bReversed);
native function NotifyEnemyDoorBreak(OLEnemyPawn EnemyPawn, OLDoor D, bool bReversed);
native function NotifyPawnTouchedTrigger(Actor TriggerActor);
native function OnInventoryItemConsumed(name ItemName);
native event  OnPickupKismetEvent(OLPickableObject Pickup);
native event  OnLocalDoorOpen(OLDoor D);
native event  OnLocalDoorClose(OLDoor D);
native function OnRecordingMarkerCompleted(OLRecordingMarker Marker);
native function OnToggleCinematicMode(SeqAct_ToggleCinematicMode Action);
native simulated event InterpolationStarted(SeqAct_Interp InterpAction, InterpGroupInst GroupInst);
native simulated event InterpolationFinished(SeqAct_Interp InterpAction);

// =============================================================================
// LIFECYCLE  (UC events — super chain must go through UC)
// =============================================================================

simulated event PostBeginPlay()
{
    super.PostBeginPlay();
    HeroChannel     = new class'HeroChannel';
    DoorChannel     = new class'DoorChannel';
    PushableChannel = new class'PushableChannel';
    EnemyChannel    = new class'EnemyChannel';
    WorldChannel    = new class'WorldChannel';
    HeroChannel.ControllerOwner     = self;
    DoorChannel.ControllerOwner     = self;
    PushableChannel.ControllerOwner = self;
    EnemyChannel.ControllerOwner    = self;
    WorldChannel.ControllerOwner    = self;
    NativeInit();
}

native function NativeDestroyed();

event NotifyEnemyDoorOpen(OLEnemyPawn EnemyPawn, OLDoor D, float Speed, float Angle) { NativeNotifyEnemyDoorOpen(EnemyPawn, D, Speed, Angle); }
event NotifyEnemyDoorDone(OLEnemyPawn EnemyPawn, OLDoor D, float CloseSpeed) { NativeNotifyEnemyDoorDone(EnemyPawn, D, CloseSpeed); }

event Destroyed()
{
    NativeDestroyed();
    super.Destroyed();
}

event PlayerTick(float DeltaTime)
{
    local int i;
    super.PlayerTick(DeltaTime);
    for (i = 0; i < RemotePlayers.Length; i++)
        RemotePlayers[i].Tick(DeltaTime);
    if (!IsConnected())
        return;
    if (Pawn != None)
    {
        DoorChannel.TickSend(DeltaTime);
        PushableChannel.TickSend(DeltaTime);
        EnemyChannel.TickSend(DeltaTime);
    }
}

function PlayerDied()
{
    EnemyChannel.SendAllDeletes();
    HeroChannel.SendPlayerDied();
    bSentPlayerDied = true;
    Super.PlayerDied();
}

event Possess(Pawn inPawn, bool bVehicleTransition)
{
    local OLHero Hero;
    Super.Possess(inPawn, bVehicleTransition);
    if (HeroChannel == None)
    {
        HeroChannel     = new class'HeroChannel';
        DoorChannel     = new class'DoorChannel';
        PushableChannel = new class'PushableChannel';
        EnemyChannel    = new class'EnemyChannel';
        WorldChannel    = new class'WorldChannel';
        HeroChannel.ControllerOwner     = self;
        DoorChannel.ControllerOwner     = self;
        PushableChannel.ControllerOwner = self;
        EnemyChannel.ControllerOwner    = self;
        WorldChannel.ControllerOwner    = self;
        NativeInit();
    }
    if (inPawn != None && IsConnected())
    {
        if (bSentPlayerDied)
            HeroChannel.SendPlayerRespawned();
        DestroyRemoteEnemies();
        // Broadcast all doors/pushables (empty filter = all levels, including persistent).
        // At Possess time the world is fully loaded so FActorIterator sees everything.
        bDoorsIndexed = false;
        bPushablesIndexed = false;
        DoorChannel.BroadcastDoorStates("");
        PushableChannel.BroadcastPushableStates();
        if (GetNetSyncEnemies())
            HeroChannel.SendRequestEnemies();
        if (GetNetSyncInteractable())
        {
            HeroChannel.SendRequestDoors();
            HeroChannel.SendRequestPushables();
        }
    }
    bSentPlayerDied = false;
    Hero = OLHero(inPawn);
    HeroChannel.HeroPawn     = Hero;
    DoorChannel.HeroPawn     = Hero;
    PushableChannel.HeroPawn = Hero;
    EnemyChannel.HeroPawn    = Hero;
    WorldChannel.HeroPawn    = Hero;
    NativeSetHero(Hero);
}

event StartNewGameAtCheckpoint(string CheckpointStr, bool bSaveToDisk)
{
    EnemyChannel.SendAllDeletes();
    CachedDoors.Length              = 0;
    LastSentDoorAngle.Length        = 0;
    RemoteDoorLockExpiry.Length     = 0;
    CachedPushables.Length          = 0;
    LastSentPushDisplacement.Length = 0;
    CachedEnemies.Length            = 0;
    LastEnemyAlive.Length           = 0;
    LastEnemySMT.Length             = 0;
    LastBashLoopSentTime.Length     = 0;
    SentEnemySpawns.Length          = 0;
    DestroyRemoteEnemies();
    PendingDoorStates.Length        = 0;
    bExcludeFromKismetPlayer        = false;
    bTriggerActObserver             = false;
    Super.StartNewGameAtCheckpoint(CheckpointStr, bSaveToDisk);
}

event OnPlayerDisconnected(int PlayerID)
{
    RemoveRemotePlayer(PlayerID);
}

// Send door and pushable snapshots to the server whenever a level becomes visible.
// This populates the server's snapshot map so newcomers receive accurate initial state.
event OnLevelBecameVisible(string PackageName)
{
    if (!IsConnected())
        return;
    // Re-index so newly streamed-in doors/pushables are discovered, then broadcast
    // only the doors belonging to this package (filter keeps the burst small).
    bDoorsIndexed = false;
    bPushablesIndexed = false;
    DoorChannel.BroadcastDoorStates(PackageName);
    // Pushables have no per-level filter — broadcast all (few in practice).
    PushableChannel.BroadcastPushableStates();
    // Request authoritative state from server for newly visible objects —
    // they may have been moved/opened by the other player while this level was unloaded.
    if (GetNetSyncInteractable())
    {
        HeroChannel.SendRequestDoors();
        HeroChannel.SendRequestPushables();
    }
}

// =============================================================================
// DEFAULT PROPERTIES
// =============================================================================

DefaultProperties
{
    //Male Ward
    CSAActBlacklist(0)="male_ward_se.TheWorld:PersistentLevel.OLCSA_0"

    //Prison Block
    TriggerActBlacklist(0)="prisonfloor_02c-art.TheWorld:PersistentLevel.Trigger_4"
    TriggerActBlacklist(1)="prison_01-ld.TheWorld:PersistentLevel.Trigger_28"
    TriggerActBlacklist(2)="prison_01-ld.TheWorld:PersistentLevel.Trigger_8"
    TriggerActBlacklist(3)="prisonfloor_02a-art.TheWorld:PersistentLevel.Trigger_18"
    TriggerActBlacklist(4)="prisonfloor_02a-art.TheWorld:PersistentLevel.Trigger_33"
    TriggerActBlacklist(5)="prisonoldcells_01-art.TheWorld:PersistentLevel.Trigger_3"
    TriggerActBlacklist(6)="prisonisolationcells_01-art.TheWorld:PersistentLevel.Trigger_5"
    TriggerActBlacklist(7)="prisonfloor_01a-art.TheWorld:PersistentLevel.Trigger_25"
    TriggerActBlacklist(8)="prison_01-ld.TheWorld:PersistentLevel.Trigger_5"
    TriggerActBlacklist(9)="prisonfloor_01a-art.TheWorld:PersistentLevel.Trigger_2"
    
    //Female Ward
    TriggerActBlacklist(10)="central_courtyard_snd.TheWorld:PersistentLevel.Trigger_10"
    TriggerActBlacklist(11)="femaleward_snd.TheWorld:PersistentLevel.Trigger_11"
    TriggerActBlacklist(12)="femaleward_floor1-ld.TheWorld:PersistentLevel.Trigger_17"
    TriggerActBlacklist(13)="x_checkpoints.TheWorld:PersistentLevel.Trigger_5"
    TriggerActBlacklist(14)="femaleward_floor1-ld.TheWorld:PersistentLevel.Trigger_41"
    TriggerActBlacklist(15)="central_courtyard_01-ld.TheWorld:PersistentLevel.Trigger_0"

    //Lab
    TriggerActBlacklist(16)="lab_ld.TheWorld:PersistentLevel.Trigger_63"
    TriggerActBlacklist(17)="lab_02.TheWorld:PersistentLevel.Trigger_5"
    TriggerActBlacklist(18)="lab_ld.TheWorld:PersistentLevel.TriggerVolume_2"
    TriggerActBlacklist(19)="lab_02.TheWorld:PersistentLevel.Trigger_7"
    TriggerActBlacklist(20)="lab_ld.TheWorld:PersistentLevel.Trigger_5"
}

