#pragma once
#include "PacketChannels.h"

// ============================================================================
// CH_PUSH (0x04) packet structs
// Packet header: [CH_PUSH][PUSH_*][payload]
// ============================================================================

// PUSH_STATE
// Layout: [CH_PUSH][PUSH_STATE][FPushStatePacket]
// KeyX/KeyY/KeyZ = int(Location.X/Y/Z) — stable actor key.
// DispX1000      = CurrentDisplacement * 1000, stored as I32.
// Seq            = monotonically increasing per-pushable sequence number (drop old packets).
// bPushing       = 1 while actively pushing, 0 when released (sent 5x redundantly).
#pragma pack(push, 1)
struct FPushStatePacket
{
    int32_t  KeyX;
    int32_t  KeyY;
    int32_t  KeyZ;
    int32_t  DispX1000;
    uint32_t Seq;
    uint8_t  bPushing;
};
#pragma pack(pop)

// PUSH_DENIED — server→client: push rejected (another player owns the object).
// Layout: [CH_PUSH][PUSH_DENIED][KeyX(4)][KeyY(4)][KeyZ(4)]

