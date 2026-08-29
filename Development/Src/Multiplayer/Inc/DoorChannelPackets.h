#pragma once
#include "PacketChannels.h"

// ============================================================================
// CH_DOOR (0x02) packet structs
// Packet header: [CH_DOOR][DOOR_*][payload]
//
// All door packets use INT location key (X,Y,Z rounded to nearest int).
// Floats sent as INT * 1000 to avoid locale issues with decimal separator.
// ============================================================================

// DOOR_LOCK
// [CH_DOOR][DOOR_LOCK][FDoorLockPacket]  Total: 2 + 37 = 39 bytes
#pragma pack(push, 1)
struct FDoorLockPacket
{
    INT  X, Y, Z;
    BYTE OpeningType;
    INT  GrabPosX, GrabPosY, GrabPosZ;   // * 10 (cm precision)
    INT  GrabDirX, GrabDirY, GrabDirZ;   // * 10000 (unit vector precision)
    BYTE PartialOpenType;
    BYTE ClosingType;
    BYTE bQuiet;
};
#pragma pack(pop)
#define DOOR_LOCK_SIZE (2 + sizeof(FDoorLockPacket))

// DOOR_UNLOCK
// [CH_DOOR][DOOR_UNLOCK][FDoorUnlockPacket]  Total: 2 + 12 = 14 bytes
#pragma pack(push, 1)
struct FDoorUnlockPacket
{
    INT X, Y, Z;
};
#pragma pack(pop)
#define DOOR_UNLOCK_SIZE (2 + sizeof(FDoorUnlockPacket))

// DOOR_STATE — interactive angle update (while holding door) and locker exit.
// [CH_DOOR][DOOR_STATE][FDoorStatePacket]  Total: 2 + 20 = 22 bytes
#pragma pack(push, 1)
struct FDoorStatePacket
{
    INT X, Y, Z;
    INT AngleX1000;
    INT SpeedX1000;
};
#pragma pack(pop)
#define DOOR_STATE_SIZE (2 + sizeof(FDoorStatePacket))

// DOOR_OPEN / DOOR_CLOSE
// [CH_DOOR][DOOR_OPEN/CLOSE][FDoorOpenPacket]  Total: 2 + 12 = 14 bytes
#pragma pack(push, 1)
struct FDoorOpenPacket
{
    INT X, Y, Z;
};
#pragma pack(pop)
#define FDoorClosePacket FDoorOpenPacket
#define DOOR_OPEN_SIZE  (2 + sizeof(FDoorOpenPacket))
#define DOOR_CLOSE_SIZE DOOR_OPEN_SIZE

// DOOR_ANGLE / DOOR_INIT — initial angle snapshot.
// [CH_DOOR][DOOR_ANGLE/INIT][FDoorAnglePacket]  Total: 2 + 16 = 18 bytes
#pragma pack(push, 1)
struct FDoorAnglePacket
{
    INT X, Y, Z;
    INT AngleX1000;
};
#pragma pack(pop)
#define DOOR_ANGLE_SIZE (2 + sizeof(FDoorAnglePacket))
#define DOOR_INIT_SIZE  DOOR_ANGLE_SIZE

// DOOR_PARAMS — sent before SMT for door SMTs 29-36.
// [CH_DOOR][DOOR_PARAMS][FDoorParamsPacket]  Total: 2 + 4 = 6 bytes
#pragma pack(push, 1)
struct FDoorParamsPacket
{
    BYTE OpeningType;
    BYTE PartialOpenType;
    BYTE ClosingType;
    BYTE bQuiet;
};
#pragma pack(pop)
#define DOOR_PARAMS_SIZE (2 + sizeof(FDoorParamsPacket))

// DOOR_DENY — server→client, sent when server rejects a DOOR_LOCK.
// [CH_DOOR][DOOR_DENY][FDoorDenyPacket]  Total: 2 + 12 = 14 bytes
#pragma pack(push, 1)
struct FDoorDenyPacket
{
    INT X, Y, Z;
};
#pragma pack(pop)
#define DOOR_DENY_SIZE (2 + sizeof(FDoorDenyPacket))
