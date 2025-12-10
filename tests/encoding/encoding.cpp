#include "EncodingEdgeCasesTests.h"
#include "ui/MainWindow.h"
#include "ui/TextEditor.h"

#include <QtCore/QByteArray>
#include <QtCore/QDir>
#include <QtCore/QFile>
#include <QtCore/QFileInfo>
#include <QtCore/QStandardPaths>
#include <QtCore/QTemporaryDir>
#include <QtTest/QTest>
#include <QtWidgets/QApplication>

#include <QStringConverter>

using namespace GnotePad::ui;

EncodingEdgeCasesTests::EncodingEdgeCasesTests(QObject* parent) : QObject(parent)
{
}

void EncodingEdgeCasesTests::initTestCase()
{
    QStandardPaths::setTestModeEnabled(true);
}

QString EncodingEdgeCasesTests::resolveTestFile(const QString& name) const
{
    const QString appPath = QCoreApplication::applicationDirPath();
    const QString testFilesPath = appPath + QStringLiteral("/testfiles/") + name;
    if (QFileInfo::exists(testFilesPath))
    {
        return testFilesPath;
    }
    return QString{};
}

void EncodingEdgeCasesTests::testEmptyFileEncoding()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    const QString emptyPath = tempDir.filePath(QStringLiteral("empty.txt"));
    QFile emptyFile(emptyPath);
    QVERIFY(emptyFile.open(QIODevice::WriteOnly));
    emptyFile.close();

    MainWindow window;
    QVERIFY(window.testLoadDocument(emptyPath));

    auto* editor = window.editorForTest();
    QVERIFY(editor);
    QVERIFY(editor->toPlainText().isEmpty());

    // Default encoding should be UTF-8
    QCOMPARE(window.currentEncodingForTest(), QStringConverter::Utf8);
    QVERIFY(!window.currentBomForTest());
}

void EncodingEdgeCasesTests::testSingleByteFile()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    const QString singleBytePath = tempDir.filePath(QStringLiteral("single.txt"));
    QFile singleByteFile(singleBytePath);
    QVERIFY(singleByteFile.open(QIODevice::WriteOnly));
    singleByteFile.write("A");
    singleByteFile.close();

    MainWindow window;
    QVERIFY(window.testLoadDocument(singleBytePath));

    auto* editor = window.editorForTest();
    QVERIFY(editor);
    QCOMPARE(editor->toPlainText(), QStringLiteral("A"));

    QCOMPARE(window.currentEncodingForTest(), QStringConverter::Utf8);
}

void EncodingEdgeCasesTests::testLargeUtf8File()
{
    // Test with a large file to ensure no performance issues
    const QString largePath = resolveTestFile(QStringLiteral("ulysses8.htm"));
    if (largePath.isEmpty())
    {
        QSKIP("ulysses8.htm not found in testfiles directory");
    }

    MainWindow window;
    QVERIFY(window.testLoadDocument(largePath));

    auto* editor = window.editorForTest();
    QVERIFY(editor);
    QVERIFY(!editor->toPlainText().isEmpty());

    // Verify encoding was detected
    QCOMPARE(window.currentEncodingForTest(), QStringConverter::Utf8);

    // Save and reload to verify round-trip
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());
    const QString savePath = tempDir.filePath(QStringLiteral("large_roundtrip.txt"));

    const QString originalContent = editor->toPlainText();
    QVERIFY(window.testSaveDocument(savePath));

    editor->clear();
    QVERIFY(window.testLoadDocument(savePath));
    QCOMPARE(editor->toPlainText(), originalContent);
}

void EncodingEdgeCasesTests::testMixedLineEndings()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    const QString mixedPath = tempDir.filePath(QStringLiteral("mixed_endings.txt"));
    QFile mixedFile(mixedPath);
    QVERIFY(mixedFile.open(QIODevice::WriteOnly));

    // Mix of CRLF, LF, and CR line endings
    mixedFile.write("Line 1\r\n"); // Windows
    mixedFile.write("Line 2\n");   // Unix
    mixedFile.write("Line 3\r");   // Old Mac
    mixedFile.write("Line 4\r\n"); // Windows again
    mixedFile.close();

    MainWindow window;
    QVERIFY(window.testLoadDocument(mixedPath));

    auto* editor = window.editorForTest();
    QVERIFY(editor);
    const QString content = editor->toPlainText();

    // Verify content was loaded (exact line ending handling depends on Qt)
    QVERIFY(content.contains(QStringLiteral("Line 1")));
    QVERIFY(content.contains(QStringLiteral("Line 2")));
    QVERIFY(content.contains(QStringLiteral("Line 3")));
    QVERIFY(content.contains(QStringLiteral("Line 4")));
}

void EncodingEdgeCasesTests::testTrailingNewlines()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    const QString trailingPath = tempDir.filePath(QStringLiteral("trailing.txt"));
    QFile trailingFile(trailingPath);
    QVERIFY(trailingFile.open(QIODevice::WriteOnly));
    trailingFile.write("Content\n\n\n\n");
    trailingFile.close();

    MainWindow window;
    QVERIFY(window.testLoadDocument(trailingPath));

    auto* editor = window.editorForTest();
    QVERIFY(editor);
    const QString content = editor->toPlainText();
    QVERIFY(content.contains(QStringLiteral("Content")));

    // Save and reload to verify preservation
    const QString savePath = tempDir.filePath(QStringLiteral("trailing_saved.txt"));
    QVERIFY(window.testSaveDocument(savePath));

    editor->clear();
    QVERIFY(window.testLoadDocument(savePath));
    QCOMPARE(editor->toPlainText(), content);
}

