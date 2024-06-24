//======================================================================
//  mainwindow.h - 
//
//  Dov Grobgeld <dov.grobgeld@gmail.com>
//  Sun Jun 23 10:00:52 2024
//----------------------------------------------------------------------
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <vsg/all.h>
#include <vsgQt/Window.h>
#include <QMainWindow>
#include "widget3d.h"

class MainWindow : public QMainWindow
{
  Q_OBJECT;

public:
    MainWindow(vsg::CommandLine& arguments,
               vsg::ref_ptr<vsg::Options> options,
               QApplication& app);

    void updateTrackball(double dx, double dy, double dz,
                         double xrot, double yrot, double zrot);

private:
    vsgQt::Window* createWindow(vsg::ref_ptr<vsg::WindowTraits> traits,
                                vsg::ref_ptr<vsg::Node> vsg_scene,
                                QWindow* parent, const QString& title = {});

    Widget3D* m_widget3d = nullptr;
};


#endif /* MAINWINDOW */
