#include "ui/MainWindow.h"
#include "ui/TextEditor.h"

#include <algorithm>
#include <array>
#include <cstddef>

#include <QtCore/qbytearray.h>
#include <QtCore/qcoreapplication.h>
#include <QtCore/qdatetime.h>
#include <QtCore/qdir.h>
#include <QtCore/qfile.h>
#include <QtCore/qfileinfo.h>
#include <QtCore/qiodevice.h>
#include <QtCore/qlocale.h>
#include <QtCore/qnamespace.h>
#include <QtCore/qrect.h>
#include <QtCore/qsavefile.h>
#include <QtCore/qsettings.h>
#include <QtCore/qstandardpaths.h>
#include <QtCore/qobject.h>
#include <QtCore/qstring.h>
#include <QtCore/qstringconverter.h>
#include <QtCore/qstringlist.h>
#include <QtCore/qstringliteral.h>
#include <QtCore/qtimer.h>
#include <QtCore/qurl.h>
#include <QtGui/qaction.h>
#include <QtGui/qactiongroup.h>
#include <QtGui/qdesktopservices.h>
#include <QtGui/qevent.h>
#include <QtGui/qfont.h>
#include <QtGui/qfontdatabase.h>
#include <QtGui/qicon.h>
#include <QtGui/qkeysequence.h>
#include <QtGui/qpixmap.h>
#include <QtGui/qtextobject.h>
#include <QtGui/qtextcursor.h>
#include <QtGui/qtextdocument.h>
#include <QtGui/qtextoption.h>
#include <QtPrintSupport/qprinter.h>
#include <QtWidgets/qboxlayout.h>
#include <QtWidgets/qcheckbox.h>
#include <QtWidgets/qdialog.h>
#include <QtWidgets/qdialogbuttonbox.h>
#include <QtWidgets/qfiledialog.h>
#include <QtWidgets/qfontdialog.h>
#include <QtWidgets/qformlayout.h>
#include <QtWidgets/qinputdialog.h>
#include <QtWidgets/qlabel.h>
#include <QtWidgets/qlineedit.h>
#include <QtWidgets/qmenu.h>
#include <QtWidgets/qmenubar.h>
#include <QtWidgets/qmessagebox.h>
#include <QtWidgets/qplaintextedit.h>
#include <QtWidgets/qpushbutton.h>
#include <QtWidgets/qstatusbar.h>
#include <QtWidgets/qtextedit.h>
#include <QtWidgets/qwidget.h>
#include <spdlog/spdlog.h>

namespace GnotePad::ui
{

namespace
{
constexpr auto UntitledDocumentTitle = "Untitled";
constexpr int MaxRecentFiles = 10;
} // namespace

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent)
{
    buildEditor();
    buildMenus();
    buildStatusBar();
    wireSignals();

    resize(900, 700);
    loadSettings();
    resetDocumentState();
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
    if (!m_editor)
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
    for (const auto& family : preferredFamilies)
    {
        if (QFontDatabase::hasFamily(family))
        {
            defaultFont.setFamily(family);
            break;
        }
    }

    defaultFont.setStyleHint(QFont::Monospace);
    m_editor->applyEditorFont(defaultFont);
    m_editor->setTabSizeSpaces(m_tabSizeSpaces);
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
    m_recentFilesMenu = fileMenu->addMenu(tr("Open &Recent"));
    refreshRecentFilesMenu();
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
    m_findAction = editMenu->addAction(tr("&Find…"), QKeySequence::Find, this, &MainWindow::handleFind);
    editMenu->addAction(tr("Find &Next"), QKeySequence(Qt::Key_F3), this, &MainWindow::handleFindNext);
    editMenu->addAction(tr("Find &Previous"), QKeySequence(Qt::SHIFT | Qt::Key_F3), this, &MainWindow::handleFindPrevious);
    m_replaceAction = editMenu->addAction(tr("&Replace…"), QKeySequence::Replace, this, &MainWindow::handleReplace);
    editMenu->addAction(tr("&Go To…"), QKeySequence(Qt::CTRL | Qt::Key_G), this, &MainWindow::handleGoToLine);
    editMenu->addSeparator();
    editMenu->addAction(tr("Select &All"), QKeySequence::SelectAll, m_editor, &QPlainTextEdit::selectAll);
    m_timeDateAction = editMenu->addAction(tr("Time/&Date"), QKeySequence(Qt::Key_F5), this, &MainWindow::handleInsertTimeDate);

    m_wordWrapAction = formatMenu->addAction(tr("&Word Wrap"));
    m_wordWrapAction->setCheckable(true);
    connect(m_wordWrapAction, &QAction::toggled, this,
            [this](bool checked)
            {
                m_editor->setWordWrapMode(checked ? QTextOption::WordWrap : QTextOption::NoWrap);
            });

    formatMenu->addAction(tr("&Font…"), this,
                          [this]
                          {
                              bool accepted{false};
                              const auto selectedFont = QFontDialog::getFont(&accepted, m_editor->font(), this, tr("Choose Font"));
                              if (accepted)
                              {
                                  m_editor->applyEditorFont(selectedFont);
                              }
                          });

    formatMenu->addAction(tr("Tab &Size…"), this, &MainWindow::handleSetTabSize);

    auto* dateFormatMenu = formatMenu->addMenu(tr("Time/&Date Format"));
    auto* dateFormatGroup = new QActionGroup(this);
    dateFormatGroup->setExclusive(true);

    m_dateFormatShortAction = dateFormatMenu->addAction(tr("&Short"));
    m_dateFormatShortAction->setCheckable(true);
    m_dateFormatShortAction->setActionGroup(dateFormatGroup);
    connect(m_dateFormatShortAction, &QAction::triggered, this,
            [this]()
            {
                setDateFormatPreference(DateFormatPreference::Short);
            });

    m_dateFormatLongAction = dateFormatMenu->addAction(tr("&Long"));
    m_dateFormatLongAction->setCheckable(true);
    m_dateFormatLongAction->setActionGroup(dateFormatGroup);
    connect(m_dateFormatLongAction, &QAction::triggered, this,
            [this]()
            {
                setDateFormatPreference(DateFormatPreference::Long);
            });

    updateDateFormatActionState();

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

    helpMenu->addAction(tr("View &Help"), QKeySequence::HelpContents, this,
                        []
                        {
                            spdlog::info("Help placeholder triggered");
                        });
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
    if (m_editor && m_editor->document())
    {
        connect(m_editor->document(), &QTextDocument::modificationChanged, this,
                [this](bool)
                {
                    updateWindowTitle();
                });
    }
}

