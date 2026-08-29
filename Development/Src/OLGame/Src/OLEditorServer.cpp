/*=============================================================================
    OLEditorServer.cpp — TCP IPC server for the native Linux editor.

    Network I/O runs on a dedicated worker thread so the game thread never
    blocks waiting for the editor (which only ticks when the game window has
    focus under Wine). Commands received from the editor are queued and
    dispatched on the game thread in OLEditorServer_Tick().
=============================================================================*/
#include "OLGame.h"
#include "OLEditorProto.h"

#if WITH_EDITOR

#define OL_EDITOR_PORT 27666

// ---------------------------------------------------------------------------
// Command queue: worker thread produces, game thread consumes
// ---------------------------------------------------------------------------
static CRITICAL_SECTION g_cs;
static char  g_cmd_queue[64][512]; // circular buffer of pending commands
static int   g_cmd_head = 0;
static int   g_cmd_tail = 0;

static void Queue_Push(const char* cmd)
{
    EnterCriticalSection(&g_cs);
    int next = (g_cmd_tail + 1) % 64;
    if (next != g_cmd_head) // drop if full
    {
        strncpy(g_cmd_queue[g_cmd_tail], cmd, 511);
        g_cmd_queue[g_cmd_tail][511] = '\0';
        g_cmd_tail = next;
    }
    LeaveCriticalSection(&g_cs);
}

static bool Queue_Pop(char* out)
{
    EnterCriticalSection(&g_cs);
    if (g_cmd_head == g_cmd_tail) { LeaveCriticalSection(&g_cs); return false; }
    strncpy(out, g_cmd_queue[g_cmd_head], 511);
    out[511] = '\0';
    g_cmd_head = (g_cmd_head + 1) % 64;
    LeaveCriticalSection(&g_cs);
    return true;
}

// ---------------------------------------------------------------------------
// Worker thread — all socket I/O here
// ---------------------------------------------------------------------------
static SOCKET  g_server_sock  = INVALID_SOCKET;
static SOCKET  g_udp_sock     = INVALID_SOCKET; // scene broadcast
static HANDLE  g_ipc_thread   = NULL;
static volatile LONG g_ipc_stop = 0;

#define OL_SCENE_PORT 27668
#define OL_SCENE_BUF  (sizeof(OLProto_Header) + OL_PROTO_MAX_ACTORS * sizeof(OLProto_Actor))
static char g_scene_send_buf[65536];

// Send queue: game thread produces (via IPC_SendLine), worker consumes
static CRITICAL_SECTION g_send_cs;
static char  g_send_queue[64][512];
static int   g_send_head = 0;
static int   g_send_tail = 0;

static void Send_Push(const char* line)
{
    EnterCriticalSection(&g_send_cs);
    int next = (g_send_tail + 1) % 64;
    if (next != g_send_head)
    {
        strncpy(g_send_queue[g_send_tail], line, 511);
        g_send_queue[g_send_tail][511] = '\0';
        g_send_tail = next;
    }
    LeaveCriticalSection(&g_send_cs);
}

static bool Send_Pop(char* out)
{
    EnterCriticalSection(&g_send_cs);
    if (g_send_head == g_send_tail) { LeaveCriticalSection(&g_send_cs); return false; }
    strncpy(out, g_send_queue[g_send_head], 511);
    out[511] = '\0';
    g_send_head = (g_send_head + 1) % 64;
    LeaveCriticalSection(&g_send_cs);
    return true;
}

static void IPC_SendLine(const char* line)
{
    char buf[514];
    int len = (int)strlen(line);
    if (len > 511) len = 511;
    memcpy(buf, line, len);
    buf[len]   = '\n';
    buf[len+1] = '\0';
    Send_Push(buf);
}

