#pragma once
/*=============================================================================
    OLImageLoader.h — Simple image loader for ImGui use (stb_image backend).

    Loads JPEG/PNG/BMP files from disk into IDirect3DTexture9.
    No UE3 packages, no GC concerns.

    Usage:
        FOLImageLoader Loader;
        Loader.SetBaseDir("Win64\\OpenOL\\res\\checkpoints\\main\\");
        IDirect3DTexture9* Tex = Loader.Get("Admin_Basement");
        // Returns NULL if not found or load failed.
        // Caches on first call — subsequent calls are O(N) lookup.
        Loader.ReleaseAll(); // releases all D3D textures
=============================================================================*/

#ifndef _WINSOCK2API_
#  define _WINSOCK2API_
#endif
#include <d3d9.h>

// Max textures per loader instance.
#define OL_IMAGE_LOADER_MAX 512

class FOLImageLoader
{
public:
    FOLImageLoader();
    ~FOLImageLoader();

    // Set base directory (Windows path, trailing backslash).
    // Call before any Get().
    void SetBaseDir(const char* Dir);

    // Look up texture by name (no extension). Tries .jpg then .png.
    // Loads on first call and caches the result (NULL = failed, also cached).
    // Safe to call from the render thread.
    IDirect3DTexture9* Get(const char* Name);

    // Release all cached D3D textures (call on device lost/reset or shutdown).
    void ReleaseAll();

    // Returns true if an entry for Name already exists in cache.
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

    FEntry* Find(const char* Name);

    // Load one file into a new D3D9 texture. Returns NULL on failure.
    IDirect3DTexture9* LoadFile(const char* Path);
};
