#include "mainwindow.h"
#include "qwindow.h"
#include "edgeview.h"
#include "qguiapplication.h"
#include "qscreen.h"

EdgeView m_edge;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    QScreen *screen = QGuiApplication::screenAt(this->geometry().center());
    if (!screen)
    {
        screen = QGuiApplication::primaryScreen();
    }
    this->setGeometry(screen->availableGeometry());

    m_edge.SetReadyCallback([&](){
        m_edge.ResizeToClient();
        m_edge.Navigate(L"https://www.bing.com/");
    });
    m_edge.Initialize((HWND)this->winId());
}

MainWindow::~MainWindow() {}
