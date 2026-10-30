#!/usr/bin/env bash
# Build script for OL1 project components via Wine + MSVC 2012.

set -e

# ---- Configure before use ------------------------------------------------
WINEPREFIX="/mnt/ext41/OL1Compile"
COOK_SHIPPINGDIR="Z:/mnt/ntfs2/SteamAPP/steamapps/common/Outlast/OLGame/CookedPCConsole"
COOK_ARGS="-platform=PCConsole -multilanguagecook=INT -modcook -full -MODTEXINPKG"
# --------------------------------------------------------------------------

if [ -z "$WINEPREFIX" ]; then
    echo "ERROR: WINEPREFIX is not set. Edit this file and set WINEPREFIX."
    exit 1
fi

# ---- Paths ----------------------------------------------------------------
# msiextract unpacks into "Program Files/..." relative to drive_c (no x86 prefix).
# DirectX SDK is cloned into Program Files (x86)/ separately.
WINEPREFIX_C="$WINEPREFIX/drive_c"
VS_ROOT="$WINEPREFIX_C/Program Files/Microsoft Visual Studio 11.0"
SDK_ROOT="$WINEPREFIX_C/Program Files/Windows Kits/8.0"
DX_ROOT="$WINEPREFIX_C/Program Files (x86)/Microsoft DirectX SDK (June 2010)"

VS_ROOT_W="C:\\Program Files\\Microsoft Visual Studio 11.0"
SDK_ROOT_W="C:\\Program Files\\Windows Kits\\8.0"
DX_ROOT_W="C:\\Program Files (x86)\\Microsoft DirectX SDK (June 2010)"

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
SRC_DIR="$SCRIPT_DIR/.."                                        # Development/Src
DEV_DIR="$SRC_DIR/.."                                           # Development
TOOLS_DIR="$DEV_DIR/Tools"
OL1_DIR="$(realpath "$DEV_DIR/..")"                             # OL1 root
OLGAME_EXE="$OL1_DIR/Binaries/Win64/OLGame.exe"

UBT_EXE="$(realpath "$DEV_DIR/Intermediate/UnrealBuildTool/Release/UnrealBuildTool.exe")"
UBT_CSPROJ="$(realpath "$SRC_DIR/UnrealBuildTool/UnrealBuildTool.csproj")"
LAUNCHER_VCXPROJ="$(realpath "$TOOLS_DIR/OutlastLauncher/OutlastLauncher.vcxproj")"

MSBUILD="C:\\windows\\Microsoft.NET\\Framework64\\v4.0.30319\\MSBuild.exe"

# ---- Wine environment -----------------------------------------------------
export WINEPREFIX
export WINEARCH=win64
export WINEDEBUG=-all
export LIBGL_ALWAYS_SOFTWARE=1
export GALLIUM_DRIVER=llvmpipe

export VS110COMNTOOLS="$VS_ROOT_W\\Common7\\Tools\\"
export WindowsSdkDir="$SDK_ROOT_W\\"
export LIB="$VS_ROOT_W\\VC\\lib\\amd64;$SDK_ROOT_W\\Lib\\win8\\um\\x64;$DX_ROOT_W\\Lib\\x64"
export FrameworkDir="C:\\windows\\Microsoft.NET\\Framework64\\"
export FrameworkVersion="v4.0.30319"
export PROCESSOR_ARCHITECTURE="AMD64"
export DXSDK_DIR="$DX_ROOT_W\\"
export WINEPATH="$VS_ROOT/Common7/IDE;$VS_ROOT/VC/bin/amd64"
export INCLUDE="$VS_ROOT_W\\VC\\include;$VS_ROOT_W\\VC\\atlmfc\\include;$SDK_ROOT_W\\Include\\um;$SDK_ROOT_W\\Include\\shared;$SDK_ROOT_W\\Include\\winrt;$DX_ROOT_W\\Include"

# ---- Helper: run MSBuild --------------------------------------------------
run_msbuild() {
    local PROJ="$1"
    local CONFIG="$2"
    local PLATFORM="$3"
    shift 3
    # Convert Linux path to Wine Z: path
    local PROJ_W="Z:$(echo "$PROJ" | sed 's|/|\\|g')"
    wine "$MSBUILD" "$PROJ_W" \
        /p:Configuration="$CONFIG" \
        /p:Platform="$PLATFORM" \
        /verbosity:minimal \
        /nologo \
        "$@"
}

