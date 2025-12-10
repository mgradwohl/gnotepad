# GnotePad

GnotePad is a cross-platform Qt 6 clone of the Windows 10 Notepad experience. The project targets modern C++23, depends on clang + CMake, and keeps its functionality portable across Linux and Windows.

Active development tracks the backlog outlined below on `main`, folding in incremental UX and tooling improvements as they land.

## Prerequisites

- CMake 3.26+ (We use 4.2)
- Ninja or Make (Ninja recommended)
- Clang 15+ (or Apple Clang 15+ on macOS)
- Qt 6.5+ development packages (Widgets, Gui, Core, PrintSupport)

## Backlog & Next Steps

**Remaining focus areas**

- Broaden Unicode/encoding regression tests with round-trip corpora to validate decoder/encoder behavior.
- Finish Notepad UX parity:
  - Page Setup and Print Preview commands.
  - Ensure the font dialog remembers prior selections (family/size/script) across launches and persists those settings alongside the editor font.
  - Add multi-document handling (tabs or multi-window management that mirrors modern Notepad).
- Ship desktop-ready packages (AppImage, MSIX, dmg) and the associated integration assets.
- Improve large-file responsiveness via async load indicators and targeted profiling.
- Modernize ownership in `MainWindow`/`TextEditor`: move long-lived members (status labels, menus, the editor widget, etc.) away from naked `new` to safer ownership constructs (smart pointers or stack members) while still honoring Qt parent-child lifetimes.
- Replace UI-related magic numbers (window sizes, zoom defaults, pixmap dimensions) with named constants.

**Recently completed**

- Menu actions (Save/Save As/Find/Cut/Copy/etc.) now enable and disable automatically based on document content and selection state to match Windows Notepad behavior.
- Font dialog launch moved out of an inline lambda and centralized in `handleChooseFont()`, opening the door for future persistence improvements.
