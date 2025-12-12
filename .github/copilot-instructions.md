# GnotePad - Copilot Instructions

This file provides guidance for GitHub Copilot coding agents working on the GnotePad project.

## Project Overview

GnotePad is a cross-platform Qt 6 clone of Windows 10 Notepad built with modern C++23. The project emphasizes:
- Feature parity with Windows Notepad (UTF-8/16/32, BOM detection, encoding picker, find/replace, zoom, etc.)
- Cross-platform support (Linux, Windows, macOS)
- Modern C++23 with clang as the primary toolchain
- Qt 6.5+ for UI and core functionality
- Clean architecture with proper separation of concerns

## Development Environment

### Required Tools
- **Compiler:** Clang 21+ with lld linker (or Apple Clang 15+ on macOS)
- **Build System:** CMake 3.26+ with Ninja (recommended)
- **Qt:** Qt 6.5+ (Core, Gui, Widgets, PrintSupport, Svg, SvgWidgets, Test)
- **Logging:** spdlog (via CMake FetchContent)
- **Static Analysis:** clang-tidy 21+, clang-format 21+

### Build Commands

Configure and build (Linux):
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
```

Configure and build (Windows):
```powershell
cmake -S . -B build/win-debug -G Ninja `
  -DCMAKE_CXX_COMPILER="$env:LLVM_ROOT/clang++.exe" `
  -DCMAKE_TOOLCHAIN_FILE="$env:VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake" `
  -DQt6_DIR="$env:VCPKG_ROOT/installed/x64-windows/share/Qt6" `
  -DCMAKE_BUILD_TYPE=Debug
cmake --build build/win-debug
```

Run tests:
```bash
ctest --test-dir build/debug
```

### Command-Line Options

GnotePad supports these command-line options:
- `--help`, `-h` - Display help information and exit
- `--version`, `-v` - Display version information and exit
- `--quit-after-init`, `--headless-smoke` - Quit shortly after startup (useful for headless smoke tests and CI)

Headless mode example:
```bash
QT_QPA_PLATFORM=offscreen ./GnotePad --quit-after-init
```

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

## Coding Standards

### Language and Style
- **C++ Standard:** C++23 (required)
- **Compiler:** Primarily validated with clang
- **Formatting:** Use `.clang-format` (LLVM base, Allman braces)
  - Run `cmake --build build/debug --target run-clang-format` before commits
- **Static Analysis:** Use `.clang-tidy` configuration
  - Run `cmake --build build/debug --target run-clang-tidy` regularly

### Include Order
   1. This file's matching header first (`.cpp` files only): `#include "ThisFile.h"`
   2. Project headers (`src/`, `include/`, `tests/`, etc.), alphabetical
   3. Non-Qt third-party libraries (spdlog, fmt, boost, etc.), alphabetical
   4. Qt headers, alphabetical
   5. C++ standard library headers, alphabetical
   6. Everything else (fallback)

Note that `#include <qnamespace.h>` is considered a Qt header.

Separate each group with a blank line. Use `#pragma once` in all headers. Include what you use.

### Preprocessor Directives
- Use `#ifdef X` instead of `#if defined(X)` for simple checks
- Use `#ifndef X` instead of `#if !defined(X)`
- Use `#elifdef X` instead of `#elif defined(X)` (C++23)
- Compound conditions like `#if defined(X) && !defined(Y)` stay as-is

**Example:**
```cpp
#include "MainWindow.h"

#include "app/Application.h"
#include "ui/TextEditor.h"

#include <spdlog/spdlog.h>

#include <QtWidgets/qfiledialog.h>
#include <QtWidgets/qmessagebox.h>

#include <memory>
#include <string>
```

### Naming Conventions
- **Classes/Types:** PascalCase (e.g., `MainWindow`, `TextEditor`)
- **Functions/Methods:** camelCase (e.g., `handleSaveFile`, `updateStatusBar`)
- **Member Variables:** camelCase with `m_` prefix for private members (e.g., `m_editor`)
- **Constants:** UPPER_SNAKE_CASE (e.g., `DEFAULT_ZOOM_LEVEL`)

