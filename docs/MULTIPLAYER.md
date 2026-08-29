# Multiplayer — Server setup and connecting

<p align="right"><a href="./MULTIPLAYER_RU.md">🇷🇺 Русский</a></p>

## Overview

OpenOL supports two server modes:

| Mode | Description |
|------|-------------|
| **Embedded relay** | Relay runs inside `OLGame.exe` — no separate process needed. The host creates a game from the in-game menu. |
| **Standalone relay** | `OpenOL_Relay` runs as a separate process on any machine with a reachable IP. |

---

## Mode 1 — Embedded relay (in-game hosting)

1. Launch the game
2. Open **Multiplayer** -> **Start Server**
3. The relay starts in a background thread; clients connect via **Steam P2P** — no open ports or public IP required
4. Room code: `DEFAULT`

The relay stops automatically when the host exits.

---

## Mode 2 — Standalone relay

Run `OpenOL_Relay` (TUI) or `OpenOL_Relay_GUI` (GUI) on a server or any machine reachable by clients.

> See [BUILD.md](./BUILD.md) for how to build the relay binaries.

### Configuration

All settings are stored in `relay.db` (JSON), created automatically next to the binary on first run:

```json
{
  "config": {
    "name":  "OLServer",
    "ip":    "0.0.0.0",
    "port":  "7777"
  },
  "rooms": [
    {
      "code":     "PUBLIC",
      "password": "",
      "trusted":  [],
      "bans":     []
    }
  ],
  "bans": []
}
```

The file is rewritten when settings are changed through commands/GUI and on server shutdown.

### Commands

Available in the TUI input line or the GUI **Log** tab:

| Command | Description |
|---------|-------------|
| `list` | List players per room |
| `rooms` | List active rooms |
| `kick <id>` | Kick a player |
| `ban <id> [reason]` | Global ban by IP |
| `ban room <code> <id> [reason]` | Ban in a specific room |
| `unban <ip>` | Remove a global ban |
| `unban room <code> <ip>` | Remove a room ban |
| `bans` | List all bans |
| `trust add <room> <ip>` | Add IP to room's trusted list |
| `trust remove <room> <ip>` | Remove IP from trusted list |
| `trust list [room]` | List trusted IPs |
| `room create <code> [password]` | Create a room |
| `setpw <code> <password>` | Change room password (clears trusted IPs) |
| `quit` | Shut down the server |

---

## Connecting as a client

### Direct UDP

1. In the game, open **Multiplayer** -> **Connect**
2. Enter the server IP and port (e.g. `123.45.67.89:7777`)
3. Enter the room code:
   - `DEFAULT` — embedded relay (in-game host)
   - `PUBLIC` — default room on a standalone relay
   - Any custom code created with `room create`
4. Enter the room password if required

### Steam P2P (embedded relay)

1. Host opens **Multiplayer** -> **Start Server**
2. Host invites friends via the Steam overlay (Shift+Tab -> Friends -> Invite to Game)
3. Client accepts the invite — the game launches and connects automatically

No open ports or IP address required on either side. Both players must be on Steam.

### Porthole (standalone relay without port forwarding)

[Porthole](https://porthole.sestudio.org) tunnels specific UDP ports through Steam — no VPN, no virtual adapter, no public IP needed.

1. Host runs a standalone relay locally (`OpenOL_Relay`, port `7777`)
2. Host opens Porthole, creates a lobby, and shares **UDP port 7777**
3. Host sends the invite code to all players
4. Each player opens Porthole, enters the code, and approves the port
5. Everyone connects in-game to **`127.0.0.1:7777`**, room code `PUBLIC`

All traffic tunnels through Steam between invited players only.
