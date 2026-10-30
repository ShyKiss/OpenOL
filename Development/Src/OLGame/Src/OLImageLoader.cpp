/*=============================================================================
    OLImageLoader.cpp — Image loader implementation (stb_image + D3D9).
=============================================================================*/
#ifndef _WINSOCK2API_
#  define _WINSOCK2API_
#endif
#include <d3d9.h>
#include <string.h>

// stb_image — implementation is compiled into imgui.lib (stb_image.cpp).
// Include only the header here.
#define STBI_NO_STDIO   // we open files manually to handle Windows paths
#include "../../External/imgui/stb/stb_image.h"

#include "OLGame.h"
#include "OLImageLoader.h"

extern IDirect3DDevice9* GLegacyDirect3DDevice9;

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------

static void SafeCopyA(char* Dst, int Len, const char* Src)
{
    int i = 0;
    for (; i < Len - 1 && Src[i]; ++i) Dst[i] = Src[i];
    Dst[i] = '\0';
}

static void AppendA(char* Dst, int Len, const char* Src)
{
    int i = 0;
    while (i < Len - 1 && Dst[i]) ++i;
    for (; i < Len - 1 && *Src; ++i, ++Src) Dst[i] = *Src;
    Dst[i] = '\0';
}

// ---------------------------------------------------------------------------
// FOLImageAsyncLoadThread — one shared background worker per FOLImageLoader
// instance. Processes a small FIFO queue of (name, path) requests: reads the
// file and runs stb_image decode entirely off the game thread. Decoded RGBA8
// buffers are picked up and uploaded to D3D9 by FOLImageLoader::PollReady()
// on the game thread, since D3D9 device calls aren't safe from a worker thread.
// ---------------------------------------------------------------------------

class FOLImageAsyncLoadThread : public FRunnable
{
public:
    struct FResult
    {
        char           Name[128];
        unsigned char* Pixels; // RGBA8, owned by whoever pops it; NULL = load failed
        int            Width;
        int            Height;
    };

    FOLImageAsyncLoadThread() : bStopRequested(FALSE), QueueHead(0), QueueTail(0)
        , ResultHead(0), ResultTail(0) {}

    virtual UBOOL Init() { return TRUE; }

    virtual DWORD Run()
    {
        while (!bStopRequested)
        {
            FRequest Req;
            bool bHasReq = false;
            {
                FScopeLock SL(&Lock);
                if (QueueHead != QueueTail)
                {
                    Req = Queue[QueueHead];
                    QueueHead = (QueueHead + 1) % OL_IMAGE_LOADER_MAX_ASYNC;
                    bHasReq = true;
                }
            }

            if (!bHasReq) { appSleep(0.01f); continue; }

            FResult Res;
            SafeCopyA(Res.Name, sizeof(Res.Name), Req.Name);
            Res.Pixels = NULL; Res.Width = 0; Res.Height = 0;

            FILE* f = fopen(Req.Path, "rb");
            if (f)
            {
                fseek(f, 0, SEEK_END);
                long FSize = ftell(f);
                fseek(f, 0, SEEK_SET);
                if (FSize > 0 && FSize <= 32 * 1024 * 1024)
                {
                    unsigned char* Raw = (unsigned char*)malloc(FSize);
                    if (Raw)
                    {
                        fread(Raw, 1, FSize, f);
                        int Channels = 0;
                        Res.Pixels = stbi_load_from_memory(Raw, (int)FSize, &Res.Width, &Res.Height, &Channels, 4);
                        free(Raw);
                    }
                }
                fclose(f);
            }

            {
                FScopeLock SL(&Lock);
                INT NextTail = (ResultTail + 1) % OL_IMAGE_LOADER_MAX_ASYNC;
                if (NextTail != ResultHead) // drop on overflow (shouldn't happen — bounded by pending count)
                {
                    Results[ResultTail] = Res;
                    ResultTail = NextTail;
                }
                else if (Res.Pixels)
                {
                    stbi_image_free(Res.Pixels);
                }
            }
        }
        return 0;
    }

    virtual void Stop() { bStopRequested = TRUE; }
    virtual void Exit() {}

    // Queues one file for background decode. Game thread only.
    void Enqueue(const char* Name, const char* Path)
    {
        FScopeLock SL(&Lock);
        INT NextTail = (QueueTail + 1) % OL_IMAGE_LOADER_MAX_ASYNC;
        if (NextTail == QueueHead) return; // queue full — drop silently, RequestAsync() de-dupes anyway
        FRequest& R = Queue[QueueTail];
        SafeCopyA(R.Name, sizeof(R.Name), Name);
        SafeCopyA(R.Path, sizeof(R.Path), Path);
        QueueTail = NextTail;
    }

    // Pops one finished result if available. Caller takes ownership of Pixels
    // (must stbi_image_free it, or hand it off). Game thread only.
    bool PopResult(FResult& Out)
    {
        FScopeLock SL(&Lock);
        if (ResultHead == ResultTail) return false;
        Out = Results[ResultHead];
        ResultHead = (ResultHead + 1) % OL_IMAGE_LOADER_MAX_ASYNC;
        return true;
    }

    volatile UBOOL bStopRequested;

private:
    struct FRequest
    {
        char Name[128];
        char Path[768];
    };

