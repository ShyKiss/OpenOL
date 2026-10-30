#!/usr/bin/env bash
# =============================================================================
# setup_wineprefix.sh — Create Wine prefix for OL1 builds from scratch.
#
# Usage:
#   WINEPREFIX=/mnt/ext41/OL1Compile ./setup_wineprefix.sh
#
# Sources (no VM, no account required):
#   VS 2012 ISO  : https://archive.org/download/en_visual_studio_professional_2012_x86_dvd/
#   DirectX SDK  : https://github.com/testing-laboratory/DirectX-SDK-June2010
#
# Dependencies:
#   pacman -S wine-staging winetricks msitools wget git p7zip
# =============================================================================

set -e

# ---- Colors ---------------------------------------------------------------
RED='\033[0;31m'; GREEN='\033[0;32m'; YELLOW='\033[1;33m'; NC='\033[0m'
info()  { echo -e "${GREEN}[setup]${NC} $*"; }
warn()  { echo -e "${YELLOW}[warn]${NC}  $*"; }
error() { echo -e "${RED}[error]${NC} $*"; exit 1; }
skip()  { echo -e "${YELLOW}[skip]${NC}  $*"; }

# ---- Configure this path before use --------------------------------------
WINEPREFIX="/mnt/ext41/OL1Compile"
# --------------------------------------------------------------------------

if [ -z "$WINEPREFIX" ]; then
    error "WINEPREFIX is not set. Edit this file and set WINEPREFIX."
fi

PREFIX="$WINEPREFIX"
PARENT="$(dirname "$PREFIX")"
if [ ! -w "$PARENT" ]; then
    error "Parent directory '$PARENT' is not writable."
fi

info "Prefix: $PREFIX"
VM_C="$PREFIX/drive_c"
PFDIR="$VM_C/Program Files (x86)"

# ---- Step 1: Check dependencies -------------------------------------------
info "Step 1: Checking dependencies..."
for cmd in wine winetricks msiextract wget git 7z; do
    if ! command -v "$cmd" &>/dev/null; then
        error "'$cmd' not found. Install: sudo pacman -S wine-staging winetricks msitools wget git p7zip"
    fi
done
info "All dependencies found."

# ---- Step 2: Download all sources first -----------------------------------
ISO_URL="https://archive.org/download/en_visual_studio_professional_2012_x86_dvd/Visual%20Studio%20Professional%202012%20%28x86%29%20-%20DVD%20%28English%29/en_visual_studio_professional_2012_x86_dvd_2262334.iso"
ISO_FILE="/tmp/vs2012.iso"
ISO_EXTRACTED="/tmp/vs2012_extracted"
DX_DEST="$PFDIR/Microsoft DirectX SDK (June 2010)"
DX_REPO="https://github.com/testing-laboratory/DirectX-SDK-June2010.git"

if [ -f "$ISO_FILE" ]; then
    skip "Step 2a: VS 2012 ISO already downloaded ($ISO_FILE)."
else
    info "Step 2a: Downloading VS 2012 ISO (~2.5 GB)..."
    wget -c -O "$ISO_FILE" "$ISO_URL"
fi

if [ -d "$DX_DEST/Include" ]; then
    skip "Step 2b: DirectX SDK already present."
else
    info "Step 2b: Cloning DirectX SDK June 2010..."
    git clone --depth=1 "$DX_REPO" "$DX_DEST"
fi

# ---- Step 3: Extract ISO --------------------------------------------------
PKG="$ISO_EXTRACTED/packages"
if [ -d "$PKG" ]; then
    skip "Step 3: ISO already extracted ($ISO_EXTRACTED)."
else
    info "Step 3: Extracting ISO..."
    mkdir -p "$ISO_EXTRACTED"
    7z x "$ISO_FILE" -o"$ISO_EXTRACTED" -y > /dev/null
fi

# ---- Step 4: Create Wine prefix -------------------------------------------
if [ -f "$PREFIX/system.reg" ]; then
    skip "Step 4: Wine prefix already exists."
else
    info "Step 4: Creating Wine prefix..."
    mkdir -p "$PREFIX"
    WINEPREFIX="$PREFIX" WINEARCH=win64 WINEDEBUG=-all \
        LIBGL_ALWAYS_SOFTWARE=1 GALLIUM_DRIVER=llvmpipe \
        wineboot --init 2>/dev/null
    info "Prefix created."
fi

# ---- Step 5: Install .NET and runtimes ------------------------------------
# winetricks tracks installed packages in $PREFIX/winetricks.log
TRICKS_LOG="$PREFIX/winetricks.log"
need_trick() { ! grep -qx "$1" "$TRICKS_LOG" 2>/dev/null; }

