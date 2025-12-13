#!/usr/bin/env bash
# Configure GnotePad for building
# Usage: ./configure.sh [OPTIONS] [BUILD_TYPE]
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"

# Defaults
BUILD_TYPE="debug"
VERBOSE=false

# LLVM and Qt paths
LLVM_ROOT="/usr/lib/llvm-21"
CLANG_CXX="${LLVM_ROOT}/bin/clang++"
CLANG_C="${LLVM_ROOT}/bin/clang"
QT6_PREFIX="/usr/lib/x86_64-linux-gnu/cmake/Qt6"

usage() {
    cat <<EOF
Usage: $(basename "$0") [OPTIONS] [BUILD_TYPE]

Configure GnotePad for building.

BUILD_TYPE:
  debug           Debug build (default)
  relwithdebinfo  Release with debug info
  release         Release build
  optimized       Optimized build (LTO, march=x86-64-v3, stripped)
  analyze         Build for scan-build analysis

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
        debug|relwithdebinfo|release|optimized|analyze)
            BUILD_TYPE="$1"; shift ;;
        *) echo "Error: Unknown argument: $1" >&2; usage ;;
    esac
done

# Build directory
BUILD_DIR="${PROJECT_ROOT}/build/${BUILD_TYPE}"

# Common CMake args
CMAKE_ARGS=(
    -S "$PROJECT_ROOT"
    -B "$BUILD_DIR"
    -G Ninja
    -DCMAKE_CXX_COMPILER="$CLANG_CXX"
    -DCMAKE_CXX_STANDARD=23
    -DCMAKE_CXX_STANDARD_REQUIRED=ON
    -DCMAKE_CXX_EXTENSIONS=OFF
    -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
    -DCMAKE_PREFIX_PATH="$QT6_PREFIX"
    -DCMAKE_EXE_LINKER_FLAGS=-fuse-ld=lld
    -DCMAKE_SHARED_LINKER_FLAGS=-fuse-ld=lld
)

# Build type specific args
case "$BUILD_TYPE" in
    debug)
        CMAKE_ARGS+=(-DCMAKE_BUILD_TYPE=Debug)
        ;;
    relwithdebinfo)
        CMAKE_ARGS+=(-DCMAKE_BUILD_TYPE=RelWithDebInfo)
        ;;
    release)
        CMAKE_ARGS+=(-DCMAKE_BUILD_TYPE=Release)
        ;;
    optimized)
        CMAKE_ARGS+=(
            -DCMAKE_BUILD_TYPE=Release
            -DGNOTE_ENABLE_IPO=ON
            -DCMAKE_INTERPROCEDURAL_OPTIMIZATION=ON
            -DCMAKE_EXE_LINKER_FLAGS_RELEASE=-s
            -DCMAKE_SHARED_LINKER_FLAGS_RELEASE=-s
            "-DCMAKE_CXX_FLAGS_RELEASE=-O3 -DNDEBUG -march=x86-64-v3 -fomit-frame-pointer"
        )
        ;;
    analyze)
        CMAKE_ARGS+=(
            -DCMAKE_C_COMPILER="$CLANG_C"
            -DCMAKE_BUILD_TYPE=Debug
        )
        ;;
esac

if $VERBOSE; then
    echo "Configuring $BUILD_TYPE build in $BUILD_DIR"
    echo "CMake args:"
    printf '  %s\n' "${CMAKE_ARGS[@]}"
fi

cmake "${CMAKE_ARGS[@]}"

echo "Configuration complete. Run 'tools/build.sh $BUILD_TYPE' to build."
