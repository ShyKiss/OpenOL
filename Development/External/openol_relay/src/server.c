/*=============================================================================
    server.c - core server logic, shared by the embedded relay (OLGame.exe,
    OPENOL_EMBEDDED) and the standalone OpenOL_Relay.
=============================================================================*/
#ifdef _MSC_VER
#  pragma warning(disable: 4127) /* conditional expression is constant */
#endif
#include "server.h"
#include "relay_compat.h"  /* snprintf polyfill — after server.h pulls in winsock2 */
#include "openol_protocol.h"
#ifndef OPENOL_EMBEDDED
#  include "cli.h"
#endif
#include <stdarg.h>

// ---------------------------------------------------------------------------
// Session token generation
// ---------------------------------------------------------------------------

// Fills out with bytes from the OS CSPRNG. Returns 0 on failure.
static int os_random(uint8_t *out, unsigned len) {
#ifdef _WIN32
    typedef BOOLEAN (APIENTRY *RtlGenRandomFn)(PVOID, ULONG);
    RtlGenRandomFn gen = NULL;
    HMODULE lib = LoadLibraryA("advapi32.dll");
    if (lib) gen = (RtlGenRandomFn)GetProcAddress(lib, "SystemFunction036");
    return gen && gen(out, len);
#else
    size_t got = 0;
    FILE *f = fopen("/dev/urandom", "rb");
    if (f) { got = fread(out, 1, len, f); fclose(f); }
    return got == len;
#endif
}

static void gen_session_token(uint8_t out[32]) {
    if (!os_random(out, 32)) {
        // Last resort only: a predictable token is still better than no session.
        int i;
        srand((unsigned int)time(NULL) ^ (unsigned int)(size_t)out);
        for (i = 0; i < 32; i++) out[i] = (uint8_t)(rand() & 0xFF);
    }
}

// ---------------------------------------------------------------------------
// Logging — history ring in the embedded build (consumed by the ImGui panel),
// cli_log in the standalone build (handles both TUI and plain-text modes)
// ---------------------------------------------------------------------------

static void server_log(Server *s, const char *fmt, ...) {
    time_t t = time(NULL);
    struct tm *tm_info = localtime(&t);
    char ts[20];
    char body[256];
    va_list ap;
    strftime(ts, sizeof(ts), "%H:%M:%S", tm_info);
    va_start(ap, fmt);
    vsnprintf(body, sizeof(body), fmt, ap);
    va_end(ap);
#ifdef OPENOL_EMBEDDED
    history_push(s, "[%s] %s", ts, body);
#else
    (void)s;
    cli_log("[%s] %s", ts, body);
#endif
}

// ---------------------------------------------------------------------------
// Rate limiting
// ---------------------------------------------------------------------------

int check_rate(Player *p) {
    double now = mono_now();
    if (now - p->pkt_window >= 1.0) {
        p->pkt_window = now;
        p->pkt_count  = 1;
        return 1;
    }
    p->pkt_count++;
    return p->pkt_count <= MAX_RATE;
}

// ---------------------------------------------------------------------------
// Room management
// ---------------------------------------------------------------------------

Room *server_find_room(Server *s, const char *code) {
    for (int i = 0; i < s->n_rooms; i++)
        if (s->rooms[i].active && strcmp(s->rooms[i].code, code) == 0)
            return &s->rooms[i];
    return NULL;
}

Room *server_add_room(Server *s, const char *code, const char *password) {
    if (s->n_rooms >= MAX_ROOMS) return NULL;
    Room *r = &s->rooms[s->n_rooms++];
    memset(r, 0, sizeof(*r));
    strncpy(r->code,     code,     MAX_ROOM_CODE - 1);
    strncpy(r->password, password, MAX_PASSWORD  - 1);
    r->active = 1;
    // Mirror into DB so trusted/bans are always accessible
    db_add_room(&s->db, code, password);
    return r;
}

// Change room password. Clears trusted IPs in DB.
void server_set_room_password(Server *s, Room *r, const char *password) {
    strncpy(r->password, password ? password : "", MAX_PASSWORD - 1);
    r->password[MAX_PASSWORD - 1] = '\0';
    DBRoom *dr = db_find_room(&s->db, r->code);
    if (dr) {
        strncpy(dr->password, r->password, DB_PASSWORD_LEN - 1);
        dr->password[DB_PASSWORD_LEN - 1] = '\0';
        db_clear_trusted(dr);
    }
}

// Clear all snapshots (doors, enemies, pickups, pushables) belonging to a room.
// Called when the last player in a room disconnects.
void server_clear_room_snapshots(Server *s, int room_idx) {
    for (int i = 0; i < MAX_DOOR_SNAPSHOTS; i++)
        if (s->doors[i].used && s->doors[i].room_idx == room_idx)
            memset(&s->doors[i], 0, sizeof(s->doors[i]));
    for (int i = 0; i < MAX_PUSH_SNAPSHOTS; i++)
        if (s->pushables[i].used && s->pushables[i].room_idx == room_idx)
            memset(&s->pushables[i], 0, sizeof(s->pushables[i]));
    for (int i = 0; i < MAX_ENEMY_SNAPSHOTS; i++)
        if (s->enemies[i].used && s->enemies[i].room_idx == room_idx)
            memset(&s->enemies[i], 0, sizeof(s->enemies[i]));
    for (int i = 0; i < MAX_PICKUP_SNAPSHOTS; i++)
        if (s->pickups[i].used && s->pickups[i].room_idx == room_idx)
            memset(&s->pickups[i], 0, sizeof(s->pickups[i]));
}

int server_room_player_count(Server *s, int room_idx) {
    int count = 0;
    for (int i = 0; i < MAX_CLIENTS; i++)
        if (s->players[i].used && s->players[i].room_idx == room_idx)
            count++;
    return count;
}

// ---------------------------------------------------------------------------
// Player management
// ---------------------------------------------------------------------------

static int next_player_id(Server *s) {
    // Use Server-owned counter instead of static local — safe if ever
    // called from multiple threads, and survives repeated server_init calls.
    for (;;) {
        if (s->player_id_seq <= 0) s->player_id_seq = 1000;
        int id = s->player_id_seq++;
        if (!server_find_player_by_id(s, id))
            return id;
    }
}

Player *server_find_player_by_addr(Server *s, const Addr *addr) {
    for (int i = 0; i < MAX_CLIENTS; i++)
        if (s->players[i].used && addr_eq(&s->players[i].addr, addr))
            return &s->players[i];
    return NULL;
}

Player *server_find_player_by_id(Server *s, int id) {
    for (int i = 0; i < MAX_CLIENTS; i++)
        if (s->players[i].used && s->players[i].id == id)
            return &s->players[i];
    return NULL;
}

Player *server_find_player_by_token(Server *s, int room_idx, const uint8_t token[32]) {
    for (int i = 0; i < MAX_CLIENTS; i++) {
        Player *p = &s->players[i];
        if (p->used && p->room_idx == room_idx && memcmp(p->session_token, token, 32) == 0)
            return p;
    }
    return NULL;
}

static Player *alloc_player(Server *s) {
    for (int i = 0; i < MAX_CLIENTS; i++)
        if (!s->players[i].used)
            return &s->players[i];
    return NULL;
}

static int total_clients(Server *s) {
    int n = 0;
    for (int i = 0; i < MAX_CLIENTS; i++)
        if (s->players[i].used) n++;
    return n;
}

// ---------------------------------------------------------------------------
// Send / relay
// ---------------------------------------------------------------------------

