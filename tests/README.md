# GnotePad Testing Guide

This directory contains the test suite for GnotePad, including smoke tests, encoding tests, and command-line parsing tests.

## Quick Start

```bash
# Build the project
cmake --build build/debug

# Run all tests
ctest --test-dir build/debug

# Run specific tests
ctest --test-dir build/debug -R "encoding|smoke|cmdline"

# Run with verbose output
ctest --test-dir build/debug -V
```

## Test Structure

```
tests/
├── smoke/                    # Smoke tests for core functionality
│   ├── MainWindowSmokeTests.h
│   └── smoke.cpp
├── cmdline/                  # Command-line argument parsing tests
│   └── cmdline.cpp
├── testfiles/                # Test data files
│   ├── encoding-tests/       # Files with various encodings and BOMs
│   ├── utf8-tests/           # UTF-8 multilingual and Unicode test files
│   ├── create_bom_files.sh   # Script to generate BOM test files
│   ├── download_encoding_tests.sh  # Script to generate UTF-8 test files
│   └── *.htm                 # Legacy HTML test files
└── CMakeLists.txt            # Test build configuration
```

## Test Categories

### Smoke Tests

The `GnotePadSmoke` suite verifies core application functionality:

- **Application Lifecycle:** Launch, minimize, maximize, close
- **File Operations:** Open, save, save as with various encodings
- **BOM Detection:** UTF-8, UTF-16 LE, UTF-16 BE with/without BOM
- **Encoding Conversions:** Round-trip tests for all encoding combinations
- **Content Preservation:** Multilingual text, Unicode characters, special characters
- **Performance:** Large file handling (60KB+)

**Run smoke tests:**
```bash
ctest --test-dir build/debug -R smoke
```

### Encoding Tests

Comprehensive tests for document encoding, BOM detection, and round-trip conversions:

**BOM Detection Tests (3):**
1. `testUtf8BomDetection` - UTF-8 BOM presence/absence detection
2. `testUtf16LEBomDetection` - UTF-16 LE BOM detection
3. `testUtf16BEBomDetection` - UTF-16 BE BOM detection

**Content Preservation Tests (2):**
4. `testMultilingualContent` - 14+ language scripts (Latin, Cyrillic, Greek, CJK, Arabic, Hebrew, Thai, Hindi)
5. `testUnicodeCharacters` - Mathematical symbols, currency, arrows, emoji

**Round-Trip Encoding Tests (4):**
6. `testEncodingRoundTripUtf8ToBom` - UTF-8 ↔ UTF-8 with BOM
7. `testEncodingRoundTripUtf16Variants` - UTF-8 ↔ UTF-16 LE ↔ UTF-16 BE
8. `testLargeFileEncodingRoundTrip` - Large file (60KB+) encoding conversions
9. `testMixedContentPreservation` - Complex encoding chain validation

**Run encoding tests:**
```bash
ctest --test-dir build/debug -R "Utf8Bom|Utf16|Multilingual|Unicode|RoundTrip"
```

### Command-Line Tests

Tests for command-line argument parsing:

- `testQuitAfterInitParsing()` - Verifies `--quit-after-init` flag parsing
- `testHeadlessSmokeParsing()` - Verifies `--headless-smoke` alias parsing
- `testNoFlagsParsing()` - Verifies normal operation without flags
- `testQuitAfterInitBehavior()` - Integration test verifying option aliases work

**Run command-line tests:**
```bash
ctest --test-dir build/debug -R cmdline
```

## Test Files

### Encoding Test Files (`testfiles/encoding-tests/`)

Files with various encodings and BOM markers:

- `utf8_with_bom.txt` - UTF-8 with BOM (EF BB BF)
- `utf8_no_bom.txt` - UTF-8 without BOM
- `utf16le_with_bom.txt` - UTF-16 Little Endian with BOM (FF FE)
- `utf16be_with_bom.txt` - UTF-16 Big Endian with BOM (FE FF)
- `mixed_content.txt` - Multilingual content for stress testing

### UTF-8 Test Files (`testfiles/utf8-tests/`)

Diverse Unicode content for comprehensive testing:

- `multilingual.txt` - Text in 14 languages (English, Spanish, French, German, Italian, Russian, Greek, Chinese, Japanese, Korean, Arabic, Hebrew, Thai, Hindi)
- `unicode_chars.txt` - Mathematical symbols, currency, arrows, Greek letters, emoji, box drawing, diacritical marks
- `special_chars.txt` - Zero-width characters, direction marks, special spaces, combining characters
- `line_endings_lf.txt` - Unix-style line endings (LF)
- `line_endings_crlf.txt` - Windows-style line endings (CR+LF)
- `large_multilingual.txt` - Large file (~60KB) with repeated multilingual content

### Regenerating Test Files

Test files can be regenerated using the provided scripts:

```bash
cd tests/testfiles

# Generate UTF-8 multilingual test files
./download_encoding_tests.sh

# Generate files with various BOM markers
./create_bom_files.sh
```

### Verifying Test Files

Check BOM markers and file integrity:

```bash
cd tests/testfiles

# Verify UTF-8 BOM (should show: ef bb bf)
hexdump -C encoding-tests/utf8_with_bom.txt | head -1

# Verify UTF-16 LE BOM (should show: ff fe)
hexdump -C encoding-tests/utf16le_with_bom.txt | head -1

# Verify UTF-16 BE BOM (should show: fe ff)
hexdump -C encoding-tests/utf16be_with_bom.txt | head -1

# Check large file size (should be > 50KB)
ls -lh utf8-tests/large_multilingual.txt
```

