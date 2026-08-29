/*=============================================================================
	ImGuiLinker.h: ImGui integration for Outlast (D3D9 + Win32).
	Provides init/shutdown/render/wndproc entry points callable from
	both D3D9Drv and WinDrv without pulling in imgui headers.
=============================================================================*/

#pragma once

// Rely on the caller having windows.h / d3d9.h already included (via Engine.h or WinDrv)
struct IDirect3DDevice9;

/** Initialize ImGui with a D3D9 device. Must be called once after device creation. */
void OLImGui_Init(IDirect3DDevice9* Device, HWND Wnd);

/** Shutdown ImGui and release all resources. */
void OLImGui_Shutdown();

/** Handle Win32 messages for ImGui input. Returns non-zero if ImGui consumed the message. */
LRESULT OLImGui_WndProcHandler(HWND Wnd, UINT Msg, WPARAM wParam, LPARAM lParam);

/** Begin a new ImGui frame. Must be called before any ImGui UI code each frame. */
void OLImGui_NewFrame();

/** End the frame, render draw data, and submit to D3D9. Call between EndScene() and Present(). */
void OLImGui_Render();

/** Toggle the debug overlay on/off (F8). */
void OLImGui_ToggleOverlay();

/** Returns true if the debug overlay is currently visible. Used by WinDrv to keep the OS cursor shown. */
bool OLImGui_IsOverlayVisible();

/** Callback type for overlay action buttons. Registered by game-side code (e.g. Multiplayer). */
typedef void (*OLImGuiActionFn)();

/** Register a named action button shown in the debug overlay. Defined in OLGame/OLGame/Src/OLImGui.cpp. */
void OLImGui_RegisterAction(const char* Label, OLImGuiActionFn Fn);

/** Drain action callbacks on the game thread. Defined in OLGame/OLGame/Src/OLImGui.cpp. */
void OLImGui_DrainPendingActions();

/** Ensure snapshot buffers and ticker are created (game thread). Defined in OLGame/OLGame/Src/OLImGui.cpp. */
void OLImGui_EnsureTicker();


/** Tunable FVector tweaks exposed in the Actions tab for runtime experimentation.
    Written by the render thread (ImGui), read by the game thread (OLHero). */
extern volatile float GDebugVec0[3]; // WorkerHead->Origin
extern volatile float GDebugVec1[3]; // AttachComponent offset

/** Fire any action bound to this Win32 virtual-key code (overlay must be closed). */
void OLImGui_Bindings_FireKey(int VK);

/** Returns the VK code bound to overlay toggle (default VK_F6). */
int OLImGui_GetOverlayToggleVK();

/** Release D3D9 device objects (call before device Reset). */
void OLImGui_InvalidateDeviceObjects();

/** Returns the number of D3DPOOL_DEFAULT resources ImGui currently holds (for diagnostics). */
int OLImGui_GetLiveResourceCount();

/** Recreate D3D9 device objects (call after device Reset). */
void OLImGui_CreateDeviceObjects();

/** Main window screen rect — written each frame by OLImGui_BuildUI so that
    OLImGui_Render can sample the backbuffer region for the blur effect. */
extern int GOLWinX, GOLWinY, GOLWinW, GOLWinH;
