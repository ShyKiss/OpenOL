/*=============================================================================
    OLVideoPlayer.cpp: Streams video frames via yt-dlp + ffmpeg into Texture2DDynamic.
    Audio is streamed via a second ffmpeg pipe into the Wwise AkAudioInput plugin.
=============================================================================*/

#include "OLGame.h"
#include <AK/Plugin/AkAudioInputSourceFactory.h>

IMPLEMENT_CLASS(AOLVideoPlayer);

// ----------------------------------------------------------------------------
// FVideoDecodeThread — reads raw BGRA frames from ffmpeg stdout pipe.
// Uses a lock-free "ready slot": decode thread writes to a back buffer and
// atomically publishes it; game thread swaps it out without ever blocking.
// ----------------------------------------------------------------------------

class FVideoDecodeThread : public FRunnable
{
public:
    INT     Width;
    INT     Height;
    HANDLE  PipeRead;
    HANDLE  ProcessHandle;
    UBOOL   bStopped;

    // Lock-free single-slot exchange:
    // Decode thread writes to Back[], then InterlockedExchange's ReadyIdx.
    // Game thread reads ReadyIdx; if >=0, swaps with -1 and uploads that slot.
    BYTE*           Slots[2];       // two raw frame buffers
    volatile LONG   ReadyIdx;       // -1 = nothing ready, 0 or 1 = slot index

    FVideoDecodeThread()
    :   Width(0), Height(0)
    ,   PipeRead(INVALID_HANDLE_VALUE)
    ,   ProcessHandle(INVALID_HANDLE_VALUE)
    ,   bStopped(FALSE), ReadyIdx(-1)
    {
        Slots[0] = Slots[1] = NULL;
    }

    virtual UBOOL Init()
    {
        INT FrameSize = Width * Height * 4;
        Slots[0] = (BYTE*)appMalloc(FrameSize);
        Slots[1] = (BYTE*)appMalloc(FrameSize);
        return (Slots[0] && Slots[1]) ? TRUE : FALSE;
    }

    virtual DWORD Run()
    {
        const INT  FrameSize       = Width * Height * 4;
        const DWORD FrameIntervalMs = 1000 / 30;
        INT BackIdx = 0; // which slot decode thread writes into

        while (!bStopped)
        {
            DWORD FrameStart = GetTickCount();

            // Read one full frame into back slot
            DWORD TotalRead = 0;
            while ((INT)TotalRead < FrameSize && !bStopped)
            {
                DWORD BytesRead = 0;
                BOOL ok = ReadFile(PipeRead,
                    Slots[BackIdx] + TotalRead,
                    FrameSize - TotalRead,
                    &BytesRead, NULL);
                if (!ok || BytesRead == 0) { bStopped = TRUE; break; }
                TotalRead += BytesRead;
            }

            if ((INT)TotalRead == FrameSize)
            {
                // Publish this slot; game thread will consume it
                InterlockedExchange(&ReadyIdx, (LONG)BackIdx);
                // Switch to the other slot for next frame
                BackIdx ^= 1;
            }

            // Throttle to ~30fps
            DWORD Elapsed = GetTickCount() - FrameStart;
            if (Elapsed < FrameIntervalMs)
                Sleep(FrameIntervalMs - Elapsed);
        }
        return 0;
    }

    virtual void Stop() { bStopped = TRUE; }

    virtual void Exit()
    {
        if (PipeRead != INVALID_HANDLE_VALUE)
        {
            CloseHandle(PipeRead);
            PipeRead = INVALID_HANDLE_VALUE;
        }
        appFree(Slots[0]); Slots[0] = NULL;
        appFree(Slots[1]); Slots[1] = NULL;
    }
};

// ----------------------------------------------------------------------------
// FAudioDecodeThread — reads PCM s16le stereo 44100 from ffmpeg pipe into a
// lock-free ring buffer. The Wwise AkAudioInput callback drains it on demand.
// ----------------------------------------------------------------------------

#define AUDIO_BUFFER_BYTES  (44100 * 2 * 2)  // 1 second of s16le stereo

class FAudioDecodeThread : public FRunnable
{
public:
    HANDLE          PipeRead;
    UBOOL           bStopped;
    AkPlayingID     WwisePlayingID;

    // Ring buffer: decode thread writes, Wwise callback reads
    BYTE*           RingBuf;
    volatile LONG   WritePos;   // bytes written (mod AUDIO_BUFFER_BYTES)
    volatile LONG   ReadPos;    // bytes read    (mod AUDIO_BUFFER_BYTES)

