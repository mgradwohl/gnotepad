#include "PerformanceTests.h"
#include "ui/MainWindow.h"
#include "ui/TextEditor.h"

#include <QtCore/QByteArray>
#include <QtCore/QCoreApplication>
#include <QtCore/QDir>
#include <QtCore/QElapsedTimer>
#include <QtCore/QFile>
#include <QtCore/QFileInfo>
#include <QtCore/QRandomGenerator>
#include <QtCore/QStandardPaths>
#include <QtCore/QStringConverter>
#include <QtCore/QTemporaryDir>
#include <QtGui/QTextCursor>
#include <QtTest/QTest>
#include <QtWidgets/QApplication>
#include <QtWidgets/QScrollBar>

#include <fstream>

#if defined(Q_OS_LINUX)
#include <unistd.h>
#include <cstdio>
#endif

using namespace GnotePad::ui;

// Performance thresholds (in milliseconds)
namespace PerformanceThresholds
{
    constexpr int LOAD_100KB_MS = 1000;   // Max time to load 100KB file
    constexpr int LOAD_500KB_MS = 3000;   // Max time to load 500KB file
    constexpr int LOAD_1MB_MS = 6000;     // Max time to load 1MB file
    constexpr int SAVE_LARGE_MS = 3000;   // Max time to save large file
    constexpr int ENCODING_MS = 2000;     // Max time for encoding conversion
    constexpr int SCROLL_100_LINES_MS = 500; // Max time to scroll 100 lines
    constexpr int FIND_MS = 1000;         // Max time for find operation
    constexpr int REPLACE_MS = 2000;      // Max time for replace operation
    constexpr int UNDO_REDO_MS = 100;     // Max time for single undo/redo
    constexpr int ZOOM_MS = 200;          // Max time for zoom operation
} // namespace PerformanceThresholds

PerformanceTests::PerformanceTests(QObject* parent) : QObject(parent)
{
}

void PerformanceTests::initTestCase()
{
    QStandardPaths::setTestModeEnabled(true);

    // Create test files directory if it doesn't exist
    const QString testFilesPath = QCoreApplication::applicationDirPath() + QStringLiteral("/testfiles/performance");
    QDir dir;
    dir.mkpath(testFilesPath);
}

void PerformanceTests::cleanupTestCase()
{
    // Cleanup is automatic via QTemporaryDir in individual tests
}

QString PerformanceTests::resolveTestFile(const QString& name) const
{
    QDir dir(QCoreApplication::applicationDirPath());
    if (!dir.cd(QStringLiteral("testfiles/performance")))
    {
        return {};
    }

    const QString fullPath = dir.absoluteFilePath(name);
    if (QFileInfo::exists(fullPath))
    {
        return fullPath;
    }
    return {};
}

void PerformanceTests::generateTestFile(const QString& path, qint64 sizeInBytes)
{
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        QFAIL("Failed to create test file");
        return;
    }

    // Generate realistic text content with line breaks
    const QString paragraph = QStringLiteral(
        "Lorem ipsum dolor sit amet, consectetur adipiscing elit. "
        "Sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. "
        "Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris. "
        "This is line %1 of the performance test file.\n");

    qint64 bytesWritten = 0;
    int lineNumber = 0;

    while (bytesWritten < sizeInBytes)
    {
        const QString line = paragraph.arg(lineNumber++);
        const QByteArray lineBytes = line.toUtf8();
        file.write(lineBytes);
        bytesWritten += lineBytes.size();
    }

    file.close();
}

qint64 PerformanceTests::getCurrentMemoryUsage() const
{
#if defined(Q_OS_LINUX)
    // Read from /proc/self/status
    std::ifstream statusFile("/proc/self/status");
    std::string line;
    while (std::getline(statusFile, line))
    {
        if (line.compare(0, 6, "VmRSS:") == 0)
        {
            // Extract the numeric value (in KB) using QString for robust parsing
            const QString qline = QString::fromStdString(line);
            const QStringList parts = qline.split(QChar(' '), Qt::SkipEmptyParts);
            if (parts.size() >= 2)
            {
                bool ok = false;
                const qint64 kb = parts[1].toLongLong(&ok);
                if (ok)
                {
                    return kb * 1024; // Convert KB to bytes
                }
            }
        }
    }
#endif
    return 0; // Not implemented for this platform
}

double PerformanceTests::measureOperationTime(const std::function<void()>& operation) const
{
    QElapsedTimer timer;
    timer.start();
    operation();
    return static_cast<double>(timer.elapsed());
}

