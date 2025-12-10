# Contributing to GnotePad

Thanks for helping improve GnotePad! This document highlights the day-to-day expectations for code changes. For environment setup, build commands, and platform details, refer to the main `README.md`.

## Workflow Basics

1. **Build before you push.** Configure with CMake (Debug or Release) and ensure `ctest` passes.
2. **Run the tooling targets.** `cmake --build build/debug --target run-clang-format` keeps style consistent; `cmake --build build/debug --target run-clang-tidy` (or the VS Code tasks) should finish cleanly before opening a PR.
3. **Keep commits focused.** Group related changes together and describe the intent plus any notable testing in the commit message or PR description.

## Local install for manual testing

- Build the optimized config first (`cmake --build build/optimized`).
- Run `bash tools/install-local.sh` (optionally set `PREFIX=/custom/prefix`). This installs the optimized binary, rewrites the desktop `Exec` to the installed path, and refreshes desktop/icon caches so the launcher entry appears under `app.gnotepad.GnotePad`.

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
   
5. **New files inherit the same rules.** Start with the include skeleton above so diffs stay small.

clang-tidy plus `.clangd`'s `-fno-modules` flag help surface violations locally. Running `cmake --build build/debug --target run-clang-tidy` (or the VS Code **Clang-Tidy (Debug)** task) will confirm that headers stay clean.

## Formatting & Static Analysis

- **clang-format:** All C++ sources follow the repository `.clang-format` (LLVM base, Allman braces). Run the `run-clang-format` target or the VS Code task before submitting.
- **clang-tidy / scan-build:** clang-tidy runs on the Debug configuration; scan-build hooks live in `tools/run-scan-build.sh` and the **Scan-Build (Debug)** task. Fix or explain any new warnings.

## Tests

- Unit and smoke tests live under `tests/`. Extend the suite when you add new user-visible functionality or regressions.
- GUI smoke tests (`GnotePadSmoke`) run via `ctest --test-dir build/debug`. Make sure they pass locally on the configurations you touch.

Questions? Open a GitHub discussion or tag `@mgradwohl` in your PR.
