# GnotePad

GnotePad is a cross-platform Qt 6 clone of the Windows 10 Notepad experience. The project targets modern C++23, depends on clang + CMake, and keeps its functionality portable across Linux and Windows.

Active development tracks the backlog outlined below on `main`, folding in incremental UX and tooling improvements as they land.

## Prerequisites

- CMake 3.26+
- Ninja or Make (Ninja recommended)
- Clang 15+ (or Apple Clang 15+ on macOS)
- Qt 6.5+ development packages (Widgets, Gui, Core, PrintSupport)
- Python 3.x (optional; not required for building/running)

On **Ubuntu/Debian** you can install requirements with:

```bash
sudo apt install clang ninja-build qt6-base-dev qt6-base-dev-tools qt6-tools-dev cmake git
```

On **Windows** (PowerShell + vcpkg, clang++/lld, Qt6 via vcpkg):
1. Install LLVM: `winget install LLVM.LLVM`
2. Install CMake + Ninja: `winget install Kitware.CMake Ninja-build.Ninja`
3. Install Qt: `vcpkg install qtbase:x64-windows qtsvg:x64-windows`
4. Ensure `C:/Program Files/LLVM/bin` is on PATH (contains `clang++.exe` and `lld-link.exe`).
5. Note: vcpkg layouts plugins under `.../installed/x64-windows/Qt6/plugins` (debug plugins under `.../debug/Qt6/plugins`). CMake now stages the Qt runtime (bin + plugins) plus a `qt.conf` into each `build/win-*` output, so you can launch `GnotePad.exe` directly from those folders without setting plugin environment variables.
6. If CMake cannot find Qt6, point it at the vcpkg install: set `$env:Qt6_DIR="$env:VCPKG_ROOT/installed/x64-windows/share/Qt6"` and optionally `$env:QT6_PREFIX_PATH="$env:VCPKG_ROOT/installed/x64-windows"` before running configure or the VS Code tasks.

### Windows toolchain notes (keep paths latest-first)

- Keep **LLVM** and **CMake** current: `winget upgrade LLVM.LLVM` and `winget upgrade Kitware.CMake` (Ninja usually updates with CMake). New installs are the same commands with `install` instead of `upgrade`.
- Keep **vcpkg** current: clone to `${env:USERPROFILE}\source\vcpkg` with `git clone https://github.com/microsoft/vcpkg %USERPROFILE%\source\vcpkg` and bootstrap via `%USERPROFILE%\source\vcpkg\bootstrap-vcpkg.bat -disableMetrics`. Periodically `git pull` in that directory to stay up to date. The CMake toolchain path for this repo expects `VCPKG_ROOT` pointing at that folder.
- PATH order should prefer your freshly installed tools ahead of older Visual Studio toolchains: `${env:LLVM_ROOT}\bin` (or `C:/Program Files/LLVM/bin`), `${env:CMAKE_ROOT}\bin` (or `C:/Program Files/CMake/bin`), then `%VCPKG_ROOT%`.
- `Devshell-Updated.ps1` (in the repo root) prepends LLVM, CMake, and vcpkg to PATH and exports `LLVM_ROOT`, `CMAKE_ROOT`, and `VCPKG_ROOT` for the current PowerShell session. Run it before configuring/building in a new shell: `pwsh -NoProfile -ExecutionPolicy Bypass -File .\Devshell-Updated.ps1`.

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

## Configure & Build (Windows, clang++/lld, vcpkg Qt6)

```powershell
cmake -S . -B build/win-debug -G Ninja `
  -DCMAKE_CXX_COMPILER="C:/Program Files/LLVM/bin/clang++.exe" `
  -DCMAKE_C_STANDARD=17 -DCMAKE_C_STANDARD_REQUIRED=ON `
  -DCMAKE_CXX_STANDARD=23 -DCMAKE_CXX_STANDARD_REQUIRED=ON -DCMAKE_CXX_EXTENSIONS=OFF `
  -DCMAKE_TOOLCHAIN_FILE="$env:USERPROFILE/source/vcpkg/scripts/buildsystems/vcpkg.cmake" `
  -DQt6_DIR="$env:VCPKG_ROOT/installed/x64-windows/share/Qt6" `
  -DCMAKE_PREFIX_PATH="$env:VCPKG_ROOT/installed/x64-windows" `
  -DCMAKE_BUILD_TYPE=Debug `
  -DCMAKE_EXE_LINKER_FLAGS="-fuse-ld=lld" -DCMAKE_SHARED_LINKER_FLAGS="-fuse-ld=lld"
cmake --build build/win-debug
```

For release-like builds, swap `build/win-debug`/`Debug` for `build/win-relwithdebinfo` + `RelWithDebInfo`, `build/win-release` + `Release`, or `build/win-optimized` + `Release` with IPO enabled (see VS Code tasks). VS Code tasks (`Build Debug (Windows)`, `Build RelWithDebInfo (Windows)`, etc.) already use these flags and the vcpkg toolchain.

## Running

