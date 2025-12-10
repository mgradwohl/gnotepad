#pragma once

#include <QObject>
#include <QString>

#include <functional>

namespace GnotePad::ui
{
    class MainWindow;
}

class PerformanceTests : public QObject
{
    Q_OBJECT

public:
    explicit PerformanceTests(QObject* parent = nullptr);

private slots:
    void initTestCase();
    void cleanupTestCase();

    // Large file handling tests
    void testLoadLargeFile100KB();
    void testLoadLargeFile500KB();
    void testLoadLargeFile1MB();
    void testScrollPerformanceLargeFile();
    void testEditPerformanceLargeFile();
    void testSaveLargeFile();

    // Encoding conversion performance tests
    void testEncodingConversionUtf8ToUtf16LE();
    void testEncodingConversionUtf8ToUtf16BE();
    void testEncodingRoundTripPerformance();
    void testMemoryUsageDuringEncoding();

    // UI responsiveness tests
    void testFindPerformanceLargeFile();
    void testReplacePerformanceLargeFile();
    void testUndoRedoStackPerformance();
    void testZoomOperationsPerformance();
    void testDocumentModificationPerformance();

    // Stress tests
    void testMassiveInsertOperations();
    void testMassiveDeleteOperations();
    void testContinuousEditing();

private:
    QString resolveTestFile(const QString& name) const;
    void generateTestFile(const QString& path, qint64 sizeInBytes);
    qint64 getCurrentMemoryUsage() const;
};
