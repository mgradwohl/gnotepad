#!/usr/bin/env bash
# Run Clang Static Analyzer (scan-build) on GnotePad
# Usage: ./run-scan-build.sh [OPTIONS]
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"

# Defaults
VERBOSE=false
REPORT_DIR="${PROJECT_ROOT}/scan-build-report"

# LLVM paths
LLVM_ROOT="/usr/lib/llvm-21"
CLANG_C="${LLVM_ROOT}/bin/clang"
CLANG_CXX="${LLVM_ROOT}/bin/clang++"

usage() {
    cat <<EOF
Usage: $(basename "$0") [OPTIONS]

Run Clang Static Analyzer (scan-build) on GnotePad source files.
Configures a separate 'analyze' build and runs scan-build.

Options:
  -v, --verbose         Show verbose output
  -o, --output DIR      Output directory for reports (default: scan-build-report)
  -h, --help            Show this help
EOF
    exit 0
}

# Parse arguments
while [[ $# -gt 0 ]]; do
    case "$1" in
        -v|--verbose) VERBOSE=true; shift ;;
        -o|--output) REPORT_DIR="$2"; shift 2 ;;
        -h|--help) usage ;;
        *) echo "Error: Unknown argument: $1" >&2; usage ;;
    esac
done

BUILD_DIR="${PROJECT_ROOT}/build/analyze"

# Configure the analyze build
if $VERBOSE; then
    echo "Configuring analyze build..."
fi
"$SCRIPT_DIR/configure.sh" $($VERBOSE && echo "-v") analyze

# Run scan-build
if $VERBOSE; then
    echo "Running scan-build..."
    echo "Report directory: $REPORT_DIR"
fi

scan-build -o "$REPORT_DIR" --status-bugs \
    --use-cc="$CLANG_C" --use-c++="$CLANG_CXX" \
    cmake --build "$BUILD_DIR"
