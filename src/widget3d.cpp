//======================================================================
//  widget3d.cpp - 
//
//  Dov Grobgeld <dov.grobgeld@gmail.com>
//  Mon Jun 24 22:49:50 2024
//----------------------------------------------------------------------

#include "widget3d.h"
#include <QVBoxLayout>
#include <spdlog/spdlog.h>
#include <fmt/core.h>

using fmt::print;

// A rotation tracking view matrix. It returns the rotation component
// of another viewMatrix.
class RotationTrackingMatrix : public vsg::Inherit<vsg::ViewMatrix, RotationTrackingMatrix> {
public:
    RotationTrackingMatrix(vsg::ref_ptr<vsg::ViewMatrix> parentTransform) :
        parentTransform_(parentTransform) {}
      
    // The transform() returns the rotation part of the tracked matrix
    vsg::dmat4 transform() const override {
        vsg::dvec3 translation, scale;
        vsg::dquat rotation; 
        vsg::decompose(parentTransform_->transform(),
                       // output
                       translation,
                       rotation,
                       scale);
  
        return vsg::rotate(rotation);
    }
  
private:
    vsg::ref_ptr<vsg::ViewMatrix> parentTransform_;
};

// Create an arrow with the back at pos and pointing in the direction of dir
// Place a cone at the end of the arrow with the color color
static vsg::ref_ptr<vsg::Node>
create_arrow(vsg::vec3 pos, vsg::vec3 dir, vsg::vec4 color)
{
    vsg::ref_ptr<vsg::Group> arrow = vsg::Group::create();

    vsg::Builder builder;
    vsg::GeometryInfo geomInfo;
    vsg::StateInfo stateInfo;

    geomInfo.color = vsg::vec4{1,1,1,1};
    geomInfo.position = pos;
    geomInfo.transform = vsg::translate(0.0f, 0.0f, 0.5f);

    // If we don't point in the z-direction, then rotate the arrow
    if (vsg::length(vsg::cross(vsg::vec3{0,0,1}, dir)) > 0.0001)
    {
        vsg::vec3 axis = vsg::cross(vsg::vec3{0,0,1}, dir);
        float angle = acos(vsg::dot(vsg::vec3{0,0,1}, dir));
        geomInfo.transform = vsg::rotate(angle, axis) * geomInfo.transform;
    }
    auto axis_transform = geomInfo.transform;
    geomInfo.transform = geomInfo.transform * vsg::scale(0.1f, 0.1f, 1.0f);
    
    // Rotate geomInfo from pos in the direction of dir
    auto node = builder.createCylinder(geomInfo, stateInfo);
    arrow->addChild(node);

    // The cone
    geomInfo.color = color;
    // This would have been cleaner with a pre_translate transform
    geomInfo.transform = vsg::scale(0.3f, 0.3f, 0.3f) * axis_transform * vsg::translate(0.0f, 0.0f, 1.0f/0.3f);
    node = builder.createCone(geomInfo, stateInfo);
    arrow->addChild(node);

    return arrow;
}

// Create three arrows of the coordinate axes
static vsg::ref_ptr<vsg::Node> create_gizmo()
{
    vsg::ref_ptr<vsg::Group> gizmo = vsg::Group::create();

    gizmo->addChild(create_arrow(vsg::vec3{0,0,0}, vsg::vec3{1,0,0}, vsg::vec4{1,0,0,1}));
    gizmo->addChild(create_arrow(vsg::vec3{0,0,0}, vsg::vec3{0,1,0}, vsg::vec4{0,1,0,1}));
    gizmo->addChild(create_arrow(vsg::vec3{0,0,0}, vsg::vec3{0,0,1}, vsg::vec4{0,0,1,1}));

    vsg::Builder builder;
    vsg::GeometryInfo geomInfo;
    vsg::StateInfo stateInfo;
    geomInfo.color = vsg::vec4{1,1,1,1};
    geomInfo.transform = vsg::scale(0.1f, 0.1f, 0.1f);

    auto sphere = builder.createSphere(geomInfo, stateInfo);
    gizmo->addChild(sphere);

    return gizmo;
}

