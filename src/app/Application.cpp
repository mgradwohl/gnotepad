#include "app/Application.h"

#include <memory>

#include <QtCore/qcoreapplication.h>
#include <QtCore/qsettings.h>
#include <QtCore/qstring.h>
#include <QtCore/qstringliteral.h>
#include <QtGui/qguiapplication.h>
#include <QtGui/qicon.h>

#include <spdlog/spdlog.h>

#ifndef GNOTE_VERSION
#define GNOTE_VERSION "0.0.0-dev"
#endif

#include "ui/MainWindow.h"

namespace GnotePad
{

Application::Application(int& argc, char** argv) : QApplication(argc, argv)
{
    configureMetadata();
    QSettings::setDefaultFormat(QSettings::IniFormat);
    configureIcon();
    spdlog::info("GnotePad Application initialized");
}

Application::~Application() = default;

int Application::run()
{
    m_mainWindow = std::make_unique<ui::MainWindow>();
    if (!m_applicationIcon.isNull())
    {
        m_mainWindow->setWindowIcon(m_applicationIcon);
    }
    m_mainWindow->show();
    return exec();
}

// Intentionally non-static to keep parity with other setup helpers.
// NOLINTNEXTLINE(readability-convert-member-functions-to-static)
void Application::configureMetadata() const
{
    QCoreApplication::setOrganizationName("GnotePad");
    QCoreApplication::setOrganizationDomain("gnotepad.app");
    QCoreApplication::setApplicationName("GnotePad");
    QCoreApplication::setApplicationVersion(QString::fromLatin1(GNOTE_VERSION));
#if defined(Q_OS_LINUX)
    QGuiApplication::setDesktopFileName(QStringLiteral("gnotepad.desktop"));
#endif
}

void Application::configureIcon()
{
#if defined(Q_OS_LINUX)
    m_applicationIcon = QIcon::fromTheme(QStringLiteral("gnotepad"));
    if (m_applicationIcon.isNull())
    {
        m_applicationIcon = QIcon(QStringLiteral(":/gnotepad-icon.svg"));
    }
#else
    m_applicationIcon = QIcon(QStringLiteral(":/gnotepad-icon.svg"));
#endif
    if (m_applicationIcon.isNull())
    {
        spdlog::warn("Failed to load embedded application icon; UI will fall back to default icons");
    }
    else
    {
        setWindowIcon(m_applicationIcon);
    }
}

} // namespace GnotePad
