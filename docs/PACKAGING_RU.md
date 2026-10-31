# Упаковка релиза

<p align="right"><a href="./PACKAGING.md">🇺🇸 English</a></p>

Чтобы собрать архив для пользователей, который можно просто перетащить в папку игры, запустите [publish_production.sh](../Development/Src/Targets/publish_production.sh):

```sh
cd $OUTLASTSRC
./Development/Src/Targets/publish_production.sh
```

Результат: `$OUTLASTSRC/PublishTemp/OpenOL.7z` (и `.zip`).

> Пользователям может потребоваться запустить игру с `-log -nosteam -seekfreeloadingpcconsole` после установки.

---

## Структура архива релиза

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

## Релизы и автообновление лаунчера

`OutlastLauncher.exe` при запуске проверяет последний релиз на GitHub и умеет его устанавливать. Код находится в [Updater.cpp](../Development/Tools/OutlastLauncher/Updater.cpp). Релиз будет предложен и установлен, только если:

1. `CURRENT_VERSION` в `Updater.cpp` повышен до сборки лаунчера и совпадает с тегом.
2. Тег имеет вид `X.Y.Z` (допускается префикс `v`) и выше установленной версии.
3. В релизе есть файл с именем ровно `OpenOL.zip`, собранный `publish_production.sh`. После скачивания проверяется его SHA-256, который сообщает GitHub.
4. В архиве есть `OutlastLauncher.exe` и `Binaries/Win64/OLGame.exe`, а сжатие только Deflate или Store.

Обновление не удаляет файлы, поэтому файл, убранный из релиза, остаётся в уже установленных копиях.

Лаунчер 2.1.0 не может устанавливать обновления под Proton: он распаковывает архив средствами Windows Shell, которых нет в Wine. Пользователям Proton с версией 2.1.0 следующий релиз нужно один раз установить вручную.

Тесты запускаются под Wine или Proton и используют mingw-w64 вместо MSVC:

```bash
python3 Development/Tools/OutlastLauncher/Tests/test_updater.py
```
