#pragma once

#include <QObject>
#include <QString>

namespace GnotePad::ui
{
    class MainWindow;
}

class EncodingEdgeCasesTests : public QObject
{
    Q_OBJECT

public:
    explicit EncodingEdgeCasesTests(QObject* parent = nullptr);

private slots:
    void initTestCase();

    // Edge case tests
    void testEmptyFileEncoding();
    void testSingleByteFile();
    void testLargeUtf8File();
    void testMixedLineEndings();
    void testTrailingNewlines();
    void testNullBytesHandling();
    void testIncompleteUtf8Sequences();
    void testBomWithoutContent();
    void testMultipleBomMarkers();
    void testEncodingConversionErrors();
    void testUnsupportedEncoding();

private:
    QString resolveTestFile(const QString& name) const;
};
