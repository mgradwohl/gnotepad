#include "ui/MainWindow.h"

#include "app/Application.h"
#include "ui/TextEditor.h"

#include <spdlog/spdlog.h>

#include <QtCore/qbytearray.h>
#include <QtCore/qfile.h>
#include <QtCore/qfileinfo.h>
#include <QtCore/qiodevice.h>
#include <QtCore/qsavefile.h>
#include <QtCore/qstring.h>
#include <QtCore/qstringconverter.h>
#include <QtCore/qstringliteral.h>
#include <QtWidgets/qfiledialog.h>
#include <QtWidgets/qinputdialog.h>
#include <QtWidgets/qmessagebox.h>

#include <algorithm>
#include <array>

namespace GnotePad::ui
{

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

    bool MainWindow::loadDocumentFromPath(const QString& filePath)
    {
        QFile file(filePath);
        if (!file.open(QIODevice::ReadOnly))
        {
            if (!GnotePad::Application::isHeadlessSmokeMode())
            {
                QMessageBox::warning(this, tr("Open File"), tr("Unable to open %1").arg(filePath));
            }
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
            if (!GnotePad::Application::isHeadlessSmokeMode())
            {
                QMessageBox::warning(this, tr("Open File"), tr("Unsupported encoding in %1").arg(filePath));
            }
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
        updateActionStates();
        return true;
    }

    bool MainWindow::saveDocumentToPath(const QString& filePath)
    {
        if (filePath.isEmpty())
        {
            spdlog::warn("saveDocumentToPath called without a file path; refusing to save.");
            Q_ASSERT(!filePath.isEmpty());
            return false;
        }

        QSaveFile file(filePath);
        if (!file.open(QIODevice::WriteOnly))
        {
            if (!GnotePad::Application::isHeadlessSmokeMode())
            {
                QMessageBox::warning(this, tr("Save File"), tr("Unable to save %1").arg(filePath));
            }
            spdlog::error("Failed to open {} for writing", filePath.toStdString());
            return false;
        }

        QStringEncoder encoder(m_currentEncoding);
        const QString text = m_editor ? m_editor->toPlainText() : QString();
        const QByteArray encoded = encoder(text);
        if (encoder.hasError())
        {
            if (!GnotePad::Application::isHeadlessSmokeMode())
            {
                QMessageBox::warning(this, tr("Save File"), tr("Unable to encode document using %1").arg(encodingLabel()));
            }
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
            if (!GnotePad::Application::isHeadlessSmokeMode())
            {
                QMessageBox::warning(this, tr("Save File"), tr("Failed to write data to %1").arg(filePath));
            }
            spdlog::error("Short write while saving {}", filePath.toStdString());
            return false;
        }

        if (!file.commit())
        {
            if (!GnotePad::Application::isHeadlessSmokeMode())
            {
                QMessageBox::warning(this, tr("Save File"), tr("Failed to finalize %1").arg(filePath));
            }
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
        updateActionStates();
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
        updateActionStates();
    }

    bool MainWindow::confirmReadyForDestructiveAction()
    {
        if (!m_editor || !m_editor->document()->isModified())
        {
            return true;
        }

#ifdef GNOTE_TEST_HOOKS
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
            EncodingChoice{.label = tr("UTF-8 (no BOM)"), .encoding = QStringConverter::Utf8, .includeBom = false},
            EncodingChoice{.label = tr("UTF-8 with BOM"), .encoding = QStringConverter::Utf8, .includeBom = true},
            EncodingChoice{.label = tr("UTF-16 LE"), .encoding = QStringConverter::Utf16LE, .includeBom = true},
            EncodingChoice{.label = tr("UTF-16 BE"), .encoding = QStringConverter::Utf16BE, .includeBom = true},
        };

        QStringList labels;
        labels.reserve(choices.size());
        int currentIndex = 0;
        int candidateIndex = 0;
        for (const auto& choice : choices)
        {
            labels.append(choice.label);
            if (choice.encoding == encoding && choice.includeBom == bom)
            {
                currentIndex = candidateIndex;
            }
            ++candidateIndex;
        }

        bool accepted = false;
        const auto selection = QInputDialog::getItem(this, tr("Select Encoding"), tr("Encoding:"), labels, currentIndex, false, &accepted);
        if (!accepted)
        {
            return false;
        }

        // Iterator type may change with container tweaks; keep auto for flexibility.
        // NOLINTNEXTLINE(readability-qualified-auto)
        const auto matchIt = std::find_if(choices.cbegin(), choices.cend(), [&](const auto& choice) { return choice.label == selection; });

        if (matchIt == choices.cend())
        {
            return false;
        }

        encoding = matchIt->encoding;
        bom = matchIt->includeBom;
        return true;
    }

    void MainWindow::applyEncodingSelection(QStringConverter::Encoding encoding, bool bom)
    {
        m_currentEncoding = encoding;
        m_hasBom = bom;
        updateEncodingDisplay(encodingLabel());
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

} // namespace GnotePad::ui
