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

`OutlastLauncher.exe` при запуске проверяет последний релиз на GitHub и умеет его устанавливать (см. [Updater.cpp](../Development/Tools/OutlastLauncher/Updater.cpp)). Релизы выпускаются через [release.sh](../Development/Src/Targets/release.sh). Версия хранится в [Version.h](../Development/Tools/OutlastLauncher/Version.h), менять её должен только `release.sh bump`.

```bash
# Поднять версию (patch, minor, major или X.Y.Z) и закоммитить, на main или в рабочей ветке.
Development/Src/Targets/release.sh bump patch
# Довести её до main и запушить main, затем на main:
Development/Src/Targets/release.sh --dry-run   # собрать и проверить всё; без коммитов, тегов и загрузок
Development/Src/Targets/release.sh draft       # собрать, проверить, запушить тег, загрузить черновик, невидимый пользователям
# Поправить заметки на GitHub и попробовать архив из черновика.
Development/Src/Targets/release.sh publish     # спросит подтверждение и предложит релиз всем пользователям
```

С `--dry-run` можно указать `--skip-build`, чтобы проверить уже собранные бинарники. В другой ветке пробный запуск сообщает, что заблокировало бы релиз, и продолжает работу.

`draft` останавливается, ничего не меняя, если `HEAD` не является чистым запушенным `main`, версия не выше последнего тега или коммит relay отсутствует на GitHub. Затем он проверяет архив тем же кодом, которым его устанавливает лаунчер. Если `draft` упал после создания тега, устраните причину и запустите его снова: тег будет использован повторно, а файлы в черновике заменены. Если для исправления нужен новый коммит, нужна и новая версия.

Лаунчер принимает только релиз с файлом ровно `OpenOL.zip`, сжатием Deflate или Store и файлами `OutlastLauncher.exe` и `Binaries/Win64/OLGame.exe` внутри. После скачивания он проверяет SHA-256 этого файла, который сообщает GitHub. `release.sh` всё это обеспечивает.

Опубликованную версию нельзя забрать у тех, кто её установил: её можно только заменить более высокой. Обновление не удаляет файлы, поэтому файл, убранный из релиза, остаётся в уже установленных копиях.

### Бета-версии

Версии вида `2.2.0-rc1` публикуются на GitHub как prerelease. Они считаются старше `2.2.0`, поэтому тестер на такой версии получит предложение обновиться до финальной, когда она выйдет. Обычным пользователям бета-версии не предлагаются. Первую бету тестер ставит вручную, затем лаунчер предлагает и более новые. Бету можно выпустить из любой запушенной ветки.

```bash
release.sh bump minor --rc   # 2.1.0 -> 2.2.0-rc1
release.sh bump rc           # 2.2.0-rc1 -> 2.2.0-rc2
release.sh bump final        # 2.2.0-rc2 -> 2.2.0
```

### CI

Workflow `Build Mod` в [build.yml](../.github/workflows/build.yml) запускается только вручную. Он собирает архивы в [CI/Containerfile](../CI/Containerfile), а с включённым `create_draft` загружает их как черновик через `release.sh upload-draft`. Он никогда не публикует релиз: после проверки черновика запустите `release.sh publish`. Перед запуском поднимите версию и запушите её. **Этот workflow ещё ни разу не запускался, при первом использовании его, скорее всего, придётся отлаживать.** Локальному `release.sh draft` он не нужен.

Лаунчер 2.1.0 не может устанавливать обновления под Proton: он распаковывает архив средствами Windows Shell, которых нет в Wine. Пользователям Proton с версией 2.1.0 следующий релиз нужно один раз установить вручную.

Тесты запускаются под Wine или Proton и используют mingw-w64 вместо MSVC:

```bash
python3 Development/Tools/OutlastLauncher/Tests/test_updater.py
```
