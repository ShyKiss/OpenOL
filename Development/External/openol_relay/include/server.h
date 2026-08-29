/*=============================================================================
    server.h - shared types and platform abstractions
=============================================================================*/
#pragma once

#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "db.h"

// ---------------------------------------------------------------------------
// Platform
// ---------------------------------------------------------------------------

#ifdef _WIN32
   /* Do NOT include winsock2.h or ws2tcpip.h here — server.h may be compiled
      in a UE3 Unity build where windows.h already came first via MinWindows.h,
      and ws2tcpip.h requires winsock2 symbols that are not available after
      windows.h alone.  Instead, define sock_t as a fixed-size opaque integer
      (UINT_PTR = 8 bytes on x64) matching the SOCKET typedef in winsock2.h,
      so the Server struct layout is identical regardless of which winsock
      header the translation unit happens to include first.
      server.c / relay_api.c include winsock2.h themselves before server.h. */
#  if !defined(_WINSOCK2API_) && !defined(_WINSOCK_H)
     /* winsock2.h not yet included — bring in minimal declarations. */
#    ifndef _WINDOWS_
#      define WIN32_LEAN_AND_MEAN
#      ifndef NOMINMAX
#        define NOMINMAX
#      endif
#      include <winsock2.h>
#    endif
#  endif
#  if defined(_MSC_VER)
#    pragma comment(lib, "ws2_32.lib")
#  endif
   /* Use UINT_PTR (always pointer-sized) as the socket handle type so the
      struct layout is stable whether SOCKET came from winsock.h or winsock2.h. */
   typedef UINT_PTR sock_t;
   typedef int      socklen_t;
#  define INVALID_SOCK  ((sock_t)(~0))
#  define sock_close(s) closesocket((SOCKET)(s))
#  define SOCK_ERR      WSAGetLastError()
#  define SOCK_WOULDBLOCK WSAEWOULDBLOCK
static inline void platform_init() { WSADATA w; WSAStartup(MAKEWORD(2,2),&w); }
static inline void platform_cleanup() { WSACleanup(); }
static inline double mono_now() {
    LARGE_INTEGER f, c;
    QueryPerformanceFrequency(&f);
    QueryPerformanceCounter(&c);
    return (double)c.QuadPart / (double)f.QuadPart;
}
#else
#  include <sys/socket.h>
#  include <sys/select.h>
#  include <netinet/in.h>
#  include <arpa/inet.h>
#  include <unistd.h>
#  include <fcntl.h>
#  include <errno.h>
   typedef int      sock_t;
#  define INVALID_SOCK  (-1)
#  define sock_close(s) close(s)
#  define SOCK_ERR      errno
#  define SOCK_WOULDBLOCK EAGAIN
static inline void platform_init() {}
static inline void platform_cleanup() {}
static inline double mono_now() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (double)ts.tv_sec + (double)ts.tv_nsec * 1e-9;
}
#endif

// ---------------------------------------------------------------------------
// Pack all relay structs tightly so layout is identical regardless of the
// #pragma pack state active in the including translation unit (UE3 Unity
// builds have pack(4) active from CoreClasses.h when server.h is included).
// ---------------------------------------------------------------------------
#ifdef _MSC_VER
#  pragma pack(push, 8)
#endif

// ---------------------------------------------------------------------------
// Constants
// ---------------------------------------------------------------------------

#define MAX_CLIENTS     128
#define MAX_PACKET      4096
#define MAX_LINE        4096
#define MAX_ROOMS       64
#define MAX_ROOM_CODE   33
#define MAX_NICK        33
#define MAX_PASSWORD    64
#define MAX_IP          46      // IPv6 max

#define DEFAULT_PORT    7777
#define DEFAULT_IP      "0.0.0.0"
#define HEARTBEAT_SEC   5.0
#define TIMEOUT_SEC     30.0
#define MAX_RATE        10000   // packets per second per player

#define MAX_DOOR_SNAPSHOTS  512
#define MAX_DOOR_KEY        32   // "X,Y,Z" int coords
#define MAX_DOOR_SNAP_DATA  256

#define MAX_PUSH_SNAPSHOTS  512
#define MAX_PUSH_KEY        32   // "X,Y,Z" int coords
#define MAX_PUSH_SNAP_DATA  32   // MPKT_PUSH_STATE is small (17 bytes + header)

#define MAX_ENEMY_SNAPSHOTS  1024
#define MAX_ENEMY_NAME       48
#define MAX_ENEMY_SNAP_DATA  512

#define MAX_PICKUP_SNAPSHOTS 2048
#define MAX_PICKUP_KEY       256  // PathName or "X,Y,Z"
#define MAX_PICKUP_SNAP_DATA 320

