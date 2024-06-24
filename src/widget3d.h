//======================================================================
//  widget3d.h - A 3D widget for a model viewer
//
//  Dov Grobgeld <dov.grobgeld@gmail.com>
//  Mon Jun 24 22:41:22 2024
//----------------------------------------------------------------------
#ifndef WIDGET3D_H
#define WIDGET3D_H

#include <vsg/all.h>
#include <vsgQt/Window.h>
#include <QWidget>

class Widget3D : public QWidget
{
    Q_OBJECT

public:
    Widget3D(QWidget *parent = 0,
             vsg::ref_ptr<vsg::Node> scene = nullptr,
             vsg::ref_ptr<vsg::WindowTraits> traits = nullptr);
    ~Widget3D();

    void updateTrackball(double dx, double dy, double dz,
                         double xrot, double yrot, double zrot);

private:
    vsgQt::Window* createWindow(
      vsg::ref_ptr<vsg::WindowTraits> traits,
      vsg::ref_ptr<vsg::Node> vsg_scene);

    vsg::ref_ptr<vsg::View> createViewGizmo(vsg::ref_ptr<vsg::Camera> camera,
                                            double aspectRatio);

    QWidget *m_vsgwidget = nullptr;
    vsg::ref_ptr<vsg::Node> m_scene;
    vsg::ref_ptr<vsgQt::Viewer> m_viewer;
    vsg::ref_ptr<vsg::Trackball> m_trackball;
};

#endif /* WIDGET3D */
