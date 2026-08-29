/*=============================================================================
    main.cpp - entry point and CLI loop
    BUILD_GUI defined at compile time selects ImGui vs TUI mode.
=============================================================================*/
#ifdef BUILD_GUI
#  include "gui_app.h"
#endif

extern "C" {
#include "server.h"
#include "cli.h"
}

#include <csignal>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#ifdef _WIN32
#  include <windows.h>
#else
#  include <unistd.h>
#endif

// ---------------------------------------------------------------------------
// Shared state — heap-allocated to avoid stack overflow on Windows
// ---------------------------------------------------------------------------

static Server      *g_server  = nullptr;
static volatile int g_running = 1;

static void sig_handler(int) { g_running = 0; }

// ---------------------------------------------------------------------------
// Resolve relay.db path relative to the executable directory
// ---------------------------------------------------------------------------

static void resolve_db_path(char *out, int outlen, const char *argv0) {
    char dir[512] = {};
#ifdef _WIN32
    char exe[512];
    GetModuleFileNameA(NULL, exe, sizeof(exe));
    char *last = strrchr(exe, '\\');
    if (last) { *last = '\0'; strncpy(dir, exe, sizeof(dir) - 1); }
    else strncpy(dir, ".", sizeof(dir) - 1);
#else
    ssize_t len = readlink("/proc/self/exe", dir, sizeof(dir) - 1);
    if (len > 0) {
        dir[len] = '\0';
        char *last = strrchr(dir, '/');
        if (last) *last = '\0';
    } else if (argv0) {
        strncpy(dir, argv0, sizeof(dir) - 1);
        char *last = strrchr(dir, '/');
        if (last) *last = '\0';
        else strncpy(dir, ".", sizeof(dir) - 1);
    } else {
        strncpy(dir, ".", sizeof(dir) - 1);
    }
#endif
    snprintf(out, outlen, "%s/relay.db", dir);
}

// ---------------------------------------------------------------------------
// Argument parsing — overrides db.config values
// ---------------------------------------------------------------------------

static void print_usage(const char *prog) {
    fprintf(stderr,
        "Usage: %s [options]\n"
        "  --port  <port>   UDP port\n"
        "  --ip    <addr>   Bind IP\n"
        "  --name  <name>   Server name\n"
        "  --log-packets    Log every relayed packet\n"
        "  --help\n"
        "All settings are stored in relay.db (same dir as binary).\n",
        prog);
}

// Returns false if --help was requested.
static bool parse_args(int argc, char **argv, DB *db) {
    for (int i = 1; i < argc; i++) {
        if (!strcmp(argv[i], "--help") || !strcmp(argv[i], "-h")) {
            print_usage(argv[0]); return false;
        } else if (!strcmp(argv[i], "--port") && i + 1 < argc) {
            db->config.port = (uint16_t)atoi(argv[++i]);
        } else if (!strcmp(argv[i], "--ip") && i + 1 < argc) {
            strncpy(db->config.bind_ip, argv[++i], DB_IP_BIND_LEN - 1);
        } else if (!strcmp(argv[i], "--name") && i + 1 < argc) {
            strncpy(db->config.name, argv[++i], DB_NAME_LEN - 1);
        } else {
            fprintf(stderr, "Unknown option: %s\n", argv[i]);
            print_usage(argv[0]); return false;
        }
    }
    return true;
}

// ---------------------------------------------------------------------------
// Start server from db
// ---------------------------------------------------------------------------

static void server_start(Server *s, DB *db) {
    server_init(s, db->config.port, db->config.name, db->config.bind_ip, db->path);
    for (int i = 0; i < s->db.n_rooms; i++)
        server_add_room(s, s->db.rooms[i].code, s->db.rooms[i].password);
    if (s->n_rooms == 0)
        server_add_room(s, "PUBLIC", "");
}

// ---------------------------------------------------------------------------
// CLI loop
// ---------------------------------------------------------------------------

#ifndef BUILD_GUI
static void run_cli(DB *db) {
    signal(SIGINT,  sig_handler);
    signal(SIGTERM, sig_handler);
#ifndef _WIN32
    signal(SIGPIPE, SIG_IGN);
#endif

    server_start(g_server, db);
    cli_init(g_server);

    char rooms_str[512] = {};
    for (int i = 0; i < g_server->n_rooms; i++) {
        Room *r = &g_server->rooms[i];
        char entry[64];
        snprintf(entry, sizeof(entry), r->password[0] ? " %s(pw)" : " %s", r->code);
        strncat(rooms_str, entry, sizeof(rooms_str) - strlen(rooms_str) - 1);
    }
    cli_log("[%s] UDP %s:%d  rooms:%s",
            db->config.name, db->config.bind_ip, db->config.port, rooms_str);
    cli_log("Type 'help' for available commands.");
    cli_redraw();

    char buf[MAX_PACKET];
    while (g_running) {
        fd_set rfds;
        FD_ZERO(&rfds);
        FD_SET(g_server->sock, &rfds);
        struct timeval tv = {0, 50000};
        select((int)g_server->sock + 1, &rfds, NULL, NULL, &tv);

        if (FD_ISSET(g_server->sock, &rfds)) {
            Addr from;
            socklen_t slen = sizeof(from.sa);
            int len = (int)recvfrom(g_server->sock, buf, sizeof(buf), 0,
                                    (struct sockaddr *)&from.sa, &slen);
            if (len > 0)
                handle_packet(g_server, buf, len, &from);
        }

        run_heartbeat(g_server);
        run_timeout(g_server);

        if (cli_poll())
            cli_dispatch(g_server, cli_last_cmd);
    }

    cli_log("Shutting down...");
    db_save(&g_server->db);
    cli_shutdown();
    server_shutdown(g_server);
}
#endif

// ---------------------------------------------------------------------------
// Entry point
// ---------------------------------------------------------------------------

int main(int argc, char **argv) {
    char db_path[512];
    resolve_db_path(db_path, sizeof(db_path), argv[0]);

    DB *db = (DB *)calloc(1, sizeof(DB));
    if (!db) { fprintf(stderr, "out of memory\n"); return 1; }
    db_init(db, db_path);

    if (!parse_args(argc, argv, db)) {
        free(db);
        return 1;
    }

    g_server = (Server *)calloc(1, sizeof(Server));
    if (!g_server) { fprintf(stderr, "out of memory\n"); free(db); return 1; }

#ifdef BUILD_GUI
    int ret = gui_run(g_server, &g_running, db);
    free(g_server);
    free(db);
    return ret;
#else
    run_cli(db);
    free(g_server);
    free(db);
    return 0;
#endif
}
