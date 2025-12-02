# GnotePad

GnotePad is a cross-platform Qt 6 clone of the Windows 10 Notepad experience. The project targets modern C++23, depends on clang + CMake, and keeps its functionality portable across Linux and Windows.

## Prerequisites

- CMake 3.26+
- Ninja or Make (Ninja recommended)
- Clang 15+ (or Apple Clang 15+ on macOS)
- Qt 6.5+ development packages (Widgets, Gui, Core, PrintSupport)
- Python 3.x (optional, for future tooling)

On **Ubuntu/Debian** you can install requirements with:

```bash
sudo apt install clang ninja-build qt6-base-dev qt6-base-dev-tools qt6-tools-dev cmake git
```

On **Windows** (PowerShell with winget and vcpkg/Qt online installer):
1. Install LLVM: `winget install LLVM.LLVM`
2. Install CMake + Ninja: `winget install Kitware.CMake Ninja-build.Ninja`
3. Install Qt 6 (MSVC or MinGW) via the Qt Maintenance Tool or vcpkg (`vcpkg install qtbase:x64-windows`).
4. Ensure `clang-cl.exe` and Qt `bin/` folders are on your PATH before configuring.

## Configure & Build (Linux/macOS)

```bash
cmake -S . -B build -G Ninja \
  -DCMAKE_CXX_COMPILER=clang++ \
  -DCMAKE_BUILD_TYPE=RelWithDebInfo
cmake --build build
ctest --test-dir build
```

## Configure & Build (Windows, clang-cl)

```powershell
cmake -S . -B build -G "Ninja" `
  -DCMAKE_C_COMPILER=clang-cl `
  -DCMAKE_CXX_COMPILER=clang-cl `
  -DCMAKE_PREFIX_PATH="C:/Qt/6.6.3/msvc2019_64" `
  -DCMAKE_BUILD_TYPE=RelWithDebInfo
cmake --build build
ctest --test-dir build
```

## Running

After building, launch the executable from `build/GnotePad` (Linux/macOS) or `build/GnotePad.exe` (Windows). Initial functionality includes:
- Qt-based main window with menu structure matching Notepad
- Status bar w/ cursor position, encoding, zoom placeholders
- Placeholder file operations, PDF export stub, and logging via spdlog

## Development Notes

- Sources live in `src/` and are split by responsibility (`app/`, `ui/`).
- Qt resources (icons, translations, etc.) live under `resources/`.
- Logging is provided by spdlog through CMake FetchContent integration.
- Tests live in `tests/` and are hooked into CTest; extend them as behavior solidifies.

## Next Steps

- Flesh out document model (encoding management, BOM detection, MRU history)
- Implement Notepad-accurate dialogs (Find/Replace, Go To, Page Setup)
- Add configurable line-number gutter, zoom UI, and PDF viewer launch per platform
- Expand automated test coverage (Qt Test + document IO unit tests)
