#pragma once

#include <QObject>

namespace GnotePad::ui
{
    class MainWindow;
}

class MenuActionsTests : public QObject
{
    Q_OBJECT

public:
    explicit MenuActionsTests(QObject* parent = nullptr);

private slots:
    void initTestCase();

    // File menu actions
    void testNewFileAction();
    void testSaveActionEnabledStates();
    void testSaveAsActionEnabledStates();
    void testPrintActionEnabledStates();

    // Edit menu actions
    void testCutCopyDeleteEnabledStates();
    void testSelectAllAction();
    void testUndoRedoActions();

    // Search menu actions
    void testFindActionEnabledStates();
    void testReplaceActionEnabledStates();
    void testGoToLineActionEnabledStates();
    void testFindNextPreviousEnabledStates();

    // View menu actions
    void testZoomActionsEnabled();
    void testStatusBarToggle();
    void testLineNumberToggle();
    void testWordWrapToggle();

    // Format menu actions
    void testTimeDateActionEnabledStates();

    // Error handling
    void testOpenNonExistentFile();
    void testSaveToReadOnlyLocation();
};
