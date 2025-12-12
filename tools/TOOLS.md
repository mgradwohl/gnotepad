# GnotePad Tools

This directory contains scripts for code quality checks, static analysis, and packaging. Most tools have both Linux (`.sh`) and Windows (`.ps1`) versions.

## Code Quality & Static Analysis

### check-format

Checks if all source files conform to the project's clang-format rules.

| Platform | Script |
|----------|--------|
| Linux | `check-format.sh` |
| Windows | `check-format.ps1` |

**Usage:**
```bash
# Linux
./tools/check-format.sh

# Windows
.\tools\check-format.ps1
```

**Options:** None

**Exit codes:**
- `0` - All files are properly formatted
- `1` - Formatting violations found

**Notes:**
- Uses `--dry-run -Werror` to detect violations without modifying files
- To fix violations, run: `cmake --build build/debug --target run-clang-format`

---

### check-include-order

Verifies that `#include` statements follow the project's ordering guidelines:
1. Matching header first (`.cpp` files only)
2. Project headers (`src/`, `include/`, `tests/`, etc.), alphabetical
3. Non-Qt third-party libraries (spdlog, fmt, boost, etc.), alphabetical
4. Qt headers, alphabetical
5. C++ standard library headers, alphabetical
6. Everything else (fallback)

| Platform | Script |
|----------|--------|
| Linux | `check-include-order.sh` |
| Windows | `check-include-order.ps1` |

**Usage:**
```bash
# Linux
./tools/check-include-order.sh

# Windows
.\tools\check-include-order.ps1
```

**Options:** None

**Exit codes:**
- `0` - All files follow include order guidelines
- `1` - Include order violations found

---

### run-scan-build

Runs the Clang Static Analyzer (scan-build) on the project to detect potential bugs.

| Platform | Script |
|----------|--------|
| Linux | `run-scan-build.sh` |
| Windows | `run-scan-build.ps1` |

**Usage:**
```bash
# Linux
./tools/run-scan-build.sh [BUILD_DIR] [REPORT_DIR]

# Windows
.\tools\run-scan-build.ps1 [-BuildDir <path>] [-ReportDir <path>]
```

**Options:**
| Option | Default | Description |
|--------|---------|-------------|
| `BUILD_DIR` / `-BuildDir` | `build/analyze` (Linux) / `build/win-analyze` (Windows) | CMake build directory |
| `REPORT_DIR` / `-ReportDir` | `scan-build-report` | HTML report output directory |

**Environment variables (Linux):**
| Variable | Default | Description |
|----------|---------|-------------|
| `QT6_PREFIX` | `/usr/lib/x86_64-linux-gnu/cmake/Qt6` | Qt6 CMake prefix path |
| `CC` | `/usr/lib/llvm-21/bin/clang` | C compiler |
| `CXX` | `/usr/lib/llvm-21/bin/clang++` | C++ compiler |

**Environment variables (Windows):**
Requires Devshell environment variables (`LLVM_ROOT`, `Qt6_DIR`, `QT6_PREFIX_PATH`, `VCPKG_ROOT`).

**Exit codes:**
- `0` - No issues found
- Non-zero - Issues found (see report)

---

## Packaging & Distribution

### publish-win.ps1

**Platform:** Windows only

Stages a clean Windows publish tree with only runtime assets and creates a distributable zip file.

**Usage:**
```powershell
.\tools\publish-win.ps1
```

**Options:** None (configured via environment)

**Environment variables:**
| Variable | Default | Description |
|----------|---------|-------------|
| `GNOTE_VERSION` | Extracted from `CMakeLists.txt` | Version string for the zip filename |

**Prerequisites:**
- Run `Build Optimized (Windows)` task first
- Requires `build/win-optimized/GnotePad.exe` to exist

**Output:**
- `build/win-publish/` - Staged publish tree
- `packaging/dist/GnotePad-<version>-windows.zip` - Distributable archive

**Notes:**
- Runs tests against the staged runtime before creating the zip
- Copies only required DLLs and Qt plugins

---

### package-appimage.sh

**Platform:** Linux only

Creates a portable AppImage for Linux distribution.

**Usage:**
```bash
./tools/package-appimage.sh
```

**Options:** None (configured via environment)

**Environment variables:**
| Variable | Default | Description |
|----------|---------|-------------|
| `VERSION` | Extracted from `CMakeLists.txt` | Version string |
| `RELEASE_DATE` | Current date | Release date in ISO format |
| `QMAKE_BIN` | Auto-detected (`qmake6` or `qmake`) | Path to qmake |
| `LINUXDEPLOY_URL` | GitHub releases URL | Download URL for linuxdeployqt |
| `LINUXDEPLOY_SHA256` | Pinned checksum | SHA256 for integrity verification |

**Prerequisites:**
- Run `Build Optimized (Linux)` task first
- Required tools: `cmake`, `patchelf`, `chrpath`, `desktop-file-validate`, `appstreamcli`, `zsyncmake`, `sha256sum`

**Output:**
- `packaging/dist/GnotePad-<version>-x86_64.AppImage`
- `packaging/dist/GnotePad-<version>-x86_64.AppImage.zsync`

---

### install-local.sh

**Platform:** Linux only

Installs GnotePad locally for the current user with proper desktop integration.

**Usage:**
```bash
./tools/install-local.sh
```

**Options:** None (configured via environment)

**Environment variables:**
| Variable | Default | Description |
|----------|---------|-------------|
| `PREFIX` | `packaging/dist/install` | Installation prefix |

**Prerequisites:**
- Run `Build Optimized (Linux)` task first
- Requires `build/optimized/GnotePad` to exist

**Output:**
- Binary installed to `$PREFIX/bin/GnotePad`
- Desktop file installed to `~/.local/share/applications/`
- Icon installed to `~/.local/share/icons/hicolor/`

---

### flatpak-build.sh

**Platform:** Linux only

Builds and optionally installs a Flatpak package.

**Usage:**
```bash
./tools/flatpak-build.sh [OPTIONS]
```

**Options:**
| Option | Description |
|--------|-------------|
| `--flathub` | Use the Flathub-pinned manifest (default) |
| `--local` | Use the checkout-sourced manifest |
| `--no-install` | Build only, skip installing to user |
| `-h`, `--help` | Show help message |

**Prerequisites:**
- `flatpak-builder` installed
- Flatpak SDK and runtime installed

**Output:**
- `build-dir/` - Flatpak build directory
- Optionally installs to user's Flatpak installation

---

## Summary

| Tool | Linux | Windows | Purpose |
|------|-------|---------|---------|
| check-format | ✅ | ✅ | Verify clang-format compliance |
| check-include-order | ✅ | ✅ | Verify include ordering |
| run-scan-build | ✅ | ✅ | Static analysis |
| publish-win | ❌ | ✅ | Package Windows release |
| package-appimage | ✅ | ❌ | Create Linux AppImage |
| install-local | ✅ | ❌ | Local Linux installation |
| flatpak-build | ✅ | ❌ | Build Flatpak package |
