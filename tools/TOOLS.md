# GnotePad Tools

This directory contains scripts for building, code quality checks, static analysis, and packaging. Most tools have both Linux (`.sh`) and Windows (`.ps1`) versions.

## Build Scripts

### configure

Configures the CMake build for a specified build type.

| Platform | Script |
|----------|--------|
| Linux | `configure.sh` |
| Windows | `configure.ps1` |

**Usage:**
```bash
# Linux
./tools/configure.sh [OPTIONS] [BUILD_TYPE]

# Windows
.\tools\configure.ps1 [-Verbose] [BUILD_TYPE]
```

**Build Types:**
| Type | Description |
|------|-------------|
| `debug` | Debug build (default) |
| `relwithdebinfo` | Release with debug info |
| `release` | Release build |
| `optimized` | Optimized build (LTO, march=x86-64-v3, stripped) |
| `analyze` | Build for scan-build analysis |

**Options:**
| Option | Description |
|--------|-------------|
| `-v`, `--verbose` | Show verbose output |
| `-h`, `--help` | Show help |

**Environment variables (Windows):**
Requires: `LLVM_ROOT`, `VCPKG_ROOT`, optionally `Qt6_DIR`, `QT6_PREFIX_PATH`

---

### build

Builds the project (run configure first).

| Platform | Script |
|----------|--------|
| Linux | `build.sh` |
| Windows | `build.ps1` |

**Usage:**
```bash
# Linux
./tools/build.sh [OPTIONS] [BUILD_TYPE]

# Windows
.\tools\build.ps1 [-Target <name>] [-Verbose] [BUILD_TYPE]
```

**Options:**
| Option | Description |
|--------|-------------|
| `-v`, `--verbose` | Show verbose build output |
| `-t`, `--target NAME` | Build specific target(s) |
| `-h`, `--help` | Show help |

**Examples:**
```bash
./tools/build.sh                    # Build debug
./tools/build.sh release            # Build release
./tools/build.sh -v optimized       # Verbose optimized build
./tools/build.sh -t run-clang-tidy  # Run clang-tidy target
```

---

## Code Quality & Static Analysis

### clang-tidy

Runs clang-tidy static analysis on source files.

| Platform | Script |
|----------|--------|
| Linux | `clang-tidy.sh` |
| Windows | `clang-tidy.ps1` |

**Usage:**
```bash
# Linux
./tools/clang-tidy.sh [OPTIONS] [BUILD_TYPE]

# Windows
.\tools\clang-tidy.ps1 [-Verbose] [BUILD_TYPE]
```

**Notes:**
- Configures automatically if build directory doesn't exist
- Build type can be `debug` or `relwithdebinfo`

---

### clang-format

Applies clang-format to all source files (in-place).

| Platform | Script |
|----------|--------|
| Linux | `clang-format.sh` |
| Windows | `clang-format.ps1` |

**Usage:**
```bash
# Linux
./tools/clang-format.sh

# Windows
.\tools\clang-format.ps1
```

**Notes:**
- Modifies files in-place
- Use `check-format.sh` / `check-format.ps1` to check without modifying

---

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

**Exit codes:**
- `0` - All files are properly formatted
- `1` - Formatting violations found

**Notes:**
- Uses `--dry-run -Werror` to detect violations without modifying files
- To fix violations, run `clang-format.sh` / `clang-format.ps1`

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
./tools/run-scan-build.sh [OPTIONS]

# Windows
.\tools\run-scan-build.ps1 [-ReportDir <path>] [-Verbose]
```

**Options:**
| Option | Default | Description |
|--------|---------|-------------|
| `-o`, `--output` | `scan-build-report` | HTML report output directory |
| `-v`, `--verbose` | | Show verbose output |

**Notes:**
- Automatically configures an `analyze` build
- Report is generated in HTML format

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
| configure | ✅ | ✅ | Configure CMake build |
| build | ✅ | ✅ | Build the project |
| clang-tidy | ✅ | ✅ | Run clang-tidy analysis |
| clang-format | ✅ | ✅ | Apply clang-format |
| check-format | ✅ | ✅ | Verify clang-format compliance |
| check-include-order | ✅ | ✅ | Verify include ordering |
| run-scan-build | ✅ | ✅ | Static analysis |
| publish-win | ❌ | ✅ | Package Windows release |
| package-appimage | ✅ | ❌ | Create Linux AppImage |
| install-local | ✅ | ❌ | Local Linux installation |
| flatpak-build | ✅ | ❌ | Build Flatpak package |
