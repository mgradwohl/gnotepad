#include "ui/MainWindow.h"
#include "ui/TextEditor.h"

#include <array>
#include <QAction>
#include <QCheckBox>
#include <QCloseEvent>
#include <QCoreApplication>
#include <QDesktopServices>
#include <QDateTime>
#include <QDialog>
#include <QDialogButtonBox>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QFontDialog>
#include <QFormLayout>
#include <QIcon>
#include <QInputDialog>
#include <QLabel>
#include <QLineEdit>
#include <QFontDatabase>
#include <QPixmap>
#include <QMessageBox>
#include <QMenu>
#include <QMenuBar>
#include <QKeySequence>
#include <QPushButton>
#include <QPlainTextEdit>
#include <QPrinter>
#include <QSaveFile>
#include <QTextBlock>
#include <QTextCursor>
#include <QHBoxLayout>
#include <QStatusBar>
#include <QStringDecoder>
#include <QStringEncoder>
#include <QStringConverter>
#include <QStringList>
#include <QTextDocument>
#include <QTextOption>
#include <QVBoxLayout>
#include <QUrl>
#include <algorithm>

#include <qnamespace.h>
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
    m_editor = new TextEditor(this);
    applyDefaultEditorFont();
    m_editor->setWordWrapMode(QTextOption::NoWrap);
    setCentralWidget(m_editor);
}

void MainWindow::applyDefaultEditorFont()
{
    if(!m_editor)
    {
        return;
    }

    QStringList preferredFamilies;
#if defined(Q_OS_WIN)
    preferredFamilies << QStringLiteral("Consolas") << QStringLiteral("Cascadia Mono");
#elif defined(Q_OS_LINUX)
    preferredFamilies << QStringLiteral("Noto Sans Mono") << QStringLiteral("DejaVu Sans Mono");
#elif defined(Q_OS_MACOS)
    preferredFamilies << QStringLiteral("SF Mono") << QStringLiteral("Menlo") << QStringLiteral("Monaco");
#else
    preferredFamilies << QStringLiteral("Monaco") << QStringLiteral("Menlo");
#endif

    QFont defaultFont = QFontDatabase::systemFont(QFontDatabase::FixedFont);
    for(const auto& family : preferredFamilies)
    {
        if(QFontDatabase::hasFamily(family))
        {
            defaultFont.setFamily(family);
            break;
        }
    }

    defaultFont.setStyleHint(QFont::Monospace);
    m_editor->applyEditorFont(defaultFont);

    const QFontMetricsF metrics(defaultFont);
    m_editor->setTabStopDistance(metrics.horizontalAdvance(QStringLiteral("    ")));
}

