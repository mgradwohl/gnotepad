# GnotePad Packaging and Release Guide

This directory contains packaging configurations and documentation for distributing GnotePad across multiple platforms.

## Quick Start

### Creating a Release Build

Always use the **optimized** build configuration for releases:

```bash
cmake -S . -B build/optimized -G Ninja \
  -DCMAKE_CXX_COMPILER=clang++ \
  -DCMAKE_PREFIX_PATH=/usr/lib/x86_64-linux-gnu/cmake/Qt6 \
  -DCMAKE_BUILD_TYPE=Release \
  -DGNOTE_ENABLE_IPO=ON \
  -DCMAKE_INTERPROCEDURAL_OPTIMIZATION=ON \
  -DCMAKE_EXE_LINKER_FLAGS_RELEASE='-s' \
  -DCMAKE_SHARED_LINKER_FLAGS_RELEASE='-s'

cmake --build build/optimized
ctest --test-dir build/optimized
```

### Packaging Quick Reference

```bash
# AppImage
bash tools/package-appimage.sh

# DEB/RPM
cd build/optimized
cpack -G DEB -C Release -B ../../packaging/dist
cpack -G RPM -C Release -B ../../packaging/dist

# Flatpak (local test)
tools/flatpak-build.sh --local --install

# Flatpak (Flathub submission)
tools/flatpak-build.sh --flathub --install
```

## Package Formats

### AppImage (Linux - Portable)

**AppImage** is a universal Linux package format that runs on most distributions without installation.

**Prerequisites:**
- `patchelf`, `chrpath`, `desktop-file-validate`, `appstreamcli`, `qmake6`, `zsync`, `sha256sum`
- `tools/package-appimage.sh` automatically downloads `linuxdeployqt` with SHA256 verification

**Build:**
```bash
# From repository root
bash tools/package-appimage.sh

# Optional: Set version/date
VERSION=0.8.1 RELEASE_DATE=2024-01-15 bash tools/package-appimage.sh
```

**Output:**
- `packaging/dist/GnotePad-<version>-x86_64.AppImage`
- `packaging/dist/GnotePad-<version>-x86_64.AppImage.zsync` (for delta updates)
- `packaging/dist/linuxdeployqt.log` (build log)

**Desktop Integration:**
- Desktop file: `app.gnotepad.GnotePad.desktop`
- AppStream metadata: `app.gnotepad.GnotePad.appdata.xml`

**Usage:**
```bash
chmod +x GnotePad-0.8.1-x86_64.AppImage
./GnotePad-0.8.1-x86_64.AppImage

# Headless smoke test (CI-friendly)
QT_QPA_PLATFORM=offscreen ./GnotePad-0.8.1-x86_64.AppImage --quit-after-init
```

### Flatpak (Linux - Sandboxed)

**Flatpak** provides sandboxed applications with controlled permissions.

**Prerequisites:**
- `flatpak-builder`
- Runtimes: `org.kde.Platform//6.7` and `org.kde.Sdk//6.7`

**Manifests:**
- `packaging/linux/flatpak/app.gnotepad.GnotePad.yml` - Builds from local checkout (development)
- `packaging/linux/flatpak/app.gnotepad.GnotePad.flathub.yml` - Pinned source archive + sha256 (Flathub submission)

Both manifests vendor LLVM 18.1.8 and force clang/lld with IPO enabled.

**Build (local development):**
```bash
# Using helper script
tools/flatpak-build.sh --local --install

# Manual build
flatpak-builder build-dir packaging/linux/flatpak/app.gnotepad.GnotePad.yml --install --user
```

**Build (Flathub submission):**
```bash
# Using helper script
tools/flatpak-build.sh --flathub --install

# Manual build
flatpak-builder build-dir packaging/linux/flatpak/app.gnotepad.GnotePad.flathub.yml --install --user
```

**Run:**
```bash
flatpak run app.gnotepad.GnotePad

# With version or headless flags
flatpak run app.gnotepad.GnotePad --version
flatpak run app.gnotepad.GnotePad --quit-after-init
```

### DEB/RPM (Linux - Native Package Managers)

**DEB** (Debian/Ubuntu) and **RPM** (Fedora/RHEL) packages for distribution-native installation.

**Prerequisites:**
- `dpkg-dev` (usually present on Ubuntu)
- `rpm` (for rpmbuild)

**Build:**
```bash
cd build/optimized

# DEB package
cpack -G DEB -C Release -B ../../packaging/dist

# RPM package
cpack -G RPM -C Release -B ../../packaging/dist
```

**Output:**
- `packaging/dist/gnotepad-<version>-Linux.deb`
- `packaging/dist/gnotepad-<version>-Linux.rpm`

**Install:**
```bash
# Debian/Ubuntu
sudo dpkg -i packaging/dist/gnotepad-0.8.1-Linux.deb

# Fedora/RHEL
sudo rpm -i packaging/dist/gnotepad-0.8.1-Linux.rpm
```

### Windows (MSIX/NSIS) - Coming Soon

