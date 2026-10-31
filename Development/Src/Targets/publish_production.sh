#!/bin/bash
# publish.sh — collect mod files from OL1 editor layout into PublishTemp/
# mirroring the destination layout used by the retail game.
#
# Sources:
#   OLGame/Script/*.u        → OLGame/CookedPCConsole/OpenOL/*.u   (Multiplayer, OLGame)
#                            → OLGame/CookedPCConsole/*.upk         (engine packages as .u→.upk)
#   OLGame/Content/04_UI/    → OLGame/CookedPCConsole/              (menuassets, OLFrontEnd)
#   Binaries, Config, Localization — same relative paths

set -e

# --strict: a missing file is an error, for archives that go to users.
STRICT=0
[ "$1" = "--strict" ] && STRICT=1
MISSING_COUNT=0

OL1="$(cd "$(dirname "$0")/../../.." && pwd)"
DEST="$OL1/PublishTemp"
ARCHIVE="$DEST/OpenOL.7z"

# Each entry: "OL1_source_path|retail_dest_path"
declare -a MAPPINGS=(
    # Executables
    "Development/Tools/OutlastLauncher/Release/OutlastLauncher.exe|OutlastLauncher.exe"
    "Binaries/Win64/Production/OLGame.exe|Binaries/Win64/OLGame.exe"

    # UnrealScript packages — mod-specific go to OpenOL/, engine packages as .upk
    "OLGame/Script/Multiplayer.u|OLGame/CookedPCConsole/OpenOL/Multiplayer.u"
    "OLGame/Script/TASGame.u|OLGame/CookedPCConsole/OpenOL/TASGame.u"
    "OLGame/Script/OLGame.u|OLGame/CookedPCConsole/OpenOL/OLGame.u"
    "OLGame/Script/Core.u|OLGame/CookedPCConsole/OpenOL/Core.u"
    "OLGame/Script/Engine.u|OLGame/CookedPCConsole/OpenOL/Engine.u"
    "OLGame/Script/GameFramework.u|OLGame/CookedPCConsole/OpenOL/GameFramework.u"
    "OLGame/Script/IpDrv.u|OLGame/CookedPCConsole/OpenOL/IpDrv.u"
    "OLGame/Script/WinDrv.u|OLGame/CookedPCConsole/OpenOL/WinDrv.u"
    "OLGame/Script/AkAudio.u|OLGame/CookedPCConsole/OpenOL/AkAudio.u"
    "OLGame/Script/GFxUI.u|OLGame/CookedPCConsole/OpenOL/GFxUI.u"
    "OLGame/Script/OnlineSubsystemPC.u|OLGame/CookedPCConsole/OpenOL/OnlineSubsystemPC.u"
    "OLGame/Script/OnlineSubsystemSteamworks.u|OLGame/CookedPCConsole/OpenOL/OnlineSubsystemSteamworks.u"

    # UI content packages
    "OLGame/Content/04_UI/menuassets.upk|OLGame/CookedPCConsole/menuassets.upk"
    "OLGame/Content/04_UI/OLFrontEnd.upk|OLGame/CookedPCConsole/OLFrontEnd.upk"

    # Localization
    "OLGame/Localization/INT/OLGame.int|OLGame/Localization/INT/OLGame.int"
    "OLGame/Localization/RUS/olgame.RUS|OLGame/Localization/RUS/olgame.RUS"
    "OLGame/Localization/DEU/olgame.DEU|OLGame/Localization/DEU/olgame.DEU"
    "OLGame/Localization/ESN/olgame.ESN|OLGame/Localization/ESN/olgame.ESN"
    "OLGame/Localization/FRA/olgame.FRA|OLGame/Localization/FRA/olgame.FRA"
    "OLGame/Localization/ITA/olgame.ITA|OLGame/Localization/ITA/olgame.ITA"
    "OLGame/Localization/JPN/OLGame.JPN|OLGame/Localization/JPN/OLGame.JPN"
    "OLGame/Localization/POL/olgame.POL|OLGame/Localization/POL/olgame.POL"
    "OLGame/Localization/PTB/OLGame.ptb|OLGame/Localization/PTB/OLGame.ptb"

    # Config
    "OLGame/Config/DefaultUI.ini|OLGame/Config/DefaultUI.ini"
    "OLGame/Config/DefaultEngine.ini|OLGame/Config/DefaultEngine.ini"
    "OLGame/Config/DefaultMultiplayer.ini|OLGame/Config/DefaultMultiplayer.ini"
    "OLGame/Config/DefaultGame.ini|OLGame/Config/DefaultGame.ini"
    "OLGame/Config/DefaultInput.ini|OLGame/Config/DefaultInput.ini"

    #ImGui Config
    "Binaries/Win64/OpenOL/locales/en.ini|Binaries/Win64/OpenOL/locales/en.ini"
    "Binaries/Win64/OpenOL/locales/ru.ini|Binaries/Win64/OpenOL/locales/ru.ini"

    # Checkpoint thumbnails (DDS)
    "Binaries/Win64/OpenOL/res/checkpoints/|Binaries/Win64/OpenOL/res/checkpoints/"

    # Hub tiles (Spawns tab)
    "Binaries/Win64/OpenOL/res/tiles/|Binaries/Win64/OpenOL/res/tiles/"
)

echo "Publishing to $DEST ..."
rm -rf "$DEST"

DEST_FILES=()
for entry in "${MAPPINGS[@]}"; do
    src_rel="${entry%%|*}"
    dst_rel="${entry##*|}"
    src="$OL1/$src_rel"
    dst="$DEST/$dst_rel"

    # Directory entry: trailing slash on both sides
    if [[ "$src_rel" == */ ]]; then
        if [ ! -d "$src" ]; then
            echo "  MISSING DIR: $src_rel"
            MISSING_COUNT=$((MISSING_COUNT + 1))
            continue
        fi
        mkdir -p "$dst"
        cp -r "$src/." "$dst/"
        # Collect all copied files for the archive
        while IFS= read -r -d '' f; do
            rel="${f#$DEST/}"
            DEST_FILES+=("$rel")
        done < <(find "$dst" -type f -print0)
        echo "  $src_rel  →  $dst_rel  (dir)"
        continue
    fi

    if [ ! -f "$src" ]; then
        echo "  MISSING: $src_rel"
        MISSING_COUNT=$((MISSING_COUNT + 1))
        continue
    fi

    mkdir -p "$(dirname "$dst")"
    cp "$src" "$dst"
    echo "  $src_rel  →  $dst_rel"
    DEST_FILES+=("$dst_rel")
done

if [ "$STRICT" = 1 ] && [ "$MISSING_COUNT" -gt 0 ]; then
    echo "ERROR: $MISSING_COUNT missing, no archive was created."
    exit 1
fi

ARCHIVE_ZIP="${ARCHIVE%.7z}.zip"

echo ""
echo "Creating archive $ARCHIVE ..."
rm -f "$ARCHIVE"
cd "$DEST"
7z a -t7z -m0=lzma2 -mx=9 -mfb=273 -md=64m -mf=BCJ2 -ms=on -mmt=on "$ARCHIVE" "${DEST_FILES[@]}"

echo ""
echo "Creating archive $ARCHIVE_ZIP ..."
rm -f "$ARCHIVE_ZIP"
# OutlastLauncher unpacks this zip itself and supports only Deflate and Store.
7z a -tzip -mm=Deflate -mx=9 -mmt=on "$ARCHIVE_ZIP" "${DEST_FILES[@]}"

echo ""
echo "Done:"
echo "  $ARCHIVE"
echo "  $ARCHIVE_ZIP"