After building, launch the executable from `build/<config>/GnotePad` (Linux/macOS) or `build/<config>/GnotePad.exe` (Windows).

Windows runtime note (Qt platform plugin):
- vcpkg places platform plugins at `.../installed/x64-windows/Qt6/plugins/platforms/qwindows.dll` (debug: `.../debug/Qt6/plugins/platforms/qwindowsd.dll`).
- CMake post-build steps copy the appropriate Qt bin and plugin trees plus a `qt.conf` into each `build/win-*` folder. You can run `GnotePad.exe` from those folders without setting `QT_PLUGIN_PATH`/`QT_QPA_PLATFORM_PLUGIN_PATH`.

The current feature set includes:
- Native QFileDialog open/save flows with UTF BOM detection, encoding picker, and per-file encoding display
- Persistent preferences via QSettings (window geometry, MRU list, directories, tab size, word wrap, line numbers, zoom, status bar visibility)
- Rich editor surface powered by `TextEditor` (line-number gutter, zoom in/out/reset, configurable tab spacing, cursor + document statistics, encoding indicator)
- Find/Replace dialogs, Go To line, time/date insertion, PDF export, and recent-files submenu mirroring Windows Notepad
- Structured logging through spdlog and CTest-driven smoke tests

## Command-Line Options

GnotePad supports the following command-line options:

- `--help`, `-h` – Display help information and exit
- `--version`, `-v` – Display version information and exit
- `--quit-after-init`, `--headless-smoke` – Quit shortly after startup (useful for headless smoke tests and CI environments)

Example usage:
```bash
# Display help
./GnotePad --help

# Display version
./GnotePad --version

# Run headless smoke test
QT_QPA_PLATFORM=offscreen ./GnotePad --quit-after-init
```

## Development Notes

- Sources live in `src/` and are split by responsibility (`app/`, `ui/`).
- Qt resources (icons, translations, etc.) live under `resources/`.
- Logging is provided by spdlog through CMake FetchContent integration.
- Tests now use Qt Test (QTest) via CTest. The `GnotePadSmoke` suite spins up the real `MainWindow` to verify launch/minimize/maximize flows; future additions will cover file I/O, MRU, encoding round-trips, UI automation (find/replace, zoom, insert date), and large-file scrolling.

## Static Analysis & Tooling

- **clang-tidy** – The repo ships a `.clang-tidy` profile plus a CMake toggle. Run `cmake -S . -B build/debug -G Ninja -DCMAKE_CXX_COMPILER=clang++ -DCMAKE_PREFIX_PATH=/usr/lib/x86_64-linux-gnu/cmake/Qt6 -DCMAKE_BUILD_TYPE=Debug -DGNOTE_ENABLE_CLANG_TIDY=ON && cmake --build build/debug` or invoke the VS Code task **Clang-Tidy (Debug)**. You can also call `cmake --build build/debug --target run-clang-tidy` after configuring to re-check just the sources.
  - The profile currently suppresses `readability-implicit-bool-conversion` (pointer guards stay terse) and ignores generated Qt headers plus `qsharedpointer_impl.h` to keep diagnostics focused on project sources.
- **scan-build (clang static analyzer)** – Use `tools/run-scan-build.sh` (defaults to `build/analyze`) or the VS Code task **Scan-Build (Debug)**. Reports land in `scan-build-report/` and can be opened in a browser.
- **clang-format** – Formatting rules live in `.clang-format`. Run `cmake --build build/debug --target run-clang-format` (or the **Clang-Format** VS Code task) to apply styling in-place. Configure editors to honor the same profile for on-save formatting.

