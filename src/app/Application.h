#pragma once

#include <memory>

#include <QApplication>
#include <QIcon>

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

    int run();

private:
    void configureMetadata();
    void configureIcon();

    std::unique_ptr<ui::MainWindow> m_mainWindow;
    QIcon m_applicationIcon;
};

} // namespace GnotePad
