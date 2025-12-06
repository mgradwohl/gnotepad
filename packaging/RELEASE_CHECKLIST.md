# GnotePad Release & Packaging Checklist

> **Scope:** Always perform these steps using the **optimized** build configuration (`build/optimized`). Debug and Release binaries are for local development only.

1. **Prep environment**
   - [ ] Verify all submodules and dependencies are current (`git pull --rebase`, `cmake --version`, `clang++ --version`).
   - [ ] Install/verify packaging prerequisites: `sudo apt install patchelf chrpath desktop-file-utils appstream qt6-base-dev-tools zsync rpm` (provides `qmake6`, `zsyncmake`, `rpmbuild`, and `sha256sum`).
   - [ ] Ensure Qt 6 toolchain, Flatpak SDK, Snapcraft, and code-signing keys are available.
   - [ ] Clean previous artifacts: `rm -rf build/optimized packaging/dist`.

2. **Review & bump version (Semantic Versioning)**
   - [ ] Decide the new version per the Semantic Versioning convention: increment MAJOR for breaking changes, MINOR for backward-compatible features, PATCH for fixes-only releases.
   - [ ] Update version fields in `CMakeLists.txt`, AppStream metadata (`app.gnotepad.GnotePad.appdata.xml`), desktop entries (`app.gnotepad.GnotePad.desktop`), packaging manifests, and any scripts that embed the version string.
   - [ ] Regenerate changelog/README release notes sections to reference the new version so downstream packages stay consistent.

3. **Configure optimized build**
   - [ ] `cmake -S . -B build/optimized -G Ninja -DCMAKE_CXX_COMPILER=clang++ -DCMAKE_PREFIX_PATH=/usr/lib/x86_64-linux-gnu/cmake/Qt6 -DCMAKE_BUILD_TYPE=Release -DGNOTE_ENABLE_IPO=ON -DCMAKE_INTERPROCEDURAL_OPTIMIZATION=ON -DCMAKE_EXE_LINKER_FLAGS_RELEASE='-s' -DCMAKE_SHARED_LINKER_FLAGS_RELEASE='-s'`

4. **Build & test optimized binaries**
   - [ ] `cmake --build build/optimized`
   - [ ] `ctest --test-dir build/optimized`
   - [ ] Capture logs (store under `packaging/logs/<version>`).

5. **Stage installation tree & kick off AppImage build**
   - [ ] Run `tools/package-appimage.sh` (optionally set `VERSION`/`RELEASE_DATE` env vars). The script wipes `packaging/dist/AppDir`, reinstalls the optimized tree, copies `packaging/linux/AppRun`, renders `app.gnotepad.GnotePad.appdata.xml`, downloads/executes `tools/linuxdeployqt.AppImage` with SHA256 checksum verification if needed, and logs to `packaging/dist/linuxdeployqt.log`.
   - [ ] Confirm the script found `patchelf`, `chrpath`, `desktop-file-validate`, `appstreamcli`, `qmake6`, `zsyncmake`, and `sha256sum`. Install any missing tools and retry if it aborts early.
   - [ ] After a successful run, the AppImage and `.zsync` live in `packaging/dist/` as `GnotePad-${VERSION}-x86_64.AppImage(.zsync)`. If linuxdeployqt fails, inspect the log before iterating.

6. **AppImage validation (primary Linux distribution)**
   - [ ] `chmod +x packaging/dist/GnotePad-${VERSION}-x86_64.AppImage` and launch it. Validate dialogs, MRU handling, and default editor registration (optional `xdg-mime default app.gnotepad.GnotePad.desktop text/plain`).
   - [ ] Headless smoke (CI/VM): `QT_QPA_PLATFORM=offscreen ./packaging/dist/GnotePad-${VERSION}-x86_64.AppImage --quit-after-init` should start and exit cleanly.
   - [ ] Retain `packaging/dist/linuxdeployqt.log` alongside the artifact for reproducibility. If linuxdeployqt fails with exit 1 and no message, run `APPIMAGE_EXTRACT_AND_RUN=1` or use the extracted `squashfs-root/AppRun`; ensure `/usr/share/doc/libc6/copyright` is copied into `AppDir` and pass `-no-copy-copyright-files` (handled by `tools/package-appimage.sh`).

