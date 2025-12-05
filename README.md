# GnotePad

GnotePad is a cross-platform Qt 6 clone of the Windows 10 Notepad experience. The project targets modern C++23, depends on clang + CMake, and keeps its functionality portable across Linux and Windows.

Current work happens on the `feature/user-preferences` branch, which adds persistent settings, MRU tracking, and richer editor controls on top of the original scaffold.

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
cmake -S . -B build/debug -G Ninja \
  -DCMAKE_CXX_COMPILER=clang++ \
  -DCMAKE_PREFIX_PATH=/usr/lib/x86_64-linux-gnu/cmake/Qt6 \
  -DCMAKE_BUILD_TYPE=Debug
cmake --build build/debug
ctest --test-dir build/debug
```

Adjust `-DCMAKE_PREFIX_PATH` to match your Qt6 installation (e.g., `/opt/Qt/6.xx/gcc_64/lib/cmake/Qt6`). Swap `Debug` for `Release` or `RelWithDebInfo` as needed.

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

After building, launch the executable from `build/<config>/GnotePad` (Linux/macOS) or `build/<config>/GnotePad.exe` (Windows). The current feature set includes:
- Native QFileDialog open/save flows with UTF BOM detection, encoding picker, and per-file encoding display
- Persistent preferences via QSettings (window geometry, MRU list, directories, tab size, word wrap, line numbers, zoom, status bar visibility)
- Rich editor surface powered by `TextEditor` (line-number gutter, zoom in/out/reset, configurable tab spacing, cursor + document statistics, encoding indicator)
- Find/Replace dialogs, Go To line, time/date insertion, PDF export, and recent-files submenu mirroring Windows Notepad
- Structured logging through spdlog and CTest-driven smoke tests

## Development Notes

- Sources live in `src/` and are split by responsibility (`app/`, `ui/`).
- Qt resources (icons, translations, etc.) live under `resources/`.
- Logging is provided by spdlog through CMake FetchContent integration.
- Tests now use Qt Test (QTest) via CTest. The `GnotePadSmoke` suite spins up the real `MainWindow` to verify launch/minimize/maximize flows; future additions will cover file I/O, MRU, encoding round-trips, UI automation (find/replace, zoom, insert date), and large-file scrolling.

## Static Analysis & Tooling

- **clang-tidy** – The repo ships a `.clang-tidy` profile plus a CMake toggle. Run `cmake -S . -B build/debug -G Ninja -DCMAKE_CXX_COMPILER=clang++ -DCMAKE_PREFIX_PATH=/usr/lib/x86_64-linux-gnu/cmake/Qt6 -DCMAKE_BUILD_TYPE=Debug -DGNOTE_ENABLE_CLANG_TIDY=ON && cmake --build build/debug` or invoke the VS Code task **Clang-Tidy (Debug)**. You can also call `cmake --build build/debug --target run-clang-tidy` after configuring to re-check just the sources.
- **scan-build (clang static analyzer)** – Use `tools/run-scan-build.sh` (defaults to `build/analyze`) or the VS Code task **Scan-Build (Debug)**. Reports land in `scan-build-report/` and can be opened in a browser.
- **clang-format** – Formatting rules live in `.clang-format`. Run `cmake --build build/debug --target run-clang-format` (or the **Clang-Format** VS Code task) to apply styling in-place. Configure editors to honor the same profile for on-save formatting.
- **Editor configuration** – VS Code's C/C++ IntelliSense engine is disabled to avoid conflicting diagnostics; clangd via CMake Tools drives code completion using the generated `compile_commands.json`.
  - Install the [clangd extension](https://marketplace.visualstudio.com/items?itemName=llvm-vs-code-extensions.vscode-clangd) (`llvm-vs-code-extensions.vscode-clangd`) for full IDE features.
  - The extension is automatically recommended when opening the workspace.
  - `.vscode/settings.json` configures clangd to use the compilation database from `build/compile_commands.json`.
  ```bash
  cmake -S . -B build/debug -G Ninja -DCMAKE_CXX_COMPILER=clang++ -DCMAKE_PREFIX_PATH=/usr/lib/x86_64-linux-gnu/cmake/Qt6 -DCMAKE_BUILD_TYPE=Debug
  cmake --build build/debug --target run-clang-format
- **Editor configuration** – VS Code’s C/C++ IntelliSense engine is disabled to avoid conflicting diagnostics; clangd via CMake Tools drives code completion using the generated `compile_commands.json`.
- **Other clang utilities** – clangd powers completions, and clang-query/clang-apply-replacements can be layered on later for AST exploration or batch rewrites if needed.

## Next Steps

- Add broader Unicode/encoding regression tests using published sample corpora and round-trip validation
- Implement remaining Notepad UX (Page Setup, print preview, font dialog persistence, multi-document handling)
- Package builds for macOS/Windows/Linux (AppImage/MSIX/dmg) plus desktop integration assets
- Investigate large-file performance improvements and background loading indicators
