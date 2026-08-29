/*=============================================================================
    gui_app.cpp - ImGui render loop (SDL2 + OpenGL3, cross-platform)
=============================================================================*/
#include "gui_app.h"

extern "C" {
#include "cli.h"
#include "db.h"
}

#include "imgui.h"

#ifdef GUI_BACKEND_DX9
#  include <windows.h>
#else
#  include "backends/imgui_impl_sdl2.h"
#  include "backends/imgui_impl_opengl3.h"
#  include <SDL.h>
#  include <SDL_opengl.h>
#endif

#include <atomic>
#include <thread>
#include <mutex>
#include <deque>
#include <string>
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <cmath>

// ---------------------------------------------------------------------------
// Log queue — pointers to avoid global C++ constructors (crash on MinGW before main)
// ---------------------------------------------------------------------------

static std::mutex              *g_log_mutex   = nullptr;
static std::deque<std::string> *g_log_lines   = nullptr;
static const int                LOG_MAX = 2000;
static bool                     g_log_scroll = true;

static void log_hook(const char *line) {
    std::lock_guard<std::mutex> lk(*g_log_mutex);
    g_log_lines->emplace_back(line);
    if ((int)g_log_lines->size() > LOG_MAX)
        g_log_lines->pop_front();
    g_log_scroll = true;
}

// ---------------------------------------------------------------------------
// Server thread
// ---------------------------------------------------------------------------

static std::atomic<bool> *g_srv_running = nullptr;

static void server_thread_fn(Server *s, volatile int *running) {
    char buf[MAX_PACKET];
    while (*g_srv_running && *running) {
        fd_set rfds;
        FD_ZERO(&rfds);
        FD_SET(s->sock, &rfds);
        struct timeval tv = {0, 50000};
        select((int)s->sock + 1, &rfds, NULL, NULL, &tv);
        if (FD_ISSET(s->sock, &rfds)) {
            Addr from;
            socklen_t slen = sizeof(from.sa);
            int len = (int)recvfrom(s->sock, buf, sizeof(buf), 0,
                                    (struct sockaddr *)&from.sa, &slen);
            if (len > 0) handle_packet(s, buf, len, &from);
        }
        run_heartbeat(s);
        run_timeout(s);
    }
    db_save(&s->db);
    server_shutdown(s);
}

// ---------------------------------------------------------------------------
// Start / Stop helpers
// ---------------------------------------------------------------------------

static void gui_server_start(Server *s, volatile int *running, DB *db) {
    server_init(s, db->config.port, db->config.name, db->config.bind_ip, db->path);
    for (int i = 0; i < s->db.n_rooms; i++)
        server_add_room(s, s->db.rooms[i].code, s->db.rooms[i].password);
    if (s->n_rooms == 0)
        server_add_room(s, "PUBLIC", "");

    log_hook(("Server started on " + std::string(db->config.bind_ip) +
              ":" + std::to_string(db->config.port)).c_str());

    *g_srv_running = true;
    // server thread is started by gui_run after this call
    (void)running;
}

static void gui_server_stop(Server *s, volatile int *running, std::thread &srv_thread) {
    *g_srv_running = false;
    if (srv_thread.joinable()) {
        srv_thread.join();
    }
    log_hook("Server stopped.");
    (void)running;
}

// ---------------------------------------------------------------------------
// Command input
// ---------------------------------------------------------------------------

static char g_cmd_buf[4096] = {};

static void dispatch_cmd(Server *s) {
    if (!g_cmd_buf[0]) return;
    log_hook((std::string("> ") + g_cmd_buf).c_str());
    cli_dispatch(s, g_cmd_buf);
    g_cmd_buf[0] = '\0';
}

// ---------------------------------------------------------------------------
// ESpecialMoveType name lookup (mirrors OLGameClasses.h enum 0-100)
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
// Hero state snapshot decoder (mirrors DecodeBinaryState in HeroChannel.cpp)
// Payload layout: [0x01][pid 4B][LocX f32][LocY f32][LocZ f32][Pitch u16][Yaw u16]
//                 [VX i16][VY i16][VZ i16][Crouch u8][CamState u8][SMT u8][LocMode u8]
//                 [LadderD i16][HangD i16][WalkD i16][Health i16][HeatShield u8]
//                 [HeatDist i16][SqueezeD i16][Hobbling u8][HobbInt i16][TargHobb i16]
//                 [Limping u8][Lean i16][PeekRatio i16][CornerIK i16][CamVis u8]
//                 [LeftAnim u8][EyeYaw i16][InDark u8][IsGhost u8]
//                 [Parrying u8][ParryDist i16][ParryYaw i16]
//                 [NickLen u8][nick bytes...]
// ---------------------------------------------------------------------------

struct SnapState {
    float loc_x, loc_y, loc_z;
    float vel_x, vel_y, vel_z;
    int   pitch, yaw;
    int   health;
    int   cam_state;
    int   smt;
    int   loc_mode;
    float ladder_d, hang_d, walk_d;
    int   heat_shield;
    float heat_dist;
    float squeeze_d;
    int   crouched;
    int   hobbling;
    float hobb_int, targ_hobb;
    int   limping;
    float lean, peek_ratio, corner_ik;
    int   cam_vis, left_anim;
    int   eye_yaw;
    int   in_dark, is_ghost;
    int   parrying;
    float parry_dist, parry_yaw;
    char  nick[33];
    int   valid;  // 1 if decoded successfully
};

static inline float snap_f32(const uint8_t *b, int off) {
    uint32_t u = (uint32_t)b[off] | ((uint32_t)b[off+1]<<8) |
                 ((uint32_t)b[off+2]<<16) | ((uint32_t)b[off+3]<<24);
    float v; memcpy(&v, &u, 4); return v;
}
static inline int snap_u8 (const uint8_t *b, int off) { return (int)b[off]; }
static inline int snap_u16(const uint8_t *b, int off) { return (int)b[off] | ((int)b[off+1]<<8); }
static inline int snap_i16(const uint8_t *b, int off) {
    int u = (int)b[off] | ((int)b[off+1]<<8);
    return u >= 0x8000 ? u - 0x10000 : u;
}

static SnapState decode_snap(const uint8_t *raw, int len) {
    SnapState s = {};
    // wire: [channel(1)][type(1)][player_id LE4][payload...]
    if (len < 6 + 62) return s;
    int o = 6; // skip channel + type + player_id
    s.loc_x     = snap_f32(raw, o); o += 4;
    s.loc_y     = snap_f32(raw, o); o += 4;
    s.loc_z     = snap_f32(raw, o); o += 4;
    s.pitch     = snap_u16(raw, o); o += 2;
    s.yaw       = snap_u16(raw, o); o += 2;
    s.vel_x     = (float)snap_i16(raw, o); o += 2;
    s.vel_y     = (float)snap_i16(raw, o); o += 2;
    s.vel_z     = (float)snap_i16(raw, o); o += 2;
    s.crouched  = snap_u8(raw, o++);
    s.cam_state = snap_u8(raw, o++);
    s.smt       = snap_u8(raw, o++);
    s.loc_mode  = snap_u8(raw, o++);
    s.ladder_d  = snap_i16(raw, o) / 1000.f; o += 2;
    s.hang_d    = snap_i16(raw, o) / 1000.f; o += 2;
    s.walk_d    = snap_i16(raw, o) / 1000.f; o += 2;
    s.health    = snap_i16(raw, o);           o += 2;
    s.heat_shield = snap_u8(raw, o++);
    s.heat_dist = snap_i16(raw, o) / 10.f;   o += 2;
    s.squeeze_d = snap_i16(raw, o) / 1000.f; o += 2;
    s.hobbling  = snap_u8(raw, o++);
    s.hobb_int  = snap_i16(raw, o) / 1000.f; o += 2;
    s.targ_hobb = snap_i16(raw, o) / 1000.f; o += 2;
    s.limping   = snap_u8(raw, o++);
    s.lean      = snap_i16(raw, o) / 1000.f; o += 2;
    s.peek_ratio= snap_i16(raw, o) / 1000.f; o += 2;
    s.corner_ik = snap_i16(raw, o) / 1000.f; o += 2;
    s.cam_vis   = snap_u8(raw, o++);
    s.left_anim = snap_u8(raw, o++);
    s.eye_yaw   = snap_i16(raw, o);           o += 2;
    s.in_dark   = snap_u8(raw, o++);
    s.is_ghost  = snap_u8(raw, o++);
    s.parrying  = snap_u8(raw, o++);
    s.parry_dist= (float)snap_i16(raw, o);    o += 2;
    s.parry_yaw = (float)snap_i16(raw, o);    o += 2;
    // nick tail (optional)
    s.nick[0] = '\0';
    if (o + 1 <= len) {
        int nlen = snap_u8(raw, o++);
        if (nlen > 32) nlen = 32;
        if (o + nlen <= len) {
            for (int i = 0; i < nlen; i++) s.nick[i] = (char)(raw[o+i] & 0x7F);
            s.nick[nlen] = '\0';
        }
    }
    s.valid = 1;
    return s;
}

// ---------------------------------------------------------------------------
// Enemy snapshot decode
// spawn_pkt format (after 5-byte header: type u8 + player_id u32):
//   [name_len u8][name...][class_len u8][class...][X f32][Y f32][Z f32][Yaw u16]
//   [mesh_len u8][mesh...][weapon u8][bColor u8][if bColor: R i16 G i16 B i16 A i16]
// smt_pkt format (after 5-byte header):
//   [name_len u8][name...][smt_type u8]
// ---------------------------------------------------------------------------

struct EnemySnapState {
    char  name[48];
    char  cls[48];
    float x, y, z;
    int   yaw;
    int   weapon;
    int   smt;       // -1 = no smt packet yet
    int   valid;
};

static EnemySnapState decode_enemy_snap(const EnemySnapshot *e) {
    EnemySnapState r = {};
    r.smt = -1;
    strncpy(r.name, e->name, sizeof(r.name) - 1);

    // Decode spawn_pkt
    if (e->spawn_len >= 5) {
        const uint8_t *b = (const uint8_t *)e->spawn_pkt;
        int o = 5; // skip type + player_id
        // name
        if (o >= e->spawn_len) goto try_smt;
        { int nl = b[o++]; if (o + nl > e->spawn_len) goto try_smt;
          int nc = nl > 47 ? 47 : nl;
          for (int i = 0; i < nc; i++) r.name[i] = (char)(b[o+i] & 0x7F);
          r.name[nc] = '\0'; o += nl; }
        // class
        if (o >= e->spawn_len) goto try_smt;
        { int cl = b[o++]; if (o + cl > e->spawn_len) goto try_smt;
          int cc = cl > 47 ? 47 : cl;
          for (int i = 0; i < cc; i++) r.cls[i] = (char)(b[o+i] & 0x7F);
          r.cls[cc] = '\0'; o += cl; }
        // X Y Z (f32 LE)
        if (o + 12 > e->spawn_len) goto try_smt;
        memcpy(&r.x, b + o, 4); o += 4;
        memcpy(&r.y, b + o, 4); o += 4;
        memcpy(&r.z, b + o, 4); o += 4;
        // Yaw u16
        if (o + 2 > e->spawn_len) goto try_smt;
        r.yaw = (int)b[o] | ((int)b[o+1] << 8); o += 2;
        // mesh (skip)
        if (o < e->spawn_len) { int ml = b[o++]; o += ml; }
        // weapon u8
        if (o < e->spawn_len) r.weapon = b[o++];
        r.valid = 1;
    }

try_smt:
    // Decode smt_pkt
    if (e->smt_len >= 5) {
        const uint8_t *b = (const uint8_t *)e->smt_pkt;
        int o = 5;
        // name (skip)
        if (o < e->smt_len) { int nl = b[o++]; o += nl; }
        // smt_type u8
        if (o < e->smt_len) r.smt = b[o];
    }

    // Override X/Y/Z from latest loc_pkt if available.
    // Stored with relay header: [type(1)][senderID LE4][name_len(1)][name bytes][null(1)][X f32][Y f32][Z f32][yaw u16]...
    if (e->loc_len >= 6) {
        const uint8_t *b = (const uint8_t *)e->loc_pkt;
        int o = 5; // skip type + senderID
        if (o < e->loc_len) {
            int nl = b[o++] + 1; // name bytes + null terminator
            o += nl;
            if (o + 12 <= e->loc_len) {
                memcpy(&r.x, b + o, 4); o += 4;
                memcpy(&r.y, b + o, 4); o += 4;
                memcpy(&r.z, b + o, 4); o += 4;
                if (o + 2 <= e->loc_len)
                    r.yaw = (int)b[o] | ((int)b[o+1] << 8);
                r.valid = 1;
            }
        }
    }

    return r;
}

// ---------------------------------------------------------------------------
// Main frame
// ---------------------------------------------------------------------------