    // Spatialization: set by game thread in Tick, read by Wwise callback
    // Volume: fixed-point Q16 (0x10000 = 1.0), Pan: signed Q16 (-0x10000=full left, +0x10000=full right)
    volatile LONG   SpatialVolume; // 0..0x10000
    volatile LONG   SpatialPan;    // -0x10000..+0x10000

    FAudioDecodeThread()
    :   PipeRead(INVALID_HANDLE_VALUE), bStopped(FALSE)
    ,   WwisePlayingID(AK_INVALID_PLAYING_ID)
    ,   RingBuf(NULL), WritePos(0), ReadPos(0)
    ,   SpatialVolume(0x10000), SpatialPan(0)
    {}

    virtual UBOOL Init()
    {
        RingBuf = (BYTE*)appMalloc(AUDIO_BUFFER_BYTES);
        appMemzero(RingBuf, AUDIO_BUFFER_BYTES);
        return RingBuf ? TRUE : FALSE;
    }

    virtual DWORD Run()
    {
        debugf(TEXT("[OLVideoPlayer] AudioThread::Run enter, PipeRead valid=%d"), PipeRead != INVALID_HANDLE_VALUE ? 1 : 0);

        // ConnectNamedPipe: wait for ffmpeg to open the named pipe (background thread — safe)
        BOOL bConnected = ConnectNamedPipe(PipeRead, NULL);
        DWORD ConnErr = GetLastError();
        debugf(TEXT("[OLVideoPlayer] AudioThread ConnectNamedPipe: connected=%d err=%u"), (INT)bConnected, (UINT)ConnErr);
        if (!bConnected && ConnErr != ERROR_PIPE_CONNECTED)
        {
            debugf(NAME_Warning, TEXT("[OLVideoPlayer] AudioThread pipe connect failed, aborting"));
            return 1;
        }

        debugf(TEXT("[OLVideoPlayer] AudioThread pipe connected, starting read loop"));
        const INT ChunkSize = 4096; // bytes per read
        BYTE Tmp[4096];

        while (!bStopped)
        {
            // How much space is free in the ring buffer?
            LONG W = WritePos;
            LONG R = ReadPos;
            INT  Free = AUDIO_BUFFER_BYTES - (INT)((W - R + AUDIO_BUFFER_BYTES) % AUDIO_BUFFER_BYTES) - 1;

            if (Free < ChunkSize)
            {
                // Buffer nearly full — wait for Wwise to consume
                Sleep(5);
                continue;
            }

            DWORD BytesRead = 0;
            BOOL ok = ReadFile(PipeRead, Tmp, ChunkSize, &BytesRead, NULL);
            if (!ok || BytesRead == 0) { bStopped = TRUE; break; }

            // Write into ring buffer (wrapping)
            INT Pos = (INT)(W % AUDIO_BUFFER_BYTES);
            INT FirstPart = ((INT)BytesRead < AUDIO_BUFFER_BYTES - Pos) ? (INT)BytesRead : (AUDIO_BUFFER_BYTES - Pos);
            appMemcpy(RingBuf + Pos, Tmp, FirstPart);
            if (FirstPart < (INT)BytesRead)
                appMemcpy(RingBuf, Tmp + FirstPart, BytesRead - FirstPart);

            InterlockedExchangeAdd(&WritePos, (LONG)BytesRead);
        }
        return 0;
    }

