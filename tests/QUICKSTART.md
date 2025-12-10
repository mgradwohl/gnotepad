# Quick Start Guide: UI and Encoding Tests

This guide helps you quickly get started with the comprehensive test suite in GnotePad, including UI interaction tests and encoding tests.

## TL;DR

```bash
# Verify test setup
cd tests
./verify_test_setup.sh

# Build and run all tests (requires Qt6)
cd ..
cmake --build build/debug
ctest --test-dir build/debug

# Run UI tests only
ctest --test-dir build/debug -R "Dialog|Menu|StatusBar|Zoom"

# Run encoding tests only
ctest --test-dir build/debug -R "Utf8Bom|Utf16|Multilingual|Unicode|RoundTrip"
```

## What's New?

### UI Tests (18 new tests)
- ✅ Menu and action validation
- ✅ Dialog invocation tracking
- ✅ Status bar interactions
- ✅ Zoom control flows
- ✅ Keyboard shortcut validation
- ✅ Action state management
- ✅ Word wrap toggle
- ✅ Recent files menu
- ✅ Tooltip presence

### Encoding Tests (9 tests)
- ✅ BOM detection (UTF-8, UTF-16 LE, UTF-16 BE)
- ✅ Multilingual content (14+ languages)
- ✅ Unicode characters (symbols, emoji, math operators)
- ✅ Round-trip encoding conversions
- ✅ Large file handling (60KB+)

## Test Categories

### UI Interaction Tests

All UI tests in `tests/smoke/MainWindowSmokeTests`:

| Test | What It Validates |
|------|------------------|
| `testStatusBarToggle` | Status bar show/hide |
| `testStatusBarLabelsUpdate` | Status labels update on changes |
| `testZoomLabelUpdates` | Zoom percentage display |
| `testMenuActionsExist` | All main menus and actions present |
| `testMenuShortcuts` | Keyboard shortcuts registered |
| `testEditMenuActionsEnabled` | Context-sensitive action states |
| `testFindDialogInvocation` | Find dialog opens correctly |
| `testReplaceDialogInvocation` | Replace dialog opens correctly |
| `testGoToLineDialog` | Go To Line dialog accessible |
| `testTabSizeDialog` | Tab Size dialog accessible |
| `testFontDialogInvocation` | Font selection dialog accessible |
| `testWordWrapToggle` | Word wrap on/off functionality |
| `testDateFormatPreference` | Date/time insertion |
| `testAboutDialog` | About dialog accessible |
| `testEncodingDialogFlow` | Encoding selection flow |
| `testActionStateManagement` | Actions enable/disable correctly |
| `testTooltipPresence` | UI elements have tooltips |
| `testRecentFilesMenuActions` | Recent files menu functions |

### Encoding Tests

```
tests/testfiles/
├── encoding-tests/          # Files with different encodings + BOMs
│   ├── utf8_with_bom.txt    # UTF-8 + BOM (EF BB BF)
│   ├── utf8_no_bom.txt      # UTF-8, no BOM
│   ├── utf16le_with_bom.txt # UTF-16 LE + BOM (FF FE)
│   ├── utf16be_with_bom.txt # UTF-16 BE + BOM (FE FF)
│   └── mixed_content.txt    # Multilingual test content
└── utf8-tests/              # UTF-8 files with diverse content
    ├── multilingual.txt     # 14 languages
    ├── unicode_chars.txt    # Symbols, emoji, math
    ├── special_chars.txt    # Zero-width, combining chars
    ├── line_endings_*.txt   # LF and CRLF variants
    └── large_multilingual.txt # 60KB performance test
```

## New Test Methods

| Test | What It Validates |
|------|------------------|
| `testUtf8BomDetection` | UTF-8 BOM detection |
| `testUtf16LEBomDetection` | UTF-16 LE BOM detection |
| `testUtf16BEBomDetection` | UTF-16 BE BOM detection |
| `testMultilingualContent` | 14+ language scripts load correctly |
| `testUnicodeCharacters` | Symbols, emoji, math operators |
| `testEncodingRoundTripUtf8ToBom` | UTF-8 ↔ UTF-8+BOM conversion |
| `testEncodingRoundTripUtf16Variants` | UTF-8 ↔ UTF-16 LE ↔ UTF-16 BE |
| `testLargeFileEncodingRoundTrip` | 60KB file encoding conversions |
| `testMixedContentPreservation` | Complex encoding chains |

## Common Tasks

### Verify Test Setup

```bash
cd tests
./verify_test_setup.sh
```

### Regenerate Test Files

```bash
cd tests/testfiles
./download_encoding_tests.sh  # Creates UTF-8 test files
./create_bom_files.sh          # Creates BOM test files
```

### Run UI Tests

```bash
# Run all UI interaction tests
ctest --test-dir build/debug -R "Dialog|Menu|StatusBar|Zoom|Action"

# Run specific UI test
build/debug/GnotePadSmoke testMenuActionsExist

# Run dialog tests
ctest --test-dir build/debug -R "Dialog"

# Run menu tests
ctest --test-dir build/debug -R "Menu"
```

### Run Encoding Tests

### Run Encoding Tests

```bash
# Run one encoding test
build/debug/GnotePadSmoke testUtf8BomDetection

# Run all encoding tests
ctest --test-dir build/debug -R "Utf|Encoding|Multilingual|Unicode"

# Run all tests
ctest --test-dir build/debug
```

### Check BOM Markers

