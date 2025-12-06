#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(git rev-parse --show-toplevel 2>/dev/null || pwd)"
BUILD_DIR="${ROOT_DIR}/build-dir"
LOCAL_MANIFEST="${ROOT_DIR}/packaging/linux/flatpak/app.gnotepad.GnotePad.yml"
FLATHUB_MANIFEST="${ROOT_DIR}/packaging/linux/flatpak/app.gnotepad.GnotePad.flathub.yml"
MANIFEST="${FLATHUB_MANIFEST}"
INSTALL_FLAGS=(--install --user)

usage() {
    cat <<'EOF'
Usage: tools/flatpak-build.sh [--flathub|--local] [--no-install]
  --flathub      Use the Flathub-pinned manifest (default)
  --local        Use the checkout-sourced manifest
  --no-install   Build only (skip installing to user)
EOF
}

while [[ $# -gt 0 ]]; do
    case "$1" in
        --flathub)
            MANIFEST="${FLATHUB_MANIFEST}"
            shift
            ;;
        --local)
            MANIFEST="${LOCAL_MANIFEST}"
            shift
            ;;
        --no-install)
            INSTALL_FLAGS=()
            shift
            ;;
        -h|--help)
            usage
            exit 0
            ;;
        *)
            echo "Unknown option: $1" >&2
            usage
            exit 1
            ;;
    esac
done

cmd=(flatpak-builder --force-clean "${BUILD_DIR}" "${MANIFEST}")
if [[ ${#INSTALL_FLAGS[@]} -gt 0 ]]; then
    cmd+=("${INSTALL_FLAGS[@]}")
fi

echo "Using manifest: ${MANIFEST}"
"${cmd[@]}"
