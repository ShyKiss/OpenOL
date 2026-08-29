#pragma once
#include "PacketChannels.h"

// ============================================================================
// CH_HERO (0x01) packet structs
// Packet header: [CH_HERO][HERO_*][payload]
// ============================================================================

// HERO_HEAD_ROT payload
#pragma pack(push, 1)
struct FHeadRotPacket
{
    INT  CamPitch;   // raw rotator units
    INT  CamYaw;     // raw rotator units
};
#pragma pack(pop)

// HERO_MESH_PRESET payload
#pragma pack(push, 1)
struct FMeshPresetPacket
{
    BYTE PresetIndex;
};
#pragma pack(pop)

// HERO_SMT_TYPE payload
#pragma pack(push, 1)
struct FSmtTypePacket
{
    INT     SMT;

    // Grab position (AdjustPosition target / expectedAnimStart)
    FLOAT   GrabPosX;
    FLOAT   GrabPosY;
    FLOAT   GrabPosZ;

    // Grab direction (AdjustPosition dir / expectedAnimFwd)
    FLOAT   GrabDirX;
    FLOAT   GrabDirY;
    FLOAT   GrabDirZ;

    // Anim length hint — used to set DummySMTLockUntil on receiver
    FLOAT   GrabLength;

    // Blend alpha (SMT_HeroKilled / Decapitate)
    INT     BlendAlphaX1000;

    // Yaw target (SMT_HeroThrown)
    INT     SpecialMoveTargetYawX1000;

    // Enemy params (SMT_HeroKilled / Decapitate)
    BYTE    EnemyType;
    BYTE    EnemyWeapon;

    // Ledge / climb enums
    BYTE    LedgeTransitionType;    // ELedgeTransitionType (SMT 16,20,21,22)
    BYTE    LedgeClimbType;         // ELedgeClimbType      (SMT 17)

    // Door params (SMT 29-36)
    BYTE    DoorOpeningType;
    BYTE    DoorPartialOpenType;
    BYTE    DoorClosingType;
    BYTE    bQuietDoorInteraction;

    // Pickup params (SMT 49)
    INT     PickupDist2DX10;
    INT     PickupDeltaZX10;
    BYTE    bPickupCrouched;
    BYTE    bPickupIsCollectible;

    // Bool flags
    BYTE    bLeftAnim;              // SMT 24,25,26,40,41,43,62,68,69
    BYTE    bPushingFromBackEdge;   // SMT 54,55
    BYTE    bExitLadderLeftHand;    // SMT 46
    BYTE    bIsCrouched;            // SMT 62
    BYTE    bBackAnim;              // SMT 68,69
    BYTE    bRunningTraversalMove;  // SMT 5,8
    BYTE    bMustCrouchAfterSMT;    // SMT 8
    BYTE    bJumpRun;               // SMT 5

    // ContextualLean params (SMT_EnterContextualLean)
    BYTE    bPeekFromLeft;
    BYTE    bPeekRounded;
    FLOAT   CornerLocX;
    FLOAT   CornerLocY;
    FLOAT   CornerLocZ;
    FLOAT   CornerFwdX;
    FLOAT   CornerFwdY;
    FLOAT   CornerFwdZ;

    // CSA params (SMT_CSA): anim name + object path, both length-prefixed ASCII
    BYTE    CSAAnimLen;
    BYTE    CSAAnimName[31];
    BYTE    CSAPathLen;
    BYTE    CSAPath[127];

    // Struggle params (SMT_EnterStruggle)
    BYTE    StruggleEntryAnimPlayerLen;
    BYTE    StruggleEntryAnimPlayer[63];
    BYTE    StruggleCycleAnimPlayerLen;
    BYTE    StruggleCycleAnimPlayer[63];
    BYTE    StruggleCycleAnimEnemyLen;
    BYTE    StruggleCycleAnimEnemy[63];
    BYTE    StruggleAnimSetPathLen;
    BYTE    StruggleAnimSetPath[127];

    BYTE    _pad[2];
};
#pragma pack(pop)

// HERO_PLAYER_EVENT payload
enum EPlayerEventType
{
    PEVT_Hit   = 0,
    PEVT_Grab  = 1,
    PEVT_Throw = 2,
    PEVT_Kill  = 3,
};

#pragma pack(push, 1)
struct FPlayerEventPacket
{
    INT   TargetPlayerID;
    BYTE  EventType;        // EPlayerEventType

    // HIT
    INT   DamageX1;
    INT   KnockbackX1;
    INT   HitDirX1000[3];

    // GRAB / KILL
    INT   LocX10[3];
    INT   DirX10000[3];
    INT   BlendAlphaX10000;
    INT   EnemyTypeInt;
    INT   WeaponType;
    INT   KillType;
    INT   VictimYaw;
    INT   ThrowRotX100000;
    INT   GrabType;
    BYTE  bCrouched;
    BYTE  bBackAnim;
    BYTE  bLeftAnim;
    BYTE  _pad;
};
#pragma pack(pop)

// HERO_PLAYER_LIFECYCLE sub-events
#define LIFECYCLE_DIED       0
#define LIFECYCLE_RESPAWNED  1
