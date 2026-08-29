/*=============================================================================
    OLBrowser.cpp: Offscreen CEF browser on a monitor mesh (OLBikScreen).

    openol_browser.exe runs as a child process and writes BGRA frames into
    shared memory (double-buffered).  Each Tick we:
      1. Copy the latest browser frame into Texture2DDynamic.
      2. Cast a mathematical ray from the camera through the screen plane.
      3. If the crosshair intersects TargetScreen, convert the hit to browser
         pixel coords and send a MouseMove via named pipe.
=============================================================================*/

#include "OLGame.h"

IMPLEMENT_CLASS(AOLBrowser);

// ---------------------------------------------------------------------------
// IPC structures (mirrors openol_browser/src/ol_browser_ipc.h)
// ---------------------------------------------------------------------------

#define OL_BROWSER_MAX_WIDTH  2560
#define OL_BROWSER_MAX_HEIGHT 1440
#define OL_BROWSER_SHM_SIZE   (sizeof(FOLBrowserFrameHeader) + \
                                (SIZE_T)OL_BROWSER_MAX_WIDTH * OL_BROWSER_MAX_HEIGHT * 4 * 2)

#pragma pack(push,1)
struct FOLBrowserFrameHeader
{
    volatile LONG WriteIdx;
    volatile LONG FrameSeq;
    volatile LONG Width;
    volatile LONG Height;
    volatile LONG BrowserReady;
    volatile LONG PageLoading;
    char          PageTitle[256];
    char          PageURL[2048];
};
#pragma pack(pop)

#define OL_INPUT_MOUSE_MOVE   1
#define OL_INPUT_MOUSE_DOWN   2
#define OL_INPUT_MOUSE_UP     3
#define OL_INPUT_MOUSE_SCROLL 4
#define OL_INPUT_KEY_DOWN     5
#define OL_INPUT_KEY_UP       6
#define OL_INPUT_KEY_CHAR     7

#define OL_MOUSE_LEFT   0
#define OL_MOUSE_MIDDLE 1
#define OL_MOUSE_RIGHT  2

#pragma pack(push,1)
struct FOLBrowserInputEvent
{
    BYTE Type;
    INT  X;
    INT  Y;
    INT  DeltaX;
    INT  DeltaY;
    BYTE Button;    // 0=left
    UINT Modifiers;
    UINT WinKeyCode;
    UINT Char;
};
#pragma pack(pop)

#define OL_CMD_NAVIGATE 1
#define OL_CMD_CLOSE    6

#pragma pack(push,1)
struct FOLBrowserCmd
{
    BYTE Type;
    INT  Width;
    INT  Height;
    char URL[2048];
};
#pragma pack(pop)

// ---------------------------------------------------------------------------
// Internal state
// ---------------------------------------------------------------------------

struct FOLBrowserState
{
    HANDLE ProcessHandle;
    HANDLE ShmHandle;
    void*  ShmBase;
    FOLBrowserFrameHeader* Header;
    BYTE*  PixelBuf[2];

    HANDLE InputPipe;
    HANDLE CmdPipe;

    LONG   LastFrameSeq;
    INT    LastCursorX;
    INT    LastCursorY;
    UBOOL  bLastUseDown;   // tracks previous use-button state to send down/up edges

    FOLBrowserState()
    :   ProcessHandle(INVALID_HANDLE_VALUE)
    ,   ShmHandle(NULL)
    ,   ShmBase(NULL)
    ,   Header(NULL)
    ,   InputPipe(INVALID_HANDLE_VALUE)
    ,   CmdPipe(INVALID_HANDLE_VALUE)
    ,   LastFrameSeq(-1)
    ,   LastCursorX(-1)
    ,   LastCursorY(-1)
    ,   bLastUseDown(FALSE)
    {
        PixelBuf[0] = PixelBuf[1] = NULL;
    }

