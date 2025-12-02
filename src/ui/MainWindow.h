#pragma once

#include <memory>

#include <QByteArray>
#include <QCloseEvent>
#include <QMainWindow>
#include <QString>
#include <QStringConverter>
#include <QTextDocument>

class QAction;
class QLabel;
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

private:
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
    bool performFind(const QString& term, QTextDocument::FindFlags flags = {});
    int replaceAllOccurrences(const QString& term, const QString& replacement, QTextDocument::FindFlags flags = {});

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

    QString m_currentFilePath;
    QStringConverter::Encoding m_currentEncoding {QStringConverter::Utf8};
    bool m_hasBom {false};
    QString m_lastSearchTerm;
    QString m_lastReplaceText;
    Qt::CaseSensitivity m_lastCaseSensitivity {Qt::CaseInsensitive};
};

} // namespace GnotePad::ui
