/*=============================================================================
    ImGuiShared.h — types and interfaces shared between D3D9Drv (render side)
    and OLGame (content/UI side) for the ImGui debug overlay.

    D3D9Drv owns: device init, NewFrame, Render, device objects.
    OLGame owns:  UI content (tabs), snapshot structs, FOLImGuiTicker.

    Both sides include this header; neither includes imgui.h directly.
=============================================================================*/

#pragma once

static const int SNAP_NAME_LEN = 128;
static const int SNAP_STR_LEN  = 128;

// ---------------------------------------------------------------------------
// Property snapshot types
// ---------------------------------------------------------------------------

enum EPropSnapType
{
    PST_Bool,
    PST_Int,
    PST_Float,
    PST_Byte,
    PST_Name,
    PST_Str,
    PST_Object,
    PST_Vector,
    PST_Rotator,
    PST_Color,        // FColor        — 4 BYTE (R,G,B,A), stored as FLOAT[4] normalized
    PST_LinearColor,  // FLinearColor  — 4 FLOAT (R,G,B,A)
    PST_Texture,      // UTexture2D*   — preview image; pointer stored in TexPtr
};

struct FPropSnap
{
    char            Name[SNAP_NAME_LEN];
    EPropSnapType   Type;
    INT             Offset;
    BITFIELD        BitMask;
    TArray<FString> EnumValues;
    union
    {
        bool  Bool;
        INT   Int;
        FLOAT Float;
        BYTE  Byte;
        FLOAT Vec[3];
        INT   Rot[3];
        FLOAT Col[4]; // R,G,B,A normalized [0..1] — used by PST_Color and PST_LinearColor
    } Value;
    char     StrValue[SNAP_STR_LEN];
    UObject* TexPtr; // PST_Texture only — pointer to UTexture2D (game thread lifetime)
};


struct FPropGroup
{
    char              ClassName[SNAP_NAME_LEN];
    TArray<FPropSnap> Props;
};

struct FDetailSnap
{
    UObject*           ActorPtr;
    TArray<FPropGroup> Groups;
};

// ---------------------------------------------------------------------------
// Property write queue (render thread → game thread)
// ---------------------------------------------------------------------------

static const int WRITE_QUEUE_SIZE = 64;

enum EPropWriteType { PWT_Bool, PWT_Int, PWT_Float, PWT_Byte, PWT_Str, PWT_Name, PWT_Vector, PWT_Rotator, PWT_Color, PWT_LinearColor };

struct FPropWrite
{
    UObject*       Actor;
    INT            Offset;
    EPropWriteType Type;
    BITFIELD       BitMask;
    union
    {
        bool  Bool;
        INT   Int;
        FLOAT Float;
        BYTE  Byte;
        FLOAT Vec[3];
        INT   Rot[3];
        FLOAT Col[4]; // R,G,B,A normalized [0..1]
    } NewValue;
    char StrValue[SNAP_STR_LEN];
};

// Shared globals — defined in OLGame/Src/OLImGui.cpp,
// read by D3D9Drv/Src/OLImGui.cpp (render thread) and game thread.

// Total live object count — updated each tick by game thread, read by render thread.
// Render thread uses this as the Clipper count and indexes GObjObjects directly.
extern volatile int GSceneObjCount;

extern FDetailSnap*  GDetailFront;
extern FDetailSnap*  GDetailBack;
extern volatile int  GDetailReady;

extern UObject* volatile GDetailRequest;

extern FPropWrite   GWriteQueue[WRITE_QUEUE_SIZE];
extern volatile int GWriteCount;

// Overlay visibility flag — defined in D3D9Drv, read in OLGame (ticker skip).
extern bool GImGuiShowDemoWindow;

// Set to 1 by OLGame when the HUD needs mouse input (e.g. Connecting banner Cancel button)
// without the debug window being open. OLImGui_IsOverlayVisible() returns true while set.
extern volatile int GImGuiWantMouse;

// Pending call queue — render thread enqueues, game thread (FOLImGuiTicker) drains.
// Used by tab widgets that need to execute code on the game thread (UE3 object access).
typedef void (*OLImGuiCallFn)();
static const int GCallQueueCapacity = 32;
extern volatile OLImGuiCallFn GCallQueue[GCallQueueCapacity];
extern volatile int           GCallCount;

// Post-tick call queue — drained after GWorld::Tick (InTick==0).
// Use this for operations that are forbidden during InTick (e.g. Streammap / GC).
static const int GPostTickQueueCapacity = 16;
extern volatile OLImGuiCallFn GPostTickQueue[GPostTickQueueCapacity];
extern volatile int           GPostTickCount;

// ---------------------------------------------------------------------------
// Multiplayer HUD snapshot — filled by AMultiplayerHUD::NativePostRender (game thread),
// read by OLImGui_BuildUI() (render thread) via double-buffer swap.
// Plain C arrays/ints only — no UE3 types so D3D9Drv can include this header.
// ---------------------------------------------------------------------------

