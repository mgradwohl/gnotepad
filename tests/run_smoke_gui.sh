#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR=$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)
REPO_ROOT=$(cd "$SCRIPT_DIR/.." && pwd)

BUILD_DIR=${1:-build/debug}
shift || true

if [[ "$BUILD_DIR" != /* ]]; then
    BUILD_DIR="$REPO_ROOT/$BUILD_DIR"
fi

BINARY="$BUILD_DIR/tests/GnotePadSmoke"

if [[ ! -x "$BINARY" ]]; then
    printf 'Test binary not found at %s. Build the project first (e.g. cmake --build build/debug).\n' "$BINARY" >&2
    exit 1
fi

printf 'Launching %s with QT_QPA_PLATFORM=wayland so test windows are visible...\n' "$BINARY"
QT_QPA_PLATFORM=wayland "$BINARY" "$@"