void server_send(Server *s, const Addr *addr, const char *msg, int len) {
    sendto(s->sock, msg, len, 0,
           (const struct sockaddr *)&addr->sa, sizeof(addr->sa));
}

void server_relay(Server *s, Player *sender, const char *payload, int len) {
    int room_idx = sender->room_idx;
    for (int i = 0; i < MAX_CLIENTS; i++) {
        Player *p = &s->players[i];
        if (p->used && p != sender && p->room_idx == room_idx)
            server_send(s, &p->addr, payload, len);
    }
}

void server_broadcast_room(Server *s, int room_idx, const char *msg, int len) {
    for (int i = 0; i < MAX_CLIENTS; i++) {
        Player *p = &s->players[i];
        if (p->used && p->room_idx == room_idx)
            server_send(s, &p->addr, msg, len);
    }
}

/*
 * Server→client binary format: [type(1)][pad LE4=0][payload...]
 * Matches the client binary framing: [type(1)][sender_id LE4][payload...].
 * SRV packets use sender_id=0 (pad) since they originate from the server.
 */
#define SRV_HDR_INIT(buf, type) \
    do { (buf)[0]=(type); (buf)[1]=0; (buf)[2]=0; (buf)[3]=0; (buf)[4]=0; } while(0)

/* Send SRV_READY: [type(1)][pad(4)][player_id LE4][name_len(1)][name...][token(32)]
 * token — 32-byte session token for NAT rebind identification (echoed in HELLO). */
static void send_ready(Server *s, const Addr *addr, int player_id, const uint8_t token[32]) {
    unsigned char buf[298]; /* 5 hdr + 4 id + 1 namelen + 255 name + 32 token */
    SRV_HDR_INIT(buf, SRV_READY);
    int name_len = (int)strlen(s->name);
    if (name_len > 255) name_len = 255;
    buf[5] = (unsigned char)(player_id & 0xFF);
    buf[6] = (unsigned char)((player_id >> 8)  & 0xFF);
    buf[7] = (unsigned char)((player_id >> 16) & 0xFF);
    buf[8] = (unsigned char)((player_id >> 24) & 0xFF);
    buf[9] = (unsigned char)name_len;
    memcpy(buf + 10, s->name, name_len);
    memcpy(buf + 10 + name_len, token, 32);
    server_send(s, addr, (const char *)buf, 10 + name_len + 32);
}

/* Send SRV_ONLINE_COUNT: [0x28][0x00000000][count LE4] */
static void send_online_count(Server *s, const Addr *addr, int count) {
    unsigned char buf[9];
    SRV_HDR_INIT(buf, SRV_ONLINE_COUNT);
    buf[5] = (unsigned char)(count & 0xFF);
    buf[6] = (unsigned char)((count >> 8)  & 0xFF);
    buf[7] = (unsigned char)((count >> 16) & 0xFF);
    buf[8] = (unsigned char)((count >> 24) & 0xFF);
    server_send(s, addr, (const char *)buf, 9);
}

/* Broadcast SRV_ONLINE_COUNT to all players in a room */
static void broadcast_online_count(Server *s, int room_idx, int count) {
    for (int i = 0; i < MAX_CLIENTS; i++) {
        Player *p = &s->players[i];
        if (p->used && p->room_idx == room_idx)
            send_online_count(s, &p->addr, count);
    }
}

/* Send SRV_HELLO_FAIL: [0x29][0x00000000][reason_len(1)][reason ASCII] */
static void send_hello_fail(Server *s, const Addr *addr, const char *reason) {
    unsigned char buf[262];
    SRV_HDR_INIT(buf, SRV_HELLO_FAIL);
    int rlen = (int)strlen(reason);
    if (rlen > 255) rlen = 255;
    buf[5] = (unsigned char)rlen;
    memcpy(buf + 6, reason, rlen);
    server_send(s, addr, (const char *)buf, 6 + rlen);
}

/* Send SRV_DISCONNECT: [0x2A][0x00000000][player_id LE4] */
static void send_disconnect_notify(Server *s, int room_idx, int player_id) {
    unsigned char buf[9];
    SRV_HDR_INIT(buf, SRV_DISCONNECT);
    buf[5] = (unsigned char)(player_id & 0xFF);
    buf[6] = (unsigned char)((player_id >> 8)  & 0xFF);
    buf[7] = (unsigned char)((player_id >> 16) & 0xFF);
    buf[8] = (unsigned char)((player_id >> 24) & 0xFF);
    server_broadcast_room(s, room_idx, (const char *)buf, 9);
}

/* Send PUSH_DENIED: [CH_PUSH][PUSH_DENIED][0 LE4][KeyX(4)][KeyY(4)][KeyZ(4)] */
static void send_push_denied(Server *s, const Addr *addr, const char *key_xyz) {
    unsigned char buf[OL_HEADER_SIZE + 12];
    memset(buf, 0, sizeof(buf));
    buf[0] = CH_PUSH;
    buf[1] = PUSH_DENIED;
    memcpy(buf + OL_HEADER_SIZE, key_xyz, 12);
    server_send(s, addr, (const char *)buf, sizeof(buf));
}

/* Send SRV_DOOR_DENY: [0x2B][0x00000000][key_len(1)][key ASCII] */
static void send_door_deny(Server *s, const Addr *addr, const char *key) {
    unsigned char buf[262];
    SRV_HDR_INIT(buf, SRV_DOOR_DENY);
    int klen = (int)strlen(key);
    if (klen > 255) klen = 255;
    buf[5] = (unsigned char)klen;
    memcpy(buf + 6, key, klen);
    server_send(s, addr, (const char *)buf, 6 + klen);
}

// ---------------------------------------------------------------------------
// Door snapshots
// ---------------------------------------------------------------------------

DoorSnapshot *door_find(Server *s, int room_idx, const char *key) {
    for (int i = 0; i < MAX_DOOR_SNAPSHOTS; i++) {
        DoorSnapshot *d = &s->doors[i];
        if (d->used && d->room_idx == room_idx && strcmp(d->key, key) == 0)
            return d;
    }
    return NULL;
}

DoorSnapshot *door_alloc(Server *s, int room_idx, const char *key) {
    DoorSnapshot *d = door_find(s, room_idx, key);
    if (d) return d;
    for (int i = 0; i < MAX_DOOR_SNAPSHOTS; i++) {
        if (!s->doors[i].used) {
            memset(&s->doors[i], 0, sizeof(s->doors[i]));
            s->doors[i].used = 1;
            s->doors[i].room_idx = room_idx;
            strncpy(s->doors[i].key, key, MAX_DOOR_KEY - 1);
            return &s->doors[i];
        }
    }
    return NULL;
}

// Release door authority held by a specific player in a room.
void door_release_authority(Server *s, int room_idx, int player_id) {
    for (int i = 0; i < MAX_DOOR_SNAPSHOTS; i++) {
        DoorSnapshot *d = &s->doors[i];
        if (d->used && d->room_idx == room_idx && d->authority_player_id == player_id)
            d->authority_player_id = 0;
    }
}

// ---------------------------------------------------------------------------
// Enemy snapshots
// ---------------------------------------------------------------------------

EnemySnapshot *enemy_find(Server *s, int room_idx, const char *name) {
    for (int i = 0; i < MAX_ENEMY_SNAPSHOTS; i++) {
        EnemySnapshot *e = &s->enemies[i];
        if (e->used && e->room_idx == room_idx && strcmp(e->name, name) == 0)
            return e;
    }
    return NULL;
}