// Snapshot drip-feed: max packets sent to a newcomer per server tick.
// Prevents sendto() bursts from flooding the socket buffer on join.
#define SNAP_DRIP_PER_TICK  32
// Max snapshot queue depth per player.
// Covers typical room state: ~10 players + ~200 doors*3 + ~300 enemies*3 + ~500 pickups = ~1710.
// 2048 gives comfortable headroom without excessive memory use (~128 MB total for all players).
#define SNAP_QUEUE_MAX      2048
#define SNAP_PKT_MAX        512   // max single snapshot packet size

// ---------------------------------------------------------------------------
// GUI packet event ring (SPSC: server thread writes, GUI thread reads)
// ---------------------------------------------------------------------------

// Packet categories shown as sub-nodes in the Graph tab
typedef enum {
    GUIPKT_LOC     = 0,  // 0x01 hero state (throttled)
    GUIPKT_SMT     = 1,  // 0x03 hero SMT
    GUIPKT_DOOR    = 2,  // 0x10-0x15 door events
    GUIPKT_RESPAWN = 3,  // 0x05 or other respawn/death packets
    GUIPKT_ENPC_SMT= 4,  // 0x1A enemy SMT
} GuiPktCategory;

#define GUI_RING_SIZE 256  // power of two

typedef struct {
    int            sender_slot;   // index in players[]
    GuiPktCategory category;
} GuiPacketEvent;

typedef struct {
    GuiPacketEvent   buf[GUI_RING_SIZE];
    volatile unsigned head;  // written by server thread
    volatile unsigned tail;  // consumed by GUI thread
} GuiEventRing;

// History ring — human-readable event log (connect/disconnect/SMT/door/respawn/etc.)
#define HISTORY_RING_SIZE 512  // power of two
#define HISTORY_MSG_LEN   128

typedef struct {
    char msg[HISTORY_MSG_LEN];
} HistoryEntry;

typedef struct {
    HistoryEntry     buf[HISTORY_RING_SIZE];
    volatile unsigned head;  // written by server thread
    volatile unsigned tail;  // consumed by GUI thread (snapshot index)
} HistoryRing;

// ---------------------------------------------------------------------------
// Addr
// ---------------------------------------------------------------------------

typedef struct {
    struct sockaddr_in sa;
} Addr;

static inline int addr_eq(const Addr *a, const Addr *b) {
    return a->sa.sin_addr.s_addr == b->sa.sin_addr.s_addr
        && a->sa.sin_port        == b->sa.sin_port;
}

static inline void addr_ip_str(const Addr *a, char *buf, int len) {
    /* inet_ntoa requires only winsock.h v1 (available in any UE3 Unity build).
       Result is a static thread-unsafe buffer — copy immediately. */
    const char *s = inet_ntoa(a->sa.sin_addr);
    if (s) { strncpy(buf, s, (size_t)len); buf[len - 1] = '\0'; }
    else   { buf[0] = '\0'; }
}

// ---------------------------------------------------------------------------
// Room
// ---------------------------------------------------------------------------

typedef struct {
    char     code[MAX_ROOM_CODE];
    char     password[MAX_PASSWORD];
    int      active;
} Room;

// ---------------------------------------------------------------------------
// Pushable snapshot (per room) — records latest displacement of a pushable
// ---------------------------------------------------------------------------

typedef struct {
    char key[MAX_PUSH_KEY];            // "X,Y,Z" of pushable spawn location
    char pkt[MAX_PUSH_SNAP_DATA];      // full relay line for MPKT_PUSH_STATE
    int  pkt_len;
    int  room_idx;
    int  used;
    int  owner_player_id;             // which player is currently pushing (0 = none)
} PushSnapshot;

// ---------------------------------------------------------------------------
// Enemy snapshot (per room, per owner player)
// ---------------------------------------------------------------------------

typedef struct {
    char     name[MAX_ENEMY_NAME];        // e.g. "OLEnemyPawn_3"
    int      owner_player_id;             // which player owns this enemy
    int      room_idx;
    int      used;
    char     spawn_pkt[MAX_ENEMY_SNAP_DATA]; // full relay line for ENPC_SPAWN
    int      spawn_len;
    char     smt_pkt[MAX_ENEMY_SNAP_DATA];   // latest ENPC_SMT relay line (optional)
    int      smt_len;
    char     loc_pkt[MAX_ENEMY_SNAP_DATA];   // latest ENPC_LOC relay line
    int      loc_len;
} EnemySnapshot;

// ---------------------------------------------------------------------------
// Pickup snapshot (per room) — records that a pickup was collected/hidden
// ---------------------------------------------------------------------------

