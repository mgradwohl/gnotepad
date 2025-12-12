#pragma once

#include <spdlog/spdlog.h>

#include <QtCore/qobject.h>
#include <QtCore/qstringlist.h>
#include <QtGui/qicon.h>
#include <QtWidgets/qapplication.h>

#include <memory>

namespace GnotePad::ui
{
    class MainWindow;
}

namespace GnotePad
{

    class Application : public QApplication
    {
        Q_OBJECT

    public:
        Application(int& argc, char** argv);
        ~Application();

        Application(const Application&) = delete;
        Application& operator=(const Application&) = delete;
        Application(Application&&) = delete;
        Application& operator=(Application&&) = delete;

        int run();

        static bool isHeadlessSmokeMode();

    private:
        void parseCommandLine(const QStringList& arguments);
        void configureMetadata() const;
        void configureIcon();
        void configureStyle();

        std::unique_ptr<ui::MainWindow> m_mainWindow;
        QIcon m_applicationIcon;
        bool m_quitAfterInit{false};
    };

} // namespace GnotePad