static void draw_frame(Server *s, volatile int *running, DB *db,
                       std::thread &srv_thread, bool &done) {
    // Local edit buffers — initialised once from db, then owned by GUI.
    static char s_name[DB_NAME_LEN]    = {};
    static char s_ip  [DB_IP_BIND_LEN] = {};
    static char s_port[8]              = {};
    static bool s_init                 = false;

    ImGuiIO &io = ImGui::GetIO();
    ImGui::SetNextWindowPos({0, 0});
    ImGui::SetNextWindowSize(io.DisplaySize);
    ImGui::Begin("##root", nullptr,
        ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoMove     | ImGuiWindowFlags_NoBringToFrontOnFocus);

    bool srv = g_srv_running->load();

    // Sync buffers from db on first run or after Stop (s_init reset to false)
    if (!s_init) {
        strncpy(s_name, db->config.name,    sizeof(s_name) - 1);
        strncpy(s_ip,   db->config.bind_ip, sizeof(s_ip)   - 1);
        snprintf(s_port, sizeof(s_port), "%d", db->config.port);
        s_init = true;
    }

    // ---- Top bar: config fields + Start/Stop ----
    // Fields are disabled while server is running; button is always active.
    float btn_w = 80.0f;
    float avail  = ImGui::GetContentRegionAvail().x;

    ImGui::BeginDisabled(srv);
    ImGui::SetNextItemWidth(180);
    if (ImGui::InputText("Name", s_name, sizeof(s_name),
                         ImGuiInputTextFlags_EnterReturnsTrue)) {
        strncpy(db->config.name, s_name, DB_NAME_LEN - 1);
        db_save(db);
    }
    ImGui::SameLine();
    ImGui::SetNextItemWidth(130);
    if (ImGui::InputText("Listen IP", s_ip, sizeof(s_ip),
                         ImGuiInputTextFlags_EnterReturnsTrue)) {
        strncpy(db->config.bind_ip, s_ip, DB_IP_BIND_LEN - 1);
        db_save(db);
    }
    ImGui::SameLine();
    ImGui::SetNextItemWidth(70);
    if (ImGui::InputText("Listen Port", s_port, sizeof(s_port),
                         ImGuiInputTextFlags_EnterReturnsTrue |
                         ImGuiInputTextFlags_CharsDecimal)) {
        db->config.port = (uint16_t)atoi(s_port);
        db_save(db);
    }
    ImGui::EndDisabled();

    // Start/Stop button — always enabled, placed at right edge of the same row
    ImGui::SameLine(avail - btn_w);
    if (srv) {
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.6f,0.1f,0.1f,1.f));
        if (ImGui::Button("Stop", {btn_w, 0})) {
            gui_server_stop(s, running, srv_thread);
            db_save(&s->db);
            s_init = false;
        }
        ImGui::PopStyleColor();
    } else {
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.1f,0.5f,0.1f,1.f));
        if (ImGui::Button("Start", {btn_w, 0})) {
            gui_server_start(s, running, db);
            srv_thread = std::thread(server_thread_fn, s, running);
        }
        ImGui::PopStyleColor();
    }

    // Status line
    if (srv) {
        int pcount = 0;
        for (int i = 0; i < MAX_CLIENTS; i++)
            if (s->players[i].used) pcount++;
        ImGui::Text("Players: %d   Rooms: %d", pcount, s->n_rooms);
    } else {
        ImGui::TextDisabled("Server stopped");
    }

    ImGui::Separator();

    if (ImGui::BeginTabBar("tabs")) {

        // ---- Log tab ----
        if (ImGui::BeginTabItem("Log")) {
            float footer_h = ImGui::GetFrameHeightWithSpacing() + 4;
            ImGui::BeginChild("log_scroll", {0, -footer_h}, false,
                              ImGuiWindowFlags_HorizontalScrollbar);
            {
                std::lock_guard<std::mutex> lk(*g_log_mutex);
                for (auto &line : *g_log_lines)
                    ImGui::TextUnformatted(line.c_str());
                if (g_log_scroll) {
                    ImGui::SetScrollHereY(1.0f);
                    g_log_scroll = false;
                }
            }
            ImGui::EndChild();
            ImGui::BeginDisabled(!srv);
            ImGui::SetNextItemWidth(-80);
            bool enter = ImGui::InputText("##cmd", g_cmd_buf, sizeof(g_cmd_buf),
                                          ImGuiInputTextFlags_EnterReturnsTrue);
            ImGui::SameLine();
            if ((ImGui::Button("Send") || enter) && srv)
                dispatch_cmd(s);
            ImGui::EndDisabled();
            ImGui::EndTabItem();
        }

        // ---- Players tab ----
        if (ImGui::BeginTabItem("Players")) {
            if (ImGui::BeginTable("players", 5,
                    ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg |
                    ImGuiTableFlags_ScrollY)) {
                ImGui::TableSetupColumn("ID",      ImGuiTableColumnFlags_WidthFixed, 40);
                ImGui::TableSetupColumn("Nick",    ImGuiTableColumnFlags_WidthStretch);
                ImGui::TableSetupColumn("IP",      ImGuiTableColumnFlags_WidthFixed, 130);
                ImGui::TableSetupColumn("Room",    ImGuiTableColumnFlags_WidthFixed, 90);
                ImGui::TableSetupColumn("Actions", ImGuiTableColumnFlags_WidthFixed, 180);
                ImGui::TableHeadersRow();
                if (srv) {
                    for (int i = 0; i < MAX_CLIENTS; i++) {
                        Player *p = &s->players[i];
                        if (!p->used) continue;
                        const char *room_code = (p->room_idx >= 0 && p->room_idx < s->n_rooms)
                                                ? s->rooms[p->room_idx].code : "?";
                        ImGui::TableNextRow();
                        ImGui::TableNextColumn(); ImGui::Text("%d", p->id);
                        ImGui::TableNextColumn(); ImGui::TextUnformatted(p->nick);
                        ImGui::TableNextColumn(); ImGui::TextUnformatted(p->ip);
                        ImGui::TableNextColumn(); ImGui::TextUnformatted(room_code);
                        ImGui::TableNextColumn();

                        ImGui::PushID(p->id);

                        // Kick
                        if (ImGui::SmallButton("Kick")) {
                            server_disconnect(s, p);
                            ImGui::PopID();
                            continue;
                        }
                        ImGui::SameLine();

                        // Ban — disconnect + add to global ban list
                        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.6f,0.1f,0.1f,1.f));
                        if (ImGui::SmallButton("Ban")) {
                            db_add_ban(&s->db, p->ip, p->nick, "GUI ban");
                            db_save(&s->db);
                            server_disconnect(s, p);
                            ImGui::PopStyleColor();
                            ImGui::PopID();
                            continue;
                        }
                        ImGui::PopStyleColor();
                        ImGui::SameLine();

                        // Untrust — remove from trusted list of their room
                        if (ImGui::SmallButton("Untrust")) {
                            if (p->room_idx >= 0 && p->room_idx < s->db.n_rooms) {
                                db_remove_trusted(&s->db.rooms[p->room_idx], p->ip);
                                db_save(&s->db);
                            }
                        }

                        ImGui::PopID();
                    }
                }
                ImGui::EndTable();
            }
            ImGui::EndTabItem();
        }

        // ---- Rooms tab ----
        if (ImGui::BeginTabItem("Rooms")) {
            // Add room row
            static char r_code[DB_ROOM_CODE_LEN] = {};
            static char r_pass[DB_PASSWORD_LEN]  = {};

            ImGui::SetNextItemWidth(160);
            ImGui::InputText("Code##new",     r_code, sizeof(r_code));
            ImGui::SameLine();
            ImGui::SetNextItemWidth(160);
            ImGui::InputText("Password##new", r_pass, sizeof(r_pass));
            ImGui::SameLine();
            if (ImGui::Button("Add Room") && r_code[0]) {
                db_add_room(db, r_code, r_pass);
                db_save(db);
                if (srv) {
                    db_add_room(&s->db, r_code, r_pass);
                    server_add_room(s, r_code, r_pass);
                }
                r_code[0] = '\0';
                r_pass[0] = '\0';
            }

            ImGui::Separator();

            // Editable rooms table — always 4 columns, Players only shown when srv
            if (ImGui::BeginTable("rooms", 4,
                    ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg |
                    ImGuiTableFlags_ScrollY)) {
                ImGui::TableSetupColumn("Code",     ImGuiTableColumnFlags_WidthFixed,   120);
                ImGui::TableSetupColumn("Password", ImGuiTableColumnFlags_WidthFixed,   120);
                ImGui::TableSetupColumn("Players",  ImGuiTableColumnFlags_WidthFixed,    60);
                ImGui::TableSetupColumn("##edit",   ImGuiTableColumnFlags_WidthStretch);
                ImGui::TableHeadersRow();

                // When server is running, source of truth is s->rooms / s->db.
                // When stopped, it's the external db pointer.
                DB   *live_db = srv ? &s->db : db;
                int   n       = srv ? s->n_rooms : db->n_rooms;

                static char edit_code[MAX_ROOMS][MAX_ROOM_CODE] = {};
                static char edit_pass[MAX_ROOMS][MAX_PASSWORD]   = {};
                static bool code_editing[MAX_ROOMS] = {};
                static bool pass_editing[MAX_ROOMS] = {};

                for (int i = 0; i < n; i++) {
                    Room   *sr = srv ? &s->rooms[i] : nullptr;
                    DBRoom *dr = (i < live_db->n_rooms) ? &live_db->rooms[i] : nullptr;

                    int  pcount     = (srv && sr) ? server_room_player_count(s, i) : 0;
                    bool no_players = (pcount == 0);

                    ImGui::PushID(i);
                    ImGui::TableNextRow();

                    // --- Code cell (editable only when no players in room) ---
                    ImGui::TableNextColumn();
                    if (!code_editing[i]) {
                        const char *cur = sr ? sr->code : (dr ? dr->code : "");
                        strncpy(edit_code[i], cur, MAX_ROOM_CODE - 1);
                    }
                    ImGui::BeginDisabled(!no_players);
                    ImGui::SetNextItemWidth(-1);
                    if (ImGui::InputText("##code", edit_code[i], MAX_ROOM_CODE,
                                         ImGuiInputTextFlags_EnterReturnsTrue)) {
                        if (edit_code[i][0]) {
                            if (sr) strncpy(sr->code, edit_code[i], MAX_ROOM_CODE - 1);
                            if (dr) strncpy(dr->code, edit_code[i], DB_ROOM_CODE_LEN - 1);
                            db_save(live_db);
                        }
                    }
                    code_editing[i] = ImGui::IsItemActive();
                    ImGui::EndDisabled();

                    // --- Password cell (always editable, masked unless hovered/active) ---
                    ImGui::TableNextColumn();
                    if (!pass_editing[i]) {
                        const char *cur = sr ? sr->password : (dr ? dr->password : "");
                        strncpy(edit_pass[i], cur, MAX_PASSWORD - 1);
                    }
                    {
                        // Show password masked unless hovered or active (one-frame lag is fine).
                        static bool pass_hovered[MAX_ROOMS] = {};
                        ImGuiInputTextFlags pass_flags = ImGuiInputTextFlags_EnterReturnsTrue;
                        if (!pass_hovered[i] && !pass_editing[i])
                            pass_flags |= ImGuiInputTextFlags_Password;
                        ImGui::SetNextItemWidth(-1);
                        if (ImGui::InputText("##pass", edit_pass[i], MAX_PASSWORD, pass_flags)) {
                            if (sr) strncpy(sr->password, edit_pass[i], MAX_PASSWORD - 1);
                            if (dr) strncpy(dr->password, edit_pass[i], DB_PASSWORD_LEN - 1);
                            db_save(live_db);
                        }
                        pass_editing[i] = ImGui::IsItemActive();
                        pass_hovered[i] = ImGui::IsItemHovered();
                    }

                    // --- Players cell ---
                    ImGui::TableNextColumn();
                    if (srv) ImGui::Text("%d", pcount);

                    // --- (reserved) ---
                    ImGui::TableNextColumn();

                    ImGui::PopID();
                }
                ImGui::EndTable();
            }
            ImGui::EndTabItem();
        }

        // ---- Graph tab ----
        if (ImGui::BeginTabItem("Graph")) {
            // --- Persistent state ---
            static ImVec2 g_pan           = {0.f, 0.f};
            static float  g_zoom          = 1.f;
            static int    g_drag_slot     = -1;    // slot being dragged
            static int    g_selected_slots[MAX_CLIENTS] = {}; // bitmask: 1=selected
            static ImVec2 g_click_pos  = {0.f, 0.f};
            static int    g_click_slot = -1;
            // overlay exclusion zones (union of all panels from last frame)
            static ImVec2 g_overlay_tl = {0.f, 0.f};
            static ImVec2 g_overlay_br = {0.f, 0.f};

            // Per-slot manual position offset (world units, added on top of orbit anchor)
            static ImVec2 g_node_offset[MAX_CLIENTS] = {};

            // Breath state per player slot (keyed by player array index)
            struct BreathState {
                float cur_x, cur_y;   // current world offset from anchor
                float tgt_x, tgt_y;   // target world offset from anchor
                float speed;           // units/sec
                bool  init;
            };
            static BreathState g_breath[MAX_CLIENTS] = {};

            // Enemy breath (keyed by s->enemies[] index)
            static BreathState g_enemy_breath[MAX_ENEMY_SNAPSHOTS] = {};
            // Selected enemy overlays: key = enemies[] index, value = 1 if selected
            static int g_selected_enemies[MAX_ENEMY_SNAPSHOTS] = {};
            static int    g_click_enemy = -1; // enemies[] index hit on LMB down
            static int    g_drag_enemy  = -1; // enemies[] index being dragged
            static ImVec2 g_enemy_offset[MAX_ENEMY_SNAPSHOTS] = {};

            // Sub-node: a packet travelling along an edge
            // Phase 0: sender → relay (t: 0→1)
            // Phase 1: relay → each receiver (t: 0→1, one sub per receiver)
            struct SubNode {
                bool  used;
                int   sender_slot;  // originating player slot
                int   recv_slot;    // target player slot (-1 = relay centre)
                float t;            // progress 0..1
                GuiPktCategory cat;
                ImU32 border_col;   // cached border color for flash
            };
            // Flash: brief highlight on a node when a sub-node arrives
            struct FlashNode {
                bool   used;
                int    slot;   // -1 = relay centre
                float  ttl;    // seconds remaining
                ImU32  color;  // border color tinted from packet type
            };
            static const int MAX_SUBNODES = 256;
            static const int MAX_FLASHES  = 64;
            static SubNode  g_sub[MAX_SUBNODES] = {};
            static FlashNode g_flash[MAX_FLASHES] = {};

            // LOC packet throttle: max 1 active sub-node per sender for LOC
            static int g_loc_active[MAX_CLIENTS] = {};  // count of active LOC subs per slot

            // Packet display speed (world-space travel per second — normalised to edge length)
            static const float SUB_SPEED    = 1.0f;
            static const float FLASH_TTL    = 0.35f;
            static const int   LOC_MAX_SAME = 1;    // max simultaneous LOC subs per sender

            // --- Canvas setup ---
            ImVec2 canvas_pos  = ImGui::GetCursorScreenPos();
            ImVec2 canvas_size = ImGui::GetContentRegionAvail();
            if (canvas_size.x < 1.f) canvas_size.x = 1.f;
            if (canvas_size.y < 1.f) canvas_size.y = 1.f;

            ImDrawList *dl = ImGui::GetWindowDrawList();
            dl->AddRectFilled(canvas_pos,
                {canvas_pos.x + canvas_size.x, canvas_pos.y + canvas_size.y},
                IM_COL32(28, 28, 32, 255));
            dl->AddRect(canvas_pos,
                {canvas_pos.x + canvas_size.x, canvas_pos.y + canvas_size.y},
                IM_COL32(70, 70, 80, 255));

            // Subtle grid
            {
                float gs = 40.f * g_zoom;
                ImVec2 origin = {canvas_pos.x + canvas_size.x * 0.5f + g_pan.x,
                                 canvas_pos.y + canvas_size.y * 0.5f + g_pan.y};
                float ox = fmodf(origin.x - canvas_pos.x, gs);
                float oy = fmodf(origin.y - canvas_pos.y, gs);
                for (float x = ox; x < canvas_size.x; x += gs)
                    dl->AddLine({canvas_pos.x + x, canvas_pos.y},
                                {canvas_pos.x + x, canvas_pos.y + canvas_size.y},
                                IM_COL32(45, 45, 52, 255));
                for (float y = oy; y < canvas_size.y; y += gs)
                    dl->AddLine({canvas_pos.x,                canvas_pos.y + y},
                                {canvas_pos.x + canvas_size.x, canvas_pos.y + y},
                                IM_COL32(45, 45, 52, 255));
            }

            // Invisible button for input
            ImGui::InvisibleButton("##graph_canvas", canvas_size,
                ImGuiButtonFlags_MouseButtonLeft | ImGuiButtonFlags_MouseButtonRight);
            bool canvas_hovered = ImGui::IsItemHovered();
            ImGuiIO &io2 = ImGui::GetIO();
            float dt = io2.DeltaTime;
            ImVec2 mouse = io2.MousePos;

            // Exclude overlay panel area (bounds from previous frame) from canvas interaction
            {
                bool over_overlay = (mouse.x >= g_overlay_tl.x && mouse.x <= g_overlay_br.x &&
                                     mouse.y >= g_overlay_tl.y && mouse.y <= g_overlay_br.y);
                if (over_overlay) canvas_hovered = false;
            }

            // Release node drag when LMB released
            if (!ImGui::IsMouseDown(ImGuiMouseButton_Left))
                g_drag_slot = -1;

            // RMB drag → pan
            if (canvas_hovered && ImGui::IsMouseDragging(ImGuiMouseButton_Right, 1.f)) {
                g_pan.x += io2.MouseDelta.x;
                g_pan.y += io2.MouseDelta.y;
            }

            // Zoom with scroll wheel — zoom towards cursor
            if (canvas_hovered && io2.MouseWheel != 0.f) {
                float f = io2.MouseWheel > 0.f ? 1.1f : (1.f / 1.1f);
                // World pos under cursor before zoom
                ImVec2 center = {canvas_pos.x + canvas_size.x * 0.5f + g_pan.x,
                                 canvas_pos.y + canvas_size.y * 0.5f + g_pan.y};
                float mwx = (io2.MousePos.x - center.x) / g_zoom;
                float mwy = (io2.MousePos.y - center.y) / g_zoom;
                float new_zoom = g_zoom * f;
                if (new_zoom < 0.5f) new_zoom = 0.5f;
                if (new_zoom > 2.f)  new_zoom = 2.f;
                // Adjust pan so cursor stays on same world point
                g_pan.x += mwx * (g_zoom - new_zoom);
                g_pan.y += mwy * (g_zoom - new_zoom);
                g_zoom = new_zoom;
            }

            // Middle-click or RMB click (no drag) → reset view
            if (canvas_hovered && ImGui::IsMouseClicked(ImGuiMouseButton_Middle))
                { g_pan = {0.f,0.f}; g_zoom = 1.f; }

            ImVec2 origin = {canvas_pos.x + canvas_size.x * 0.5f + g_pan.x,
                             canvas_pos.y + canvas_size.y * 0.5f + g_pan.y};
            auto w2s = [&](float wx, float wy) -> ImVec2 {
                return {origin.x + wx * g_zoom, origin.y + wy * g_zoom};
            };
            auto s2w = [&](float sx, float sy) -> ImVec2 {
                return {(sx - origin.x) / g_zoom, (sy - origin.y) / g_zoom};
            };

            // --- Collect players ---
            struct NodeInfo {
                int  id;
                char nick[MAX_NICK];
                char room[MAX_ROOM_CODE];
                char   ip[MAX_IP];
                int    slot;       // index in s->players[]
                double last_seen;  // mono_now() snapshot
            };
            NodeInfo nodes[MAX_CLIENTS];
            int n_nodes = 0;
            if (srv) {
                for (int i = 0; i < MAX_CLIENTS; i++) {
                    Player *p = &s->players[i];
                    if (!p->used) continue;
                    nodes[n_nodes].id        = p->id;
                    nodes[n_nodes].slot      = i;
                    nodes[n_nodes].last_seen = p->last_seen;
                    strncpy(nodes[n_nodes].nick, p->nick[0] ? p->nick : "?", MAX_NICK - 1);
                    strncpy(nodes[n_nodes].ip,   p->ip, MAX_IP - 1);
                    const char *rc = (p->room_idx >= 0 && p->room_idx < s->n_rooms)
                                     ? s->rooms[p->room_idx].code : "?";
                    strncpy(nodes[n_nodes].room, rc, MAX_ROOM_CODE - 1);
                    n_nodes++;
                }
            }

            // --- Constants ---
            const float ORBIT_R      = 380.f;  // player orbit radius
            const float ENEMY_R      = 120.f;  // enemy orbit radius around player node
            const float BREATH_R     = 14.f;
            const float BREATH_SPEED = 7.f;
            const float NODE_W       = 130.f;
            const float NODE_H       = 58.f;
            const float ENM_W        = 90.f;   // enemy node width
            const float ENM_H        = 38.f;   // enemy node height
            const float SRV_W        = 110.f;
            const float SRV_H        = 48.f;
            const float CORNER       = 6.f;
            const ImU32 COL_SRV_BG   = IM_COL32( 35,  70, 120, 255);
            const ImU32 COL_SRV_BD   = IM_COL32( 90, 160, 230, 255);
            const ImU32 COL_PLR_BG   = IM_COL32( 30,  80,  50, 255);
            const ImU32 COL_PLR_BD   = IM_COL32( 80, 180, 100, 255);
            const ImU32 COL_ENM_BG   = IM_COL32( 55,  20,  80, 255);
            const ImU32 COL_ENM_BD   = IM_COL32(170,  70, 230, 255);
            const ImU32 COL_HDR_ENM  = IM_COL32( 80,  30, 110, 220);
            const ImU32 COL_EDGE     = IM_COL32(100, 100, 120, 160);
            const ImU32 COL_ENM_EDGE = IM_COL32(120,  50, 160, 100);
            const ImU32 COL_TEXT     = IM_COL32(230, 230, 230, 255);
            const ImU32 COL_SUB      = IM_COL32(150, 170, 150, 255);
            const ImU32 COL_HDR      = IM_COL32( 55, 100, 170, 220);
            const ImU32 COL_HDR_PLR  = IM_COL32( 45, 110,  65, 220);

            // --- Update breath ---
            // Use a simple LCG seeded per slot for target selection
            auto pseudo_rand = [](unsigned &seed) -> float {
                seed = seed * 1664525u + 1013904223u;
                return (float)(seed >> 1) / (float)0x7fffffffu; // 0..1
            };
            for (int ni = 0; ni < n_nodes; ni++) {
                int slot = nodes[ni].slot;
                BreathState &b = g_breath[slot];
                if (!b.init) {
                    b.cur_x = b.cur_y = b.tgt_x = b.tgt_y = 0.f;
                    b.speed = BREATH_SPEED * (0.8f + (float)(slot % 5) * 0.1f);
                    b.init  = true;
                }
                // Move toward target
                float dx = b.tgt_x - b.cur_x;
                float dy = b.tgt_y - b.cur_y;
                float dist = sqrtf(dx*dx + dy*dy);
                float step = b.speed * dt;
                if (dist <= step || dist < 0.01f) {
                    // Reached target — pick new one from anchor
                    b.cur_x = b.tgt_x;
                    b.cur_y = b.tgt_y;
                    unsigned seed = (unsigned)(slot * 2654435761u + (unsigned)(ImGui::GetTime() * 97.f));
                    float angle = pseudo_rand(seed) * 6.2831853f;
                    float r     = pseudo_rand(seed) * BREATH_R;
                    b.tgt_x = cosf(angle) * r;
                    b.tgt_y = sinf(angle) * r;
                    b.speed = BREATH_SPEED * (0.7f + pseudo_rand(seed) * 0.6f);
                } else {
                    b.cur_x += dx / dist * step;
                    b.cur_y += dy / dist * step;
                }
            }

            // --- Update enemy breath ---
            if (srv) {
                for (int ei = 0; ei < MAX_ENEMY_SNAPSHOTS; ei++) {
                    EnemySnapshot *e = &s->enemies[ei];
                    if (!e->used) continue;
                    BreathState &b = g_enemy_breath[ei];
                    if (!b.init) {
                        b.cur_x = b.cur_y = b.tgt_x = b.tgt_y = 0.f;
                        b.speed = BREATH_SPEED * (0.6f + (float)(ei % 7) * 0.08f);
                        b.init  = true;
                    }
                    float dx = b.tgt_x - b.cur_x, dy = b.tgt_y - b.cur_y;
                    float dist = sqrtf(dx*dx + dy*dy);
                    float step = b.speed * dt;
                    if (dist <= step || dist < 0.01f) {
                        b.cur_x = b.tgt_x; b.cur_y = b.tgt_y;
                        unsigned seed = (unsigned)(ei * 2246822519u + (unsigned)(ImGui::GetTime() * 113.f));
                        float ang = pseudo_rand(seed) * 6.2831853f;
                        float r   = pseudo_rand(seed) * BREATH_R * 0.6f;
                        b.tgt_x = cosf(ang) * r; b.tgt_y = sinf(ang) * r;
                        b.speed = BREATH_SPEED * (0.5f + pseudo_rand(seed) * 0.5f);
                    } else {
                        b.cur_x += dx / dist * step;
                        b.cur_y += dy / dist * step;
                    }
                }
            }

            // Helper: compute player node world pos
            auto player_world = [&](int ni) -> ImVec2 {
                int slot  = nodes[ni].slot;
                float ang = (float)ni / (float)(n_nodes > 0 ? n_nodes : 1) * 6.2831853f;
                float ax  = ORBIT_R * cosf(ang) + g_node_offset[slot].x;
                float ay  = ORBIT_R * sinf(ang) + g_node_offset[slot].y;
                return {ax + g_breath[slot].cur_x, ay + g_breath[slot].cur_y};
            };

            // Helper: compute enemy node world pos (orbiting its owner player)
            // owner_ni = index in nodes[] for this enemy's owner, ei = enemies[] index
            auto enemy_world = [&](int owner_ni, int ei, int e_local_idx, int e_local_total) -> ImVec2 {
                ImVec2 pw = player_world(owner_ni);
                float ang = (float)e_local_idx / (float)(e_local_total > 0 ? e_local_total : 1) * 6.2831853f;
                float ex  = pw.x + ENEMY_R * cosf(ang) + g_enemy_offset[ei].x + g_enemy_breath[ei].cur_x;
                float ey  = pw.y + ENEMY_R * sinf(ang) + g_enemy_offset[ei].y + g_enemy_breath[ei].cur_y;
                return {ex, ey};
            };

            // Build per-player enemy list (only for current room)
            // enemies_for_player[ni] = list of enemies[] indices owned by nodes[ni].player_id
            struct EnemyList { int idx[64]; int count; };
            EnemyList enm_lists[MAX_CLIENTS] = {};
            if (srv) {
                for (int ei = 0; ei < MAX_ENEMY_SNAPSHOTS; ei++) {
                    EnemySnapshot *e = &s->enemies[ei];
                    if (!e->used) continue;
                    for (int ni = 0; ni < n_nodes; ni++) {
                        if (s->players[nodes[ni].slot].id == e->owner_player_id &&
                            s->players[nodes[ni].slot].room_idx == e->room_idx) {
                            EnemyList &el = enm_lists[ni];
                            if (el.count < 64) el.idx[el.count++] = ei;
                            break;
                        }
                    }
                }
            }

            // --- LMB: hit-test (enemy nodes first, then player nodes) ---
            if (ImGui::IsMouseClicked(ImGuiMouseButton_Left) && canvas_hovered) {
                g_drag_slot   = -1;
                g_drag_enemy  = -1;
                g_click_slot  = -1;
                g_click_enemy = -1;
                g_click_pos   = mouse;

                // Hit-test enemy nodes first (they're smaller, on top)
                bool hit_enemy = false;
                for (int ni = n_nodes - 1; ni >= 0 && !hit_enemy; ni--) {
                    EnemyList &el = enm_lists[ni];
                    for (int li = el.count - 1; li >= 0; li--) {
                        int ei = el.idx[li];
                        ImVec2 ew = enemy_world(ni, ei, li, el.count);
                        ImVec2 sc = w2s(ew.x, ew.y);
                        float hw = ENM_W * 0.5f * g_zoom, hh = ENM_H * 0.5f * g_zoom;
                        if (mouse.x >= sc.x - hw && mouse.x <= sc.x + hw &&
                            mouse.y >= sc.y - hh && mouse.y <= sc.y + hh) {
                            g_drag_enemy  = ei;
                            g_click_enemy = ei;
                            hit_enemy = true;
                            break;
                        }
                    }
                }

                // Hit-test player nodes
                if (!hit_enemy) {
                    for (int ni = n_nodes - 1; ni >= 0; ni--) {
                        int slot = nodes[ni].slot;
                        ImVec2 pw = player_world(ni);
                        ImVec2 sc = w2s(pw.x, pw.y);
                        float hw = NODE_W * 0.5f * g_zoom, hh = NODE_H * 0.5f * g_zoom;
                        if (mouse.x >= sc.x - hw && mouse.x <= sc.x + hw &&
                            mouse.y >= sc.y - hh && mouse.y <= sc.y + hh) {
                            g_drag_slot  = slot;
                            g_click_slot = slot;
                            break;
                        }
                    }
                }
            }
            if (g_drag_slot >= 0 && ImGui::IsMouseDragging(ImGuiMouseButton_Left, 0.f)) {
                g_node_offset[g_drag_slot].x += io2.MouseDelta.x / g_zoom;
                g_node_offset[g_drag_slot].y += io2.MouseDelta.y / g_zoom;
            }
            if (g_drag_enemy >= 0 && ImGui::IsMouseDragging(ImGuiMouseButton_Left, 0.f)) {
                g_enemy_offset[g_drag_enemy].x += io2.MouseDelta.x / g_zoom;
                g_enemy_offset[g_drag_enemy].y += io2.MouseDelta.y / g_zoom;
            }
            // On release: update selection
            if (ImGui::IsMouseReleased(ImGuiMouseButton_Left) && canvas_hovered) {
                float dx = mouse.x - g_click_pos.x;
                float dy = mouse.y - g_click_pos.y;
                if (dx*dx + dy*dy < 16.f) {
                    bool ctrl = io2.KeyCtrl;
                    if (g_click_enemy >= 0) {
                        // Enemy clicked
                        if (ctrl) {
                            g_selected_enemies[g_click_enemy] ^= 1;
                        } else {
                            for (int i = 0; i < MAX_ENEMY_SNAPSHOTS; i++) g_selected_enemies[i] = 0;
                            g_selected_enemies[g_click_enemy] = 1;
                        }
                    } else if (g_click_slot >= 0) {
                        // Player clicked
                        if (ctrl) {
                            g_selected_slots[g_click_slot] ^= 1;
                        } else {
                            for (int i = 0; i < MAX_CLIENTS; i++) g_selected_slots[i] = 0;
                            g_selected_slots[g_click_slot] = 1;
                        }
                    } else {
                        // Empty: clear all
                        for (int i = 0; i < MAX_CLIENTS; i++) g_selected_slots[i] = 0;
                        for (int i = 0; i < MAX_ENEMY_SNAPSHOTS; i++) g_selected_enemies[i] = 0;
                    }
                }
            }

            // Border color per packet category (shared between spawn and draw)
            auto cat_border = [](GuiPktCategory cat) -> ImU32 {
                switch (cat) {
                    case GUIPKT_LOC:      return IM_COL32( 80, 200,  80, 255);
                    case GUIPKT_SMT:      return IM_COL32(230, 190,  50, 255);
                    case GUIPKT_DOOR:     return IM_COL32( 80, 170, 240, 255);
                    case GUIPKT_RESPAWN:  return IM_COL32(240,  80,  80, 255);
                    case GUIPKT_ENPC_SMT: return IM_COL32(190,  80, 240, 255);
                    default:              return IM_COL32(160, 160, 160, 255);
                }
            };

            // Reset LOC counters for disconnected slots
            if (srv) {
                for (int i = 0; i < MAX_CLIENTS; i++)
                    if (!s->players[i].used && g_loc_active[i] > 0)
                        g_loc_active[i] = 0;
            } else {
                for (int i = 0; i < MAX_CLIENTS; i++) g_loc_active[i] = 0;
            }

            // --- Drain GUI event ring and spawn sub-nodes ---
            if (srv) {
                // Build slot→ni lookup
                int slot_to_ni[MAX_CLIENTS];
                for (int i = 0; i < MAX_CLIENTS; i++) slot_to_ni[i] = -1;
                for (int ni = 0; ni < n_nodes; ni++) slot_to_ni[nodes[ni].slot] = ni;

                GuiPacketEvent ev;
                while (gui_pop_event(s, &ev)) {
                    int sslot = ev.sender_slot;
                    if (sslot < 0 || sslot >= MAX_CLIENTS) continue;
                    if (slot_to_ni[sslot] < 0) continue; // sender not in current view

                    // LOC throttle: skip if already at cap
                    if (ev.category == GUIPKT_LOC && g_loc_active[sslot] >= LOC_MAX_SAME)
                        continue;

                    // Find a free sub-node slot for phase-0 (sender → relay)
                    for (int si = 0; si < MAX_SUBNODES; si++) {
                        if (!g_sub[si].used) {
                            g_sub[si].used        = true;
                            g_sub[si].sender_slot = sslot;
                            g_sub[si].recv_slot   = -1;
                            g_sub[si].t           = 0.f;
                            g_sub[si].cat         = ev.category;
                            g_sub[si].border_col  = cat_border(ev.category);
                            if (ev.category == GUIPKT_LOC) g_loc_active[sslot]++;
                            break;
                        }
                    }
                }
            }

            // --- Update sub-nodes ---
            {
                // Build slot→ni lookup (may be rebuilt, but keep it local)
                int slot_to_ni[MAX_CLIENTS];
                for (int i = 0; i < MAX_CLIENTS; i++) slot_to_ni[i] = -1;
                for (int ni = 0; ni < n_nodes; ni++) slot_to_ni[nodes[ni].slot] = ni;

                for (int si = 0; si < MAX_SUBNODES; si++) {
                    SubNode &sn = g_sub[si];
                    if (!sn.used) continue;

                    sn.t += SUB_SPEED * dt;
                    if (sn.t >= 1.f) {
                        sn.t = 0.f;
                        if (sn.recv_slot == -1) {
                            // Arrived at relay — spawn phase-1 subs to all other players
                            bool spawned = false;
                            for (int ni = 0; ni < n_nodes; ni++) {
                                int dslot = nodes[ni].slot;
                                if (dslot == sn.sender_slot) continue;
                                for (int si2 = 0; si2 < MAX_SUBNODES; si2++) {
                                    if (!g_sub[si2].used) {
                                        g_sub[si2].used        = true;
                                        g_sub[si2].sender_slot = sn.sender_slot;
                                        g_sub[si2].recv_slot   = dslot;
                                        g_sub[si2].t           = 0.f;
                                        g_sub[si2].cat         = sn.cat;
                                        g_sub[si2].border_col  = sn.border_col;
                                        spawned = true;
                                        break;
                                    }
                                }
                            }
                            // Flash relay node
                            for (int fi = 0; fi < MAX_FLASHES; fi++) {
                                if (!g_flash[fi].used) {
                                    g_flash[fi].used  = true;
                                    g_flash[fi].slot  = -1;
                                    g_flash[fi].ttl   = FLASH_TTL;
                                    g_flash[fi].color = sn.border_col;
                                    break;
                                }
                            }
                            if (sn.cat == GUIPKT_LOC) g_loc_active[sn.sender_slot]--;
                            sn.used = false;
                        } else {
                            // Arrived at receiver — flash that node
                            for (int fi = 0; fi < MAX_FLASHES; fi++) {
                                if (!g_flash[fi].used) {
                                    g_flash[fi].used  = true;
                                    g_flash[fi].slot  = sn.recv_slot;
                                    g_flash[fi].ttl   = FLASH_TTL;
                                    g_flash[fi].color = sn.border_col;
                                    break;
                                }
                            }
                            sn.used = false;
                        }
                    }
                }

                // Update flashes
                for (int fi = 0; fi < MAX_FLASHES; fi++) {
                    if (g_flash[fi].used) {
                        g_flash[fi].ttl -= dt;
                        if (g_flash[fi].ttl <= 0.f) g_flash[fi].used = false;
                    }
                }
            }

            // --- Draw edges first ---
            for (int ni = 0; ni < n_nodes; ni++) {
                int slot = nodes[ni].slot;
                float angle = (float)ni / (float)(n_nodes > 0 ? n_nodes : 1) * 6.2831853f;
                float ax    = ORBIT_R * cosf(angle) + g_node_offset[slot].x;
                float ay    = ORBIT_R * sinf(angle) + g_node_offset[slot].y;
                BreathState &b = g_breath[slot];
                float px = ax + b.cur_x, py = ay + b.cur_y;
                ImVec2 ps = w2s(px, py);
                ImVec2 ss = w2s(0.f, 0.f);
                dl->AddLine(ss, ps, COL_EDGE, 1.5f);
            }

            // --- Draw sub-nodes (packet travellers) ---
            {
                struct SubStyle { ImU32 bg; ImU32 border; ImU32 text; const char *label; };
                auto sub_style = [](GuiPktCategory cat) -> SubStyle {
                    switch (cat) {
                        case GUIPKT_LOC:
                            return { IM_COL32( 30,  90,  30, 230), IM_COL32( 80, 200,  80, 255),
                                     IM_COL32(180, 255, 180, 255), "LOC" };
                        case GUIPKT_SMT:
                            return { IM_COL32( 90,  70,  10, 230), IM_COL32(230, 190,  50, 255),
                                     IM_COL32(255, 230, 130, 255), "SMT" };
                        case GUIPKT_DOOR:
                            return { IM_COL32( 20,  60, 110, 230), IM_COL32( 80, 170, 240, 255),
                                     IM_COL32(160, 220, 255, 255), "DOOR" };
                        case GUIPKT_RESPAWN:
                            return { IM_COL32(100,  20,  20, 230), IM_COL32(240,  80,  80, 255),
                                     IM_COL32(255, 180, 180, 255), "LIFE" };
                        case GUIPKT_ENPC_SMT:
                            return { IM_COL32( 70,  20, 100, 230), IM_COL32(190,  80, 240, 255),
                                     IM_COL32(230, 180, 255, 255), "ESMT" };
                        default:
                            return { IM_COL32( 60,  60,  60, 200), IM_COL32(160, 160, 160, 255),
                                     IM_COL32(220, 220, 220, 255), "PKT" };
                    }
                };

                // Helper: world pos of a player node
                auto node_world = [&](int slot, int ni_hint) -> ImVec2 {
                    int ni = ni_hint;
                    if (ni < 0) {
                        for (int i = 0; i < n_nodes; i++)
                            if (nodes[i].slot == slot) { ni = i; break; }
                    }
                    if (ni < 0) return {0.f, 0.f};
                    float angle = (float)ni / (float)(n_nodes > 0 ? n_nodes : 1) * 6.2831853f;
                    float ax = ORBIT_R * cosf(angle) + g_node_offset[slot].x;
                    float ay = ORBIT_R * sinf(angle) + g_node_offset[slot].y;
                    return {ax + g_breath[slot].cur_x, ay + g_breath[slot].cur_y};
                };

                // Build slot→ni
                int slot_to_ni2[MAX_CLIENTS];
                for (int i = 0; i < MAX_CLIENTS; i++) slot_to_ni2[i] = -1;
                for (int ni = 0; ni < n_nodes; ni++) slot_to_ni2[nodes[ni].slot] = ni;

                ImFont *sfont   = ImGui::GetFont();
                float   sfs     = ImGui::GetFontSize() * g_zoom * 0.72f;
                if (sfs < 5.f)  sfs = 5.f;
                if (sfs > 14.f) sfs = 14.f;
                const float PAD = 3.f * g_zoom;

                for (int si = 0; si < MAX_SUBNODES; si++) {
                    SubNode &sn = g_sub[si];
                    if (!sn.used) continue;

                    ImVec2 from_w, to_w;
                    if (sn.recv_slot == -1) {
                        int ni = slot_to_ni2[sn.sender_slot];
                        if (ni < 0) continue;
                        from_w = node_world(sn.sender_slot, ni);
                        to_w   = {0.f, 0.f};
                    } else {
                        int ni = slot_to_ni2[sn.recv_slot];
                        if (ni < 0) continue;
                        from_w = {0.f, 0.f};
                        to_w   = node_world(sn.recv_slot, ni);
                    }

                    float t  = sn.t;
                    float wx = from_w.x + (to_w.x - from_w.x) * t;
                    float wy = from_w.y + (to_w.y - from_w.y) * t;
                    ImVec2 sc = w2s(wx, wy);

                    SubStyle ss = sub_style(sn.cat);
                    ImVec2 tsz = sfont->CalcTextSizeA(sfs, FLT_MAX, 0.f, ss.label);
                    float hw = tsz.x * 0.5f + PAD;
                    float hh = tsz.y * 0.5f + PAD * 0.6f;
                    ImVec2 tl = {sc.x - hw, sc.y - hh};
                    ImVec2 br = {sc.x + hw, sc.y + hh};
                    dl->AddRectFilled(tl, br, ss.bg,     2.f * g_zoom);
                    dl->AddRect      (tl, br, ss.border, 2.f * g_zoom, 0, 1.2f);
                    dl->AddText(sfont, sfs,
                                {sc.x - tsz.x * 0.5f, sc.y - tsz.y * 0.5f},
                                ss.text, ss.label);
                }

                // Flash overlays on nodes
                ImVec2 relay_s = w2s(0.f, 0.f);
                for (int fi = 0; fi < MAX_FLASHES; fi++) {
                    FlashNode &fl = g_flash[fi];
                    if (!fl.used) continue;
                    float alpha = fl.ttl / FLASH_TTL; // 1→0
                    // Tint the packet border color with alpha fade
                    ImU32 base = fl.color;
                    int   fa   = (int)(alpha * 180.f);
                    if (fa > 255) fa = 255;
                    ImU32 fcol = (base & 0x00FFFFFFu) | ((ImU32)fa << 24);

                    if (fl.slot == -1) {
                        // relay centre
                        float hr = (SRV_W * 0.5f + 4.f) * g_zoom;
                        dl->AddRect({relay_s.x - hr, relay_s.y - hr * 0.44f},
                                    {relay_s.x + hr, relay_s.y + hr * 0.44f},
                                    fcol, 6.f * g_zoom, 0, 3.f);
                    } else {
                        int ni = slot_to_ni2[fl.slot];
                        if (ni < 0) continue;
                        ImVec2 nw = node_world(fl.slot, ni);
                        ImVec2 ns = w2s(nw.x, nw.y);
                        float hw = (NODE_W * 0.5f + 4.f) * g_zoom;
                        float hh = (NODE_H * 0.5f + 4.f) * g_zoom;
                        dl->AddRect({ns.x - hw, ns.y - hh}, {ns.x + hw, ns.y + hh},
                                    fcol, 6.f * g_zoom, 0, 3.f);
                    }
                }
            }

            // Helper: draw a rounded rect node with header bar
            ImFont *font      = ImGui::GetFont();
            float   base_fs   = ImGui::GetFontSize();
            float   fs        = base_fs * g_zoom;
            float   fs_sub    = fs * 0.82f;
            // clamp so text doesn't go microscopic or huge
            if (fs     < 6.f)  fs     = 6.f;
            if (fs     > 32.f) fs     = 32.f;
            if (fs_sub < 5.f)  fs_sub = 5.f;
            if (fs_sub > 26.f) fs_sub = 26.f;

            // scaled CalcTextSize helper
            auto text_size = [&](const char *t, float size) -> ImVec2 {
                return font->CalcTextSizeA(size, FLT_MAX, 0.f, t);
            };

            auto draw_node = [&](float cx, float cy,
                                 float nw, float nh,
                                 ImU32 bg, ImU32 border, ImU32 hdr,
                                 const char *title,
                                 const char *line1, const char *line2)
            {
                ImVec2 c  = w2s(cx, cy);
                float  hw = nw * 0.5f * g_zoom;
                float  hh = nh * 0.5f * g_zoom;
                float  cr = CORNER * g_zoom;
                ImVec2 tl = {c.x - hw, c.y - hh};
                ImVec2 br = {c.x + hw, c.y + hh};
                float  hdr_h = (fs + 6.f * g_zoom);

                dl->AddRectFilled(tl, br, bg, cr);
                dl->AddRectFilled(tl, {br.x, tl.y + hdr_h}, hdr, cr);
                dl->AddRectFilled({tl.x, tl.y + hdr_h - cr}, {br.x, tl.y + hdr_h}, hdr);
                dl->AddRect(tl, br, border, cr, 0, 1.8f);

                // Title in header (scaled)
                ImVec2 tsz = text_size(title, fs);
                dl->AddText(font, fs,
                            {c.x - tsz.x * 0.5f, tl.y + (hdr_h - tsz.y) * 0.5f},
                            COL_TEXT, title);

                float text_y = tl.y + hdr_h + 3.f * g_zoom;
                if (line1) {
                    ImVec2 l1sz = text_size(line1, fs_sub);
                    dl->AddText(font, fs_sub, {c.x - l1sz.x * 0.5f, text_y}, COL_TEXT, line1);
                    text_y += l1sz.y + 2.f * g_zoom;
                }
                if (line2) {
                    ImVec2 l2sz = text_size(line2, fs_sub);
                    dl->AddText(font, fs_sub, {c.x - l2sz.x * 0.5f, text_y}, COL_SUB, line2);
                }
            };

            // --- Server node ---
            {
                const char *srv_label = srv ? s->name : "SERVER";
                char status[32];
                snprintf(status, sizeof(status), srv ? "online  %d players" : "offline", n_nodes);
                char port_str[16];
                snprintf(port_str, sizeof(port_str), ":%d", (int)s->port);
                draw_node(0.f, 0.f, SRV_W, SRV_H,
                          COL_SRV_BG, COL_SRV_BD, COL_HDR,
                          srv_label, status, port_str);
            }

            // --- Player nodes ---
            for (int ni = 0; ni < n_nodes; ni++) {
                int slot = nodes[ni].slot;
                float angle = (float)ni / (float)(n_nodes > 0 ? n_nodes : 1) * 6.2831853f;
                float ax    = ORBIT_R * cosf(angle) + g_node_offset[slot].x;
                float ay    = ORBIT_R * sinf(angle) + g_node_offset[slot].y;
                BreathState &b = g_breath[slot];
                float px = ax + b.cur_x, py = ay + b.cur_y;

                // Highlight border when dragging
                ImU32 bd = (g_drag_slot == slot) ? IM_COL32(200, 240, 140, 255) : COL_PLR_BD;

                char id_room[64];
                snprintf(id_room, sizeof(id_room), "#%d  [%s]", nodes[ni].id, nodes[ni].room);
                draw_node(px, py, NODE_W, NODE_H,
                          COL_PLR_BG, bd, COL_HDR_PLR,
                          nodes[ni].nick, nodes[ni].ip, id_room);

                // Timeout arc: drawn above the node, clockwise from top
                {
                    double elapsed = mono_now() - nodes[ni].last_seen;
                    float  frac    = (float)(elapsed / TIMEOUT_SEC); // 0=fresh, 1=timeout
                    if (frac < 0.f) frac = 0.f;
                    if (frac > 1.f) frac = 1.f;

                    // Only show arc when player has been silent for >10% of timeout
                    if (frac >= 0.1f) {
                        ImVec2 sc     = w2s(px, py);
                        float  radius = (NODE_W * 0.5f + 7.f) * g_zoom;
                        float  thick  = 3.f * g_zoom;
                        if (thick < 1.5f) thick = 1.5f;

                        // Background track (dark grey full circle)
                        dl->PathClear();
                        dl->PathArcTo(sc, radius, -1.5707963f, -1.5707963f + 6.2831853f, 40);
                        dl->PathStroke(IM_COL32(50, 50, 55, 180), false, thick);

                        // Filled arc — color lerp green→yellow→red
                        float sweep = frac * 6.2831853f;
                        ImU32 arc_col;
                        if (frac < 0.5f) {
                            // green→yellow
                            float t2 = frac * 2.f;
                            arc_col = IM_COL32(
                                (int)(80  + t2 * (220 - 80)),
                                (int)(200 - t2 * (200 - 190)),
                                (int)(60  - t2 * 60),
                                230);
                        } else {
                            // yellow→red
                            float t2 = (frac - 0.5f) * 2.f;
                            arc_col = IM_COL32(
                                (int)(220 + t2 * (240 - 220)),
                                (int)(190 - t2 * 190),
                                0,
                                230);
                        }

                        int segs = (int)(sweep / 6.2831853f * 40.f) + 2;
                        dl->PathClear();
                        dl->PathArcTo(sc, radius, -1.5707963f, -1.5707963f + sweep, segs);
                        dl->PathStroke(arc_col, false, thick);
                    }
                }
            }

            // --- Enemy edges (player→enemy, thin purple) ---
            if (srv) {
                for (int ni = 0; ni < n_nodes; ni++) {
                    EnemyList &el = enm_lists[ni];
                    ImVec2 pw = player_world(ni);
                    ImVec2 ps = w2s(pw.x, pw.y);
                    for (int li = 0; li < el.count; li++) {
                        int ei = el.idx[li];
                        ImVec2 ew = enemy_world(ni, ei, li, el.count);
                        ImVec2 es = w2s(ew.x, ew.y);
                        dl->AddLine(ps, es, COL_ENM_EDGE, 1.2f);
                    }
                }
            }

            // --- Enemy nodes ---
            if (srv) {
                for (int ni = 0; ni < n_nodes; ni++) {
                    EnemyList &el = enm_lists[ni];
                    for (int li = 0; li < el.count; li++) {
                        int ei = el.idx[li];
                        EnemySnapshot *e = &s->enemies[ei];
                        EnemySnapState es = decode_enemy_snap(e);

                        ImVec2 ew = enemy_world(ni, ei, li, el.count);
                        ImVec2 sc = w2s(ew.x, ew.y);
                        float  hw = ENM_W * 0.5f * g_zoom;
                        float  hh = ENM_H * 0.5f * g_zoom;
                        float  cr = 4.f * g_zoom;
                        ImVec2 tl = {sc.x - hw, sc.y - hh};
                        ImVec2 br = {sc.x + hw, sc.y + hh};

                        ImU32 bd = (g_drag_enemy == ei)        ? IM_COL32(200, 240, 140, 255) :
                                   g_selected_enemies[ei]      ? IM_COL32(220, 150, 255, 255) :
                                                                  COL_ENM_BD;
                        float hdr_h = (fs * 0.85f + 4.f * g_zoom);

                        dl->AddRectFilled(tl, br, COL_ENM_BG, cr);
                        dl->AddRectFilled(tl, {br.x, tl.y + hdr_h}, COL_HDR_ENM, cr);
                        dl->AddRectFilled({tl.x, tl.y + hdr_h - cr}, {br.x, tl.y + hdr_h}, COL_HDR_ENM);
                        dl->AddRect(tl, br, bd, cr, 0, 1.5f);

                        float  enm_fs = fs * 0.82f;
                        if (enm_fs < 5.f) enm_fs = 5.f;
                        if (enm_fs > 26.f) enm_fs = 26.f;

                        // Name in header
                        const char *ename = es.name[0] ? es.name : e->name;
                        ImVec2 nsz = font->CalcTextSizeA(enm_fs * 0.9f, FLT_MAX, 0.f, ename);
                        dl->AddText(font, enm_fs * 0.9f,
                                    {sc.x - nsz.x * 0.5f, tl.y + (hdr_h - nsz.y) * 0.5f},
                                    COL_TEXT, ename);

                        // SMT line below header
                        if (es.smt >= 0) {
                            char smt_buf[48];
                            snprintf(smt_buf, sizeof(smt_buf), "%s", smt_name(es.smt));
                            ImVec2 ssz = font->CalcTextSizeA(enm_fs * 0.78f, FLT_MAX, 0.f, smt_buf);
                            dl->AddText(font, enm_fs * 0.78f,
                                        {sc.x - ssz.x * 0.5f, tl.y + hdr_h + 2.f * g_zoom},
                                        IM_COL32(200, 160, 230, 220), smt_buf);
                        }
                    }
                }
            }

            // Bottom bar: zoom + hint
            {
                char zoom_str[32];
                snprintf(zoom_str, sizeof(zoom_str), "Zoom: %.0f%%", g_zoom * 100.f);
                ImVec2 zsz = ImGui::CalcTextSize(zoom_str);
                float  by  = canvas_pos.y + canvas_size.y - 18.f;
                dl->AddText({canvas_pos.x + 6.f, by},
                            IM_COL32(180, 180, 200, 220), zoom_str);
                dl->AddText({canvas_pos.x + zsz.x + 18.f, by},
                            IM_COL32(90, 90, 100, 180),
                            "RMB drag: pan   Scroll: zoom   MMB: reset   LMB node: move");
            }

            // --- Selected player info overlays (one per selected slot, stacked right→left) ---
            {
                const float OW     = 360.f;
                const float OPAD   = 10.f;  // top/bottom inner padding
                const float OBORD  = 1.2f;  // border stroke width
                const float OMARGIN= OPAD + OBORD; // bottom clearance below last row
                const float HDR_H  = 22.f;  // header text height
                const float ROW_H  = 17.f;
                const float OFS    = 14.f;
                const float VAL_X  = 160.f;
                const float GAP    = 6.f;

                // Panel height: top-pad + header + rows + bottom margin
                auto panel_h = [&](int n_rows) {
                    return OPAD + HDR_H + n_rows * ROW_H + OMARGIN;
                };

                ImFont *ofont = ImGui::GetFont();

                // Reset overlay exclusion zone; will be set to union of all panels
                float union_x0 = canvas_pos.x + canvas_size.x;
                float union_y0 = canvas_pos.y + canvas_size.y;
                float union_x1 = canvas_pos.x;
                float union_y1 = canvas_pos.y;
                bool any_panel = false;

                // Auto-clear disconnected slots
                if (srv) {
                    for (int i = 0; i < MAX_CLIENTS; i++)
                        if (g_selected_slots[i] && !s->players[i].used)
                            g_selected_slots[i] = 0;
                }

                // Draw panels right-to-left for each selected slot
                float ox = canvas_pos.x + canvas_size.x - GAP;
                for (int si = 0; si < MAX_CLIENTS; si++) {
                    if (!g_selected_slots[si]) continue;
                    if (!srv || !s->players[si].used) continue;

                    Player *sel = &s->players[si];
                    SnapState snap = {};
                    if (sel->state_snap_len > 0)
                        snap = decode_snap((const uint8_t *)sel->state_snap, sel->state_snap_len);

                    // 21 fixed rows + 2 if parrying; 1 row if no snapshot yet
                    int N_ROWS = snap.valid
                        ? (21 + (snap.parrying ? 2 : 0))
                        : 1;
                    float oh = panel_h(N_ROWS);
                    ox -= OW;
                    float oy = canvas_pos.y + GAP;

                    ImVec2 otl = {ox, oy};
                    ImVec2 obr = {ox + OW, oy + oh};

                    // Update exclusion union
                    if (otl.x < union_x0) union_x0 = otl.x;
                    if (otl.y < union_y0) union_y0 = otl.y;
                    if (obr.x > union_x1) union_x1 = obr.x;
                    if (obr.y > union_y1) union_y1 = obr.y;
                    any_panel = true;

                    dl->AddRectFilled(otl, obr, IM_COL32(18, 22, 30, 220), 6.f);
                    dl->AddRect      (otl, obr, IM_COL32(80, 120, 180, 200), 6.f, 0, 1.2f);

                    float cx = ox + OPAD;
                    float cy = oy + OPAD;

                    char hdr[64];
                    snprintf(hdr, sizeof(hdr), "%s  #%d", sel->nick[0] ? sel->nick : "?", sel->id);
                    dl->AddText(ofont, 16.f, {cx, cy}, IM_COL32(120, 200, 255, 255), hdr);
                    cy += 22.f;

                    if (!snap.valid) {
                        dl->AddText(ofont, OFS, {cx, cy}, IM_COL32(150,150,160,200), "No snapshot yet");
                    } else {
                        const ImU32 CLB = IM_COL32(130, 150, 170, 200);
                        const ImU32 CVL = IM_COL32(220, 230, 240, 255);
                        const ImU32 CVB = IM_COL32(100, 220, 120, 255);
                        const ImU32 CVF = IM_COL32(200, 90,  90, 255);

                        auto row = [&](const char *label, const char *val, ImU32 vcol) {
                            dl->AddText(ofont, OFS, {cx, cy},         CLB, label);
                            dl->AddText(ofont, OFS, {cx + VAL_X, cy}, vcol, val);
                            cy += ROW_H;
                        };
                        auto rowf = [&](const char *label, float v, const char *fmt = "%.2f") {
                            char buf[32]; snprintf(buf, sizeof(buf), fmt, v);
                            row(label, buf, CVL);
                        };
                        auto rowb = [&](const char *label, int v) {
                            row(label, v ? "true" : "false", v ? CVB : CVF);
                        };
                        auto rowi = [&](const char *label, int v) {
                            char buf[16]; snprintf(buf, sizeof(buf), "%d", v);
                            row(label, buf, CVL);
                        };

                        { char buf[48]; snprintf(buf, sizeof(buf), "%.0f  %.0f  %.0f",
                            snap.loc_x, snap.loc_y, snap.loc_z); row("Location", buf, CVL); }
                        { char buf[32]; snprintf(buf, sizeof(buf), "P%d  Y%d",
                            snap.pitch, snap.yaw); row("Rotation", buf, CVL); }
                        { char buf[48]; snprintf(buf, sizeof(buf), "%.0f  %.0f  %.0f",
                            snap.vel_x, snap.vel_y, snap.vel_z); row("Velocity", buf, CVL); }

                        rowi("Health",         snap.health);
                        row("LocomotionMode",  loco_name(snap.loc_mode), CVL);
                        row("SpecialMove",     smt_name(snap.smt), CVL);
                        rowi("CamcorderState", snap.cam_state);
                        rowi("EyeYaw",         snap.eye_yaw);
                        rowb("Crouched",       snap.crouched);
                        rowb("Ghost",          snap.is_ghost);
                        rowb("InDarkness",     snap.in_dark);
                        rowb("HeatShield",     snap.heat_shield);
                        rowb("Hobbling",       snap.hobbling);
                        rowb("Limping",        snap.limping);
                        rowb("CamVisible",     snap.cam_vis);
                        rowb("LeftAnim",       snap.left_anim);
                        rowf("HeatDist",       snap.heat_dist,    "%.1f");
                        rowf("Lean",           snap.lean,         "%.3f");
                        rowf("PeekRatio",      snap.peek_ratio,   "%.3f");
                        rowf("CornerIK",       snap.corner_ik,    "%.3f");
                        rowb("Parrying",       snap.parrying);
                        if (snap.parrying) {
                            rowf("ParryDist",  snap.parry_dist, "%.0f");
                            rowf("ParryYaw",   snap.parry_yaw,  "%.0f");
                        }
                    }
                    ox -= GAP; // gap between panels
                }

                // --- Enemy overlays (stacked left of player overlays) ---
                if (srv) {
                    for (int ei = 0; ei < MAX_ENEMY_SNAPSHOTS; ei++) {
                        if (!g_selected_enemies[ei]) continue;
                        EnemySnapshot *e = &s->enemies[ei];
                        if (!e->used) {
                            g_selected_enemies[ei] = 0;
                            g_enemy_offset[ei]     = {0.f, 0.f};
                            if (g_drag_enemy == ei) g_drag_enemy = -1;
                            continue;
                        }

                        EnemySnapState es = decode_enemy_snap(e);

                        // Location + Yaw + Weapon (3 fixed) + Class? + SMT?
                        int ENM_ROWS = es.valid
                            ? (3 + (es.cls[0] ? 1 : 0) + (es.smt >= 0 ? 1 : 0))
                            : 1;
                        float eoh = panel_h(ENM_ROWS);
                        ox -= OW;
                        float eoy = canvas_pos.y + GAP;

                        ImVec2 etl = {ox, eoy};
                        ImVec2 ebr = {ox + OW, eoy + eoh};

                        if (etl.x < union_x0) union_x0 = etl.x;
                        if (etl.y < union_y0) union_y0 = etl.y;
                        if (ebr.x > union_x1) union_x1 = ebr.x;
                        if (ebr.y > union_y1) union_y1 = ebr.y;
                        any_panel = true;

                        dl->AddRectFilled(etl, ebr, IM_COL32(22, 12, 32, 220), 6.f);
                        dl->AddRect      (etl, ebr, IM_COL32(140, 60, 200, 200), 6.f, 0, 1.2f);

                        float ecx = ox + OPAD;
                        float ecy = eoy + OPAD;

                        // Header: enemy name + owner
                        char ehdr[80];
                        const char *ename = es.name[0] ? es.name : e->name;
                        snprintf(ehdr, sizeof(ehdr), "%s", ename);
                        dl->AddText(ofont, 16.f, {ecx, ecy}, IM_COL32(200, 140, 255, 255), ehdr);
                        ecy += 22.f;

                        const ImU32 ECLB = IM_COL32(150, 120, 180, 200);
                        const ImU32 ECVL = IM_COL32(220, 200, 240, 255);

                        auto erow = [&](const char *label, const char *val, ImU32 vcol) {
                            dl->AddText(ofont, OFS, {ecx, ecy},         ECLB, label);
                            dl->AddText(ofont, OFS, {ecx + VAL_X, ecy}, vcol, val);
                            ecy += ROW_H;
                        };

                        if (!es.valid) {
                            erow("No data", "", ECLB);
                        } else {
                            char buf[64];
                            snprintf(buf, sizeof(buf), "%.0f  %.0f  %.0f", es.x, es.y, es.z);
                            erow("Location", buf, ECVL);
                            snprintf(buf, sizeof(buf), "%d", es.yaw);
                            erow("Yaw", buf, ECVL);
                            erow("Weapon", weapon_name(es.weapon), ECVL);
                            if (es.cls[0]) erow("Class", es.cls, ECVL);
                            if (es.smt >= 0) erow("SpecialMove", smt_name(es.smt), ECVL);
                        }
                        ox -= GAP;
                    }
                }

                if (any_panel) {
                    g_overlay_tl = {union_x0, union_y0};
                    g_overlay_br = {union_x1, union_y1};
                } else {
                    g_overlay_tl = {0.f, 0.f};
                    g_overlay_br = {0.f, 0.f};
                }
            }

            ImGui::EndTabItem();
        }

        // ---- Map tab ----
        if (ImGui::BeginTabItem("Map")) {
            // --- Persistent state ---
            static ImVec2 mp_pan  = {0.f, 0.f};
            static float  mp_zoom = 0.05f; // world coords are large (UU), scale way down
            // Selected node: category + index into its array
            enum MapNodeCat { MNC_NONE=-1, MNC_PLAYER=0, MNC_ENEMY=1, MNC_DOOR=2,
                              MNC_PUSHABLE=3, MNC_PICKUP=4 };
            static MapNodeCat mp_sel_cat = MNC_NONE;
            static int        mp_sel_idx = -1;

            // --- Canvas setup ---
            ImVec2 mp_cpos  = ImGui::GetCursorScreenPos();
            ImVec2 mp_csize = ImGui::GetContentRegionAvail();
            if (mp_csize.x < 1.f) mp_csize.x = 1.f;
            if (mp_csize.y < 1.f) mp_csize.y = 1.f;
            // Reserve bottom bar
            const float MAP_BOTTOM_H = 20.f;
            ImVec2 mp_draw_size = {mp_csize.x, mp_csize.y - MAP_BOTTOM_H};
            if (mp_draw_size.y < 1.f) mp_draw_size.y = 1.f;

            ImDrawList *mdl = ImGui::GetWindowDrawList();

            // Background
            mdl->AddRectFilled(mp_cpos,
                {mp_cpos.x + mp_csize.x, mp_cpos.y + mp_csize.y},
                IM_COL32(22, 22, 28, 255));
            mdl->AddRect(mp_cpos,
                {mp_cpos.x + mp_csize.x, mp_cpos.y + mp_csize.y},
                IM_COL32(60, 60, 72, 255));

            // Grid
            {
                float gs = 40.f * mp_zoom * 1000.f; // grid cell every 1000 UU
                // clamp grid density
                while (gs < 20.f) gs *= 5.f;
                while (gs > 200.f) gs /= 5.f;
                ImVec2 origin = {mp_cpos.x + mp_draw_size.x * 0.5f + mp_pan.x,
                                 mp_cpos.y + mp_draw_size.y * 0.5f + mp_pan.y};
                float ox = fmodf(origin.x - mp_cpos.x, gs);
                float oy = fmodf(origin.y - mp_cpos.y, gs);
                for (float x = ox; x < mp_draw_size.x; x += gs)
                    mdl->AddLine({mp_cpos.x + x, mp_cpos.y},
                                 {mp_cpos.x + x, mp_cpos.y + mp_draw_size.y},
                                 IM_COL32(38, 38, 48, 255));
                for (float y = oy; y < mp_draw_size.y; y += gs)
                    mdl->AddLine({mp_cpos.x,                  mp_cpos.y + y},
                                 {mp_cpos.x + mp_draw_size.x, mp_cpos.y + y},
                                 IM_COL32(38, 38, 48, 255));
                // Axis cross
                mdl->AddLine({origin.x, mp_cpos.y}, {origin.x, mp_cpos.y + mp_draw_size.y},
                             IM_COL32(55, 55, 70, 200));
                mdl->AddLine({mp_cpos.x, origin.y}, {mp_cpos.x + mp_draw_size.x, origin.y},
                             IM_COL32(55, 55, 70, 200));
            }

            // Invisible input button
            ImGui::InvisibleButton("##map_canvas", mp_draw_size,
                ImGuiButtonFlags_MouseButtonLeft | ImGuiButtonFlags_MouseButtonRight);
            bool mp_hov = ImGui::IsItemHovered();
            ImGuiIO &mio = ImGui::GetIO();
            ImVec2   mmouse = mio.MousePos;

            // Overlay panel bounds from previous frame (exclusion zone)
            static ImVec2 mp_overlay_tl = {0.f, 0.f};
            static ImVec2 mp_overlay_br = {0.f, 0.f};
            {
                bool over = (mmouse.x >= mp_overlay_tl.x && mmouse.x <= mp_overlay_br.x &&
                             mmouse.y >= mp_overlay_tl.y && mmouse.y <= mp_overlay_br.y);
                if (over) mp_hov = false;
            }

            // Pan (RMB drag)
            if (mp_hov && ImGui::IsMouseDragging(ImGuiMouseButton_Right, 1.f)) {
                mp_pan.x += mio.MouseDelta.x;
                mp_pan.y += mio.MouseDelta.y;
            }
            // Zoom (scroll)
            if (mp_hov && mio.MouseWheel != 0.f) {
                float f = mio.MouseWheel > 0.f ? 1.15f : (1.f / 1.15f);
                // World pos under cursor before zoom (Y flipped: world_y = -(screen_y - origin_y) / zoom)
                ImVec2 cur_origin = {mp_cpos.x + mp_draw_size.x * 0.5f + mp_pan.x,
                                     mp_cpos.y + mp_draw_size.y * 0.5f + mp_pan.y};
                float mwx =  (mio.MousePos.x - cur_origin.x) / mp_zoom;
                float mwy = -(mio.MousePos.y - cur_origin.y) / mp_zoom;
                float new_zoom = mp_zoom * f;
                if (new_zoom < 0.001f) new_zoom = 0.001f;
                if (new_zoom > 5.f)    new_zoom = 5.f;
                // Adjust pan so cursor stays on same world point
                mp_pan.x += mwx * (mp_zoom - new_zoom);
                mp_pan.y -= mwy * (mp_zoom - new_zoom); // Y flipped
                mp_zoom = new_zoom;
            }
            // Reset (MMB)
            if (mp_hov && ImGui::IsMouseClicked(ImGuiMouseButton_Middle))
                { mp_pan = {0.f, 0.f}; mp_zoom = 0.05f; }

            // World → screen helpers (Y flipped: UE +Y = up on screen = -Y)
            ImVec2 mp_origin = {mp_cpos.x + mp_draw_size.x * 0.5f + mp_pan.x,
                                mp_cpos.y + mp_draw_size.y * 0.5f + mp_pan.y};
            auto mw2s = [&](float wx, float wy) -> ImVec2 {
                return {mp_origin.x + wx * mp_zoom, mp_origin.y - wy * mp_zoom};
            };

            // Node radii per category
            const float MR_PLR  = 8.f;
            const float MR_ENM  = 5.f;
            const float MR_DOOR = 5.f;
            const float MR_PUSH = 4.f;
            const float MR_PICK = 4.f;

            // Colors per category  [fill, border]
            const ImU32 MC_PLR_F  = IM_COL32( 60, 180,  80, 210);
            const ImU32 MC_PLR_B  = IM_COL32(120, 255, 140, 255);
            const ImU32 MC_ENM_F  = IM_COL32(120,  30, 180, 210);
            const ImU32 MC_ENM_B  = IM_COL32(200,  80, 255, 255);
            const ImU32 MC_DOOR_F = IM_COL32( 40, 110, 220, 210);
            const ImU32 MC_DOOR_B = IM_COL32(100, 170, 255, 255);
            const ImU32 MC_PUSH_F = IM_COL32(200, 130,  30, 210);
            const ImU32 MC_PUSH_B = IM_COL32(255, 190,  80, 255);
            const ImU32 MC_PICK_F = IM_COL32(200,  60,  60, 210);
            const ImU32 MC_PICK_B = IM_COL32(255, 120, 100, 255);
            const ImU32 MC_SEL_B  = IM_COL32(255, 240, 100, 255); // selected border

            // Helper: draw a map node circle, returns true if LMB clicked on it
            // Uses draw list clip rect (mp_cpos .. mp_cpos+mp_draw_size) implicitly.
            auto draw_map_node = [&](float wx, float wy, float r,
                                     ImU32 fill, ImU32 border, bool selected) -> bool
            {
                ImVec2 sc = mw2s(wx, wy);
                // Clip: skip if outside canvas
                if (sc.x < mp_cpos.x - r || sc.x > mp_cpos.x + mp_draw_size.x + r) return false;
                if (sc.y < mp_cpos.y - r || sc.y > mp_cpos.y + mp_draw_size.y + r) return false;
                float sr = r; // screen radius fixed (independent of zoom for readability)
                ImU32 bord = selected ? MC_SEL_B : border;
                mdl->AddCircleFilled(sc, sr, fill);
                mdl->AddCircle(sc, sr, bord, 0, selected ? 2.f : 1.4f);

                // Hit test on LMB click
                if (mp_hov && ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
                    float dx = mmouse.x - sc.x, dy = mmouse.y - sc.y;
                    if (dx*dx + dy*dy <= (sr + 4.f) * (sr + 4.f))
                        return true;
                }
                return false;
            };

            // Draw clip rect so nodes don't spill outside canvas
            mdl->PushClipRect(mp_cpos, {mp_cpos.x + mp_draw_size.x, mp_cpos.y + mp_draw_size.y}, true);

            if (srv) {
                // --- Doors ---
                for (int i = 0; i < MAX_DOOR_SNAPSHOTS; i++) {
                    DoorSnapshot *d = &s->doors[i];
                    if (!d->used) continue;
                    // Key is "X,Y,Z"
                    int kx = 0, ky = 0, kz = 0;
                    sscanf(d->key, "%d,%d,%d", &kx, &ky, &kz);
                    bool sel = (mp_sel_cat == MNC_DOOR && mp_sel_idx == i);
                    if (draw_map_node((float)kx, (float)ky, MR_DOOR, MC_DOOR_F, MC_DOOR_B, sel)) {
                        mp_sel_cat = MNC_DOOR; mp_sel_idx = i;
                    }
                }
                // --- Pushables ---
                for (int i = 0; i < MAX_PUSH_SNAPSHOTS; i++) {
                    PushSnapshot *ps = &s->pushables[i];
                    if (!ps->used) continue;
                    int kx = 0, ky = 0, kz = 0;
                    sscanf(ps->key, "%d,%d,%d", &kx, &ky, &kz);
                    bool sel = (mp_sel_cat == MNC_PUSHABLE && mp_sel_idx == i);
                    if (draw_map_node((float)kx, (float)ky, MR_PUSH, MC_PUSH_F, MC_PUSH_B, sel)) {
                        mp_sel_cat = MNC_PUSHABLE; mp_sel_idx = i;
                    }
                }
                // --- Pickups ---
                for (int i = 0; i < MAX_PICKUP_SNAPSHOTS; i++) {
                    PickupSnapshot *pk = &s->pickups[i];
                    if (!pk->used) continue;
                    // Key may be "X,Y,Z" or path string; try to parse coords
                    int kx = 0, ky = 0, kz = 0;
                    bool has_coords = (sscanf(pk->key, "%d,%d,%d", &kx, &ky, &kz) == 3);
                    if (!has_coords) continue; // path-keyed pickups have no map position
                    bool sel = (mp_sel_cat == MNC_PICKUP && mp_sel_idx == i);
                    if (draw_map_node((float)kx, (float)ky, MR_PICK, MC_PICK_F, MC_PICK_B, sel)) {
                        mp_sel_cat = MNC_PICKUP; mp_sel_idx = i;
                    }
                }
                // --- Enemies ---
                for (int i = 0; i < MAX_ENEMY_SNAPSHOTS; i++) {
                    EnemySnapshot *e = &s->enemies[i];
                    if (!e->used) continue;
                    EnemySnapState es = decode_enemy_snap(e);
                    if (!es.valid) continue;
                    bool sel = (mp_sel_cat == MNC_ENEMY && mp_sel_idx == i);
                    if (draw_map_node(es.x, es.y, MR_ENM, MC_ENM_F, MC_ENM_B, sel)) {
                        mp_sel_cat = MNC_ENEMY; mp_sel_idx = i;
                    }
                }
                // --- Players ---
                for (int i = 0; i < MAX_CLIENTS; i++) {
                    Player *p = &s->players[i];
                    if (!p->used) continue;
                    if (p->state_snap_len <= 0) continue;
                    SnapState ss = decode_snap((const uint8_t*)p->state_snap, p->state_snap_len);
                    if (!ss.valid) continue;
                    bool sel = (mp_sel_cat == MNC_PLAYER && mp_sel_idx == i);
                    if (draw_map_node(ss.loc_x, ss.loc_y, MR_PLR, MC_PLR_F, MC_PLR_B, sel)) {
                        mp_sel_cat = MNC_PLAYER; mp_sel_idx = i;
                    }
                    // Vision cone — two rays from node centre, ±30° around yaw
                    {
                        ImVec2 sc = mw2s(ss.loc_x, ss.loc_y);
                        // UE yaw: uint16 0=East(+X), 16384=North(+Y); screen Y is flipped
                        float yaw_rad = (float)ss.yaw * (2.f * 3.14159265f / 65536.f);
                        float cone_len = 40.f + mp_zoom * 400.f; // world ~400-800 UU
                        if (cone_len > 120.f) cone_len = 120.f;
                        const float half_angle = 3.14159265f / 6.f; // 30 degrees
                        // Left and right rays
                        for (int side = -1; side <= 1; side += 2) {
                            float a = yaw_rad + side * half_angle;
                            // screen: +x = East, +y = South (Y flipped from world)
                            float dx = cosf(a) * cone_len;
                            float dy = -sinf(a) * cone_len; // flip Y for screen
                            mdl->AddLine(sc, {sc.x + dx, sc.y + dy},
                                         IM_COL32(180, 255, 180, 100), 1.0f);
                        }
                        // Fill triangle
                        {
                            float a0 = yaw_rad - half_angle;
                            float a1 = yaw_rad + half_angle;
                            ImVec2 p0 = {sc.x + cosf(a0) * cone_len, sc.y - sinf(a0) * cone_len};
                            ImVec2 p1 = {sc.x + cosf(a1) * cone_len, sc.y - sinf(a1) * cone_len};
                            mdl->AddTriangleFilled(sc, p0, p1, IM_COL32(180, 255, 180, 30));
                        }
                    }
                    // Nick label above node
                    if (mp_zoom > 0.01f) {
                        ImVec2 sc = mw2s(ss.loc_x, ss.loc_y);
                        const char *nick = p->nick[0] ? p->nick : "?";
                        ImVec2 tsz = ImGui::CalcTextSize(nick);
                        mdl->AddText({sc.x - tsz.x * 0.5f, sc.y - MR_PLR - tsz.y - 2.f},
                                     IM_COL32(180, 255, 180, 200), nick);
                    }
                }
            }

            // After PopClipRect: if LMB was clicked on canvas but no node was hit
            // (draw_map_node would have set a new sel_cat), clear the selection.
            // We detect "no hit" by comparing sel_cat to what it was when we entered
            // the draw block — tracked via mp_pre_click_cat/idx set each frame.
            static MapNodeCat mp_pre_cat = MNC_NONE;
            static int        mp_pre_idx = -1;

            mdl->PopClipRect();

            if (mp_hov && ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
                // If selection didn't change from what it was at start of draw,
                // the click was on empty space → deselect.
                if (mp_sel_cat == mp_pre_cat && mp_sel_idx == mp_pre_idx) {
                    mp_sel_cat = MNC_NONE;
                    mp_sel_idx = -1;
                }
            }
            // Save current selection for next frame's comparison
            mp_pre_cat = mp_sel_cat;
            mp_pre_idx = mp_sel_idx;

            // --- Legend (top-left corner) ---
            {
                float lx = mp_cpos.x + 8.f;
                float ly = mp_cpos.y + 8.f;
                struct LegEntry { const char *label; ImU32 fill; ImU32 bord; };
                LegEntry leg[] = {
                    {"Player",   MC_PLR_F,  MC_PLR_B  },
                    {"Enemy",    MC_ENM_F,  MC_ENM_B  },
                    {"Door",     MC_DOOR_F, MC_DOOR_B },
                    {"Pushable", MC_PUSH_F, MC_PUSH_B },
                    {"Pickup",   MC_PICK_F, MC_PICK_B },
                };
                for (auto &le : leg) {
                    mdl->AddCircleFilled({lx + 6.f, ly + 6.f}, 5.f, le.fill);
                    mdl->AddCircle      ({lx + 6.f, ly + 6.f}, 5.f, le.bord, 0, 1.2f);
                    mdl->AddText({lx + 15.f, ly}, IM_COL32(190, 190, 200, 220), le.label);
                    ly += 16.f;
                }
            }

            // --- Info overlay for selected node (top-right) ---
            {
                const float OW   = 320.f;
                const float OPAD = 8.f;
                const float ROW  = 16.f;
                const float OFS  = 13.f;

                // Collect rows
                char hdr_text[64]    = {};
                char rows[32][2][80] = {};
                int  n_rows          = 0;
                ImU32 hdr_col        = IM_COL32(200, 200, 200, 255);
                bool  has_info       = false;
                ImU32 panel_border   = IM_COL32(100, 100, 130, 200);

                auto addrow = [&](const char *label, const char *val) {
                    if (n_rows >= 32) return;
                    strncpy(rows[n_rows][0], label, 79);
                    strncpy(rows[n_rows][1], val,   79);
                    n_rows++;
                };
                auto addrowf = [&](const char *label, float v, const char *fmt = "%.1f") {
                    char buf[32]; snprintf(buf, sizeof(buf), fmt, v);
                    addrow(label, buf);
                };
                auto addrowi = [&](const char *label, int v) {
                    char buf[16]; snprintf(buf, sizeof(buf), "%d", v);
                    addrow(label, buf);
                };

                if (srv && mp_sel_cat == MNC_PLAYER && mp_sel_idx >= 0) {
                    Player *p = &s->players[mp_sel_idx];
                    if (p->used) {
                        has_info   = true;
                        hdr_col    = MC_PLR_B;
                        panel_border = IM_COL32(80, 180, 100, 200);
                        snprintf(hdr_text, sizeof(hdr_text), "Player: %s  #%d",
                                 p->nick[0] ? p->nick : "?", p->id);
                        addrow("IP", p->ip);
                        const char *rc = (p->room_idx >= 0 && p->room_idx < s->n_rooms)
                                          ? s->rooms[p->room_idx].code : "?";
                        addrow("Room", rc);
                        if (p->state_snap_len > 0) {
                            SnapState ss = decode_snap((const uint8_t*)p->state_snap, p->state_snap_len);
                            if (ss.valid) {
                                char buf[64];
                                snprintf(buf, sizeof(buf), "%.0f  %.0f  %.0f",
                                         ss.loc_x, ss.loc_y, ss.loc_z);
                                addrow("Location", buf);
                                addrowi("Health", ss.health);
                                addrow("LocMode", loco_name(ss.loc_mode));
                                addrow("SMT",     smt_name(ss.smt));
                                addrowf("Lean",   ss.lean, "%.3f");
                            }
                        }
                    }
                } else if (srv && mp_sel_cat == MNC_ENEMY && mp_sel_idx >= 0) {
                    EnemySnapshot *e = &s->enemies[mp_sel_idx];
                    if (e->used) {
                        has_info   = true;
                        hdr_col    = MC_ENM_B;
                        panel_border = IM_COL32(150, 60, 210, 200);
                        snprintf(hdr_text, sizeof(hdr_text), "Enemy: %s", e->name);
                        EnemySnapState es = decode_enemy_snap(e);
                        if (es.valid) {
                            char buf[64];
                            snprintf(buf, sizeof(buf), "%.0f  %.0f  %.0f", es.x, es.y, es.z);
                            addrow("Location", buf);
                            addrowi("Yaw", es.yaw);
                            addrow("Class",  es.cls[0] ? es.cls : "?");
                            addrow("Weapon", weapon_name(es.weapon));
                            if (es.smt >= 0) addrow("SMT", smt_name(es.smt));
                        }
                        // Owner player
                        Player *owner = server_find_player_by_id(s, e->owner_player_id);
                        if (owner) {
                            char buf[32];
                            snprintf(buf, sizeof(buf), "#%d %s", owner->id,
                                     owner->nick[0] ? owner->nick : "?");
                            addrow("Owner", buf);
                        } else {
                            addrowi("OwnerID", e->owner_player_id);
                        }
                    }
                } else if (srv && mp_sel_cat == MNC_DOOR && mp_sel_idx >= 0) {
                    DoorSnapshot *d = &s->doors[mp_sel_idx];
                    if (d->used) {
                        has_info   = true;
                        hdr_col    = MC_DOOR_B;
                        panel_border = IM_COL32(60, 130, 220, 200);
                        snprintf(hdr_text, sizeof(hdr_text), "Door");
                        addrow("Key (X,Y,Z)", d->key);
                        // Decode state from state_pkt: [type][pid4][X4][Y4][Z4][Angle4][Speed4]
                        if (d->state_len >= 21) {
                            const uint8_t *b = (const uint8_t*)d->state_pkt;
                            // Identify packet type: 0x12=STATE, 0x13=OPEN, 0x14=CLOSE
                            uint8_t pt = b[0];
                            const char *type_name = (pt == 0x12) ? "STATE" :
                                                    (pt == 0x13) ? "OPEN"  :
                                                    (pt == 0x14) ? "CLOSE" : "?";
                            addrow("LastEvent", type_name);
                            if (pt == 0x12 && d->state_len >= 25) {
                                int angle_raw = (int)b[17] | ((int)b[18]<<8) |
                                                ((int)b[19]<<16) | ((int)b[20]<<24);
                                float angle = angle_raw / 1000.f;
                                addrowf("Angle", angle);
                                int spd_raw = (int)b[21] | ((int)b[22]<<8) |
                                              ((int)b[23]<<16) | ((int)b[24]<<24);
                                float spd = spd_raw / 1000.f;
                                addrowf("Speed", spd);
                            }
                        } else if (d->state_len > 0) {
                            uint8_t pt = ((const uint8_t*)d->state_pkt)[0];
                            addrow("LastEvent", (pt == 0x13) ? "OPEN" :
                                                (pt == 0x14) ? "CLOSE" : "?");
                        }
                        if (d->angle_len >= 21) {
                            const uint8_t *b = (const uint8_t*)d->angle_pkt;
                            int angle_raw = (int)b[17] | ((int)b[18]<<8) |
                                            ((int)b[19]<<16) | ((int)b[20]<<24);
                            addrowf("SnapAngle", angle_raw / 1000.f);
                        }
                        if (d->authority_player_id != 0) {
                            Player *auth = server_find_player_by_id(s, d->authority_player_id);
                            if (auth) {
                                char buf[32];
                                snprintf(buf, sizeof(buf), "#%d %s", auth->id,
                                         auth->nick[0] ? auth->nick : "?");
                                addrow("LockedBy", buf);
                            } else {
                                addrowi("LockedBy ID", d->authority_player_id);
                            }
                        } else {
                            addrow("Locked", "no");
                        }
                        const char *rc = (d->room_idx >= 0 && d->room_idx < s->n_rooms)
                                          ? s->rooms[d->room_idx].code : "?";
                        addrow("Room", rc);
                    }
                } else if (srv && mp_sel_cat == MNC_PUSHABLE && mp_sel_idx >= 0) {
                    PushSnapshot *ps = &s->pushables[mp_sel_idx];
                    if (ps->used) {
                        has_info   = true;
                        hdr_col    = MC_PUSH_B;
                        panel_border = IM_COL32(200, 140, 40, 200);
                        snprintf(hdr_text, sizeof(hdr_text), "Pushable");
                        addrow("Key (X,Y,Z)", ps->key);
                        // wire: [channel(1)][type(1)][pid(4)][KeyX(4)][KeyY(4)][KeyZ(4)][Disp*1000(4)][Seq(4)][bPushing(1)]
                        // offsets:  0         1        2       6        10       14       18            22      26
                        if (ps->pkt_len >= 27) {
                            const uint8_t *b = (const uint8_t*)ps->pkt;
                            int disp_raw = (int)b[18] | ((int)b[19]<<8) |
                                           ((int)b[20]<<16) | ((int)b[21]<<24);
                            uint32_t seq = (uint32_t)b[22] | ((uint32_t)b[23]<<8) |
                                           ((uint32_t)b[24]<<16) | ((uint32_t)b[25]<<24);
                            int bpushing = (int)b[26];
                            addrowf("Displacement", disp_raw / 1000.f);
                            addrowf("Seq",          (float)seq);
                            addrow ("Pushing",      bpushing ? "yes" : "no");
                        }
                        const char *rc = (ps->room_idx >= 0 && ps->room_idx < s->n_rooms)
                                          ? s->rooms[ps->room_idx].code : "?";
                        addrow("Room", rc);
                    }
                } else if (srv && mp_sel_cat == MNC_PICKUP && mp_sel_idx >= 0) {
                    PickupSnapshot *pk = &s->pickups[mp_sel_idx];
                    if (pk->used) {
                        has_info   = true;
                        hdr_col    = MC_PICK_B;
                        panel_border = IM_COL32(210, 70, 60, 200);
                        snprintf(hdr_text, sizeof(hdr_text), "Pickup");
                        addrow("Key", pk->key);
                        const char *rc = (pk->room_idx >= 0 && pk->room_idx < s->n_rooms)
                                          ? s->rooms[pk->room_idx].code : "?";
                        addrow("Room", rc);
                    }
                }

                if (has_info) {
                    float oh = OPAD + 20.f + n_rows * ROW + OPAD;
                    float ox = mp_cpos.x + mp_draw_size.x - OW - 6.f;
                    float oy = mp_cpos.y + 6.f;
                    ImVec2 otl = {ox, oy};
                    ImVec2 obr = {ox + OW, oy + oh};
                    mp_overlay_tl = otl;
                    mp_overlay_br = obr;

                    mdl->AddRectFilled(otl, obr, IM_COL32(16, 20, 28, 230), 6.f);
                    mdl->AddRect      (otl, obr, panel_border, 6.f, 0, 1.2f);

                    float cx = ox + OPAD;
                    float cy = oy + OPAD;
                    mdl->AddText(ImGui::GetFont(), 15.f, {cx, cy}, hdr_col, hdr_text);
                    cy += 20.f;

                    const ImU32 CLB = IM_COL32(130, 140, 160, 200);
                    const ImU32 CVL = IM_COL32(220, 230, 240, 255);
                    const float VAL_X = 120.f;
                    for (int ri = 0; ri < n_rows; ri++) {
                        mdl->AddText(ImGui::GetFont(), OFS, {cx, cy},          CLB, rows[ri][0]);
                        mdl->AddText(ImGui::GetFont(), OFS, {cx + VAL_X, cy},  CVL, rows[ri][1]);
                        cy += ROW;
                    }
                } else {
                    mp_overlay_tl = {0.f, 0.f};
                    mp_overlay_br = {0.f, 0.f};
                }
            }

            // Bottom bar
            {
                float by  = mp_cpos.y + mp_draw_size.y + 2.f;
                char  zbuf[48];
                snprintf(zbuf, sizeof(zbuf), "Zoom: %.3f", mp_zoom);
                ImVec2 zsz = ImGui::CalcTextSize(zbuf);
                mdl->AddText({mp_cpos.x + 6.f, by},
                             IM_COL32(170, 170, 190, 220), zbuf);
                mdl->AddText({mp_cpos.x + zsz.x + 18.f, by},
                             IM_COL32(80, 80, 96, 180),
                             "RMB drag: pan   Scroll: zoom   MMB: reset   LMB node: info");
            }

            ImGui::EndTabItem();
        }

        // ---- History tab ----
        if (ImGui::BeginTabItem("History")) {
            // Snapshot the ring head once; read all entries from tail to head.
            static unsigned g_hist_tail = 0;
            static std::deque<std::string> g_hist_lines;
            static bool g_hist_scroll = true;
            static const int HIST_MAX = 1000;

            unsigned head = __atomic_load_n(&s->history.head, __ATOMIC_ACQUIRE);
            while (g_hist_tail != head) {
                unsigned idx = g_hist_tail & (HISTORY_RING_SIZE - 1);
                g_hist_lines.emplace_back(s->history.buf[idx].msg);
                if ((int)g_hist_lines.size() > HIST_MAX)
                    g_hist_lines.pop_front();
                g_hist_tail++;
                g_hist_scroll = true;
            }

            if (ImGui::Button("Clear")) {
                g_hist_lines.clear();
                g_hist_scroll = false;
            }

            ImGui::BeginChild("hist_scroll", {0, 0}, false,
                              ImGuiWindowFlags_HorizontalScrollbar);
            for (auto &line : g_hist_lines)
                ImGui::TextUnformatted(line.c_str());
            if (g_hist_scroll) {
                ImGui::SetScrollHereY(1.0f);
                g_hist_scroll = false;
            }
            ImGui::EndChild();
            ImGui::EndTabItem();
        }

        ImGui::EndTabBar();
    }
    ImGui::End();
    (void)done;
}