    FCriticalSection Lock;
    FRequest Queue[OL_IMAGE_LOADER_MAX_ASYNC];
    INT      QueueHead, QueueTail;
    FResult  Results[OL_IMAGE_LOADER_MAX_ASYNC];
    INT      ResultHead, ResultTail;
};

// ---------------------------------------------------------------------------
// FOLImageLoader
// ---------------------------------------------------------------------------

FOLImageLoader::FOLImageLoader()
    : Count(0), AsyncPendingCount(0), AsyncThread(NULL), AsyncRunnableThread(NULL)
{
    BaseDir[0] = '\0';
}

FOLImageLoader::~FOLImageLoader()
{
    if (AsyncRunnableThread)
    {
        AsyncThread->Stop();
        AsyncRunnableThread->WaitForCompletion();
        GThreadFactory->Destroy(AsyncRunnableThread);
        AsyncRunnableThread = NULL;
    }
    delete AsyncThread;
    AsyncThread = NULL;
    ReleaseAll();
}

void FOLImageLoader::SetBaseDir(const char* Dir)
{
    SafeCopyA(BaseDir, sizeof(BaseDir), Dir);
}

FOLImageLoader::FEntry* FOLImageLoader::Find(const char* Name)
{
    for (int i = 0; i < Count; ++i)
        if (strcmp(Entries[i].Name, Name) == 0)
            return &Entries[i];
    return NULL;
}

bool FOLImageLoader::IsCached(const char* Name) const
{
    for (int i = 0; i < Count; ++i)
        if (strcmp(Entries[i].Name, Name) == 0)
            return true;
    return false;
}

IDirect3DTexture9* FOLImageLoader::LoadFile(const char* Path)
{
    if (!GLegacyDirect3DDevice9)
        return NULL;

    // Read file into memory (fopen handles Windows paths under Wine/MSVC).
    FILE* f = fopen(Path, "rb");
    if (!f) return NULL;

    fseek(f, 0, SEEK_END);
    long fsize = ftell(f);
    fseek(f, 0, SEEK_SET);

    if (fsize <= 0 || fsize > 32 * 1024 * 1024) { fclose(f); return NULL; }

    unsigned char* buf = (unsigned char*)malloc(fsize);
    if (!buf) { fclose(f); return NULL; }
    fread(buf, 1, fsize, f);
    fclose(f);

    // Decode with stb_image → always RGBA8.
    int w, h, ch;
    unsigned char* pixels = stbi_load_from_memory(buf, (int)fsize, &w, &h, &ch, 4);
    free(buf);

    if (!pixels) return NULL;

    // Create D3D9 texture (A8R8G8B8, MANAGED).
    IDirect3DTexture9* Tex = NULL;
    HRESULT hr = GLegacyDirect3DDevice9->CreateTexture(
        (UINT)w, (UINT)h,
        1,               // 1 mip level
        0,               // usage
        D3DFMT_A8R8G8B8,
        D3DPOOL_MANAGED,
        &Tex, NULL);

    if (FAILED(hr))
    {
        stbi_image_free(pixels);
        return NULL;
    }

    // Upload pixels: stb_image gives RGBA, D3D9 A8R8G8B8 is BGRA in memory.
    D3DLOCKED_RECT Locked;
    hr = Tex->LockRect(0, &Locked, NULL, 0);
    if (SUCCEEDED(hr))
    {
        for (int y = 0; y < h; ++y)
        {
            unsigned char*       dst = (unsigned char*)Locked.pBits + y * Locked.Pitch;
            const unsigned char* src = pixels + y * w * 4;
            for (int x = 0; x < w; ++x)
            {
                dst[0] = src[2]; // B
                dst[1] = src[1]; // G
                dst[2] = src[0]; // R
                dst[3] = src[3]; // A
                dst += 4; src += 4;
            }
        }
        Tex->UnlockRect(0);
    }
    else
    {
        Tex->Release();
        Tex = NULL;
    }

    stbi_image_free(pixels);
    return Tex;
}

IDirect3DTexture9* FOLImageLoader::Get(const char* Name)
{
    // Cache hit
    FEntry* E = Find(Name);
    if (E) return E->Tex;

    // Cache miss
    if (Count >= OL_IMAGE_LOADER_MAX)
        return NULL;

    // Build path: try .jpg first, then .png
    char Path[768];
    IDirect3DTexture9* Tex = NULL;

    const char* Exts[] = { ".jpg", ".png", NULL };
    for (int ei = 0; Exts[ei] && !Tex; ++ei)
    {
        SafeCopyA(Path, sizeof(Path), BaseDir);
        AppendA(Path, sizeof(Path), Name);
        AppendA(Path, sizeof(Path), Exts[ei]);
        Tex = LoadFile(Path);
    }

    if (!Tex)
        debugf(NAME_Log, TEXT("OLImageLoader: failed to load '%s'"), ANSI_TO_TCHAR(Name));

    // Store in cache (NULL also cached to avoid repeated IO).
    FEntry& New = Entries[Count++];
    SafeCopyA(New.Name, sizeof(New.Name), Name);
    New.Tex = Tex;

    return Tex;
}

void FOLImageLoader::ReleaseAll()
{
    for (int i = 0; i < Count; ++i)
    {
        if (Entries[i].Tex)
        {
            Entries[i].Tex->Release();
            Entries[i].Tex = NULL;
        }
    }
    Count = 0;
}
