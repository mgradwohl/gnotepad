#include "ui/MainWindow.h"

#include <algorithm>

#include <QtCore/qcoreapplication.h>
#include <QtCore/qdatetime.h>
#include <QtCore/qfileinfo.h>
#include <QtCore/qlocale.h>
#include <QtCore/qsignalblocker.h>
#include <QtCore/qstring.h>
#include <QtCore/qstringliteral.h>
#include <QtCore/qstringlist.h>
#include <QtCore/qstringconverter.h>
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
#include <QtGui/qtextcursor.h>
#include <QtGui/qtextdocument.h>
#include <QtGui/qtextoption.h>
#include <QtPrintSupport/qprinter.h>
#include <QtWidgets/qboxlayout.h>
#include <QtWidgets/qdialog.h>
#include <QtWidgets/qdialogbuttonbox.h>
#include <QtWidgets/qfiledialog.h>
#include <QtWidgets/qfontdialog.h>
#include <QtWidgets/qinputdialog.h>
#include <QtWidgets/qlabel.h>
#include <QtWidgets/qmenu.h>
#include <QtWidgets/qmenubar.h>
#include <QtWidgets/qmessagebox.h>
#include <QtWidgets/qplaintextedit.h>
#include <QtWidgets/qstatusbar.h>
#include <QtWidgets/qwidget.h>

#include <spdlog/spdlog.h>

#include "ui/PrintSupport.h"
#include "ui/TextEditor.h"

// NOTE: Qt's parent-child memory management deletes QObjects given a parent, so raw pointer
// assignments of child widgets in this file are intentional despite the cppcoreguidelines-owning-memory warning.

namespace GnotePad::ui
{

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent)
{
    buildEditor();
    buildMenus();
    buildStatusBar();
    wireSignals();

    resize(DefaultWindowWidth, DefaultWindowHeight);
    loadSettings();
    resetDocumentState();
}

void MainWindow::buildEditor()
{
    // Qt parents clean up child widgets; suppress ownership warning for intentional raw pointer.
    // NOLINTNEXTLINE(cppcoreguidelines-owning-memory)
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
    m_saveAction = fileMenu->addAction(tr("&Save"), QKeySequence::Save, this, &MainWindow::handleSaveFile);
    m_saveAsAction = fileMenu->addAction(tr("Save &As…"), QKeySequence::SaveAs, this, &MainWindow::handleSaveFileAs);
    fileMenu->addAction(tr("E&ncoding…"), this, &MainWindow::handleChangeEncoding);
    fileMenu->addSeparator();
    m_printAction = fileMenu->addAction(tr("&Print"), QKeySequence::Print, this, &MainWindow::handlePrint);
    fileMenu->addSeparator();
    fileMenu->addAction(tr("E&xit"), QKeySequence::Quit, this, &QWidget::close);

    editMenu->addAction(tr("&Undo"), QKeySequence::Undo, m_editor, &QPlainTextEdit::undo);
    m_cutAction = editMenu->addAction(tr("Cu&t"), QKeySequence::Cut, m_editor, &QPlainTextEdit::cut);
    m_copyAction = editMenu->addAction(tr("&Copy"), QKeySequence::Copy, m_editor, &QPlainTextEdit::copy);
    editMenu->addAction(tr("&Paste"), QKeySequence::Paste, m_editor, &QPlainTextEdit::paste);
    m_deleteAction = editMenu->addAction(tr("De&lete"), m_editor, &QPlainTextEdit::cut);
    editMenu->addSeparator();
    m_findAction = editMenu->addAction(tr("&Find…"), QKeySequence::Find, this, &MainWindow::handleFind);
    m_findNextAction = editMenu->addAction(tr("Find &Next"), QKeySequence(Qt::Key_F3), this, &MainWindow::handleFindNext);
    m_findPreviousAction =
        editMenu->addAction(tr("Find &Previous"), QKeySequence(Qt::SHIFT | Qt::Key_F3), this, &MainWindow::handleFindPrevious);
    m_replaceAction = editMenu->addAction(tr("&Replace…"), QKeySequence::Replace, this, &MainWindow::handleReplace);
    m_goToAction = editMenu->addAction(tr("&Go To…"), QKeySequence(Qt::CTRL | Qt::Key_G), this, &MainWindow::handleGoToLine);
    editMenu->addSeparator();
    editMenu->addAction(tr("Select &All"), QKeySequence::SelectAll, m_editor, &QPlainTextEdit::selectAll);
    m_timeDateAction = editMenu->addAction(tr("Time/&Date"), QKeySequence(Qt::Key_F5), this, &MainWindow::handleInsertTimeDate);

    m_wordWrapAction = formatMenu->addAction(tr("&Word Wrap"));
    m_wordWrapAction->setCheckable(true);
    connect(m_wordWrapAction, &QAction::toggled, this,
            [this](bool checked) { m_editor->setWordWrapMode(checked ? QTextOption::WordWrap : QTextOption::NoWrap); });

    formatMenu->addAction(tr("&Font…"), this, &MainWindow::handleChooseFont);
    formatMenu->addAction(tr("Tab &Size…"), this, &MainWindow::handleSetTabSize);

    auto* dateFormatMenu = formatMenu->addMenu(tr("Time/&Date Format"));
    // NOLINTNEXTLINE(cppcoreguidelines-owning-memory)  // QActionGroups are owned by their QObject parent.
    auto* dateFormatGroup = new QActionGroup(this);
    dateFormatGroup->setExclusive(true);

    m_dateFormatShortAction = dateFormatMenu->addAction(tr("&Short"));
    m_dateFormatShortAction->setCheckable(true);
    m_dateFormatShortAction->setActionGroup(dateFormatGroup);
    connect(m_dateFormatShortAction, &QAction::triggered, this, [this]() { setDateFormatPreference(DateFormatPreference::Short); });

    m_dateFormatLongAction = dateFormatMenu->addAction(tr("&Long"));
    m_dateFormatLongAction->setCheckable(true);
    m_dateFormatLongAction->setActionGroup(dateFormatGroup);
    connect(m_dateFormatLongAction, &QAction::triggered, this, [this]() { setDateFormatPreference(DateFormatPreference::Long); });

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

    helpMenu->addAction(tr("View &Help"), QKeySequence::HelpContents, this, &MainWindow::handleViewHelp);
    helpMenu->addAction(tr("&About GnotePad"), this, &MainWindow::showAboutDialog);

    updateActionStates();
}

