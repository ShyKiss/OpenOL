#pragma once
/*=============================================================================
    OLImGui_LoadingOverlay.h -- global "loading in progress" debug overlay.

    Shows a small indeterminate progress bar + stage text + elapsed time
    whenever a loading movie is playing or the player is travelling to a
    checkpoint. Always drawn, independent of the F6 debug menu state.
=============================================================================*/

// Refresh the render-thread snapshot (game thread). Call every tick,
// regardless of whether the debug overlay is open.
void OLLoadingOverlay_UpdateSnapshot();

// Set the current stage text shown in the overlay (game thread only) — call
// from wherever a meaningful loading phase actually starts (checkpoint
// travel begun, checkpoint data applied, player spawned, streaming level
// became visible, ...). Empty/NULL clears it back to the generic fallback.
void OLLoadingOverlay_SetStage(const TCHAR* Stage);

// Draw the overlay if currently loading (render thread). Call every frame,
// regardless of whether the debug overlay is open.
void OLLoadingOverlay_Draw();