static DWORD WINAPI IPC_WorkerThread(LPVOID)
{
    SOCKET client = INVALID_SOCKET;
    char   recv_buf[4096];
    int    recv_len = 0;

    while (!g_ipc_stop)
    {
        // Accept new client
        if (client == INVALID_SOCKET)
        {
            fd_set fds; FD_ZERO(&fds); FD_SET(g_server_sock, &fds);
            timeval tv = {0, 100000}; // 100ms
            if (select(0, &fds, NULL, NULL, &tv) > 0)
            {
                client = accept(g_server_sock, NULL, NULL);
                if (client != INVALID_SOCKET)
                    debugf(TEXT("OLEditorServer: client connected"));
            }
            continue;
        }

        // Flush send queue
        char sline[514];
        while (Send_Pop(sline))
            send(client, sline, (int)strlen(sline), 0);

        // Receive commands
        fd_set fds; FD_ZERO(&fds); FD_SET(client, &fds);
        timeval tv = {0, 10000}; // 10ms
        int sel = select(0, &fds, NULL, NULL, &tv);
        if (sel > 0)
        {
            int n = recv(client, recv_buf + recv_len, sizeof(recv_buf) - recv_len - 1, 0);
            if (n <= 0)
            {
                debugf(TEXT("OLEditorServer: client disconnected"));
                closesocket(client); client = INVALID_SOCKET;
                recv_len = 0;
                continue;
            }
            recv_len += n;
            recv_buf[recv_len] = '\0';

            char* start = recv_buf;
            char* nl;
            while ((nl = strchr(start, '\n')) != NULL)
            {
                *nl = '\0';
                if (nl > start && *(nl-1) == '\r') *(nl-1) = '\0';
                if (*start) Queue_Push(start);
                start = nl + 1;
            }
            int remaining = (int)(recv_buf + recv_len - start);
            if (remaining > 0 && start != recv_buf)
                memmove(recv_buf, start, remaining);
            recv_len = remaining;
        }
    }

    if (client != INVALID_SOCKET) closesocket(client);
    return 0;
}

// ---------------------------------------------------------------------------
// Game-thread side
// ---------------------------------------------------------------------------
// Implemented in UnrealEd/Src/OLEditorBridge.cpp — has access to GUnrealEd
extern void OLEditor_SetView(FLOAT X, FLOAT Y, FLOAT Z, INT Pitch, INT Yaw, INT Roll);
extern void OLEditor_BroadcastScene();
extern int  OLEditor_GetScenePacket(void* out_buf, int max_len);
extern void OLEditor_ClickAt(FLOAT NormX, FLOAT NormY);
extern void OLEditor_MoveActor(const char* name, FLOAT dx, FLOAT dy, FLOAT dz);
extern void OLEditor_RotateActor(const char* name, INT dp, INT dy, INT dr);
extern void OLEditor_RotateActorAxis(const char* name, FLOAT ax, FLOAT ay, FLOAT az, FLOAT angle_rad);
extern void OLEditor_ScaleActor(const char* name, FLOAT sx, FLOAT sy, FLOAT sz);

static void IPC_HandleLine(const char* line)
{
    if (!GEngine) return;

    if (strncmp(line, "OL_SETVIEW ", 11) == 0)
    {
        float x, y, z; int pitch, yaw, roll;
        if (sscanf(line + 11, "%f %f %f %d %d %d", &x, &y, &z, &pitch, &yaw, &roll) == 6)
            OLEditor_SetView(x, y, z, pitch, yaw, roll);
        return;
    }

    if (strncmp(line, "OL_CLICK ", 9) == 0)
    {
        float nx, ny;
        if (sscanf(line + 9, "%f %f", &nx, &ny) == 2)
            OLEditor_ClickAt(nx, ny);
        return;
    }

    if (strncmp(line, "OL_MOVE ", 8) == 0)
    {
        char name[128]; float dx, dy, dz;
        if (sscanf(line + 8, "%127s %f %f %f", name, &dx, &dy, &dz) == 4)
            OLEditor_MoveActor(name, dx, dy, dz);
        return;
    }

    if (strncmp(line, "OL_ROTATE_AXIS ", 15) == 0)
    {
        char name[128]; float rax, ray, raz, angle;
        if (sscanf(line + 15, "%127s %f %f %f %f", name, &rax, &ray, &raz, &angle) == 5)
            OLEditor_RotateActorAxis(name, rax, ray, raz, angle);
        return;
    }

    if (strncmp(line, "OL_ROTATE ", 10) == 0)
    {
        char name[128]; int dp, dy, dr;
        if (sscanf(line + 10, "%127s %d %d %d", name, &dp, &dy, &dr) == 4)
            OLEditor_RotateActor(name, dp, dy, dr);
        return;
    }

    if (strncmp(line, "OL_SCALE ", 9) == 0)
    {
        char name[128]; float sx, sy, sz;
        if (sscanf(line + 9, "%127s %f %f %f", name, &sx, &sy, &sz) == 4)
            OLEditor_ScaleActor(name, sx, sy, sz);
        return;
    }

    if (appStrcmpANSI(line, "GET_ACTORS") == 0)
    {
        for (FActorIterator It; It; ++It)
        {
            AActor* A = *It;
            if (!A || A->bDeleteMe) continue;
            FString json = FString::Printf(
                TEXT("{\"class\":\"%s\",\"name\":\"%s\",\"x\":%.1f,\"y\":%.1f,\"z\":%.1f}"),
                *A->GetClass()->GetName(), *A->GetName(),
                A->Location.X, A->Location.Y, A->Location.Z);
            IPC_SendLine(TCHAR_TO_ANSI(*json));
        }
        IPC_SendLine("END_ACTORS");
        return;
    }

    GEngine->Exec(ANSI_TO_TCHAR(line), *GNull);
}

