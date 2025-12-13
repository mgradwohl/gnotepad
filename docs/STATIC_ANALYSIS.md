# Static Analysis and Tooling

This document describes the static analysis tools, configuration, and automated checks used in the GnotePad project.

## Overview

GnotePad enforces code quality through:
- **clang-format** for consistent code formatting
- **clang-tidy** for static analysis and best practices
- **Automated tests** for tooling configuration validation
- **CI/CD integration** via GitHub Actions

## Configuration Files

### `.clang-format`

Defines code formatting rules based on LLVM style with Allman braces:
- 140 character line limit
- 4-space indentation
- Allman brace style (braces on new lines)
- Automatic include sorting and grouping

**Include Order:**

See [CONTRIBUTING.md Include Guidelines](../CONTRIBUTING.md#include-guidelines) for the authoritative include order rules.

Each group should be separated by a blank line.

### `.clang-tidy`

Configures static analysis checks:
- Enabled check categories: `bugprone-*`, `clang-analyzer-*`, `cppcoreguidelines-*`, `modernize-*`, `performance-*`, etc.
- Header filter: Only analyzes project code under `src/` and `tests/`
- Include-cleaner exclusions: Qt6, spdlog, and vcpkg headers are excluded

## Local Development

### Running clang-format

Format all source files:
```bash
cmake --build build/debug --target run-clang-format
```

Check formatting without modifying files:
```bash
./tools/check-format.sh
```

### Running clang-tidy

Run static analysis on project sources:
```bash
cmake --build build/debug --target run-clang-tidy
```

Enable clang-tidy during builds:
```bash
cmake -S . -B build/debug -G Ninja \
  -DCMAKE_CXX_COMPILER=clang++ \
  -DGNOTE_ENABLE_CLANG_TIDY=ON
cmake --build build/debug
```

### Checking Include Order

Verify include order follows project guidelines:
```bash
./tools/check-include-order.sh
```

Or run the automated test:
```bash
ctest --test-dir build/debug -R include_order
```

## Automated Tests

### Tooling Configuration Tests

Located in `tests/tooling/ToolingConfigTests.{h,cpp}`:
- `testClangTidyConfigExists` - Verifies `.clang-tidy` file exists
- `testClangTidyConfigIsValid` - Validates `.clang-tidy` content
- `testClangFormatConfigExists` - Verifies `.clang-format` file exists
- `testClangFormatConfigIsValid` - Validates `.clang-format` content
- `testIncludeCleanerExclusionsPresent` - Ensures external libraries are excluded

Run these tests:
```bash
ctest --test-dir build/debug -R tooling
```

### Include Order Tests

Located in `tests/tooling/IncludeOrderTests.{h,cpp}`:
- `testSampleSourceFileIncludeOrder` - Validates Application.cpp
- `testMainWindowIncludeOrder` - Validates MainWindow.cpp
- `testTextEditorIncludeOrder` - Validates TextEditor.cpp

Run these tests:
```bash
ctest --test-dir build/debug -R include_order
```

## Continuous Integration (CI)

### GitHub Actions Workflow

The `.github/workflows/ci.yml` workflow runs on all pushes and pull requests to `main`:

#### Jobs

**1. format-check**
- Installs clang-format-15
- Checks all `.cpp` and `.h` files for formatting violations
- Fails if any files need formatting

**2. static-analysis**
- Installs clang-tidy-15 and project dependencies
- Configures CMake with compile commands
- Runs clang-tidy on all project sources
- Fails if violations are found

**3. build-and-test**
- Matrix: Linux (Ubuntu latest) × Build type (Debug, Release)
- Builds the project
- Runs all tests via `ctest`
- Fails if build or tests fail

### Local CI Simulation

Before pushing, simulate CI checks locally:

```bash
# Format check
./tools/check-format.sh

# Static analysis
cmake -S . -B build/ci -G Ninja \
  -DCMAKE_CXX_COMPILER=clang++ \
  -DCMAKE_BUILD_TYPE=Debug
cmake --build build/ci --target run-clang-tidy

# Build and test
cmake --build build/ci
ctest --test-dir build/ci --output-on-failure
```

## Best Practices

### Before Committing

1. **Format your code:**
   ```bash
   cmake --build build/debug --target run-clang-format
   ```

2. **Run static analysis:**
   ```bash
   cmake --build build/debug --target run-clang-tidy
   ```

3. **Verify include order:**
   ```bash
   ./tools/check-include-order.sh
   ```

4. **Run all tests:**
   ```bash
   ctest --test-dir build/debug
   ```

### Handling Violations

**Format violations:**
- Run `cmake --build build/debug --target run-clang-format` to auto-fix
- Or manually adjust code to match `.clang-format` rules

**clang-tidy warnings:**
- Address genuine issues (logic errors, performance problems, undefined behavior)
- Use `// NOLINT` comments sparingly for false positives (with explanation)
- Consider if the code can be refactored to avoid the warning

**Include order violations:**
- Reorder includes to match project guidelines (see `.clang-format` IncludeCategories)
- Run clang-format which will automatically sort includes
- Ensure proper blank lines between include groups

## VS Code Integration

The project includes predefined tasks in `.vscode/tasks.json`:

- **Clang-Format (Linux)** - Format all sources
- **Clang-Tidy (Debug, Linux)** - Run static analysis during build

Use `Ctrl+Shift+B` to access build tasks in VS Code.

## Troubleshooting

### clang-format not found

Install clang-format (version 15+ recommended):
```bash
# Ubuntu/Debian
sudo apt install clang-format-15

# macOS
brew install clang-format

# Windows
winget install LLVM.LLVM
```

### clang-tidy not found

Install clang-tidy (version 15+ recommended):
```bash
# Ubuntu/Debian
sudo apt install clang-tidy-15

# macOS
brew install llvm

# Windows (included with LLVM)
winget install LLVM.LLVM
```

### Include-cleaner false positives

If include-cleaner suggests removing headers that are actually needed:
1. Check if the header is in the exclusion list (`.clang-tidy` `misc-include-cleaner.IgnoreHeaders`)
2. Add the library/path to exclusions if it's an external dependency
3. Ensure the compile commands database is up to date

### CI failures on pull requests

1. Check the Actions tab for detailed error logs
2. Run the same checks locally (see "Local CI Simulation")
3. Fix violations before pushing again
4. Request help in the PR if you're unsure about a violation

## References

- [clang-format documentation](https://clang.llvm.org/docs/ClangFormat.html)
- [clang-tidy documentation](https://clang.llvm.org/extra/clang-tidy/)
- [GitHub Actions documentation](https://docs.github.com/en/actions)
- Project coding standards: See this document and `CONTRIBUTING.md`