// Create a tracking overlay with a gizmo that shows the orientation
// of the camera view matrix
vsg::ref_ptr<vsg::View>
Widget3D::createViewGizmo(vsg::ref_ptr<vsg::Camera> camera,
                          double aspectRatio)
{
    auto viewMat = RotationTrackingMatrix::create(camera->viewMatrix);

    // Place the gizmo in the view port by modifying its ortho camera x
    // and y limits
    double camWidth = 10;
    double camXOffs = -8;
    double camYOffs = -8;
    auto ortho = vsg::Orthographic::create(
        camXOffs,camXOffs+camWidth,  // left, right
        camYOffs/aspectRatio,(camYOffs+camWidth)/aspectRatio, // bottom, top
        -1000,1000); // near, far

    auto gizmoCamera = vsg::Camera::create(
        ortho,
        viewMat,
        camera->viewportState);

    auto scene = vsg::Group::create();
    scene->addChild(create_gizmo());

    auto view = vsg::View::create(gizmoCamera);
    view->addChild(vsg::createHeadlight());
    view->addChild(scene);
    return view;
}

// constructor

Widget3D::Widget3D(QWidget *parent,
                   vsg::ref_ptr<vsg::Node> vsg_scene,
                   vsg::ref_ptr<vsg::WindowTraits> windowTraits)
  : QWidget(parent),
    m_scene(vsg_scene)
{
    auto window = createWindow(windowTraits, vsg_scene);
    m_vsgwidget = QWidget::createWindowContainer(window, this);
    auto layout = new QVBoxLayout;
    layout->addWidget(m_vsgwidget);
    layout->setContentsMargins(0, 0, 0, 0);
    this->setLayout(layout);
    m_vsgwidget->show();
    m_viewer->continuousUpdate = true;
    m_viewer->setInterval(20);

    m_viewer->addEventHandler(vsg::CloseHandler::create(m_viewer));
    m_viewer->compile();

    setAcceptDrops(true);
    setFocusPolicy(Qt::StrongFocus);
    setFocus(Qt::NoFocusReason);
}

Widget3D::~Widget3D()
{
}


vsgQt::Window* Widget3D::createWindow(
  vsg::ref_ptr<vsg::WindowTraits> windowTraits,
  vsg::ref_ptr<vsg::Node> vsg_scene)

{
    m_viewer = vsgQt::Viewer::create();
    auto window = new vsgQt::Window(m_viewer, windowTraits, (QWindow*)nullptr);

    window->initializeWindow();

    // if this is the first window to be created, use its device for future window creation.
    if (!windowTraits->device)
        windowTraits->device = window->windowAdapter->getOrCreateDevice();

    // compute the bounds of the scene graph to help position camera
    vsg::ComputeBounds computeBounds;
    vsg_scene->accept(computeBounds);
    m_center = (computeBounds.bounds.min + computeBounds.bounds.max) * 0.5;
    m_radius = vsg::length(computeBounds.bounds.max - computeBounds.bounds.min) * 0.6;
    double nearFarRatio = 0.001;

    uint32_t width = window->traits->width;
    uint32_t height = window->traits->height;
    double aspectRatio = 1.0*width/height;

    vsg::ref_ptr<vsg::EllipsoidModel> ellipsoidModel(vsg_scene->getObject<vsg::EllipsoidModel>("EllipsoidModel"));
    vsg::ref_ptr<vsg::Camera> camera;
    {
        // set up the camera
        auto lookAt = vsg::LookAt::create(m_center + vsg::dvec3(m_radius, -m_radius * 2.5, m_radius),
                                          m_center, vsg::dvec3(0.0, 0.0, 1.0));

        vsg::ref_ptr<vsg::ProjectionMatrix> perspective;
        if (ellipsoidModel)
        {
            perspective = vsg::EllipsoidPerspective::create(
                lookAt, ellipsoidModel, 30.0, aspectRatio,
                nearFarRatio, false);
        }
        else
        {
            perspective = vsg::Perspective::create(
                30.0,
                aspectRatio,
                nearFarRatio * m_radius, m_radius * 4.5);
        }

        camera = vsg::Camera::create(perspective, lookAt, vsg::ViewportState::create(VkExtent2D{width, height}));
    }

    m_trackball = vsg::Trackball::create(camera, ellipsoidModel);
    m_trackball->addWindow(*window);
    autoScale();

    m_viewer->addEventHandler(m_trackball);
    auto scene = vsg::StateGroup::create();
    scene->addChild(vsg::createHeadlight());
    scene->addChild(vsg_scene);

    m_commandGraph = vsg::CommandGraph::create(*window);
    auto renderGraph = vsg::RenderGraph::create(*window);
    auto view = vsg::View::create(camera);
    view->addChild(scene);

    renderGraph->addChild(view);

    renderGraph->addChild(createViewGizmo(camera, aspectRatio));

    m_commandGraph->addChild(renderGraph);

    m_viewer->addRecordAndSubmitTaskAndPresentation({m_commandGraph});

    return window;
}

