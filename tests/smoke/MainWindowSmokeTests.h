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

private:
    QString resolveTestFile(const QString& name) const;
};