void MainWindow::handleNewFile()
{
    if (!m_editor)
    {
        return;
    }

    if (!confirmReadyForDestructiveAction())
    {
        return;
    }

    resetDocumentState();
    spdlog::info("New document created");
}

void MainWindow::handleOpenFile()
{
    if (!confirmReadyForDestructiveAction())
    {
        return;
    }

    const auto filePath =
        QFileDialog::getOpenFileName(this, tr("Open"), dialogDirectory(m_lastOpenDirectory), tr("Text Files (*.txt);;All Files (*.*)"));
    if (filePath.isEmpty())
    {
        return;
    }

    if (loadDocumentFromPath(filePath))
    {
        spdlog::info("Loaded file {}", filePath.toStdString());
    }
}

void MainWindow::handleOpenRecentFile()
{
    auto* action = qobject_cast<QAction*>(sender());
    if (!action)
    {
        return;
    }

    const QString filePath = action->data().toString();
    if (filePath.isEmpty())
    {
        return;
    }

    if (!confirmReadyForDestructiveAction())
    {
        return;
    }

    if (loadDocumentFromPath(filePath))
    {
        spdlog::info("Loaded recent file {}", filePath.toStdString());
    }
}

void MainWindow::handleClearRecentFiles()
{
    if (m_recentFiles.isEmpty())
    {
        return;
    }

    m_recentFiles.clear();
    refreshRecentFilesMenu();
    spdlog::info("Cleared recent files list");
}

void MainWindow::handleSaveFile()
{
    if (saveCurrentDocument())
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
    if (promptEncodingSelection(desiredEncoding, desiredBom))
    {
        applyEncodingSelection(desiredEncoding, desiredBom);
        spdlog::info("Encoding preference updated to {}", encodingLabel().toStdString());
    }
}

void MainWindow::handleSetTabSize()
{
    if (!m_editor)
    {
        return;
    }

    bool accepted = false;
    const int currentSize = m_editor->tabSizeSpaces();
    const int newSize = QInputDialog::getInt(this, tr("Tab Size"), tr("Spaces per tab:"), currentSize, 1, 16, 1, &accepted);
    if (!accepted || newSize == currentSize)
    {
        return;
    }

    m_tabSizeSpaces = newSize;
    m_editor->setTabSizeSpaces(newSize);
    spdlog::info("Tab size updated to {} spaces", newSize);
}

