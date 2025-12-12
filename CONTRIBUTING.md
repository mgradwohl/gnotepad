# Contributing to GnotePad

Thanks for helping improve GnotePad! This document provides comprehensive guidance for developers working on the project.

## Development Environment Setup

### Prerequisites

- **CMake** 3.26+ (we are using 4.2)
- **Build System:** Ninja or Make (Ninja recommended)
- **Compiler:** Clang 21+ with lld linker (or Apple Clang 15+ on macOS)
- **Qt:** 6.5+ development packages (Widgets, Gui, Core, PrintSupport, Svg, SvgWidgets, Test)
- **Logging:** spdlog (automatically fetched via CMake FetchContent)
- **Static Analysis:** clang-tidy 21+, clang-format 21+

### Linux/Ubuntu Setup

Install dependencies:

```bash
sudo apt install clang-21 lld-21 clang-tidy-21 clang-format-21 ninja-build qt6-base-dev qt6-base-dev-tools qt6-tools-dev cmake git
```

Note: clang-21 packages are available from the LLVM apt repository. See https://apt.llvm.org/ for setup instructions.

### Windows Setup

Windows development uses PowerShell + vcpkg with clang++/lld and Qt6:

1. **Install LLVM:**
   ```powershell
   winget install LLVM.LLVM
   ```

2. **Install CMake and Ninja:**
   ```powershell
   winget install Kitware.CMake Ninja-build.Ninja
   ```

3. **Install Qt via vcpkg:**
   ```powershell
   vcpkg install qtbase:x64-windows qtsvg:x64-windows
   ```

4. **Ensure `C:/Program Files/LLVM/bin` is on PATH** (contains `clang++.exe` and `lld-link.exe`)

5. **Configure environment:** vcpkg layouts plugins under `.../installed/x64-windows/Qt6/plugins` (debug plugins under `.../debug/Qt6/plugins`). CMake stages the Qt runtime (bin + plugins) plus a `qt.conf` into each `build/win-*` output, so you can launch `GnotePad.exe` directly without setting plugin environment variables.

6. **If CMake cannot find Qt6,** point it at the vcpkg install:
   ```powershell
   $env:Qt6_DIR="$env:VCPKG_ROOT/installed/x64-windows/share/Qt6"
   $env:QT6_PREFIX_PATH="$env:VCPKG_ROOT/installed/x64-windows"
   ```

#### Windows Toolchain Management

- **Keep LLVM and CMake current:**
  ```powershell
  winget upgrade LLVM.LLVM
  winget upgrade Kitware.CMake
  ```

- **Keep vcpkg current:**
  ```powershell
  # Initial setup
  git clone https://github.com/microsoft/vcpkg $env:USERPROFILE\source\vcpkg
  & $env:USERPROFILE\source\vcpkg\bootstrap-vcpkg.bat -disableMetrics
  
  # Update periodically
  cd $env:USERPROFILE\source\vcpkg
  git pull
  ```

- **PATH order:** Prefer fresh tools ahead of older Visual Studio toolchains: `${env:LLVM_ROOT}\bin`, `${env:CMAKE_ROOT}\bin`, then `${env:VCPKG_ROOT}`

- **Development shell:** `Devshell-Updated.ps1` (in repo root) prepends LLVM, CMake, vcpkg, and Qt6 to PATH and exports necessary environment variables:
  ```powershell
  pwsh -NoProfile -ExecutionPolicy Bypass -File .\Devshell-Updated.ps1
  ```
  
  For Windows Terminal integration, add this as a profile command:
  ```powershell
  pwsh.exe -NoLogo -NoExit -ExecutionPolicy Bypass -File "D:\source\gnotepad\Devshell-Updated.ps1"
  ```
  (Replace path with your actual repository location)

### macOS Setup

Install dependencies via Homebrew:

```bash
brew install cmake ninja llvm qt@6
```

## Building the Project

### Configure and Build (Linux/macOS)

```bash
cmake -S . -B build/debug -G Ninja \
  -DCMAKE_CXX_COMPILER=/usr/lib/llvm-21/bin/clang++ \
  -DCMAKE_CXX_STANDARD=23 \
  -DCMAKE_CXX_STANDARD_REQUIRED=ON \
  -DCMAKE_CXX_EXTENSIONS=OFF \
  -DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
  -DCMAKE_PREFIX_PATH=/usr/lib/x86_64-linux-gnu/cmake/Qt6 \
  -DCMAKE_BUILD_TYPE=Debug \
  -DCMAKE_EXE_LINKER_FLAGS=-fuse-ld=lld \
  -DCMAKE_SHARED_LINKER_FLAGS=-fuse-ld=lld
cmake --build build/debug
ctest --test-dir build/debug
```

