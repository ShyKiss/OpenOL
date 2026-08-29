/*=============================================================================
    OLViewportSHM.cpp — copies the D3D9 backbuffer into a shared memory file.

    On llvmpipe (software) StretchRect+GetRenderTargetData stalls the pipeline.
    Instead we lock the backbuffer directly (works on llvmpipe D3DPOOL_DEFAULT)
    and memcpy to the SHM file on a worker thread.
=============================================================================*/
#include "OLGame.h"
#include <windows.h>
#include <d3d9.h>

static const char* OL_SHM_PATH = "Z:\\dev\\shm\\OLViewport";
static const UINT  OL_SHM_HDR  = 16;

// SHM
static HANDLE g_hFile   = INVALID_HANDLE_VALUE;
static HANDLE g_hMap    = NULL;
static BYTE*  g_pView   = NULL;
static DWORD  g_mapSize = 0;

// Staging: CPU-side buffer that worker copies into SHM
static BYTE*  g_stage     = NULL;
static UINT   g_stage_w   = 0;
static UINT   g_stage_h   = 0;

// Sync between render thread and worker
static HANDLE g_work_event = NULL;
static HANDLE g_done_event = NULL;
static HANDLE g_thread     = NULL;
static volatile LONG g_stop     = 0;
static volatile DWORD g_frame_w = 0;
static volatile DWORD g_frame_h = 0;
static volatile DWORD g_frame_id = 0;

static bool EnsureSHM(UINT w, UINT h)
{
    DWORD needed = OL_SHM_HDR + w * h * 4;
    if (g_pView && g_mapSize >= needed) return true;

    if (g_pView)  { UnmapViewOfFile(g_pView); g_pView = NULL; }
    if (g_hMap)   { CloseHandle(g_hMap);      g_hMap  = NULL; }
    if (g_hFile != INVALID_HANDLE_VALUE) { CloseHandle(g_hFile); g_hFile = INVALID_HANDLE_VALUE; }

    g_hFile = CreateFileA(OL_SHM_PATH, GENERIC_READ | GENERIC_WRITE,
        FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, CREATE_ALWAYS,
        FILE_ATTRIBUTE_NORMAL, NULL);
    if (g_hFile == INVALID_HANDLE_VALUE) return false;

    g_mapSize = needed;
    g_hMap = CreateFileMappingA(g_hFile, NULL, PAGE_READWRITE, 0, g_mapSize, NULL);
    if (!g_hMap) { CloseHandle(g_hFile); g_hFile = INVALID_HANDLE_VALUE; return false; }

    g_pView = (BYTE*)MapViewOfFile(g_hMap, FILE_MAP_ALL_ACCESS, 0, 0, g_mapSize);
    if (!g_pView) { CloseHandle(g_hMap); g_hMap = NULL; CloseHandle(g_hFile); g_hFile = INVALID_HANDLE_VALUE; return false; }

    debugf(TEXT("OLViewportSHM: mapped %u bytes"), g_mapSize);
    return true;
}

// Worker: copies g_stage -> SHM (pure CPU memcpy, no D3D)
static DWORD WINAPI WorkerThread(LPVOID)
{
    while (!g_stop)
    {
        if (WaitForSingleObject(g_work_event, 200) != WAIT_OBJECT_0)
            continue;
        if (g_stop) break;

        UINT W = (UINT)g_frame_w, H = (UINT)g_frame_h;
        if (!W || !H || !g_stage) { SetEvent(g_done_event); continue; }

        if (!EnsureSHM(W, H)) { SetEvent(g_done_event); continue; }

        BYTE* dst = g_pView + OL_SHM_HDR;
        appMemcpy(dst, g_stage, (SIZE_T)W * H * 4);

        DWORD* hdr = (DWORD*)g_pView;
        hdr[0] = W; hdr[1] = H; hdr[2] = InterlockedIncrement(&g_frame_id); hdr[3] = 0;

        SetEvent(g_done_event);
    }
    return 0;
}

