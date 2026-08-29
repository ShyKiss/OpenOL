/*=============================================================================
    OLDDSLoader.cpp — DDS texture loader implementation.
=============================================================================*/
#ifndef _WINSOCK2API_
#  define _WINSOCK2API_
#endif
#include <d3d9.h>
#include <d3dx9tex.h>
#include <string.h>

#include "OLGame.h"
#include "OLDDSLoader.h"

extern IDirect3DDevice9* GLegacyDirect3DDevice9;

// ---------------------------------------------------------------------------
// FOLDDSLoader
// ---------------------------------------------------------------------------

FOLDDSLoader::FOLDDSLoader()
    : Count(0)
{
    BaseDir[0] = '\0';
}

FOLDDSLoader::~FOLDDSLoader()
{
    ReleaseAll();
}

void FOLDDSLoader::SetBaseDir(const char* Dir)
{
    int i = 0;
    for (; i < (int)sizeof(BaseDir) - 1 && Dir[i]; ++i)
        BaseDir[i] = Dir[i];
    BaseDir[i] = '\0';
}

FOLDDSLoader::FEntry* FOLDDSLoader::Find(const char* Name)
{
    for (int i = 0; i < Count; ++i)
        if (strcmp(Entries[i].Name, Name) == 0)
            return &Entries[i];
    return NULL;
}

bool FOLDDSLoader::IsCached(const char* Name) const
{
    for (int i = 0; i < Count; ++i)
        if (strcmp(Entries[i].Name, Name) == 0)
            return true;
    return false;
}

IDirect3DTexture9* FOLDDSLoader::Get(const char* Name)
{
    // Cache hit
    FEntry* E = Find(Name);
    if (E) return E->Tex;

    // Cache miss — try to load
    if (Count >= OL_DDS_LOADER_MAX || !GLegacyDirect3DDevice9)
        return NULL;

    // Build full path: BaseDir + Name + ".dds"
    char Path[768];
    int pi = 0;
    for (int i = 0; BaseDir[i] && pi < (int)sizeof(Path) - 1; ++i)
        Path[pi++] = BaseDir[i];
    for (int i = 0; Name[i] && pi < (int)sizeof(Path) - 1; ++i)
        Path[pi++] = Name[i];
    const char Ext[] = ".dds";
    for (int i = 0; Ext[i] && pi < (int)sizeof(Path) - 1; ++i)
        Path[pi++] = Ext[i];
    Path[pi] = '\0';

    IDirect3DTexture9* Tex = NULL;
    HRESULT hr = D3DXCreateTextureFromFileExA(
        GLegacyDirect3DDevice9,
        Path,
        D3DX_DEFAULT_NONPOW2,       // width: preserve exact size, no pow2 rounding
        D3DX_DEFAULT_NONPOW2,       // height: preserve exact size, no pow2 rounding
        1,                          // 1 mip level — no chain needed for UI
        0,                          // usage
        D3DFMT_UNKNOWN,             // keep DXT format from file
        D3DPOOL_MANAGED,
        D3DX_FILTER_NONE,
        D3DX_FILTER_NONE,
        0,                          // color key — none
        NULL, NULL,
        &Tex);

    if (FAILED(hr))
    {
        debugf(NAME_Log, TEXT("OLDDSLoader: failed to load '%s' (hr=0x%08X)"),
            ANSI_TO_TCHAR(Path), (unsigned)hr);
        Tex = NULL;
    }

    // Store in cache (NULL = not found, also cached to avoid repeated IO)
    FEntry& New = Entries[Count++];
    int ni = 0;
    for (; ni < (int)sizeof(New.Name) - 1 && Name[ni]; ++ni)
        New.Name[ni] = Name[ni];
    New.Name[ni] = '\0';
    New.Tex = Tex;

    return Tex;
}

void FOLDDSLoader::ReleaseAll()
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
