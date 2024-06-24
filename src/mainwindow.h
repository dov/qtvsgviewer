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

class MainWindow : public QMainWindow
{
  Q_OBJECT;

public:
    MainWindow(vsg::CommandLine& arguments,
               vsg::ref_ptr<vsg::Options> options,
               QApplication& app);

    vsg::ref_ptr<vsgQt::Viewer> viewer;
    vsg::ref_ptr<vsg::Trackball> trackball;

private:
    vsgQt::Window* createWindow(vsg::ref_ptr<vsg::WindowTraits> traits,
                                vsg::ref_ptr<vsg::Node> vsg_scene,
                                QWindow* parent, const QString& title = {});

};


#endif /* MAINWINDOW */
