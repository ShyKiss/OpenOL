#pragma once

extern "C" {
#include "server.h"
}

// Runs the ImGui event loop.
// db is pre-loaded; gui can edit db.config and start/stop the server.
// Returns exit code.
int gui_run(Server *s, volatile int *running, DB *db);

// ---------------------------------------------------------------------------
// EWeapon name lookup (mirrors OLEnemyPawn.uc)
// ---------------------------------------------------------------------------
static inline const char *weapon_name(int v) {
    switch (v) {
        case 0: return "None";
        case 1: return "Knife";
        case 2: return "ButcherKnife";
        case 3: return "BoneShear";
        case 4: return "Machete";
        case 5: return "NightStick";
        case 6: return "Pipe";
        case 7: return "WoodPlank";
        case 8: return "CannibalDrill";
        default: return "?";
    }
}

// ---------------------------------------------------------------------------
// ELocomotionMode name lookup (mirrors OLPawn.uc)
// ---------------------------------------------------------------------------
static inline const char *loco_name(int v) {
    switch (v) {
        case  0: return "Walk";
        case  1: return "Fall";
        case  2: return "SpecialMove";
        case  3: return "Ladder";
        case  4: return "LedgeHang";
        case  5: return "LedgeWalk";
        case  6: return "Squeeze";
        case  7: return "Door";
        case  8: return "Locker";
        case  9: return "Cinematic";
        case 10: return "Bed";
        case 11: return "LookBack";
        case 12: return "Struggle";
        case 13: return "Grabbed";
        case 14: return "Pushing";
        case 15: return "ContextualLean";
        default: return "?";
    }
}

// ---------------------------------------------------------------------------
// ESpecialMoveType name lookup (mirrors OLPawn.uc, 0-100 + MP codes 118-121)
// ---------------------------------------------------------------------------
static inline const char *smt_name(int v) {
    switch (v) {
        case   0: return "None";
        case   1: return "Crouch";
        case   2: return "Uncrouch";
        case   3: return "JumpOnSpot";
        case   4: return "BigLanding";
        case   5: return "JumpOver";
        case   6: return "JumpOverAndGrabLedge";
        case   7: return "SlideOver";
        case   8: return "ClimbUpObstacle";
        case   9: return "ClimbUpWall";
        case  10: return "ClimbOverWall";
        case  11: return "StepUpAndLand";
        case  12: return "EnterLookBack";
        case  13: return "ExitLookBack";
        case  14: return "GrabLedgeFromGround";
        case  15: return "GrabLedgeFromAir";
        case  16: return "LedgeHangTransition";
        case  17: return "ClimbUpLedge";
        case  18: return "DropFromLedge";
        case  19: return "GrabAndClimb";
        case  20: return "EnterLedgeWalk";
        case  21: return "ExitLedgeWalk";
        case  22: return "LedgeWalkTransition";
        case  23: return "JumpFromLedgeWalk";
        case  24: return "EnterSqueeze";
        case  25: return "ExitSqueeze";
        case  26: return "AutomaticSqueeze";
        case  27: return "SqueezeReload";
        case  28: return "EnterDoorInteraction";
        case  29: return "OpenDoorInstant";
        case  30: return "OpenDoorPartial";
        case  31: return "TryOpenLockedDoor";
        case  32: return "RunThroughDoor";
        case  33: return "CloseDoor";
        case  34: return "CloseDoorPositionned";
        case  35: return "ClearClosingDoor";
        case  36: return "DoorClosedFromOtherSide";
        case  37: return "OpenLockerFromOutside";
        case  38: return "EnterLocker";
        case  39: return "ExitLocker";
        case  40: return "EnterBed";
        case  41: return "ExitBed";
        case  42: return "BedReload";
        case  43: return "EnterLadderFromGround";
        case  44: return "EnterLadderFromAbove";
        case  45: return "ExitLadderOnGround";
        case  46: return "ExitLadderOnTop";
        case  47: return "DropFromLadder";
        case  48: return "GrabLadderFromAir";
        case  49: return "PickupObject";
        case  50: return "CSA";
        case  51: return "EnterStruggle";
        case  52: return "ExitStruggle";
        case  53: return "KilledInStruggle";
        case  54: return "StartPushingObject";
        case  55: return "StopPushingObject";
        case  56: return "PushFromLedgeProcedural";
        case  57: return "PushFromLedgeAnimated";
        case  58: return "EnterContextualLean";
        case  59: return "ExitContextualLean";
        case  60: return "ExitContextualLeanForward";
        case  61: return "ContextualLeanInsideTransition";
        case  62: return "HeroGrabbedNormal";
        case  63: return "HeroGrabbedFromSqueeze";
        case  64: return "HeroGrabbedFromLocker";
        case  65: return "HeroGrabbedFromBed";
        case  66: return "HeroGrabbedFromUnder";
        case  67: return "HeroThrown";
        case  68: return "HeroDecapitate";
        case  69: return "HeroKilled";
        case  70: return "Dying";
        case  71: return "AttackNormal";
        case  72: return "AttackGrab";
        case  73: return "AttackLocker";
        case  74: return "AttackBed";
        case  75: return "AttackGrabUnder";
        case  76: return "AttackCrouch";
        case  77: return "AttackQuick";
        case  78: return "AttackPush";
        case  79: return "AttackSqueezeStart";
        case  80: return "AttackSqueezeStartToWait";
        case  81: return "AttackSqueezeWaitToFail";
        case  82: return "AttackSqueezeWaitToSuccess";
        case  83: return "AttackSqueezeSuccess";
        case  84: return "ThrowHero";
        case  85: return "KillHero";
        case  86: return "InvestigateLocker";
        case  87: return "InvestigateBed";
        case  88: return "Bash";
        case  89: return "BashDoorStart";
        case  90: return "BashDoorLoop";
        case  91: return "BashDoorFinish";
        case  92: return "BashDoorFailed";
        case  93: return "Avoiding";
        case  94: return "Knockedback";
        case  95: return "TurnOnSpot";
        case  96: return "AIVault";
        case  97: return "NanoThroughDoor";
        case  98: return "Disturbed";
        case  99: return "AlignAnim";
        case 100: return "SlideToNavMesh";
        case 118: return "MP_WallToPeek";
        case 119: return "MP_LeavePeek";
        case 120: return "MP_WallExit";
        case 121: return "MP_WallTransition";
        default:  return "?";
    }
}