void EncodingEdgeCasesTests::testNullBytesHandling()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    const QString nullPath = tempDir.filePath(QStringLiteral("nullbytes.txt"));
    QFile nullFile(nullPath);
    QVERIFY(nullFile.open(QIODevice::WriteOnly));

    // Write text with embedded null bytes
    QByteArray data = "Before";
    data.append('\0');
    data.append("After");
    nullFile.write(data);
    nullFile.close();

    MainWindow window;
    // Loading might fail or succeed depending on implementation
    // The important thing is it doesn't crash
    window.testLoadDocument(nullPath);

    auto* editor = window.editorForTest();
    QVERIFY(editor);
    // Content handling of null bytes is implementation-defined
}

void EncodingEdgeCasesTests::testIncompleteUtf8Sequences()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    const QString incompletePath = tempDir.filePath(QStringLiteral("incomplete_utf8.txt"));
    QFile incompleteFile(incompletePath);
    QVERIFY(incompleteFile.open(QIODevice::WriteOnly));

    // Write valid UTF-8 followed by incomplete sequence
    incompleteFile.write("Valid text ");
    incompleteFile.write("\xC3"); // Start of 2-byte UTF-8 but truncated
    incompleteFile.close();

    MainWindow window;
    // Should handle gracefully, possibly with replacement character
    bool loaded = window.testLoadDocument(incompletePath);

    auto* editor = window.editorForTest();
    QVERIFY(editor);

    if (loaded)
    {
        const QString content = editor->toPlainText();
        QVERIFY(content.contains(QStringLiteral("Valid text")));
    }
}

void EncodingEdgeCasesTests::testBomWithoutContent()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    const QString bomOnlyPath = tempDir.filePath(QStringLiteral("bom_only.txt"));
    QFile bomOnlyFile(bomOnlyPath);
    QVERIFY(bomOnlyFile.open(QIODevice::WriteOnly));

    // Write UTF-8 BOM only, no content
    bomOnlyFile.write(QByteArray::fromHex("efbbbf"));
    bomOnlyFile.close();

    MainWindow window;
    QVERIFY(window.testLoadDocument(bomOnlyPath));

    auto* editor = window.editorForTest();
    QVERIFY(editor);

    // Should detect BOM even with no content
    QCOMPARE(window.currentEncodingForTest(), QStringConverter::Utf8);
    QVERIFY(window.currentBomForTest());

    // Content should be empty (BOM is not content)
    QVERIFY(editor->toPlainText().isEmpty());
}

void EncodingEdgeCasesTests::testMultipleBomMarkers()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    const QString multiPath = tempDir.filePath(QStringLiteral("multi_bom.txt"));
    QFile multiFile(multiPath);
    QVERIFY(multiFile.open(QIODevice::WriteOnly));

    // Write UTF-8 BOM twice (unusual but should handle)
    multiFile.write(QByteArray::fromHex("efbbbf"));
    multiFile.write(QByteArray::fromHex("efbbbf"));
    multiFile.write("Content");
    multiFile.close();

    MainWindow window;
    bool loaded = window.testLoadDocument(multiPath);

    auto* editor = window.editorForTest();
    QVERIFY(editor);

    if (loaded)
    {
        // Should detect encoding, handling of multiple BOMs is implementation-defined
        QCOMPARE(window.currentEncodingForTest(), QStringConverter::Utf8);
    }
}

void EncodingEdgeCasesTests::testEncodingConversionErrors()
{
    // Test converting between incompatible encodings
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    MainWindow window;
    auto* editor = window.editorForTest();
    QVERIFY(editor);

    // Set content with characters that might not convert well
    editor->setPlainText(QStringLiteral("Test content with Unicode: 你好世界"));

    const QString utf8Path = tempDir.filePath(QStringLiteral("test_utf8.txt"));
    QVERIFY(window.testSaveDocumentWithEncoding(utf8Path, QStringConverter::Utf8, false));

    // Reload and verify content preserved
    editor->clear();
    QVERIFY(window.testLoadDocument(utf8Path));
    QVERIFY(editor->toPlainText().contains(QStringLiteral("你好世界")));

    // Save as UTF-16
    const QString utf16Path = tempDir.filePath(QStringLiteral("test_utf16.txt"));
    QVERIFY(window.testSaveDocumentWithEncoding(utf16Path, QStringConverter::Utf16LE, true));

    // Reload and verify
    editor->clear();
    QVERIFY(window.testLoadDocument(utf16Path));
    QVERIFY(editor->toPlainText().contains(QStringLiteral("你好世界")));
}

void EncodingEdgeCasesTests::testUnsupportedEncoding()
{
    // Test that unsupported encoding scenarios are handled gracefully
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    const QString testPath = tempDir.filePath(QStringLiteral("test.txt"));
    QFile testFile(testPath);
    QVERIFY(testFile.open(QIODevice::WriteOnly));
    testFile.write("Simple ASCII text");
    testFile.close();

    MainWindow window;
    QVERIFY(window.testLoadDocument(testPath));

    auto* editor = window.editorForTest();
    QVERIFY(editor);

    // Should default to UTF-8 for unknown/ambiguous files
    QCOMPARE(window.currentEncodingForTest(), QStringConverter::Utf8);
    QCOMPARE(editor->toPlainText(), QStringLiteral("Simple ASCII text"));
}

int main(int argc, char** argv)
{
    QApplication app(argc, argv);
    EncodingEdgeCasesTests tc;
    return QTest::qExec(&tc, argc, argv);
}