void PerformanceTests::testLoadLargeFile100KB()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());
    const QString tempFile = tempDir.filePath(QStringLiteral("test_100kb.txt"));
    generateTestFile(tempFile, 100 * 1024); // 100KB

    MainWindow window;
    QElapsedTimer timer;
    timer.start();

    QVERIFY(window.testLoadDocument(tempFile));

    const qint64 loadTime = timer.elapsed();
    qInfo() << "Load time for 100KB file:" << loadTime << "ms";

    QVERIFY2(loadTime < PerformanceThresholds::LOAD_100KB_MS,
             qPrintable(QString("Load time %1ms exceeds threshold %2ms")
                            .arg(loadTime)
                            .arg(PerformanceThresholds::LOAD_100KB_MS)));

    auto* editor = window.editorForTest();
    QVERIFY(editor);
    QVERIFY(!editor->toPlainText().isEmpty());
    QVERIFY(editor->toPlainText().length() > 100000);
}

void PerformanceTests::testLoadLargeFile500KB()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());
    const QString tempFile = tempDir.filePath(QStringLiteral("test_500kb.txt"));
    generateTestFile(tempFile, 500 * 1024); // 500KB

    MainWindow window;
    QElapsedTimer timer;
    timer.start();

    QVERIFY(window.testLoadDocument(tempFile));

    const qint64 loadTime = timer.elapsed();
    qInfo() << "Load time for 500KB file:" << loadTime << "ms";

    QVERIFY2(loadTime < PerformanceThresholds::LOAD_500KB_MS,
             qPrintable(QString("Load time %1ms exceeds threshold %2ms")
                            .arg(loadTime)
                            .arg(PerformanceThresholds::LOAD_500KB_MS)));

    auto* editor = window.editorForTest();
    QVERIFY(editor);
    QVERIFY(editor->toPlainText().length() > 500000);
}

void PerformanceTests::testLoadLargeFile1MB()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());
    const QString tempFile = tempDir.filePath(QStringLiteral("test_1mb.txt"));
    generateTestFile(tempFile, 1024 * 1024); // 1MB

    MainWindow window;
    QElapsedTimer timer;
    timer.start();

    QVERIFY(window.testLoadDocument(tempFile));

    const qint64 loadTime = timer.elapsed();
    qInfo() << "Load time for 1MB file:" << loadTime << "ms";

    QVERIFY2(loadTime < PerformanceThresholds::LOAD_1MB_MS,
             qPrintable(QString("Load time %1ms exceeds threshold %2ms")
                            .arg(loadTime)
                            .arg(PerformanceThresholds::LOAD_1MB_MS)));

    auto* editor = window.editorForTest();
    QVERIFY(editor);
    QVERIFY(editor->toPlainText().length() > 1000000);
}

void PerformanceTests::testScrollPerformanceLargeFile()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());
    const QString tempFile = tempDir.filePath(QStringLiteral("test_scroll.txt"));
    generateTestFile(tempFile, 200 * 1024); // 200KB

    MainWindow window;
    QVERIFY(window.testLoadDocument(tempFile));

    auto* editor = window.editorForTest();
    QVERIFY(editor);

    auto* scrollBar = editor->verticalScrollBar();
    QVERIFY(scrollBar);

    // Test scrolling through the document
    QElapsedTimer timer;
    timer.start();

    const int initialValue = scrollBar->value();
    const int maxValue = scrollBar->maximum();
    const int scrollSteps = 100;
    const int stepSize = maxValue / scrollSteps;

    for (int i = 0; i < scrollSteps; ++i)
    {
        scrollBar->setValue(initialValue + (i * stepSize));
        QApplication::processEvents();
    }

    const qint64 scrollTime = timer.elapsed();
    qInfo() << "Scroll time for 100 steps:" << scrollTime << "ms";

    QVERIFY2(scrollTime < PerformanceThresholds::SCROLL_100_LINES_MS,
             qPrintable(QString("Scroll time %1ms exceeds threshold %2ms")
                            .arg(scrollTime)
                            .arg(PerformanceThresholds::SCROLL_100_LINES_MS)));
}