Adjust `-DCMAKE_PREFIX_PATH` to match your Qt6 installation (e.g., `/opt/Qt/6.xx/gcc_64/lib/cmake/Qt6`). Swap `Debug` for `Release` or `RelWithDebInfo` as needed.

### Configure and Build (Windows)

```powershell
cmake -S . -B build/win-debug -G Ninja `
  -DCMAKE_CXX_COMPILER="$env:LLVM_ROOT/clang++.exe" `
  -DCMAKE_CXX_STANDARD=23 -DCMAKE_CXX_STANDARD_REQUIRED=ON -DCMAKE_CXX_EXTENSIONS=OFF `
  -DCMAKE_TOOLCHAIN_FILE="$env:VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake" `
  -DQt6_DIR="$env:VCPKG_ROOT/installed/x64-windows/share/Qt6" `
  -DCMAKE_PREFIX_PATH="$env:VCPKG_ROOT/installed/x64-windows" `
  -DCMAKE_BUILD_TYPE=Debug `
  -DCMAKE_EXE_LINKER_FLAGS="-fuse-ld=lld" -DCMAKE_SHARED_LINKER_FLAGS="-fuse-ld=lld"
cmake --build build/win-debug
```

For release-like builds, swap `build/win-debug`/`Debug` for `build/win-relwithdebinfo` + `RelWithDebInfo`, `build/win-release` + `Release`, or `build/win-optimized` + `Release` with IPO enabled. VS Code tasks (`Build Debug (Windows)`, `Build RelWithDebInfo (Windows)`, etc.) already use these flags and the vcpkg toolchain.

### Running the Application

After building, launch the executable:
- **Linux/macOS:** `build/<config>/GnotePad`
- **Windows:** `build/<config>/GnotePad.exe`

**Windows runtime note:** CMake post-build steps copy the appropriate Qt bin and plugin trees plus a `qt.conf` into each `build/win-*` folder. You can run `GnotePad.exe` from those folders without setting `QT_PLUGIN_PATH` or `QT_QPA_PLATFORM_PLUGIN_PATH`.

### Command-Line Options

- `--help`, `-h` – Display help information and exit
- `--version`, `-v` – Display version information and exit
- `--quit-after-init`, `--headless-smoke` – Quit shortly after startup (useful for headless smoke tests and CI)

Example usage:
```bash
# Display help
./GnotePad --help

# Run headless smoke test
QT_QPA_PLATFORM=offscreen ./GnotePad --quit-after-init
```

## Project Structure

```
src/
├── gnotepad.cpp          # Application entry point
├── app/
│   ├── Application.h/cpp # Custom QApplication subclass
└── ui/
    ├── MainWindow.h/cpp  # Main window with menus, toolbars
    └── TextEditor.h/cpp  # Editor widget with line numbers, zoom

resources/
├── gnotepad.qrc          # Qt resource file
└── icons/                # Application icons

tests/
├── smoke/                # Smoke tests using Qt Test framework
├── cmdline/              # Command-line parsing tests
├── menuactions/          # Menu action state and behavior tests
├── encoding/             # Encoding edge case tests
├── style/                # Qt style configuration tests
├── tooling/              # Clang tooling configuration tests
└── testfiles/            # Test data files with various encodings

packaging/
├── linux/                # Linux packaging files (AppImage, Flatpak, DEB, RPM)
└── dist/                 # Build output directory
```

Sources live in `src/` and are split by responsibility (`app/`, `ui/`). Qt resources (icons, translations, etc.) live under `resources/`. Logging is provided by spdlog through CMake FetchContent integration.

## Workflow Basics

1. **Build before you push.** Configure with CMake (Debug or Release) and ensure `ctest` passes.
2. **Run the tooling targets.** `cmake --build build/debug --target run-clang-format` keeps style consistent; `cmake --build build/debug --target run-clang-tidy` (or the VS Code tasks) should finish cleanly before opening a PR.
3. **Keep commits focused.** Group related changes together and describe the intent plus any notable testing in the commit message or PR description.

## Static Analysis & Tooling

### clang-tidy

The repo ships a `.clang-tidy` profile plus a CMake toggle. Run:

```bash
cmake -S . -B build/debug -G Ninja \
  -DCMAKE_CXX_COMPILER=clang++ \
  -DCMAKE_PREFIX_PATH=/usr/lib/x86_64-linux-gnu/cmake/Qt6 \
  -DCMAKE_BUILD_TYPE=Debug \
  -DGNOTE_ENABLE_CLANG_TIDY=ON