    // Called by Wwise callback — drain up to io_pBufferOut->MaxFrames samples
    void FillWwiseBuffer(AkAudioBuffer* io_pBufferOut)
    {
        LONG W = WritePos;
        LONG R = ReadPos;
        INT  Available = (INT)((W - R + AUDIO_BUFFER_BYTES * 2) % (AUDIO_BUFFER_BYTES * 2));
        // Clamp to ring buffer size
        if (Available > AUDIO_BUFFER_BYTES) Available = AUDIO_BUFFER_BYTES;

        INT SamplesNeeded = (INT)io_pBufferOut->MaxFrames();  // frames (stereo = 2 samples/frame)
        INT BytesNeeded   = SamplesNeeded * 2 * 2;            // s16le stereo
        INT BytesToCopy   = Available < BytesNeeded ? Available : BytesNeeded;
        INT FramesCopied  = BytesToCopy / 4;

        if (FramesCopied == 0)
        {
            // Underrun — output silence to keep Wwise stream alive
            io_pBufferOut->uValidFrames = (AkUInt16)SamplesNeeded;
            return; // channels already zeroed in AudioInputExecute before calling FillWwiseBuffer
        }

        // Read spatialization params (set by game thread)
        FLOAT Vol = (FLOAT)SpatialVolume / 65536.0f;  // 0..1
        FLOAT Pan = (FLOAT)SpatialPan    / 65536.0f;  // -1..+1 (negative = left, positive = right)
        // Constant-power stereo pan: left = cos(angle), right = sin(angle), angle in [0..pi/2]
        FLOAT PanAngle = (Pan + 1.0f) * 0.5f * 1.5707963f; // map [-1,1] → [0, pi/2]
        FLOAT GainL = Vol * appCos(PanAngle);
        FLOAT GainR = Vol * appSin(PanAngle);

        // Convert s16le interleaved → Wwise float32 deinterleaved, apply volume+pan
        AkReal32* ChL = io_pBufferOut->GetChannel(0);
        AkReal32* ChR = io_pBufferOut->GetChannel(1);
        INT Pos = (INT)((R % AUDIO_BUFFER_BYTES + AUDIO_BUFFER_BYTES) % AUDIO_BUFFER_BYTES);

        for (INT i = 0; i < FramesCopied; i++)
        {
            INT Idx = (Pos + i * 4) % AUDIO_BUFFER_BYTES;
            INT16 L = (INT16)(RingBuf[Idx]     | (RingBuf[(Idx+1) % AUDIO_BUFFER_BYTES] << 8));
            INT16 R2= (INT16)(RingBuf[(Idx+2) % AUDIO_BUFFER_BYTES] | (RingBuf[(Idx+3) % AUDIO_BUFFER_BYTES] << 8));
            FLOAT Mono = (L + R2) * 0.5f / 32768.0f; // mix to mono, then pan
            ChL[i] = Mono * GainL;
            ChR[i] = Mono * GainR;
        }

        InterlockedExchangeAdd(&ReadPos, (LONG)(FramesCopied * 4));
        io_pBufferOut->uValidFrames = (AkUInt16)FramesCopied;
    }

    virtual void Stop() { bStopped = TRUE; }

    virtual void Exit()
    {
        if (RingBuf) { appFree(RingBuf); RingBuf = NULL; }
        if (PipeRead != INVALID_HANDLE_VALUE)
        {
            CloseHandle(PipeRead);
            PipeRead = INVALID_HANDLE_VALUE;
        }
    }
};

// ----------------------------------------------------------------------------
// Global map: AkPlayingID → FAudioDecodeThread* for Wwise callbacks
// ----------------------------------------------------------------------------

static FCriticalSection GAudioMapLock;
static TMap<AkPlayingID, FAudioDecodeThread*> GAudioThreadMap;

// Wwise AkAudioInput callback — called from Wwise audio thread to fill a buffer
static void AudioInputExecute(AkPlayingID in_PlayingID, AkAudioBuffer* io_pBufferOut)
{
    // Zero out channels so underrun = silence, not garbage
    for (AkUInt32 ch = 0; ch < io_pBufferOut->NumChannels(); ch++)
        appMemzero(io_pBufferOut->GetChannel(ch), io_pBufferOut->MaxFrames() * sizeof(AkReal32));
    io_pBufferOut->uValidFrames = io_pBufferOut->MaxFrames(); // default: silence but keep going
    io_pBufferOut->eState       = AK_DataReady;              // must always be set or Wwise stops

    FAudioDecodeThread** ppThread = NULL;
    {
        FScopeLock SL(&GAudioMapLock);
        ppThread = GAudioThreadMap.Find(in_PlayingID);
    }

    if (ppThread && *ppThread)
        (*ppThread)->FillWwiseBuffer(io_pBufferOut);
}

// Wwise AkAudioInput format callback — tells Wwise what PCM format we provide
static void AudioInputGetFormat(AkPlayingID in_PlayingID, AkAudioFormat& io_AudioFormat)
{
    io_AudioFormat.SetAll(
        44100,                          // sample rate
        AK_SPEAKER_SETUP_STEREO,        // channel config (stereo)
        32,                             // bits per sample (AkReal32 = 32 bit float)
        sizeof(AkReal32) * 2,           // block align: 2 channels × 4 bytes
        AK_FLOAT,                       // data type: float32 (Wwise native)
        AK_NONINTERLEAVED               // deinterleaved channels
    );
}

// Gain callback — return 1.0 (full volume, Wwise handles mixing)
static AkReal32 AudioInputGetGain(AkPlayingID in_PlayingID)
{
    return 1.0f;
}