EnemySnapshot *enemy_alloc(Server *s, int room_idx, const char *name, int owner_player_id) {
    EnemySnapshot *e = enemy_find(s, room_idx, name);
    if (e) {
        e->owner_player_id = owner_player_id;
        return e;
    }
    for (int i = 0; i < MAX_ENEMY_SNAPSHOTS; i++) {
        if (!s->enemies[i].used) {
            memset(&s->enemies[i], 0, sizeof(s->enemies[i]));
            s->enemies[i].used = 1;
            s->enemies[i].room_idx = room_idx;
            s->enemies[i].owner_player_id = owner_player_id;
            strncpy(s->enemies[i].name, name, MAX_ENEMY_NAME - 1);
            return &s->enemies[i];
        }
    }
    return NULL;
}

void enemy_remove(Server *s, int room_idx, const char *name) {
    EnemySnapshot *e = enemy_find(s, room_idx, name);
    if (e) memset(e, 0, sizeof(*e));
}

void enemy_remove_all_for_player(Server *s, int room_idx, int player_id) {
    for (int i = 0; i < MAX_ENEMY_SNAPSHOTS; i++) {
        EnemySnapshot *e = &s->enemies[i];
        if (e->used && e->room_idx == room_idx && e->owner_player_id == player_id)
            memset(e, 0, sizeof(*e));
    }
}

// ---------------------------------------------------------------------------
// Pickup snapshots
// ---------------------------------------------------------------------------

PickupSnapshot *pickup_find(Server *s, int room_idx, const char *key) {
    for (int i = 0; i < MAX_PICKUP_SNAPSHOTS; i++) {
        PickupSnapshot *p = &s->pickups[i];
        if (p->used && p->room_idx == room_idx && strcmp(p->key, key) == 0)
            return p;
    }
    return NULL;
}

PickupSnapshot *pickup_alloc(Server *s, int room_idx, const char *key) {
    PickupSnapshot *p = pickup_find(s, room_idx, key);
    if (p) return p;
    for (int i = 0; i < MAX_PICKUP_SNAPSHOTS; i++) {
        if (!s->pickups[i].used) {
            memset(&s->pickups[i], 0, sizeof(s->pickups[i]));
            s->pickups[i].used = 1;
            s->pickups[i].room_idx = room_idx;
            strncpy(s->pickups[i].key, key, MAX_PICKUP_KEY - 1);
            return &s->pickups[i];
        }
    }
    return NULL;
}

PushSnapshot *push_find(Server *s, int room_idx, const char *key) {
    for (int i = 0; i < MAX_PUSH_SNAPSHOTS; i++) {
        PushSnapshot *p = &s->pushables[i];
        if (p->used && p->room_idx == room_idx && strcmp(p->key, key) == 0)
            return p;
    }
    return NULL;
}

PushSnapshot *push_alloc(Server *s, int room_idx, const char *key) {
    PushSnapshot *p = push_find(s, room_idx, key);
    if (p) return p;
    for (int i = 0; i < MAX_PUSH_SNAPSHOTS; i++) {
        if (!s->pushables[i].used) {
            memset(&s->pushables[i], 0, sizeof(s->pushables[i]));
            s->pushables[i].used = 1;
            s->pushables[i].room_idx = room_idx;
            strncpy(s->pushables[i].key, key, MAX_PUSH_KEY - 1);
            return &s->pushables[i];
        }
    }
    return NULL;
}

void push_release_owner(Server *s, int room_idx, int player_id) {
    for (int i = 0; i < MAX_PUSH_SNAPSHOTS; i++) {
        PushSnapshot *ps = &s->pushables[i];
        if (ps->used && ps->room_idx == room_idx && ps->owner_player_id == player_id)
            ps->owner_player_id = 0;
    }
}

// ---------------------------------------------------------------------------
// Snapshot drip-feed queue
// ---------------------------------------------------------------------------

// Enqueue one snapshot packet for drip-feed delivery to a player.
// Silently drops if the queue is full (caller already logged a warning at join).
void server_snap_enqueue(Player *p, const char *data, int len) {
    if (len <= 0 || len > SNAP_PKT_MAX) return;
    int next = (p->snap_head + 1) % SNAP_QUEUE_MAX;
    if (next == p->snap_tail) return; // queue full — drop
    memcpy(p->snap_queue[p->snap_head].data, data, len);
    p->snap_queue[p->snap_head].len = len;
    p->snap_head = next;
}

static int server_has_queued_snaps(const Server *s) {
    int i;
    for (i = 0; i < MAX_CLIENTS; i++) {
        const Player *p = &s->players[i];
        if (p->used && p->snap_head != p->snap_tail) return 1;
    }
    return 0;
}

// Send up to SNAP_DRIP_PER_TICK queued snapshots per player every SNAP_DRIP_INTERVAL.
// Called from server_run each iteration; time-gated because an iteration runs for
// every incoming packet, which would otherwise tie the drip rate to server load.
void server_drain_snaps(Server *s) {
    double now = mono_now();
    if (now < s->next_snap_drain) return;
    s->next_snap_drain = now + SNAP_DRIP_INTERVAL;
    for (int i = 0; i < MAX_CLIENTS; i++) {
        Player *p = &s->players[i];
        if (!p->used || p->snap_head == p->snap_tail) continue;
        int sent = 0;
        while (p->snap_head != p->snap_tail && sent < SNAP_DRIP_PER_TICK) {
            SnapEntry *e = &p->snap_queue[p->snap_tail];
            server_send(s, &p->addr, (const char *)e->data, e->len);
            p->snap_tail = (p->snap_tail + 1) % SNAP_QUEUE_MAX;
            sent++;
        }
    }
}

// Queue all player STATE snapshots and world snapshots for drip-feed delivery to a newcomer.
void send_snapshots_to_newcomer(Server *s, Player *newcomer) {
    // Reset drip queue for this player (may have stale data from previous join)
    newcomer->snap_head = 0;
    newcomer->snap_tail = 0;

    // Player state snapshots (binary STATE packets from other players)
    for (int i = 0; i < MAX_CLIENTS; i++) {
        Player *p = &s->players[i];
        if (!p->used || p == newcomer || p->room_idx != newcomer->room_idx)
            continue;
        if (p->state_snap_len > 0)
            server_snap_enqueue(newcomer, (const char *)p->state_snap, p->state_snap_len);
        if (p->matinee_snap_len > 0)
            server_snap_enqueue(newcomer, (const char *)p->matinee_snap, p->matinee_snap_len);
    }
    // Door snapshots: DOOR_LOCK (if held), DOOR_STATE, DOOR_ANGLE
    for (int i = 0; i < MAX_DOOR_SNAPSHOTS; i++) {
        DoorSnapshot *d = &s->doors[i];
        if (!d->used || d->room_idx != newcomer->room_idx) continue;
        if (d->authority_player_id != 0 && d->lock_len > 0) {
            Player *auth = server_find_player_by_id(s, d->authority_player_id);
            if (auth && auth->used)
                server_snap_enqueue(newcomer, d->lock_pkt, d->lock_len);
        }
        if (d->state_len > 0) server_snap_enqueue(newcomer, d->state_pkt, d->state_len);
        if (d->angle_len > 0) server_snap_enqueue(newcomer, d->angle_pkt, d->angle_len);
    }
    // Enemy snapshots: skip enemies owned by newcomer
    for (int i = 0; i < MAX_ENEMY_SNAPSHOTS; i++) {
        EnemySnapshot *e = &s->enemies[i];
        if (!e->used || e->room_idx != newcomer->room_idx) continue;
        if (e->owner_player_id == newcomer->id) continue;
        if (e->spawn_len > 0) server_snap_enqueue(newcomer, e->spawn_pkt, e->spawn_len);
        if (e->smt_len   > 0) server_snap_enqueue(newcomer, e->smt_pkt,   e->smt_len);
        if (e->loc_len   > 0) server_snap_enqueue(newcomer, e->loc_pkt,   e->loc_len);
    }
    // Pushable snapshots — always send with bPushing=0 (offset 26) so newcomers
    // don't start a looping sound for a push that is no longer active.
    {
        int i;
        for (i = 0; i < MAX_PUSH_SNAPSHOTS; i++) {
            PushSnapshot *ps = &s->pushables[i];
            if (!ps->used || ps->room_idx != newcomer->room_idx) continue;
            if (ps->pkt_len > 0) {
                char tmp[MAX_PUSH_SNAP_DATA];
                memcpy(tmp, ps->pkt, ps->pkt_len);
                if (ps->pkt_len > 26) tmp[26] = 0; /* clear bPushing */
                server_snap_enqueue(newcomer, tmp, ps->pkt_len);
            }
        }
    }
    // Pickup snapshots
    for (int i = 0; i < MAX_PICKUP_SNAPSHOTS; i++) {
        PickupSnapshot *p = &s->pickups[i];
        if (!p->used || p->room_idx != newcomer->room_idx) continue;
        if (p->pkt_len > 0) server_snap_enqueue(newcomer, p->pkt, p->pkt_len);
    }
}

