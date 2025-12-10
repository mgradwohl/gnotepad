#!/bin/bash
# Create files with different encodings and BOM markers

mkdir -p encoding-tests

# Create UTF-8 with BOM
printf '\xEF\xBB\xBF' > encoding-tests/utf8_with_bom.txt
cat >> encoding-tests/utf8_with_bom.txt << 'BOMEOF'
This file has UTF-8 BOM marker
Test special chars: äöü ñ é è ê
Chinese: 你好世界
Japanese: こんにちは世界
BOMEOF

# Create UTF-8 without BOM
cat > encoding-tests/utf8_no_bom.txt << 'NOBOMEOF'
This file has NO BOM marker
Test special chars: äöü ñ é è ê
Chinese: 你好世界
Japanese: こんにちは世界
NOBOMEOF

# Create UTF-16 LE with BOM
printf '\xFF\xFE' > encoding-tests/utf16le_with_bom.txt
if command -v iconv >/dev/null 2>&1; then
    echo -n "This file is UTF-16 LE with BOM" | iconv -f UTF-8 -t UTF-16LE >> encoding-tests/utf16le_with_bom.txt
else
    # Fallback: create UTF-16 LE manually (basic ASCII only)
    printf 'T\x00h\x00i\x00s\x00 \x00f\x00i\x00l\x00e\x00 \x00i\x00s\x00 \x00U\x00T\x00F\x00-\x001\x006\x00 \x00L\x00E\x00 \x00w\x00i\x00t\x00h\x00 \x00B\x00O\x00M\x00' >> encoding-tests/utf16le_with_bom.txt
fi

# Create UTF-16 BE with BOM
printf '\xFE\xFF' > encoding-tests/utf16be_with_bom.txt
if command -v iconv >/dev/null 2>&1; then
    echo -n "This file is UTF-16 BE with BOM" | iconv -f UTF-8 -t UTF-16BE >> encoding-tests/utf16be_with_bom.txt
else
    # Fallback: create UTF-16 BE manually (basic ASCII only)
    printf '\x00T\x00h\x00i\x00s\x00 \x00f\x00i\x00l\x00e\x00 \x00i\x00s\x00 \x00U\x00T\x00F\x00-\x001\x006\x00 \x00B\x00E\x00 \x00w\x00i\x00t\x00h\x00 \x00B\x00O\x00M' >> encoding-tests/utf16be_with_bom.txt
fi

# Create a test file with mixed content for encoding detection
cat > encoding-tests/mixed_content.txt << 'MIXEOF'
English: The quick brown fox
Español: El zorro marrón rápido
Français: Le renard brun rapide
Deutsch: Der schnelle braune Fuchs
中文测试：快速的棕色狐狸
日本語テスト：速い茶色のキツネ
한국어 테스트: 빠른 갈색 여우
Русский: Быстрая коричневая лиса
العربية: الثعلب البني السريع
MIXEOF

echo "Encoding test files created!"
ls -lh encoding-tests/
