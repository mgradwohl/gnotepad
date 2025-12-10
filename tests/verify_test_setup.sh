#!/bin/bash
# Verification script for GnotePad encoding test files
# Run this script to verify test files are correctly set up

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
TESTFILES_DIR="${SCRIPT_DIR}/testfiles"

echo "========================================="
echo "GnotePad Encoding Test Verification"
echo "========================================="
echo ""

# Color codes for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

pass() {
    echo -e "${GREEN}✓${NC} $1"
}

fail() {
    echo -e "${RED}✗${NC} $1"
    exit 1
}

warn() {
    echo -e "${YELLOW}⚠${NC} $1"
}

# 1. Check directory structure
echo "1. Checking directory structure..."
if [ -d "${TESTFILES_DIR}/encoding-tests" ]; then
    pass "encoding-tests directory exists"
else
    fail "encoding-tests directory not found"
fi

if [ -d "${TESTFILES_DIR}/utf8-tests" ]; then
    pass "utf8-tests directory exists"
else
    fail "utf8-tests directory not found"
fi

# 2. Check required encoding test files
echo ""
echo "2. Checking encoding test files..."
ENCODING_FILES=(
    "utf8_with_bom.txt"
    "utf8_no_bom.txt"
    "utf16le_with_bom.txt"
    "utf16be_with_bom.txt"
    "mixed_content.txt"
)

for file in "${ENCODING_FILES[@]}"; do
    if [ -f "${TESTFILES_DIR}/encoding-tests/${file}" ]; then
        pass "${file} exists"
    else
        fail "${file} not found"
    fi
done

# 3. Check required UTF-8 test files
echo ""
echo "3. Checking UTF-8 test files..."
UTF8_FILES=(
    "multilingual.txt"
    "unicode_chars.txt"
    "special_chars.txt"
    "line_endings_lf.txt"
    "line_endings_crlf.txt"
    "large_multilingual.txt"
)

for file in "${UTF8_FILES[@]}"; do
    if [ -f "${TESTFILES_DIR}/utf8-tests/${file}" ]; then
        pass "${file} exists"
    else
        fail "${file} not found"
    fi
done

# 4. Verify BOM markers
echo ""
echo "4. Verifying BOM markers..."

# UTF-8 BOM check
UTF8_BOM=$(hexdump -n 3 -e '3/1 "%02x"' "${TESTFILES_DIR}/encoding-tests/utf8_with_bom.txt")
if [ "$UTF8_BOM" = "efbbbf" ]; then
    pass "UTF-8 BOM correct (EF BB BF)"
else
    fail "UTF-8 BOM incorrect: got $UTF8_BOM, expected efbbbf"
fi

# UTF-16 LE BOM check
UTF16LE_BOM=$(hexdump -n 2 -e '2/1 "%02x"' "${TESTFILES_DIR}/encoding-tests/utf16le_with_bom.txt")
if [ "$UTF16LE_BOM" = "fffe" ]; then
    pass "UTF-16 LE BOM correct (FF FE)"
else
    fail "UTF-16 LE BOM incorrect: got $UTF16LE_BOM, expected fffe"
fi

# UTF-16 BE BOM check
UTF16BE_BOM=$(hexdump -n 2 -e '2/1 "%02x"' "${TESTFILES_DIR}/encoding-tests/utf16be_with_bom.txt")
if [ "$UTF16BE_BOM" = "feff" ]; then
    pass "UTF-16 BE BOM correct (FE FF)"
else
    fail "UTF-16 BE BOM incorrect: got $UTF16BE_BOM, expected feff"
fi

# UTF-8 no BOM check
UTF8_NO_BOM=$(hexdump -n 3 -e '3/1 "%02x"' "${TESTFILES_DIR}/encoding-tests/utf8_no_bom.txt")
if [ "$UTF8_NO_BOM" != "efbbbf" ] && [ "$UTF8_NO_BOM" != "fffe" ] && [ "$UTF8_NO_BOM" != "feff" ]; then
    pass "UTF-8 without BOM correct (no BOM marker)"
else
    fail "UTF-8 without BOM has unexpected BOM: $UTF8_NO_BOM"
fi

# 5. Verify file sizes
echo ""
echo "5. Checking file sizes..."