// ---------------------------------------------------------------------------
// Disconnect
// ---------------------------------------------------------------------------

void server_disconnect(Server *s, Player *p) {
    int  room_idx = p->room_idx;
    int  pid      = p->id;
    char ip[MAX_IP];
    char nick[MAX_NICK];
    strncpy(ip,   p->ip,   sizeof(ip)   - 1); ip[sizeof(ip)-1]     = '\0';
    strncpy(nick, p->nick, sizeof(nick) - 1); nick[sizeof(nick)-1] = '\0';

    // Release any door authority this player held
    door_release_authority(s, room_idx, pid);
    // Remove all enemy snapshots owned by this player
    enemy_remove_all_for_player(s, room_idx, pid);
    // Release any pushable ownership this player held
    push_release_owner(s, room_idx, pid);

    memset(p, 0, sizeof(*p));   // mark as free

    // Notify remaining players
    send_disconnect_notify(s, room_idx, pid);

    // Updated online count
    int count = server_room_player_count(s, room_idx);
    broadcast_online_count(s, room_idx, count);

    // Clear all snapshots when the room becomes empty
    if (count == 0)
        server_clear_room_snapshots(s, room_idx);

    server_log(s, "[%s] ID=%d ('%s') disconnected (%d left in room)", ip, pid, nick, count);
    history_push(s, "[%s] disconnected", nick);
}

// ---------------------------------------------------------------------------
// HELLO rate limiter — prevents db_save DoS from unauthenticated senders.
// Simple fixed-window per-IP: max 5 HELLO attempts per 10-second window.
// ---------------------------------------------------------------------------

#define HELLO_RL_MAX      5
#define HELLO_RL_WINDOW   10.0
#define HELLO_RL_BUCKETS  64

typedef struct {
    char   ip[MAX_IP];
    int    count;
    double window_start;
} HelloBucket;

static HelloBucket hello_rl[HELLO_RL_BUCKETS];

// Returns 1 if allowed, 0 if rate-limited.
static int hello_check_rate(const char *ip) {
    double now = mono_now();
    // Hash IP to bucket (djb2-style)
    unsigned h = 5381;
    for (const char *p = ip; *p; p++) h = ((h << 5) + h) ^ (unsigned char)*p;
    int slot = (int)(h % HELLO_RL_BUCKETS);

    HelloBucket *b = &hello_rl[slot];
    // Reset window if IP changed or window expired
    if (b->ip[0] == '\0' || strcmp(b->ip, ip) != 0 ||
        now - b->window_start >= HELLO_RL_WINDOW) {
        strncpy(b->ip, ip, MAX_IP - 1);
        b->ip[MAX_IP - 1] = '\0';
        b->count = 1;
        b->window_start = now;
        return 1;
    }
    b->count++;
    return b->count <= HELLO_RL_MAX;
}

// ---------------------------------------------------------------------------
// HELLO handshake
// ---------------------------------------------------------------------------

static void handle_hello(Server *s, const char *line, const Addr *addr) {
    // HELLO,<room>[,<password>]
    char code[MAX_ROOM_CODE]  = {0};
    char password[MAX_PASSWORD] = {0};

    const char *p = line + 6; // skip "HELLO,"
    const char *comma = strchr(p, ',');
    if (comma) {
        int clen = (int)(comma - p);
        if (clen >= MAX_ROOM_CODE) clen = MAX_ROOM_CODE - 1;
        memcpy(code, p, clen);
        strncpy(password, comma + 1, MAX_PASSWORD - 1);
        // Strip trailing newline
        char *nl = strchr(password, '\n');
        if (nl) *nl = '\0';
        nl = strchr(password, '\r');
        if (nl) *nl = '\0';
    } else {
        int clen = (int)strlen(p);
        // Strip trailing newline
        while (clen > 0 && (p[clen-1] == '\n' || p[clen-1] == '\r')) clen--;
        if (clen >= MAX_ROOM_CODE) clen = MAX_ROOM_CODE - 1;
        memcpy(code, p, clen);
    }

    if (!code[0]) {
        send_hello_fail(s, addr, "invalid room");
        return;
    }

    char client_ip[MAX_IP];
    addr_ip_str(addr, client_ip, sizeof(client_ip));

    if (!hello_check_rate(client_ip)) {
        // Silently drop — don't send a response (prevents amplification)
        return;
    }

    if (db_find_ban(&s->db, client_ip)) {
        send_hello_fail(s, addr, "banned");
        return;
    }

    Room *room = server_find_room(s, code);
    if (!room) {
        send_hello_fail(s, addr, "unknown room");
        return;
    }

    {
        DBRoom *dr_check = db_find_room(&s->db, code);
        if (dr_check && db_find_room_ban(dr_check, client_ip)) {
            send_hello_fail(s, addr, "banned from room");
            return;
        }
    }

    if (room->password[0]) {
        DBRoom *dr = db_find_room(&s->db, room->code);
        int trusted = dr ? db_is_trusted(dr, client_ip) : 0;
        if (!trusted && strcmp(room->password, password) != 0) {
            send_hello_fail(s, addr, "wrong password");
            return;
        }
        if (!trusted && dr) {
            db_add_trusted(dr, client_ip);
            db_save(&s->db);
        }
    }

    // Parse optional session token (4th HELLO field): "HELLO,ROOM,PASS,<hex64>"
    // The token is 32 bytes encoded as 64 lowercase hex chars.
    uint8_t hello_token[32];
    int     has_token = 0;
    {
        // Count commas to find the 4th field
        const char *scan = line + 6; // skip "HELLO,"
        int commas = 0;
        while (*scan) {
            if (*scan == ',') { commas++; if (commas == 3) { scan++; break; } }
            scan++;
        }
        if (commas == 3 && strlen(scan) >= 64) {
            has_token = 1;
            for (int i = 0; i < 32; i++) {
                unsigned int byte_val = 0;
                if (sscanf(scan + i * 2, "%02x", &byte_val) == 1)
                    hello_token[i] = (uint8_t)byte_val;
                else { has_token = 0; break; }
            }
        }
    }

    // Re-use existing slot if same addr reconnects (exact match first).
    Player *pl = server_find_player_by_addr(s, addr);
    if (!pl) {
        int room_idx = (int)(room - s->rooms);

        // Token match only — no IP-only fallback.
        // IP fallback is unsafe on loopback (multiple clients share 127.0.0.1).
        if (has_token) {
            Player *candidate = server_find_player_by_token(s, room_idx, hello_token);
            if (candidate) {
                server_log(s, "[%s] ID=%d NAT rebind via token (port %d→%d), reusing slot",
                           client_ip, candidate->id,
                           ntohs(candidate->addr.sa.sin_port), ntohs(addr->sa.sin_port));
                candidate->addr = *addr;
                pl = candidate;
            }
        }
    }
    if (pl && pl->room_idx == (int)(room - s->rooms)) {
        int count = server_room_player_count(s, pl->room_idx);
        send_ready(s, addr, pl->id, pl->session_token);
        send_online_count(s, addr, count);
        // Resend snapshots so reconnecting player gets up-to-date world state
        send_snapshots_to_newcomer(s, pl);
        return;
    }

    if (total_clients(s) >= MAX_CLIENTS) {
        send_hello_fail(s, addr, "server full");
        return;
    }

    pl = alloc_player(s);
    if (!pl) {
        send_hello_fail(s, addr, "server full");
        return;
    }

    memset(pl, 0, sizeof(*pl));
    pl->used      = 1;
    pl->id        = next_player_id(s);
    pl->room_idx  = (int)(room - s->rooms);
    pl->addr      = *addr;
    pl->last_seen = mono_now();
    pl->pkt_window = pl->last_seen;
    addr_ip_str(addr, pl->ip, sizeof(pl->ip));
    snprintf(pl->nick, sizeof(pl->nick), "Player%d", pl->id);
    gen_session_token(pl->session_token);

    int count = server_room_player_count(s, pl->room_idx);
    server_log(s, "[%s:%d] ID=%d joined room '%s' (%d players)",
               pl->ip, ntohs(addr->sa.sin_port), pl->id, code, count);
    history_push(s, "[%s] joined room %s", pl->nick, code);

    // Notify existing players of new count
    for (int i = 0; i < MAX_CLIENTS; i++) {
        Player *other = &s->players[i];
        if (other->used && other != pl && other->room_idx == pl->room_idx)
            send_online_count(s, &other->addr, count);
    }

    send_ready(s, addr, pl->id, pl->session_token);
    send_online_count(s, addr, count);

    // Send existing player and door state snapshots so the newcomer is in sync
    send_snapshots_to_newcomer(s, pl);
}

