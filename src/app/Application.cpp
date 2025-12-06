#include "app/Application.h"

#include <memory>

#include <QtCore/qcommandlineoption.h>
#include <QtCore/qcommandlineparser.h>
#include <QtCore/qcoreapplication.h>
#include <QtCore/qsettings.h>
#include <QtCore/qstring.h>
#include <QtCore/qstringliteral.h>
#include <QtCore/qtimer.h>
#include <QtGui/qguiapplication.h>
#include <QtGui/qicon.h>

#include <spdlog/spdlog.h>

#ifndef GNOTE_VERSION
#define GNOTE_VERSION "0.0.0-dev"
#endif

#include "ui/MainWindow.h"

namespace
{
constexpr int kQuitAfterInitDelayMs = 2000;
}

namespace GnotePad
{

Application::Application(int& argc, char** argv) : QApplication(argc, argv)
{
    configureMetadata();
    QSettings::setDefaultFormat(QSettings::IniFormat);
    configureIcon();
    parseCommandLine(arguments());
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
    if (m_quitAfterInit)
    {
        spdlog::info("Headless smoke flag detected; quitting shortly after startup");
        QTimer::singleShot(kQuitAfterInitDelayMs, this, &QCoreApplication::quit);
    }
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
    // Qt warns if the desktop file name includes the ".desktop" suffix; provide the id only.
    QGuiApplication::setDesktopFileName(QStringLiteral("app.gnotepad.GnotePad"));
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

void Application::parseCommandLine(const QStringList& arguments)
{
    QCommandLineParser parser;
    parser.setApplicationDescription(QStringLiteral("GnotePad - A modern Qt text editor"));
    parser.addHelpOption();
    parser.addVersionOption();
    parser.setSingleDashWordOptionMode(QCommandLineParser::ParseAsLongOptions);
    
    QCommandLineOption quitAfterInitOption(
        {QStringLiteral("quit-after-init"), QStringLiteral("headless-smoke")},
        QStringLiteral("Quit shortly after startup (useful for headless smoke tests)."));

    parser.addOption(quitAfterInitOption);
    parser.process(arguments);

    m_quitAfterInit = parser.isSet(quitAfterInitOption);
}

} // namespace GnotePad