cmake --build build/debug
```

Or use the VS Code task **Clang-Tidy (Debug)**. You can also call `cmake --build build/debug --target run-clang-tidy` after configuring to re-check just the sources.

The profile currently suppresses `readability-implicit-bool-conversion` (pointer guards stay terse) and ignores generated Qt headers plus `qsharedpointer_impl.h` to keep diagnostics focused on project sources.

### scan-build (clang static analyzer)

Use `tools/run-scan-build.sh` (defaults to `build/analyze`) or the VS Code task **Scan-Build (Debug)**. Reports land in `scan-build-report/` and can be opened in a browser.

### clang-format

Formatting rules live in `.clang-format` (LLVM base, Allman braces). Run:

```bash
cmake --build build/debug --target run-clang-format
```

Or use the **Clang-Format** VS Code task to apply styling in-place. Configure editors to honor the same profile for on-save formatting.

### Editor Configuration (VS Code)

VS Code's C/C++ IntelliSense engine is disabled to avoid conflicting diagnostics; clangd via CMake Tools drives code completion using the generated `compile_commands.json`.

1. **Install the clangd extension:** [clangd](https://marketplace.visualstudio.com/items?itemName=llvm-vs-code-extensions.vscode-clangd) (`llvm-vs-code-extensions.vscode-clangd`) for full IDE features. The extension is automatically recommended when opening the workspace.

2. **Configuration:** `.vscode/settings.json` points clangd at the workspace root and `cmake.copyCompileCommands` copies the active build's `compile_commands.json` there on every configure.

3. **Keep clangd in sync:** Run the appropriate configure task for whichever config you are using (e.g., `Build Debug (Windows)` or `Build Debug (Linux)`); re-run configure after switching build dirs like `build/win-release` vs. `build/debug`.

### VS Code Tasks

The project includes predefined tasks in `.vscode/tasks.json`:
- **Build Debug (Linux)** - Build with debug symbols (default)
- **Build RelWithDebInfo (Linux)** - Build with optimizations + debug info
- **Build Release (Linux)** - Build optimized release version
- **Build Optimized (Linux)** - Build with LTO, march=x86-64-v3, stripped
- **Clang-Tidy (Debug, Linux)** - Run static analysis
- **Scan-Build (Debug, Linux)** - Run clang static analyzer
- **Clang-Format (Linux)** - Format all source files

### VS Code Launch Configurations

The project includes predefined launch configs in `.vscode/launch.json`:
- **Debug GnotePad (Debug, Linux)** - Debug build with full symbols
- **Debug GnotePad (RelWithDebInfo, Linux)** - Optimized with debug info
- **Run GnotePad (Release/Optimized, Linux)** - Production builds

## Testing

Tests use Qt Test (QTest) via CTest. The `GnotePadSmoke` suite spins up the real `MainWindow` to verify launch/minimize/maximize flows, file I/O, encoding round-trips, and UI automation (find/replace, zoom, insert date).

### Running Tests

```bash
# Build tests
cmake --build build/debug

# Run all tests
ctest --test-dir build/debug

# Run specific tests
ctest --test-dir build/debug -R "encoding|smoke"

# Run with verbose output
ctest --test-dir build/debug -V
```

### Adding Tests

Unit and smoke tests live under `tests/`. Extend the suite when you add new user-visible functionality or regressions:

1. Add test file to appropriate subdirectory
2. Add test method to test class header
3. Implement test in test source file
4. Update `tests/CMakeLists.txt` if new test functions are added
5. Ensure tests pass locally before submitting PR

See [tests/README.md](tests/README.md) for detailed testing documentation.

## Include Guidelines

Keeping headers tidy shrinks rebuild times and keeps clang-tidy's include-cleaner happy. Follow these rules whenever you touch headers or add new modules:

1. **Include what you use.** If a symbol appears in a translation unit, include the header that provides it instead of relying on transitive includes.
2. **Use `#pragma once`.** All project headers already opt in—match that style on any new files.
3. **Honor the include order:**
   1. This file's matching header first (`.cpp` files only): `#include "ThisFile.h"`
   2. Project headers (`src/`, `include/`, `tests/`, etc.), alphabetical
   3. Non-Qt third-party libraries (spdlog, fmt, boost, etc.), alphabetical
   4. Qt headers, alphabetical
   5. C++ standard library headers, alphabetical
   6. Everything else (fallback)

   Separate each group with a blank line and avoid redundant includes.

