/*=============================================================================
    db.h - in-memory database with JSON persistence
=============================================================================*/
#pragma once

#include <stdint.h>

// ---------------------------------------------------------------------------
// Limits
// ---------------------------------------------------------------------------

#define DB_MAX_ROOMS          64
#define DB_MAX_TRUSTED        256   // per room
#define DB_MAX_BANS           512   // per room (and global)
#define DB_ROOM_CODE_LEN      33
#define DB_PASSWORD_LEN       64
#define DB_IP_LEN             46
#define DB_NICK_LEN           33
#define DB_REASON_LEN         128
#define DB_TIME_LEN           20    // "YYYY-MM-DD HH:MM:SS"
#define DB_PATH_LEN           256
#define DB_NAME_LEN           64
#define DB_IP_BIND_LEN        46

// ---------------------------------------------------------------------------
// Server config (stored in relay.db under "config" key)
// ---------------------------------------------------------------------------

typedef struct {
    char     name[DB_NAME_LEN];
    char     bind_ip[DB_IP_BIND_LEN];
    uint16_t port;
} DBConfig;

// ---------------------------------------------------------------------------
// Records
// ---------------------------------------------------------------------------

typedef struct {
    char ip[DB_IP_LEN];
    char nick[DB_NICK_LEN];
    char reason[DB_REASON_LEN];
    char banned_at[DB_TIME_LEN];
} DBBan;

typedef struct {
    char code[DB_ROOM_CODE_LEN];
    char password[DB_PASSWORD_LEN];

    // Trusted IPs: allowed to skip password re-entry.
    // Cleared when password changes.
    char trusted[DB_MAX_TRUSTED][DB_IP_LEN];
    int  n_trusted;

    // Per-room bans
    DBBan bans[DB_MAX_BANS];
    int   n_bans;
} DBRoom;

// ---------------------------------------------------------------------------
// Database
// ---------------------------------------------------------------------------

typedef struct {
    char   path[DB_PATH_LEN];

    DBConfig config;

    DBRoom rooms[DB_MAX_ROOMS];
    int    n_rooms;

    // Global bans (affect all rooms)
    DBBan  bans[DB_MAX_BANS];
    int    n_bans;
} DB;

// ---------------------------------------------------------------------------
// Lifecycle
// ---------------------------------------------------------------------------

void db_init(DB *db, const char *path);
int  db_save(DB *db);

// ---------------------------------------------------------------------------
// Rooms
// ---------------------------------------------------------------------------

DBRoom *db_find_room(DB *db, const char *code);
DBRoom *db_add_room(DB *db, const char *code, const char *password);
int     db_update_room_password(DB *db, const char *code, const char *password);

// ---------------------------------------------------------------------------
// Trusted IPs (per room)
// ---------------------------------------------------------------------------

int  db_is_trusted(DBRoom *r, const char *ip);
int  db_add_trusted(DBRoom *r, const char *ip);
int  db_remove_trusted(DBRoom *r, const char *ip);
void db_clear_trusted(DBRoom *r);   // called on password change

// ---------------------------------------------------------------------------
// Global bans
// ---------------------------------------------------------------------------

DBBan *db_find_ban(DB *db, const char *ip);
DBBan *db_add_ban(DB *db, const char *ip, const char *nick, const char *reason);
int    db_remove_ban(DB *db, const char *ip);

// ---------------------------------------------------------------------------
// Per-room bans
// ---------------------------------------------------------------------------

DBBan *db_find_room_ban(DBRoom *r, const char *ip);
DBBan *db_add_room_ban(DBRoom *r, const char *ip, const char *nick, const char *reason);
int    db_remove_room_ban(DBRoom *r, const char *ip);