if need_trick dotnet45 || need_trick vcrun2012; then
    info "Step 5: Installing .NET 4.5 and VC++ runtimes..."
    WINEPREFIX="$PREFIX" WINEDEBUG=-all \
        LIBGL_ALWAYS_SOFTWARE=1 GALLIUM_DRIVER=llvmpipe \
        winetricks -q dotnet40 dotnet45 vcrun2012 vcrun2013
    info "Runtimes installed."
else
    skip "Step 5: Runtimes already installed."
fi

# ---- Step 6: Extract MSI packages into prefix ----------------------------
extract_msi() {
    local MSI="$1"
    local DEST="$2"
    local SENTINEL="$3"   # file to check — skip if already exists
    local NAME
    NAME="$(basename "$MSI")"
    if [ -e "$SENTINEL" ]; then
        skip "  $NAME already extracted."
        return
    fi
    info "  Extracting $NAME..."
    mkdir -p "$DEST"
    # Count total files for progress bar
    local TOTAL
    TOTAL=$(msiextract --list "$MSI" 2>/dev/null | wc -l)
    local COUNT=0 BAR_WIDTH=40
    local HASHES SPACES
    # Extract and update progress bar line-by-line
    (cd "$DEST" && msiextract "$MSI" 2>/dev/null) | while IFS= read -r _line; do
        COUNT=$((COUNT + 1))
        local FILLED=$(( COUNT * BAR_WIDTH / (TOTAL > 0 ? TOTAL : 1) ))
        local EMPTY=$(( BAR_WIDTH - FILLED ))
        local PCT=$(( COUNT * 100 / (TOTAL > 0 ? TOTAL : 1) ))
        HASHES=$(head -c "$FILLED" /dev/zero | tr '\0' '#')
        SPACES=$(head -c "$EMPTY"  /dev/zero | tr '\0' ' ')
        printf "\r    [%s%s] %3d%%" "$HASHES" "$SPACES" "$PCT" >&2
    done
    printf "\r%*s\r" $(( BAR_WIDTH + 12 )) "" >&2
}

info "Step 6: Extracting VS 2012 components..."

# All MSI packages use paths starting with "Program Files/..." or "MSBuild/..."
# so we extract into VM_C — it becomes drive_c/ which maps to C:\ in Wine.

# Compiler: cl.exe, link.exe, x86_amd64/, mspdb110.dll, CppTasks.dll
extract_msi \
    "$PKG/vc_compilercore/vc_compilercore.msi" \
    "$VM_C" \
    "$VM_C/Program Files/Microsoft Visual Studio 11.0/VC/bin/x86_amd64/cl.exe"

# Professional core: amd64/cvtres.exe (required by linker for .res embedding)
extract_msi \
    "$PKG/professionalcore/Setup/vs_professionalcore.msi" \
    "$VM_C" \
    "$VM_C/Program Files/Microsoft Visual Studio 11.0/VC/bin/amd64/cvtres.exe"

# Professional LP: amd64/1033/clui.dll (compiler language resource)
extract_msi \
    "$PKG/professionallp/vs_professionalcoreres.msi" \
    "$VM_C" \
    "$VM_C/Program Files/Microsoft Visual Studio 11.0/VC/bin/amd64/1033/clui.dll"

# CRT/STL headers + amd64 libs
extract_msi \
    "$PKG/vc_librarycore/vc_librarycore.msi" \
    "$VM_C" \
    "$VM_C/Program Files/Microsoft Visual Studio 11.0/VC/include/stdio.h"

# ATL/MFC headers and libs
extract_msi \
    "$PKG/vc_libraryselectablemfc/vc_libraryselectablemfc.msi" \
    "$VM_C" \
    "$VM_C/Program Files/Microsoft Visual Studio 11.0/VC/atlmfc/include/afx.h"

# MSBuild C++ targets (Microsoft.Cpp/v4.0/V110/)
extract_msi \
    "$PKG/vc_compilercoreres/vc_compilercoreres.msi" \
    "$VM_C" \
    "$VM_C/Program Files/MSBuild/Microsoft.Cpp/v4.0/V110/Microsoft.Cpp.Default.props"

# Windows SDK 8.0 — core headers/libs/tools (rc.exe, midl.exe, wininet.lib, etc.)
extract_msi \
    "$PKG/Windows_SDK/Windows Software Development Kit-x86_en-us.msi" \
    "$VM_C" \
    "$VM_C/Program Files/Windows Kits/8.0/Lib/win8/um/x64/wininet.lib"

