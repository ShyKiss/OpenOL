# Сборка OpenOL

<p align="right"><a href="./BUILD.md">🇺🇸 English</a></p>

См. также: [Настройка Linux](./LINUX_RU.md) | [Упаковка релиза](./PACKAGING_RU.md)

## Требования

| Компонент | Требование |
|-----------|------------|
| Исходники Outlast | Обязательно (здесь не предоставляются) |
| Visual Studio 2012 | [Скачать](https://archive.org/details/en_visual_studio_professional_2012_x86_dvd) |
| DirectX SDK (June 2010) | [Скачать](https://download.microsoft.com/download/A/E/7/AE743F1F-632B-4809-87A9-AA1BB3458E31/DXSDK_Jun10.exe) · [Извлечённая копия](https://github.com/testing-laboratory/DirectX-SDK-June2010) |
| Wine 11+ *(только Linux)* | Для запуска MSVC под Linux |
| cmake + ninja *(для Relay)* | Для отдельного relay-сервера |
| mingw-w64 *(Linux -> Windows кросс)* | `sudo pacman -S mingw-w64-gcc` |

**Ссылки на исходники Outlast здесь не предоставляются.**

После получения исходников распакуйте следующие папки в `$OUTLASTSRC`:

```
┌─── Binaries/        5 GB
├─── Development/
│    ├── External/    30 GB
│    ├── Src/         8.6 GB
│    └── Tools/       350 MB
├─── Engine/          300 MB
└─── OLGame/          22.6 GB
```

Затем скопируйте папки `Development` и `OLGame` из этого репозитория в `$OUTLASTSRC/`, подтверждая замену файлов.

---

## Порядок сборки

Собирайте компоненты в следующем порядке:

1. **UnrealBuildTool** — нужен перед сборкой UnrealScript
2. **openol_relay.lib** и **imgui.lib** — нужны перед сборкой OLGame.exe
3. **OLGame.exe** (и OutlastLauncher)
4. **UnrealScript** и Cook Packages

---

## Linux — build_wine.sh

Вся компиляция C++ и UnrealScript выполняется через `build_wine.sh` (Wine + MSVC 2012).

> Сначала настройте Wine-окружение — см. [LINUX_RU.md](./LINUX_RU.md).

```bash
cd $OUTLASTSRC/Development/Src/Targets
./build_wine.sh        # интерактивное меню
./build_wine.sh <N>    # запустить шаг N напрямую
```

| Шаг | Действие |
|-----|----------|
| 1 | Сборка OLGame.exe (инкрементальная — только изменённые файлы) |
| 2 | Пересборка OLGame.exe (полная, удаляет все .obj) |
| 3 | Сборка OLGame Production (`WITH_EDITOR=0`, инкрементальная) |
| 4 | Пересборка OLGame Production (`WITH_EDITOR=0`, удаляет все .obj) |
| 5 | Сборка OutlastLauncher (C++ MSVC, Release\|Win32) |
| 6 | Сборка UnrealBuildTool (C#, MSBuild Release) |
| 7 | Сборка UnrealScript (make -auto, только изменённые .uc) |
| 8 | Полная пересборка UnrealScript (make -auto -full, все .uc) |
| 9 | Cook Packages |
| 10 | Сборка openol_browser (CEF offscreen, MinGW-w64 x64) |
| 11 | Сборка openol_relay.lib (статическая lib, MSVC Release\|x64) |
| 12 | Сборка imgui.lib (статическая lib, MSVC Release\|x64) |

**После сборки OLGame.exe** скопируйте файлы в директорию игры:

```
Binaries/Win64/Editor/Release/OLGame.exe
Binaries/Win64/Editor/Release/UnrealEdCSharp.dll
```

---

## Windows — Visual Studio 2012

1. Установите Visual Studio 2012 и DirectX SDK (June 2010)
   - При установке VS 2012 убедитесь, что в выбранных компонентах включён **.NET Framework 4.5** (требуется для UnrealBuildTool)
2. Добавьте системную переменную окружения `DXSDK_DIR`, указывающую на папку установки DirectX SDK (например, `C:\Program Files (x86)\Microsoft DirectX SDK (June 2010)\`)
3. Откройте `Development/Src/Windows/Windows.vcxproj` (или родительский solution)
4. Выберите конфигурацию **Release | Mixed Platforms**
5. В Solution Explorer: ПКМ по **OLGame Win64** -> **Build** (или **OLGame Win64 Production** для production-сборки)

---

## OpenOL Relay (отдельный) — Linux

```bash
cd $OUTLASTSRC/Development/Src/Multiplayer/server

# TUI-сборка (консольный интерфейс)
./build.sh

# GUI-сборка (ImGui + SDL2 + OpenGL)
./build.sh --gui
```

Результат в `build/`:
- `OpenOL_Relay` — TUI-версия
- `OpenOL_Relay_GUI` — GUI-версия

**Вручную через CMake:**
```bash
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release [-DBUILD_GUI=ON]
cmake --build build
```

---

## OpenOL Relay (отдельный) — Windows (кросс-компиляция с Linux)

```bash
cd $OUTLASTSRC/Development/Src/Multiplayer/server

# TUI-сборка
./build_win.sh

# GUI-сборка (требует Qt6 для mingw)
export QT6_MINGW_DIR=/path/to/Qt/6.x.x/mingw_64
./build_win.sh --gui
```

Результат в `build_win/`:
- `OpenOL_Relay.exe`
- `OpenOL_Relay_GUI.exe` (если передан `--gui`)

**Вручную через CMake:**
```bash
cmake -S . -B build -G Ninja \
  -DCMAKE_C_COMPILER=x86_64-w64-mingw32-gcc \
  -DCMAKE_CXX_COMPILER=x86_64-w64-mingw32-g++ \
  -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

---

## Параметры запуска

| Параметр | Описание |
|----------|----------|
| `-make` | Запустить компилятор UnrealScript (make) при старте |
| `-nosteam` | Отключить Steam |
| `-log` | Показать окно лога |
| `-forcedebuginfo` | Принудительная отладочная информация |
| `-silent` | Тихий режим |
| `-seekfreeloading` | Режим seekfree загрузки |
| `-seekfreeloadingpcconsole` | Seekfree загрузка (PC console) |
| `-VERBOSE` | Подробный вывод |

---

## CI / Containerfile (экспериментально)

Поместите `Outlast1.7z` в корень репозитория, затем:

```sh
podman build --device /dev/dri -t openol -f CI/Containerfile .
```

> **Примечание:** CI требует premium или self-hosted runner — исходники весят 20+ ГБ.
> Укажите ссылку `$OUTLASTSRC` как секрет. См. [workflows](../.github/workflows).
