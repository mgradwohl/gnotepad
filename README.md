# GnotePad

GnotePad is a cross-platform Qt 6 clone of the Windows 10 Notepad experience. The project targets modern C++23 and keeps its functionality portable across Linux, Windows, and macOS.

## Features

- Native file dialogs with UTF-8/16 BOM detection and encoding picker
- Persistent preferences (window geometry, recent files, tab size, word wrap, line numbers, zoom)
- Advanced text editor with line numbers, zoom controls, and configurable tab spacing
- Find/Replace dialogs, Go To Line, time/date insertion
- PDF export and recent-files menu
- Command-line support with headless mode for testing

## Installation

### Pre-built Packages

Download the latest release from the [Releases page](https://github.com/mgradwohl/gnotepad/releases):

- **Linux: DEB, or RPM packages (AppImage and Flatpak are coming soon)**
- **Windows**: ZIP (coming soon)
- **macOS**: DMG package (coming soon)

### Linux Quick Start

**DEB/RPM:**
```bash
sudo dpkg -i gnotepad-0.8.1-Linux.deb  # Debian/Ubuntu
sudo rpm -i gnotepad-0.8.1-Linux.rpm   # Fedora/RHEL
```

**AppImage (coming soon)**
```bash
chmod +x GnotePad-0.8.1-x86_64.AppImage
./GnotePad-0.8.1-x86_64.AppImage
```
**Flatpak: (coming soon)**
```bash
flatpak install flathub app.gnotepad.GnotePad
flatpak run app.gnotepad.GnotePad
```
## Building from Source

For developers who want to build from source, see [CONTRIBUTING.md](CONTRIBUTING.md) for detailed build instructions and development environment setup.

## Documentation

- [CONTRIBUTING.md](CONTRIBUTING.md) - Development environment setup, build instructions, and contribution guidelines
- [tests/README.md](tests/README.md) - Testing guide and test file documentation
- [packaging/README.md](packaging/README.md) - Packaging and release documentation

## Backlog & Next Steps

**Current focus areas:**

- Broaden Unicode/encoding regression tests with round-trip corpora to validate decoder/encoder behavior
- Finish Notepad UX parity:
  - Page Setup and Print Preview commands
  - Ensure the font dialog remembers prior selections (family/size/script) across launches and persists those settings alongside the editor font
  - Add multi-document handling (tabs or multi-window management that mirrors modern Notepad)
- Ship desktop-ready packages (MSIX for Windows, DMG for macOS)
- Improve large-file responsiveness via async load indicators and targeted profiling
- Modernize ownership in `MainWindow`/`TextEditor`: move long-lived members away from naked `new` to safer ownership constructs (smart pointers or stack members) while honoring Qt parent-child lifetimes
- Replace UI-related magic numbers with named constants

**Recently completed:**

- Menu actions (Save/Save As/Find/Cut/Copy/etc.) now enable and disable automatically based on document content and selection state to match Windows Notepad behavior
- Font dialog launch centralized in `handleChooseFont()`, opening the door for future persistence improvements
- AppImage, DEB, and RPM packaging with proper desktop integration

## Contributing

Contributions are welcome! Please see [CONTRIBUTING.md](CONTRIBUTING.md) for:
- Development environment setup
- Build and test instructions
- Code style and formatting guidelines
- Workflow basics

## License

See [LICENSE](LICENSE) file for details.

## Contact

- GitHub Issues: https://github.com/mgradwohl/GnotePad/issues
- Project Maintainer: @mgradwohl