void MainWindow::buildMenus()
{
    auto* bar = menuBar();
    auto* fileMenu = bar->addMenu(tr("&File"));
    auto* editMenu = menuBar()->addMenu(tr("&Edit"));
    auto* formatMenu = menuBar()->addMenu(tr("F&ormat"));
    auto* viewMenu = menuBar()->addMenu(tr("&View"));
    auto* helpMenu = menuBar()->addMenu(tr("&Help"));

    fileMenu->addAction(tr("&New"), QKeySequence::New, this, &MainWindow::handleNewFile);
    fileMenu->addAction(tr("&Open…"), QKeySequence::Open, this, &MainWindow::handleOpenFile);
    fileMenu->addAction(tr("&Save"), QKeySequence::Save, this, &MainWindow::handleSaveFile);
    fileMenu->addAction(tr("Save &As…"), QKeySequence::SaveAs, this, &MainWindow::handleSaveFileAs);
    fileMenu->addAction(tr("E&ncoding…"), this, &MainWindow::handleChangeEncoding);
    fileMenu->addSeparator();
    fileMenu->addAction(tr("&Print to PDF…"), QKeySequence::Print, this, &MainWindow::handlePrintToPdf);
    fileMenu->addSeparator();
    fileMenu->addAction(tr("E&xit"), QKeySequence::Quit, this, &QWidget::close);

    editMenu->addAction(tr("&Undo"), QKeySequence::Undo, m_editor, &QPlainTextEdit::undo);
    editMenu->addAction(tr("Cu&t"), QKeySequence::Cut, m_editor, &QPlainTextEdit::cut);
    editMenu->addAction(tr("&Copy"), QKeySequence::Copy, m_editor, &QPlainTextEdit::copy);
    editMenu->addAction(tr("&Paste"), QKeySequence::Paste, m_editor, &QPlainTextEdit::paste);
    editMenu->addAction(tr("De&lete"), m_editor, &QPlainTextEdit::cut);
    editMenu->addSeparator();
    editMenu->addAction(tr("&Find…"), QKeySequence::Find, this, &MainWindow::handleFind);
    editMenu->addAction(tr("Find &Next"), QKeySequence::FindNext, this, &MainWindow::handleFindNext);
    editMenu->addAction(tr("Find &Previous"), QKeySequence::FindPrevious, this, &MainWindow::handleFindPrevious);
    editMenu->addAction(tr("&Replace…"), QKeySequence::Replace, this, &MainWindow::handleReplace);
    editMenu->addAction(tr("&Go To…"), QKeySequence(Qt::CTRL | Qt::Key_G), this, &MainWindow::handleGoToLine);
    editMenu->addSeparator();
    editMenu->addAction(tr("Select &All"), QKeySequence::SelectAll, m_editor, &QPlainTextEdit::selectAll);
    editMenu->addAction(tr("Time/&Date"), QKeySequence(Qt::Key_F5), this, &MainWindow::handleInsertTimeDate);

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
            m_editor->applyEditorFont(selectedFont);
        }
    });

    m_statusBarToggle = viewMenu->addAction(tr("Status &Bar"), this, &MainWindow::handleToggleStatusBar);
    m_statusBarToggle->setCheckable(true);
    m_statusBarToggle->setChecked(true);

    m_lineNumberToggle = viewMenu->addAction(tr("Line &Numbers"), this, &MainWindow::handleToggleLineNumbers);
    m_lineNumberToggle->setCheckable(true);
    m_lineNumberToggle->setChecked(m_editor ? m_editor->lineNumbersVisible() : false);

    auto* zoomMenu = viewMenu->addMenu(tr("&Zoom"));
    zoomMenu->addAction(tr("Zoom &In"), QKeySequence::ZoomIn, this, &MainWindow::handleZoomIn);
    zoomMenu->addAction(tr("Zoom &Out"), QKeySequence::ZoomOut, this, &MainWindow::handleZoomOut);
    zoomMenu->addAction(tr("Restore &Default Zoom"), QKeySequence(Qt::CTRL | Qt::Key_0), this, &MainWindow::handleZoomReset);

    helpMenu->addAction(tr("View &Help"), QKeySequence::HelpContents, this, [] { spdlog::info("Help placeholder triggered"); });
    helpMenu->addAction(tr("&About GnotePad"), this, &MainWindow::showAboutDialog);
}

void MainWindow::buildStatusBar()
{
    m_statusBar = statusBar();

    m_cursorLabel = new QLabel(tr("Ln 1, Col 1"), this);
    m_documentStatsLabel = new QLabel(tr("Length: 0  Lines: 1"), this);
    m_encodingLabel = new QLabel(tr("UTF-8"), this);
    m_zoomLabel = new QLabel(tr("100%"), this);

    m_statusBar->addPermanentWidget(m_cursorLabel);
    m_statusBar->addPermanentWidget(m_documentStatsLabel);
    m_statusBar->addPermanentWidget(m_encodingLabel);
    m_statusBar->addPermanentWidget(m_zoomLabel);

    updateEncodingDisplay(encodingLabel());
    updateDocumentStats();
    updateZoomLabel(100);
}

void MainWindow::wireSignals()
{
    connect(m_editor, &QPlainTextEdit::cursorPositionChanged, this, &MainWindow::handleUpdateCursorStatus);
    connect(m_editor, &QPlainTextEdit::textChanged, this, &MainWindow::updateDocumentStats);
    connect(m_editor, &TextEditor::zoomPercentageChanged, this, &MainWindow::updateZoomLabel);
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

    if(!confirmReadyForDestructiveAction())
    {
        return;
    }

    resetDocumentState();
    spdlog::info("New document created");
}

void MainWindow::handleOpenFile()
{
    if(!confirmReadyForDestructiveAction())
    {
        return;
    }

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
    if(saveCurrentDocument())
    {
        spdlog::info("Saved file {}", m_currentFilePath.toStdString());
    }
}

void MainWindow::handleSaveFileAs()
{
    saveCurrentDocument(true);
}