typedef struct {
    char key[MAX_PICKUP_KEY];           // "X,Y,Z" for PICKUP_STATE, PathName for PICKUP_KISMET
    char pkt[MAX_PICKUP_SNAP_DATA];     // full relay line to replay to newcomers
    int  pkt_len;
    int  room_idx;
    int  used;
} PickupSnapshot;

// ---------------------------------------------------------------------------
// Door snapshot (per room)
// ---------------------------------------------------------------------------

// Stores the authoritative state for a single door, keyed by "X,Y,Z".
// authority_player_id == 0  =>  no one is currently interacting.
typedef struct {
    char key[MAX_DOOR_KEY];           // "X,Y,Z" of door location
    char lock_pkt[MAX_DOOR_SNAP_DATA]; // latest full relay line for DOOR_LOCK (with player_id prefix)
    int  lock_len;
    char state_pkt[MAX_DOOR_SNAP_DATA]; // latest full relay line for DOOR_STATE
    int  state_len;
    char angle_pkt[MAX_DOOR_SNAP_DATA]; // latest full relay line for DOOR_ANGLE
    int  angle_len;
    int  authority_player_id;         // player holding the door; 0 = nobody
    int  room_idx;
    int  used;
} DoorSnapshot;

// ---------------------------------------------------------------------------
// Player
// ---------------------------------------------------------------------------

// One entry in the per-player snapshot drip queue.
typedef struct {
    uint8_t data[SNAP_PKT_MAX];
    int     len;
} SnapEntry;

typedef struct {
    int      used;
    int      id;            // assigned player_id
    int      room_idx;      // index into rooms[]
    Addr     addr;
    char     ip[MAX_IP];
    char     nick[MAX_NICK];
    double   last_seen;     // mono_now()
    int      pkt_count;
    double   pkt_window;    // start of current rate window
    // Last binary STATE snapshot (type byte 0x01), raw relay packet (with player_id)
    uint8_t  state_snap[256];
    int      state_snap_len; // 0 = no snapshot yet
    // Last MATINEE_STATE snapshot (type byte 0x27) — active matinees with position/playrate.
    uint8_t  matinee_snap[1280];
    int      matinee_snap_len; // 0 = no snapshot yet
    // Drip-feed queue: pending snapshots to send on join (to avoid burst flood)
    SnapEntry snap_queue[SNAP_QUEUE_MAX];
    int       snap_head;   // next slot to write
    int       snap_tail;   // next slot to read
    // Session token — 32 random bytes generated on alloc, sent in SRV_READY.
    // Client echoes as 64-char hex in HELLO for reliable NAT rebind identification.
    uint8_t  session_token[32];
} Player;

// ---------------------------------------------------------------------------
// Server state
// ---------------------------------------------------------------------------

#define MAX_LOG_LINES   2000

typedef struct {
    sock_t         sock;
    char           bind_ip[MAX_IP];
    uint16_t       port;
    DB             db;
    Room           rooms[MAX_ROOMS];
    int            n_rooms;
    Player         players[MAX_CLIENTS];
    int            n_players;     // used slots (may have holes)
    DoorSnapshot    doors[MAX_DOOR_SNAPSHOTS];
    PushSnapshot    pushables[MAX_PUSH_SNAPSHOTS];
    EnemySnapshot   enemies[MAX_ENEMY_SNAPSHOTS];
    PickupSnapshot  pickups[MAX_PICKUP_SNAPSHOTS];
    double         next_heartbeat;
    double         next_timeout;
    char           name[64];
    int            player_id_seq; // monotonic counter for next_player_id
    GuiEventRing   gui_ring;
    HistoryRing    history;
    // Set to 1 from outside thread to stop server_run() gracefully.
    volatile int   shutdown_requested;
} Server;

// ---------------------------------------------------------------------------
// Atomic helpers — portable MSVC VS2012 / GCC
// ---------------------------------------------------------------------------

#if defined(_MSC_VER)
#  include <intrin.h>
#  define relay_load_acq(p)      (*(volatile unsigned *)(p))
#  define relay_load_rlx(p)      (*(volatile unsigned *)(p))
#  define relay_store_rel(p, v)  do { _WriteBarrier(); *(volatile unsigned *)(p) = (v); } while(0)
#  define relay_fence_acq()      _ReadBarrier()
#else
#  define relay_load_acq(p)      __atomic_load_n((p), __ATOMIC_ACQUIRE)
#  define relay_load_rlx(p)      __atomic_load_n((p), __ATOMIC_RELAXED)
#  define relay_store_rel(p, v)  __atomic_store_n((p), (v), __ATOMIC_RELEASE)
#  define relay_fence_acq()      __atomic_thread_fence(__ATOMIC_ACQUIRE)
#endif

#include <stdio.h>
#include <stdarg.h>