void MainWindow::buildStatusBar()
{
    m_statusBar = statusBar();

    // Status labels live as QObject children of the window; Qt deletes them with the parent.
    // NOLINTBEGIN(cppcoreguidelines-owning-memory)
    m_cursorLabel = new QLabel(tr("Ln 1, Col 1"), this);
    m_documentStatsLabel = new QLabel(tr("Length: 0  Lines: 1"), this);
    m_encodingLabel = new QLabel(tr("UTF-8"), this);
    const QString defaultZoomText = tr("%1%").arg(DefaultZoomPercent);
    m_zoomLabel = new QLabel(defaultZoomText, this);
    // NOLINTEND(cppcoreguidelines-owning-memory)

    m_statusBar->addPermanentWidget(m_cursorLabel);
    m_statusBar->addPermanentWidget(m_documentStatsLabel);
    m_statusBar->addPermanentWidget(m_encodingLabel);
    m_statusBar->addPermanentWidget(m_zoomLabel);

    updateEncodingDisplay(encodingLabel());
    updateDocumentStats();
    updateZoomLabel(DefaultZoomPercent);
}

void MainWindow::wireSignals()
{
    connect(m_editor, &QPlainTextEdit::cursorPositionChanged, this, &MainWindow::handleUpdateCursorStatus);
    connect(m_editor, &QPlainTextEdit::textChanged, this, &MainWindow::updateDocumentStats);
    connect(m_editor, &QPlainTextEdit::textChanged, this, &MainWindow::updateActionStates);
    connect(m_editor, &QPlainTextEdit::selectionChanged, this, &MainWindow::updateActionStates);
    connect(m_editor, &TextEditor::zoomPercentageChanged, this, &MainWindow::updateZoomLabel);
    if (m_editor && m_editor->document())
    {
        connect(m_editor->document(), &QTextDocument::modificationChanged, this,
            [this](bool)
            {
                updateWindowTitle();
                updateActionStates();
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
    const int newSize = QInputDialog::getInt(this, tr("Tab Size"), tr("Spaces per tab:"), currentSize, MinTabSizeSpaces, MaxTabSizeSpaces,
                            TabSizeStep, &accepted);
    if (!accepted || newSize == currentSize)
    {
        return;
    }

    m_tabSizeSpaces = newSize;
    m_editor->setTabSizeSpaces(newSize);
    spdlog::info("Tab size updated to {} spaces", newSize);
}

void MainWindow::handleChooseFont()
{
    if (!m_editor)
    {
        return;
    }

    QFontDialog dialog(m_editor->font(), this);
    // uncommenting the line below will force the use of Qt's font dialog instead of the native platform dialog
    // dialog.setOption(QFontDialog::DontUseNativeDialog, true); // force Qt dialog

    // these lines are ignored when using the native dialog
    dialog.resize(FontDialogWidth, FontDialogHeight);
    dialog.setMinimumSize(FontDialogWidth, FontDialogHeight);

    // Configure options
    dialog.setOption(QFontDialog::ScalableFonts, true);
    dialog.setOption(QFontDialog::NonScalableFonts, true);
    dialog.setOption(QFontDialog::MonospacedFonts, true);
    dialog.setOption(QFontDialog::ProportionalFonts, true);
    // Ensure dialog takes focus from parent
    dialog.setWindowModality(Qt::WindowModal);
    dialog.setFocusPolicy(Qt::StrongFocus);
    dialog.raise();
    dialog.activateWindow();

    if (dialog.exec() == QDialog::Accepted)
    {
        m_editor->applyEditorFont(dialog.selectedFont());
    }
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

void MainWindow::handleViewHelp()
{
    const QUrl helpUrl(QStringLiteral("https://github.com/mgradwohl/GnotePad#readme"));
    if (!QDesktopServices::openUrl(helpUrl))
    {
        QMessageBox::information(this, tr("Help"), tr("Open %1 in your browser for the latest documentation.").arg(helpUrl.toString()));
    }
}

void MainWindow::showAboutDialog()
{
    const QString appName = QCoreApplication::applicationName();
    const QString version = QCoreApplication::applicationVersion();
    const QString org = QCoreApplication::organizationName();
    const QString maintainer = org.isEmpty() ? tr("the GnotePad contributors") : org;
    const QString details = tr("<p><b>%1</b> %2</p>"
                               "<p>Modern Qt 6 / C++23 refresh of the Windows Notepad experience for Linux, Windows, and macOS.</p>"
                               "<p>Maintained by %3 and built with Qt %4.</p>"
                               "<p>Source & documentation: <a href=\"https://github.com/mgradwohl/GnotePad\">github.com/mgradwohl/GnotePad</a></p>"
                               "<p>Licensed under the MIT License. Not affiliated with the legacy gnotepad or gnotepad+ projects.</p>"
                               "<p>Contributions, bug reports, and packaging help are welcome!</p>")
                                .arg(appName, version, maintainer, QString::fromLatin1(qVersion()));
    const QIcon icon = brandIcon();

    QDialog dialog(this);
    dialog.setWindowTitle(tr("About %1").arg(appName));
    dialog.setModal(true);
    dialog.setWindowIcon(icon);

    // get the icon as a larger image (dialog owns controls below)
    // NOLINTBEGIN(cppcoreguidelines-owning-memory)
    QLabel* iconLabel = new QLabel(&dialog);
    QPixmap aboutPixmap;
    if (!icon.isNull())
    {
        spdlog::info("About dialog: using icon for branding.");
        aboutPixmap = icon.pixmap(AboutDialogIconSize, AboutDialogIconSize);
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
    textLabel->setMinimumWidth(AboutDialogMinTextWidth);

    contentLayout->addWidget(textLabel, 1);

    auto* buttons = new QDialogButtonBox(QDialogButtonBox::Ok, Qt::Horizontal, &dialog);
    connect(buttons, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    layout->addWidget(buttons);
    // NOLINTEND(cppcoreguidelines-owning-memory)

    dialog.exec();
}

void MainWindow::handlePrint()
{
    if (!m_editor)
    {
        return;
    }

    const QString displayName = m_currentFilePath.isEmpty() ? tr(UntitledDocumentTitle) : QFileInfo(m_currentFilePath).fileName();
    PrintSupport::showPrintPreview(this, m_editor, displayName, m_editor->lineNumbersVisible());
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

void MainWindow::updateActionStates()
{
    const bool hasContent = documentHasContent();
    const bool hasSelection = editorHasSelection();

    if (m_saveAction)
    {
        m_saveAction->setEnabled(hasContent);
    }
    if (m_saveAsAction)
    {
        m_saveAsAction->setEnabled(hasContent);
    }
    if (m_printAction)
    {
        m_printAction->setEnabled(hasContent);
    }
    if (m_findAction)
    {
        m_findAction->setEnabled(hasContent);
    }
    if (m_findNextAction)
    {
        m_findNextAction->setEnabled(hasContent);
    }
    if (m_findPreviousAction)
    {
        m_findPreviousAction->setEnabled(hasContent);
    }
    if (m_replaceAction)
    {
        m_replaceAction->setEnabled(hasContent);
    }
    if (m_goToAction)
    {
        m_goToAction->setEnabled(hasContent);
    }
    if (m_cutAction)
    {
        m_cutAction->setEnabled(hasSelection);
    }
    if (m_copyAction)
    {
        m_copyAction->setEnabled(hasSelection);
    }
    if (m_deleteAction)
    {
        m_deleteAction->setEnabled(hasSelection);
    }
    if (m_wordWrapAction)
    {
        m_wordWrapAction->setEnabled(true);
    }
    if (m_statusBarToggle)
    {
        m_statusBarToggle->setEnabled(true);
    }
}

bool MainWindow::documentHasContent() const
{
    if (!m_editor)
    {
        return false;
    }

    const auto* document = m_editor->document();
    return document && !document->isEmpty();
}

bool MainWindow::editorHasSelection() const
{
    return m_editor && m_editor->textCursor().hasSelection();
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

} // namespace GnotePad::ui
