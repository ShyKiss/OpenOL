#pragma once
/*=============================================================================
    OLQuickSave.h -- debug-only quick save/load (exact position, not checkpoints).

    Separate from the OLCheckpoint/CheckpointRecord system on purpose: that
    system only ever restores to a level-placed AOLCheckpoint actor and only
    stores progress data (battery/documents/objectives), never a free
    position. This stores just a map name + player location/rotation to a
    small debug-only file, independent of the real save system.
=============================================================================*/

// exec QuickSave() / QuickLoad() in OLCheatManager.uc call these.
void OLQuickSave_Save();
void OLQuickSave_Load();

// Call once per AOLPlayerController::Tick — applies a pending load's location
// once the target map has finished loading and the player pawn exists.
void OLQuickSave_TickPendingLoad(class AOLPlayerController* PC);
