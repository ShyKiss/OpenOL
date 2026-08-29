#pragma once
/*=============================================================================
    OLDDSLoader.h — Simple DDS texture loader for ImGui use.

    Loads DDS files from disk directly into IDirect3DTexture9 via D3DX.
    No UE3 packages, no GC concerns, no AddToRoot.

    Usage:
        FOLDDSLoader Loader;
        Loader.SetBaseDir("Win64\\OpenOL\\checkpoints\\OL_Checkpoints\\");
        IDirect3DTexture9* Tex = Loader.Get("Admin_Basement");
        // Returns NULL if not found or load failed.
        // Caches on first call — subsequent calls are O(N) map lookup.
        Loader.ReleaseAll(); // releases all D3D textures (call on device lost)
=============================================================================*/

#ifndef _WINSOCK2API_
#  define _WINSOCK2API_
#endif
#include <d3d9.h>

// Max textures per loader instance.
#define OL_DDS_LOADER_MAX 512

class FOLDDSLoader
{
public:
    FOLDDSLoader();
    ~FOLDDSLoader();

    // Set the base directory (Windows path, trailing backslash).
    // Call before any Get().
    void SetBaseDir(const char* Dir);

    // Look up texture by name (no extension). Loads from <BaseDir><Name>.dds
    // on first call and caches the result.
    // Returns NULL if file not found or D3D device not ready.
    // Safe to call from the render thread.
    IDirect3DTexture9* Get(const char* Name);

    // Release all cached D3D textures (call on device lost/reset or shutdown).
    void ReleaseAll();

    // Returns true if the entry for Name exists in cache (hit or miss).
    bool IsCached(const char* Name) const;

private:
    struct FEntry
    {
        char               Name[128];
        IDirect3DTexture9* Tex;   // NULL = load failed / not found
    };

    char    BaseDir[512];
    FEntry  Entries[OL_DDS_LOADER_MAX];
    int     Count;

    FEntry* Find(const char* Name);
};
