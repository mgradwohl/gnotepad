#pragma once

#ifdef GNOTE_TEST_HOOKS
#include <deque>
#endif

#include <QtCore/qbytearray.h>
#include <QtCore/qnamespace.h>
#include <QtCore/qobject.h>
#include <QtCore/qsettings.h>
#include <QtCore/qstring.h>
#include <QtCore/qstringconverter.h>
#include <QtCore/qstringlist.h>
#include <QtGui/qevent.h>
#include <QtGui/qicon.h>
#include <QtGui/qtextdocument.h>
#include <QtWidgets/qmainwindow.h>
#include <QtWidgets/qmessagebox.h>
#include <QtWidgets/qwidget.h>

#include <cstdint>

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

#ifdef GNOTE_TEST_HOOKS
        bool testLoadDocument(const QString& path)
        {
            return loadDocumentFromPath(path);
        }

        bool testSaveDocument(const QString& path)
        {
            return saveDocumentToPath(path);
        }

        bool testSaveDocumentWithEncoding(const QString& path, QStringConverter::Encoding encoding, bool bom)
        {
            applyEncodingSelection(encoding, bom);
            return saveDocumentToPath(path);
        }

        TextEditor* editorForTest() const
        {
            return m_editor;
        }

        QStringConverter::Encoding currentEncodingForTest() const
        {
            return m_currentEncoding;
        }

        bool currentBomForTest() const
        {
            return m_hasBom;
        }

        void setSearchStateForTest(const QString& term, Qt::CaseSensitivity sensitivity, const QString& replacement = {});
        bool testFindNext(QTextDocument::FindFlags extraFlags = {});
        bool testFindPrevious();
        bool testReplaceNext(const QString& replacementOverride = {});
        int testReplaceAll(const QString& term, const QString& replacement, QTextDocument::FindFlags extraFlags = {});

        const QStringList& recentFilesForTest() const
        {
            return m_recentFiles;
        }

        QMenu* recentFilesMenuForTest() const
        {
            return m_recentFilesMenu;
        }

        void enqueueDestructivePromptResponseForTest(QMessageBox::StandardButton button)
        {
            m_testPromptResponses.push_back(button);
        }

        void clearDestructivePromptResponsesForTest()
        {
            m_testPromptResponses.clear();
        }

        void setAutoDismissDialogsForTest(bool enabled)
        {
            m_testAutoDismissDialogs = enabled;
        }

        int findDialogInvocationCountForTest() const
        {
            return m_testFindDialogInvocations;
        }

        int replaceDialogInvocationCountForTest() const
        {
            return m_testReplaceDialogInvocations;
        }

        QAction* findActionForTest() const
        {
            return m_findAction;
        }

        QAction* replaceActionForTest() const
        {
            return m_replaceAction;
        }

        QAction* timeDateActionForTest() const
        {
            return m_timeDateAction;
        }

        QString defaultPrinterNameForTest() const
        {
            return m_defaultPrinterName;
        }

        void setDefaultPrinterNameForTest(const QString& printerName)
        {
            m_defaultPrinterName = printerName;
        }

        void testLoadPrinterSettings(QSettings& settings)
        {
            loadPrinterSettings(settings);
        }

        void testSavePrinterSettings(QSettings& settings) const
        {
            savePrinterSettings(settings);
        }
