# planetary-system-webgpu

Домашнее задание №1 по курсу «Компьютерная графика реального времени».
Графический API — WebGPU (через [wgpu-native](https://github.com/gfx-rs/wgpu-native)).

## Что внутри

Один исполняемый файл `planetary-system-webgpu`, который использует общие библиотеки
`math` (header-only математика) и `wgpu_app` (обёртка над SDL3 + wgpu-native для создания
окна и поверхности).

```text
homework-repo/
├── CMakeLists.txt
├── include/
│   ├── math/              # математика (вектора, матрицы, кватернионы и т.п.)
│   └── wgpu_app/          # заголовки обёртки над SDL3 + wgpu-native
├── resources/             # шейдеры, модели, текстуры
│   └── ...
├── src/
│   ├── main.cpp
│   └── wgpu_app/          # реализация обёртки
└── third_party/
    └── stb_image.h
```

Некоторые файлы взял из [репозитория практик курса](https://github.com/lisyarus/realtime-graphics-course-v2-practice)

## Требования

1. **CMake** версии не меньше 3.20
   * Windows: [скачайте установщик](https://cmake.org/download/) (например,
     [4.4.3 для x64](https://github.com/Kitware/CMake/releases/download/v4.4.3/cmake-4.4.3-windows-x86_64.msi))
     и при установке отметьте `Add CMake to current user PATH`
   * Linux: `apt install cmake` (или аналог для вашего дистрибутива)
   * macOS: [.dmg-образ](https://cmake.org/download/) либо `brew install cmake`

2. **SDL3**
   * Windows: скачайте **dev-архив**
     ([SDL3-devel-3.4.14-VC.zip](https://github.com/libsdl-org/SDL/releases/download/release-3.4.14/SDL3-devel-3.4.14-VC.zip))
     и распакуйте куда-нибудь
   * Linux: `apt install libsdl3-dev`
   * macOS: [.dmg-образ](https://github.com/libsdl-org/SDL/releases/download/release-3.4.14/SDL3-3.4.14.dmg)
     либо `brew install sdl3`

3. **wgpu-native** версии [v.29.0.1.1](https://github.com/gfx-rs/wgpu-native/releases/tag/v29.0.1.1)
   * Windows: [wgpu-windows-x86_64-msvc-release.zip](https://github.com/gfx-rs/wgpu-native/releases/download/v29.0.1.1/wgpu-windows-x86_64-msvc-release.zip)
   * Linux: [wgpu-linux-x86_64-release.zip](https://github.com/gfx-rs/wgpu-native/releases/download/v29.0.1.1/wgpu-linux-x86_64-release.zip)
   * macOS: [x86_64](https://github.com/gfx-rs/wgpu-native/releases/download/v29.0.1.1/wgpu-macos-x86_64-release.zip)
     или [aarch64](https://github.com/gfx-rs/wgpu-native/releases/download/v29.0.1.1/wgpu-macos-aarch64-release.zip),
     в зависимости от процессора, либо `brew version-install wgpu-native@29.0.1.1`

## Сборка и запуск

### Linux / macOS

```bash
mkdir build && cd build
cmake .. -DWGPU_NATIVE_ROOT='<путь до wgpu-native>'
cmake --build .
./planetary-system-webgpu/planetary-system-webgpu
```

### Windows

1. Создайте директорию сборки, например `build/` в корне репозитория.
2. Откройте терминал (PowerShell, Git Bash и т.п.) и перейдите в неё.
3. Сконфигурируйте проект:

   ```powershell
   cmake .. -G "<генератор>" `
            -DWGPU_NATIVE_ROOT="<путь до wgpu-native>" `
            -DCMAKE_PREFIX_PATH="<путь до SDL3>"
   ```

   * `<генератор>` — например, `"Visual Studio 17 2022"` (полный список —
     [в документации CMake](https://cmake.org/cmake/help/latest/manual/cmake-generators.7.html#visual-studio-generators)).
   * `<путь до wgpu-native>` — директория, куда распакован архив wgpu-native
     (внутри должны быть `include/`, `lib/`, `wgpu-native-meta/`).
   * `<путь до SDL3>` — директория, куда распакован архив SDL3
     (внутри должны быть `cmake/`, `include/`, `lib/` и несколько `.txt`/`.md`).

   В некоторых терминалах нужны двойные, а не одинарные кавычки.

4. Соберите проект:

   ```powershell
   cmake --build .
   ```

5. Скопируйте `SDL3.dll` и `wgpu_native.dll` рядом с исполняемым файлом
   `build/planetary-system-webgpu/planetary-system-webgpu.exe`.
   В Windows нет концепции RPATH, поэтому это придётся делать каждый раз после
   чистой пересборки.

6. Запустите `planetary-system-webgpu.exe`.

