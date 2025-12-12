#!/usr/bin/env bash
set -euo pipefail

BUILD_DIR=${1:-build/analyze}
REPORT_DIR=${2:-scan-build-report}
: "${QT6_PREFIX:=/usr/lib/x86_64-linux-gnu/cmake/Qt6}"
: "${CC:=/usr/lib/llvm-21/bin/clang}"
: "${CXX:=/usr/lib/llvm-21/bin/clang++}"

cmake -S . -B "${BUILD_DIR}" -G Ninja \
  -DCMAKE_C_COMPILER="${CC}" \
  -DCMAKE_CXX_COMPILER="${CXX}" \
  -DCMAKE_PREFIX_PATH="${QT6_PREFIX}" \
  -DCMAKE_BUILD_TYPE=Debug \
  -DCMAKE_EXE_LINKER_FLAGS=-fuse-ld=lld \
  -DCMAKE_SHARED_LINKER_FLAGS=-fuse-ld=lld

scan-build -o "${REPORT_DIR}" --status-bugs \
  --use-cc="${CC}" --use-c++="${CXX}" \
  cmake --build "${BUILD_DIR}"
