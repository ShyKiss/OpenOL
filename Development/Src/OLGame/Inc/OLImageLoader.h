#pragma once
/*=============================================================================
    OLImageLoader.h — Simple image loader for ImGui use (stb_image backend).

    Loads JPEG/PNG/BMP files from disk into IDirect3DTexture9.
    No UE3 packages, no GC concerns.

    Synchronous usage (blocks the calling thread on file I/O + decode — fine
    for small one-off loads like a tooltip image, called from the game thread):
        FOLImageLoader Loader;
        Loader.SetBaseDir("Win64\\OpenOL\\res\\checkpoints\\main\\");
        IDirect3DTexture9* Tex = Loader.Get("Admin_Basement");
        // Returns NULL if not found or load failed.
        // Caches on first call — subsequent calls are O(N) lookup.
        Loader.ReleaseAll(); // releases all D3D textures

    Async usage (file read + PNG/JPEG decode happens on a shared background
    thread; only the final D3D9 texture upload happens on the game thread —
    call PollReady() every tick to pick up finished loads):
        Loader.RequestAsync("tile_spawns_lighting");   // no-op once cached/in-flight
        Loader.PollReady();                            // call every tick
        IDirect3DTexture9* Tex = Loader.Get("tile_spawns_lighting"); // NULL until ready
        Loader.Release("tile_spawns_lighting");        // free VRAM when no longer shown
=============================================================================*/

#ifndef _WINSOCK2API_
#  define _WINSOCK2API_
#endif
#include <d3d9.h>

// Max textures per loader instance.
#define OL_IMAGE_LOADER_MAX 512

// Max async requests in flight (queued + loading) per loader instance.
#define OL_IMAGE_LOADER_MAX_ASYNC 16

class FOLImageAsyncLoadThread;
class FRunnableThread;

class FOLImageLoader
{
public:
    FOLImageLoader();
    ~FOLImageLoader();

    // Set base directory (Windows path, trailing backslash).
    // Call before any Get()/RequestAsync().
    void SetBaseDir(const char* Dir);

    // Look up texture by name (no extension). Tries .jpg then .png.
    // Loads on first call and caches the result (NULL = failed, also cached).
    // Blocks the calling thread on file I/O + decode — for on-demand async
    // loading use RequestAsync()+PollReady() instead. Also returns an async
    // result once PollReady() has uploaded it, so this doubles as the async
    // result getter (returns NULL while still loading).
    IDirect3DTexture9* Get(const char* Name);

    // Queues Name for background decode if not already cached or in flight.
    // Returns immediately; call PollReady() every tick to pick up the result,
    // then Get(Name) to read it. No-op if already cached/queued/loading.
    void RequestAsync(const char* Name);

    // Call once per tick on the game thread: uploads any background-decoded
    // pixel buffers into D3D9 textures and starts the next queued request.
    void PollReady();

    // Release one cached texture (and drop it from the cache) so it can be
    // reloaded later via Get() or RequestAsync(). No-op if not cached.
    void Release(const char* Name);

    // Release all cached D3D textures (call on device lost/reset or shutdown).
    void ReleaseAll();

    // Returns true if an entry for Name already exists in cache (loaded or
    // failed) — NOT true while an async request is only queued/in flight.
    bool IsCached(const char* Name) const;

private:
    struct FEntry
    {
        char               Name[128];
        IDirect3DTexture9* Tex;   // NULL = load failed / not found
    };

    char    BaseDir[512];
    FEntry  Entries[OL_IMAGE_LOADER_MAX];
    int     Count;

    // Names queued/loading for RequestAsync — prevents duplicate requests.
    char    AsyncPending[OL_IMAGE_LOADER_MAX_ASYNC][128];
    int     AsyncPendingCount;

    // Shared background worker; created lazily on first RequestAsync().
    FOLImageAsyncLoadThread* AsyncThread;
    FRunnableThread*         AsyncRunnableThread;

    FEntry* Find(const char* Name);
    bool    IsAsyncPending(const char* Name) const;
    void    RemoveAsyncPending(int Idx);

    // Load one file into a new D3D9 texture. Returns NULL on failure. Game/render thread only.
    IDirect3DTexture9* LoadFile(const char* Path);

    // Build BaseDir+Name+Ext for the first extension that exists, trying .jpg then .png.
    // Returns false if neither exists.
    bool ResolvePath(const char* Name, char* OutPath, int OutPathLen);
};
