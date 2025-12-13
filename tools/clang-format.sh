#!/usr/bin/env bash
# Apply clang-format to GnotePad source files
# Uses .clang-format configuration from project root
# Usage: ./clang-format.sh [OPTIONS] [BUILD_TYPE]
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"

# Defaults
BUILD_TYPE="debug"
VERBOSE=false

usage() {
    cat <<EOF
Usage: $(basename "$0") [OPTIONS] [BUILD_TYPE]

Apply clang-format to all GnotePad source files (in-place).
Use check-format.sh to check without modifying files.

BUILD_TYPE:
  debug           Use debug build (default)

Options:
  -v, --verbose   Show verbose output
  -h, --help      Show this help
EOF
    exit 0
}

# Parse arguments
while [[ $# -gt 0 ]]; do
    case "$1" in
        -v|--verbose) VERBOSE=true; shift ;;
        -h|--help) usage ;;
        debug) BUILD_TYPE="$1"; shift ;;
        *) echo "Error: Unknown argument: $1" >&2; usage ;;
    esac
done

BUILD_DIR="${PROJECT_ROOT}/build/${BUILD_TYPE}"

# Configure if needed
if [[ ! -f "$BUILD_DIR/build.ninja" ]]; then
    echo "Build not configured. Running configure.sh $BUILD_TYPE..."
    "$SCRIPT_DIR/configure.sh" $($VERBOSE && echo "-v") "$BUILD_TYPE"
fi

if $VERBOSE; then
    echo "Applying clang-format to source files..."
fi

cmake --build "$BUILD_DIR" --target run-clang-format