void PerformanceTests::testEditPerformanceLargeFile()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());
    const QString tempFile = tempDir.filePath(QStringLiteral("test_edit.txt"));
    generateTestFile(tempFile, 100 * 1024); // 100KB

    MainWindow window;
    QVERIFY(window.testLoadDocument(tempFile));

    auto* editor = window.editorForTest();
    QVERIFY(editor);

    // Test insert performance
    QElapsedTimer timer;
    timer.start();

    editor->moveCursor(QTextCursor::End);
    const QString insertText = QStringLiteral("\nPerformance test insertion.");

    for (int i = 0; i < 100; ++i)
    {
        editor->insertPlainText(insertText);
        QApplication::processEvents();
    }

    const qint64 insertTime = timer.elapsed();
    qInfo() << "Insert time for 100 operations:" << insertTime << "ms";

    // Verify the inserts worked
    QVERIFY(editor->document()->isModified());
    QVERIFY(editor->toPlainText().contains(QStringLiteral("Performance test insertion")));
}

void PerformanceTests::testSaveLargeFile()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());
    const QString tempFile = tempDir.filePath(QStringLiteral("test_save.txt"));
    generateTestFile(tempFile, 500 * 1024); // 500KB

    MainWindow window;
    QVERIFY(window.testLoadDocument(tempFile));

    auto* editor = window.editorForTest();
    QVERIFY(editor);

    // Modify the document
    editor->moveCursor(QTextCursor::End);
    editor->insertPlainText(QStringLiteral("\nModified content for save test."));

    const QString saveFile = tempDir.filePath(QStringLiteral("test_save_output.txt"));

    QElapsedTimer timer;
    timer.start();

    QVERIFY(window.testSaveDocumentWithEncoding(saveFile, QStringConverter::Utf8, false));

    const qint64 saveTime = timer.elapsed();
    qInfo() << "Save time for 500KB file:" << saveTime << "ms";

    QVERIFY2(saveTime < PerformanceThresholds::SAVE_LARGE_MS,
             qPrintable(QString("Save time %1ms exceeds threshold %2ms")
                            .arg(saveTime)
                            .arg(PerformanceThresholds::SAVE_LARGE_MS)));

    // Verify file was saved
    QVERIFY(QFileInfo::exists(saveFile));
}

void PerformanceTests::testEncodingConversionUtf8ToUtf16LE()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());
    const QString tempFile = tempDir.filePath(QStringLiteral("test_encoding.txt"));
    generateTestFile(tempFile, 200 * 1024); // 200KB

    MainWindow window;
    QVERIFY(window.testLoadDocument(tempFile));

    auto* editor = window.editorForTest();
    QVERIFY(editor);
    const QString originalContent = editor->toPlainText();

    const QString outputFile = tempDir.filePath(QStringLiteral("test_utf16le.txt"));

    QElapsedTimer timer;
    timer.start();

    QVERIFY(window.testSaveDocumentWithEncoding(outputFile, QStringConverter::Utf16LE, true));

    const qint64 conversionTime = timer.elapsed();
    qInfo() << "UTF-8 to UTF-16LE conversion time:" << conversionTime << "ms";

    QVERIFY2(conversionTime < PerformanceThresholds::ENCODING_MS,
             qPrintable(QString("Encoding time %1ms exceeds threshold %2ms")
                            .arg(conversionTime)
                            .arg(PerformanceThresholds::ENCODING_MS)));

    // Verify the conversion preserved content
    editor->clear();
    QVERIFY(window.testLoadDocument(outputFile));
    QCOMPARE(editor->toPlainText(), originalContent);
}

void PerformanceTests::testEncodingConversionUtf8ToUtf16BE()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());
    const QString tempFile = tempDir.filePath(QStringLiteral("test_encoding.txt"));
    generateTestFile(tempFile, 200 * 1024); // 200KB

    MainWindow window;
    QVERIFY(window.testLoadDocument(tempFile));

    auto* editor = window.editorForTest();
    QVERIFY(editor);
    const QString originalContent = editor->toPlainText();

    const QString outputFile = tempDir.filePath(QStringLiteral("test_utf16be.txt"));

    QElapsedTimer timer;
    timer.start();

    QVERIFY(window.testSaveDocumentWithEncoding(outputFile, QStringConverter::Utf16BE, true));

    const qint64 conversionTime = timer.elapsed();
    qInfo() << "UTF-8 to UTF-16BE conversion time:" << conversionTime << "ms";

    QVERIFY2(conversionTime < PerformanceThresholds::ENCODING_MS,
             qPrintable(QString("Encoding time %1ms exceeds threshold %2ms")
                            .arg(conversionTime)
                            .arg(PerformanceThresholds::ENCODING_MS)));

    // Verify the conversion preserved content
    editor->clear();
    QVERIFY(window.testLoadDocument(outputFile));
    QCOMPARE(editor->toPlainText(), originalContent);
}

