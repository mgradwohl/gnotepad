#pragma once

#include <QObject>
#include <QString>

namespace GnotePad::ui
{
    class MainWindow;
}

class MainWindowSmokeTests : public QObject
{
    Q_OBJECT

public:
    explicit MainWindowSmokeTests(QObject* parent = nullptr);

private slots:
    void initTestCase();
    void testLaunchShowsWindow();
    void testWindowStateTransitions();
    void testDefaultsWithoutSettings();
    void testHandlesCorruptSettings();
    void testZoomActions();
    void testInsertTimeDate();
    void testToggleLineNumbers();
    void testOpenSampleFile();
    void testLargeFileScrolling();
    void testSaveAsWithEncoding();
    void testEncodingRoundTripVariants();
    void testFindNavigation();
    void testReplaceOperations();
    void testRecentFilesMenu();
    void testDestructivePrompts();
    void testShortcutCommands();
    void testLoadPrinterSettingsValid();
    void testLoadPrinterSettingsInvalid();
    void testSavePrinterSettingsEmpty();
    void testSavePrinterSettingsNonEmpty();
    void testPrinterRoundTrip();
    void testDefaultPrinterBehavior();
    void testUtf8BomDetection();
    void testUtf16LEBomDetection();
    void testUtf16BEBomDetection();
    void testMultilingualContent();
    void testUnicodeCharacters();
    void testEncodingRoundTripUtf8ToBom();
    void testEncodingRoundTripUtf16Variants();
    void testLargeFileEncodingRoundTrip();
    void testMixedContentPreservation();

private: // NOLINT(readability-redundant-access-specifiers)
    QString resolveTestFile(const QString& name) const;
};