// ----------------------------------------------------------------------------
// FStartupParams — passed to the startup thread (resolves URL + launches ffmpeg)
// ----------------------------------------------------------------------------

struct FStartupParams
{
    FString BinDir;
    FString URL;
    INT     Width;
    INT     Height;
    // Output — written by startup thread, read by game thread after bDone
    HANDLE  VideoProc;
    HANDLE  VideoPipe;
    HANDLE  AudioPipe;
    volatile LONG bDone;   // 1 = startup finished (success or fail)

    FStartupParams()
    :   Width(0), Height(0)
    ,   VideoProc(INVALID_HANDLE_VALUE)
    ,   VideoPipe(INVALID_HANDLE_VALUE)
    ,   AudioPipe(INVALID_HANDLE_VALUE)
    ,   bDone(0)
    {}
};

static DWORD WINAPI StartupThreadProc(LPVOID pParam);

// Internal state stored in NativeHandle
struct FOLVideoPlayerState
{
    FVideoDecodeThread* Thread;
    FRunnableThread*    RunnableThread;
    HANDLE              FfmpegProcess;

    FAudioDecodeThread* AudioThread;
    FRunnableThread*    AudioRunnableThread;

    // Startup thread — resolves URL and launches ffmpeg off the game thread
    HANDLE          StartupThread;
    FStartupParams* StartupParams;

    FOLVideoPlayerState()
    :   Thread(NULL), RunnableThread(NULL)
    ,   FfmpegProcess(INVALID_HANDLE_VALUE)
    ,   AudioThread(NULL), AudioRunnableThread(NULL)
    ,   StartupThread(INVALID_HANDLE_VALUE), StartupParams(NULL)
    {}
};

// ----------------------------------------------------------------------------
// Helpers
// ----------------------------------------------------------------------------

// Get directory containing yt-dlp.exe and ffmpeg.exe.
// Checks <ExeDir>\Tools\ first, then falls back to <ExeDir>\.
static FString GetBinDir()
{
    TCHAR ExePath[MAX_PATH];
    GetModuleFileNameW(NULL, ExePath, MAX_PATH);
    FFilename F(ExePath);
    FString ToolsDir = F.GetPath() + TEXT("\\Tools\\");
    if (GetFileAttributesW(*ToolsDir) != INVALID_FILE_ATTRIBUTES)
        return ToolsDir;
    return F.GetPath() + TEXT("\\");
}

// Run a process and capture its stdout as a string
static FString RunAndCapture(const FString& Cmd)
{
    SECURITY_ATTRIBUTES sa;
    appMemzero(&sa, sizeof(sa));
    sa.nLength = sizeof(sa);
    sa.bInheritHandle = TRUE;

    HANDLE PipeRead, PipeWrite;
    if (!CreatePipe(&PipeRead, &PipeWrite, &sa, 0))
        return TEXT("");

    SetHandleInformation(PipeRead, HANDLE_FLAG_INHERIT, 0);

    STARTUPINFOW si;
    appMemzero(&si, sizeof(si));
    si.cb = sizeof(si);
    si.dwFlags = STARTF_USESTDHANDLES;
    si.hStdOutput = PipeWrite;
    si.hStdError  = GetStdHandle(STD_ERROR_HANDLE);
    si.hStdInput  = GetStdHandle(STD_INPUT_HANDLE);

    PROCESS_INFORMATION pi;
    appMemzero(&pi, sizeof(pi));
    TArray<TCHAR> CmdBuf;
    CmdBuf.AddZeroed(Cmd.Len() + 1);
    appMemcpy(CmdBuf.GetData(), *Cmd, (Cmd.Len() + 1) * sizeof(TCHAR));

    if (!CreateProcessW(NULL, CmdBuf.GetData(), NULL, NULL, TRUE, CREATE_NO_WINDOW, NULL, NULL, &si, &pi))
    {
        CloseHandle(PipeRead);
        CloseHandle(PipeWrite);
        return TEXT("");
    }

    CloseHandle(PipeWrite);
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);

    FString Result;
    char Buf[4096];
    DWORD BytesRead;
    while (ReadFile(PipeRead, Buf, sizeof(Buf) - 1, &BytesRead, NULL) && BytesRead > 0)
    {
        Buf[BytesRead] = '\0';
        Result += ANSI_TO_TCHAR(Buf);
    }
    CloseHandle(PipeRead);

    // Trim whitespace/newlines
    Result = Result.Trim();
    return Result;
}

