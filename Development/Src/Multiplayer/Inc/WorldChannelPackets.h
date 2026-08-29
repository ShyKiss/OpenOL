#pragma once
#include "PacketChannels.h"

// ============================================================================
// CH_WORLD (0x05) packet layouts
// Packet header: [CH_WORLD][WORLD_*][payload]
//
// String payloads: [strLen(1)][ASCII bytes]  (max 255 bytes)
//
// WORLD_TRIGGER_ACT    [countLE(4)][pathLen(1)][path ASCII]
// WORLD_ITEM_CONSUME   [pathLen(1)][item name ASCII]
// WORLD_PICKUP_STATE   [X(4)][Y(4)][Z(4)]
// WORLD_PICKUP_START   [X(4)][Y(4)][Z(4)]
// WORLD_PICKUP_ATTACH  [X(4)][Y(4)][Z(4)]
// WORLD_PICKUP_KISMET  [pathLen(1)][path ASCII]
// WORLD_RECORDING      [pathLen(1)][path ASCII]
// WORLD_DISCONNECT     (no payload)
// WORLD_REQUEST_STATE  [filterLen(1)][level filter ASCII]  (may be 0-len)
// WORLD_REQUEST_ENEMIES   (no payload)
// WORLD_REQUEST_DOORS     (no payload)
// WORLD_REQUEST_PUSHABLES (no payload)
// WORLD_MATINEE_STATE  [count(1)] { [pathLen(1)][path ASCII][position f32][playRate f32] } * count
// WORLD_TRIGGER_FIRE   [subtype(1)][maxCount(1)][pathLen(1)][path ASCII]
// WORLD_TRIGGER_DENIED [pathLen(1)][path ASCII]  (server→client)
// WORLD_REQUEST_TRIGGERS  (no payload)
// ============================================================================
