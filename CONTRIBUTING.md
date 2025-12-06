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

We rely on clang-tidy's include-cleaner and `.clangd` to keep headers stable. Follow these rules whenever editing or adding headers/translation units:

1. **Include what you use.** Every symbol that appears must pull in its own header; do not depend on transitive includes.
2. **Use `#pragma once`.** All project headers already use it; mirror that style in any new files.
3. **Apply the standard include order:**
   - (Only in `.cpp` files) the corresponding header: `#include "ThisFile.h"`
   - C++ standard library headers, alphabetical
   - Third-party/SDK headers, alphabetical (Qt, spdlog, etc.)
   - Project headers, alphabetical
   Separate each group with a blank line and keep redundant includes out of the diff.
4. **Blank templates for new files.** When creating a new `.h/.hpp/.cpp`, start with the above include skeleton so reviews stay small and rebuilds stay fast.

## Formatting & Static Analysis

- **clang-format:** All C++ sources follow the repository `.clang-format` (LLVM base, Allman braces). Run the `run-clang-format` target or the VS Code task before submitting.
- **clang-tidy / scan-build:** clang-tidy runs on the Debug configuration; scan-build hooks live in `tools/run-scan-build.sh` and the **Scan-Build (Debug)** task. Fix or explain any new warnings.

## Tests

- Unit and smoke tests live under `tests/`. Extend the suite when you add new user-visible functionality or regressions.
- GUI smoke tests (`GnotePadSmoke`) run via `ctest --test-dir build/debug`. Make sure they pass locally on the configurations you touch.

Questions? Open a GitHub discussion or tag `@mgradwohl` in your PR.