```bash
cd tests/testfiles

# UTF-8 BOM (should show: ef bb bf)
hexdump -C encoding-tests/utf8_with_bom.txt | head -1

# UTF-16 LE BOM (should show: ff fe)
hexdump -C encoding-tests/utf16le_with_bom.txt | head -1

# UTF-16 BE BOM (should show: fe ff)
hexdump -C encoding-tests/utf16be_with_bom.txt | head -1
```

### View Test Content

```bash
cd tests/testfiles

# Show multilingual content
cat utf8-tests/multilingual.txt

# Show Unicode characters
cat utf8-tests/unicode_chars.txt | head -20

# Check file size
ls -lh utf8-tests/large_multilingual.txt
```

## Understanding the Tests

### BOM Detection Tests

These tests verify that GnotePad correctly identifies Byte Order Mark (BOM) sequences:

- **UTF-8 BOM**: `EF BB BF` (optional, indicates UTF-8)
- **UTF-16 LE BOM**: `FF FE` (required for UTF-16 Little Endian)
- **UTF-16 BE BOM**: `FE FF` (required for UTF-16 Big Endian)

```cpp
// Example: testUtf8BomDetection
MainWindow window;
window.testLoadDocument("encoding-tests/utf8_with_bom.txt");
QVERIFY(window.currentBomForTest());  // Should detect BOM

window.testLoadDocument("encoding-tests/utf8_no_bom.txt");
QVERIFY(!window.currentBomForTest()); // Should not detect BOM
```

### Round-Trip Tests

These tests verify encoding conversions don't lose data:

1. Load file in encoding A
2. Save as encoding B
3. Reload file
4. Verify content unchanged

```cpp
// Example: UTF-8 → UTF-16 LE → reload
window.testLoadDocument("source.txt");
QString original = editor->toPlainText();

window.testSaveDocumentWithEncoding("output.txt", 
    QStringConverter::Utf16LE, true);

editor->clear();
window.testLoadDocument("output.txt");
QCOMPARE(editor->toPlainText(), original); // Content preserved!
```

### Content Preservation Tests

These tests verify Unicode characters survive loading:

```cpp
// Example: testMultilingualContent
window.testLoadDocument("multilingual.txt");
QString content = editor->toPlainText();

QVERIFY(content.contains("English"));     // Latin
QVERIFY(content.contains("Быстрая"));     // Cyrillic
QVERIFY(content.contains("敏捷的"));       // Chinese
QVERIFY(content.contains("素早い"));       // Japanese
QVERIFY(content.contains("빠른"));        // Korean
```

## Font Requirements

For proper character display, install fonts with wide Unicode coverage:

**Ubuntu/Debian:**
```bash
sudo apt install fonts-noto fonts-noto-cjk fonts-noto-color-emoji
```

**Windows:**
Download from https://fonts.google.com/noto

**macOS:**
```bash
brew tap homebrew/cask-fonts
brew install --cask font-noto-sans font-noto-sans-cjk
```

## Troubleshooting

### Test Files Missing

```bash
cd tests/testfiles
./download_encoding_tests.sh
./create_bom_files.sh
```

### BOM Markers Incorrect

Regenerate files:
```bash
cd tests/testfiles
./create_bom_files.sh
./verify_test_setup.sh  # Verify correctness
```

### Tests Fail to Build

Ensure Qt6 is installed:
```bash
# Ubuntu/Debian
sudo apt install qt6-base-dev

# Check Qt version
qmake6 --version
```

### Tests Fail to Run

Check test files are copied to build directory:
```bash
ls build/debug/testfiles/
# Should show encoding-tests/ and utf8-tests/
```

## Documentation

- **tests/UI_TEST_COVERAGE.md** - Complete UI test coverage requirements
- **tests/UI_TEST_ANALYSIS.md** - Test coverage analysis and recommendations
- **tests/testfiles/README.md** - Detailed test file documentation
- **tests/ENHANCEMENT_SUMMARY.md** - Complete implementation summary
- **tests/PDF_TESTING.md** - PDF printing test strategy
- **IMPLEMENTATION_COMPLETE.md** - Project completion summary

## Support

For issues or questions:
1. Check verification script: `./tests/verify_test_setup.sh`
2. Review test documentation: `tests/testfiles/README.md`
3. Check test output: Run tests with `-v` for verbose output

## Contributing

To add new encoding tests:

1. Add test file to `tests/testfiles/encoding-tests/` or `utf8-tests/`
2. Add test method to `tests/smoke/MainWindowSmokeTests.h`
3. Implement test in `tests/smoke/smoke.cpp`
4. Add test name to `tests/CMakeLists.txt`
5. Document in `tests/testfiles/README.md`
6. Run `./verify_test_setup.sh` to validate

## Quick Reference

| File | Purpose |
|------|---------|
| `verify_test_setup.sh` | Validate test configuration |
| `download_encoding_tests.sh` | Generate UTF-8 test files |
| `create_bom_files.sh` | Generate BOM test files |
| `testfiles/README.md` | Test file documentation |
| `ENHANCEMENT_SUMMARY.md` | Implementation details |

## Next Steps

1. ✅ Verify setup: `./verify_test_setup.sh`
2. ✅ Build project: `cmake --build build/debug`
3. ✅ Run tests: `ctest --test-dir build/debug`
4. 📖 Read docs: `tests/testfiles/README.md`

---

**Need Help?** See `tests/ENHANCEMENT_SUMMARY.md` for detailed implementation notes.
