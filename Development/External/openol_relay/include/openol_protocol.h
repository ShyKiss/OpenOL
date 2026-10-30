/*=============================================================================
    openol_protocol.h - packet IDs shared by the game client and the relay.

    This is the only place where packet numbers are defined. The standalone
    relay keeps an identical copy in Multiplayer/server/src, verified by
    Multiplayer/Tests/test_relay.py. Must stay valid C for the relay build.

    Client -> server:  [channel(1)][type(1)][player_id LE4][payload...]
    Server -> client:  relayed packets keep the layout above, with player_id
                       being the sender (0 for packets from the server itself);
                       server notifications are [type >= 0xE0][0 LE4][payload...]
=============================================================================*/
#pragma once

#define OL_HEADER_SIZE 6

/* Channel IDs */
#define CH_SRV   0x00   /* Server control (HELLO, PING) */
#define CH_HERO  0x01   /* Hero/player state */
#define CH_DOOR  0x02   /* Door interactions */
#define CH_ENEMY 0x03   /* Enemy NPC */
#define CH_PUSH  0x04   /* Pushable objects */
#define CH_WORLD 0x05   /* World events, pickups, triggers, matinees */

/* CH_SRV */
#define SRV_HELLO  0x01   /* [nickLen(1)][nick ASCII] */
#define SRV_PING   0x02   /* [sent_ms LE u32(4)] */

/* CH_HERO */
#define HERO_STATE            0x01   /* binary state snapshot */
#define HERO_NICK             0x02   /* [nickLen(1)][nick ASCII] - broadcast after HELLO */
#define HERO_HEAD_ROT         0x03   /* FHeadRotPacket */
#define HERO_MESH_PRESET      0x04   /* FMeshPresetPacket */
#define HERO_CINEMATIC_ANIM   0x05   /* [bStop(1)][animPathLen(1)][path ASCII] */
#define HERO_SMT_TYPE         0x06   /* FSmtTypePacket */
#define HERO_PLAYER_EVENT     0x07   /* FPlayerEventPacket */
#define HERO_PLAYER_LIFECYCLE 0x08   /* [event(1)]: 0=Died 1=Respawned */

/* CH_DOOR */
#define DOOR_LOCK    0x01   /* FDoorLockPacket */
#define DOOR_UNLOCK  0x02   /* FDoorUnlockPacket */
#define DOOR_STATE   0x03   /* FDoorStatePacket */
#define DOOR_OPEN    0x04   /* FDoorOpenPacket */
#define DOOR_CLOSE   0x05   /* FDoorOpenPacket (same layout) */
#define DOOR_ANGLE   0x06   /* FDoorAnglePacket */
#define DOOR_PARAMS  0x07   /* FDoorParamsPacket */
#define DOOR_DENY    0x08   /* FDoorDenyPacket (server->client) */
#define DOOR_INIT    0x09   /* FDoorAnglePacket (first-write-wins registration) */

/* CH_ENEMY */
#define ENEMY_SPAWN      0x01   /* [namelen(1)][name][classlen(1)][class][X][Y][Z][Yaw]... */
#define ENEMY_DEL        0x02   /* [namelen(1)][name] */
#define ENEMY_SMT        0x03   /* [namelen(1)][name][FEnpcSmtBody] */
#define ENEMY_LOC        0x04   /* [namelen(1)][name][0][X][Y][Z][Yaw]... */
#define ENEMY_DOOR_OPEN  0x05   /* [namelen(1)][name][FEnpcDoorBody] */
#define ENEMY_DOOR_DONE  0x06   /* [namelen(1)][name][FEnpcDoorBody] */
#define ENEMY_DOOR_BASH  0x07   /* [namelen(1)][name][FEnpcDoorBashBody] */
#define ENEMY_DOOR_BREAK 0x08   /* [namelen(1)][name][FEnpcDoorBashBody] */

/* CH_PUSH */
#define PUSH_STATE  0x01   /* FPushStatePacket */
#define PUSH_DENIED 0x02   /* [KeyX(4)][KeyY(4)][KeyZ(4)] (server->client, player_id=0) */
#define PUSH_INIT   0x03   /* FPushStatePacket (first-write-wins, never relayed) */

/* CH_WORLD */
#define WORLD_TRIGGER_ACT       0x01   /* [countLE(4)][pathLen(1)][path ASCII] */
#define WORLD_ITEM_CONSUME      0x02   /* [pathLen(1)][item name ASCII] */
#define WORLD_PICKUP_STATE      0x03   /* [X(4)][Y(4)][Z(4)] */
#define WORLD_PICKUP_START      0x04   /* [X(4)][Y(4)][Z(4)] */
#define WORLD_PICKUP_ATTACH     0x05   /* [X(4)][Y(4)][Z(4)] */
#define WORLD_PICKUP_KISMET     0x06   /* [pathLen(1)][path ASCII] */
#define WORLD_RECORDING         0x07   /* [pathLen(1)][path ASCII] */
#define WORLD_DISCONNECT        0x08   /* (no payload) */
#define WORLD_REQUEST_STATE     0x09   /* [filterLen(1)][level filter ASCII] */
#define WORLD_REQUEST_ENEMIES   0x0A   /* (no payload) */
#define WORLD_REQUEST_DOORS     0x0B   /* (no payload) */
#define WORLD_REQUEST_PUSHABLES 0x0C   /* (no payload) */
#define WORLD_MATINEE_STATE     0x0D   /* [count(1)] { [pathLen(1)][path][pos f32][rate f32] } * N */
#define WORLD_TRIGGER_FIRE      0x0E   /* [subtype(1)][maxCount(1)][pathLen(1)][path ASCII] */
#define WORLD_TRIGGER_DENIED    0x0F   /* [pathLen(1)][path ASCII] (server->client) */
#define WORLD_REQUEST_TRIGGERS  0x10   /* (no payload) */

/* Server -> client notifications */
#define SRV_READY        0xE0   /* [player_id LE(4)][name_len(1)][name ASCII][token(32)] */
#define SRV_ONLINE_COUNT 0xE1   /* [count LE(4)] */
#define SRV_HELLO_FAIL   0xE2   /* [reason_len(1)][reason ASCII] */
#define SRV_DISCONNECT   0xE3   /* [player_id LE(4)] - remote player disconnected */
#define SRV_DOOR_DENY    0xE4   /* [key_len(1)][key ASCII] */
