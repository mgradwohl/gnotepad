#pragma once

#include <QByteArray>
#include <QCloseEvent>
#include <QMainWindow>
#include <QMessageBox>
#include <QString>
#include <QStringConverter>
#include <QStringList>
#include <QTextDocument>
#include <deque>

class QAction;
class QLabel;
class QCheckBox;
class QMenu;
class QPrinter;
class QStatusBar;

namespace GnotePad::ui
{

class TextEditor;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);

#if defined(GNOTE_TEST_HOOKS)
    bool testLoadDocument(const QString& path) { return loadDocumentFromPath(path); }
    bool testSaveDocument(const QString& path) { return saveDocumentToPath(path); }
    bool testSaveDocumentWithEncoding(const QString& path, QStringConverter::Encoding encoding, bool bom)
    {
        applyEncodingSelection(encoding, bom);
        return saveDocumentToPath(path);
    }
    TextEditor* editorForTest() const { return m_editor; }
    QStringConverter::Encoding currentEncodingForTest() const { return m_currentEncoding; }
    bool currentBomForTest() const { return m_hasBom; }
    void setSearchStateForTest(const QString& term, Qt::CaseSensitivity sensitivity, const QString& replacement = {});
    bool testFindNext(QTextDocument::FindFlags extraFlags = {});
    bool testFindPrevious();
    bool testReplaceNext(const QString& replacementOverride = {});
    int testReplaceAll(const QString& term, const QString& replacement, QTextDocument::FindFlags extraFlags = {});
    QStringList recentFilesForTest() const { return m_recentFiles; }
    QMenu* recentFilesMenuForTest() const { return m_recentFilesMenu; }
    void enqueueDestructivePromptResponseForTest(QMessageBox::StandardButton button) { m_testPromptResponses.push_back(button); }
    void clearDestructivePromptResponsesForTest() { m_testPromptResponses.clear(); }
    void setAutoDismissDialogsForTest(bool enabled) { m_testAutoDismissDialogs = enabled; }
    int findDialogInvocationCountForTest() const { return m_testFindDialogInvocations; }
    int replaceDialogInvocationCountForTest() const { return m_testReplaceDialogInvocations; }
    QAction* findActionForTest() const { return m_findAction; }
    QAction* replaceActionForTest() const { return m_replaceAction; }
    QAction* timeDateActionForTest() const { return m_timeDateAction; }
#endif

private slots:
    void handleNewFile();
    void handleOpenFile();
    void handleSaveFile();
    void handleSaveFileAs();
    void handleChangeEncoding();
    void handlePrintToPdf();
    void handleToggleStatusBar(bool checked);
    void handleToggleLineNumbers(bool checked);
    void handleZoomIn();
    void handleZoomOut();
    void handleZoomReset();
    void handleFind();
    void handleFindNext();
    void handleFindPrevious();
    void handleReplace();
    void handleGoToLine();
    void handleInsertTimeDate();
    void handleUpdateCursorStatus();
    void showAboutDialog();
    void handleSetTabSize();
    void handleOpenRecentFile();
    void handleClearRecentFiles();

private:
    enum class DateFormatPreference
    {
        Short,
        Long
    };

    void buildMenus();
    void buildStatusBar();
    void buildEditor();
    void wireSignals();
    void updateEncodingDisplay(const QString& encodingLabel);
    void updateWindowTitle();
    void updateDocumentStats();
    void updateZoomLabel(int percentage);
    void applyDefaultEditorFont();

    bool loadDocumentFromPath(const QString& filePath);
    bool saveDocumentToPath(const QString& filePath);
    bool saveDocumentAsDialog();
    bool saveCurrentDocument(bool forceSaveAs = false);
    bool confirmReadyForDestructiveAction();
    bool promptEncodingSelection(QStringConverter::Encoding& encoding, bool& bom);
    void applyEncodingSelection(QStringConverter::Encoding encoding, bool bom);
    void resetDocumentState();
    QTextDocument::FindFlags buildFindFlags(QTextDocument::FindFlags baseFlags = {}) const;
    bool performFind(const QString& term, QTextDocument::FindFlags flags = {});
    bool replaceNextOccurrence(const QString& term, const QString& replacement, QTextDocument::FindFlags flags = {});
    int replaceAllOccurrences(const QString& term, const QString& replacement, QTextDocument::FindFlags flags = {});
    QIcon brandIcon() const;
    void loadSettings();
    void saveSettings() const;
    void addRecentFile(const QString& path);
    void refreshRecentFilesMenu();
    QString dialogDirectory(const QString& lastDir) const;
    QString defaultDocumentsDirectory() const;
    void setDateFormatPreference(DateFormatPreference preference);
    void updateDateFormatActionState();

    void closeEvent(QCloseEvent* event) override;

    QString encodingLabel() const;
    static QByteArray viewBomForEncoding(QStringConverter::Encoding encoding);
    static QStringConverter::Encoding detectEncodingFromData(const QByteArray& data, int& bomLength);

    TextEditor* m_editor {nullptr};
    QStatusBar* m_statusBar {nullptr};
    QLabel* m_cursorLabel {nullptr};
    QLabel* m_encodingLabel {nullptr};
    QLabel* m_zoomLabel {nullptr};
    QLabel* m_documentStatsLabel {nullptr};

    QAction* m_statusBarToggle {nullptr};
    QAction* m_lineNumberToggle {nullptr};
    QAction* m_wordWrapAction {nullptr};
    QAction* m_dateFormatShortAction {nullptr};
    QAction* m_dateFormatLongAction {nullptr};
    QAction* m_findAction {nullptr};
    QAction* m_replaceAction {nullptr};
    QAction* m_timeDateAction {nullptr};
    QMenu* m_recentFilesMenu {nullptr};

    QString m_currentFilePath;
    QStringConverter::Encoding m_currentEncoding {QStringConverter::Utf8};
    bool m_hasBom {false};
    QString m_lastSearchTerm;
    QString m_lastReplaceText;
    Qt::CaseSensitivity m_lastCaseSensitivity {Qt::CaseInsensitive};
    QStringList m_recentFiles;
    QString m_lastOpenDirectory;
    QString m_lastSaveDirectory;
    int m_tabSizeSpaces {4};
    int m_currentZoomPercent {100};
    DateFormatPreference m_dateFormatPreference {DateFormatPreference::Short};
#if defined(GNOTE_TEST_HOOKS)
    std::deque<QMessageBox::StandardButton> m_testPromptResponses;
    bool m_testAutoDismissDialogs {false};
    int m_testFindDialogInvocations {0};
    int m_testReplaceDialogInvocations {0};
#endif
};

} // namespace GnotePad::ui
