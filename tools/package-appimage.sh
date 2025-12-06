#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(git rev-parse --show-toplevel 2>/dev/null || pwd)"
cd "${ROOT_DIR}"

VERSION="${VERSION:-$(sed -n 's/project(\s*GnotePad\s*VERSION\s*\([0-9.]*\).*/\1/p' CMakeLists.txt | head -n 1)}"
if [[ -z "${VERSION}" ]]; then
    echo "Unable to determine project version from CMakeLists.txt" >&2
    exit 1
fi

RELEASE_DATE="${RELEASE_DATE:-$(date -I)}"
BUILD_DIR="${ROOT_DIR}/build/optimized"
APPDIR="${ROOT_DIR}/packaging/dist/AppDir"
INSTALL_PREFIX="${APPDIR}/usr"
LINUXDEPLOY_BIN="${ROOT_DIR}/tools/linuxdeployqt.AppImage"
LINUXDEPLOY_URL="${LINUXDEPLOY_URL:-https://github.com/probonopd/linuxdeployqt/releases/download/continuous/linuxdeployqt-continuous-x86_64.AppImage}"
LOG_FILE="${ROOT_DIR}/packaging/dist/linuxdeployqt.log"
APPSTREAM_TEMPLATE="${ROOT_DIR}/packaging/linux/app.gnotepad.GnotePad.appdata.xml.in"
APPRUN_TEMPLATE="${ROOT_DIR}/packaging/linux/AppRun"

require_cmd() {
    if ! command -v "$1" >/dev/null 2>&1; then
        echo "Missing required command: $1" >&2
        exit 1
    fi
}

for tool in cmake patchelf chrpath desktop-file-validate appstreamcli zsyncmake; do
    require_cmd "${tool}"
done

QMAKE_BIN="${QMAKE_BIN:-}"
if [[ -z "${QMAKE_BIN}" ]]; then
    if command -v qmake6 >/dev/null 2>&1; then
        QMAKE_BIN="$(command -v qmake6)"
    elif command -v qmake >/dev/null 2>&1; then
        QMAKE_BIN="$(command -v qmake)"
    else
        echo "Neither qmake6 nor qmake found. Install qt6-base-dev-tools or provide QMAKE_BIN." >&2
        exit 1
    fi
fi

if [[ ! -x "${LINUXDEPLOY_BIN}" ]]; then
    echo "Downloading linuxdeployqt into tools/" >&2
    if command -v curl >/dev/null 2>&1; then
        curl -L --fail -o "${LINUXDEPLOY_BIN}" "${LINUXDEPLOY_URL}"
    elif command -v wget >/dev/null 2>&1; then
        wget -q -O "${LINUXDEPLOY_BIN}" "${LINUXDEPLOY_URL}"
    else
        echo "Need curl or wget to download linuxdeployqt" >&2
        exit 1
    fi
    chmod +x "${LINUXDEPLOY_BIN}"
fi

if [[ ! -x "${BUILD_DIR}/GnotePad" ]]; then
    echo "Missing optimized binary at ${BUILD_DIR}/GnotePad. Build the optimized configuration before packaging." >&2
    exit 1
fi

rm -rf "${APPDIR}"
install -d "${INSTALL_PREFIX}"
cmake --install "${BUILD_DIR}" --prefix "${INSTALL_PREFIX}"

install -m 755 "${APPRUN_TEMPLATE}" "${APPDIR}/AppRun"
install -d "${INSTALL_PREFIX}/share/metainfo"
sed -e "s/@VERSION@/${VERSION}/g" -e "s/@RELEASE_DATE@/${RELEASE_DATE}/g" "${APPSTREAM_TEMPLATE}" \
    > "${INSTALL_PREFIX}/share/metainfo/app.gnotepad.GnotePad.appdata.xml"

LIBC_COPYRIGHT_SRC="/usr/share/doc/libc6/copyright"
LIBC_COPYRIGHT_DST="${INSTALL_PREFIX}/share/doc/libc6/copyright"
if [[ -f "${LIBC_COPYRIGHT_SRC}" ]]; then
    install -Dm644 "${LIBC_COPYRIGHT_SRC}" "${LIBC_COPYRIGHT_DST}"
fi

DESKTOP_FILE="${INSTALL_PREFIX}/share/applications/app.gnotepad.GnotePad.desktop"
if [[ ! -f "${DESKTOP_FILE}" ]]; then
    echo "Desktop file missing at ${DESKTOP_FILE}" >&2
    exit 1
fi

desktop-file-validate "${DESKTOP_FILE}"

export VERSION
if ! "${LINUXDEPLOY_BIN}" "${DESKTOP_FILE}" \
    -appimage \
    -bundle-non-qt-libs \
    -extra-plugins=platforms,platformthemes,iconengines,imageformats \
    -no-copy-copyright-files \
    -unsupported-allow-new-glibc \
    -qmake="${QMAKE_BIN}" \
    >"${LOG_FILE}" 2>&1; then
    echo "linuxdeployqt failed. Review ${LOG_FILE} for details." >&2
    exit 1
fi

DEST_DIR="${ROOT_DIR}/packaging/dist"
install -d "${DEST_DIR}"

# Manually (re)package the AppDir to ensure all staged plugins are included
TMP_EXTRACT_DIR="$(mktemp -d)"
trap 'rm -rf "${TMP_EXTRACT_DIR}"' EXIT
pushd "${TMP_EXTRACT_DIR}" >/dev/null
"${LINUXDEPLOY_BIN}" --appimage-extract >/dev/null
APPIMAGETOOL_BIN="${TMP_EXTRACT_DIR}/squashfs-root/usr/bin/appimagetool"
popd >/dev/null

OUTPUT_APPIMAGE="${DEST_DIR}/GnotePad-${VERSION}-x86_64.AppImage"
"${APPIMAGETOOL_BIN}" "${APPDIR}" "${OUTPUT_APPIMAGE}"

echo "AppImage staging complete. Outputs live in ${DEST_DIR}."