- **Editor configuration** – VS Code's C/C++ IntelliSense engine is disabled to avoid conflicting diagnostics; clangd via CMake Tools drives code completion using the generated `compile_commands.json`.
  - Install the [clangd extension](https://marketplace.visualstudio.com/items?itemName=llvm-vs-code-extensions.vscode-clangd) (`llvm-vs-code-extensions.vscode-clangd`) for full IDE features.
  - The extension is automatically recommended when opening the workspace.
  - `.vscode/settings.json` points clangd at the workspace root and `cmake.copyCompileCommands` copies the active build's `compile_commands.json` there on every configure. Run the appropriate configure task for whichever config you are using (e.g., `Build Debug (Windows)` or `Build Debug (Linux)`) so clangd stays in sync; re-run configure after switching build dirs like `build/win-release` vs. `build/debug`.
  - Run formatting when needed: `cmake --build build/debug --target run-clang-format`.
- **Other clang utilities** – clangd powers completions, and clang-query/clang-apply-replacements can be layered on later for AST exploration or batch rewrites if needed.

## Packaging (AppImage)

- Prereqs: `patchelf`, `chrpath`, `desktop-file-validate`, `appstreamcli`, `qmake6`, `zsync`, `sha256sum`; `tools/package-appimage.sh` downloads `linuxdeployqt` automatically with SHA256 checksum verification for security.
- Build artifact: `bash tools/package-appimage.sh` (requires `build/optimized`); outputs `packaging/dist/GnotePad-<version>-x86_64.AppImage` and logs to `packaging/dist/linuxdeployqt.log`.
- Desktop/AppStream IDs: `app.gnotepad.GnotePad.desktop` and `app.gnotepad.GnotePad.appdata.xml` are baked into the AppImage.
- Headless smoke (CI-friendly): `QT_QPA_PLATFORM=offscreen ./packaging/dist/GnotePad-<version>-x86_64.AppImage --quit-after-init`.
- Quick try (AppImage):
  ```bash
  chmod +x GnotePad-0.8.1-x86_64.AppImage && ./GnotePad-0.8.1-x86_64.AppImage
  ```
- Publishing pointers: see `packaging/PUBLISHING.md` for AppImageHub/AppImage notes.

## Packaging (DEB/RPM)

- Prereqs: `dpkg-dev` (usually present on Ubuntu) and `rpm` (for rpmbuild).
- Build: from `build/optimized`, run:
  ```bash
  cpack -G DEB -C Release -B ../../packaging/dist
  cpack -G RPM -C Release -B ../../packaging/dist
  ```
- Outputs land in `packaging/dist/` as `gnotepad-<version>-Linux.deb` and `gnotepad-<version>-Linux.rpm`.

## Packaging (Flatpak)

- Prereqs: `flatpak-builder`, plus runtimes `org.kde.Platform//6.7` and `org.kde.Sdk//6.7`.
- Manifest: `packaging/linux/flatpak/app.gnotepad.GnotePad.yml` (builds from the repo checkout) vendors LLVM 18.1.8 and forces clang/lld with IPO enabled; no host GCC toolchain is used inside the sandbox. A `libtinfo.so.5` symlink is added to satisfy clang binaries in the vendor tree.
- Flathub-ready manifest: `packaging/linux/flatpak/app.gnotepad.GnotePad.flathub.yml` (pinned source archive + sha256 for submissions). Keep the LLVM tarball URL + sha256 in sync when bumping releases.
- Build helper: `tools/flatpak-build.sh --flathub --install` (default) runs the pinned manifest with a clean build-dir. Use `--local` to target the checkout manifest.
- Manual build (from repo root):
  ```bash
  flatpak-builder build-dir packaging/linux/flatpak/app.gnotepad.GnotePad.yml --install --user
  flatpak run app.gnotepad.GnotePad
  ```
  To test the Flathub-pinned manifest manually, swap the manifest path: `flatpak-builder build-dir packaging/linux/flatpak/app.gnotepad.GnotePad.flathub.yml --install --user`.
- Publishing pointers: see `packaging/PUBLISHING.md` for Flathub submission steps.

## Include Guidelines

Keeping headers tidy shrinks rebuild times and keeps clang-tidy's include-cleaner happy. Follow these rules whenever you touch headers or add new modules:

1. **Include what you use.** If a symbol appears in a translation unit, include the header that provides it instead of relying on transitive includes.
2. **Use `#pragma once`.** All project headers already opt in—match that style on any new files.
3. **Honor the include order:**
   - (Only in `.cpp` files) the corresponding header: `#include "ThisFile.h"`
   - C++ standard library headers, alphabetical
   - Third-party/library headers, alphabetical (e.g., `<spdlog/spdlog.h>`, `<QtCore/qstring.h>`)
   - Project headers, alphabetical
   Separate each group with a blank line and avoid redundant includes.
4. **New files inherit the same rules.** Start with the include skeleton above so diffs stay small.

clang-tidy plus `.clangd`'s `-fno-modules` flag help surface violations locally. Running `cmake --build build/debug --target run-clang-tidy` (or the VS Code **Clang-Tidy (Debug)** task) will confirm that headers stay clean.

## Backlog & Next Steps

**Remaining focus areas**

- Broaden Unicode/encoding regression tests with round-trip corpora to validate decoder/encoder behavior.
- Finish Notepad UX parity:
  - Page Setup and Print Preview commands.
  - Ensure the font dialog remembers prior selections (family/size/script) across launches and persists those settings alongside the editor font.
  - Add multi-document handling (tabs or multi-window management that mirrors modern Notepad).
- Ship desktop-ready packages (AppImage, MSIX, dmg) and the associated integration assets.
- Improve large-file responsiveness via async load indicators and targeted profiling.
- Modernize ownership in `MainWindow`/`TextEditor`: move long-lived members (status labels, menus, the editor widget, etc.) away from naked `new` to safer ownership constructs (smart pointers or stack members) while still honoring Qt parent-child lifetimes.
- Replace UI-related magic numbers (window sizes, zoom defaults, pixmap dimensions) with named constants.

**Recently completed**

- Menu actions (Save/Save As/Find/Cut/Copy/etc.) now enable and disable automatically based on document content and selection state to match Windows Notepad behavior.
- Font dialog launch moved out of an inline lambda and centralized in `handleChooseFont()`, opening the door for future persistence improvements.