void OLEditorServer_Tick();

// ---------------------------------------------------------------------------
// Standalone game-thread ticker — active only in editor (GIsEditor)
// ---------------------------------------------------------------------------
class FOLEditorServerTicker : public FTickableObject
{
public:
    virtual void Tick(FLOAT) { OLEditorServer_Tick(); }
    virtual UBOOL IsTickable() const { return TRUE; }
    virtual UBOOL IsTickableWhenPaused() const { return TRUE; }
};
static FOLEditorServerTicker* GEditorServerTicker = NULL;

void OLEditorServer_Init()
{
    if (g_ipc_thread) return; // already running

    InitializeCriticalSection(&g_cs);
    InitializeCriticalSection(&g_send_cs);

    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
    {
        debugf(TEXT("OLEditorServer: WSAStartup failed"));
        return;
    }

    g_server_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (g_server_sock == INVALID_SOCKET)
    {
        debugf(TEXT("OLEditorServer: socket() failed %d"), WSAGetLastError());
        return;
    }

    int opt = 1;
    setsockopt(g_server_sock, SOL_SOCKET, SO_REUSEADDR, (const char*)&opt, sizeof(opt));

    sockaddr_in addr;
    appMemzero(&addr, sizeof(addr));
    addr.sin_family      = AF_INET;
    addr.sin_port        = (WORD)(((OL_EDITOR_PORT & 0xFF) << 8) | (OL_EDITOR_PORT >> 8));
    addr.sin_addr.s_addr = 0x0100007F; // 127.0.0.1

    if (bind(g_server_sock, (sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR)
    {
        debugf(TEXT("OLEditorServer: bind() failed %d"), WSAGetLastError());
        closesocket(g_server_sock); g_server_sock = INVALID_SOCKET;
        return;
    }

    listen(g_server_sock, 1);
    debugf(TEXT("OLEditorServer: listening on 127.0.0.1:%d"), OL_EDITOR_PORT);

    g_ipc_thread = CreateThread(NULL, 0, IPC_WorkerThread, NULL, 0, NULL);

    g_udp_sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

    // Register standalone ticker so OLEditorServer_Tick runs every world tick.
    if (!GEditorServerTicker)
        GEditorServerTicker = new FOLEditorServerTicker();
}

void OLEditorServer_Tick()
{
    // Dispatch commands queued by the worker thread onto the game thread
    char cmd[512];
    while (Queue_Pop(cmd))
        IPC_HandleLine(cmd);

    if (g_udp_sock == INVALID_SOCKET) return;

    OLEditor_BroadcastScene();

    int len = OLEditor_GetScenePacket(g_scene_send_buf, sizeof(g_scene_send_buf));
    if (len <= 0) return;

    // UDP max payload — send in 64KB chunks if needed
    const int CHUNK = 65000;
    sockaddr_in dst;
    appMemzero(&dst, sizeof(dst));
    dst.sin_family      = AF_INET;
    dst.sin_port        = (WORD)(((OL_SCENE_PORT & 0xFF) << 8) | (OL_SCENE_PORT >> 8));
    dst.sin_addr.s_addr = 0x0100007F;

    // First packet: header + as many actors as fit
    sendto(g_udp_sock, g_scene_send_buf, len < CHUNK ? len : CHUNK, 0, (sockaddr*)&dst, sizeof(dst));
}

void OLEditorServer_Shutdown()
{
    if (g_ipc_thread)
    {
        InterlockedExchange(&g_ipc_stop, 1);
        WaitForSingleObject(g_ipc_thread, 2000);
        CloseHandle(g_ipc_thread); g_ipc_thread = NULL;
    }
    if (g_server_sock != INVALID_SOCKET) { closesocket(g_server_sock); g_server_sock = INVALID_SOCKET; }
    if (g_udp_sock    != INVALID_SOCKET) { closesocket(g_udp_sock);    g_udp_sock    = INVALID_SOCKET; }
    WSACleanup();
    DeleteCriticalSection(&g_cs);
    DeleteCriticalSection(&g_send_cs);
}

#endif // WITH_EDITOR
