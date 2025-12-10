#include "ui/MainWindow.h"

#include "ui/TextEditor.h"

#include <spdlog/spdlog.h>

#include <QtCore/qdir.h>
#include <QtCore/qfileinfo.h>
#include <QtCore/qsettings.h>
#include <QtCore/qstandardpaths.h>
#include <QtCore/qstringlist.h>
#include <QtGui/qfont.h>
#include <QtGui/qtextoption.h>
#include <QtPrintSupport/qprinterinfo.h>
#include <QtWidgets/qaction.h>
#include <QtWidgets/qcombobox.h>
#include <QtWidgets/qdialog.h>
#include <QtWidgets/qdialogbuttonbox.h>
#include <QtWidgets/qformlayout.h>
#include <QtWidgets/qlabel.h>
#include <QtWidgets/qmenu.h>
#include <QtWidgets/qstatusbar.h>

#include <QSignalBlocker>
#include <algorithm>

namespace
{
    constexpr int MaxRecentFiles = 10;
}

namespace GnotePad::ui
{

    void MainWindow::handleOpenRecentFile()
    {
        const auto* action = qobject_cast<const QAction*>(sender());
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

    void MainWindow::loadSettings()
    {
        QSettings settings;
        const QFileInfo settingsFile(settings.fileName());
        const bool hasExistingPreferences = settingsFile.exists();

        loadWindowGeometrySettings(settings);
        loadPathSettings(settings);
        loadRecentFilesSettings(settings);
        loadEditorFontSettings(settings, hasExistingPreferences);
        loadEditorViewSettings(settings);
        loadEditorBehaviorSettings(settings);
        loadPrinterSettings(settings);
    }

    void MainWindow::saveSettings() const
    {
        QSettings settings;

        saveWindowGeometrySettings(settings);
        savePathSettings(settings);
        saveRecentFilesSettings(settings);
        saveEditorFontSettings(settings);
        saveEditorBehaviorSettings(settings);
        savePrinterSettings(settings);
        clearLegacySettings(settings);
    }

    void MainWindow::loadWindowGeometrySettings(QSettings& settings)
    {
        const bool hasRectKeys = settings.contains("window/posX") && settings.contains("window/posY") &&
                                 settings.contains("window/width") && settings.contains("window/height");

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

        const bool windowMaximized = settings.value("window/maximized", false).toBool();
        setWindowState(windowMaximized ? Qt::WindowMaximized : Qt::WindowNoState);
    }

    void MainWindow::loadPathSettings(QSettings& settings)
    {
        m_lastOpenDirectory = settings.value("paths/lastOpenDirectory").toString();
        m_lastSaveDirectory = settings.value("paths/lastSaveDirectory").toString();
    }

    void MainWindow::loadRecentFilesSettings(QSettings& settings)
    {
        m_recentFiles = settings.value("documents/recentFiles").toStringList();
        while (m_recentFiles.size() > MaxRecentFiles)
        {
            m_recentFiles.removeLast();
        }
        refreshRecentFilesMenu();
    }

    void MainWindow::loadEditorFontSettings(QSettings& settings, bool hasExistingPreferences)
    {
        if (!m_editor)
        {
            return;
        }

        const QString fontFamily = settings.value("editor/fontFamily").toString();
        const qreal fontPointSize = settings.value("editor/fontPointSize", InvalidFontPointSize).toDouble();

        if (!fontFamily.isEmpty())
        {
            QFont storedFont(fontFamily);
            if (fontPointSize > 0)
            {
                storedFont.setPointSizeF(fontPointSize);
            }
            m_editor->applyEditorFont(storedFont);
            return;
        }

        if (settings.contains("editor/font"))
        {
            const QFont legacyFont = settings.value("editor/font").value<QFont>();
            if (!legacyFont.family().isEmpty())
            {
                m_editor->applyEditorFont(legacyFont);
                return;
            }
        }

        if (!hasExistingPreferences)
        {
            applyDefaultEditorFont();
        }
    }

    void MainWindow::loadEditorViewSettings(QSettings& settings)
    {
        const bool lineNumbersVisible = settings.value("editor/lineNumbersVisible", true).toBool();
        if (m_editor)
        {
            m_editor->setLineNumbersVisible(lineNumbersVisible);
        }
        if (m_lineNumberToggle)
        {
            const QSignalBlocker blocker(m_lineNumberToggle);
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
            const QSignalBlocker blocker(m_statusBarToggle);
            m_statusBarToggle->setChecked(statusBarVisible);
        }
    }

    void MainWindow::loadEditorBehaviorSettings(QSettings& settings)
    {
        m_tabSizeSpaces = std::clamp(settings.value("editor/tabSizeSpaces", m_tabSizeSpaces).toInt(), MinTabSizeSpaces, MaxTabSizeSpaces);
        if (m_editor)
        {
            m_editor->setTabSizeSpaces(m_tabSizeSpaces);
        }

        const int encodingValue = settings.value("editor/defaultEncoding", static_cast<int>(m_currentEncoding)).toInt();
        const bool bom = settings.value("editor/defaultBom", m_hasBom).toBool();
        applyEncodingSelection(static_cast<QStringConverter::Encoding>(encodingValue), bom);

        const int zoomPercent = settings.value("editor/zoomPercent", m_currentZoomPercent).toInt();
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

    void MainWindow::saveWindowGeometrySettings(QSettings& settings) const
    {
        const QRect windowRect = isMaximized() ? normalGeometry() : geometry();
        settings.setValue("window/posX", windowRect.x());
        settings.setValue("window/posY", windowRect.y());
        settings.setValue("window/width", windowRect.width());
        settings.setValue("window/height", windowRect.height());
        settings.setValue("window/maximized", isMaximized());
    }

    void MainWindow::savePathSettings(QSettings& settings) const
    {
        settings.setValue("paths/lastOpenDirectory", m_lastOpenDirectory);
        settings.setValue("paths/lastSaveDirectory", m_lastSaveDirectory);
    }

    void MainWindow::saveRecentFilesSettings(QSettings& settings) const
    {
        settings.setValue("documents/recentFiles", m_recentFiles);
    }

    void MainWindow::saveEditorFontSettings(QSettings& settings) const
    {
        if (m_editor)
        {
            const QFont editorFont = m_editor->font();
            settings.setValue("editor/fontFamily", editorFont.family());
            settings.setValue("editor/fontPointSize", editorFont.pointSizeF());
            settings.setValue("editor/lineNumbersVisible", m_editor->lineNumbersVisible());
            settings.setValue("editor/wordWrap", m_editor->wordWrapMode() != QTextOption::NoWrap);
            return;
        }

        settings.remove("editor/fontFamily");
        settings.remove("editor/fontPointSize");
        settings.setValue("editor/lineNumbersVisible", true);
        settings.setValue("editor/wordWrap", false);
    }

    void MainWindow::saveEditorBehaviorSettings(QSettings& settings) const
    {
        settings.setValue("editor/tabSizeSpaces", m_tabSizeSpaces);
        settings.setValue("editor/statusBarVisible", m_statusBar ? m_statusBar->isVisible() : true);
        settings.setValue("editor/defaultEncoding", static_cast<int>(m_currentEncoding));
        settings.setValue("editor/defaultBom", m_hasBom);
        settings.setValue("editor/zoomPercent", m_currentZoomPercent);
        settings.setValue("editor/dateFormat",
                          m_dateFormatPreference == DateFormatPreference::Long ? QStringLiteral("long") : QStringLiteral("short"));
    }

    void MainWindow::clearLegacySettings(QSettings& settings)
    {
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

    QString MainWindow::defaultDocumentsDirectory()
    {
        QString documentsLocation = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
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

    void MainWindow::loadPrinterSettings(QSettings& settings)
    {
        const QString savedPrinter = settings.value("printer/defaultPrinter").toString();

        // Validate that the saved printer still exists
        if (!savedPrinter.isEmpty())
        {
            const QList<QPrinterInfo> printers = QPrinterInfo::availablePrinters();
            const bool printerExists = std::any_of(
                printers.begin(), printers.end(), [&savedPrinter](const QPrinterInfo& info) { return info.printerName() == savedPrinter; });

            if (printerExists)
            {
                m_defaultPrinterName = savedPrinter;
                spdlog::info("Loaded printer preference: {}", savedPrinter.toStdString());
                return;
            }
            spdlog::warn("Saved printer '{}' no longer available", savedPrinter.toStdString());
        }

        // No saved preference or printer gone - use system default (empty string)
        m_defaultPrinterName.clear();
    }

    void MainWindow::savePrinterSettings(QSettings& settings) const
    {
        if (m_defaultPrinterName.isEmpty())
        {
            settings.remove("printer/defaultPrinter");
        }
        else
        {
            settings.setValue("printer/defaultPrinter", m_defaultPrinterName);
        }
    }

    void MainWindow::handleChoosePrinter()
    {
        const QList<QPrinterInfo> printers = QPrinterInfo::availablePrinters();
        if (printers.isEmpty())
        {
            QMessageBox::information(this, tr("No Printers"), tr("No printers are available on this system."));
            return;
        }

        // Build dialog
        QDialog dialog(this);
        dialog.setWindowTitle(tr("Choose Printer"));
        dialog.setMinimumWidth(350);

        // Qt parents clean up child widgets; suppress ownership warning for intentional raw pointer.
        // NOLINTNEXTLINE(cppcoreguidelines-owning-memory)
        auto* layout = new QFormLayout(&dialog);

        // Qt parents clean up child widgets; suppress ownership warning for intentional raw pointer.
        // NOLINTNEXTLINE(cppcoreguidelines-owning-memory)
        auto* printerCombo = new QComboBox(&dialog);

        // Add "Use System Default" option first
        const QPrinterInfo systemDefault = QPrinterInfo::defaultPrinter();
        const QString systemDefaultLabel =
            systemDefault.isNull() ? tr("(System Default)") : tr("(System Default: %1)").arg(systemDefault.printerName());
        printerCombo->addItem(systemDefaultLabel, QString{});

        // Add all available printers
        int currentIndex = 0;
        for (const QPrinterInfo& info : printers)
        {
            const QString name = info.printerName();
            printerCombo->addItem(name, name);

            if (!m_defaultPrinterName.isEmpty() && name == m_defaultPrinterName)
            {
                currentIndex = printerCombo->count() - 1;
            }
        }
        printerCombo->setCurrentIndex(currentIndex);

        layout->addRow(tr("Printer:"), printerCombo);

        // Qt parents clean up child widgets; suppress ownership warning for intentional raw pointer.
        // NOLINTNEXTLINE(cppcoreguidelines-owning-memory)
        auto* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dialog);
        connect(buttonBox, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
        connect(buttonBox, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
        layout->addRow(buttonBox);

        if (dialog.exec() == QDialog::Accepted)
        {
            m_defaultPrinterName = printerCombo->currentData().toString();
            if (m_defaultPrinterName.isEmpty())
            {
                spdlog::info("Printer preference cleared (using system default)");
            }
            else
            {
                spdlog::info("Printer preference set to: {}", m_defaultPrinterName.toStdString());
            }
        }
    }

} // namespace GnotePad::ui
