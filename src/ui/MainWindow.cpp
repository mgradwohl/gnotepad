#include "ui/MainWindow.h"

#include <QAction>
#include <QDesktopServices>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QFontDialog>
#include <QLabel>
#include <QMessageBox>
#include <QMenu>
#include <QMenuBar>
#include <QKeySequence>
#include <QPlainTextEdit>
#include <QPrinter>
#include <QSaveFile>
#include <QStatusBar>
#include <QStringDecoder>
#include <QStringEncoder>
#include <QStringConverter>
#include <QTextDocument>
#include <QTextOption>
#include <QUrl>

#include <spdlog/spdlog.h>

namespace GnotePad::ui
{

namespace
{
constexpr auto UntitledDocumentTitle = "Untitled";
}

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
{
    buildEditor();
    buildMenus();
    buildStatusBar();
    wireSignals();

    resetDocumentState();
    resize(900, 700);
}

void MainWindow::buildEditor()
{
    m_editor = new QPlainTextEdit(this);
    m_editor->setTabStopDistance(fontMetrics().horizontalAdvance(QStringLiteral(" ")) * 4);
    m_editor->setWordWrapMode(QTextOption::NoWrap);
    setCentralWidget(m_editor);
}

void MainWindow::buildMenus()
{
    auto* fileMenu = menuBar()->addMenu(tr("&File"));
    auto* editMenu = menuBar()->addMenu(tr("&Edit"));
    auto* formatMenu = menuBar()->addMenu(tr("F&ormat"));
    auto* viewMenu = menuBar()->addMenu(tr("&View"));
    auto* helpMenu = menuBar()->addMenu(tr("&Help"));

    auto* newAction = fileMenu->addAction(tr("&New"), QKeySequence::New, this, &MainWindow::handleNewFile);
    auto* openAction = fileMenu->addAction(tr("&Open…"), QKeySequence::Open, this, &MainWindow::handleOpenFile);
    auto* saveAction = fileMenu->addAction(tr("&Save"), QKeySequence::Save, this, &MainWindow::handleSaveFile);
    auto* saveAsAction = fileMenu->addAction(tr("Save &As…"), QKeySequence::SaveAs, this, &MainWindow::handleSaveFileAs);
    fileMenu->addSeparator();
    auto* printAction = fileMenu->addAction(tr("&Print to PDF…"), QKeySequence::Print, this, &MainWindow::handlePrintToPdf);
    fileMenu->addSeparator();
    fileMenu->addAction(tr("E&xit"), QKeySequence::Quit, this, &QWidget::close);

    editMenu->addAction(tr("&Undo"), QKeySequence::Undo, m_editor, &QPlainTextEdit::undo);
    editMenu->addAction(tr("Cu&t"), QKeySequence::Cut, m_editor, &QPlainTextEdit::cut);
    editMenu->addAction(tr("&Copy"), QKeySequence::Copy, m_editor, &QPlainTextEdit::copy);
    editMenu->addAction(tr("&Paste"), QKeySequence::Paste, m_editor, &QPlainTextEdit::paste);
    editMenu->addAction(tr("De&lete"), m_editor, &QPlainTextEdit::cut);
    editMenu->addSeparator();
    editMenu->addAction(tr("&Find…"), QKeySequence::Find, this, [] { spdlog::info("Find placeholder triggered"); });
    editMenu->addAction(tr("Find &Next"), this, [] { spdlog::info("Find next placeholder triggered"); });
    editMenu->addAction(tr("Find &Previous"), this, [] { spdlog::info("Find previous placeholder triggered"); });
    editMenu->addAction(tr("&Replace…"), QKeySequence::Replace, this, [] { spdlog::info("Replace placeholder triggered"); });
    editMenu->addAction(tr("&Go To…"), QKeySequence(Qt::CTRL | Qt::Key_G), this, [] { spdlog::info("Go To placeholder triggered"); });
    editMenu->addSeparator();
    editMenu->addAction(tr("Select &All"), QKeySequence::SelectAll, m_editor, &QPlainTextEdit::selectAll);
    editMenu->addAction(tr("Time/&Date"), this, [] { spdlog::info("Time/Date placeholder triggered"); });

    auto* wordWrapAction = formatMenu->addAction(tr("&Word Wrap"));
    wordWrapAction->setCheckable(true);
    connect(wordWrapAction, &QAction::toggled, this, [this](bool checked) {
        m_editor->setWordWrapMode(checked ? QTextOption::WordWrap : QTextOption::NoWrap);
    });

    formatMenu->addAction(tr("&Font…"), this, [this] {
        bool accepted {false};
        const auto selectedFont = QFontDialog::getFont(&accepted, m_editor->font(), this, tr("Choose Font"));
        if(accepted)
        {
            m_editor->setFont(selectedFont);
        }
    });

    m_statusBarToggle = viewMenu->addAction(tr("Status &Bar"), this, &MainWindow::handleToggleStatusBar);
    m_statusBarToggle->setCheckable(true);
    m_statusBarToggle->setChecked(true);

    m_lineNumberToggle = viewMenu->addAction(tr("Line &Numbers"), this, &MainWindow::handleToggleLineNumbers);
    m_lineNumberToggle->setCheckable(true);

    helpMenu->addAction(tr("View &Help"), QKeySequence::HelpContents, this, [] { spdlog::info("Help placeholder triggered"); });
    helpMenu->addAction(tr("&About GnotePad"), this, [] { spdlog::info("About dialog placeholder triggered"); });

    Q_UNUSED(newAction)
    Q_UNUSED(openAction)
    Q_UNUSED(saveAction)
    Q_UNUSED(saveAsAction)
    Q_UNUSED(printAction)
}

void MainWindow::buildStatusBar()
{
    m_statusBar = statusBar();

    m_cursorLabel = new QLabel(tr("Ln 1, Col 1"), this);
    m_encodingLabel = new QLabel(tr("UTF-8"), this);
    m_zoomLabel = new QLabel(tr("100%"), this);

    m_statusBar->addPermanentWidget(m_cursorLabel);
    m_statusBar->addPermanentWidget(m_encodingLabel);
    m_statusBar->addPermanentWidget(m_zoomLabel);

    updateEncodingDisplay(encodingLabel());
}

void MainWindow::wireSignals()
{
    connect(m_editor, &QPlainTextEdit::cursorPositionChanged, this, &MainWindow::handleUpdateCursorStatus);
    if(m_editor && m_editor->document())
    {
        connect(m_editor->document(), &QTextDocument::modificationChanged, this, [this](bool) {
            updateWindowTitle();
        });
    }
}

void MainWindow::handleNewFile()
{
    if(!m_editor)
    {
        return;
    }

    resetDocumentState();
    spdlog::info("New document created");
}

void MainWindow::handleOpenFile()
{
    const auto filePath = QFileDialog::getOpenFileName(this, tr("Open"), QString(), tr("Text Files (*.txt);;All Files (*.*)"));
    if(filePath.isEmpty())
    {
        return;
    }

    if(loadDocumentFromPath(filePath))
    {
        spdlog::info("Loaded file {}", filePath.toStdString());
    }
}

void MainWindow::handleSaveFile()
{
    if(m_currentFilePath.isEmpty())
    {
        saveDocumentAsDialog();
        return;
    }

    if(saveDocumentToPath(m_currentFilePath))
    {
        spdlog::info("Saved file {}", m_currentFilePath.toStdString());
    }
}

void MainWindow::handleSaveFileAs()
{
    saveDocumentAsDialog();
}

void MainWindow::handlePrintToPdf()
{
    QPrinter printer(QPrinter::HighResolution);
    printer.setOutputFormat(QPrinter::PdfFormat);
    const auto target = QFileDialog::getSaveFileName(this, tr("Export PDF"), QString(), tr("PDF Files (*.pdf)"));
    if(target.isEmpty())
    {
        return;
    }

    printer.setOutputFileName(target);
    m_editor->print(&printer);
    QDesktopServices::openUrl(QUrl::fromLocalFile(target));
    spdlog::info("Exported PDF to {}", target.toStdString());
}

void MainWindow::handleToggleStatusBar(bool checked)
{
    if(m_statusBar)
    {
        m_statusBar->setVisible(checked);
    }
}

void MainWindow::handleToggleLineNumbers(bool checked)
{
    spdlog::info("Line numbers toggled: {}", checked);
}

void MainWindow::handleUpdateCursorStatus()
{
    if(!m_editor || !m_cursorLabel)
    {
        return;
    }

    const auto cursor = m_editor->textCursor();
    const int line = cursor.blockNumber() + 1;
    const int column = cursor.columnNumber() + 1;
    m_cursorLabel->setText(tr("Ln %1, Col %2").arg(line).arg(column));
}

void MainWindow::updateEncodingDisplay(const QString& encodingLabel)
{
    if(m_encodingLabel)
    {
        m_encodingLabel->setText(encodingLabel);
    }
}

void MainWindow::updateWindowTitle()
{
    const auto baseName = m_currentFilePath.isEmpty() ? tr(UntitledDocumentTitle) : QFileInfo(m_currentFilePath).fileName();
    QString decoratedName = baseName;
    if(m_editor && m_editor->document()->isModified())
    {
        decoratedName.prepend('*');
    }

    setWindowTitle(tr("%1 - GnotePad").arg(decoratedName));
}

bool MainWindow::loadDocumentFromPath(const QString& filePath)
{
    QFile file(filePath);
    if(!file.open(QIODevice::ReadOnly))
    {
        QMessageBox::warning(this, tr("Open File"), tr("Unable to open %1").arg(filePath));
        spdlog::error("Failed to open {}", filePath.toStdString());
        return false;
    }

    const QByteArray rawData = file.readAll();
    int bomLength = 0;
    const auto encoding = detectEncodingFromData(rawData, bomLength);

    QStringDecoder decoder(encoding);
    const QString text = decoder(rawData.mid(bomLength));
    if(decoder.hasError())
    {
        QMessageBox::warning(this, tr("Open File"), tr("Unsupported encoding in %1").arg(filePath));
        spdlog::error("Unsupported encoding while opening {}", filePath.toStdString());
        return false;
    }

    if(m_editor)
    {
        m_editor->setPlainText(text);
        m_editor->document()->setModified(false);
    }

    m_currentFilePath = filePath;
    m_currentEncoding = encoding;
    m_hasBom = bomLength > 0;
    updateEncodingDisplay(encodingLabel());
    updateWindowTitle();
    return true;
}

bool MainWindow::saveDocumentToPath(const QString& filePath)
{
    if(filePath.isEmpty())
    {
        return saveDocumentAsDialog();
    }

    QSaveFile file(filePath);
    if(!file.open(QIODevice::WriteOnly))
    {
        QMessageBox::warning(this, tr("Save File"), tr("Unable to save %1").arg(filePath));
        spdlog::error("Failed to open {} for writing", filePath.toStdString());
        return false;
    }

    QStringEncoder encoder(m_currentEncoding);
    const QString text = m_editor ? m_editor->toPlainText() : QString();
    QByteArray encoded = encoder(text);
    if(encoder.hasError())
    {
        QMessageBox::warning(this, tr("Save File"), tr("Unable to encode document using %1").arg(encodingLabel()));
        spdlog::error("Encoding error while saving {}", filePath.toStdString());
        return false;
    }

    QByteArray payload;
    if(m_hasBom)
    {
        payload.append(viewBomForEncoding(m_currentEncoding));
    }
    payload.append(encoded);

    if(file.write(payload) != payload.size())
    {
        QMessageBox::warning(this, tr("Save File"), tr("Failed to write data to %1").arg(filePath));
        spdlog::error("Short write while saving {}", filePath.toStdString());
        return false;
    }

    if(!file.commit())
    {
        QMessageBox::warning(this, tr("Save File"), tr("Failed to finalize %1").arg(filePath));
        spdlog::error("Failed to commit save file for {}", filePath.toStdString());
        return false;
    }

    m_currentFilePath = filePath;
    if(m_editor)
    {
        m_editor->document()->setModified(false);
    }
    updateWindowTitle();
    return true;
}

bool MainWindow::saveDocumentAsDialog()
{
    const auto target = QFileDialog::getSaveFileName(this, tr("Save As"), m_currentFilePath, tr("Text Files (*.txt);;All Files (*.*)"));
    if(target.isEmpty())
    {
        return false;
    }

    if(saveDocumentToPath(target))
    {
        spdlog::info("Saved file {}", target.toStdString());
        return true;
    }
    return false;
}

void MainWindow::resetDocumentState()
{
    m_currentFilePath.clear();
    m_currentEncoding = QStringConverter::Utf8;
    m_hasBom = false;
    if(m_editor)
    {
        m_editor->document()->clear();
        m_editor->document()->setModified(false);
    }
    updateEncodingDisplay(encodingLabel());
    updateWindowTitle();
}

QString MainWindow::encodingLabel() const
{
    QString label;
    switch(m_currentEncoding)
    {
        case QStringConverter::Utf8:
            label = tr("UTF-8");
            break;
        case QStringConverter::Utf16LE:
            label = tr("UTF-16 LE");
            break;
        case QStringConverter::Utf16BE:
            label = tr("UTF-16 BE");
            break;
        default:
            label = tr("Encoding %1").arg(static_cast<int>(m_currentEncoding));
            break;
    }
    if(m_hasBom)
    {
        label.append(tr(" BOM"));
    }
    return label;
}

QByteArray MainWindow::viewBomForEncoding(QStringConverter::Encoding encoding)
{
    switch(encoding)
    {
        case QStringConverter::Utf8:
            return QByteArray::fromHex("efbbbf");
        case QStringConverter::Utf16LE:
            return QByteArray::fromHex("fffe");
        case QStringConverter::Utf16BE:
            return QByteArray::fromHex("feff");
        default:
            return {};
    }
}

QStringConverter::Encoding MainWindow::detectEncodingFromData(const QByteArray& data, int& bomLength)
{
    bomLength = 0;
    if(data.startsWith(QByteArray::fromHex("efbbbf")))
    {
        bomLength = 3;
        return QStringConverter::Utf8;
    }
    if(data.startsWith(QByteArray::fromHex("fffe")))
    {
        bomLength = 2;
        return QStringConverter::Utf16LE;
    }
    if(data.startsWith(QByteArray::fromHex("feff")))
    {
        bomLength = 2;
        return QStringConverter::Utf16BE;
    }

    return QStringConverter::Utf8;
}

} // namespace GnotePad::ui