// ---------------------------------------------------------------------------
// Packet dispatch
// ---------------------------------------------------------------------------

void handle_packet(Server *s, const char *data, int len, const Addr *addr) {
    if (len <= 0) return;

    // Binary packet: [channel(1)][type(1)][player_id LE uint32(4)][payload...]
    // Channel 0x00..0x05 — client game packets.
    // HELLO starts with 'H' (0x48) and is handled separately below.
    if ((unsigned char)data[0] <= 0x05) {
        unsigned char channel, ptype;
        uint32_t player_id;
        Player *pl;
        int sender_slot;
        const char *room;

        if (len < 6) return;
        channel = (unsigned char)data[0];
        ptype   = (unsigned char)data[1];
        player_id =
            (unsigned char)data[2]        |
            ((unsigned char)data[3] << 8) |
            ((unsigned char)data[4] << 16)|
            ((unsigned char)data[5] << 24);
        pl = server_find_player_by_id(s, (int)player_id);
        if (!pl || !addr_eq(&pl->addr, addr)) return;
        pl->last_seen = mono_now();
        if (!check_rate(pl)) return;

        // CH_SRV: HELLO and PING
        if (channel == CH_SRV) {
            if (ptype == SRV_HELLO && len >= 8) {
                unsigned char nick_len = (unsigned char)data[6];
                int max_nick = len - 7;
                char clean[MAX_NICK];
                int ci = 0, i;
                memset(clean, 0, sizeof(clean));
                if (nick_len > max_nick) nick_len = (unsigned char)max_nick;
                if (nick_len > MAX_NICK - 1) nick_len = MAX_NICK - 1;
                for (i = 0; i < nick_len && ci < MAX_NICK - 1; i++) {
                    char c = data[7 + i];
                    if ((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') ||
                        (c >= '0' && c <= '9') || c == '_')
                        clean[ci++] = c;
                }
                if (!clean[0]) snprintf(clean, sizeof(clean), "Player%d", pl->id);
                memcpy(pl->nick, clean, MAX_NICK - 1);
                pl->nick[MAX_NICK - 1] = '\0';
                server_log(s, "[%s] ID=%d identified as '%s'", pl->ip, pl->id, pl->nick);
                history_push(s, "[%s] identified", pl->nick);
            } else if (ptype == SRV_PING) {
                server_send(s, addr, data, len);
            }
            return;
        }

        // CH_HERO: STATE snapshot
        if (channel == CH_HERO && ptype == HERO_STATE) {
            int snap_len = len < (int)sizeof(pl->state_snap) ? len : (int)sizeof(pl->state_snap);
            memcpy(pl->state_snap, data, snap_len);
            pl->state_snap_len = snap_len;
        }

        // CH_WORLD: special types
        if (channel == CH_WORLD) {
            if (ptype == WORLD_MATINEE_STATE) {
                int snap_len = len < (int)sizeof(pl->matinee_snap) ? len : (int)sizeof(pl->matinee_snap);
                memcpy(pl->matinee_snap, data, snap_len);
                pl->matinee_snap_len = snap_len;
            }
            if (ptype == WORLD_DISCONNECT) {
                server_disconnect(s, pl);
                return;
            }
            if (ptype == WORLD_REQUEST_ENEMIES) {
                int i;
                for (i = 0; i < MAX_ENEMY_SNAPSHOTS; i++) {
                    EnemySnapshot *e = &s->enemies[i];
                    if (!e->used || e->room_idx != pl->room_idx) continue;
                    if (e->owner_player_id == pl->id) continue;
                    if (e->spawn_len > 0) server_send(s, &pl->addr, e->spawn_pkt, e->spawn_len);
                    if (e->smt_len > 0)   server_send(s, &pl->addr, e->smt_pkt,   e->smt_len);
                    if (e->loc_len > 0)   server_send(s, &pl->addr, e->loc_pkt,   e->loc_len);
                }
                return;
            }
            if (ptype == WORLD_REQUEST_DOORS) {
                int i;
                for (i = 0; i < MAX_DOOR_SNAPSHOTS; i++) {
                    DoorSnapshot *d = &s->doors[i];
                    if (!d->used || d->room_idx != pl->room_idx) continue;
                    if (d->state_len > 0) server_send(s, &pl->addr, d->state_pkt, d->state_len);
                    if (d->angle_len > 0) server_send(s, &pl->addr, d->angle_pkt, d->angle_len);
                }
                return;
            }
            if (ptype == WORLD_REQUEST_PUSHABLES) {
                int i;
                for (i = 0; i < MAX_PUSH_SNAPSHOTS; i++) {
                    PushSnapshot *ps = &s->pushables[i];
                    if (!ps->used || ps->room_idx != pl->room_idx) continue;
                    if (ps->pkt_len > 0) {
                        char tmp[MAX_PUSH_SNAP_DATA];
                        memcpy(tmp, ps->pkt, ps->pkt_len);
                        if (ps->pkt_len > 26) tmp[26] = 0; /* clear bPushing */
                        server_send(s, &pl->addr, tmp, ps->pkt_len);
                    }
                }
                return;
            }
        }

        // CH_DOOR: door snapshots — payload at data[6+]: [X I32][Y I32][Z I32][...]
        if (channel == CH_DOOR && len >= 18) {
            int dx = (int)((unsigned char)data[6]  | ((unsigned char)data[7]  << 8) |
                           ((unsigned char)data[8]  << 16) | ((unsigned char)data[9]  << 24));
            int dy = (int)((unsigned char)data[10] | ((unsigned char)data[11] << 8) |
                           ((unsigned char)data[12] << 16) | ((unsigned char)data[13] << 24));
            int dz = (int)((unsigned char)data[14] | ((unsigned char)data[15] << 8) |
                           ((unsigned char)data[16] << 16) | ((unsigned char)data[17] << 24));
            char key[MAX_DOOR_KEY];
            snprintf(key, sizeof(key), "%d,%d,%d", dx, dy, dz);

            if (ptype == DOOR_INIT) {
                if (!door_find(s, pl->room_idx, key)) {
                    DoorSnapshot *d = door_alloc(s, pl->room_idx, key);
                    if (d) {
                        int store = len < MAX_DOOR_SNAP_DATA ? len : MAX_DOOR_SNAP_DATA - 1;
                        memcpy(d->angle_pkt, data, store);
                        d->angle_pkt[1] = DOOR_ANGLE; // rewrite type byte for replay
                        d->angle_len = store;
                    }
                }
                return;
            } else if (ptype == DOOR_LOCK) {
                DoorSnapshot *d = door_find(s, pl->room_idx, key);
                if (d && d->authority_player_id != 0 && d->authority_player_id != pl->id) {
                    send_door_deny(s, &pl->addr, key);
                    return;
                }
                d = door_alloc(s, pl->room_idx, key);
                if (d) {
                    int store = len < MAX_DOOR_SNAP_DATA ? len : MAX_DOOR_SNAP_DATA - 1;
                    d->authority_player_id = pl->id;
                    memcpy(d->lock_pkt, data, store);
                    d->lock_len = store;
                }
            } else if (ptype == DOOR_UNLOCK) {
                DoorSnapshot *d = door_find(s, pl->room_idx, key);
                if (d && d->authority_player_id == pl->id) {
                    d->authority_player_id = 0;
                    d->lock_len = 0;
                }
            } else if (ptype == DOOR_STATE) {
                DoorSnapshot *d = door_alloc(s, pl->room_idx, key);
                if (d) {
                    int store = len < MAX_DOOR_SNAP_DATA ? len : MAX_DOOR_SNAP_DATA - 1;
                    memcpy(d->state_pkt, data, store);
                    d->state_len = store;
                }
            } else if (ptype == DOOR_OPEN || ptype == DOOR_CLOSE) {
                DoorSnapshot *d = door_alloc(s, pl->room_idx, key);
                if (d) {
                    int store = len < MAX_DOOR_SNAP_DATA ? len : MAX_DOOR_SNAP_DATA - 1;
                    memcpy(d->state_pkt, data, store);
                    d->state_len = store;
                    d->angle_len = 0;
                }
            } else if (ptype == DOOR_ANGLE) {
                DoorSnapshot *d = door_alloc(s, pl->room_idx, key);
                if (d) {
                    int store = len < MAX_DOOR_SNAP_DATA ? len : MAX_DOOR_SNAP_DATA - 1;
                    memcpy(d->angle_pkt, data, store);
                    d->angle_len = store;
                }
            }
        }

        // CH_PUSH: pushable init snapshot — first-write-wins, never relayed.
        // Sent by BroadcastPushableStates to register initial positions on server.
        if (channel == CH_PUSH && ptype == PUSH_INIT && len >= 27) {
            int px = (int)((unsigned char)data[6]  | ((unsigned char)data[7]  << 8) |
                           ((unsigned char)data[8]  << 16) | ((unsigned char)data[9]  << 24));
            int py = (int)((unsigned char)data[10] | ((unsigned char)data[11] << 8) |
                           ((unsigned char)data[12] << 16) | ((unsigned char)data[13] << 24));
            int pz = (int)((unsigned char)data[14] | ((unsigned char)data[15] << 8) |
                           ((unsigned char)data[16] << 16) | ((unsigned char)data[17] << 24));
            char pkey[MAX_PUSH_KEY];
            snprintf(pkey, sizeof(pkey), "%d,%d,%d", px, py, pz);
            if (!push_find(s, pl->room_idx, pkey)) {
                PushSnapshot *ps = push_alloc(s, pl->room_idx, pkey);
                if (ps) {
                    int store = len < MAX_PUSH_SNAP_DATA ? len : MAX_PUSH_SNAP_DATA - 1;
                    memcpy(ps->pkt, data, store);
                    ps->pkt_len = store;
                    /* rewrite as PUSH_STATE so clients handle it correctly on replay */
                    ps->pkt[1] = PUSH_STATE;
                }
            }
            return; /* never relay PUSH_INIT */
        }

        // CH_PUSH: pushable snapshot — payload at data[6+]: [KeyX(4)][KeyY(4)][KeyZ(4)][DispX1000(4)][Seq(4)][bPushing(1)]
        // The first player pushing owns the object until they stop; others get PUSH_DENIED.
        if (channel == CH_PUSH && ptype == PUSH_STATE && len >= 27) {
            int px = (int)((unsigned char)data[6]  | ((unsigned char)data[7]  << 8) |
                           ((unsigned char)data[8]  << 16) | ((unsigned char)data[9]  << 24));
            int py = (int)((unsigned char)data[10] | ((unsigned char)data[11] << 8) |
                           ((unsigned char)data[12] << 16) | ((unsigned char)data[13] << 24));
            int pz = (int)((unsigned char)data[14] | ((unsigned char)data[15] << 8) |
                           ((unsigned char)data[16] << 16) | ((unsigned char)data[17] << 24));
            char pkey[MAX_PUSH_KEY];
            snprintf(pkey, sizeof(pkey), "%d,%d,%d", px, py, pz);
            {
                PushSnapshot *ps = push_alloc(s, pl->room_idx, pkey);
                if (ps) {
                    int store = len < MAX_PUSH_SNAP_DATA ? len : MAX_PUSH_SNAP_DATA - 1;
                    if (data[26]) {
                        if (ps->owner_player_id != 0 && ps->owner_player_id != pl->id) {
                            send_push_denied(s, &pl->addr, data + OL_HEADER_SIZE);
                            return;
                        }
                        ps->owner_player_id = pl->id;
                    } else if (ps->owner_player_id == pl->id) {
                        ps->owner_player_id = 0;
                    }
                    memcpy(ps->pkt, data, store);
                    ps->pkt_len = store;
                }
            }
        }

        // CH_WORLD: PICKUP_KISMET snapshot — payload at data[6+]: [strLen(1)][path bytes]
        if (channel == CH_WORLD && ptype == WORLD_PICKUP_KISMET && len >= 8) {
            int plen = (int)(unsigned char)data[6];
            int pbytes = len - 7;
            char key[MAX_PICKUP_KEY];
            memset(key, 0, sizeof(key));
            if (plen > pbytes) plen = pbytes;
            if (plen > MAX_PICKUP_KEY - 1) plen = MAX_PICKUP_KEY - 1;
            memcpy(key, data + 7, plen);
            if (key[0]) {
                PickupSnapshot *p = pickup_alloc(s, pl->room_idx, key);
                if (p) {
                    int store = len < MAX_PICKUP_SNAP_DATA ? len : MAX_PICKUP_SNAP_DATA - 1;
                    memcpy(p->pkt, data, store);
                    p->pkt_len = store;
                }
            }
        }

        // CH_WORLD: PICKUP_STATE snapshot — payload at data[6+]: [X I32][Y I32][Z I32]
        if (channel == CH_WORLD && ptype == WORLD_PICKUP_STATE && len >= 18) {
            int px = (int)((unsigned char)data[6]  | ((unsigned char)data[7]  << 8) |
                           ((unsigned char)data[8]  << 16) | ((unsigned char)data[9]  << 24));
            int py = (int)((unsigned char)data[10] | ((unsigned char)data[11] << 8) |
                           ((unsigned char)data[12] << 16) | ((unsigned char)data[13] << 24));
            int pz = (int)((unsigned char)data[14] | ((unsigned char)data[15] << 8) |
                           ((unsigned char)data[16] << 16) | ((unsigned char)data[17] << 24));
            char key[MAX_PICKUP_KEY];
            snprintf(key, sizeof(key), "%d,%d,%d", px, py, pz);
            {
                PickupSnapshot *p = pickup_alloc(s, pl->room_idx, key);
                if (p) {
                    int store = len < MAX_PICKUP_SNAP_DATA ? len : MAX_PICKUP_SNAP_DATA - 1;
                    memcpy(p->pkt, data, store);
                    p->pkt_len = store;
                }
            }
        }

        // CH_ENEMY: snapshots — payload at data[6+]: [name_len(1)][name bytes][...]
        if (channel == CH_ENEMY && len >= 8) {
            unsigned char nlen = (unsigned char)data[6];
            int max_n = len - 7;
            char ename[MAX_ENEMY_NAME];
            memset(ename, 0, sizeof(ename));
            if (nlen > max_n) nlen = (unsigned char)max_n;
            if (nlen > MAX_ENEMY_NAME - 1) nlen = MAX_ENEMY_NAME - 1;
            memcpy(ename, data + 7, nlen);

            if (ptype == ENEMY_LOC) {
                EnemySnapshot *e = enemy_find(s, pl->room_idx, ename);
                if (e) {
                    int store = len < MAX_ENEMY_SNAP_DATA ? len : MAX_ENEMY_SNAP_DATA - 1;
                    memcpy(e->loc_pkt, data, store);
                    e->loc_len = store;
                }
            } else if (ptype == ENEMY_SPAWN) {
                EnemySnapshot *e = enemy_alloc(s, pl->room_idx, ename, pl->id);
                if (e) {
                    int store = len < MAX_ENEMY_SNAP_DATA ? len : MAX_ENEMY_SNAP_DATA - 1;
                    memcpy(e->spawn_pkt, data, store);
                    e->spawn_len = store;
                    e->smt_len = 0;
                    e->loc_len = 0;
                }
            } else if (ptype == ENEMY_DEL) {
                enemy_remove(s, pl->room_idx, ename);
            } else if (ptype == ENEMY_SMT) {
                int smt_off = 7 + nlen;
                unsigned char smt_type = (smt_off < len) ? (unsigned char)data[smt_off] : 0;
                int is_transient = (smt_type >= 77 && smt_type <= 93);
                EnemySnapshot *e = enemy_find(s, pl->room_idx, ename);
                if (e) {
                    if (is_transient) {
                        e->smt_len = 0;
                    } else {
                        int store = len < MAX_ENEMY_SNAP_DATA ? len : MAX_ENEMY_SNAP_DATA - 1;
                        memcpy(e->smt_pkt, data, store);
                        e->smt_len = store;
                    }
                }
            }
        }

        // GUI events + history
        sender_slot = (int)(pl - s->players);
        room = s->rooms[pl->room_idx].code;
        if (channel == CH_HERO) {
            switch (ptype) {
                case HERO_STATE: gui_push_event(s, sender_slot, GUIPKT_LOC); break;
                case HERO_SMT_TYPE: {
                    int smt_type = (len > 6) ? (unsigned char)data[6] : -1;
                    gui_push_event(s, sender_slot, GUIPKT_SMT);
                    history_push(s, "[%s@%s] SMT %d", pl->nick, room, smt_type);
                    break;
                }
                case HERO_PLAYER_LIFECYCLE: {
                    gui_push_event(s, sender_slot, GUIPKT_RESPAWN);
                    history_push(s, "[%s@%s] Lifecycle", pl->nick, room);
                    break;
                }
                default: break;
            }
        } else if (channel == CH_DOOR) {
            gui_push_event(s, sender_slot, GUIPKT_DOOR);
            history_push(s, "[%s@%s] Door 0x%02x", pl->nick, room, ptype);
        } else if (channel == CH_ENEMY) {
            switch (ptype) {
                case ENEMY_SPAWN: {
                    char ename2[MAX_ENEMY_NAME];
                    memset(ename2, 0, sizeof(ename2));
                    if (len >= 8) {
                        unsigned char nl2 = (unsigned char)data[6];
                        int mn2 = len - 7; if (nl2 > mn2) nl2 = (unsigned char)mn2;
                        if (nl2 > MAX_ENEMY_NAME - 1) nl2 = MAX_ENEMY_NAME - 1;
                        memcpy(ename2, data + 7, nl2);
                    }
                    history_push(s, "[%s@%s] EnemySpawn %s", pl->nick, room, ename2);
                    break;
                }
                case ENEMY_DEL: {
                    char ename2[MAX_ENEMY_NAME];
                    memset(ename2, 0, sizeof(ename2));
                    if (len >= 8) {
                        unsigned char nl2 = (unsigned char)data[6];
                        int mn2 = len - 7; if (nl2 > mn2) nl2 = (unsigned char)mn2;
                        if (nl2 > MAX_ENEMY_NAME - 1) nl2 = MAX_ENEMY_NAME - 1;
                        memcpy(ename2, data + 7, nl2);
                    }
                    history_push(s, "[%s@%s] EnemyDel %s", pl->nick, room, ename2);
                    break;
                }
                case ENEMY_SMT: {
                    char ename2[MAX_ENEMY_NAME];
                    int smt_type2 = -1;
                    memset(ename2, 0, sizeof(ename2));
                    gui_push_event(s, sender_slot, GUIPKT_ENPC_SMT);
                    if (len >= 8) {
                        unsigned char nl2 = (unsigned char)data[6];
                        int mn2 = len - 7; if (nl2 > mn2) nl2 = (unsigned char)mn2;
                        if (nl2 > MAX_ENEMY_NAME - 1) nl2 = MAX_ENEMY_NAME - 1;
                        memcpy(ename2, data + 7, nl2);
                        { int smt_off2 = 7 + nl2; if (smt_off2 < len) smt_type2 = (unsigned char)data[smt_off2]; }
                    }
                    history_push(s, "[%s@%s] EnemySMT %s #%d", pl->nick, room, ename2, smt_type2);
                    break;
                }
                default: break;
            }
        } else if (channel == CH_WORLD && ptype == WORLD_TRIGGER_ACT) {
            char path[128];
            memset(path, 0, sizeof(path));
            if (len >= 12) {
                unsigned char slen = (unsigned char)data[10];
                int ml = len - 11; if (slen > ml) slen = (unsigned char)ml;
                if (slen > 127) slen = 127;
                memcpy(path, data + 11, slen);
            }
            history_push(s, "[%s@%s] Trigger %s", pl->nick, room, path);
        }

        // Relay as-is — receivers parse the binary payload themselves
        server_relay(s, pl, data, len);
        return;
    }

    // Sanitize: work on a null-terminated copy
    char buf[MAX_PACKET + 1];
    if (len > MAX_PACKET) len = MAX_PACKET;
    memcpy(buf, data, len);
    buf[len] = '\0';

    // Trim trailing whitespace
    int end = len - 1;
    while (end >= 0 && (buf[end] == '\n' || buf[end] == '\r' || buf[end] == ' '))
        buf[end--] = '\0';

    if (!buf[0]) return;

    // HELLO handshake — no auth yet
    if (strncmp(buf, "HELLO,", 6) == 0) {
        handle_hello(s, buf, addr);
        return;
    }

    // All other packets: <player_id>,<rest>
    char *comma = strchr(buf, ',');
    if (!comma) return;

    int player_id = atoi(buf);
    Player *pl = server_find_player_by_id(s, player_id);
    if (!pl || !addr_eq(&pl->addr, addr)) return;

    pl->last_seen = mono_now();
    if (!check_rate(pl)) return;

    const char *rest = comma + 1;

    // Server-consumed packets
    if (strcmp(rest, "HEARTBEAT") == 0)
        return;


    if (strncmp(rest, "NICK,", 5) == 0) {
        const char *nick = rest + 5;
        // Sanitize: alphanumeric + underscore only
        char clean[MAX_NICK] = {0};
        int  ci = 0;
        for (int i = 0; nick[i] && ci < MAX_NICK - 1; i++)
            if ((nick[i] >= 'A' && nick[i] <= 'Z') ||
                (nick[i] >= 'a' && nick[i] <= 'z') ||
                (nick[i] >= '0' && nick[i] <= '9') ||
                nick[i] == '_')
                clean[ci++] = nick[i];
        if (!clean[0]) snprintf(clean, sizeof(clean), "Player%d", player_id);
        int first = (pl->nick[0] == '\0' || strcmp(pl->nick, clean) != 0);
        memcpy(pl->nick, clean, MAX_NICK - 1);
        pl->nick[MAX_NICK - 1] = '\0';
        if (first) {
            server_log(s, "[%s:%d] ID=%d identified as '%s'",
                       pl->ip, ntohs(addr->sa.sin_port), player_id, clean);
            history_push(s, "[%s] identified", clean);
        }
        // NICK is also relayed so other players see the nick update
    }

    if (strcmp(rest, "DISCONNECT") == 0) {
        server_disconnect(s, pl);
        return;
    }

    // All remaining text packets are legacy — drop silently.
    // All game data (doors, enemies, pickups, world) is now binary.
}

// ---------------------------------------------------------------------------
// Background tasks
// ---------------------------------------------------------------------------

void run_heartbeat(Server *s) {
    // No-op: server no longer sends heartbeat packets.
    // Client keepalive is maintained by the client's own tick sends.
    (void)s;
}

void run_timeout(Server *s) {
    double now = mono_now();
    if (now < s->next_timeout) return;
    s->next_timeout = now + HEARTBEAT_SEC;

    for (int i = 0; i < MAX_CLIENTS; i++) {
        Player *p = &s->players[i];
        if (!p->used) continue;
        if (now - p->last_seen > TIMEOUT_SEC) {
            server_log(s, "[%s] ID=%d ('%s') timed out", p->ip, p->id, p->nick);
            history_push(s, "[%s] timed out", p->nick);
            server_disconnect(s, p);
        }
    }
}

// ---------------------------------------------------------------------------
// Init / run / shutdown
// ---------------------------------------------------------------------------

void server_init(Server *s, uint16_t port, const char *name, const char *bind_ip, const char *db_path) {
    memset(s, 0, sizeof(*s));
    s->player_id_seq = 1000;
    strncpy(s->name, name ? name : "OLServer", sizeof(s->name) - 1);
    strncpy(s->bind_ip, bind_ip && bind_ip[0] ? bind_ip : DEFAULT_IP, MAX_IP - 1);
    s->port = port;
    db_init(&s->db, db_path ? db_path : "relay.json");

    platform_init();

    s->sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (s->sock == INVALID_SOCK) {
#ifdef _WIN32
        FILE *_ef = fopen("startup_debug.txt", "a");
        if (_ef) { fprintf(_ef, "socket() failed: WSA error %d\n", WSAGetLastError()); fclose(_ef); }
#else
        perror("socket");
#endif
        exit(1);
    }

    // Non-blocking
#ifdef _WIN32
    u_long nb = 1;
    ioctlsocket(s->sock, FIONBIO, &nb);
#else
    int flags = fcntl(s->sock, F_GETFL, 0);
    fcntl(s->sock, F_SETFL, flags | O_NONBLOCK);
#endif

    // SO_REUSEADDR
    int yes = 1;
    setsockopt(s->sock, SOL_SOCKET, SO_REUSEADDR, (const char *)&yes, sizeof(yes));

    struct sockaddr_in sa = {0};
    sa.sin_family = AF_INET;
    sa.sin_port   = htons(port);
    if (strcmp(s->bind_ip, DEFAULT_IP) == 0 || !s->bind_ip[0])
        sa.sin_addr.s_addr = INADDR_ANY;
    else
        inet_pton(AF_INET, s->bind_ip, &sa.sin_addr);

    if (bind(s->sock, (struct sockaddr *)&sa, sizeof(sa)) < 0) {
#ifdef _WIN32
        FILE *_ef = fopen("startup_debug.txt", "a");
        if (_ef) { fprintf(_ef, "bind() failed on %s:%d — WSA error %d\n", s->bind_ip, port, WSAGetLastError()); fclose(_ef); }
#else
        perror("bind");
#endif
        exit(1);
    }

    double now = mono_now();
    s->next_heartbeat = now + HEARTBEAT_SEC;
    s->next_timeout   = now + HEARTBEAT_SEC;

    history_push(s, "Listening on %s:%d  name='%s'", s->bind_ip, (int)s->port, s->name);
}

void server_poll(Server *s) {
    char buf[MAX_PACKET];
    fd_set rfds;
    struct timeval tv;
    int r;
    FD_ZERO(&rfds);
    FD_SET(s->sock, &rfds);
    tv.tv_sec  = 0;
    /* 50ms keeps shutdown checks and the CLI responsive; wake up sooner while
       snapshots are queued so the drip rate does not depend on incoming traffic. */
    tv.tv_usec = server_has_queued_snaps(s) ? (long)(SNAP_DRIP_INTERVAL * 1e6) : 50000;
    r = select((int)s->sock + 1, &rfds, NULL, NULL, &tv);

    if (s->shutdown_requested) return;

    if (r > 0 && FD_ISSET(s->sock, &rfds)) {
        Addr from;
        socklen_t slen = sizeof(from.sa);
        int len = (int)recvfrom(s->sock, buf, sizeof(buf), 0,
                                (struct sockaddr *)&from.sa, &slen);
        if (len > 0)
            handle_packet(s, buf, len, &from);
    }

    run_heartbeat(s);
    run_timeout(s);
    server_drain_snaps(s);
}

void server_run(Server *s) {
    while (!s->shutdown_requested)
        server_poll(s);
}

void server_shutdown(Server *s) {
    if (s->sock != INVALID_SOCK) {
        sock_close(s->sock);
        s->sock = INVALID_SOCK;
    }
    platform_cleanup();
}