void MainWindow::handleFind()
{
    if (!m_editor)
    {
        return;
    }

#if defined(GNOTE_TEST_HOOKS)
    ++m_testFindDialogInvocations;
#endif

    QDialog dialog(this);
    dialog.setWindowTitle(tr("Find"));
    dialog.setModal(true);

    auto* form = new QFormLayout(&dialog);
    auto* findField = new QLineEdit(m_lastSearchTerm, &dialog);
    auto* matchCase = new QCheckBox(tr("Match case"), &dialog);
    matchCase->setChecked(m_lastCaseSensitivity == Qt::CaseSensitive);

    form->addRow(tr("Find what:"), findField);
    form->addRow(QString(), matchCase);

    auto* buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal, &dialog);
    form->addWidget(buttons);

    connect(buttons, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

#if defined(GNOTE_TEST_HOOKS)
    if (m_testAutoDismissDialogs)
    {
        QTimer::singleShot(0, &dialog, &QDialog::reject);
    }
#endif

    if (dialog.exec() != QDialog::Accepted)
    {
        return;
    }

    const QString term = findField->text();
    if (term.isEmpty())
    {
        return;
    }

    m_lastSearchTerm = term;
    m_lastCaseSensitivity = matchCase->isChecked() ? Qt::CaseSensitive : Qt::CaseInsensitive;

    if (!performFind(term, buildFindFlags()))
    {
        QMessageBox::information(this, tr("Find"), tr("Cannot find \"%1\".").arg(term));
    }
}

void MainWindow::handleFindNext()
{
    if (m_lastSearchTerm.isEmpty())
    {
        handleFind();
        return;
    }

    if (!performFind(m_lastSearchTerm, buildFindFlags()))
    {
        QMessageBox::information(this, tr("Find"), tr("Cannot find \"%1\".").arg(m_lastSearchTerm));
    }
}

void MainWindow::handleFindPrevious()
{
    if (m_lastSearchTerm.isEmpty())
    {
        handleFind();
        return;
    }

    if (!performFind(m_lastSearchTerm, buildFindFlags(QTextDocument::FindBackward)))
    {
        QMessageBox::information(this, tr("Find"), tr("Cannot find \"%1\".").arg(m_lastSearchTerm));
    }
}

void MainWindow::handleReplace()
{
    if (!m_editor)
    {
        return;
    }

#if defined(GNOTE_TEST_HOOKS)
    ++m_testReplaceDialogInvocations;
#endif

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

    const auto applyDialogState = [this, findField, replaceField, matchCase]()
    {
        m_lastSearchTerm = findField->text();
        m_lastReplaceText = replaceField->text();
        m_lastCaseSensitivity = matchCase->isChecked() ? Qt::CaseSensitive : Qt::CaseInsensitive;
    };

    connect(findNextButton, &QPushButton::clicked, &dialog,
            [this, applyDialogState]()
            {
                applyDialogState();
                if (m_lastSearchTerm.isEmpty())
                {
                    return;
                }
                if (!performFind(m_lastSearchTerm, buildFindFlags()))
                {
                    QMessageBox::information(this, tr("Replace"), tr("Cannot find \"%1\".").arg(m_lastSearchTerm));
                }
            });

    connect(replaceButton, &QPushButton::clicked, &dialog,
            [this, applyDialogState]()
            {
                applyDialogState();
                if (m_lastSearchTerm.isEmpty())
                {
                    return;
                }
                if (!replaceNextOccurrence(m_lastSearchTerm, m_lastReplaceText, buildFindFlags()))
                {
                    QMessageBox::information(this, tr("Replace"), tr("Cannot find \"%1\".").arg(m_lastSearchTerm));
                }
            });

    connect(replaceAllButton, &QPushButton::clicked, &dialog,
            [this, applyDialogState]()
            {
                applyDialogState();
                if (m_lastSearchTerm.isEmpty())
                {
                    return;
                }
                const int count = replaceAllOccurrences(m_lastSearchTerm, m_lastReplaceText, buildFindFlags());
                QMessageBox::information(this, tr("Replace"), tr("Replaced %1 occurrence(s).").arg(count));
            });

    connect(closeButton, &QPushButton::clicked, &dialog, &QDialog::reject);

#if defined(GNOTE_TEST_HOOKS)
    if (m_testAutoDismissDialogs)
    {
        QTimer::singleShot(0, &dialog, &QDialog::reject);
    }
#endif

    dialog.exec();
}