# Windows SDK 8.0 — windows.h and main Win32 headers (in Metro Apps MSI)
extract_msi \
    "$PKG/Windows_SDK/Windows Software Development Kit for Metro style Apps-x86_en-us.msi" \
    "$VM_C" \
    "$VM_C/Program Files/Windows Kits/8.0/Include/um/Windows.h"

# Windows SDK 8.0 — .NET libs: mscoree.lib, corguids.lib, etc.
extract_msi \
    "$PKG/sdk_tools4/sdk_tools4.msi" \
    "$VM_C" \
    "$VM_C/Program Files/Windows Kits/8.0/Lib/Win8/um/x64/mscoree.lib"

# Merge SDK Lib dirs: main SDK MSI uses "win8", sdk_tools4 uses "Win8".
# On case-sensitive ext4 these are separate — merge Win8/ into win8/ then symlink.
SDK_LIB="$VM_C/Program Files/Windows Kits/8.0/Lib"
if [ -d "$SDK_LIB/Win8" ] && [ ! -L "$SDK_LIB/Win8" ]; then
    cp -rn "$SDK_LIB/Win8/." "$SDK_LIB/win8/" 2>/dev/null || true
    rm -rf "$SDK_LIB/Win8"
    ln -s "win8" "$SDK_LIB/Win8"
fi

info "VS 2012 components extracted."

# ---- Step 6b: Create amd64 symlink ----------------------------------------
# ISO only contains x86_amd64 (cross-compiler); UBT expects VC/bin/amd64/.
VC_BIN="$VM_C/Program Files/Microsoft Visual Studio 11.0/VC/bin"
if [ -e "$VC_BIN/amd64" ]; then
    skip "Step 6b: amd64 symlink already exists."
else
    info "Step 6b: Creating VC/bin/amd64 -> x86_amd64 symlink..."
    ln -s "x86_amd64" "$VC_BIN/amd64"
fi

# ---- Step 7: Fix CppTasks DLL case ----------------------------------------
# msiextract unpacks MSI paths relative to dest, e.g. "Program Files/MSBuild/..."
CPPTASKS_DIR="$VM_C/Program Files/MSBuild/Microsoft.Cpp/v4.0/V110"
UPPER="$CPPTASKS_DIR/Microsoft.Build.CPPTasks.Common.v110.dll"
LOWER="$CPPTASKS_DIR/Microsoft.Build.CppTasks.Common.v110.dll"
FROMGAC="$VM_C/Program Files/Microsoft Visual Studio 11.0/Common7/IDE/FromGAC/Microsoft.Build.CPPTasks.Common.v110.dll"

if [ -e "$LOWER" ]; then
    skip "Step 7: CppTasks symlink already exists."
else
    info "Step 7: Fixing CppTasks DLL case..."
    if [ -f "$FROMGAC" ] && [ ! -f "$UPPER" ]; then
        cp "$FROMGAC" "$UPPER"
    fi
    if [ -f "$UPPER" ]; then
        ln -s "$UPPER" "$LOWER"
        info "  Symlink created."
    else
        warn "  Source DLL not found — MSBuild C++ builds may fail."
    fi
fi

# ---- Step 8: Register CppTasks in Wine GAC --------------------------------
GAC_DIR="$VM_C/windows/Microsoft.NET/assembly/GAC_MSIL/Microsoft.Build.CppTasks.Common.v110/v4.0_4.0.0.0__b03f5f7f11d50a3a"
GAC_DLL="$GAC_DIR/Microsoft.Build.CppTasks.Common.v110.dll"

if [ -f "$GAC_DLL" ]; then
    skip "Step 8: CppTasks already registered in GAC."
else
    info "Step 8: Registering CppTasks in Wine GAC..."
    mkdir -p "$GAC_DIR"
    if [ -f "$UPPER" ]; then
        cp "$UPPER" "$GAC_DLL"
        info "  Registered in GAC_MSIL."
    else
        warn "  Source DLL not found — skipping GAC registration."
    fi
fi

# ---- Done -----------------------------------------------------------------
echo ""
info "Done. Wine prefix ready at: $PREFIX"
echo ""

# Ask to remove downloaded sources (~2.5 GB ISO + git clone)
printf "  Remove VS 2012 ISO and extracted files (~5 GB)? [y/N] "
read -r REPLY
if [[ "$REPLY" =~ ^[Yy]$ ]]; then
    rm -f "$ISO_FILE"
    rm -rf "$ISO_EXTRACTED"
    info "  VS 2012 ISO and extracted files removed."
fi

echo ""
echo "  Next: run ./build_wine.sh"
echo ""
