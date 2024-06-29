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
#include <QSettings>
#include "widget3d.h"
#include <QDateTime>
#include <QTimer>


class MainWindow : public QMainWindow
{
  Q_OBJECT;

public:
    MainWindow(vsg::CommandLine& arguments,
               vsg::ref_ptr<vsg::Options> options,
               std::shared_ptr<QSettings> settings,
               QApplication& app);

    void updateTrackball(double dx, double dy, double dz,
                         double xrot, double yrot, double zrot);

    void setStatusMessage(const std::string& message);

private:
    vsgQt::Window* createWindow(vsg::ref_ptr<vsg::WindowTraits> traits,
                                vsg::ref_ptr<vsg::Node> vsg_scene,
                                QWindow* parent, const QString& title = {});

    void loadfile(const std::string& filename);

    Widget3D* m_widget3d = nullptr;
    QTimer *autoloadTimer = nullptr;
    std::string currentFilename;
    QDateTime currentFilenameLastModified;
    QStatusBar *statusBar =  nullptr;
    vsg::ref_ptr<vsg::MatrixTransform> modelContainer;
    vsg::ref_ptr<vsg::Options> options;
    std::shared_ptr<QSettings> m_settings;

private slots:
    void open();
    void reload();
    void toggleAutoload(bool DoAutoload);

};


#endif /* MAINWINDOW */