void MainWindow::handleGoToLine()
{
    if (!m_editor)
    {
        return;
    }

    const auto* document = m_editor->document();
    const int maxLine = std::max(1, document ? document->blockCount() : 1);
    bool accepted = false;
    const int currentLine = m_editor->textCursor().blockNumber() + 1;
    const int targetLine = QInputDialog::getInt(this, tr("Go To"), tr("Line number:"), currentLine, 1, maxLine, 1, &accepted);
    if (!accepted)
    {
        return;
    }

    QTextBlock block = document->findBlockByNumber(targetLine - 1);
    if (!block.isValid())
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
    if (!m_editor)
    {
        return;
    }

    const QDateTime now = QDateTime::currentDateTime();
    const QLocale locale = QLocale::system();
    const auto preferredFormat = m_dateFormatPreference == DateFormatPreference::Long ? QLocale::LongFormat : QLocale::ShortFormat;

    QString stamp = locale.toString(now, preferredFormat);
    if (stamp.isEmpty())
    {
        stamp = locale.toString(now, QLocale::ShortFormat);
    }
    if (stamp.isEmpty())
    {
        stamp = now.toString(Qt::DateFormat::TextDate);
    }
    if (stamp.isEmpty())
    {
        stamp = now.toString(QStringLiteral("h:mm A M/d/yyyy"));
    }
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
    if (!icon.isNull())
    {
        spdlog::info("About dialog: using icon for branding.");
        aboutPixmap = icon.pixmap(64, 64);
    }
    if (aboutPixmap.isNull())
    {
        spdlog::info("About dialog: creating pixmap from SVG resource.");
        aboutPixmap = QPixmap(QStringLiteral(":/gnotepad-icon.svg"));
    }

    if (aboutPixmap.isNull())
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

    iconLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
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
    const auto target = QFileDialog::getSaveFileName(this, tr("Export PDF"), dialogDirectory(m_lastSaveDirectory), tr("PDF Files (*.pdf)"));
    if (target.isEmpty())
    {
        return;
    }

    printer.setOutputFileName(target);
    if (m_editor)
    {
        m_editor->print(&printer);
    }
    QDesktopServices::openUrl(QUrl::fromLocalFile(target));
    spdlog::info("Exported PDF to {}", target.toStdString());
}

void MainWindow::handleToggleStatusBar(bool checked)
{
    if (m_statusBar)
    {
        m_statusBar->setVisible(checked);
    }
    if (m_statusBarToggle && m_statusBarToggle->isChecked() != checked)
    {
        const QSignalBlocker blocker(m_statusBarToggle);
        m_statusBarToggle->setChecked(checked);
    }
}

void MainWindow::handleToggleLineNumbers(bool checked)
{
    if (m_editor)
    {
        m_editor->setLineNumbersVisible(checked);
    }
    spdlog::info("Line numbers toggled: {}", checked);
}

void MainWindow::handleZoomIn()
{
    if (m_editor)
    {
        m_editor->increaseZoom();
    }
}

void MainWindow::handleZoomOut()
{
    if (m_editor)
    {
        m_editor->decreaseZoom();
    }
}

void MainWindow::handleZoomReset()
{
    if (m_editor)
    {
        m_editor->resetZoom();
    }
}

void MainWindow::handleUpdateCursorStatus()
{
    if (!m_editor || !m_cursorLabel)
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
    if (m_encodingLabel)
    {
        m_encodingLabel->setText(encodingLabel);
    }
}

void MainWindow::updateDocumentStats()
{
    if (!m_editor || !m_documentStatsLabel)
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
    m_currentZoomPercent = percentage;
    if (m_zoomLabel)
    {
        m_zoomLabel->setText(tr("%1%").arg(percentage));
    }
}

void MainWindow::updateWindowTitle()
{
    const auto baseName = m_currentFilePath.isEmpty() ? tr(UntitledDocumentTitle) : QFileInfo(m_currentFilePath).fileName();
    QString decoratedName = baseName;
    if (m_editor && m_editor->document()->isModified())
    {
        decoratedName.prepend('*');
    }

    setWindowTitle(tr("%1 - GnotePad").arg(decoratedName));
}