### Qt Specific
- Use Qt's parent-child ownership for widgets when appropriate
- Prefer smart pointers for non-Qt objects
- Use Qt's signal/slot mechanism for event handling
- Follow Qt naming conventions for slots (e.g., `onActionTriggered()`)

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
├── releases/             # Release notes by version
└── dist/                 # Build output directory
```

**Key directories:**
- `src/` - Application source code split by responsibility (app/, ui/)
- `resources/` - Qt resources (icons, translations)
- `tests/` - Qt Test framework tests and test data
- `packaging/` - Packaging scripts and configurations

## Testing

### Test Framework
- Use **Qt Test (QTest)** for all tests
- Tests live in `tests/` directory
- Run via `ctest --test-dir build/debug`

### Test Coverage
When adding new features, include:
- **Smoke tests:** Verify basic functionality (launch, minimize, maximize)
- **Functionality tests:** Test specific features (file I/O, encoding, find/replace)
- **Edge cases:** Large files, special characters, encoding round-trips

### Example Test Structure
```cpp
#include <QtTest/QtTest>
#include "ui/MainWindow.h"

class TestFeature : public QObject {
    Q_OBJECT
private slots:
    void initTestCase() { /* setup */ }
    void testSomething() { /* test */ }
    void cleanupTestCase() { /* cleanup */ }
};
```

## Common Tasks

### Adding a New Feature
1. Create issue with clear acceptance criteria
2. Add/modify source files in `src/`
3. Update corresponding headers with `#pragma once`
4. Follow include order guidelines
5. Add tests in `tests/`
6. Run clang-format: `cmake --build build/debug --target run-clang-format`
7. Run clang-tidy: `cmake --build build/debug --target run-clang-tidy`
8. Build and test: `cmake --build build/debug && ctest --test-dir build/debug`

### Fixing a Bug
1. Write a failing test that reproduces the bug
2. Fix the issue with minimal changes
3. Verify the test passes
4. Run static analysis tools
5. Ensure no regressions in other tests

### Adding Dependencies
- Prefer Qt built-in functionality when available
- Use CMake `FetchContent` for header-only libraries (see spdlog example)
- Update `CMakeLists.txt` and document in README.md

## Best Practices

### Code Quality
- **Minimal changes:** Make the smallest possible changes to achieve the goal
- **DRY principle:** Don't repeat yourself; extract common code
- **RAII:** Use smart pointers and Qt parent-child ownership
- **Const correctness:** Mark methods and variables `const` when appropriate
- **Error handling:** Check return values, use exceptions sparingly

### Performance
- Avoid unnecessary copies (use const references, move semantics)
- Profile before optimizing
- Consider large file performance for text editor operations

### Documentation
- Use clear, descriptive names that minimize need for comments
- Add comments only when code intent is not obvious
- Keep README.md and CONTRIBUTING.md up to date
- Document public APIs in headers

### Git Workflow
- Write clear commit messages
- Keep commits focused and atomic
- Reference issues in commit messages
- Ensure code builds and tests pass before committing

## Security Considerations

- Validate all file paths and user input
- Handle untrusted text files safely (encoding issues, large files)
- Avoid buffer overflows in encoding conversion
- Use Qt's secure file handling mechanisms

## Platform-Specific Notes

### Linux
- Primary development platform
- Uses system Qt6 packages
- AppImage, DEB, RPM, and Flatpak packaging available

### Windows
- Use clang-cl (LLVM on Windows)
- Qt installation via vcpkg or Qt Maintenance Tool
- Ensure `clang-cl.exe` is on PATH

### macOS
- Use Apple Clang 15+
- Qt installation via Homebrew or Qt Maintenance Tool

## Resources

- **README.md:** Project overview, installation, and quick start
- **CONTRIBUTING.md:** Development environment setup, build instructions, workflow, and contribution guidelines
- **TESTING.md:** Comprehensive testing guide and best practices
- **tests/README.md:** Test file documentation and test coverage details
- **packaging/README.md:** Packaging and release guide for all platforms
- **docs/STATIC_ANALYSIS.md:** Static analysis tooling and CI checks
- **CMakeLists.txt:** Build configuration and options
- **.clang-tidy:** Static analysis rules
- **.clang-format:** Code formatting rules

## Common Pitfalls to Avoid

- Don't use `using namespace std` or `using namespace Qt` in headers
- Don't mix Qt ownership with smart pointers inappropriately
- Don't ignore clang-tidy warnings
- Don't skip running tests before committing
- Don't add unnecessary dependencies
- Don't break existing functionality when adding features
- Don't commit without running clang-format

## When in Doubt

1. Check existing code for patterns and conventions
2. Run static analysis tools
3. Ask for clarification in the issue or PR
4. Refer to Qt documentation for Qt-specific questions
5. Follow the principle of least surprise
