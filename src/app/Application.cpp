#include "app/Application.h"

#if defined(_WIN32)
#include <windows.h>
#endif

#include <spdlog/spdlog.h>

#include <QtCore/qcommandlineoption.h>
#include <QtCore/qcommandlineparser.h>
#include <QtCore/qcoreapplication.h>
#include <QtCore/qsettings.h>
#include <QtCore/qstring.h>
#include <QtCore/qstringliteral.h>
#include <QtCore/qtimer.h>
#include <QtGui/qguiapplication.h>
#include <QtGui/qicon.h>
#include <QtWidgets/qstyle.h>
#include <QtWidgets/qstylefactory.h>

#include <memory>
#if defined(_WIN32)
#include <spdlog/sinks/msvc_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#endif

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

    bool Application::s_headlessSmokeMode = false;

    Application::Application(int& argc, char** argv) : QApplication(argc, argv)
    {
#if defined(_WIN32) && !defined(NDEBUG)
        AttachConsole(ATTACH_PARENT_PROCESS);
        // Route logs to both the debugger output window and the console (Windows Terminal).
        auto msvcSink = std::make_shared<spdlog::sinks::msvc_sink_mt>();
        auto consoleSink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
        auto dualSinkLogger = std::make_shared<spdlog::logger>("debugger+console", spdlog::sinks_init_list{msvcSink, consoleSink});
        spdlog::set_default_logger(dualSinkLogger);
#endif
#if !defined(NDEBUG)
        spdlog::set_level(spdlog::level::debug);
        spdlog::flush_on(spdlog::level::debug);
#endif
        configureMetadata();
        QSettings::setDefaultFormat(QSettings::IniFormat);
        configureIcon();
        parseCommandLine(arguments());
        spdlog::info("GnotePad Application initialized");
    }

    Application::~Application() = default;

    int Application::run()
    {
        configureStyle();

        const auto platformName = QGuiApplication::platformName();
        const auto styles = QStyleFactory::keys();
        const auto* currentStyle = qApp->style();
        const std::string currentStyleName = currentStyle != nullptr ? currentStyle->objectName().toStdString() : std::string("<none>");

        spdlog::debug("Qt platform: {}", platformName.toStdString());
        spdlog::debug("Available Qt styles: {}", styles.join(", ").toStdString());
        spdlog::debug("Current Qt style: {}", currentStyleName);

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
        QCommandLineOption quitAfterInitOption({QStringLiteral("quit-after-init"), QStringLiteral("headless-smoke")},
                                               QStringLiteral("Quit shortly after startup (useful for headless smoke tests)."));

        parser.addOption(quitAfterInitOption);
        parser.process(arguments);

        m_quitAfterInit = parser.isSet(quitAfterInitOption);
        s_headlessSmokeMode = m_quitAfterInit;
    }

    bool Application::isHeadlessSmokeMode()
    {
        return s_headlessSmokeMode;
    }

    void Application::configureStyle()
    {
        const QStringList availableStyles = QStyleFactory::keys();

#if defined(_WIN32)
        // Windows: prefer windows11, fallback to fusion, then default
        if (availableStyles.contains(QStringLiteral("windows11"), Qt::CaseInsensitive))
        {
            QApplication::setStyle(QStringLiteral("windows11"));
            spdlog::debug("Qt style set to 'windows11'");
        }
        else if (availableStyles.contains(QStringLiteral("fusion"), Qt::CaseInsensitive))
        {
            QApplication::setStyle(QStringLiteral("fusion"));
            spdlog::debug("Qt style 'windows11' not available; using 'fusion' instead");
        }
        else
        {
            spdlog::debug("Qt styles 'windows11' and 'fusion' not available; using default style");
        }
#elif defined(Q_OS_LINUX)
        // Linux: prefer fusion, fallback to windows, then default
        if (availableStyles.contains(QStringLiteral("fusion"), Qt::CaseInsensitive))
        {
            QApplication::setStyle(QStringLiteral("fusion"));
            spdlog::debug("Qt style set to 'fusion'");
        }
        else if (availableStyles.contains(QStringLiteral("windows"), Qt::CaseInsensitive))
        {
            QApplication::setStyle(QStringLiteral("windows"));
            spdlog::debug("Qt style 'fusion' not available; using 'windows' instead");
        }
        else
        {
            spdlog::debug("Qt styles 'fusion' and 'windows' not available; using default style");
        }
#else
        // macOS and other platforms: use default style
        spdlog::debug("Using default Qt style for this platform");
#endif
    }

} // namespace GnotePad
