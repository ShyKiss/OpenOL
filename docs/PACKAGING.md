# Packaging

<p align="right"><a href="./PACKAGING_RU.md">🇷🇺 Русский</a></p>

To ship a release archive that users can drag-and-drop into their game folder, run [publish_production.sh](../Development/Src/Targets/publish_production.sh):

```sh
cd $OUTLASTSRC
./Development/Src/Targets/publish_production.sh
```

This produces `$OUTLASTSRC/PublishTemp/OpenOL.7z` (and `.zip`).

> Users may need to launch the game with `-log -nosteam -seekfreeloadingpcconsole` after installing.

---

## Release archive layout

```
├── OutlastLauncher.exe
├── Binaries/
│   └── Win64/
│       ├── OLGame.exe
│       └── OpenOL/
│           ├── locales/
│           │   ├── en.ini
│           │   └── ru.ini
│           └── res/
│               └── checkpoints/
├── OLGame/
│   ├── Config/
│   │   ├── DefaultEngine.ini
│   │   ├── DefaultGame.ini
│   │   ├── DefaultInput.ini
│   │   ├── DefaultMultiplayer.ini
│   │   └── DefaultUI.ini
│   ├── CookedPCConsole/
│   │   ├── menuassets.upk
│   │   ├── OLFrontEnd.upk
│   │   └── OpenOL/
│   │       ├── AkAudio.u
│   │       ├── Core.u
│   │       ├── Engine.u
│   │       ├── GameFramework.u
│   │       ├── GFxUI.u
│   │       ├── IpDrv.u
│   │       ├── Multiplayer.u
│   │       ├── OLGame.u
│   │       ├── OnlineSubsystemPC.u
│   │       ├── OnlineSubsystemSteamworks.u
│   │       ├── TASGame.u
│   │       └── WinDrv.u
│   └── Localization/
│       ├── DEU/olgame.DEU
│       ├── ESN/olgame.ESN
│       ├── FRA/olgame.FRA
│       ├── INT/OLGame.int
│       ├── ITA/olgame.ITA
│       ├── JPN/OLGame.JPN
│       ├── POL/olgame.POL
│       ├── PTB/OLGame.ptb
│       └── RUS/olgame.RUS
```

## Releases and the launcher's auto-update

`OutlastLauncher.exe` checks the latest GitHub release at startup and can install it. The update check is in [Updater.cpp](../Development/Tools/OutlastLauncher/Updater.cpp). A release is offered and installed only if:

1. `CURRENT_VERSION` in `Updater.cpp` was bumped before the launcher was built, and matches the tag.
2. The tag has the form `X.Y.Z` (an optional `v` prefix is allowed) and is higher than the installed version.
3. The release has an asset named exactly `OpenOL.zip`, as built by `publish_production.sh`. GitHub's SHA-256 digest of that asset is checked after the download.
4. The archive contains `OutlastLauncher.exe` and `Binaries/Win64/OLGame.exe`, and uses only Deflate or Store compression.

The update never deletes files, so a file removed from a release stays in existing installs.

Launcher 2.1.0 cannot install updates under Proton, because its unpacking relies on a Windows Shell feature that Wine does not have. Proton users on 2.1.0 have to install the next release by hand once.

The tests run under Wine or Proton, and use mingw-w64 instead of MSVC:

```bash
python3 Development/Tools/OutlastLauncher/Tests/test_updater.py
```
