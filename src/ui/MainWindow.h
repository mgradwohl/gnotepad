#pragma once

#include <memory>

#include <QByteArray>
#include <QMainWindow>
#include <QString>
#include <QStringConverter>

class QAction;
class QLabel;
class QMenu;
class QPlainTextEdit;
class QPrinter;
class QStatusBar;

namespace GnotePad::ui
{

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
    void handlePrintToPdf();
    void handleToggleStatusBar(bool checked);
    void handleToggleLineNumbers(bool checked);
    void handleUpdateCursorStatus();

private:
    void buildMenus();
    void buildStatusBar();
    void buildEditor();
    void wireSignals();
    void updateEncodingDisplay(const QString& encodingLabel);
    void updateWindowTitle();

    bool loadDocumentFromPath(const QString& filePath);
    bool saveDocumentToPath(const QString& filePath);
    bool saveDocumentAsDialog();
    void resetDocumentState();

    QString encodingLabel() const;
    static QByteArray viewBomForEncoding(QStringConverter::Encoding encoding);
    static QStringConverter::Encoding detectEncodingFromData(const QByteArray& data, int& bomLength);

    QPlainTextEdit* m_editor {nullptr};
    QStatusBar* m_statusBar {nullptr};
    QLabel* m_cursorLabel {nullptr};
    QLabel* m_encodingLabel {nullptr};
    QLabel* m_zoomLabel {nullptr};

    QAction* m_statusBarToggle {nullptr};
    QAction* m_lineNumberToggle {nullptr};

    QString m_currentFilePath;
    QStringConverter::Encoding m_currentEncoding {QStringConverter::Utf8};
    bool m_hasBom {false};
};

} // namespace GnotePad::ui
