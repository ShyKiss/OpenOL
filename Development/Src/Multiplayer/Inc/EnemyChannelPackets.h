#pragma once
#include "PacketChannels.h"

// ============================================================================
// CH_ENEMY (0x03) packet structs
// Packet header: [CH_ENEMY][ENEMY_*][payload]
//
// Variable-length string helper: [len(1)][bytes...]  max 63 chars.
// All packets start with the enemy name in this format.
// Fixed fields follow immediately after the name.
// ============================================================================

// ENEMY_SPAWN: [CH_ENEMY][ENEMY_SPAWN][namelen(1)][name][classlen(1)][class]
//              [X(4)][Y(4)][Z(4)][Yaw(2)][meshlen(1)][mesh][Weapon(1)]
//              [bColor(1)] — if 1, [R(2)][G(2)][B(2)][A(2)] follow (* 1000, I16)
// No fixed-size struct due to variable strings; encoded manually.

// ENEMY_DEL: [CH_ENEMY][ENEMY_DEL][namelen(1)][name]

// ENEMY_SMT: [CH_ENEMY][ENEMY_SMT][namelen(1)][name][FEnpcSmtBody]
#pragma pack(push, 1)
struct FEnpcSmtBody
{
    BYTE SMTType;
    INT  Param1;
    INT  Param2;
    // Optional door coords (DoorX != 0 || DoorY != 0 || DoorZ != 0)
    INT  DoorX, DoorY, DoorZ;
};
#pragma pack(pop)
#define ENPC_SMT_BODY_SIZE_NODOOR (1 + 4 + 4)
#define ENPC_SMT_BODY_SIZE        sizeof(FEnpcSmtBody)

// ENEMY_DOOR_OPEN / ENEMY_DOOR_DONE: [CH_ENEMY][type][namelen(1)][name][FEnpcDoorBody]
#pragma pack(push, 1)
struct FEnpcDoorBody
{
    INT   DoorX, DoorY, DoorZ;
    short Speed10;  // deg/s * 10
    short Angle10;  // AngleWhenOpen * 10, 0 = use door default
};
#pragma pack(pop)
#define ENPC_DOOR_BODY_SIZE sizeof(FEnpcDoorBody)

// ENEMY_DOOR_BASH / ENEMY_DOOR_BREAK: [CH_ENEMY][type][namelen(1)][name][FEnpcDoorBashBody]
#pragma pack(push, 1)
struct FEnpcDoorBashBody
{
    INT  DoorX, DoorY, DoorZ;
    BYTE bReversed;
};
#pragma pack(pop)
#define ENPC_DOOR_BASH_BODY_SIZE sizeof(FEnpcDoorBashBody)
