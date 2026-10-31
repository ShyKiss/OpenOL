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

`OutlastLauncher.exe` checks the latest GitHub release at startup and can install it (see [Updater.cpp](../Development/Tools/OutlastLauncher/Updater.cpp)). Releases are made with [release.sh](../Development/Src/Targets/release.sh). The version lives in [Version.h](../Development/Tools/OutlastLauncher/Version.h), and only `release.sh bump` should change it.

```bash
# On a work branch: raise the version (patch, minor, major or X.Y.Z) and commit it.
Development/Src/Targets/release.sh bump patch
# Merge the branch into main and push main, then, on main:
Development/Src/Targets/release.sh --dry-run   # build and check everything; nothing is committed, tagged or uploaded
Development/Src/Targets/release.sh draft       # build, check, push the tag, upload a draft that users cannot see
# Edit the release notes on GitHub and try the archive from the draft.
Development/Src/Targets/release.sh publish     # asks for confirmation, then offers it to all users
```

Add `--skip-build` to `--dry-run` to check the binaries already on disk. A dry run on another branch reports what would block a release and carries on.

`draft` stops before changing anything unless `HEAD` is a clean, pushed `main`, the version is higher than the newest tag, and the relay commit is on GitHub. It then checks the archive with the code the launcher uses to install it. If `draft` fails after tagging, fix the problem and run it again: it reuses the tag and replaces the files in the draft. A fix that needs a new commit needs a new version.

The launcher only accepts a release that has an asset named exactly `OpenOL.zip`, with Deflate or Store compression, containing `OutlastLauncher.exe` and `Binaries/Win64/OLGame.exe`. It checks GitHub's SHA-256 digest of that asset after the download. `release.sh` produces all of this.

A published version cannot be taken back from those who installed it, only replaced by a higher one. The update never deletes files, so a file removed from a release stays in existing installs.

Launcher 2.1.0 cannot install updates under Proton, because its unpacking relies on a Windows Shell feature that Wine does not have. Proton users on 2.1.0 have to install the next release by hand once.

The tests run under Wine or Proton, and use mingw-w64 instead of MSVC:

```bash
python3 Development/Tools/OutlastLauncher/Tests/test_updater.py
```
