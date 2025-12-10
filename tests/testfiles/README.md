# GnotePad Test Files

This directory contains test files for validating GnotePad's encoding support, file I/O operations, and text editor functionality.

## Directory Structure

```
testfiles/
├── README.md                    # This file
├── download_encoding_tests.sh   # Script to generate UTF-8 test files
├── create_bom_files.sh          # Script to generate BOM test files
├── encoding-tests/              # Files with various encodings and BOM markers
├── utf8-tests/                  # UTF-8 encoded multilingual and Unicode test files
├── *.htm                        # Legacy HTML test files from humancomp.org
└── findreplace.txt              # Test file for find/replace operations
```

## Test File Categories

### Encoding Test Files (`encoding-tests/`)

These files test BOM (Byte Order Mark) detection and encoding handling:

- **utf8_with_bom.txt** - UTF-8 file with BOM marker (EF BB BF)
- **utf8_no_bom.txt** - UTF-8 file without BOM marker
- **utf16le_with_bom.txt** - UTF-16 Little Endian with BOM (FF FE)
- **utf16be_with_bom.txt** - UTF-16 Big Endian with BOM (FE FF)
- **mixed_content.txt** - UTF-8 file with content from multiple languages/scripts

**Purpose:** Validate correct BOM detection, encoding identification, and round-trip encoding conversions.

### UTF-8 Test Files (`utf8-tests/`)

These files contain various Unicode characters and multilingual content:

- **multilingual.txt** - Text in 14 different languages including English, Spanish, French, German, Italian, Russian, Greek, Chinese, Japanese, Korean, Arabic, Hebrew, Thai, and Hindi
- **unicode_chars.txt** - Mathematical symbols, currency symbols, arrows, Greek letters, emoji, box drawing characters, and diacritical marks
- **special_chars.txt** - Zero-width characters, direction marks, special spaces, and combining characters
- **line_endings_lf.txt** - File with Unix-style line endings (LF)
- **line_endings_crlf.txt** - File with Windows-style line endings (CR+LF)
- **large_multilingual.txt** - Large file (~60KB) with repeated multilingual content for performance testing

**Purpose:** Test Unicode character handling, multilingual text preservation, and performance with large files.

### Legacy HTML Test Files

From https://www.humancomp.org/unichtm/ (various encodings):

- **sample68.htm** - Sample Korean text (UTF-8)
- **ulysses8.htm** - Selection from Chinese translation of James Joyce's Ulysses (UTF-8)
- **danish8.htm** - Danish text sample (UTF-8)
- **neural8.htm** - Neural network article (UTF-8)
- **calblur8.htm** - CalBlur text (UTF-8)
- **findreplace.txt** - Simple text file for find/replace functionality testing

**Purpose:** Real-world HTML files with diverse character sets for legacy compatibility testing.

## Test Sources

The test files are derived from these authoritative sources:

1. **UTF-8 Unicode Test Documents**
   - Repository: https://github.com/bits/UTF-8-Unicode-Test-Documents
   - Contains various UTF-8 test cases and edge cases

2. **W3C UTF-8 Test Files**
   - URL: https://www.w3.org/2001/06/utf-8-test/
   - Official W3C test files for UTF-8 validation

3. **Columbia University UTF-8 Resources**
   - URL: https://www.columbia.edu/~fdc/utf8.html
   - Comprehensive UTF-8 documentation and test files

4. **Humancomp.org Unicode HTML Test Files**
   - URL: https://www.humancomp.org/unichtm/
   - Real-world HTML files in various languages and encodings

## Creating Test Files

The test files can be regenerated using the provided scripts:

```bash
cd tests/testfiles

# Generate UTF-8 multilingual test files
./download_encoding_tests.sh

# Generate files with various BOM markers
./create_bom_files.sh
```

## Test Coverage

The test files are used to validate:

1. **Encoding Detection**
   - Correct identification of UTF-8, UTF-16 LE, and UTF-16 BE
   - BOM marker detection and preservation
   - Fallback to UTF-8 for files without BOM

2. **Round-trip Encoding**
   - Load file → Change encoding → Save → Reload → Verify content unchanged
   - Test all encoding combinations:
     - UTF-8 ↔ UTF-8 with BOM
     - UTF-8 ↔ UTF-16 LE
     - UTF-8 ↔ UTF-16 BE
     - UTF-16 LE ↔ UTF-16 BE

3. **Character Preservation**
   - Mathematical symbols and special characters
   - Emoji and pictographs
   - Right-to-left text (Arabic, Hebrew)
   - Combining characters and diacritics
   - Zero-width characters

4. **Performance**
   - Large file loading and saving
   - Encoding conversion of large files
   - Memory efficiency with multilingual content

5. **Edge Cases**
   - Mixed content (multiple scripts in one file)
   - Line ending preservation (LF vs CRLF)
   - Files with and without BOM markers

## Font Requirements

For proper display of all test characters, the system should have fonts that support:

- **Latin Extended** - European languages with diacritics
- **Cyrillic** - Russian and other Slavic languages
- **Greek** - Greek alphabet
- **CJK (Chinese, Japanese, Korean)** - East Asian characters
- **Arabic** - Arabic script
- **Hebrew** - Hebrew script
- **Thai** - Thai script
- **Devanagari** - Hindi and Sanskrit
- **Mathematical Symbols** - Unicode math operators
- **Emoji** - Color emoji support

Recommended fonts:
- **Noto Sans** / **Noto Sans Mono** - Comprehensive Unicode coverage
- **Noto CJK** - CJK character support
- **Noto Emoji** - Emoji support
- **DejaVu Sans Mono** - Good fallback with wide character coverage

## Adding New Test Files

When adding new test files:

1. Place them in the appropriate subdirectory
2. Update this README with file description
3. Add corresponding test cases in `tests/smoke/smoke.cpp`
4. Update `tests/CMakeLists.txt` if new test functions are added
5. Ensure files are copied to the build directory (handled by CMake)

## Verification

To verify the integrity of test files:

```bash
# Check BOM markers
hexdump -C encoding-tests/utf8_with_bom.txt | head -1
# Should show: ef bb bf (UTF-8 BOM)

hexdump -C encoding-tests/utf16le_with_bom.txt | head -1
# Should show: ff fe (UTF-16 LE BOM)

hexdump -C encoding-tests/utf16be_with_bom.txt | head -1
# Should show: fe ff (UTF-16 BE BOM)

# Check file sizes
ls -lh utf8-tests/large_multilingual.txt
# Should be > 50KB

# Verify character encoding
file encoding-tests/*.txt utf8-tests/*.txt
```

## Notes

- All test files are committed to the repository for reproducibility
- Test files should not be modified manually; use the generator scripts
- When updating test files, ensure backward compatibility with existing tests
- Large files (> 100KB) should be added sparingly to keep repository size manageable
