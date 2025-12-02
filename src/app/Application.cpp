#include "app/Application.h"

#include <QCoreApplication>
#include <QGuiApplication>
#include <QString>

#include <spdlog/spdlog.h>

#ifndef GNOTE_VERSION
#define GNOTE_VERSION "0.0.0-dev"
#endif

#include "ui/MainWindow.h"

namespace GnotePad
{

Application::Application(int& argc, char** argv)
    : QApplication(argc, argv)
{
    configureMetadata();
    configureIcon();
    spdlog::info("GnotePad Application initialized");
}

Application::~Application() = default;

int Application::run()
{
    m_mainWindow = std::make_unique<ui::MainWindow>();
    if(!m_applicationIcon.isNull())
    {
        m_mainWindow->setWindowIcon(m_applicationIcon);
    }
    m_mainWindow->show();
    return exec();
}

void Application::configureMetadata()
{
    QCoreApplication::setOrganizationName("GnotePad Project");
    QCoreApplication::setOrganizationDomain("gnotepad.app");
    QCoreApplication::setApplicationName("GnotePad");
    QCoreApplication::setApplicationVersion(QString::fromLatin1(GNOTE_VERSION));
}

void Application::configureIcon()
{
    m_applicationIcon = QIcon(QStringLiteral(":/gnotepad-icon.svg"));
    if(m_applicationIcon.isNull())
    {
        spdlog::warn("Failed to load embedded application icon; UI will fall back to default icons");
    }
    else
    {
        setWindowIcon(m_applicationIcon);
    }
}

} // namespace GnotePad