    ~FOLBrowserState()
    {
        if (ShmBase)   UnmapViewOfFile(ShmBase);
        if (ShmHandle) CloseHandle(ShmHandle);
        if (InputPipe != INVALID_HANDLE_VALUE) CloseHandle(InputPipe);
        if (CmdPipe   != INVALID_HANDLE_VALUE) CloseHandle(CmdPipe);
        if (ProcessHandle != INVALID_HANDLE_VALUE)
        {
            TerminateProcess(ProcessHandle, 0);
            CloseHandle(ProcessHandle);
        }
    }
};

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------

static FString GetBrowserBinDir()
{
    // openol_browser.exe lives in <GameRoot>/Tools/openol_browser/
    // OLGame.exe is at <GameRoot>/Binaries/Win64/OLGame.exe
    TCHAR ExePath[MAX_PATH];
    GetModuleFileNameW(NULL, ExePath, MAX_PATH);
    FFilename F(ExePath);
    // openol_browser lives in Tools/openol_browser/ next to Win64/
    return F.GetPath() * TEXT("Tools\\openol_browser");
}

static void SendBrowserInput(HANDLE Pipe, const FOLBrowserInputEvent& Ev)
{
    if (Pipe == INVALID_HANDLE_VALUE) return;
    DWORD Written = 0;
    WriteFile(Pipe, &Ev, sizeof(Ev), &Written, NULL);
}

// ---------------------------------------------------------------------------
// AOLBrowser::Open
// ---------------------------------------------------------------------------

void AOLBrowser::Open()
{
    if (NativeHandle) return;

    FString BinDir = GetBrowserBinDir();

    // Unique names per instance
    PTRINT ID = (PTRINT)this;
    FString ShmName       = FString::Printf(TEXT("OLBrowser_%I64x_Frame"), (UINT64)ID);
    FString InputPipeName = FString::Printf(TEXT("\\\\.\\pipe\\OLBrowser_%I64x_Input"), (UINT64)ID);
    FString CmdPipeName   = FString::Printf(TEXT("\\\\.\\pipe\\OLBrowser_%I64x_Cmd"),   (UINT64)ID);

    FOLBrowserState* State = new FOLBrowserState();

    // Shared memory
    State->ShmHandle = CreateFileMappingW(INVALID_HANDLE_VALUE, NULL,
        PAGE_READWRITE, 0, (DWORD)OL_BROWSER_SHM_SIZE, *ShmName);
    if (!State->ShmHandle)
    {
        debugf(NAME_Warning, TEXT("[OLBrowser] CreateFileMapping failed: %u"), GetLastError());
        delete State;
        return;
    }

    State->ShmBase = MapViewOfFile(State->ShmHandle, FILE_MAP_ALL_ACCESS, 0, 0, OL_BROWSER_SHM_SIZE);
    if (!State->ShmBase)
    {
        debugf(NAME_Warning, TEXT("[OLBrowser] MapViewOfFile failed: %u"), GetLastError());
        delete State;
        return;
    }
    appMemzero(State->ShmBase, sizeof(FOLBrowserFrameHeader));
    State->Header = (FOLBrowserFrameHeader*)State->ShmBase;

    SIZE_T FrameBytes = (SIZE_T)OL_BROWSER_MAX_WIDTH * OL_BROWSER_MAX_HEIGHT * 4;
    BYTE* PixBase = (BYTE*)State->ShmBase + sizeof(FOLBrowserFrameHeader);
    State->PixelBuf[0] = PixBase;
    State->PixelBuf[1] = PixBase + FrameBytes;

    // Launch openol_browser.exe
    // Args: <shm_name> <input_pipe> <cmd_pipe> <w> <h> [url]
    FString BrowserExe = BinDir * TEXT("openol_browser.exe");
    FString CmdLine = FString::Printf(
        TEXT("\"%s\" \"%s\" \"%s\" \"%s\" %d %d \"%s\""),
        *BrowserExe,
        *ShmName,
        *InputPipeName,
        *CmdPipeName,
        BrowserWidth, BrowserHeight,
        *StartURL);

    STARTUPINFOW SI;
    appMemzero(&SI, sizeof(SI));
    SI.cb = sizeof(SI);
    SI.dwFlags = STARTF_USESHOWWINDOW;
    SI.wShowWindow = SW_HIDE;

    PROCESS_INFORMATION ProcInfo;
    appMemzero(&ProcInfo, sizeof(ProcInfo));

    if (!CreateProcessW(NULL, (LPWSTR)*CmdLine, NULL, NULL, FALSE,
        CREATE_NO_WINDOW, NULL, NULL, &SI, &ProcInfo))
    {
        debugf(NAME_Warning, TEXT("[OLBrowser] CreateProcess failed: %u  cmd=%s"),
            GetLastError(), *CmdLine);
        delete State;
        return;
    }

    State->ProcessHandle = ProcInfo.hProcess;
    CloseHandle(ProcInfo.hThread);

    NativeHandle = State;
    bRunning = TRUE;

    // Create Texture2DDynamic (same pattern as OLVideoPlayer)
    BrowserTexture = Cast<UTexture2DDynamic>(
        StaticConstructObject(UTexture2DDynamic::StaticClass(),
            GetTransientPackage(), NAME_None, RF_Transient));
    if (BrowserTexture)
    {
        BrowserTexture->CompressionNone    = TRUE;
        BrowserTexture->MipGenSettings     = TMGS_NoMipmaps;
        BrowserTexture->CompressionNoAlpha = FALSE;
        BrowserTexture->bNoTiling          = TRUE;
        BrowserTexture->Init(BrowserWidth, BrowserHeight, PF_A8R8G8B8);
        // Prevent GC from collecting the texture before it is assigned to the material.
        // RF_Transient objects are not rooted automatically; with two browser instances
        // the GC may collect the second texture before the UC Tick assigns it.
        BrowserTexture->AddToRoot();
    }

    debugf(TEXT("[OLBrowser] Started PID=%u  shm=%s"), (UINT)ProcInfo.dwProcessId, *ShmName);
}