void OLViewportSHM_CopyFrame(IDirect3DDevice9* Device, IDirect3DSurface9* BackBuffer, UINT W, UINT H)
{
    // Throttle to ~10 fps
    static DWORD s_last = 0;
    DWORD now = GetTickCount();
    if (now - s_last < 8) return;

    // Don't start a new capture if worker is still copying previous frame
    if (g_done_event && WaitForSingleObject(g_done_event, 0) != WAIT_OBJECT_0)
        return;

    // Init worker thread on first call
    if (!g_thread)
    {
        g_work_event = CreateEvent(NULL, FALSE, FALSE, NULL);
        g_done_event = CreateEvent(NULL, FALSE, TRUE, NULL); // starts signaled
        g_thread = CreateThread(NULL, 0, WorkerThread, NULL, 0, NULL);
        debugf(TEXT("OLViewportSHM: worker thread started"));
    }

    // Resize staging buffer if needed
    if (!g_stage || g_stage_w != W || g_stage_h != H)
    {
        if (g_stage) { appFree(g_stage); g_stage = NULL; }
        g_stage = (BYTE*)appMalloc(W * H * 4);
        g_stage_w = W; g_stage_h = H;
    }

    // Try to lock the backbuffer directly (works on llvmpipe software renderer)
    D3DLOCKED_RECT lr;
    HRESULT hr = BackBuffer->LockRect(&lr, NULL, D3DLOCK_READONLY | D3DLOCK_NOSYSLOCK);
    if (FAILED(hr))
    {
        debugf(TEXT("OLViewportSHM: LockRect on backbuffer failed hr=0x%08X — trying GetRenderTargetData"), hr);

        // Fallback: GetRenderTargetData into systemmem surface
        static IDirect3DSurface9* s_sys = NULL;
        static UINT s_sw = 0, s_sh = 0;
        if (!s_sys || s_sw != W || s_sh != H)
        {
            if (s_sys) { s_sys->Release(); s_sys = NULL; }
            if (FAILED(Device->CreateOffscreenPlainSurface(W, H, D3DFMT_A8R8G8B8,
                D3DPOOL_SYSTEMMEM, &s_sys, NULL))) return;
            s_sw = W; s_sh = H;
        }
        hr = Device->GetRenderTargetData(BackBuffer, s_sys);
        if (FAILED(hr)) return;
        if (FAILED(s_sys->LockRect(&lr, NULL, D3DLOCK_READONLY | D3DLOCK_NOSYSLOCK))) return;

        UINT row = W * 4;
        if ((UINT)lr.Pitch == row)
            appMemcpy(g_stage, lr.pBits, (SIZE_T)row * H);
        else
            for (UINT y = 0; y < H; y++)
                appMemcpy(g_stage + y * row, (BYTE*)lr.pBits + y * lr.Pitch, row);

        s_sys->UnlockRect();
    }
    else
    {
        // Direct lock succeeded — fast path
        UINT row = W * 4;
        if ((UINT)lr.Pitch == row)
            appMemcpy(g_stage, lr.pBits, (SIZE_T)row * H);
        else
            for (UINT y = 0; y < H; y++)
                appMemcpy(g_stage + y * row, (BYTE*)lr.pBits + y * lr.Pitch, row);

        BackBuffer->UnlockRect();
    }

    s_last = now;
    g_frame_w = W;
    g_frame_h = H;
    SetEvent(g_work_event); // wake worker to copy stage -> SHM
}

void OLViewportSHM_Shutdown()
{
    if (g_thread)
    {
        InterlockedExchange(&g_stop, 1);
        SetEvent(g_work_event);
        WaitForSingleObject(g_thread, 2000);
        CloseHandle(g_thread);     g_thread     = NULL;
        CloseHandle(g_work_event); g_work_event = NULL;
        CloseHandle(g_done_event); g_done_event = NULL;
    }
    if (g_stage) { appFree(g_stage); g_stage = NULL; }
    if (g_pView) { UnmapViewOfFile(g_pView); g_pView = NULL; }
    if (g_hMap)  { CloseHandle(g_hMap);      g_hMap  = NULL; }
    if (g_hFile != INVALID_HANDLE_VALUE) { CloseHandle(g_hFile); g_hFile = INVALID_HANDLE_VALUE; }
}
