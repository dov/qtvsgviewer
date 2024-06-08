// Write a qt application with a menu bar on top and a status bar on the bottom. The menu bar should have a file menu with a quit option. The status bar should have a label that says "Ready". When the user selects the quit option, the application should close.

#include <QApplication>
#include <QMainWindow>
#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <QStatusBar>
#include <QLabel>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QMainWindow window;
    window.resize(800, 600);

    QMenuBar *menuBar = window.menuBar();
    QMenu *fileMenu = menuBar->addMenu("File");
    QAction *quitAction = fileMenu->addAction("Quit");
    QObject::connect(quitAction, &QAction::triggered, &app, &QApplication::quit);

    QStatusBar *statusBar = window.statusBar();
    QLabel *statusLabel = new QLabel("Ready");
    statusBar->addWidget(statusLabel);

    window.show();

    return app.exec();
}
 