7. **Flatpak package (Flathub)**
   - [ ] Update manifests: `packaging/linux/flatpak/app.gnotepad.GnotePad.yml` (builds from the checkout) and `packaging/linux/flatpak/app.gnotepad.GnotePad.flathub.yml` (pinned source archive + sha256 for Flathub). Both vendor LLVM 18.1.8 and force clang/lld with IPO enabled; refresh the LLVM tarball URL + sha256 and the source archive hash per release/tag.
   - [ ] Build/test local checkout: `flatpak-builder build-dir packaging/linux/flatpak/app.gnotepad.GnotePad.yml --install --user`. Launch via `flatpak run app.gnotepad.GnotePad`.
   - [ ] Build/test Flathub manifest: `flatpak-builder build-dir packaging/linux/flatpak/app.gnotepad.GnotePad.flathub.yml --install --user`.
   - [ ] Prepare merge request to Flathub with the pinned manifest and release notes.

8. **Snap package (optional)**
   - [ ] Update `packaging/linux/snap/snapcraft.yaml` (version, confinement, plugs).
   - [ ] `snapcraft --use-lxd` (ensuring optimized binary is staged).
   - [ ] Install locally with `snap install gnotepad_<version>.snap --dangerous` and verify sandbox access (file dialogs, theming).

9. **DEB / RPM artifacts**
   - [ ] Generate packages from the optimized build dir:
     - `cpack -G DEB -C Release -B ../../packaging/dist`
     - `cpack -G RPM -C Release -B ../../packaging/dist`
   - [ ] Verify outputs: `packaging/dist/gnotepad-<version>-Linux.deb` and `.rpm`.
   - [ ] Test install on clean VM (e.g., `sudo dpkg -i ...deb` or `sudo rpm -i ...rpm`) and set default editor via desktop settings.

10. **Windows & macOS builds**
   - [ ] Windows: run optimized build with clang-cl, then `windeployqt` or `cpack -G NSIS/MSI`. Sign executables (Authenticode).
   - [ ] macOS: `cmake -S . -B build/optimized-mac -GXcode -DCMAKE_OSX_ARCHITECTURES="arm64;x86_64"`, `cmake --build`. Use `macdeployqt` to create `.app`, sign & notarize, wrap in `.dmg`.

11. **GitHub release (plan now, execute when ready)**
   - [ ] Tag version (`git tag vX.Y.Z`), push tags.
   - [ ] Draft release notes (features, fixes, known issues). Attach AppImage, Flatpak bundle, DEB/RPM, Windows installer, macOS dmg (skip Snap unless we opt in).
   - [ ] Include SHA256 hashes for all binaries.
   - [ ] Publish the release (remove draft/pre-release flag once validated).
   - [ ] For downstream endpoints (Flathub/AppImageHub/AUR), follow `packaging/PUBLISHING.md`; validate Flathub with `tools/flatpak-build.sh --flathub --install` before submitting.

12. **Publish to distribution endpoints**
   - [ ] Flathub MR with updated pinned manifest and release notes.
   - [ ] AppImageHub listing (upload AppImage + `.zsync`).
   - [ ] AUR (PKGBUILD) and/or Debian PPA if maintained.
   - [ ] Update README / website / landing page download links to point to the GitHub release artifacts.

13. **Promotion & communication**
    - [ ] Post announcement (blog, mailing list, Reddit, Hacker News, Qt/KDE forums).
    - [ ] Update README “Downloads” section with direct links.
    - [ ] Notify package maintainers / distro contacts if they track upstream.

14. **Post-release maintenance**
    - [ ] Monitor crash/bug reports, file issues.
    - [ ] Schedule follow-up for next backlog items (font dialog persistence, ownership cleanup, etc.).
    - [ ] Archive build + packaging logs for auditing.

> Keep this checklist updated with each release. If a new distribution channel is added, append steps above the communication section so the ordering remains prep → build → package → publish → promote.
