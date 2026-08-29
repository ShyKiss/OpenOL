/*=============================================================================
    cli.h - minimal TUI: scrolling log pane + command input line
=============================================================================*/
#pragma once

#include "server.h"

// ---------------------------------------------------------------------------
// Public API
// ---------------------------------------------------------------------------

// Initialize CLI/TUI. Must be called once before any other cli_* functions.
// If stdout is not a TTY, falls back to plain line-buffered I/O.
void cli_init(Server *s);

// Append a log line. Thread-safe on platforms that guarantee atomic writes to
// a pointer-sized field. Called from server_log() replacement.
void cli_log(const char *fmt, ...);

// Optional hook: if set, cli_log calls this instead of writing to TUI/stdout.
// Used by the GUI to redirect log output to a Qt signal.
typedef void (*gui_log_hook_t)(const char *line);
extern gui_log_hook_t g_gui_log_hook;

// Process pending input — reads available keystrokes without blocking.
// Returns 1 if a command line was completed (stored in cli_last_cmd),
// 0 if no complete line yet.
int cli_poll(void);

// Dispatch a completed command line against the server.
// Called after cli_poll() returns 1.
void cli_dispatch(Server *s, const char *line);

// Redraw the screen (call after any state change).
void cli_redraw(void);

// Cleanup terminal state (call before exit).
void cli_shutdown(void);

// Last completed command line (valid after cli_poll() returns 1).
extern char cli_last_cmd[4096];