// Pop a GUI packet event (called from GUI/ImGui thread).
static
#if defined(_MSC_VER)
__inline
#else
inline
#endif
int gui_pop_event(Server *s, GuiPacketEvent *out) {
    unsigned t = relay_load_rlx(&s->gui_ring.tail);
    unsigned h = relay_load_acq(&s->gui_ring.head);
    if (t == h) return 0;
    *out = s->gui_ring.buf[t & (GUI_RING_SIZE - 1)];
    relay_fence_acq();
    relay_store_rel(&s->gui_ring.tail, t + 1);
    return 1;
}

// Push a GUI packet event (called from server thread; drops on overflow).
static
#if defined(_MSC_VER)
__inline
#else
inline
#endif
void gui_push_event(Server *s, int sender_slot, GuiPktCategory cat) {
    unsigned h = relay_load_rlx(&s->gui_ring.head);
    unsigned t = relay_load_acq(&s->gui_ring.tail);
    if ((h - t) >= GUI_RING_SIZE) return;
    s->gui_ring.buf[h & (GUI_RING_SIZE - 1)].sender_slot = sender_slot;
    s->gui_ring.buf[h & (GUI_RING_SIZE - 1)].category    = cat;
    relay_store_rel(&s->gui_ring.head, h + 1);
}

// Push a log entry to history ring (called from server thread).
static
#if defined(_MSC_VER)
__inline
#else
inline
#endif
void history_push(Server *s, const char *fmt, ...) {
    unsigned h = relay_load_rlx(&s->history.head);
    HistoryEntry *e = &s->history.buf[h & (HISTORY_RING_SIZE - 1)];
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(e->msg, HISTORY_MSG_LEN, fmt, ap);
    va_end(ap);
    relay_store_rel(&s->history.head, h + 1);
}

// Pop a log entry (called from ImGui/GUI thread).
static
#if defined(_MSC_VER)
__inline
#else
inline
#endif
int history_pop(Server *s, char out[HISTORY_MSG_LEN]) {
    unsigned t = relay_load_rlx(&s->history.tail);
    unsigned h = relay_load_acq(&s->history.head);
    if (t == h) return 0;
    memcpy(out, s->history.buf[t & (HISTORY_RING_SIZE - 1)].msg, HISTORY_MSG_LEN);
    relay_fence_acq();
    relay_store_rel(&s->history.tail, t + 1);
    return 1;
}

// ---------------------------------------------------------------------------
// Forward declarations
// ---------------------------------------------------------------------------

void server_init(Server *s, uint16_t port, const char *name, const char *bind_ip, const char *db_path);
void server_run(Server *s);
void server_shutdown(Server *s);

Room   *server_find_room(Server *s, const char *code);
Room   *server_add_room(Server *s, const char *code, const char *password);
void    server_set_room_password(Server *s, Room *r, const char *password);
void    server_clear_room_snapshots(Server *s, int room_idx);
Player *server_find_player_by_addr(Server *s, const Addr *addr);
Player *server_find_player_by_id(Server *s, int id);
Player *server_find_player_by_token(Server *s, int room_idx, const uint8_t token[32]);
int     server_room_player_count(Server *s, int room_idx);
void    server_disconnect(Server *s, Player *p);

void    server_send(Server *s, const Addr *addr, const char *msg, int len);
void    server_relay(Server *s, Player *sender, const char *payload, int len);
void    server_broadcast_room(Server *s, int room_idx, const char *msg, int len);
void    server_snap_enqueue(Player *p, const char *data, int len);
void    server_drain_snaps(Server *s);  // called each tick

int     check_rate(Player *p);

DoorSnapshot  *door_find(Server *s, int room_idx, const char *key);
DoorSnapshot  *door_alloc(Server *s, int room_idx, const char *key);
void           door_release_authority(Server *s, int room_idx, int player_id);

EnemySnapshot  *enemy_find(Server *s, int room_idx, const char *name);
EnemySnapshot  *enemy_alloc(Server *s, int room_idx, const char *name, int owner_player_id);
void            enemy_remove(Server *s, int room_idx, const char *name);
void            enemy_remove_all_for_player(Server *s, int room_idx, int player_id);

PickupSnapshot *pickup_find(Server *s, int room_idx, const char *key);
PickupSnapshot *pickup_alloc(Server *s, int room_idx, const char *key);

PushSnapshot   *push_find(Server *s, int room_idx, const char *key);
PushSnapshot   *push_alloc(Server *s, int room_idx, const char *key);

void           send_snapshots_to_newcomer(Server *s, Player *newcomer);

// Exposed for main.c
void handle_packet(Server *s, const char *data, int len, const Addr *addr);
void run_heartbeat(Server *s);
void run_timeout(Server *s);

#ifdef _MSC_VER
#  pragma pack(pop)
#endif