# ---- Helper: run UBT ------------------------------------------------------
run_ubt() {
    local CONFIG="$1"
    local REBUILD="${2:-}"
    local EXTRA_DEFINES="${3:-}"
    local OUTPUT="${4:-../../Binaries/Win64/OLGame.exe}"
    local INTERMEDIATE="${5:-../../Intermediate}"
    local DEBUG_FLAGS="${6--forcedebuginfo -pdb}"
    cd "$SRC_DIR"
    if [ -n "$REBUILD" ]; then
        # Delete all obj files to force full rebuild
        echo "Cleaning obj files..."
        local INTERMEDIATE_ABS
        INTERMEDIATE_ABS="$(cd "$SRC_DIR" && realpath "$INTERMEDIATE")"
        find "$INTERMEDIATE_ABS/OLGame" -name "*.obj" -delete 2>/dev/null || true
    fi
    wine "$UBT_EXE" OLGame Win64 "$CONFIG" -DEPLOY \
        -output "$OUTPUT" -intermediate "$INTERMEDIATE" $DEBUG_FLAGS -define WITH_STEAMWORKS=1 $EXTRA_DEFINES
}

# ---- Menu -----------------------------------------------------------------
echo ""
echo "  OL1 Build Menu"
echo "  ─────────────────────────────────"
echo "  1) Build OLGame.exe          (UBT, incremental)"
echo "  2) Rebuild OLGame.exe        (UBT, clean obj)"
echo "  3) Build OLGame Production   (WITH_EDITOR=0, incremental)"
echo "  4) Rebuild OLGame Production (WITH_EDITOR=0, clean obj)"
echo "  5) Build OutlastLauncher     (MSVC, Release|Win32)"
echo "  6) Build UnrealBuildTool     (MSBuild, Release)"
echo "  7) Build UnrealScript        (make -auto)"
echo "  8) Rebuild UnrealScript      (make -auto -full)"
echo "  9) Cook Packages             (CookPackages \$COOK_ARGS)"
echo " 10) Build openol_browser      (CEF offscreen, Release|x64)"
echo " 11) Build openol_relay.lib    (MSVC, Release|x64)"
echo " 12) Build imgui.lib           (MSVC, Release|x64)"
echo ""
if [ -n "$1" ]; then
    CHOICE="$1"
    shift
else
    printf "  Choice [1-12]: "
    read -r CHOICE
fi

open_link() {
    echo ""
    echo "  Output: $1"
}

