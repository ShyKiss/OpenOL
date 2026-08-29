#pragma once
#include "Multiplayer.h"
#include "ImGuiShared.h"

// Helper called from C++ to add a notification to the HUD without ProcessEvent.
// Safe to call with NULL hud (no-op).
void HUD_AddNotification(AMultiplayerHUD* HUD, const FString& Msg);

// Called from FOLImGuiTicker::Tick — fills GMpHudBack from GMultiplayerController.
// No UE3 types leak into the caller.
void MpHud_UpdateSnapshot();