// Launch a single ffmpeg process with both video and audio outputs:
//   stdout        → raw BGRA video frames
//   named pipe    → s16le stereo 44100 audio
// Audio named pipe is created first (server side), ffmpeg connects as client.
// ConnectNamedPipe is called AFTER ffmpeg starts so it doesn't block launch.
static HANDLE LaunchFfmpegAV(const FString& BinDir, const FString& StreamURL,
    INT Width, INT Height, HANDLE& OutProcess, HANDLE& OutAudioPipe)
{
    OutAudioPipe = INVALID_HANDLE_VALUE;

    // Unique name per instance
    static INT PipeSeq = 0;
    FString AudioPipeName = FString::Printf(TEXT("\\\\.\\pipe\\OLAudio%04d"), PipeSeq++);

    // Create named pipe server for audio.
    // ConnectNamedPipe will be called inside FAudioDecodeThread::Init() (background thread)
    // so the game thread never blocks waiting for ffmpeg to open the pipe.
    HANDLE AudioServer = CreateNamedPipeW(
        *AudioPipeName,
        PIPE_ACCESS_INBOUND,
        PIPE_TYPE_BYTE | PIPE_READMODE_BYTE | PIPE_WAIT,
        1,
        AUDIO_BUFFER_BYTES,
        AUDIO_BUFFER_BYTES,
        5000, NULL);
    if (AudioServer == INVALID_HANDLE_VALUE)
        return INVALID_HANDLE_VALUE;

    // Create anonymous pipe for video stdout
    SECURITY_ATTRIBUTES sa;
    appMemzero(&sa, sizeof(sa));
    sa.nLength = sizeof(sa);
    sa.bInheritHandle = TRUE;

    HANDLE VideoPipeRead, VideoPipeWrite;
    if (!CreatePipe(&VideoPipeRead, &VideoPipeWrite, &sa, Width * Height * 4 * 4))
    {
        CloseHandle(AudioServer);
        return INVALID_HANDLE_VALUE;
    }
    SetHandleInformation(VideoPipeRead, HANDLE_FLAG_INHERIT, 0);

    // Single ffmpeg: video → stdout, audio → named pipe (both from same input/clock)
    // Note: named pipe path without quotes so ffmpeg recognises it as a pipe URI
    FString Cmd = FString::Printf(
        TEXT("\"%sffmpeg.exe\" -re -i \"%s\"")
        TEXT(" -map 0:v -f rawvideo -pix_fmt bgra -r 30 -vf scale=%d:%d pipe:1")
        TEXT(" -map 0:a -f s16le -ar 44100 -ac 2 %s"),
        *BinDir, *StreamURL, Width, Height, *AudioPipeName);

    debugf(TEXT("[OLVideoPlayer] ffmpeg cmd: %s"), *Cmd);

    STARTUPINFOW si;
    appMemzero(&si, sizeof(si));
    si.cb = sizeof(si);
    si.dwFlags    = STARTF_USESTDHANDLES;
    si.hStdOutput = VideoPipeWrite;
    si.hStdError  = GetStdHandle(STD_ERROR_HANDLE);
    si.hStdInput  = GetStdHandle(STD_INPUT_HANDLE);

    PROCESS_INFORMATION pi;
    appMemzero(&pi, sizeof(pi));
    TArray<TCHAR> CmdBuf;
    CmdBuf.AddZeroed(Cmd.Len() + 1);
    appMemcpy(CmdBuf.GetData(), *Cmd, (Cmd.Len() + 1) * sizeof(TCHAR));

    if (!CreateProcessW(NULL, CmdBuf.GetData(), NULL, NULL, TRUE, CREATE_NO_WINDOW, NULL, NULL, &si, &pi))
    {
        CloseHandle(VideoPipeRead);
        CloseHandle(VideoPipeWrite);
        CloseHandle(AudioServer);
        OutProcess = INVALID_HANDLE_VALUE;
        return INVALID_HANDLE_VALUE;
    }
    CloseHandle(VideoPipeWrite);
    CloseHandle(pi.hThread);
    OutProcess = pi.hProcess;

    // Do NOT call ConnectNamedPipe here — it blocks the game thread.
    // FAudioDecodeThread::Init() will connect from the background thread.
    OutAudioPipe = AudioServer;

    return VideoPipeRead;
}