#endif

    private slots:
        void handleNewFile();
        void handleOpenFile();
        void handleSaveFile();
        void handleSaveFileAs();
        void handleChangeEncoding();
        void handlePrint();
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
        void handleViewHelp();
        void handleUpdateCursorStatus();
        void showAboutDialog();
        void handleSetTabSize();
        void handleChooseFont();
        void handleChoosePrinter();
        void handleOpenRecentFile();
        void handleClearRecentFiles();

        // NOLINTNEXTLINE(readability-redundant-access-specifiers)
    private:
        enum class DateFormatPreference : std::uint8_t
        {
            Short,
            Long
        };

        static constexpr int DefaultWindowWidth = 900;
        static constexpr int DefaultWindowHeight = 700;
        static constexpr int DefaultZoomPercent = 100;
        static constexpr int DefaultTabSizeSpaces = 4;
        static constexpr int MinTabSizeSpaces = 1;
        static constexpr int MaxTabSizeSpaces = 16;
        static constexpr int TabSizeStep = 1;
        static constexpr int AboutDialogIconSize = 64;
        static constexpr int AboutDialogMinTextWidth = 500;
        static constexpr int FontDialogWidth = 640;
        static constexpr int FontDialogHeight = 480;
        static constexpr auto UntitledDocumentTitle = "Untitled";
        static constexpr qreal InvalidFontPointSize = -1.0;

        void buildMenus();
        void buildStatusBar();
        void buildEditor();
        void wireSignals();
        void updateEncodingDisplay(const QString& encodingLabel);
        void updateWindowTitle();
        void updateDocumentStats();
        void updateZoomLabel(int percentage);
        void updateActionStates();
        [[nodiscard]] bool documentHasContent() const;
        [[nodiscard]] bool editorHasSelection() const;
        void applyDefaultEditorFont();

        bool loadDocumentFromPath(const QString& filePath);
        bool saveDocumentToPath(const QString& filePath);
        bool saveDocumentAsDialog();
        bool saveCurrentDocument(bool forceSaveAs = false);
        bool confirmReadyForDestructiveAction();
        bool promptEncodingSelection(QStringConverter::Encoding& encoding, bool& bom);
        void applyEncodingSelection(QStringConverter::Encoding encoding, bool bom);
        void resetDocumentState();
        [[nodiscard]] QTextDocument::FindFlags buildFindFlags(QTextDocument::FindFlags baseFlags = {}) const;
        bool performFind(const QString& term, QTextDocument::FindFlags flags = {});
        bool replaceNextOccurrence(const QString& term, const QString& replacement, QTextDocument::FindFlags flags = {});
        int replaceAllOccurrences(const QString& term, const QString& replacement, QTextDocument::FindFlags flags = {});
        [[nodiscard]] QIcon brandIcon() const;
        void loadSettings();
        void saveSettings() const;
        void loadWindowGeometrySettings(QSettings& settings);
        void loadPathSettings(QSettings& settings);
        void loadRecentFilesSettings(QSettings& settings);
        void loadEditorFontSettings(QSettings& settings, bool hasExistingPreferences);
        void loadEditorViewSettings(QSettings& settings);
        void loadEditorBehaviorSettings(QSettings& settings);
        void saveWindowGeometrySettings(QSettings& settings) const;
        void savePathSettings(QSettings& settings) const;
        void saveRecentFilesSettings(QSettings& settings) const;
        void saveEditorFontSettings(QSettings& settings) const;
        void saveEditorBehaviorSettings(QSettings& settings) const;
        void loadPrinterSettings(QSettings& settings);
        void savePrinterSettings(QSettings& settings) const;
        static void clearLegacySettings(QSettings& settings);
        void addRecentFile(const QString& path);
        void refreshRecentFilesMenu();
        [[nodiscard]] QString dialogDirectory(const QString& lastDir) const;
        static QString defaultDocumentsDirectory();
        void setDateFormatPreference(DateFormatPreference preference);
        void updateDateFormatActionState();

        void closeEvent(QCloseEvent* event) override;

        [[nodiscard]] QString encodingLabel() const;
        static QByteArray viewBomForEncoding(QStringConverter::Encoding encoding);
        static QStringConverter::Encoding detectEncodingFromData(const QByteArray& data, int& bomLength);

        TextEditor* m_editor{nullptr};
        QStatusBar* m_statusBar{nullptr};
        QLabel* m_cursorLabel{nullptr};
        QLabel* m_encodingLabel{nullptr};
        QLabel* m_zoomLabel{nullptr};
        QLabel* m_documentStatsLabel{nullptr};

        QAction* m_statusBarToggle{nullptr};
        QAction* m_lineNumberToggle{nullptr};
        QAction* m_wordWrapAction{nullptr};
        QAction* m_saveAction{nullptr};
        QAction* m_saveAsAction{nullptr};
        QAction* m_printAction{nullptr};
        QAction* m_cutAction{nullptr};
        QAction* m_copyAction{nullptr};
        QAction* m_deleteAction{nullptr};
        QAction* m_dateFormatShortAction{nullptr};
        QAction* m_dateFormatLongAction{nullptr};
        QAction* m_findAction{nullptr};
        QAction* m_findNextAction{nullptr};
        QAction* m_findPreviousAction{nullptr};
        QAction* m_replaceAction{nullptr};
        QAction* m_goToAction{nullptr};
        QAction* m_timeDateAction{nullptr};
        QAction* m_tabSizeAction{nullptr};
        QAction* m_encodingAction{nullptr};
        QMenu* m_recentFilesMenu{nullptr};

        QString m_currentFilePath;
        QStringConverter::Encoding m_currentEncoding{QStringConverter::Utf8};
        bool m_hasBom{false};
        QString m_lastSearchTerm;
        QString m_lastReplaceText;
        Qt::CaseSensitivity m_lastCaseSensitivity{Qt::CaseInsensitive};
        QStringList m_recentFiles;
        QString m_lastOpenDirectory;
        QString m_lastSaveDirectory;
        QString m_defaultPrinterName;
        int m_tabSizeSpaces{DefaultTabSizeSpaces};
        int m_currentZoomPercent{DefaultZoomPercent};
        DateFormatPreference m_dateFormatPreference{DateFormatPreference::Short};
#ifdef GNOTE_TEST_HOOKS
        std::deque<QMessageBox::StandardButton> m_testPromptResponses;
        bool m_testAutoDismissDialogs{false};
        int m_testFindDialogInvocations{0};
        int m_testReplaceDialogInvocations{0};
#endif
    };

} // namespace GnotePad::ui