4. **Use modern preprocessor directives (C++23):**
   - Use `#ifdef X` instead of `#if defined(X)` for simple checks
   - Use `#ifndef X` instead of `#if !defined(X)`
   - Use `#elifdef X` instead of `#elif defined(X)`
   - Compound conditions like `#if defined(X) && !defined(Y)` stay as-is
   
5. **New files inherit the same rules.** Start with the include skeleton above so diffs stay small.


clang-tidy plus `.clangd`'s `-fno-modules` flag help surface violations locally. Running `cmake --build build/debug --target run-clang-tidy` (or the VS Code **Clang-Tidy (Debug)** task) will confirm that headers stay clean.

## Code Style and Formatting

### Language Standards
- **C++ Standard:** C++23 (required)
- **Compiler:** Primarily validated with clang
- **Formatting:** Use `.clang-format` (LLVM base, Allman braces) - run `cmake --build build/debug --target run-clang-format` before commits

### Naming Conventions
- **Classes/Types:** PascalCase (e.g., `MainWindow`, `TextEditor`)
- **Functions/Methods:** camelCase (e.g., `handleSaveFile`, `updateStatusBar`)
- **Member Variables:** camelCase with `m_` prefix for private members (e.g., `m_editor`)
- **Constants:** UPPER_SNAKE_CASE (e.g., `DEFAULT_ZOOM_LEVEL`)

### Qt-Specific Guidelines
- Use Qt's parent-child ownership for widgets when appropriate
- Prefer smart pointers for non-Qt objects
- Use Qt's signal/slot mechanism for event handling
- Follow Qt naming conventions for slots (e.g., `onActionTriggered()`)

## Local Install for Manual Testing

Build the optimized config first, then install locally:

```bash
cmake --build build/optimized
bash tools/install-local.sh
```

Optionally set `PREFIX=/custom/prefix`. This installs the optimized binary, rewrites the desktop `Exec` to the installed path, and refreshes desktop/icon caches so the launcher entry appears under `app.gnotepad.GnotePad`.

## Packaging

For information on creating release packages (AppImage, DEB, RPM, Flatpak), see [packaging/README.md](packaging/README.md).

## Best Practices

- **Minimal changes:** Make the smallest possible changes to achieve the goal
- **DRY principle:** Don't repeat yourself; extract common code
- **RAII:** Use smart pointers and Qt parent-child ownership
- **Const correctness:** Mark methods and variables `const` when appropriate
- **Error handling:** Check return values, use exceptions sparingly
- **Documentation:** Use clear, descriptive names that minimize need for comments
- **Performance:** Avoid unnecessary copies (use const references, move semantics)

## Security Considerations

- Validate all file paths and user input
- Handle untrusted text files safely (encoding issues, large files)
- Avoid buffer overflows in encoding conversion
- Use Qt's secure file handling mechanisms

## Common Pitfalls to Avoid

- Don't use `using namespace std` or `using namespace Qt` in headers
- Don't mix Qt ownership with smart pointers inappropriately
- Don't ignore clang-tidy warnings
- Don't skip running tests before committing
- Don't add unnecessary dependencies
- Don't break existing functionality when adding features
- Don't commit without running clang-format

GnotePad has comprehensive test coverage across multiple test suites. All functional changes should include relevant tests.

- **Test suites** live under `tests/` and include:
  - `cmdline/` - Command-line argument parsing tests
  - `smoke/` - High-level integration tests for core functionality
  - `menuactions/` - Menu action behavior and state tests
  - `encoding/` - Encoding edge case and conversion tests
  - `style/` - Qt style configuration tests

- **Running tests:** Use `ctest --test-dir build/debug` to run all tests, or run specific test suites with `ctest -R <suite_name>`

- **clang-format:** All C++ sources follow the repository `.clang-format` (LLVM base, Allman braces). Run the `run-clang-format` target or the VS Code task before submitting.
- **clang-tidy / scan-build:** clang-tidy runs on the Debug configuration; scan-build hooks live in `tools/run-scan-build.sh` and the **Scan-Build (Debug)** task. Fix or explain any new warnings.
- **Automated checks:** Before committing, run `./tools/check-format.sh` and `./tools/check-include-order.sh` to verify formatting and include order.
- For detailed information on static analysis tooling and CI checks, see [`docs/STATIC_ANALYSIS.md`](docs/STATIC_ANALYSIS.md).

- **Testing guidelines:** See `TESTING.md` for comprehensive guidelines, examples, and best practices

- GUI smoke tests (`GnotePadSmoke`) run via `ctest --test-dir build/debug`. Make sure they pass locally on the configurations you touch.
## Questions?

Open a GitHub discussion or tag `@mgradwohl` in your PR.
