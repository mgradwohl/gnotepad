#!/bin/bash
# Download UTF-8 encoding test files from various sources

# Create subdirectories for organized test files
mkdir -p utf8-tests
mkdir -p w3c-tests
mkdir -p columbia-tests

# From https://github.com/bits/UTF-8-Unicode-Test-Documents (we'll create our own test files)
# These are example multilingual text files
cat > utf8-tests/multilingual.txt << 'MULTIEOF'
English: The quick brown fox jumps over the lazy dog
Spanish: El rápido zorro marrón salta sobre el perro perezoso
French: Le rapide renard brun saute par-dessus le chien paresseux
German: Der schnelle braune Fuchs springt über den faulen Hund
Italian: La volpe marrone veloce salta sopra il cane pigro
Russian: Быстрая коричневая лиса прыгает через ленивую собаку
Greek: Η γρήγορη καφέ αλεπού πηδάει πάνω από το τεμπέλικο σκυλί
Chinese: 敏捷的棕色狐狸跳过懒狗
Japanese: 素早い茶色のキツネが怠け者の犬を飛び越える
Korean: 빠른 갈색 여우가 게으른 개를 뛰어 넘습니다
Arabic: الثعلب البني السريع يقفز فوق الكلب الكسول
Hebrew: השועל החום המהיר קופץ מעל הכלב העצלן
Thai: จิ้งจอกน้ำตาลที่รวดเร็วกระโดดข้ามสุนัขขี้เกียจ
Hindi: तेज भूरी लोमड़ी आलसी कुत्ते के ऊपर कूदती है
MULTIEOF

# Create a file with various Unicode characters and emoji
cat > utf8-tests/unicode_chars.txt << 'UNIEOF'
Mathematical Symbols: ∀∃∄∅∆∇∈∉∊∋∌∍∎∏∐∑−∓∔∕∖∗∘∙√∛∜∝∞∟∠∡∢∣∤∥
Currency Symbols: $¢£¤¥₠₡₢₣₤₥₦₧₨₩₪₫€₭₮₯₰₱₲₳₴₵₶₷₸₹₺₻₼₽₾₿
Arrows: ←↑→↓↔↕↖↗↘↙↚↛↜↝↞↟↠↡↢↣↤↥↦↧↨↩↪↫↬↭↮↯
Greek Letters: ΑΒΓΔΕΖΗΘΙΚΛΜΝΞΟΠΡΣΤΥΦΧΨΩαβγδεζηθικλμνξοπρστυφχψω
Emoji: 😀😃😄😁😆😅🤣😂🙂🙃😉😊😇🥰😍🤩😘😗☺️😚😙🥲
Box Drawing: ┌┬┐├┼┤└┴┘─│╔╦╗╠╬╣╚╩╝═║
Diacritics: áàâäãåāăąćĉċčďđéèêëēĕėęěĝğġģĥħíìîïĩīĭįıĵķĺļľŀłńņň
UNIEOF

# Create a file with zero-width and special characters
cat > utf8-tests/special_chars.txt << 'SPECIALEOF'
Zero-width characters:
- Zero width space: A​B (between A and B)
- Zero width non-joiner: A‌B
- Zero width joiner: A‍B
- Byte order mark: ﻿(at start)

Direction marks:
- Left-to-right mark: A‎B
- Right-to-left mark: A‏B

Special spaces:
- Non-breaking space: A B
- Em space: A B
- En space: A B
- Thin space: A B

Combining characters:
- e + combining acute: é
- n + combining tilde: ñ
- a + combining ring: å
SPECIALEOF

# Create files with different line endings
cat > utf8-tests/line_endings_lf.txt << 'LFEOF'
Line 1 with LF
Line 2 with LF
Line 3 with LF
LFEOF

cat > utf8-tests/line_endings_crlf.txt << 'CRLFEOF'
Line 1 with CRLF
Line 2 with CRLF
Line 3 with CRLF
CRLFEOF
unix2dos utf8-tests/line_endings_crlf.txt 2>/dev/null || (printf "Line 1 with CRLF\r\nLine 2 with CRLF\r\nLine 3 with CRLF\r\n" > utf8-tests/line_endings_crlf.txt)

# Create a large file for performance testing
cat > utf8-tests/large_multilingual.txt << 'LARGEEOF'
=== Large Multilingual Document ===

LARGEEOF

# Add repeated multilingual content
for i in {1..100}; do
  cat >> utf8-tests/large_multilingual.txt << 'REPEATEOF'
English text with common words: the, and, or, but, if, then, else, while, for, with, without
Español: con, sin, para, por, de, del, desde, hasta, cuando, donde, porque, aunque
Français: avec, sans, pour, par, de, du, depuis, jusqu'à, quand, où, parce que, bien que
Deutsch: mit, ohne, für, durch, von, vom, seit, bis, wenn, wo, weil, obwohl
中文: 的, 了, 在, 是, 我, 有, 和, 人, 这, 中, 大, 来, 上, 国, 个
日本語: の, に, は, を, た, が, で, て, と, し, れ, さ, ある, いる, も
한국어: 의, 가, 이, 은, 들, 는, 좀, 잘, 되, 과, 도, 를, 으로, 자, 에

REPEATEOF
done

echo "UTF-8 test files created successfully!"
echo "Files created in:"
echo "  - utf8-tests/multilingual.txt"
echo "  - utf8-tests/unicode_chars.txt"
echo "  - utf8-tests/special_chars.txt"
echo "  - utf8-tests/line_endings_lf.txt"
echo "  - utf8-tests/line_endings_crlf.txt"
echo "  - utf8-tests/large_multilingual.txt"
