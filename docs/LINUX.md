# How to setup development environment on linux

# Disclaimer: Make sure the `$WINEPREFIX` you're using is as same as the one in [build_wine.sh](../Src/Targets/build_wine.sh)

### 1.Install visual studio 2012

Wine version confirmed to work: 11.14

Install these winetricks packages

```sh
winetricks -q dotnet20 dotnet40 gdiplus corefonts riched20 atmlib msxml3 msls31
```

Run `vs_professional.exe`

Pick only;
- Microsoft Foundation Classes for C++

Run `sudo pkill -9 -f "\\.exe"` if visual studio asks for some sort of restart.

### 2. Install DirectX SDK
Run `DXSDK_Jun10.exe`

### 3. Building
Cd into `$OUTLASTSRC/Development/Src/Targets` and run `build_wine.sh`

# Troubleshooting
Shader error;

```log
Warning, 0 Shader compiler errors compiling global for platform pc-d3d-sm3:
Critical: appError called: Failed to compile global shader TFilterPixelShader<16>
Failed to compile global shader TFilterPixelShader<16>
```

Solution:

winetricks -q d3dcompiler_43

Use this command for it instead of the script "wine ../../../Binaries/Win64/OLGame.exe CookPackages -platform=PCConsole -multilanguagecook=INT -VERBOSE"

Maybe Set "Engine/Config/BaseEngine.ini" bAllowMultiThreadedShaderCompile=False

Maybe Delete OLGame/Content/GlobalShaderCache-PC-D3D-SM3.bin

Fatal error of some sort;
```log
OLGame - Release
Analyzing...

Fatal error!
Address = 0xfa041470 (filename not found)
```

Solution:
Compile everything from scratch like in [BUILD.md](./BUILD.md)

# Setting up the editor

```sh
wine OLGame.exe editor -NoGADWarning
WINEDLLOVERRIDES="d3d8=b;d3d9=b;d3d10core=b;d3d11=b;dxgi=b" wine OLGame.exe editor -NoGADWarning # To force WineD3D
```