// Connection status shown in the HUD during connect/resolve.
enum EMpConnStatus
{
    MCS_Disconnected = 0,  // not connecting
    MCS_Resolving,         // DNS resolve in progress
    MCS_Connecting,        // resolved, waiting for server handshake
    MCS_Connected,         // fully connected
};

static const int MP_HUD_MAX_PLAYERS   = 16;
static const int MP_HUD_MAX_NOTIF     = 12;
static const int MP_HUD_NICK_LEN      = 64;
static const int MP_HUD_SERVER_LEN    = 64;
static const int MP_HUD_NOTIF_LEN     = 128;

struct FMpHudPlayer
{
    char  Nick[MP_HUD_NICK_LEN];
    float HealthRatio;    // 0..1
    float ScreenX;        // projected head position (nametag), -1 if behind camera
    float ScreenY;
    float NametagScale;
    bool  bIsSelf;
};

struct FMpHudNotif
{
    char  Text[MP_HUD_NOTIF_LEN];
    float Alpha;          // 0..1 (fade in/out already computed)
    bool  bIsDisconnect;  // true → red accent, false → green
};

struct FMpHudSnapshot
{
    // Server/ping panel
    char  ServerName[MP_HUD_SERVER_LEN];
    int   OnlineCount;
    float PingMs;
    bool  bPingStale;
    int   PingBars;       // 0..4

    // Player list (nick list + nametags share the same array)
    FMpHudPlayer Players[MP_HUD_MAX_PLAYERS];
    int          PlayerCount;

    // Notifications
    FMpHudNotif  Notifs[MP_HUD_MAX_NOTIF];
    int          NotifCount;

    // Screen size (for ImGui coordinate mapping)
    float ScreenW;
    float ScreenH;

    // Tilt animation phase (WorldInfo.TimeSeconds passed through)
    float TimeSeconds;

    // Connection status (set while connecting, MCS_Connected once handshake done).
    EMpConnStatus ConnStatus;

    bool  bValid;   // false until first frame is written
};

// Double-buffer: game thread writes Back, render thread reads Front.
extern FMpHudSnapshot GMpHudFront;
extern FMpHudSnapshot GMpHudBack;
extern volatile int   GMpHudReady;  // set to 1 when Back has new data

// ---------------------------------------------------------------------------
// Main window screen rect — written each frame by OLImGui_BuildUI so that
// OLImGui_Render (D3D9Drv) can sample the backbuffer region for blur.
// ---------------------------------------------------------------------------
extern int GOLWinX, GOLWinY, GOLWinW, GOLWinH;

// ---------------------------------------------------------------------------
// OL_SNAP — thread-safe snapshot double-buffer pattern.
//
// Two buffers (A/B). Game thread always writes to the buffer NOT currently
// pointed to by the read pointer, then atomically swaps the pointer.
// Render thread reads through the pointer — it always sees a complete buffer.
//
// Usage:
//   // File scope:
//   OL_SNAP_DECL(FMySnap, GMySnap);
//
//   // Game thread — write and publish:
//   OL_SNAP_BEGIN_WRITE(GMySnap)
//       OL_SNAP_BACK(GMySnap).Field = value;
//   OL_SNAP_END_WRITE(FMySnap, GMySnap);
//
//   // Render thread — read only:
//   if (OL_SNAP_READY(GMySnap)) OL_SNAP_READ(GMySnap).Field
// ---------------------------------------------------------------------------

// OL_SNAP_DECL must be placed at file scope (not inside a function).
// Type_ must be a plain-data struct (no TArray/FString members).
//
// Pattern:
//   Game thread writes to Back, sets Ready=1.
//   Render thread checks Ready, reads Front.
//   Game thread copies Back→Front only when render thread is not reading
//   (guarded by FlushRenderingCommands before each write).
#define OL_SNAP_DECL(Type_, Name_) \
    static Type_        Name_##Front; \
    static Type_        Name_##Back; \
    static volatile int Name_##Ready = 0

#define OL_SNAP_BEGIN_WRITE(Name_) \
    do { (Name_##Ready) = 0;

#define OL_SNAP_BACK(Name_)  (Name_##Back)

// Pass the same Type_ and Name_ as in OL_SNAP_DECL.
// Caller must ensure render thread is not reading Front (e.g. via FlushRenderingCommands).
#define OL_SNAP_END_WRITE(Type_, Name_) \
    (Name_##Front) = (Name_##Back); \
    (Name_##Ready) = 1; } while(0)

// Render thread: safe to read Front whenever Ready==1.
#define OL_SNAP_READY(Name_)    ((Name_##Ready) != 0)
#define OL_SNAP_READ(Name_)     (Name_##Front)

// ---------------------------------------------------------------------------
// Entry point: OLGame builds the entire ImGui UI each frame.
// Called from D3D9Drv inside OLImGui_Render(), between NewFrame and Render.
// ---------------------------------------------------------------------------
void OLImGui_BuildUI();