// ---------------------------------------------------------------------------
// AOLBrowser::Close
// ---------------------------------------------------------------------------

void AOLBrowser::Close()
{
    if (!NativeHandle) return;
    FOLBrowserState* State = (FOLBrowserState*)NativeHandle;

    if (State->CmdPipe != INVALID_HANDLE_VALUE)
    {
        FOLBrowserCmd Cmd;
        appMemzero(&Cmd, sizeof(Cmd));
        Cmd.Type = OL_CMD_CLOSE;
        DWORD Written = 0;
        WriteFile(State->CmdPipe, &Cmd, sizeof(Cmd), &Written, NULL);
    }

    delete State;
    NativeHandle = NULL;
    bRunning = FALSE;

    // Release root reference so GC can collect the texture
    if (BrowserTexture)
    {
        BrowserTexture->RemoveFromRoot();
        BrowserTexture = NULL;
    }
}

// ---------------------------------------------------------------------------
// AOLBrowser::Navigate
// ---------------------------------------------------------------------------

void AOLBrowser::Navigate(const FString& URL)
{
    if (!NativeHandle) return;
    FOLBrowserState* State = (FOLBrowserState*)NativeHandle;
    if (State->CmdPipe == INVALID_HANDLE_VALUE) return;

    FOLBrowserCmd Cmd;
    appMemzero(&Cmd, sizeof(Cmd));
    Cmd.Type = OL_CMD_NAVIGATE;
    WideCharToMultiByte(CP_UTF8, 0, *URL, -1, Cmd.URL, (INT)sizeof(Cmd.URL), NULL, NULL);

    DWORD Written = 0;
    WriteFile(State->CmdPipe, &Cmd, sizeof(Cmd), &Written, NULL);
}

// ---------------------------------------------------------------------------
// AOLBrowser::SendClick
// ---------------------------------------------------------------------------

