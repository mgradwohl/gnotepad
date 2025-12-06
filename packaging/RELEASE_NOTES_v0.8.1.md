# GnotePad v0.8.1

## Highlights
- Desktop integration is solid: AppStream developer ID corrected, desktop `Exec` rewritten to the installed binary, and full hicolor icon index + cache refresh so the launcher icon shows reliably on GNOME/KDE/Wayland.
- Packaging polish: optimized (IPO) build artifacts produced via `linuxdeployqt`; `tools/install-local.sh` installs to a local prefix and refreshes desktop/icon caches automatically.
- Distribution artifacts: AppImage plus DEB/RPM built from the optimized tree; headless smoke flag `--quit-after-init` remains available for CI.

## Downloads (attach in the GitHub release)
- `packaging/dist/GnotePad-0.8.1-x86_64.AppImage`
- `packaging/dist/gnotepad-0.8.1-Linux.deb`
- `packaging/dist/gnotepad-0.8.1-Linux.rpm`
- Optional: `packaging/dist/SHA256SUMS.txt` alongside the assets

## Checksums
```
d7acef1096eea7e2b29b8d9fac541dfa396057defe94cd559b11d4d9beeda777  GnotePad-0.8.1-x86_64.AppImage
cc1eb3ccf7384185213a86942d3c6514c386e47260e0f433d59ebe5e0a258eb5  gnotepad-0.8.1-Linux.deb
a0082a8d9b738d445b534f8aed0026751d82978b4ba5adf8c8d803a6ce33616c  gnotepad-0.8.1-Linux.rpm
```

## Screenshots
Attach from `docs/screenshots/`:
- `gnotepad.png`
- `gnotepad_about.png`
- `gnotepad_pdf.png`

## Usage notes
- AppImage: `chmod +x GnotePad-0.8.1-x86_64.AppImage && ./GnotePad-0.8.1-x86_64.AppImage`
- DEB/RPM: install on a clean system and ensure the desktop icon appears without manual cache resets.