void MainWindow::closeEvent(QCloseEvent* event)
{
    if (confirmReadyForDestructiveAction())
    {
        saveSettings();
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
    if (!file.open(QIODevice::ReadOnly))
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
    if (decoder.hasError())
    {
        QMessageBox::warning(this, tr("Open File"), tr("Unsupported encoding in %1").arg(filePath));
        spdlog::error("Unsupported encoding while opening {}", filePath.toStdString());
        return false;
    }

    if (m_editor)
    {
        m_editor->setPlainText(text);
        m_editor->document()->setModified(false);
    }

    m_currentFilePath = filePath;
    applyEncodingSelection(encoding, bomLength > 0);
    addRecentFile(filePath);
    m_lastOpenDirectory = QFileInfo(filePath).absolutePath();
    updateWindowTitle();
    updateDocumentStats();
    return true;
}

bool MainWindow::saveDocumentToPath(const QString& filePath)
{
    if (filePath.isEmpty())
    {
        return saveDocumentAsDialog();
    }

    QSaveFile file(filePath);
    if (!file.open(QIODevice::WriteOnly))
    {
        QMessageBox::warning(this, tr("Save File"), tr("Unable to save %1").arg(filePath));
        spdlog::error("Failed to open {} for writing", filePath.toStdString());
        return false;
    }

    QStringEncoder encoder(m_currentEncoding);
    const QString text = m_editor ? m_editor->toPlainText() : QString();
    QByteArray encoded = encoder(text);
    if (encoder.hasError())
    {
        QMessageBox::warning(this, tr("Save File"), tr("Unable to encode document using %1").arg(encodingLabel()));
        spdlog::error("Encoding error while saving {}", filePath.toStdString());
        return false;
    }

    QByteArray payload;
    if (m_hasBom)
    {
        payload.append(viewBomForEncoding(m_currentEncoding));
    }
    payload.append(encoded);

    if (file.write(payload) != payload.size())
    {
        QMessageBox::warning(this, tr("Save File"), tr("Failed to write data to %1").arg(filePath));
        spdlog::error("Short write while saving {}", filePath.toStdString());
        return false;
    }

    if (!file.commit())
    {
        QMessageBox::warning(this, tr("Save File"), tr("Failed to finalize %1").arg(filePath));
        spdlog::error("Failed to commit save file for {}", filePath.toStdString());
        return false;
    }

    m_currentFilePath = filePath;
    m_lastSaveDirectory = QFileInfo(filePath).absolutePath();
    addRecentFile(filePath);
    if (m_editor)
    {
        m_editor->document()->setModified(false);
    }
    updateWindowTitle();
    return true;
}

bool MainWindow::saveDocumentAsDialog()
{
    const QString initialPath = m_currentFilePath.isEmpty() ? dialogDirectory(m_lastSaveDirectory) : m_currentFilePath;
    const auto target = QFileDialog::getSaveFileName(this, tr("Save As"), initialPath, tr("Text Files (*.txt);;All Files (*.*)"));
    if (target.isEmpty())
    {
        return false;
    }

    QStringConverter::Encoding desiredEncoding = m_currentEncoding;
    bool desiredBom = m_hasBom;
    if (!promptEncodingSelection(desiredEncoding, desiredBom))
    {
        return false;
    }

    applyEncodingSelection(desiredEncoding, desiredBom);

    if (saveDocumentToPath(target))
    {
        spdlog::info("Saved file {}", target.toStdString());
        return true;
    }
    return false;
}

bool MainWindow::saveCurrentDocument(bool forceSaveAs)
{
    if (forceSaveAs || m_currentFilePath.isEmpty())
    {
        return saveDocumentAsDialog();
    }
    return saveDocumentToPath(m_currentFilePath);
}

void MainWindow::resetDocumentState()
{
    m_currentFilePath.clear();
    if (m_editor)
    {
        m_editor->document()->clear();
        m_editor->document()->setModified(false);
    }
    updateEncodingDisplay(encodingLabel());
    updateWindowTitle();
    updateDocumentStats();
}

bool MainWindow::confirmReadyForDestructiveAction()
{
    if (!m_editor || !m_editor->document()->isModified())
    {
        return true;
    }

#if defined(GNOTE_TEST_HOOKS)
    if (!m_testPromptResponses.empty())
    {
        const auto response = m_testPromptResponses.front();
        m_testPromptResponses.pop_front();
        if (response == QMessageBox::Save)
        {
            return saveCurrentDocument();
        }
        if (response == QMessageBox::Discard)
        {
            return true;
        }
        return false;
    }
#endif

    const auto title = m_currentFilePath.isEmpty() ? tr(UntitledDocumentTitle) : QFileInfo(m_currentFilePath).fileName();
    QMessageBox prompt(this);
    prompt.setIcon(QMessageBox::Warning);
    prompt.setWindowTitle(tr("GnotePad"));
    prompt.setText(tr("Do you want to save changes to %1?").arg(title));
    prompt.setStandardButtons(QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
    prompt.setDefaultButton(QMessageBox::Save);

    const auto result = static_cast<QMessageBox::StandardButton>(prompt.exec());
    if (result == QMessageBox::Save)
    {
        return saveCurrentDocument();
    }
    if (result == QMessageBox::Discard)
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

    const std::array<EncodingChoice, 4> choices{
        EncodingChoice{tr("UTF-8 (no BOM)"), QStringConverter::Utf8, false},
        EncodingChoice{tr("UTF-8 with BOM"), QStringConverter::Utf8, true},
        EncodingChoice{tr("UTF-16 LE"), QStringConverter::Utf16LE, true},
        EncodingChoice{tr("UTF-16 BE"), QStringConverter::Utf16BE, true},
    };

    QStringList labels;
    labels.reserve(choices.size());
    int currentIndex = 0;
    for (std::size_t i = 0; i < choices.size(); ++i)
    {
        labels.append(choices[i].label);
        if (choices[i].encoding == encoding && choices[i].includeBom == bom)
        {
            currentIndex = static_cast<int>(i);
        }
    }

    bool accepted = false;
    const auto selection = QInputDialog::getItem(this, tr("Select Encoding"), tr("Encoding:"), labels, currentIndex, false, &accepted);
    if (!accepted)
    {
        return false;
    }

    const auto match = std::find_if(choices.cbegin(), choices.cend(),
                                    [&](const auto& choice)
                                    {
                                        return choice.label == selection;
                                    });

    if (match == choices.cend())
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

void MainWindow::loadSettings()
{
    QSettings settings;
    const QFileInfo settingsFile(settings.fileName());
    const bool hasExistingPreferences = settingsFile.exists();

    const bool hasRectKeys = settings.contains("window/posX") && settings.contains("window/posY") && settings.contains("window/width") &&
                             settings.contains("window/height");
    const bool windowMaximized = settings.value("window/maximized", false).toBool();

    if (hasRectKeys)
    {
        const int windowX = settings.value("window/posX", x()).toInt();
        const int windowY = settings.value("window/posY", y()).toInt();
        const int windowWidth = settings.value("window/width", width()).toInt();
        const int windowHeight = settings.value("window/height", height()).toInt();

        if (windowWidth > 0 && windowHeight > 0)
        {
            resize(windowWidth, windowHeight);
        }
        move(windowX, windowY);
    }
    else if (settings.contains("window/geometry"))
    {
        const QByteArray legacyGeometry = settings.value("window/geometry").toByteArray();
        restoreGeometry(legacyGeometry);
    }

    if (windowMaximized)
    {
        setWindowState(Qt::WindowMaximized);
    }
    else
    {
        setWindowState(Qt::WindowNoState);
    }

    m_lastOpenDirectory = settings.value("paths/lastOpenDirectory").toString();
    m_lastSaveDirectory = settings.value("paths/lastSaveDirectory").toString();

    m_recentFiles = settings.value("documents/recentFiles").toStringList();
    while (m_recentFiles.size() > MaxRecentFiles)
    {
        m_recentFiles.removeLast();
    }
    refreshRecentFilesMenu();

    const QString fontFamily = settings.value("editor/fontFamily").toString();
    const qreal fontPointSize = settings.value("editor/fontPointSize", -1.0).toDouble();
    if (m_editor && !fontFamily.isEmpty())
    {
        QFont storedFont(fontFamily);
        if (fontPointSize > 0)
        {
            storedFont.setPointSizeF(fontPointSize);
        }
        m_editor->applyEditorFont(storedFont);
    }
    else if (m_editor && settings.contains("editor/font"))
    {
        const QFont legacyFont = settings.value("editor/font").value<QFont>();
        if (!legacyFont.family().isEmpty())
        {
            m_editor->applyEditorFont(legacyFont);
        }
    }
    else if (m_editor && !hasExistingPreferences)
    {
        applyDefaultEditorFont();
    }

    const bool lineNumbersVisible = settings.value("editor/lineNumbersVisible", true).toBool();
    if (m_editor)
    {
        m_editor->setLineNumbersVisible(lineNumbersVisible);
    }
    if (m_lineNumberToggle)
    {
        m_lineNumberToggle->setChecked(lineNumbersVisible);
    }

    const bool wrapEnabled = settings.value("editor/wordWrap", false).toBool();
    if (m_editor)
    {
        m_editor->setWordWrapMode(wrapEnabled ? QTextOption::WordWrap : QTextOption::NoWrap);
    }
    if (m_wordWrapAction)
    {
        const QSignalBlocker blocker(m_wordWrapAction);
        m_wordWrapAction->setChecked(wrapEnabled);
    }

    const bool statusBarVisible = settings.value("editor/statusBarVisible", true).toBool();
    if (m_statusBar)
    {
        m_statusBar->setVisible(statusBarVisible);
    }
    if (m_statusBarToggle)
    {
        m_statusBarToggle->setChecked(statusBarVisible);
    }

    m_tabSizeSpaces = std::clamp(settings.value("editor/tabSizeSpaces", m_tabSizeSpaces).toInt(), 1, 16);
    if (m_editor)
    {
        m_editor->setTabSizeSpaces(m_tabSizeSpaces);
    }

    const int encodingValue = settings.value("editor/defaultEncoding", static_cast<int>(m_currentEncoding)).toInt();
    const bool bom = settings.value("editor/defaultBom", m_hasBom).toBool();
    applyEncodingSelection(static_cast<QStringConverter::Encoding>(encodingValue), bom);

    const int zoomPercent = settings.value("editor/zoomPercent", 100).toInt();
    if (m_editor)
    {
        m_editor->setZoomPercentage(zoomPercent);
    }
    else
    {
        updateZoomLabel(zoomPercent);
    }

    const QString dateFormatValue = settings.value("editor/dateFormat", QStringLiteral("short")).toString();
    if (dateFormatValue.compare(QStringLiteral("long"), Qt::CaseInsensitive) == 0)
    {
        setDateFormatPreference(DateFormatPreference::Long);
    }
    else
    {
        setDateFormatPreference(DateFormatPreference::Short);
    }
}

void MainWindow::saveSettings() const
{
    QSettings settings;

    const QRect windowRect = isMaximized() ? normalGeometry() : geometry();
    settings.setValue("window/posX", windowRect.x());
    settings.setValue("window/posY", windowRect.y());
    settings.setValue("window/width", windowRect.width());
    settings.setValue("window/height", windowRect.height());
    settings.setValue("window/maximized", isMaximized());

    settings.setValue("paths/lastOpenDirectory", m_lastOpenDirectory);
    settings.setValue("paths/lastSaveDirectory", m_lastSaveDirectory);
    settings.setValue("documents/recentFiles", m_recentFiles);

    if (m_editor)
    {
        const QFont editorFont = m_editor->font();
        settings.setValue("editor/fontFamily", editorFont.family());
        settings.setValue("editor/fontPointSize", editorFont.pointSizeF());
        settings.setValue("editor/lineNumbersVisible", m_editor->lineNumbersVisible());
        settings.setValue("editor/wordWrap", m_editor->wordWrapMode() != QTextOption::NoWrap);
    }
    else
    {
        settings.remove("editor/fontFamily");
        settings.remove("editor/fontPointSize");
        settings.setValue("editor/lineNumbersVisible", true);
        settings.setValue("editor/wordWrap", false);
    }

    settings.setValue("editor/tabSizeSpaces", m_tabSizeSpaces);
    settings.setValue("editor/statusBarVisible", m_statusBar ? m_statusBar->isVisible() : true);
    settings.setValue("editor/defaultEncoding", static_cast<int>(m_currentEncoding));
    settings.setValue("editor/defaultBom", m_hasBom);
    settings.setValue("editor/zoomPercent", m_currentZoomPercent);
    settings.setValue("editor/dateFormat",
                      m_dateFormatPreference == DateFormatPreference::Long ? QStringLiteral("long") : QStringLiteral("short"));

    settings.remove("window/geometry");
    settings.remove("window/state");
    settings.remove("editor/font");
}

void MainWindow::addRecentFile(const QString& path)
{
    if (path.isEmpty())
    {
        return;
    }

    const QString normalizedPath = QFileInfo(path).absoluteFilePath();
    if (normalizedPath.isEmpty())
    {
        return;
    }

    m_recentFiles.removeAll(normalizedPath);
    m_recentFiles.prepend(normalizedPath);
    while (m_recentFiles.size() > MaxRecentFiles)
    {
        m_recentFiles.removeLast();
    }
    refreshRecentFilesMenu();
}

void MainWindow::refreshRecentFilesMenu()
{
    if (!m_recentFilesMenu)
    {
        return;
    }

    m_recentFilesMenu->clear();
    if (m_recentFiles.isEmpty())
    {
        auto* emptyAction = m_recentFilesMenu->addAction(tr("(No Recent Files)"));
        emptyAction->setEnabled(false);
    }
    else
    {
        for (const auto& path : m_recentFiles)
        {
            if (path.isEmpty())
            {
                continue;
            }

            const QFileInfo info(path);
            auto* action = m_recentFilesMenu->addAction(info.fileName().isEmpty() ? path : info.fileName());
            action->setData(path);
            action->setToolTip(path);
            connect(action, &QAction::triggered, this, &MainWindow::handleOpenRecentFile);
        }
    }

    m_recentFilesMenu->addSeparator();
    auto* clearAction = m_recentFilesMenu->addAction(tr("Clear Recent Files"), this, &MainWindow::handleClearRecentFiles);
    clearAction->setEnabled(!m_recentFiles.isEmpty());
}

QString MainWindow::dialogDirectory(const QString& lastDir) const
{
    if (!lastDir.isEmpty())
    {
        return lastDir;
    }
    return defaultDocumentsDirectory();
}

QString MainWindow::defaultDocumentsDirectory() const
{
    const QString documentsLocation = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
    if (!documentsLocation.isEmpty())
    {
        return documentsLocation;
    }
    return QDir::homePath();
}

void MainWindow::setDateFormatPreference(DateFormatPreference preference)
{
    if (m_dateFormatPreference == preference)
    {
        updateDateFormatActionState();
        return;
    }

    m_dateFormatPreference = preference;
    updateDateFormatActionState();
}

void MainWindow::updateDateFormatActionState()
{
    if (m_dateFormatShortAction)
    {
        const QSignalBlocker blocker(m_dateFormatShortAction);
        m_dateFormatShortAction->setChecked(m_dateFormatPreference == DateFormatPreference::Short);
    }
    if (m_dateFormatLongAction)
    {
        const QSignalBlocker blocker(m_dateFormatLongAction);
        m_dateFormatLongAction->setChecked(m_dateFormatPreference == DateFormatPreference::Long);
    }
}

QTextDocument::FindFlags MainWindow::buildFindFlags(QTextDocument::FindFlags baseFlags) const
{
    QTextDocument::FindFlags flags = baseFlags;
    if (m_lastCaseSensitivity == Qt::CaseSensitive)
    {
        flags |= QTextDocument::FindCaseSensitively;
    }
    return flags;
}

QIcon MainWindow::brandIcon() const
{
    QIcon icon = windowIcon();
    if (icon.isNull())
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
    if (!m_editor || term.isEmpty())
    {
        return false;
    }

    QTextCursor originalCursor = m_editor->textCursor();
    if (m_editor->find(term, flags))
    {
        return true;
    }

    QTextCursor searchCursor = originalCursor;
    if (flags.testFlag(QTextDocument::FindBackward))
    {
        searchCursor.movePosition(QTextCursor::End);
    }
    else
    {
        searchCursor.movePosition(QTextCursor::Start);
    }
    m_editor->setTextCursor(searchCursor);

    const bool foundAfterWrap = m_editor->find(term, flags);
    if (!foundAfterWrap)
    {
        m_editor->setTextCursor(originalCursor);
    }
    return foundAfterWrap;
}

bool MainWindow::replaceNextOccurrence(const QString& term, const QString& replacement, QTextDocument::FindFlags flags)
{
    if (!m_editor || term.isEmpty())
    {
        return false;
    }

    QTextCursor cursor = m_editor->textCursor();
    const bool selectionMatches = cursor.hasSelection() && QString::compare(cursor.selectedText(), term, m_lastCaseSensitivity) == 0;
    if (!selectionMatches)
    {
        if (!performFind(term, flags))
        {
            return false;
        }
        cursor = m_editor->textCursor();
    }

    cursor.insertText(replacement);
    m_editor->setTextCursor(cursor);
    return true;
}

int MainWindow::replaceAllOccurrences(const QString& term, const QString& replacement, QTextDocument::FindFlags flags)
{
    if (!m_editor || term.isEmpty())
    {
        return 0;
    }

    QTextCursor originalCursor = m_editor->textCursor();
    QTextCursor searchCursor = originalCursor;
    searchCursor.movePosition(QTextCursor::Start);
    m_editor->setTextCursor(searchCursor);

    int replacedCount = 0;
    while (m_editor->find(term, flags))
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
    switch (m_currentEncoding)
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
    if (m_hasBom)
    {
        label.append(tr(" BOM"));
    }
    return label;
}

QByteArray MainWindow::viewBomForEncoding(QStringConverter::Encoding encoding)
{
    switch (encoding)
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
    if (data.startsWith(QByteArray::fromHex("efbbbf")))
    {
        bomLength = 3;
        return QStringConverter::Utf8;
    }
    if (data.startsWith(QByteArray::fromHex("fffe")))
    {
        bomLength = 2;
        return QStringConverter::Utf16LE;
    }
    if (data.startsWith(QByteArray::fromHex("feff")))
    {
        bomLength = 2;
        return QStringConverter::Utf16BE;
    }

    return QStringConverter::Utf8;
}

#if defined(GNOTE_TEST_HOOKS)
void MainWindow::setSearchStateForTest(const QString& term, Qt::CaseSensitivity sensitivity, const QString& replacement)
{
    m_lastSearchTerm = term;
    m_lastCaseSensitivity = sensitivity;
    m_lastReplaceText = replacement;
}

bool MainWindow::testFindNext(QTextDocument::FindFlags extraFlags)
{
    if (m_lastSearchTerm.isEmpty())
    {
        return false;
    }
    return performFind(m_lastSearchTerm, buildFindFlags(extraFlags));
}

bool MainWindow::testFindPrevious()
{
    if (m_lastSearchTerm.isEmpty())
    {
        return false;
    }
    return performFind(m_lastSearchTerm, buildFindFlags(QTextDocument::FindBackward));
}

bool MainWindow::testReplaceNext(const QString& replacementOverride)
{
    if (m_lastSearchTerm.isEmpty())
    {
        return false;
    }
    const QString replacement = replacementOverride.isNull() ? m_lastReplaceText : replacementOverride;
    return replaceNextOccurrence(m_lastSearchTerm, replacement, buildFindFlags());
}

int MainWindow::testReplaceAll(const QString& term, const QString& replacement, QTextDocument::FindFlags extraFlags)
{
    return replaceAllOccurrences(term, replacement, buildFindFlags(extraFlags));
}
#endif

} // namespace GnotePad::ui