void AOLBrowser::SendClick(UBOOL bDown)
{
    if (!NativeHandle) return;
    FOLBrowserState* State = (FOLBrowserState*)NativeHandle;

    FOLBrowserInputEvent Ev;
    appMemzero(&Ev, sizeof(Ev));
    Ev.Type   = bDown ? OL_INPUT_MOUSE_DOWN : OL_INPUT_MOUSE_UP;
    Ev.Button = 0;
    Ev.X      = State->LastCursorX;
    Ev.Y      = State->LastCursorY;
    SendBrowserInput(State->InputPipe, Ev);
}

// ---------------------------------------------------------------------------
// AOLBrowser::SendRightClick
// ---------------------------------------------------------------------------

void AOLBrowser::SendRightClick(UBOOL bDown)
{
    if (!NativeHandle) return;
    FOLBrowserState* State = (FOLBrowserState*)NativeHandle;

    FOLBrowserInputEvent Ev;
    appMemzero(&Ev, sizeof(Ev));
    Ev.Type   = bDown ? OL_INPUT_MOUSE_DOWN : OL_INPUT_MOUSE_UP;
    Ev.Button = 2; // OL_MOUSE_RIGHT
    Ev.X      = State->LastCursorX;
    Ev.Y      = State->LastCursorY;
    SendBrowserInput(State->InputPipe, Ev);
}

// ---------------------------------------------------------------------------
// AOLBrowser::SendScroll
// ---------------------------------------------------------------------------

void AOLBrowser::SendScroll(INT DeltaY)
{
    if (!NativeHandle) return;
    FOLBrowserState* State = (FOLBrowserState*)NativeHandle;

    FOLBrowserInputEvent Ev;
    appMemzero(&Ev, sizeof(Ev));
    Ev.Type   = OL_INPUT_MOUSE_SCROLL;
    Ev.X      = State->LastCursorX;
    Ev.Y      = State->LastCursorY;
    Ev.DeltaX = 0;
    Ev.DeltaY = DeltaY;
    SendBrowserInput(State->InputPipe, Ev);
}

// ---------------------------------------------------------------------------
// AOLBrowser::SendKey
// ---------------------------------------------------------------------------

void AOLBrowser::SendKey(INT WinKeyCode, UBOOL bDown)
{
    if (!NativeHandle) return;
    FOLBrowserState* State = (FOLBrowserState*)NativeHandle;

    FOLBrowserInputEvent Ev;
    appMemzero(&Ev, sizeof(Ev));
    Ev.Type       = bDown ? OL_INPUT_KEY_DOWN : OL_INPUT_KEY_UP;
    Ev.WinKeyCode = (UINT)WinKeyCode;
    SendBrowserInput(State->InputPipe, Ev);
}

// ---------------------------------------------------------------------------
// AOLBrowser::SendChar
// ---------------------------------------------------------------------------

void AOLBrowser::SendChar(INT CharCode)
{
    if (!NativeHandle) return;
    FOLBrowserState* State = (FOLBrowserState*)NativeHandle;

    FOLBrowserInputEvent Ev;
    appMemzero(&Ev, sizeof(Ev));
    Ev.Type = OL_INPUT_KEY_CHAR;
    Ev.Char = (UINT)CharCode;
    SendBrowserInput(State->InputPipe, Ev);
}

// ---------------------------------------------------------------------------
// AOLBrowser::SetScreenMesh — called from UnrealScript Tick once texture is assigned
// ---------------------------------------------------------------------------

void AOLBrowser::SetScreenMesh(UStaticMeshComponent* Mesh)
{
    ScreenMeshComponent = Mesh;
}

// ---------------------------------------------------------------------------
// AOLBrowser::BeginDestroy
// ---------------------------------------------------------------------------

void AOLBrowser::BeginDestroy()
{
    Close();
    Super::BeginDestroy();
}

// ---------------------------------------------------------------------------
// AOLBrowser::Tick
// ---------------------------------------------------------------------------