Placeholder for Windows packaging instructions.

### macOS (DMG) - Coming Soon

Placeholder for macOS packaging instructions.

## Release Process

### Semantic Versioning

GnotePad follows [Semantic Versioning](https://semver.org/):
- **MAJOR** version for breaking changes
- **MINOR** version for backward-compatible features
- **PATCH** version for backward-compatible fixes

### Release Checklist

#### 1. Prepare Environment

- [ ] Update all dependencies: `git pull --rebase`, verify CMake, clang, Qt versions
- [ ] Install packaging prerequisites:
  ```bash
  sudo apt install patchelf chrpath desktop-file-utils appstream qt6-base-dev-tools zsync rpm
  ```
- [ ] Clean previous artifacts: `rm -rf build/optimized packaging/dist`

#### 2. Update Version

- [ ] Update version in `CMakeLists.txt`
- [ ] Update AppStream metadata: `app.gnotepad.GnotePad.appdata.xml`
- [ ] Update desktop entry: `app.gnotepad.GnotePad.desktop`
- [ ] Update Flatpak manifests with new version and source archive SHA256
- [ ] Update release notes template

#### 3. Build and Test

- [ ] Configure optimized build (see command above)
- [ ] Build: `cmake --build build/optimized`
- [ ] Run tests: `ctest --test-dir build/optimized`
- [ ] Capture build logs: `packaging/logs/<version>/`

#### 4. Create Packages

- [ ] **AppImage:** `bash tools/package-appimage.sh`
  - Verify: `chmod +x packaging/dist/GnotePad-*.AppImage && ./packaging/dist/GnotePad-*.AppImage`
  - Headless: `QT_QPA_PLATFORM=offscreen ./packaging/dist/GnotePad-*.AppImage --quit-after-init`
- [ ] **Flatpak (local):** `tools/flatpak-build.sh --local --install`
  - Test: `flatpak run app.gnotepad.GnotePad`
- [ ] **Flatpak (Flathub):** `tools/flatpak-build.sh --flathub --install`
  - Test: `flatpak run app.gnotepad.GnotePad`
- [ ] **DEB:** `cd build/optimized && cpack -G DEB -C Release -B ../../packaging/dist`
  - Test on clean VM: `sudo dpkg -i packaging/dist/gnotepad-*.deb`
- [ ] **RPM:** `cd build/optimized && cpack -G RPM -C Release -B ../../packaging/dist`
  - Test on clean VM: `sudo rpm -i packaging/dist/gnotepad-*.rpm`

#### 5. Generate Checksums

```bash
cd packaging/dist
sha256sum GnotePad-*.AppImage gnotepad-*.deb gnotepad-*.rpm > SHA256SUMS.txt
```

#### 6. Create GitHub Release

- [ ] Tag version: `git tag v0.8.1 && git push origin v0.8.1`
- [ ] Draft release notes (features, fixes, known issues)
- [ ] Attach artifacts:
  - `GnotePad-<version>-x86_64.AppImage`
  - `gnotepad-<version>-Linux.deb`
  - `gnotepad-<version>-Linux.rpm`
  - `SHA256SUMS.txt`
- [ ] Attach screenshots from `docs/screenshots/`
- [ ] Publish release

#### 7. Publish to Distribution Channels

**Flathub:**
1. Update `packaging/linux/flatpak/app.gnotepad.GnotePad.flathub.yml`:
   - Set source URL to GitHub release tarball
   - Update SHA256 hash of source archive
   - Update LLVM tarball URL and SHA256 if needed
2. Test locally: `tools/flatpak-build.sh --flathub --install`
3. Submit merge request to Flathub repository
4. Include release notes and checksums
5. Attach screenshots if requested

**AppImageHub:**
1. Publish AppImage on GitHub release (already done)
2. Optional: Generate `.zsync` for delta updates:
   ```bash
   cd packaging/dist
   zsyncmake GnotePad-<version>-x86_64.AppImage
   ```
3. Submit to AppImageHub with:
   - Direct link to AppImage from GitHub release
   - SHA256 from `SHA256SUMS.txt`
   - Changelog excerpt

**AUR (Arch User Repository):**
- Create/update PKGBUILD pointing to v<version> source tarball
- Update `pkgver` and `sha256sums`
- Push to AUR repository

**PPA/Other Repositories:**
- Upload DEB/RPM to distribution-specific repositories
- Update repository metadata

#### 8. Update Documentation

- [ ] Update README.md download links
- [ ] Update website/landing page
- [ ] Update documentation links

#### 9. Announce Release

- [ ] Post announcement (blog, mailing list, social media)
- [ ] Notify package maintainers
- [ ] Post on relevant forums (Reddit, Hacker News, Qt/KDE communities)

#### 10. Post-Release

- [ ] Monitor crash/bug reports
- [ ] File issues for discovered bugs
- [ ] Archive build and packaging logs
- [ ] Plan next release cycle

## Publishing Workflows

### Flathub Submission

Flathub is the primary distribution channel for Flatpak applications.

**Steps:**

1. **Prepare manifest:**
   - Use `packaging/linux/flatpak/app.gnotepad.GnotePad.flathub.yml`
   - Update source URL to GitHub release tarball: `https://github.com/mgradwohl/gnotepad/archive/refs/tags/v0.8.1.tar.gz`
   - Update source SHA256 (get from GitHub release or compute locally)
   - Verify LLVM tarball URL and SHA256 are current

2. **Test locally:**
   ```bash
   tools/flatpak-build.sh --flathub --install
   flatpak run app.gnotepad.GnotePad --quit-after-init
   flatpak run app.gnotepad.GnotePad
   ```

3. **Submit to Flathub:**
   - Fork the [Flathub repository](https://github.com/flathub/flathub) if you haven't already
   - Create branch: `git checkout -b gnotepad-v0.8.1`
   - Copy updated manifest to your fork
   - Commit and push to your fork
   - Open merge request from your fork to the main Flathub repository with:
     - Release notes
     - Checksums from `SHA256SUMS.txt`
     - Screenshots from `docs/screenshots/`
     - Note any warnings (e.g., benign Qt DPR cache warning)

4. **Review process:**
   - Flathub maintainers will review
   - Address any feedback
   - Once approved, package becomes available on Flathub

### AppImageHub Submission

AppImageHub provides a directory of available AppImages.

**Steps:**

1. **Publish on GitHub:**
   - Ensure AppImage is attached to GitHub release
   - Include `.zsync` file for delta updates (optional for first release)

2. **Submit to AppImageHub:**
   - Open issue or pull request on AppImageHub repository
   - Provide:
     - Direct download link to AppImage
     - SHA256 checksum
     - Short description
     - Changelog excerpt
     - Category tags

3. **Verification:**
   - AppImageHub will test the AppImage
   - Once verified, it appears in the directory

### Repository Maintenance

**Updating for new releases:**

1. Increment version number across all files
2. Update source archive URLs and checksums
3. Test all package formats
4. Update documentation
5. Tag and release
6. Submit to distribution channels

**Security updates:**

- For security fixes, bump PATCH version
- Create patches for supported release branches
- Notify distribution channel maintainers
- Document CVE if applicable

## Directory Structure

```
packaging/
├── README.md                      # This file
├── dist/                          # Build output (generated)
│   ├── GnotePad-*.AppImage        # AppImage package
│   ├── gnotepad-*.deb             # Debian package
│   ├── gnotepad-*.rpm             # RPM package
│   ├── SHA256SUMS.txt             # Checksums
│   └── linuxdeployqt.log          # AppImage build log
├── linux/
│   ├── flatpak/
│   │   ├── app.gnotepad.GnotePad.yml         # Flatpak manifest (local)
│   │   └── app.gnotepad.GnotePad.flathub.yml # Flatpak manifest (Flathub)
│   └── AppRun                     # AppImage entry point
└── logs/                          # Release logs (gitignored)
```

## Troubleshooting

### AppImage Build Fails

**Check prerequisites:**
```bash
which patchelf chrpath desktop-file-validate appstreamcli qmake6 zsyncmake sha256sum
```

**Install missing tools:**
```bash
sudo apt install patchelf chrpath desktop-file-utils appstream qt6-base-dev-tools zsync
```

**Check linuxdeployqt log:**
```bash
cat packaging/dist/linuxdeployqt.log
```

**Common issues:**
- Missing copyright file: Ensure `/usr/share/doc/libc6/copyright` exists
- Plugin path issues: Verify Qt installation and `QT_PLUGIN_PATH`

### Flatpak Build Fails

**Verify runtimes:**
```bash
flatpak list | grep org.kde
```

**Install missing runtimes:**
```bash
flatpak install flathub org.kde.Platform//6.7 org.kde.Sdk//6.7
```

**Clean build directory:**
```bash
rm -rf build-dir .flatpak-builder
tools/flatpak-build.sh --flathub --install
```

### DEB/RPM Build Fails

**Check CPack configuration:**
```bash
cd build/optimized
cmake . -LAH | grep CPACK
```

**Install packaging tools:**
```bash
sudo apt install dpkg-dev rpm
```

**Check build output:**
- DEB errors: `dpkg-deb --contents packaging/dist/gnotepad-*.deb`
- RPM errors: `rpm -qilp packaging/dist/gnotepad-*.rpm`

## Resources

- **Semantic Versioning:** https://semver.org/
- **Flathub Documentation:** https://docs.flathub.org/
- **AppImage Documentation:** https://docs.appimage.org/
- **CPack Documentation:** https://cmake.org/cmake/help/latest/module/CPack.html
- **Qt Deployment:** https://doc.qt.io/qt-6/linux-deployment.html

## Support

For packaging issues:
1. Check this documentation
2. Review build logs in `packaging/dist/` and `packaging/logs/`
3. Test on clean VM or container
4. Open GitHub issue with:
   - Build output/logs
   - Platform details
   - Steps to reproduce