void Widget3D::updateTrackball(double dx, double dy, double dz,
                               double xrot, double yrot, double zrot)
{
  m_trackball->pan({dx,dy});
  m_trackball->zoom(dz);
  m_trackball->rotate(xrot, vsg::dvec3(1,0,0));
  m_trackball->rotate(yrot, vsg::dvec3(0,1,0));
  m_trackball->rotate(zrot, vsg::dvec3(0,0,1));
    
  m_viewer->update();
  m_viewer->request();
  m_viewer->render();
}

void Widget3D::compile()
{
  m_viewer->compile();
}

// Setup the camera to match the contents in the m_scene
void Widget3D::autoScale()
{
    vsg::ComputeBounds computeBounds;
    m_scene->accept(computeBounds);
    m_center = (computeBounds.bounds.min + computeBounds.bounds.max) * 0.5;
    m_radius = vsg::length(computeBounds.bounds.max - computeBounds.bounds.min) * 0.6;

    // set up the camera
    auto lookAt = vsg::LookAt::create(m_center + vsg::dvec3(m_radius, -m_radius * 2.5, m_radius),
                                      m_center, vsg::dvec3(0.0, 0.0, 1.0));

    m_trackball->setViewpoint(lookAt, 0.1);

    // Setup keybindings for looking from different dirs
    auto lookAtDiag = vsg::LookAt::create(m_center + vsg::dvec3(m_radius, -m_radius * 2.5, m_radius),
                                          m_center,
                                          vsg::dvec3(0.0, 0.0, 1.0));
    auto lookAtTop = vsg::LookAt::create(m_center + vsg::dvec3(0.0, 0.0, m_radius * 2.5),
                                         m_center,
                                         vsg::dvec3(0.0, 1.0, 0.0));
    auto lookAtFront = vsg::LookAt::create(m_center + vsg::dvec3(0.0, -m_radius * 2.5, 0.0),
                                           m_center,
                                           vsg::dvec3(0.0, 0.0, 1.0));
    auto lookAtRight = vsg::LookAt::create(m_center + vsg::dvec3(m_radius * 2.5, 0.0, 0.0),
                                           m_center,
                                           vsg::dvec3(0.0, 0.0, 1.0));

    m_trackball->addKeyViewpoint(vsg::KeySymbol::KEY_f, lookAtDiag, 0.5);

    // Add keybindings for looking from different directions
    m_trackball->addKeyViewpoint(vsg::KeySymbol::KEY_7, lookAtTop, 0.5);
    m_trackball->addKeyViewpoint(vsg::KeySymbol::KEY_KP_7, lookAtTop, 0.5);
    m_trackball->addKeyViewpoint(vsg::KeySymbol::KEY_KP_Home, lookAtTop, 0.5);
    m_trackball->addKeyViewpoint(vsg::KeySymbol::KEY_Home, lookAtTop, 0.5);
    
    m_trackball->addKeyViewpoint(vsg::KeySymbol::KEY_1, lookAtFront, 0.5);
    m_trackball->addKeyViewpoint(vsg::KeySymbol::KEY_KP_1, lookAtFront, 0.5);
    m_trackball->addKeyViewpoint(vsg::KeySymbol::KEY_KP_End, lookAtFront, 0.5);
    m_trackball->addKeyViewpoint(vsg::KeySymbol::KEY_End, lookAtFront, 0.5);

    m_trackball->addKeyViewpoint(vsg::KeySymbol::KEY_3, lookAtRight, 0.5);
    m_trackball->addKeyViewpoint(vsg::KeySymbol::KEY_KP_3, lookAtRight, 0.5);
    m_trackball->addKeyViewpoint(vsg::KeySymbol::KEY_KP_Page_Down, lookAtRight, 0.5);
    m_trackball->addKeyViewpoint(vsg::KeySymbol::KEY_Page_Down, lookAtRight, 0.5);
}