// Startup thread: resolves URL via yt-dlp and launches ffmpeg — all off game thread
static DWORD WINAPI StartupThreadProc(LPVOID pParam)
{
    FStartupParams* P = (FStartupParams*)pParam;

    debugf(TEXT("[OLVideoPlayer] BinDir: %s"), *P->BinDir);

    // Step 1: resolve stream URL via yt-dlp
    FString YtdlpCmd = FString::Printf(
        TEXT("\"%syt-dlp.exe\" --no-playlist --cookies \"%syt-cookies.txt\" -f b --get-url \"%s\""),
        *P->BinDir, *P->BinDir, *P->URL);

    debugf(TEXT("[OLVideoPlayer] yt-dlp cmd: %s"), *YtdlpCmd);

    FString StreamURL = RunAndCapture(YtdlpCmd);
    debugf(TEXT("[OLVideoPlayer] StreamURL len=%d"), StreamURL.Len());

    INT NewLine = StreamURL.InStr(TEXT("\n"));
    if (NewLine != INDEX_NONE)
        StreamURL = StreamURL.Left(NewLine).Trim();

    if (StreamURL.IsEmpty())
    {
        debugf(NAME_Warning, TEXT("[OLVideoPlayer] yt-dlp returned empty URL"));
        InterlockedExchange(&P->bDone, 1L);
        return 0;
    }

    // Step 2: launch ffmpeg (video + audio)
    P->VideoPipe = LaunchFfmpegAV(P->BinDir, StreamURL,
        P->Width, P->Height, P->VideoProc, P->AudioPipe);

    debugf(TEXT("[OLVideoPlayer] VideoPipe valid: %d, AudioPipe valid: %d"),
        P->VideoPipe != INVALID_HANDLE_VALUE ? 1 : 0,
        P->AudioPipe != INVALID_HANDLE_VALUE ? 1 : 0);

    // Signal done (success or fail)
    InterlockedExchange(&P->bDone, 1L);
    return 0;
}

// ----------------------------------------------------------------------------
// AOLVideoPlayer — native methods
// UMake generates execPlay/execStop wrappers that call Play()/Stop()
// ----------------------------------------------------------------------------

void AOLVideoPlayer::Play(const FString& URL)
{
    // Stop any existing playback first
    Stop();

    INT W = (VideoWidth  > 0) ? VideoWidth  : 1280;
    INT H = (VideoHeight > 0) ? VideoHeight : 720;

    // Create texture immediately so TargetScreen can be assigned right away
    if (VideoTexture == NULL || VideoTexture->SizeX != W || VideoTexture->SizeY != H)
    {
        VideoTexture = Cast<UTexture2DDynamic>(
            StaticConstructObject(UTexture2DDynamic::StaticClass(),
                GetTransientPackage(), NAME_None, RF_Transient));
        if (VideoTexture)
        {
            VideoTexture->CompressionNone    = TRUE;
            VideoTexture->MipGenSettings     = TMGS_NoMipmaps;
            VideoTexture->CompressionNoAlpha = TRUE;
            VideoTexture->bNoTiling          = TRUE;
            VideoTexture->Init(W, H, PF_A8R8G8B8);
        }
    }
    if (!VideoTexture) return;

    // Launch startup thread — resolves URL and starts ffmpeg without blocking game thread
    FOLVideoPlayerState* State = new FOLVideoPlayerState();
    FStartupParams* Params = new FStartupParams();
    Params->BinDir = GetBinDir();
    Params->URL    = URL;
    Params->Width  = W;
    Params->Height = H;

    State->StartupParams = Params;
    State->StartupThread = CreateThread(NULL, 0, StartupThreadProc, Params, 0, NULL);

    NativeHandle = State;
    VideoWidth   = W;
    VideoHeight  = H;
    bPlaying     = TRUE;

    debugf(TEXT("[OLVideoPlayer] Startup thread launched for: %s"), *URL);
}