case "$CHOICE" in
    1)
        echo "[build_wine] Build OLGame.exe (incremental)"
        run_ubt Release
        open_link "$(realpath "$DEV_DIR/../Binaries/Win64/OLGame.exe")"
        ;;
    2)
        echo "[build_wine] Rebuild OLGame.exe (clean)"
        run_ubt Release "rebuild"
        open_link "$(realpath "$DEV_DIR/../Binaries/Win64/OLGame.exe")"
        ;;
    3)
        echo "[build_wine] Build OLGame Production (WITH_EDITOR=0, incremental)"
        run_ubt Release "" "-define WITH_EDITOR=0" "../../Binaries/Win64/Production/OLGame.exe" "../../Intermediate/Production" ""
        open_link "$(realpath "$DEV_DIR/../Binaries/Win64/Production/OLGame.exe")"
        ;;
    4)
        echo "[build_wine] Rebuild OLGame Production (WITH_EDITOR=0, clean)"
        run_ubt Release "rebuild" "-define WITH_EDITOR=0" "../../Binaries/Win64/Production/OLGame.exe" "../../Intermediate/Production" ""
        open_link "$(realpath "$DEV_DIR/../Binaries/Win64/Production/OLGame.exe")"
        ;;
    5)
        echo "[build_wine] Build OutlastLauncher"
        wine "$MSBUILD" \
            "Z:$(echo "$LAUNCHER_VCXPROJ" | sed 's|/|\\|g')" \
            /p:Configuration=Release \
            /p:Platform=Win32 \
            "/p:VCTargetsPath=C:\\Program Files\\MSBuild\\Microsoft.Cpp\\v4.0\\V110\\" \
            /p:TrackFileAccess=false \
            "/p:WindowsSdkDir=$SDK_ROOT_W\\" \
            "/p:VCInstallDir=$VS_ROOT_W\\VC\\" \
            /verbosity:minimal /nologo
        open_link "$(realpath "$TOOLS_DIR/OutlastLauncher/Release/OutlastLauncher.exe")"
        ;;
    6)
        echo "[build_wine] Build UnrealBuildTool"
        UBT_OUT_W="Z:$(realpath "$DEV_DIR/Intermediate/UnrealBuildTool/Release/" | sed 's|/|\\|g')\\"
        run_msbuild "$UBT_CSPROJ" "Release" "AnyCPU" "/p:OutputPath=$UBT_OUT_W"
        open_link "$(realpath "$DEV_DIR/Intermediate/UnrealBuildTool/Release/UnrealBuildTool.exe")"
        ;;
    7|8|9)
        if [ ! -f "$OLGAME_EXE" ]; then
            echo "WARNING: OLGame.exe not found: $OLGAME_EXE"
            echo "         Build it first with option 1 or 2."
            exit 1
        fi
        case "$CHOICE" in
            7)
                echo "[build_wine] Build UnrealScript"
                WINEPATH="" wine "$OLGAME_EXE" make -auto -unattended -nopause -stdout
                ;;
            8)
                echo "[build_wine] Rebuild UnrealScript"
                WINEPATH="" wine "$OLGAME_EXE" make -auto -full -unattended -nopause -stdout
                ;;
            9)
                # Maps can be passed as extra arguments: ./build_wine.sh 9 sewers_01.udk sewers_02.udk
                # If none provided, prompt for them.
                if [ "$#" -gt 0 ]; then
                    COOK_MAPS="$*"
                else
                    printf "  Maps to cook (e.g. sewers_01.udk sewers_02.udk): "
                    read -r COOK_MAPS
                fi
                COOK_SHIPPINGDIR_ARG=""
                if [ -n "$COOK_SHIPPINGDIR" ]; then
                    COOK_SHIPPINGDIR_ARG="-shippingdir=$COOK_SHIPPINGDIR"
                fi
                echo "[build_wine] Cook Packages: $COOK_MAPS $COOK_ARGS $COOK_SHIPPINGDIR_ARG"
                # shellcheck disable=SC2086
                WINEPATH="" wine "$OLGAME_EXE" CookPackages $COOK_MAPS $COOK_ARGS $COOK_SHIPPINGDIR_ARG -unattended -nopause -stdout
                ;;
        esac
        ;;
    10)
        echo "[build_wine] Build openol_browser (MinGW-w64, CEF offscreen, x64)"
        BROWSER_DIR="$(realpath "$DEV_DIR/External/openol_browser")"
        make -C "$BROWSER_DIR" -j"$(nproc)"
        open_link "$BROWSER_DIR/bin/openol_browser.exe"
        ;;
    11)
        echo "[build_wine] Build openol_relay.lib (MSVC, Release|x64)"
        RELAY_VCXPROJ="$(realpath "$DEV_DIR/External/openol_relay/OpenOL_Relay.vcxproj")"
        wine "$MSBUILD" \
            "Z:$(echo "$RELAY_VCXPROJ" | sed 's|/|\\|g')" \
            /p:Configuration=Release \
            /p:Platform=x64 \
            "/p:VCTargetsPath=C:\\Program Files\\MSBuild\\Microsoft.Cpp\\v4.0\\V110\\" \
            /p:TrackFileAccess=false \
            "/p:WindowsSdkDir=$SDK_ROOT_W\\" \
            "/p:VCInstallDir=$VS_ROOT_W\\VC\\" \
            /verbosity:minimal /nologo
        RELAY_LIB="$(realpath "$DEV_DIR/External/openol_relay/x64/Release/openol_relay.lib" 2>/dev/null || echo "$DEV_DIR/External/openol_relay/x64/Release/openol_relay.lib")"
        open_link "$RELAY_LIB"
        ;;
    12)
        echo "[build_wine] Build imgui.lib (MSVC, Release|x64)"
        IMGUI_VCXPROJ="$(realpath "$DEV_DIR/External/imgui/ImGui.vcxproj")"
        wine "$MSBUILD" \
            "Z:$(echo "$IMGUI_VCXPROJ" | sed 's|/|\\|g')" \
            /p:Configuration=Release \
            /p:Platform=x64 \
            "/p:VCTargetsPath=C:\\Program Files\\MSBuild\\Microsoft.Cpp\\v4.0\\V110\\" \
            /p:TrackFileAccess=false \
            "/p:WindowsSdkDir=$SDK_ROOT_W\\" \
            "/p:VCInstallDir=$VS_ROOT_W\\VC\\" \
            /verbosity:minimal /nologo
        open_link "$(realpath "$DEV_DIR/External/imgui/lib/imgui.lib" 2>/dev/null || echo "$DEV_DIR/External/imgui/lib/imgui.lib")"
        ;;
    *)
        echo "Invalid choice."
        exit 1
        ;;
esac

echo "[build_wine] Done."
