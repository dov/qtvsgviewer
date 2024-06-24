#include <vsg/all.h>
#include "mainwindow.h"
#include <vsgXchange/all.h>
#include <QTimer>
#include <QApplication>
#include <QMainWindow>
#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <QStatusBar>
#include <QLabel>
#include <iostream>

using namespace std;

// constructor
MainWindow::MainWindow(vsg::CommandLine& arguments,
                       vsg::ref_ptr<vsg::Options> options,
                       QApplication& app
    )
{
    auto windowTraits = vsg::WindowTraits::create();
    windowTraits->windowTitle = "vsgQt viewer";
    windowTraits->debugLayer = arguments.read({"--debug", "-d"});
    windowTraits->apiDumpLayer = arguments.read({"--api", "-a"});
    arguments.read("--samples", windowTraits->samples);
    arguments.read({"--window", "-w"}, windowTraits->width, windowTraits->height);
    if (arguments.read({"--fullscreen", "--fs"})) windowTraits->fullscreen = true;

    if (arguments.errors())
    {
        arguments.writeErrorMessages(std::cerr);
        exit(-1);
    }
    
    if (arguments.argc() <= 1)
    {
        std::cout << "Please specify a 3d model or image file on the command line."
                  << std::endl;
        exit(-1);
    }

    // Read the filename
    vsg::Path filename = arguments[1];
    auto node = vsg::read_cast<vsg::Node>(filename, options);
    if (!node)
    {
        std::cout << "Failed to load a valid file"
                  << std::endl;
        exit(-1);
    }

    auto transform = vsg::MatrixTransform::create();

    // I don't know why, but read swaps the y and the z-axis. This transform node
    // transforms it back
    if (string(arguments[1]).find(".stl") != string::npos)
    {
        transform = vsg::MatrixTransform::create(
            vsg::dmat4 {{ 1.0f, 0.0f, 0.0f, 0.0f },
                        { 0.0f, 0.0f, -1.0f, 0.0f },
                        { 0.0f, 1.0f, 0.0f, 0.0f },
                        { 0.0f, 0.0f, 0.0f, 1.0f }} );
    }
      
    transform->addChild(node);
    auto vsg_scene = vsg::Group::create();
    vsg_scene->addChild(transform);

    this->resize(800, 600);

    m_widget3d = new Widget3D(this, vsg_scene, windowTraits);
    m_widget3d->show();

    this->setCentralWidget(m_widget3d);

    QMenuBar *menuBar = this->menuBar();
    QMenu *fileMenu = menuBar->addMenu("File");
    QAction *quitAction = fileMenu->addAction("Quit");
    QObject::connect(quitAction, &QAction::triggered, &app, &QApplication::quit);

    QStatusBar *statusBar = this->statusBar();
    QLabel *statusLabel = new QLabel("Ready");
    statusBar->addWidget(statusLabel);
}

// Update the trackball with the new values, e.g. from a spnav device
void MainWindow::updateTrackball(double dx, double dy, double dz,
                                 double xrot, double yrot, double zrot)
{
    m_widget3d->updateTrackball(dx, dy, dz, xrot, yrot, zrot);
}