void AOLVideoPlayer::Stop()
{
    if (!NativeHandle)
        return;

    FOLVideoPlayerState* State = (FOLVideoPlayerState*)NativeHandle;

    // Kill startup thread if still running
    if (State->StartupThread != INVALID_HANDLE_VALUE)
    {
        TerminateThread(State->StartupThread, 0);
        CloseHandle(State->StartupThread);
        State->StartupThread = INVALID_HANDLE_VALUE;
    }
    if (State->StartupParams)
    {
        // Close any handles the startup thread may have already filled
        if (State->StartupParams->VideoPipe != INVALID_HANDLE_VALUE)
            CloseHandle(State->StartupParams->VideoPipe);
        if (State->StartupParams->AudioPipe != INVALID_HANDLE_VALUE)
            CloseHandle(State->StartupParams->AudioPipe);
        if (State->StartupParams->VideoProc != INVALID_HANDLE_VALUE)
        {
            TerminateProcess(State->StartupParams->VideoProc, 0);
            CloseHandle(State->StartupParams->VideoProc);
        }
        delete State->StartupParams;
        State->StartupParams = NULL;
    }

    if (State->Thread)
        State->Thread->Stop();

    if (State->RunnableThread)
    {
        State->RunnableThread->Kill(TRUE);
        GThreadFactory->Destroy(State->RunnableThread);
        State->RunnableThread = NULL;
    }
    if (State->Thread)
    {
        delete State->Thread;
        State->Thread = NULL;
    }
    if (State->FfmpegProcess != INVALID_HANDLE_VALUE)
    {
        TerminateProcess(State->FfmpegProcess, 0);
        CloseHandle(State->FfmpegProcess);
        State->FfmpegProcess = INVALID_HANDLE_VALUE;
    }

    // Stop Wwise audio input and remove from global map
    if (State->AudioThread)
    {
        FAudioDecodeThread* AT = State->AudioThread;
        if (AT->WwisePlayingID != AK_INVALID_PLAYING_ID)
        {
            StopAudioInput(AT->WwisePlayingID);
            {
                FScopeLock SL(&GAudioMapLock);
                GAudioThreadMap.Remove(AT->WwisePlayingID);
            }
            // Unregister game object
            AkGameObjectID ObjID = (AkGameObjectID)(PTRINT)this;
            AK::SoundEngine::UnregisterGameObj(ObjID);
        }
        AT->Stop();
    }

    if (State->AudioRunnableThread)
    {
        State->AudioRunnableThread->Kill(TRUE);
        GThreadFactory->Destroy(State->AudioRunnableThread);
        State->AudioRunnableThread = NULL;
    }
    if (State->AudioThread)
    {
        delete State->AudioThread;
        State->AudioThread = NULL;
    }
    // FfmpegProcess (single process) handles both A/V — already terminated above

    delete State;
    NativeHandle = NULL;
    bPlaying = FALSE;
}

void AOLVideoPlayer::BeginDestroy()
{
    Stop();
    Super::BeginDestroy();
}

