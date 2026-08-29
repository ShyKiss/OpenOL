# Linux development environment setup

<p align="right"><a href="./LINUX_RU.md">🇷🇺 Русский</a></p>

See also: [Building the mod](./BUILD.md)

> Make sure the `$WINEPREFIX` you use matches the one configured in `build_wine.sh`.

---

## 1. Install Visual Studio 2012 under Wine

Wine version confirmed to work: **11.14**

Install required winetricks packages:

```sh
winetricks -q dotnet20 dotnet40 gdiplus corefonts riched20 atmlib msxml3 msls31
```

Run the VS installer:

```sh
wine vs_professional.exe
```

Select only:
- **Microsoft Foundation Classes for C++**

If Visual Studio asks to restart, kill the processes instead:

```sh
sudo pkill -9 -f "\.exe"
```

---

## 2. Install DirectX SDK (June 2010)

```sh
wine DXSDK_Jun10.exe
```

---

## 3. Building

```sh
cd $OUTLASTSRC/Development/Src/Targets
./build_wine.sh
```

See [BUILD.md](./BUILD.md) for the full list of build steps.

---

## 4. Editor

```sh
wine OLGame.exe editor -NoGADWarning

# Force WineD3D (software renderer):
WINEDLLOVERRIDES="d3d8=b;d3d9=b;d3d10core=b;d3d11=b;dxgi=b" wine OLGame.exe editor -NoGADWarning
```

---

## Troubleshooting

### Shader compiler error

```log
Warning, 0 Shader compiler errors compiling global for platform pc-d3d-sm3:
Critical: appError called: Failed to compile global shader TFilterPixelShader<16>
```

Fix:
```sh
winetricks -q d3dcompiler_43
```

If that doesn't help, try cooking manually:
```sh
wine ../../../Binaries/Win64/OLGame.exe CookPackages -platform=PCConsole -multilanguagecook=INT -VERBOSE
```

Also try:
- Set `bAllowMultiThreadedShaderCompile=False` in `Engine/Config/BaseEngine.ini`
- Delete `OLGame/Content/GlobalShaderCache-PC-D3D-SM3.bin`

### Fatal error on launch

```log
Fatal error!
Address = 0xfa041470 (filename not found)
```

Rebuild everything from scratch following the order in [BUILD.md](./BUILD.md).