void PerformanceTests::testEncodingRoundTripPerformance()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());
    const QString tempFile = tempDir.filePath(QStringLiteral("test_roundtrip.txt"));
    generateTestFile(tempFile, 100 * 1024); // 100KB

    MainWindow window;
    QVERIFY(window.testLoadDocument(tempFile));

    auto* editor = window.editorForTest();
    QVERIFY(editor);
    const QString originalContent = editor->toPlainText();

    QElapsedTimer timer;
    timer.start();

    // UTF-8 -> UTF-16LE -> UTF-16BE -> UTF-8
    const QString utf16lePath = tempDir.filePath(QStringLiteral("roundtrip_utf16le.txt"));
    QVERIFY(window.testSaveDocumentWithEncoding(utf16lePath, QStringConverter::Utf16LE, true));

    editor->clear();
    QVERIFY(window.testLoadDocument(utf16lePath));

    const QString utf16bePath = tempDir.filePath(QStringLiteral("roundtrip_utf16be.txt"));
    QVERIFY(window.testSaveDocumentWithEncoding(utf16bePath, QStringConverter::Utf16BE, true));

    editor->clear();
    QVERIFY(window.testLoadDocument(utf16bePath));

    const QString utf8Path = tempDir.filePath(QStringLiteral("roundtrip_utf8.txt"));
    QVERIFY(window.testSaveDocumentWithEncoding(utf8Path, QStringConverter::Utf8, false));

    const qint64 roundTripTime = timer.elapsed();
    qInfo() << "Round-trip encoding time (UTF-8→UTF-16LE→UTF-16BE→UTF-8):" << roundTripTime << "ms";

    // Verify content preservation
    editor->clear();
    QVERIFY(window.testLoadDocument(utf8Path));
    QCOMPARE(editor->toPlainText(), originalContent);
}

void PerformanceTests::testMemoryUsageDuringEncoding()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());
    const QString tempFile = tempDir.filePath(QStringLiteral("test_memory.txt"));
    generateTestFile(tempFile, 1024 * 1024); // 1MB

    const qint64 memoryBefore = getCurrentMemoryUsage();

    MainWindow window;
    QVERIFY(window.testLoadDocument(tempFile));

    auto* editor = window.editorForTest();
    QVERIFY(editor);

    const qint64 memoryAfterLoad = getCurrentMemoryUsage();

    const QString outputFile = tempDir.filePath(QStringLiteral("test_memory_utf16.txt"));
    QVERIFY(window.testSaveDocumentWithEncoding(outputFile, QStringConverter::Utf16LE, true));

    const qint64 memoryAfterSave = getCurrentMemoryUsage();

    if (memoryBefore > 0)
    {
        const qint64 memoryDelta = memoryAfterSave - memoryBefore;
        qInfo() << "Memory usage before:" << memoryBefore / 1024 << "KB";
        qInfo() << "Memory usage after load:" << memoryAfterLoad / 1024 << "KB";
        qInfo() << "Memory usage after save:" << memoryAfterSave / 1024 << "KB";
        qInfo() << "Memory delta:" << memoryDelta / 1024 << "KB";

        // Memory usage should be reasonable (< 50MB for 1MB file)
        QVERIFY2(memoryDelta < 50 * 1024 * 1024,
                 qPrintable(QString("Memory usage %1MB exceeds expected range")
                                .arg(memoryDelta / (1024 * 1024))));
    }
    else
    {
        qInfo() << "Memory monitoring not available on this platform";
    }
}

void PerformanceTests::testFindPerformanceLargeFile()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());
    const QString tempFile = tempDir.filePath(QStringLiteral("test_find.txt"));
    generateTestFile(tempFile, 200 * 1024); // 200KB

    MainWindow window;
    QVERIFY(window.testLoadDocument(tempFile));

    auto* editor = window.editorForTest();
    QVERIFY(editor);

    // Set up search state
    window.setSearchStateForTest(QStringLiteral("Lorem"), Qt::CaseInsensitive);

    QElapsedTimer timer;
    timer.start();

    // Perform multiple find operations
    editor->moveCursor(QTextCursor::Start);
    for (int i = 0; i < 10; ++i)
    {
        window.testFindNext();
        QApplication::processEvents();
    }

    const qint64 findTime = timer.elapsed();
    qInfo() << "Find time for 10 operations on 200KB file:" << findTime << "ms";

    QVERIFY2(findTime < PerformanceThresholds::FIND_MS,
             qPrintable(QString("Find time %1ms exceeds threshold %2ms")
                            .arg(findTime)
                            .arg(PerformanceThresholds::FIND_MS)));
}

