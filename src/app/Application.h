#pragma once

#include <memory>

#include <QtCore/qobject.h>
#include <QtGui/qicon.h>
#include <QtWidgets/qapplication.h>

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

private:
    void configureMetadata() const;
    void configureIcon();

    std::unique_ptr<ui::MainWindow> m_mainWindow;
    QIcon m_applicationIcon;
};

} // namespace GnotePad