// ---------------------------------------------------------------------------
// gui_run
// ---------------------------------------------------------------------------

#ifdef GUI_BACKEND_DX9

#include "backends/imgui_impl_win32.h"
#include "backends/imgui_impl_dx9.h"
#include <d3d9.h>

static LPDIRECT3D9       g_d3d      = nullptr;
static LPDIRECT3DDEVICE9 g_d3d_dev  = nullptr;
static D3DPRESENT_PARAMETERS g_d3d_pp = {};
static HWND              g_hwnd     = nullptr;

static bool create_d3d(HWND hwnd) {
    g_d3d = Direct3DCreate9(D3D_SDK_VERSION);
    if (!g_d3d) return false;
    ZeroMemory(&g_d3d_pp, sizeof(g_d3d_pp));
    g_d3d_pp.Windowed               = TRUE;
    g_d3d_pp.SwapEffect             = D3DSWAPEFFECT_DISCARD;
    g_d3d_pp.BackBufferFormat       = D3DFMT_UNKNOWN;
    g_d3d_pp.EnableAutoDepthStencil = TRUE;
    g_d3d_pp.AutoDepthStencilFormat = D3DFMT_D16;
    g_d3d_pp.PresentationInterval   = D3DPRESENT_INTERVAL_ONE;
    HRESULT hr = g_d3d->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hwnd,
        D3DCREATE_HARDWARE_VERTEXPROCESSING, &g_d3d_pp, &g_d3d_dev);
    if (FAILED(hr))
        hr = g_d3d->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hwnd,
            D3DCREATE_SOFTWARE_VERTEXPROCESSING, &g_d3d_pp, &g_d3d_dev);
    if (FAILED(hr))
        hr = g_d3d->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_REF, hwnd,
            D3DCREATE_SOFTWARE_VERTEXPROCESSING, &g_d3d_pp, &g_d3d_dev);
    return SUCCEEDED(hr);
}