void PerformanceTests::testReplacePerformanceLargeFile()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());
    const QString tempFile = tempDir.filePath(QStringLiteral("test_replace.txt"));
    generateTestFile(tempFile, 100 * 1024); // 100KB

    MainWindow window;
    QVERIFY(window.testLoadDocument(tempFile));

    auto* editor = window.editorForTest();
    QVERIFY(editor);

    window.setSearchStateForTest(QStringLiteral("Lorem"), Qt::CaseInsensitive, QStringLiteral("REPLACED"));

    QElapsedTimer timer;
    timer.start();

    const int replacedCount = window.testReplaceAll(QStringLiteral("Lorem"), QStringLiteral("REPLACED"));

    const qint64 replaceTime = timer.elapsed();
    qInfo() << "Replace time for" << replacedCount << "occurrences:" << replaceTime << "ms";

    QVERIFY2(replaceTime < PerformanceThresholds::REPLACE_MS,
             qPrintable(QString("Replace time %1ms exceeds threshold %2ms")
                            .arg(replaceTime)
                            .arg(PerformanceThresholds::REPLACE_MS)));

    QVERIFY(replacedCount > 0);
}

void PerformanceTests::testUndoRedoStackPerformance()
{
    MainWindow window;
    auto* editor = window.editorForTest();
    QVERIFY(editor);

    editor->clear();

    // Perform 100 insert operations
    QElapsedTimer timer;
    timer.start();

    for (int i = 0; i < 100; ++i)
    {
        editor->insertPlainText(QString::number(i) + QStringLiteral(" "));
    }

    const qint64 insertTime = timer.elapsed();
    qInfo() << "100 insert operations time:" << insertTime << "ms";

    // Test undo performance
    timer.restart();

    for (int i = 0; i < 100; ++i)
    {
        editor->undo();
    }

    const qint64 undoTime = timer.elapsed();
    qInfo() << "100 undo operations time:" << undoTime << "ms";

    // Test redo performance
    timer.restart();

    for (int i = 0; i < 100; ++i)
    {
        editor->redo();
    }

    const qint64 redoTime = timer.elapsed();
    qInfo() << "100 redo operations time:" << redoTime << "ms";

    QVERIFY2(undoTime < PerformanceThresholds::UNDO_REDO_MS * 100,
             qPrintable(QString("Undo time %1ms exceeds threshold").arg(undoTime)));

    QVERIFY2(redoTime < PerformanceThresholds::UNDO_REDO_MS * 100,
             qPrintable(QString("Redo time %1ms exceeds threshold").arg(redoTime)));
}

void PerformanceTests::testZoomOperationsPerformance()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());
    const QString tempFile = tempDir.filePath(QStringLiteral("test_zoom.txt"));
    generateTestFile(tempFile, 100 * 1024); // 100KB

    MainWindow window;
    QVERIFY(window.testLoadDocument(tempFile));

    auto* editor = window.editorForTest();
    QVERIFY(editor);

    QElapsedTimer timer;
    timer.start();

    // Perform multiple zoom operations
    const int originalZoom = editor->zoomPercentage();
    for (int i = 0; i < 10; ++i)
    {
        editor->increaseZoom();
        QApplication::processEvents();
    }

    for (int i = 0; i < 10; ++i)
    {
        editor->decreaseZoom();
        QApplication::processEvents();
    }

    const qint64 zoomTime = timer.elapsed();
    qInfo() << "Zoom operations time (20 operations):" << zoomTime << "ms";

    QVERIFY2(zoomTime < PerformanceThresholds::ZOOM_MS * 20,
             qPrintable(QString("Zoom time %1ms exceeds threshold").arg(zoomTime)));

    QCOMPARE(editor->zoomPercentage(), originalZoom);
}

void PerformanceTests::testDocumentModificationPerformance()
{
    MainWindow window;
    auto* editor = window.editorForTest();
    QVERIFY(editor);

    editor->clear();
    QVERIFY(!editor->document()->isModified());

    QElapsedTimer timer;
    timer.start();

    // Perform rapid modifications
    for (int i = 0; i < 1000; ++i)
    {
        editor->insertPlainText(QStringLiteral("x"));
        const bool isModified = editor->document()->isModified();
        QVERIFY(isModified);
    }

    const qint64 modificationTime = timer.elapsed();
    qInfo() << "1000 modifications detection time:" << modificationTime << "ms";

    // Should be very fast since it's just flag checking
    QVERIFY2(modificationTime < 1000,
             qPrintable(QString("Modification detection time %1ms is too slow").arg(modificationTime)));
}