## Test Coverage

### Encodings Tested ✅
- UTF-8 (with and without BOM)
- UTF-16 Little Endian (with BOM)
- UTF-16 Big Endian (with BOM)

### Character Sets Tested ✅
- Latin scripts (Western European languages)
- Cyrillic (Russian)
- Greek
- CJK (Chinese, Japanese, Korean)
- Arabic (right-to-left script)
- Hebrew (right-to-left script)
- Thai
- Devanagari (Hindi)
- Mathematical symbols (∀∃∄∅∆∇)
- Currency symbols (€₹₽₿)
- Arrows and box drawing
- Emoji (😀😃😄)
- Combining characters and diacritics

### Round-Trip Conversions Tested ✅
- UTF-8 → UTF-8 with BOM → UTF-8
- UTF-8 → UTF-16 LE → UTF-8
- UTF-8 → UTF-16 BE → UTF-8
- UTF-16 LE → UTF-16 BE → UTF-8
- All conversions on large files (60KB+)

### Edge Cases Tested ✅
- Large files (60KB+) with encoding conversions
- Mixed-script content (multiple languages in one file)
- Zero-width characters and special spaces
- Different line ending styles (LF vs CRLF)

## Font Requirements

For proper display and rendering of all test characters, install fonts with wide Unicode coverage:

**Ubuntu/Debian:**
```bash
sudo apt install fonts-noto fonts-noto-cjk fonts-noto-color-emoji fonts-dejavu
```

**Windows:**
Download Noto fonts from https://fonts.google.com/noto

**macOS:**
```bash
brew tap homebrew/cask-fonts
brew install --cask font-noto-sans font-noto-sans-mono font-noto-sans-cjk
```

## Adding New Tests

When adding new tests:

1. Add test file to `testfiles/encoding-tests/` or `utf8-tests/`
2. Add test method declaration to `smoke/MainWindowSmokeTests.h`
3. Implement test in `smoke/smoke.cpp`
4. Add test function name to `CMakeLists.txt`
5. Run tests to verify: `ctest --test-dir build/debug`
6. Update this documentation

## Running Tests in CI/Headless Environments

For headless environments (CI servers, Docker containers):

```bash
# Set Qt to use offscreen platform
export QT_QPA_PLATFORM=offscreen

# Run tests
ctest --test-dir build/debug

# Run application in headless mode
./build/debug/GnotePad --quit-after-init
```

## Future Enhancements

### PDF Export Tests (Not Yet Implemented)

GnotePad currently uses `QPrintPreviewDialog` with native printing. Future enhancements could include:

1. **Add Headless PDF Export API:**
   ```cpp
   bool exportToPdf(const QString& pdfPath, 
                    const QString& documentName,
                    bool includeLineNumbers);
   ```

2. **Implement PDF Content Validation:**
   - Use Poppler-Qt or similar library for PDF text extraction
   - Compare extracted text with source content
   - Verify character preservation across encodings

3. **Visual Regression Testing:**
   - Render PDFs to images
   - Compare rendered output across encodings
   - Detect rendering regressions

**Current approach:** The encoding test suite ensures text content is correctly preserved before printing, which is the foundation for correct PDF output. Manual PDF testing can be performed using the application's print dialog.

### Additional Test Areas

1. **UTF-32** - If added to application support
2. **Legacy Encodings** - ISO-8859-1, Windows-1252 (if needed)
3. **Malformed Encodings** - Invalid byte sequences, partial characters
4. **Very Large Files** - Files > 100MB (performance testing)
5. **Error Handling** - Graceful handling of encoding errors
6. **Performance Benchmarks** - Load/save time, memory usage

## Test Sources

Test files are based on authoritative sources:

1. **UTF-8 Unicode Test Documents**
   - https://github.com/bits/UTF-8-Unicode-Test-Documents

2. **W3C UTF-8 Test Files**
   - https://www.w3.org/2001/06/utf-8-test/

3. **Columbia University UTF-8 Resources**
   - https://www.columbia.edu/~fdc/utf8.html

4. **Humancomp.org Unicode HTML Test Files**
   - https://www.humancomp.org/unichtm/

## Troubleshooting

### Tests Fail to Build

Ensure Qt6 is installed:
```bash
# Ubuntu/Debian
sudo apt install qt6-base-dev qt6-base-dev-tools

# Check Qt version
qmake6 --version
```

### Test Files Missing

Regenerate test files:
```bash
cd tests/testfiles
./download_encoding_tests.sh
./create_bom_files.sh
```

### BOM Markers Incorrect

Verify with hexdump and regenerate if needed:
```bash
cd tests/testfiles
hexdump -C encoding-tests/utf8_with_bom.txt | head -1
./create_bom_files.sh
```

### Tests Fail in Headless Environment

Set the Qt platform:
```bash
export QT_QPA_PLATFORM=offscreen
ctest --test-dir build/debug
```

## Support

For issues or questions:
1. Review this documentation
2. Check test output with verbose flag: `ctest --test-dir build/debug -V`
3. Open a GitHub issue with test output and environment details
