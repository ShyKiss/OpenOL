/*
 * ol_browser_ipc.h — shared IPC definitions between openol_browser.exe and OLGame.exe
 *
 * Shared memory layout (name: "OLBrowser_<pid>_Frame"):
 *   OLBrowserFrameHeader  at offset 0
 *   BGRA pixels           at offset sizeof(OLBrowserFrameHeader), size = width * height * 4
 *   (second buffer)       at offset sizeof(OLBrowserFrameHeader) + width*height*4
 *
 * Named pipe (name: "\\.\pipe\OLBrowser_<pid>_Input"):
 *   Game → Browser: OLBrowserInputEvent structs (mouse, keyboard, scroll)
 *
 * Named pipe (name: "\\.\pipe\OLBrowser_<pid>_Cmd"):
 *   Game → Browser: OLBrowserCmd structs (navigate, resize, close)
 */

#pragma once
#ifndef OL_BROWSER_IPC_H
#define OL_BROWSER_IPC_H

#include <windows.h>
#include <stdint.h>

#define OL_BROWSER_MAX_WIDTH  2560
#define OL_BROWSER_MAX_HEIGHT 1440
// Max shared memory: header + 2 frames (double buffer) at max resolution
#define OL_BROWSER_SHM_SIZE   (sizeof(struct OLBrowserFrameHeader) + \
                                OL_BROWSER_MAX_WIDTH * OL_BROWSER_MAX_HEIGHT * 4 * 2)

// Header at start of shared memory
#pragma pack(push, 1)
struct OLBrowserFrameHeader
{
    volatile LONG  WriteIdx;     // which buffer browser is writing (0 or 1)
    volatile LONG  FrameSeq;     // incremented each time a new frame is written
    volatile LONG  Width;
    volatile LONG  Height;
    volatile LONG  BrowserReady; // 1 = browser initialized and running
    volatile LONG  PageLoading;  // 1 = page currently loading
    char           PageTitle[256];
    char           PageURL[2048];
};
#pragma pack(pop)

// Input event types
enum OLBrowserInputType
{
    OL_INPUT_MOUSE_MOVE   = 1,
    OL_INPUT_MOUSE_DOWN   = 2,
    OL_INPUT_MOUSE_UP     = 3,
    OL_INPUT_MOUSE_SCROLL = 4,
    OL_INPUT_KEY_DOWN     = 5,
    OL_INPUT_KEY_UP       = 6,
    OL_INPUT_KEY_CHAR     = 7,
};

// Mouse buttons
enum OLBrowserMouseButton
{
    OL_MOUSE_LEFT   = 0,
    OL_MOUSE_MIDDLE = 1,
    OL_MOUSE_RIGHT  = 2,
};

#pragma pack(push, 1)
struct OLBrowserInputEvent
{
    uint8_t  Type;        // OLBrowserInputType
    int32_t  X;           // mouse X (pixels in browser space)
    int32_t  Y;           // mouse Y
    int32_t  DeltaX;      // scroll delta X
    int32_t  DeltaY;      // scroll delta Y
    uint8_t  Button;      // OLBrowserMouseButton
    uint32_t Modifiers;   // CEF modifiers (EVENTFLAG_*)
    uint32_t WinKeyCode;  // Windows virtual key code
    uint32_t Char;        // Unicode char (for KEY_CHAR)
};
#pragma pack(pop)

// Command types (game → browser)
enum OLBrowserCmdType
{
    OL_CMD_NAVIGATE = 1,  // navigate to URL
    OL_CMD_RELOAD   = 2,
    OL_CMD_BACK     = 3,
    OL_CMD_FORWARD  = 4,
    OL_CMD_RESIZE   = 5,  // resize browser viewport
    OL_CMD_CLOSE    = 6,
};

#pragma pack(push, 1)
struct OLBrowserCmd
{
    uint8_t  Type;        // OLBrowserCmdType
    int32_t  Width;       // for RESIZE
    int32_t  Height;      // for RESIZE
    char     URL[2048];   // for NAVIGATE
};
#pragma pack(pop)

#endif // OL_BROWSER_IPC_H
