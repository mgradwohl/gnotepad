# PDF Printing and Encoding Tests

## Current Status

GnotePad supports printing through Qt's `QPrintPreviewDialog` using `QPrinter::NativeFormat`. The application does not currently have a direct "Export to PDF" feature, though Qt's QPrinter can output to PDF format when configured with `QPrinter::PdfFormat`.

## Testing Approach for PDF Output with Different Encodings

### Why PDF Testing is Complex

1. **Native Print Dialog**: The current implementation uses `QPrintPreviewDialog`, which requires UI interaction
2. **No Direct PDF Export**: The app doesn't expose a programmatic PDF export API
3. **Platform-Specific Rendering**: PDF output can vary by platform and installed fonts
4. **Binary Comparison Challenges**: Comparing PDFs byte-by-byte is unreliable due to metadata (timestamps, etc.)

### Recommended Testing Strategy

#### 1. Content Preservation Tests (✅ Implemented)

The encoding test suite already validates that:
- Text content survives round-trip encoding conversions
- Unicode characters are preserved across UTF-8, UTF-16 LE, and UTF-16 BE
- BOM markers are correctly handled
- Multilingual content remains intact

These tests ensure the **source text** is correct before printing, which is the foundation for correct PDF output.

#### 2. Future PDF Export Tests (Not Yet Implemented)

To fully test PDF generation with different encodings, we would need to:

**Option A: Add Headless PDF Export**
```cpp
// Add to MainWindow or PrintSupport
bool exportToPdf(const QString& pdfPath, 
                 const QString& documentName,
                 bool includeLineNumbers)
{
    QPrinter printer(QPrinter::HighResolution);
    printer.setOutputFormat(QPrinter::PdfFormat);
    printer.setOutputFileName(pdfPath);
    
    // Use existing renderDocument logic
    PrintSupport::renderDocument(editor, &printer, documentName, includeLineNumbers);
    return true;
}
```

**Option B: Test PDF Content Extraction**
```cpp
// Pseudo-code for PDF validation
void testPdfEncodingPreservation()
{
    // 1. Load multilingual test file
    window.testLoadDocument("utf8-tests/multilingual.txt");
    
    // 2. Export to PDF
    QTemporaryDir tempDir;
    QString pdfPath = tempDir.filePath("test_utf8.pdf");
    window.exportToPdf(pdfPath, "Multilingual Test", false);
    
    // 3. Extract text from PDF (requires external library like Poppler)
    QString extractedText = extractTextFromPdf(pdfPath);
    
    // 4. Compare with original
    QVERIFY(extractedText.contains("Быстрая")); // Russian
    QVERIFY(extractedText.contains("敏捷的"));   // Chinese
    QVERIFY(extractedText.contains("素早い"));   // Japanese
}
```

**Option C: Visual Regression Testing**
```cpp
// Render PDF pages to images and compare
void testPdfRendering()
{
    // 1. Export test files to PDF with different encodings
    exportWithEncoding("multilingual.txt", QStringConverter::Utf8);
    exportWithEncoding("multilingual.txt", QStringConverter::Utf16LE);
    
    // 2. Render PDFs to PNG images
    QImage img1 = renderPdfToImage("test_utf8.pdf");
    QImage img2 = renderPdfToImage("test_utf16le.pdf");
    
    // 3. Compare images (allowing for minor rendering differences)
    QVERIFY(imagesSimilar(img1, img2, 0.99)); // 99% similarity threshold
}
```

### Current Test Coverage

The test suite currently covers:

✅ **Encoding Detection and Preservation**
- UTF-8 with/without BOM
- UTF-16 LE and BE with BOM
- Round-trip encoding conversions

✅ **Character Set Preservation**
- Multilingual text (14+ languages)
- Unicode symbols, emoji, mathematical operators
- Special characters and combining marks

✅ **Performance and Edge Cases**
- Large files (60KB+)
- Mixed content (multiple scripts)
- Line ending handling

❌ **Not Yet Covered: PDF Output**
- Actual PDF generation with different encodings
- Font embedding for Unicode characters
- PDF text extraction and validation

### Manual PDF Testing Procedure

Until automated PDF tests are implemented, use this manual procedure:

1. **Open Test File**
   ```
   File → Open → tests/testfiles/utf8-tests/multilingual.txt
   ```

2. **Verify Display**
   - Check that all languages display correctly
   - Verify Unicode characters render properly
   - Ensure no replacement characters (�) appear

3. **Print Preview**
   ```
   File → Print (Ctrl+P)
   ```
   - In print preview, verify all characters are visible
   - Check that fonts are properly selected
   - Ensure no character substitution occurs

4. **Export to PDF**
   - In print dialog, select "Print to PDF" (platform-specific)
   - Save PDF with descriptive name (e.g., "multilingual_utf8.pdf")

5. **Validate PDF**
   - Open PDF in viewer (Adobe Reader, Evince, Preview)
   - Verify all characters are readable
   - Check that character shapes match original
   - Ensure no font substitution warnings

6. **Test Different Encodings**
   - Repeat steps 1-5 with same content saved in:
     - UTF-8 with BOM
     - UTF-16 LE
     - UTF-16 BE
   - Compare PDFs visually for consistency

### Required Fonts for Complete Coverage

For comprehensive multilingual PDF output, ensure these fonts are installed:

**System Fonts**
- **Noto Sans** / **Noto Sans Mono** - Wide Unicode coverage
- **Noto CJK** (Chinese, Japanese, Korean)
- **Noto Emoji** - Emoji and symbols
- **DejaVu Sans Mono** - Good fallback font

**Installation on Ubuntu/Debian**
```bash
sudo apt install fonts-noto fonts-noto-cjk fonts-noto-color-emoji fonts-dejavu
```

**Installation on Windows**
- Download from https://fonts.google.com/noto
- Install Noto Sans, Noto Sans Mono, Noto CJK

**Installation on macOS**
```bash
brew tap homebrew/cask-fonts
brew install --cask font-noto-sans font-noto-sans-mono font-noto-sans-cjk
```

### Future Enhancements

To fully automate PDF testing:

1. **Add PDF Export API**
   - Expose headless PDF export function
   - Support encoding parameter
   - Return success/failure status

2. **Integrate PDF Library**
   - Add Poppler-Qt for PDF text extraction
   - Or use PyMuPDF/pdftotext for validation
   - Compare extracted text with source

3. **Font Metrics Testing**
   - Verify correct font selection for different scripts
   - Check font embedding in generated PDFs
   - Validate glyph coverage

4. **Visual Regression**
   - Render PDFs to images
   - Compare rendered output across encodings
   - Detect rendering regressions

## References

- Qt QPrinter documentation: https://doc.qt.io/qt-6/qprinter.html
- Poppler-Qt PDF library: https://poppler.freedesktop.org/
- Font coverage: https://fonts.google.com/noto/specimen/Noto+Sans
- PDF/A standard for archival: https://www.iso.org/standard/57229.html

## Summary

While direct PDF encoding tests are not yet implemented, the comprehensive encoding test suite provides confidence that:
1. Text content is correctly loaded and preserved
2. Encoding conversions maintain data integrity
3. Unicode characters are handled correctly

These guarantees mean that PDF output will be accurate as long as appropriate fonts are available, which is a system/user configuration concern rather than an application bug.

Future work should focus on adding headless PDF export and automated validation, but the current test coverage provides a strong foundation for encoding correctness.