UBOOL AOLBrowser::Tick(FLOAT DeltaTime, ELevelTick TickType)
{
    UBOOL bResult = Super::Tick(DeltaTime, TickType);

    if (!NativeHandle || !bRunning)
        return bResult;

    FOLBrowserState* State = (FOLBrowserState*)NativeHandle;
    FOLBrowserFrameHeader* Hdr = State->Header;

    // Once browser is ready, open named pipe client ends
    if (State->InputPipe == INVALID_HANDLE_VALUE
        && InterlockedExchangeAdd(&Hdr->BrowserReady, 0L))
    {
        PTRINT ID = (PTRINT)this;
        FString InputPipeName = FString::Printf(
            TEXT("\\\\.\\pipe\\OLBrowser_%I64x_Input"), (UINT64)ID);
        FString CmdPipeName = FString::Printf(
            TEXT("\\\\.\\pipe\\OLBrowser_%I64x_Cmd"), (UINT64)ID);

        State->InputPipe = CreateFileW(*InputPipeName,
            GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_FLAG_NO_BUFFERING, NULL);
        State->CmdPipe = CreateFileW(*CmdPipeName,
            GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_FLAG_NO_BUFFERING, NULL);

        if (State->InputPipe != INVALID_HANDLE_VALUE)
            debugf(TEXT("[OLBrowser] Pipes connected"));
    }

    // Upload new frame when FrameSeq changes
    LONG Seq = InterlockedExchangeAdd(&Hdr->FrameSeq, 0L);
    if (Seq != State->LastFrameSeq && BrowserTexture)
    {
        State->LastFrameSeq = Seq;

        // Read from the buffer the browser just finished writing (WriteIdx = newest)
        LONG ReadIdx = InterlockedExchangeAdd(&Hdr->WriteIdx, 0L);
        BYTE* Src = State->PixelBuf[ReadIdx & 1];

        INT W = (INT)Hdr->Width;
        INT H = (INT)Hdr->Height;
        if (W > 0 && H > 0 && W <= OL_BROWSER_MAX_WIDTH && H <= OL_BROWSER_MAX_HEIGHT)
        {
            INT FrameSize = W * H * 4;
            TArray<BYTE> Frame;
            Frame.Add(FrameSize);
            appMemcpy(Frame.GetData(), Src, FrameSize);
            BrowserTexture->UpdateMip(0, Frame);
        }
    }

    // Crosshair -> cursor: mathematical ray-plane intersection with TargetScreen mesh.
    // OLBikScreen has no collision (bCollideActors=false), so we bypass SingleLineCheck
    // and intersect the camera ray directly with the screen plane.
    // ScreenMeshComponent is set by SetScreenMesh() called from UnrealScript Tick.
    UStaticMeshComponent* Mesh = (UStaticMeshComponent*)ScreenMeshComponent;
    if (Mesh == NULL)
        return bResult;

    FVector CamLoc;
    FRotator CamRot;
    if (!GEngine || GEngine->GamePlayers.Num() == 0
        || !GEngine->GamePlayers(0) || !GEngine->GamePlayers(0)->Actor)
        return bResult;

    GEngine->GamePlayers(0)->Actor->eventGetPlayerViewPoint(CamLoc, CamRot);

    // Plane normal = mesh local Y axis in world space.
    // security_screen lies in its local XZ plane, so Y is the face normal.
    FVector PlaneNormal = Mesh->LocalToWorld.GetAxis(1).SafeNormal();
    FVector PlaneOrigin = Mesh->Bounds.Origin; // world-space centre

    FVector RayDir = FRotationMatrix(CamRot).GetAxis(0); // camera forward

    // Ray-plane: t = dot(PlaneOrigin - CamLoc, Normal) / dot(RayDir, Normal)
    FLOAT Denom = RayDir | PlaneNormal;
    if (Denom > -0.0001f && Denom < 0.0001f)
        return bResult; // ray parallel to plane

    FLOAT t = ((PlaneOrigin - CamLoc) | PlaneNormal) / Denom;
    if (t < 0.0f || t > 10000.0f)
        return bResult; // plane behind camera or too far

    FVector HitWorld = CamLoc + RayDir * t;

    // Transform hit point into mesh local space
    FMatrix WorldToLocal = Mesh->LocalToWorld.Inverse();
    FVector LocalHit     = WorldToLocal.TransformFVector(HitWorld);
    FVector LocalOrigin  = WorldToLocal.TransformFVector(PlaneOrigin);
    FVector Rel = LocalHit - LocalOrigin;

    // Rel is in unscaled local space (WorldToLocal already divides out Scale3D),
    // so compare against the raw asset extents without re-applying Scale.
    FLOAT HalfW = Mesh->StaticMesh ? Mesh->StaticMesh->Bounds.BoxExtent.X : 50.f;
    FLOAT HalfH = Mesh->StaticMesh ? Mesh->StaticMesh->Bounds.BoxExtent.Z : 50.f;
    if (HalfW < 1.0f) HalfW = 1.0f;
    if (HalfH < 1.0f) HalfH = 1.0f;

    // If crosshair misses the screen, release any held click and clear cursor
    if (Rel.X < -HalfW || Rel.X > HalfW || Rel.Z < -HalfH || Rel.Z > HalfH)
    {
        // Send mouse-up if the button was held while the player looked away
        if (State->bLastUseDown && State->InputPipe != INVALID_HANDLE_VALUE)
        {
            FOLBrowserInputEvent Ev;
            appMemzero(&Ev, sizeof(Ev));
            Ev.Type   = OL_INPUT_MOUSE_UP;
            Ev.Button = 0;
            Ev.X      = State->LastCursorX;
            Ev.Y      = State->LastCursorY;
            SendBrowserInput(State->InputPipe, Ev);
        }
        State->bLastUseDown = FALSE;
        State->LastCursorX  = -1;
        State->LastCursorY  = -1;
        bFocused = FALSE;
        return bResult;
    }

    // U: local X (right→left, negated), V: local Z (bottom→top, negated+flipped)
    FLOAT U = (-Rel.X / HalfW) * 0.5f + 0.5f;
    FLOAT V = 1.0f - ((Rel.Z / HalfH) * 0.5f + 0.5f);

    U = U < 0.0f ? 0.0f : (U > 1.0f ? 1.0f : U);
    V = V < 0.0f ? 0.0f : (V > 1.0f ? 1.0f : V);

    INT CX = (INT)(U * (FLOAT)BrowserWidth);
    INT CY = (INT)(V * (FLOAT)BrowserHeight);

    bFocused = TRUE;

    if (CX != State->LastCursorX || CY != State->LastCursorY)
    {
        State->LastCursorX = CX;
        State->LastCursorY = CY;

        if (State->InputPipe != INVALID_HANDLE_VALUE)
        {
            FOLBrowserInputEvent Ev;
            appMemzero(&Ev, sizeof(Ev));
            Ev.Type = OL_INPUT_MOUSE_MOVE;
            Ev.X    = CX;
            Ev.Y    = CY;
            SendBrowserInput(State->InputPipe, Ev);
        }
    }

    // Forward use-button (E / LMB) as left mouse button click to the browser.
    // bUseButtonDown lives on AOLPlayerController (input byte).
    AOLPlayerController* PC = Cast<AOLPlayerController>(GEngine->GamePlayers(0)->Actor);
    UBOOL bUseNow = (PC && PC->bUseButtonDown != 0) ? TRUE : FALSE;

    if (bUseNow != State->bLastUseDown && State->InputPipe != INVALID_HANDLE_VALUE)
    {
        FOLBrowserInputEvent Ev;
        appMemzero(&Ev, sizeof(Ev));
        Ev.Type   = bUseNow ? OL_INPUT_MOUSE_DOWN : OL_INPUT_MOUSE_UP;
        Ev.Button = 0;
        Ev.X      = State->LastCursorX;
        Ev.Y      = State->LastCursorY;
        SendBrowserInput(State->InputPipe, Ev);
        State->bLastUseDown = bUseNow;
    }

    return bResult;
}
