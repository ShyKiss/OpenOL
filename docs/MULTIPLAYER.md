# Multiplayer — Server setup and connecting

<p align="right"><a href="./MULTIPLAYER_RU.md">🇷🇺 Русский</a></p>

## Overview

OpenOL supports two server modes:

| Mode | Description |
|------|-------------|
| **Embedded relay** | Relay runs inside `OLGame.exe` — no separate process needed. The host creates a game from the in-game menu. |
| **Standalone relay** | `OpenOL_Relay` runs as a separate process on any machine with a reachable IP. |

All traffic is encrypted. Everyone needs the same version of the game and the relay: older versions cannot connect.

## Getting the relay source

The relay is the [OpenOL-Relay](https://github.com/ShyKiss/OpenOL-Relay) repository, included in OpenOL as a git submodule at `Development/Src/Multiplayer/server`. `OLGame.exe` needs it too: the embedded relay (`openol_relay.lib`, step 11 in `build_wine.sh`) is compiled from the same code.

```sh
git clone --recursive https://github.com/ShyKiss/OpenOL.git

# Already cloned without it, or after pulling a newer version:
git submodule update --init
```

To only host a server (for example on a VPS), clone the relay on its own. It doesn't need the game sources:

```sh
git clone https://github.com/ShyKiss/OpenOL-Relay.git
```

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

Build on Linux from the `src` folder of the relay (`Development/Src/Multiplayer/server/src` inside OpenOL):

```sh
./build.sh         # TUI: build/OpenOL_Relay
./build.sh --gui   # also GUI: build/OpenOL_Relay_GUI
```

On Windows, use `build_win.bat` from the same folder. To run the relay in a container, see [HOSTING.md](https://github.com/ShyKiss/OpenOL-Relay/blob/main/docs/HOSTING.md).

Command line options: `--port <port>`, `--ip <address>`, `--name <name>`.

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

The file is rewritten when settings are changed through commands/GUI and on server shutdown. Room passwords are stored only as Argon2 hashes, and plain passwords in an older file are converted on start. The `trusted` list is no longer used.

### Server key

On first start the relay creates `relay.db.key` next to `relay.db` and logs `Server key: <hex>`. Clients remember this key and refuse the server if it changes.

- Keep this file and back it up. In a container, store `relay.db` and `relay.db.key` on a volume.
- If the key is lost, players must delete the server's line from their `OLMultiplayer.ini`, and room passwords must be set again with `setpw`.

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
| `stats` | Server key, traffic, refused joins, and each player's connection |
| `room create <code> [password]` | Create a room |
| `setpw <code> <password>` | Change room password |
| `log on\|off` | Toggle packet logging |
| `quit` | Shut down the server |

Bans work by IP. Guests connected through Steam P2P all share `127.0.0.1`, so banning one bans them all.

### Enemy AI simulator (optional, experimental)

By default, enemy AI runs on one of the players. It can run in a separate headless game process instead. This needs the game files on the same machine as the relay, and the script below is for Linux (Wine + Xvfb).

Start the relay and the simulator together:

```sh
OUTLAST_DIR=/path/to/Outlast Development/Src/Targets/run_simnode.sh --relay [port] [room]
```

The default port is `7777` and the default room is `PUBLIC`. Set `WINEPREFIX` if your prefix isn't `~/OL1Editor`.

To use a relay that is already running on this machine, start it with `OPENOL_SIM_SECRET` set, then run the script with the same value and without `--relay`. A relay without this variable refuses all simulators.

Start the simulator before players enter the game. In a world that is already running, it takes over the enemies only after their current owner leaves or dies.

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

If the server refuses the connection, the reason (for example, a wrong password) appears as an in-game notification.

#### If the server key changed

On the first connection, the game saves the server's key in `OLGame/Config/OLMultiplayer.ini`:

```ini
[OLGame.OLKnownServers]
123.45.67.89:7777=<hex>
```

If the key later differs, the game won't connect, because someone may be impersonating the server. If the server's admin confirms the key really changed, delete that line and connect again.

The line is matched against the address exactly as you enter it. To protect the first connection too, get the key from the admin and add the line yourself before connecting. Connections to `127.0.0.1` aren't saved. For Steam P2P, the line is named `steam:<host Steam ID>`.

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
