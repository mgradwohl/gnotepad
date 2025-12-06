# Publishing GnotePad 0.8.1

This guide covers pushing the 0.8.1 release to downstream endpoints: Flathub, AppImageHub, and optional AUR/DEB/RPM mirrors. All commands assume the repo root as the working directory.

## Prerequisites
- GitHub release `v0.8.1` with artifacts: `GnotePad-0.8.1-x86_64.AppImage`, `gnotepad-0.8.1-Linux.deb`, `gnotepad-0.8.1-Linux.rpm`, and `SHA256SUMS.txt`.
- Optimized build already produced (`build/optimized`).
- Flatpak tooling: `flatpak-builder`, `org.kde.Platform//6.7`, `org.kde.Sdk//6.7` installed.
- AppImage tooling: `zsyncmake` if regenerating `.zsync`.

## Flathub merge request (primary)
1. Ensure the Flathub manifest is pinned to the tag (already updated): `packaging/linux/flatpak/app.gnotepad.GnotePad.flathub.yml` points to `https://github.com/mgradwohl/GnotePad/archive/refs/tags/v0.8.1.tar.gz` with SHA `5d59e2898d1db67d2ccd2ae1b5f1b58e8c89fd7b7983f25728046949fe39f840`.
2. Build and install locally for verification (uses the Flathub manifest):
   ```bash
   tools/flatpak-build.sh --flathub --install
   flatpak run app.gnotepad.GnotePad --quit-after-init
   ```
3. Submit MR to the Flathub repo with:
   - Updated `app.gnotepad.GnotePad.flathub.yml`.
   - Release notes: `packaging/RELEASE_NOTES_v0.8.1.md`.
   - Mention checksums from `packaging/dist/SHA256SUMS.txt` and attach screenshots from `docs/screenshots/` if requested.
   - Note the benign Qt cached DPR warning observed during headless run.

## AppImageHub submission (optional but recommended)
1. Ensure AppImage and `.zsync` are available. If `.zsync` is missing, regenerate via:
   ```bash
   bash tools/package-appimage.sh
   ```
   Artifacts land in `packaging/dist/`.
2. Publish artifacts on the GitHub release (already done).
3. Submit to AppImageHub (appimage.github.io): open a pull request or issue providing:
   - Direct links to the AppImage and `.zsync` from the GitHub release.
   - The AppImage SHA256 from `packaging/dist/SHA256SUMS.txt`.
   - A short changelog from `packaging/RELEASE_NOTES_v0.8.1.md`.

## AUR / other downstreams (optional)
- **AUR**: craft a PKGBUILD that pulls the `v0.8.1` source tarball or AppImage. Set `pkgver=0.8.1`, update `sha256sums`, and push to your AUR repo.
- **DEB/RPM mirrors**: the release already includes `gnotepad-0.8.1-Linux.deb` and `.rpm` from `cpack`. Upload to your PPA/repo with your usual tooling; cite `packaging/dist/SHA256SUMS.txt`.

## Quick commands reference
- Flatpak build/test (Flathub manifest):
  ```bash
  tools/flatpak-build.sh --flathub --install
  flatpak run app.gnotepad.GnotePad --version
  flatpak run app.gnotepad.GnotePad --quit-after-init
  ```
- Regenerate AppImage + .zsync:
  ```bash
  bash tools/package-appimage.sh
  ```
- DEB/RPM from optimized build:
  ```bash
  (cd build/optimized && cpack -G DEB -C Release -B ../../packaging/dist)
  (cd build/optimized && cpack -G RPM -C Release -B ../../packaging/dist)
  ```
