//=============================================================================
// OpenOLGlobals.h — forward declarations of OpenOL global objects.
//
// Include this header to access GRelayThread from any project that has
// Engine/Inc in its include path (OLGame, D3D9Drv, Multiplayer, etc.).
// No winsock2 or heavy headers required — just forward-declares the class
// and externs the global instance defined in Multiplayer/Src/RelayThread.cpp.
//=============================================================================

#pragma once

// Forward-declare FRelayThread so callers can use GRelayThread without pulling
// in RelayThread.h (which requires winsock2 before all UE3 headers).
class FRelayThread;
extern FRelayThread GRelayThread;

// Steam persona name of the local user. Implemented where Steamworks is available.
// Writes an empty string if Steam is not initialised.
extern "C" void GetSteamPersonaName(char* Out, int OutMax);

// Rich Presence: set "<mySteamID>/<room>" connect string so friends see Join Game.
extern "C" void SetRelayRichPresence(const char* RoomCode);
// Clear Rich Presence when the relay stops.
extern "C" void ClearRelayRichPresence();
