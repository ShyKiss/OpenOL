# Настройка окружения для разработки на Linux

<p align="right"><a href="./LINUX.md">🇺🇸 English</a></p>

См. также: [Сборка](./BUILD_RU.md)

> Убедитесь, что `$WINEPREFIX` совпадает с тем, что указан в `build_wine.sh`.

---

## Автоматическая настройка (рекомендуется)

Скрипт `setup_wineprefix.sh` автоматически скачивает VS 2012 и DirectX SDK и настраивает весь Wine-префикс:

```sh
WINEPREFIX=/path/to/prefix ./Development/Src/Targets/setup_wineprefix.sh
```

После завершения сразу можно собирать:

```sh
cd Development/Src/Targets
./build_wine.sh
```

---

## Ручная настройка

### 1. Установка Visual Studio 2012 под Wine

Подтверждённая версия Wine: **11.14**

Установите необходимые пакеты winetricks:

```sh
winetricks -q dotnet20 dotnet40 gdiplus corefonts riched20 atmlib msxml3 msls31
```

Запустите установщик VS:

```sh
wine vs_professional.exe
```

Выберите только:
- **Microsoft Foundation Classes for C++**

Если Visual Studio предложит перезагрузку, завершите процессы вместо этого:

```sh
sudo pkill -9 -f "\.exe"
```

### 2. Установка DirectX SDK (June 2010)

```sh
wine DXSDK_Jun10.exe
```

### 3. Сборка

```sh
cd $OUTLASTSRC/Development/Src/Targets
./build_wine.sh
```

Полный список шагов сборки — в [BUILD_RU.md](./BUILD_RU.md).

---

## Редактор

```sh
wine OLGame.exe editor -NoGADWarning

# Принудительно использовать WineD3D (программный рендерер):
WINEDLLOVERRIDES="d3d8=b;d3d9=b;d3d10core=b;d3d11=b;dxgi=b" wine OLGame.exe editor -NoGADWarning
```

---

## Решение проблем

### Ошибка компилятора шейдеров

```log
Warning, 0 Shader compiler errors compiling global for platform pc-d3d-sm3:
Critical: appError called: Failed to compile global shader TFilterPixelShader<16>
```

Исправление:
```sh
winetricks -q d3dcompiler_43
```

Если не помогает, попробуйте cook вручную:
```sh
wine ../../../Binaries/Win64/OLGame.exe CookPackages -platform=PCConsole -multilanguagecook=INT -VERBOSE
```

Также попробуйте:
- Установить `bAllowMultiThreadedShaderCompile=False` в `Engine/Config/BaseEngine.ini`
- Удалить `OLGame/Content/GlobalShaderCache-PC-D3D-SM3.bin`

### Fatal error при запуске

```log
Fatal error!
Address = 0xfa041470 (filename not found)
```

Пересоберите всё с нуля в порядке, указанном в [BUILD_RU.md](./BUILD_RU.md).
