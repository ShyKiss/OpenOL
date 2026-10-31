#!/bin/bash
# Runs the headless enemy AI process (-simnode) from the Steam install, using the
# OLGame.exe built in this tree. With --relay also starts a standalone relay.
#
#   run_simnode.sh [--relay] [port] [room] [password]
#
# Log: <Outlast>/OLGame/Logs/simnode.log
set -u

OUTLAST=${OUTLAST_DIR:-/mnt/ntfs2/SteamAPP/steamapps/common/Outlast}
ROOT=$(cd "$(dirname "$0")/../../.." && pwd)
export WINEPREFIX=${WINEPREFIX:-$HOME/OL1Editor} WINEDEBUG=-all

RELAY=0
if [ "${1:-}" = "--relay" ]; then RELAY=1; shift; fi
PORT=${1:-7777}
ROOM=${2:-PUBLIC}
PASSWORD=${3:-}

EXE=OLGame_simnode.exe
RELAY_PID=
cleanup() {
    pkill -f "^[^ ]*${EXE%.exe}[.]exe" 2>/dev/null
    [ -n "$RELAY_PID" ] && kill "$RELAY_PID" 2>/dev/null
    rm -f "$OUTLAST/Binaries/Win64/$EXE"
}
trap cleanup EXIT
trap "exit 130" INT TERM

if [ "$RELAY" = 1 ]; then
    BUILD=${TMPDIR:-/tmp}/ol_relay_build
    cmake -S "$ROOT/Development/Src/Multiplayer/server/src" -B "$BUILD" -G Ninja -DCMAKE_BUILD_TYPE=Release >/dev/null || exit 1
    cmake --build "$BUILD" >/dev/null || exit 1
    (cd "$BUILD" && exec ./OpenOL_Relay --port "$PORT" </dev/null) &
    RELAY_PID=$!
    sleep 0.5
fi

cp "$ROOT/Binaries/Win64/OLGame.exe" "$OUTLAST/Binaries/Win64/$EXE" || exit 1
cd "$OUTLAST/Binaries/Win64" || exit 1
echo "SimNode -> 127.0.0.1:$PORT room '$ROOM'. Log: $OUTLAST/OLGame/Logs/simnode.log. Ctrl+C to stop."
xvfb-run -a wine "$EXE" server "OLEntry?Game=Multiplayer.MultiplayerGame" -simnode \
    -simaddr=127.0.0.1 -simport="$PORT" -simroom="$ROOM" -simpass="$PASSWORD" \
    -seekfreeloadingpcconsole -log -nosteam -nohomedir -nosound -unattended -nopause -forcelogflush LOG=simnode.log \
    >/dev/null 2>&1