UBOOL AOLVideoPlayer::Tick(FLOAT DeltaTime, ELevelTick TickType)
{
    UBOOL bResult = Super::Tick(DeltaTime, TickType);

    if (!NativeHandle || !VideoTexture || !bPlaying)
        return bResult;

    FOLVideoPlayerState* State = (FOLVideoPlayerState*)NativeHandle;

    // Check if startup thread finished and pick up the ffmpeg handles
    LONG DoneVal = State->StartupParams ? InterlockedExchangeAdd(&State->StartupParams->bDone, 0L) : -1L;

    if (State->StartupParams && DoneVal == 1)
    {
        FStartupParams* P = State->StartupParams;
        if (P->VideoPipe != INVALID_HANDLE_VALUE)
        {
            // Wire up video decode thread
            FVideoDecodeThread* VT = new FVideoDecodeThread();
            VT->Width         = P->Width;
            VT->Height        = P->Height;
            VT->PipeRead      = P->VideoPipe;
            VT->ProcessHandle = P->VideoProc;
            State->Thread         = VT;
            State->RunnableThread = GThreadFactory->CreateThread(VT, TEXT("OLVideoDecodeThread"), FALSE, FALSE, 0, TPri_Normal);
            State->FfmpegProcess  = P->VideoProc;

            // Wire up audio decode thread → Wwise AkAudioInput
            if (P->AudioPipe != INVALID_HANDLE_VALUE)
            {
                FAudioDecodeThread* AT = new FAudioDecodeThread();
                AT->PipeRead = P->AudioPipe;

                // Register a Wwise game object for this video player instance
                AkGameObjectID ObjID = (AkGameObjectID)(PTRINT)this;
                AKRESULT RegResult = AK::SoundEngine::RegisterGameObj(ObjID, "OLVideoPlayer");
                debugf(TEXT("[OLVideoPlayer] RegisterGameObj result=%d"), (INT)RegResult);

                // Bind to listener 0 (player camera) so Wwise applies 3D spatialization
                AK::SoundEngine::SetActiveListeners(ObjID, 0x01);

                // Set callbacks: Wwise will call AudioInputExecute each audio frame
                SetAudioInputCallbacks(AudioInputExecute, AudioInputGetFormat, AudioInputGetGain);

                // Start AkAudioInput plugin playback (no event/bank needed)
                AkPlayingID PlayID = AK_INVALID_PLAYING_ID;
                AKRESULT PlayResult = PlayAudioInput(PlayID, ObjID);
                debugf(TEXT("[OLVideoPlayer] PlayAudioInput result=%d PlayingID=%u"), (INT)PlayResult, (UINT)PlayID);
                AT->WwisePlayingID = PlayID;

                // Register in global map so the callback finds this thread
                {
                    FScopeLock SL(&GAudioMapLock);
                    GAudioThreadMap.Set(PlayID, AT);
                }

                State->AudioThread         = AT;
                State->AudioRunnableThread = GThreadFactory->CreateThread(AT, TEXT("OLAudioDecodeThread"), FALSE, FALSE, 0, TPri_AboveNormal);

                // Verify ring buffer is filling after a short delay (checked in next Tick)
                debugf(TEXT("[OLVideoPlayer] Audio thread started, WwisePlayingID=%u"), (UINT)PlayID);
            }
            debugf(TEXT("[OLVideoPlayer] Playback started (%dx%d)"), P->Width, P->Height);
        }
        else
        {
            debugf(NAME_Warning, TEXT("[OLVideoPlayer] Startup failed (yt-dlp or ffmpeg error)"));
            bPlaying = FALSE;
        }

        // Cleanup startup thread handle and params
        if (State->StartupThread != INVALID_HANDLE_VALUE)
        {
            CloseHandle(State->StartupThread);
            State->StartupThread = INVALID_HANDLE_VALUE;
        }
        delete P;
        State->StartupParams = NULL;
    }

    FVideoDecodeThread* Thread = State->Thread;
    if (!Thread)
        return bResult;

    // Check if thread finished (ffmpeg pipe closed)
    if (Thread->bStopped)
    {
        bPlaying = FALSE;
        return bResult;
    }

    // Manual 3D spatialization: PlayAudioInput bypasses Wwise positioning,
    // so we compute volume (distance falloff) and pan (angle) ourselves.
    if (State->AudioThread)
    {
        AActor* SoundSource = (TargetScreen != NULL) ? (AActor*)TargetScreen : (AActor*)this;
        FVector SourceLoc = SoundSource->Location;

        FVector  CamLoc = SourceLoc;
        FRotator CamRot;
        if (GEngine && GEngine->GamePlayers.Num() > 0 && GEngine->GamePlayers(0) && GEngine->GamePlayers(0)->Actor)
            GEngine->GamePlayers(0)->Actor->eventGetPlayerViewPoint(CamLoc, CamRot);

        // Distance falloff: full volume within MinDist, zero at MaxDist
        const FLOAT MinDist = 200.0f;
        const FLOAT MaxDist = 5000.0f;
        FLOAT Dist = (SourceLoc - CamLoc).Size();
        FLOAT Vol  = 1.0f;
        if (Dist > MaxDist)
            Vol = 0.0f;
        else if (Dist > MinDist)
        {
            FLOAT t = (Dist - MinDist) / (MaxDist - MinDist);
            Vol = (1.0f - t) * (1.0f - t); // quadratic falloff
        }

        // Stereo pan from camera-relative horizontal angle
        FLOAT Pan = 0.0f;
        if (Dist > 1.0f)
        {
            // Vector from camera to source in camera space
            FVector ToSource   = (SourceLoc - CamLoc).SafeNormal();
            FVector CamRight   = FRotationMatrix(CamRot).GetAxis(1); // Y = right
            Pan = ToSource | CamRight;           // -1=left, +1=right
            Pan = Pan < -1.0f ? -1.0f : (Pan > 1.0f ? 1.0f : Pan);
        }

        InterlockedExchange(&State->AudioThread->SpatialVolume, (LONG)(Vol * 65536.0f));
        InterlockedExchange(&State->AudioThread->SpatialPan,    (LONG)(Pan * 65536.0f));
    }

    // Lock-free frame consume: atomically claim the ready slot (-1 = none)
    LONG SlotIdx = InterlockedExchange(&Thread->ReadyIdx, -1L);
    if (SlotIdx >= 0)
    {
        INT FrameSize = Thread->Width * Thread->Height * 4;
        // Wrap raw pointer in a TArray view for UpdateMip (no copy — just set pointer/count)
        TArray<BYTE> FrameView;
        FrameView.Add(FrameSize);
        appMemcpy(FrameView.GetData(), Thread->Slots[SlotIdx], FrameSize);
        VideoTexture->UpdateMip(0, FrameView);
    }

    return bResult;
}
