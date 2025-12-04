#!/usr/bin/env bash
set -euo pipefail

BUILD_DIR=${1:-build/analyze}
REPORT_DIR=${2:-scan-build-report}
: "${QT6_PREFIX:=/usr/lib/x86_64-linux-gnu/cmake/Qt6}"
: "${CXX:=clang++}"

cmake -S . -B "${BUILD_DIR}" -G Ninja \
  -DCMAKE_CXX_COMPILER="${CXX}" \
  -DCMAKE_PREFIX_PATH="${QT6_PREFIX}" \
  -DCMAKE_BUILD_TYPE=Debug

scan-build -o "${REPORT_DIR}" --status-bugs cmake --build "${BUILD_DIR}"