void MainWindow::handleChangeEncoding()
{
    QStringConverter::Encoding desiredEncoding = m_currentEncoding;
    bool desiredBom = m_hasBom;
    if(promptEncodingSelection(desiredEncoding, desiredBom))
    {
        applyEncodingSelection(desiredEncoding, desiredBom);
        spdlog::info("Encoding preference updated to {}", encodingLabel().toStdString());
    }
}

void MainWindow::handleFind()
{
    if(!m_editor)
    {
        return;
    }

    QDialog dialog(this);
    dialog.setWindowTitle(tr("Find"));
    dialog.setModal(true);

    QFormLayout form(&dialog);
    auto* findField = new QLineEdit(m_lastSearchTerm, &dialog);
    auto* matchCase = new QCheckBox(tr("Match case"), &dialog);
    matchCase->setChecked(m_lastCaseSensitivity == Qt::CaseSensitive);

    form.addRow(tr("Find what:"), findField);
    form.addRow(QString(), matchCase);

    QDialogButtonBox buttons(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal, &dialog);
    form.addWidget(&buttons);

    connect(&buttons, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    connect(&buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

    if(dialog.exec() != QDialog::Accepted)
    {
        return;
    }

    const QString term = findField->text();
    if(term.isEmpty())
    {
        return;
    }

    m_lastSearchTerm = term;
    m_lastCaseSensitivity = matchCase->isChecked() ? Qt::CaseSensitive : Qt::CaseInsensitive;

    if(!performFind(term, buildFindFlags()))
    {
        QMessageBox::information(this, tr("Find"), tr("Cannot find \"%1\".").arg(term));
    }
}

void MainWindow::handleFindNext()
{
    if(m_lastSearchTerm.isEmpty())
    {
        handleFind();
        return;
    }

    if(!performFind(m_lastSearchTerm, buildFindFlags()))
    {
        QMessageBox::information(this, tr("Find"), tr("Cannot find \"%1\".").arg(m_lastSearchTerm));
    }
}

void MainWindow::handleFindPrevious()
{
    if(m_lastSearchTerm.isEmpty())
    {
        handleFind();
        return;
    }

    if(!performFind(m_lastSearchTerm, buildFindFlags(QTextDocument::FindBackward)))
    {
        QMessageBox::information(this, tr("Find"), tr("Cannot find \"%1\".").arg(m_lastSearchTerm));
    }
}

void MainWindow::handleReplace()
{
    if(!m_editor)
    {
        return;
    }

    QDialog dialog(this);
    dialog.setWindowTitle(tr("Replace"));

    auto* layout = new QVBoxLayout(&dialog);
    auto* formLayout = new QFormLayout();
    layout->addLayout(formLayout);

    auto* findField = new QLineEdit(m_lastSearchTerm, &dialog);
    auto* replaceField = new QLineEdit(m_lastReplaceText, &dialog);
    auto* matchCase = new QCheckBox(tr("Match case"), &dialog);
    matchCase->setChecked(m_lastCaseSensitivity == Qt::CaseSensitive);

    formLayout->addRow(tr("Find what:"), findField);
    formLayout->addRow(tr("Replace with:"), replaceField);
    formLayout->addRow(QString(), matchCase);

    auto* buttonsLayout = new QHBoxLayout();
    layout->addLayout(buttonsLayout);

    auto* findNextButton = new QPushButton(tr("Find Next"), &dialog);
    auto* replaceButton = new QPushButton(tr("Replace"), &dialog);
    auto* replaceAllButton = new QPushButton(tr("Replace All"), &dialog);
    auto* closeButton = new QPushButton(tr("Close"), &dialog);

    buttonsLayout->addWidget(findNextButton);
    buttonsLayout->addWidget(replaceButton);
    buttonsLayout->addWidget(replaceAllButton);
    buttonsLayout->addWidget(closeButton);

    const auto applyDialogState = [this, findField, replaceField, matchCase]() {
        m_lastSearchTerm = findField->text();
        m_lastReplaceText = replaceField->text();
        m_lastCaseSensitivity = matchCase->isChecked() ? Qt::CaseSensitive : Qt::CaseInsensitive;
    };

    const auto replaceSingle = [this](const QString& term, const QString& replacement) -> bool {
        if(term.isEmpty())
        {
            return false;
        }

        QTextCursor cursor = m_editor->textCursor();
        const bool selectionMatches = cursor.hasSelection() && QString::compare(cursor.selectedText(), term, m_lastCaseSensitivity) == 0;
        if(!selectionMatches)
        {
            if(!performFind(term, buildFindFlags()))
            {
                return false;
            }
            cursor = m_editor->textCursor();
        }

        cursor.insertText(replacement);
        m_editor->setTextCursor(cursor);
        return true;
    };

    connect(findNextButton, &QPushButton::clicked, &dialog, [this, applyDialogState]() {
        applyDialogState();
        if(m_lastSearchTerm.isEmpty())
        {
            return;
        }
        if(!performFind(m_lastSearchTerm, buildFindFlags()))
        {
            QMessageBox::information(this, tr("Replace"), tr("Cannot find \"%1\".").arg(m_lastSearchTerm));
        }
    });

    connect(replaceButton, &QPushButton::clicked, &dialog, [this, applyDialogState, replaceSingle]() {
        applyDialogState();
        if(m_lastSearchTerm.isEmpty())
        {
            return;
        }
        if(!replaceSingle(m_lastSearchTerm, m_lastReplaceText))
        {
            QMessageBox::information(this, tr("Replace"), tr("Cannot find \"%1\".").arg(m_lastSearchTerm));
        }
    });

    connect(replaceAllButton, &QPushButton::clicked, &dialog, [this, applyDialogState]() {
        applyDialogState();
        if(m_lastSearchTerm.isEmpty())
        {
            return;
        }
        const int count = replaceAllOccurrences(m_lastSearchTerm, m_lastReplaceText, buildFindFlags());
        QMessageBox::information(this, tr("Replace"), tr("Replaced %1 occurrence(s).").arg(count));
    });

    connect(closeButton, &QPushButton::clicked, &dialog, &QDialog::reject);

    dialog.exec();
}

void MainWindow::handleGoToLine()
{
    if(!m_editor)
    {
        return;
    }

    const auto* document = m_editor->document();
    const int maxLine = std::max(1, document ? document->blockCount() : 1);
    bool accepted = false;
    const int currentLine = m_editor->textCursor().blockNumber() + 1;
    const int targetLine = QInputDialog::getInt(this, tr("Go To"), tr("Line number:"), currentLine, 1, maxLine, 1, &accepted);
    if(!accepted)
    {
        return;
    }

    QTextBlock block = document->findBlockByNumber(targetLine - 1);
    if(!block.isValid())
    {
        return;
    }

    QTextCursor cursor(block);
    cursor.movePosition(QTextCursor::StartOfLine);
    m_editor->setTextCursor(cursor);
    m_editor->centerCursor();
}

void MainWindow::handleInsertTimeDate()
{
    if(!m_editor)
    {
        return;
    }

    const QString stamp = QDateTime::currentDateTime().toString(QStringLiteral("h:mm A M/d/yyyy"));
    m_editor->insertPlainText(stamp);
}

void MainWindow::showAboutDialog()
{
    const QString appName = QCoreApplication::applicationName();
    const QString version = QCoreApplication::applicationVersion();
    const QString org = QCoreApplication::organizationName();
    const QString details = tr("<p><b>%1</b> %2</p>"
                              "<p>A lightweight Qt-based text editor inspired by Windows Notepad.</p>"
                              "<p>Qt %3 • %4</p>")
                                .arg(appName, version, QString::fromLatin1(qVersion()), org);
    const QIcon icon = brandIcon();

    QDialog dialog(this);
    dialog.setWindowTitle(tr("About %1").arg(appName));
    dialog.setModal(true);
    dialog.setWindowIcon(icon);

    // get the icon as a larger image
    QLabel* iconLabel = new QLabel(&dialog);
    QPixmap aboutPixmap;
    if(!icon.isNull())
    {
        spdlog::info("About dialog: using icon for branding.");
        aboutPixmap = icon.pixmap(64, 64);
    }
    if(aboutPixmap.isNull())
    {
        spdlog::info("About dialog: creating pixmap from SVG resource.");
        aboutPixmap = QPixmap(QStringLiteral(":/gnotepad-icon.svg"));
    }

    if(aboutPixmap.isNull())
    {
        spdlog::info("About dialog: failed to resolve icon pixmap.");
        iconLabel->setVisible(false);
    }
    else
    {
        iconLabel->setPixmap(aboutPixmap);
        iconLabel->setFixedSize(aboutPixmap.size());
    }

    // layout the dialog
    auto* layout = new QVBoxLayout(&dialog);
    auto* contentLayout = new QHBoxLayout();
    layout->addLayout(contentLayout);

    iconLabel->setAlignment(Qt::AlignLeft| Qt:: AlignVCenter);
    contentLayout->addWidget(iconLabel, 0, Qt::AlignLeft);

    QLabel* textLabel = new QLabel(details, &dialog);
    textLabel->setTextFormat(Qt::RichText);
    textLabel->setWordWrap(true);
    contentLayout->addWidget(textLabel, 1);

    auto* buttons = new QDialogButtonBox(QDialogButtonBox::Ok, Qt::Horizontal, &dialog);
    connect(buttons, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    layout->addWidget(buttons);

    dialog.exec();
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
    if(m_editor)
    {
        m_editor->setLineNumbersVisible(checked);
    }
    spdlog::info("Line numbers toggled: {}", checked);
}

void MainWindow::handleZoomIn()
{
    if(m_editor)
    {
        m_editor->increaseZoom();
    }
}

void MainWindow::handleZoomOut()
{
    if(m_editor)
    {
        m_editor->decreaseZoom();
    }
}

void MainWindow::handleZoomReset()
{
    if(m_editor)
    {
        m_editor->resetZoom();
    }
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

void MainWindow::updateDocumentStats()
{
    if(!m_editor || !m_documentStatsLabel)
    {
        return;
    }

    const auto* document = m_editor->document();
    const int lines = document ? document->blockCount() : 0;
    const int characters = document ? std::max(0, document->characterCount() - 1) : 0;
    m_documentStatsLabel->setText(tr("Length: %1  Lines: %2").arg(characters).arg(std::max(1, lines)));
}

void MainWindow::updateZoomLabel(int percentage)
{
    if(m_zoomLabel)
    {
        m_zoomLabel->setText(tr("%1%").arg(percentage));
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

void MainWindow::closeEvent(QCloseEvent* event)
{
    if(confirmReadyForDestructiveAction())
    {
        event->accept();
    }
    else
    {
        event->ignore();
    }
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
    applyEncodingSelection(encoding, bomLength > 0);
    updateWindowTitle();
    updateDocumentStats();
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

    QStringConverter::Encoding desiredEncoding = m_currentEncoding;
    bool desiredBom = m_hasBom;
    if(!promptEncodingSelection(desiredEncoding, desiredBom))
    {
        return false;
    }

    applyEncodingSelection(desiredEncoding, desiredBom);

    if(saveDocumentToPath(target))
    {
        spdlog::info("Saved file {}", target.toStdString());
        return true;
    }
    return false;
}

bool MainWindow::saveCurrentDocument(bool forceSaveAs)
{
    if(forceSaveAs || m_currentFilePath.isEmpty())
    {
        return saveDocumentAsDialog();
    }
    return saveDocumentToPath(m_currentFilePath);
}

void MainWindow::resetDocumentState()
{
    m_currentFilePath.clear();
    if(m_editor)
    {
        m_editor->document()->clear();
        m_editor->document()->setModified(false);
    }
    applyEncodingSelection(QStringConverter::Utf8, false);
    updateWindowTitle();
    updateDocumentStats();
}

bool MainWindow::confirmReadyForDestructiveAction()
{
    if(!m_editor || !m_editor->document()->isModified())
    {
        return true;
    }

    const auto title = m_currentFilePath.isEmpty() ? tr(UntitledDocumentTitle) : QFileInfo(m_currentFilePath).fileName();
        QMessageBox prompt(this);
        prompt.setIcon(QMessageBox::Warning);
        prompt.setWindowTitle(tr("GnotePad"));
        prompt.setText(tr("Do you want to save changes to %1?").arg(title));
        prompt.setStandardButtons(QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
        prompt.setDefaultButton(QMessageBox::Save);

        const auto result = static_cast<QMessageBox::StandardButton>(prompt.exec());
        if(result == QMessageBox::Save)
        {
            return saveCurrentDocument();
        }
        if(result == QMessageBox::Discard)
        {
            return true;
        }

        return false;
}

bool MainWindow::promptEncodingSelection(QStringConverter::Encoding& encoding, bool& bom)
{
    struct EncodingChoice
    {
        QString label;
        QStringConverter::Encoding encoding;
        bool includeBom;
    };

    const std::array<EncodingChoice, 4> choices {
        EncodingChoice {tr("UTF-8 (no BOM)"), QStringConverter::Utf8, false},
        EncodingChoice {tr("UTF-8 with BOM"), QStringConverter::Utf8, true},
        EncodingChoice {tr("UTF-16 LE"), QStringConverter::Utf16LE, true},
        EncodingChoice {tr("UTF-16 BE"), QStringConverter::Utf16BE, true},
    };

    QStringList labels;
    labels.reserve(static_cast<int>(choices.size()));
    int currentIndex = 0;
    for(std::size_t i = 0; i < choices.size(); ++i)
    {
        labels.append(choices[i].label);
        if(choices[i].encoding == encoding && choices[i].includeBom == bom)
        {
            currentIndex = static_cast<int>(i);
        }
    }

    bool accepted = false;
    const auto selection = QInputDialog::getItem(this, tr("Select Encoding"), tr("Encoding:"), labels, currentIndex, false, &accepted);
    if(!accepted)
    {
        return false;
    }

    const auto match = std::find_if(choices.cbegin(), choices.cend(), [&](const auto& choice) {
        return choice.label == selection;
    });

    if(match == choices.cend())
    {
        return false;
    }

    encoding = match->encoding;
    bom = match->includeBom;
    return true;
}

void MainWindow::applyEncodingSelection(QStringConverter::Encoding encoding, bool bom)
{
    m_currentEncoding = encoding;
    m_hasBom = bom;
    updateEncodingDisplay(encodingLabel());
}

QTextDocument::FindFlags MainWindow::buildFindFlags(QTextDocument::FindFlags baseFlags) const
QIcon MainWindow::brandIcon() const
{
    QIcon icon = windowIcon();
    if(icon.isNull())
    {
        spdlog::info("brandIcon: windowIcon() failed.");
        icon = QIcon(QStringLiteral(":/gnotepad-icon.svg"));
    }
    else
    {
        spdlog::info("brandIcon: using windowIcon().");
    }

    if (icon.isNull())
    {
        spdlog::info("brandIcon: returning null icon.");
    }
    return icon;
}

bool MainWindow::performFind(const QString& term, QTextDocument::FindFlags flags)
{
    QTextDocument::FindFlags flags = baseFlags;
    if(m_lastCaseSensitivity == Qt::CaseSensitive)
    {
        flags |= QTextDocument::FindCaseSensitively;
    }
    return flags;
}

bool MainWindow::performFind(const QString& term, QTextDocument::FindFlags flags)
{
    if(!m_editor || term.isEmpty())
    {
        return false;
    }

    QTextCursor originalCursor = m_editor->textCursor();
    if(m_editor->find(term, flags))
    {
        return true;
    }

    QTextCursor searchCursor = originalCursor;
    if(flags.testFlag(QTextDocument::FindBackward))
    {
        searchCursor.movePosition(QTextCursor::End);
    }
    else
    {
        searchCursor.movePosition(QTextCursor::Start);
    }
    m_editor->setTextCursor(searchCursor);

    const bool foundAfterWrap = m_editor->find(term, flags);
    if(!foundAfterWrap)
    {
        m_editor->setTextCursor(originalCursor);
    }
    return foundAfterWrap;
}

int MainWindow::replaceAllOccurrences(const QString& term, const QString& replacement, QTextDocument::FindFlags flags)
{
    if(!m_editor || term.isEmpty())
    {
        return 0;
    }

    QTextCursor originalCursor = m_editor->textCursor();
    QTextCursor searchCursor = originalCursor;
    searchCursor.movePosition(QTextCursor::Start);
    m_editor->setTextCursor(searchCursor);

    int replacedCount = 0;
    while(m_editor->find(term, flags))
    {
        QTextCursor matchCursor = m_editor->textCursor();
            matchCursor.insertText(replacement);
            // Move cursor to the end of the replacement to avoid re-matching just-inserted text
            matchCursor.movePosition(QTextCursor::NextCharacter, QTextCursor::MoveAnchor, static_cast<int>(replacement.length()));
        m_editor->setTextCursor(matchCursor);
        ++replacedCount;
    }

    m_editor->setTextCursor(originalCursor);
    return replacedCount;
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
