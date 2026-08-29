# Building OpenOL

<p align="right"><a href="./BUILD_RU.md">🇷🇺 Русский</a></p>

See also: [Linux environment setup](./LINUX.md) | [Packaging a release](./PACKAGING.md)

## Requirements

| Component | Requirement |
|-----------|-------------|
| Outlast source | Required (not provided here) |
| Visual Studio 2012 | [Download](https://archive.org/details/en_visual_studio_professional_2012_x86_dvd) |
| DirectX SDK (June 2010) | [Download](https://download.microsoft.com/download/A/E/7/AE743F1F-632B-4809-87A9-AA1BB3458E31/DXSDK_Jun10.exe) · [Extracted mirror](https://github.com/testing-laboratory/DirectX-SDK-June2010) |
| Wine 11+ *(Linux only)* | For running MSVC under Linux |
| cmake + ninja *(for Relay)* | For the standalone relay server |
| mingw-w64 *(Linux -> Windows cross)* | `sudo pacman -S mingw-w64-gcc` |

**We won't be providing any links to the Outlast source here.**

Once you have it, extract these folders into `$OUTLASTSRC`:

```
┌─── Binaries/        5 GB
├─── Development/
│    ├── External/    30 GB
│    ├── Src/         8.6 GB
│    └── Tools/       350 MB
├─── Engine/          300 MB
└─── OLGame/          22.6 GB
```

Then copy the `Development` and `OLGame` folders from this repository into `$OUTLASTSRC/`, replacing when asked.

---

## Build order

Build components in this order:

1. **UnrealBuildTool** — needed before building UnrealScript
2. **openol_relay.lib** and **imgui.lib** — needed before building OLGame.exe
3. **OLGame.exe** (and OutlastLauncher)
4. **UnrealScript** and Cook Packages

---

## Linux — build_wine.sh

All C++ and UnrealScript compilation runs through `build_wine.sh` (Wine + MSVC 2012).

> See [LINUX.md](./LINUX.md) to set up the Wine environment first.

```bash
cd $OUTLASTSRC/Development/Src/Targets
./build_wine.sh        # interactive menu
./build_wine.sh <N>    # run step N directly
```

| Step | Action |
|------|--------|
| 1 | Build OLGame.exe (incremental — only changed files) |
| 2 | Rebuild OLGame.exe (full rebuild, deletes all .obj) |
| 3 | Build OLGame Production (`WITH_EDITOR=0`, incremental) |
| 4 | Rebuild OLGame Production (`WITH_EDITOR=0`, deletes all .obj) |
| 5 | Build OutlastLauncher (C++ MSVC, Release\|Win32) |
| 6 | Build UnrealBuildTool (C#, MSBuild Release) |
| 7 | Build UnrealScript (make -auto, compiles only changed .uc files) |
| 8 | Rebuild UnrealScript (make -auto -full, full recompile of all .uc) |
| 9 | Cook Packages |
| 10 | Build openol_browser (CEF offscreen, MinGW-w64 x64) |
| 11 | Build openol_relay.lib (static lib, MSVC Release\|x64) |
| 12 | Build imgui.lib (static lib, MSVC Release\|x64) |

**After building OLGame.exe**, copy output to the game directory:

```
Binaries/Win64/Editor/Release/OLGame.exe
Binaries/Win64/Editor/Release/UnrealEdCSharp.dll
```

---

## Windows — Visual Studio 2012

1. Install Visual Studio 2012 and DirectX SDK (June 2010)
   - During VS 2012 installation, make sure **.NET Framework 4.5** is included in the selected components (required for UnrealBuildTool)
2. Add a system environment variable `DXSDK_DIR` pointing to the DirectX SDK install folder (e.g. `C:\Program Files (x86)\Microsoft DirectX SDK (June 2010)\`)
3. Open `Development/Src/Windows/Windows.vcxproj` (or the parent solution)
4. Set configuration to **Release | Mixed Platforms**
5. In Solution Explorer, right-click **OLGame Win64** -> **Build** (or **OLGame Win64 Production** for a production build)

---

## OpenOL Relay (standalone) — Linux

```bash
cd $OUTLASTSRC/Development/Src/Multiplayer/server

# TUI build (console interface)
./build.sh

# GUI build (ImGui + SDL2 + OpenGL)
./build.sh --gui
```

Output in `build/`:
- `OpenOL_Relay` — TUI version
- `OpenOL_Relay_GUI` — GUI version

**Manual CMake:**
```bash
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release [-DBUILD_GUI=ON]
cmake --build build
```

---

## OpenOL Relay (standalone) — Windows (cross-compile from Linux)

```bash
cd $OUTLASTSRC/Development/Src/Multiplayer/server

# TUI build
./build_win.sh

# GUI build (requires Qt6 for mingw)
export QT6_MINGW_DIR=/path/to/Qt/6.x.x/mingw_64
./build_win.sh --gui
```

Output in `build_win/`:
- `OpenOL_Relay.exe`
- `OpenOL_Relay_GUI.exe` (if `--gui` was passed)

**Manual CMake:**
```bash
cmake -S . -B build -G Ninja \
  -DCMAKE_C_COMPILER=x86_64-w64-mingw32-gcc \
  -DCMAKE_CXX_COMPILER=x86_64-w64-mingw32-g++ \
  -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

---

## Launch options

| Option | Description |
|--------|-------------|
| `-make` | Run UnrealScript compiler (make) on launch |
| `-nosteam` | Disable Steam |
| `-log` | Show log window |
| `-forcedebuginfo` | Force debug info |
| `-silent` | Silent mode |
| `-seekfreeloading` | Seekfree loading mode |
| `-seekfreeloadingpcconsole` | Seekfree loading (PC console) |
| `-VERBOSE` | Verbose output |

---

## CI / Containerfile (Experimental)

Place `Outlast1.7z` in the repository root, then:

```sh
podman build --device /dev/dri -t openol -f CI/Containerfile .
```

> **Note:** CI requires a premium or self-hosted runner — the source is 20+ GB.
> Define the `$OUTLASTSRC` link as a secret. See [the workflows](../.github/workflows).