void PerformanceTests::testMassiveInsertOperations()
{
    MainWindow window;
    auto* editor = window.editorForTest();
    QVERIFY(editor);

    editor->clear();

    QElapsedTimer timer;
    timer.start();

    // Insert 1000 lines
    for (int i = 0; i < 1000; ++i)
    {
        editor->insertPlainText(QString("Line %1: This is a test line with some content.\n").arg(i));
        if (i % 100 == 0)
        {
            QApplication::processEvents();
        }
    }

    const qint64 insertTime = timer.elapsed();
    qInfo() << "Massive insert (1000 lines) time:" << insertTime << "ms";

    // Verify the content
    const QString content = editor->toPlainText();
    QVERIFY(content.contains(QStringLiteral("Line 0:")));
    QVERIFY(content.contains(QStringLiteral("Line 999:")));
}

void PerformanceTests::testMassiveDeleteOperations()
{
    MainWindow window;
    auto* editor = window.editorForTest();
    QVERIFY(editor);

    editor->clear();

    // Prepare content
    for (int i = 0; i < 1000; ++i)
    {
        editor->insertPlainText(QString("Line %1: This is a test line with some content.\n").arg(i));
    }

    QApplication::processEvents();

    QElapsedTimer timer;
    timer.start();

    // Delete half the content
    QTextCursor cursor = editor->textCursor();
    cursor.movePosition(QTextCursor::Start);
    cursor.movePosition(QTextCursor::Down, QTextCursor::MoveAnchor, 500);
    cursor.movePosition(QTextCursor::End, QTextCursor::KeepAnchor);
    cursor.removeSelectedText();

    const qint64 deleteTime = timer.elapsed();
    qInfo() << "Massive delete (500 lines) time:" << deleteTime << "ms";

    // Verify deletion
    const QString content = editor->toPlainText();
    QVERIFY(content.contains(QStringLiteral("Line 0:")));
    QVERIFY(!content.contains(QStringLiteral("Line 999:")));
}

void PerformanceTests::testContinuousEditing()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());
    const QString tempFile = tempDir.filePath(QStringLiteral("test_continuous.txt"));
    generateTestFile(tempFile, 50 * 1024); // 50KB

    MainWindow window;
    QVERIFY(window.testLoadDocument(tempFile));

    auto* editor = window.editorForTest();
    QVERIFY(editor);

    QElapsedTimer timer;
    timer.start();

    // Simulate continuous editing: mix of inserts, deletes, and cursor movements
    for (int i = 0; i < 50; ++i)
    {
        // Move cursor randomly
        QTextCursor cursor = editor->textCursor();
        cursor.movePosition(QTextCursor::Start);
        cursor.movePosition(QTextCursor::Down, QTextCursor::MoveAnchor, i % 100);

        // Insert text
        editor->setTextCursor(cursor);
        editor->insertPlainText(QString("[Edit %1] ").arg(i));

        // Process events occasionally
        if (i % 10 == 0)
        {
            QApplication::processEvents();
        }
    }

    const qint64 editingTime = timer.elapsed();
    qInfo() << "Continuous editing (50 operations) time:" << editingTime << "ms";

    // Verify the document was modified
    QVERIFY(editor->document()->isModified());
    QVERIFY(editor->toPlainText().contains(QStringLiteral("[Edit 0]")));
    QVERIFY(editor->toPlainText().contains(QStringLiteral("[Edit 49]")));
}

int main(int argc, char** argv)
{
    if (!qEnvironmentVariableIsSet("QT_QPA_PLATFORM"))
    {
        qputenv("QT_QPA_PLATFORM", QByteArray("offscreen"));
    }
    QCoreApplication::setAttribute(Qt::AA_Use96Dpi, true);
    QApplication app(argc, argv);
    QCoreApplication::setOrganizationName(QStringLiteral("GnotePadTests"));
    QCoreApplication::setOrganizationDomain(QStringLiteral("tests.gnotepad.app"));
    QCoreApplication::setApplicationName(QStringLiteral("GnotePadPerformanceTests"));

    PerformanceTests tc;
    return QTest::qExec(&tc, argc, argv);
}
