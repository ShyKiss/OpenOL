/*=============================================================================
    OLLinuxBridge.cpp — stub; SHM and IPC now use plain Win32 (CreateFileMapping
    for shared memory via Z:\dev\shm\, Winsock for IPC). No POSIX needed.
=============================================================================*/
#include "OLGame.h"

bool OLLinuxBridge_Load() { return true; }
void OLLinuxBridge_Unload() {}
