#pragma once

// Finishes or undoes an interrupted update, then offers the latest GitHub release.
// Returns true if an update was installed and a restarted launcher has taken over:
// the caller must exit without starting the game. Any failure leaves the current
// version in place and returns false, so the game can still be started.
bool Updater_Run(LPCWSTR gameDir, LPCWSTR exePath);