LARGE_FILE="${TESTFILES_DIR}/utf8-tests/large_multilingual.txt"
if [ -f "$LARGE_FILE" ]; then
    SIZE=$(stat -f%z "$LARGE_FILE" 2>/dev/null || stat -c%s "$LARGE_FILE" 2>/dev/null)
    if [ "$SIZE" -gt 50000 ]; then
        pass "Large test file is ${SIZE} bytes (> 50KB)"
    else
        warn "Large test file is only ${SIZE} bytes (expected > 50KB)"
    fi
else
    fail "Large test file not found"
fi

# 6. Verify content in multilingual file
echo ""
echo "6. Verifying multilingual content..."

MULTI_FILE="${TESTFILES_DIR}/utf8-tests/multilingual.txt"
if [ -f "$MULTI_FILE" ]; then
    # Check for various languages
    if grep -q "English" "$MULTI_FILE" && \
       grep -q "中文" "$MULTI_FILE" && \
       grep -q "日本語" "$MULTI_FILE" && \
       grep -q "한국어" "$MULTI_FILE"; then
        pass "Multilingual content includes English, Chinese, Japanese, Korean"
    else
        warn "Some expected languages not found in multilingual.txt"
    fi
else
    fail "multilingual.txt not found"
fi

# 7. Verify Unicode characters
echo ""
echo "7. Verifying Unicode characters..."

UNICODE_FILE="${TESTFILES_DIR}/utf8-tests/unicode_chars.txt"
if [ -f "$UNICODE_FILE" ]; then
    # Check for mathematical symbols
    if grep -q "∀∃" "$UNICODE_FILE"; then
        pass "Mathematical symbols found"
    else
        warn "Mathematical symbols not found"
    fi
    
    # Check for emoji
    if grep -q "😀" "$UNICODE_FILE"; then
        pass "Emoji characters found"
    else
        warn "Emoji characters not found"
    fi
    
    # Check for currency
    if grep -q "€" "$UNICODE_FILE"; then
        pass "Currency symbols found"
    else
        warn "Currency symbols not found"
    fi
else
    fail "unicode_chars.txt not found"
fi

# 8. Check script files are executable
echo ""
echo "8. Checking generator scripts..."

if [ -x "${TESTFILES_DIR}/download_encoding_tests.sh" ]; then
    pass "download_encoding_tests.sh is executable"
else
    warn "download_encoding_tests.sh is not executable (run: chmod +x download_encoding_tests.sh)"
fi

if [ -x "${TESTFILES_DIR}/create_bom_files.sh" ]; then
    pass "create_bom_files.sh is executable"
else
    warn "create_bom_files.sh is not executable (run: chmod +x create_bom_files.sh)"
fi

# 9. Count total test files
echo ""
echo "9. Test file summary..."

ENCODING_COUNT=$(find "${TESTFILES_DIR}/encoding-tests" -type f -name "*.txt" | wc -l)
UTF8_COUNT=$(find "${TESTFILES_DIR}/utf8-tests" -type f -name "*.txt" | wc -l)
TOTAL_COUNT=$((ENCODING_COUNT + UTF8_COUNT))

echo "   Encoding test files: ${ENCODING_COUNT}"
echo "   UTF-8 test files: ${UTF8_COUNT}"
echo "   Total test files: ${TOTAL_COUNT}"

if [ "$TOTAL_COUNT" -ge 11 ]; then
    pass "Test file count looks good (${TOTAL_COUNT} files)"
else
    warn "Expected at least 11 test files, found ${TOTAL_COUNT}"
fi

# 10. Check documentation
echo ""
echo "10. Checking documentation..."

if [ -f "${TESTFILES_DIR}/README.md" ]; then
    pass "testfiles/README.md exists"
else
    fail "testfiles/README.md not found"
fi

if [ -f "${SCRIPT_DIR}/ENHANCEMENT_SUMMARY.md" ]; then
    pass "ENHANCEMENT_SUMMARY.md exists"
else
    warn "ENHANCEMENT_SUMMARY.md not found"
fi

if [ -f "${SCRIPT_DIR}/PDF_TESTING.md" ]; then
    pass "PDF_TESTING.md exists"
else
    warn "PDF_TESTING.md not found"
fi

echo ""
echo "========================================="
echo -e "${GREEN}All checks passed!${NC}"
echo "========================================="
echo ""
echo "Test files are correctly configured and ready for use."
echo "Run tests with: ctest --test-dir build/debug"
echo ""
