//======================================================================
//  myapp.h - 
//
//  Dov Grobgeld <dov.grobgeld@gmail.com>
//  Sun Jun 23 09:58:51 2024
//----------------------------------------------------------------------
#include <vsg/all.h>
#include <vsgXchange/all.h>
#include <QTimer>
#include <QApplication>
#include <QMainWindow>
#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <QStatusBar>
#include <QLabel>
#include <vsgQt/Window.h>
#include <QObject>
#include <spnav.h>
#include <fmt/core.h>
#include "myapp.h"

// Constructor
MyApp::MyApp(int argc, char *argv[])
  : QApplication(argc, argv)
{
    bool hasSpnav = true;
    if(spnav_open() == -1)
        hasSpnav = false;

    vsg::CommandLine arguments(&argc, argv);

    // set up vsg::Options to pass in filepaths, ReaderWriters and other IO
    // related options to use when reading and writing files.
    auto options = vsg::Options::create();
    options->fileCache = vsg::getEnv("VSG_FILE_CACHE");
    options->paths = vsg::getEnvPaths("VSG_FILE_PATH");
    options->add(vsgXchange::all::create());

    arguments.read(options);

    auto mainWindow = new MainWindow(arguments, options, *this);

    mainWindow->show();

    if (hasSpnav)
    {
        // Poll for spnav events and forward them to the trackball
        QTimer *timer = new QTimer();
        QObject::connect(timer, &QTimer::timeout, [mainWindow]() {
            double dx = 0, dy = 0, dz = 0, xrot = 0, yrot = 0, zrot = 0;
      
            spnav_event event;
            if ( spnav_poll_event(&event) > 0)
            {
                if (event.type == SPNAV_EVENT_MOTION)
                {
                    dx = 0.0005* event.motion.x;
                    dy = -0.0005* event.motion.y;
                    dz = -0.0005* event.motion.z;
                    xrot = -0.0005* event.motion.rx;
                    yrot = -0.0005* event.motion.ry;
                    zrot = 0.0005* event.motion.rz;
                }
#if 0
                if (swapyz)
                {
                    std::swap(dy, dz);
                    dy = -dy;
                    std::swap(yrot, zrot);
                }
#endif
      
                // Don't accumulate events
                spnav_remove_events(SPNAV_EVENT_MOTION);

                mainWindow->updateTrackball(
                  dx,dy,dz,xrot,yrot,zrot);
            }
      
        });
        timer->start(20);
    }


}


