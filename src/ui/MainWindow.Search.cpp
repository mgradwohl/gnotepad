#include "ui/MainWindow.h"
#include "ui/TextEditor.h"

#include <QtCore/qtimer.h>
#include <QtGui/qtextcursor.h>
#include <QtGui/qtextdocument.h>
#include <QtGui/qtextobject.h>
#include <QtWidgets/qboxlayout.h>
#include <QtWidgets/qcheckbox.h>
#include <QtWidgets/qdialog.h>
#include <QtWidgets/qdialogbuttonbox.h>
#include <QtWidgets/qformlayout.h>
#include <QtWidgets/qinputdialog.h>
#include <QtWidgets/qlineedit.h>
#include <QtWidgets/qmessagebox.h>
#include <QtWidgets/qplaintextedit.h>
#include <QtWidgets/qpushbutton.h>

namespace GnotePad::ui
{

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

    // Dialog owns its child widgets; suppress ownership warnings for stack-local helpers.
    // NOLINTBEGIN(cppcoreguidelines-owning-memory)
    auto* const form = new QFormLayout(&dialog);
    auto* const findField = new QLineEdit(m_lastSearchTerm, &dialog);
    auto* const matchCase = new QCheckBox(tr("Match case"), &dialog);
    matchCase->setChecked(m_lastCaseSensitivity == Qt::CaseSensitive);

    form->addRow(tr("Find what:"), findField);
    form->addRow(QString(), matchCase);

    auto* const buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal, &dialog);
    form->addWidget(buttons);
    // NOLINTEND(cppcoreguidelines-owning-memory)

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

    // Dialog owns child controls created below; raw pointers mirror Qt patterns.
    // NOLINTBEGIN(cppcoreguidelines-owning-memory)
    auto* const layout = new QVBoxLayout(&dialog);
    auto* const formLayout = new QFormLayout();
    layout->addLayout(formLayout);

    auto* const findField = new QLineEdit(m_lastSearchTerm, &dialog);
    auto* const replaceField = new QLineEdit(m_lastReplaceText, &dialog);
    auto* const matchCase = new QCheckBox(tr("Match case"), &dialog);
    matchCase->setChecked(m_lastCaseSensitivity == Qt::CaseSensitive);

    formLayout->addRow(tr("Find what:"), findField);
    formLayout->addRow(tr("Replace with:"), replaceField);
    formLayout->addRow(QString(), matchCase);

    auto* const buttonsLayout = new QHBoxLayout();
    layout->addLayout(buttonsLayout);

    auto* const findNextButton = new QPushButton(tr("Find Next"), &dialog);
    auto* const replaceButton = new QPushButton(tr("Replace"), &dialog);
    auto* const replaceAllButton = new QPushButton(tr("Replace All"), &dialog);
    auto* const closeButton = new QPushButton(tr("Close"), &dialog);

    buttonsLayout->addWidget(findNextButton);
    buttonsLayout->addWidget(replaceButton);
    buttonsLayout->addWidget(replaceAllButton);
    buttonsLayout->addWidget(closeButton);
    // NOLINTEND(cppcoreguidelines-owning-memory)

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

    const QTextBlock block = document->findBlockByNumber(targetLine - 1);
    if (!block.isValid())
    {
        return;
    }

    QTextCursor cursor(block);
    cursor.movePosition(QTextCursor::StartOfLine);
    m_editor->setTextCursor(cursor);
    m_editor->centerCursor();
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

bool MainWindow::performFind(const QString& term, QTextDocument::FindFlags flags)
{
    if (!m_editor || term.isEmpty())
    {
        return false;
    }

    const QTextCursor originalCursor = m_editor->textCursor();
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

    const QTextCursor originalCursor = m_editor->textCursor();
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