static void cleanup_d3d() {
    if (g_d3d_dev) { g_d3d_dev->Release(); g_d3d_dev = nullptr; }
    if (g_d3d)     { g_d3d->Release();     g_d3d     = nullptr; }
}

static void reset_d3d() {
    ImGui_ImplDX9_InvalidateDeviceObjects();
    g_d3d_dev->Reset(&g_d3d_pp);
    ImGui_ImplDX9_CreateDeviceObjects();
}

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND, UINT, WPARAM, LPARAM);

static LRESULT WINAPI wnd_proc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
    if (ImGui_ImplWin32_WndProcHandler(hwnd, msg, wp, lp)) return true;
    switch (msg) {
        case WM_SIZE:
            if (g_d3d_dev && wp != SIZE_MINIMIZED) {
                g_d3d_pp.BackBufferWidth  = LOWORD(lp);
                g_d3d_pp.BackBufferHeight = HIWORD(lp);
                reset_d3d();
            }
            return 0;
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
    }
    return DefWindowProcA(hwnd, msg, wp, lp);
}

int gui_run(Server *s, volatile int *running, DB *db) {
    g_log_mutex   = new std::mutex();
    g_log_lines   = new std::deque<std::string>();
    g_srv_running = new std::atomic<bool>(false);

    WNDCLASSEXA wc = { sizeof(wc), CS_CLASSDC, wnd_proc, 0, 0,
                       GetModuleHandleA(nullptr), nullptr, nullptr, nullptr, nullptr,
                       "OpenOL_Relay_GUI", nullptr };
    RegisterClassExA(&wc);

    char title[128];
    snprintf(title, sizeof(title), "OpenOL Relay - %s", db->config.name);
    g_hwnd = CreateWindowA("OpenOL_Relay_GUI", title,
        WS_OVERLAPPEDWINDOW, 100, 100, 1000, 650,
        nullptr, nullptr, wc.hInstance, nullptr);

    if (!create_d3d(g_hwnd)) {
        MessageBoxA(nullptr, "Failed to create DirectX 9 device", "OpenOL Relay", MB_OK | MB_ICONERROR);
        cleanup_d3d();
        DestroyWindow(g_hwnd);
        UnregisterClassA("OpenOL_Relay_GUI", wc.hInstance);
        return 1;
    }

    ShowWindow(g_hwnd, SW_SHOWDEFAULT);
    UpdateWindow(g_hwnd);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();
    ImGui::GetIO().IniFilename = nullptr;

    ImGui_ImplWin32_Init(g_hwnd);
    ImGui_ImplDX9_Init(g_d3d_dev);

    g_gui_log_hook = log_hook;
    cli_init(s);

    std::thread srv_thread;
    bool done = false;

    while (!done) {
        MSG msg;
        while (PeekMessageA(&msg, nullptr, 0, 0, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessageA(&msg);
            if (msg.message == WM_QUIT) done = true;
        }
        if (done) break;

        ImGui_ImplDX9_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

        draw_frame(s, running, db, srv_thread, done);

        // Update window title with server name
        snprintf(title, sizeof(title), "OpenOL Relay - %s", db->config.name);
        SetWindowTextA(g_hwnd, title);

        ImGui::Render();
        g_d3d_dev->SetRenderState(D3DRS_ZENABLE, FALSE);
        g_d3d_dev->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
        g_d3d_dev->SetRenderState(D3DRS_SCISSORTESTENABLE, FALSE);
        g_d3d_dev->Clear(0, nullptr, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER,
                         D3DCOLOR_RGBA(26, 26, 26, 255), 1.0f, 0);
        if (g_d3d_dev->BeginScene() >= 0) {
            ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
            g_d3d_dev->EndScene();
        }
        HRESULT hr = g_d3d_dev->Present(nullptr, nullptr, nullptr, nullptr);
        if (hr == D3DERR_DEVICELOST && g_d3d_dev->TestCooperativeLevel() == D3DERR_DEVICENOTRESET)
            reset_d3d();
    }

    if (*g_srv_running) {
        *g_srv_running = false;
        *running = 0;
        if (srv_thread.joinable())
            srv_thread.join();
    }

    ImGui_ImplDX9_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();
    cleanup_d3d();
    DestroyWindow(g_hwnd);
    UnregisterClassA("OpenOL_Relay_GUI", wc.hInstance);
    return 0;
}

#else // GUI_BACKEND_SDL_GL

int gui_run(Server *s, volatile int *running, DB *db) {
    g_log_mutex   = new std::mutex();
    g_log_lines   = new std::deque<std::string>();
    g_srv_running = new std::atomic<bool>(false);

    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        fprintf(stderr, "SDL_Init error: %s\n", SDL_GetError());
        return 1;
    }

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

    SDL_Window *window = SDL_CreateWindow("OpenOL Relay",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1000, 650,
        SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
    if (!window) {
        fprintf(stderr, "SDL_CreateWindow error: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    SDL_GLContext gl_ctx = SDL_GL_CreateContext(window);
    if (!gl_ctx) {
        fprintf(stderr, "SDL_GL_CreateContext error: %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }
    SDL_GL_MakeCurrent(window, gl_ctx);
    SDL_GL_SetSwapInterval(1);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();
    ImGui::GetIO().IniFilename = nullptr;

    ImGui_ImplSDL2_InitForOpenGL(window, gl_ctx);
    ImGui_ImplOpenGL3_Init("#version 330");

    g_gui_log_hook = log_hook;
    cli_init(s);

    std::thread srv_thread;
    bool done = false;
    char title[128];

    while (!done) {
        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            ImGui_ImplSDL2_ProcessEvent(&e);
            if (e.type == SDL_QUIT) done = true;
            if (e.type == SDL_WINDOWEVENT &&
                e.window.event == SDL_WINDOWEVENT_CLOSE &&
                e.window.windowID == SDL_GetWindowID(window))
                done = true;
        }

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplSDL2_NewFrame();
        ImGui::NewFrame();

        draw_frame(s, running, db, srv_thread, done);

        snprintf(title, sizeof(title), "OpenOL Relay - %s", db->config.name);
        SDL_SetWindowTitle(window, title);

        ImGui::Render();
        int w, h;
        SDL_GetWindowSize(window, &w, &h);
        glViewport(0, 0, w, h);
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        SDL_GL_SwapWindow(window);
    }

    if (*g_srv_running) {
        *g_srv_running = false;
        *running = 0;
        if (srv_thread.joinable())
            srv_thread.join();
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();
    SDL_GL_DeleteContext(gl_ctx);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}

#endif // GUI_BACKEND_DX9
