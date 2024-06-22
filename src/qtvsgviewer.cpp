//======================================================================
// A user friendly 3D model viewer with a qt interface.
//
// 2024-06-22 Sat
// Dov Grobgeld <dov.grobgeld@gmail.com>
//
// License:
//   This program is licensed under the GPL v2 license
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
#include <iostream>
#include <QObject>
#include <spnav.h>
#include <fmt/core.h>
#include <string>

using namespace std;

vsg::ref_ptr<vsg::Trackball> trackball;

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
vsg::ref_ptr<vsg::Node> create_arrow(vsg::vec3 pos, vsg::vec3 dir, vsg::vec4 color)
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
vsg::ref_ptr<vsg::Node> create_gizmo()
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
vsg::ref_ptr<vsg::View> createViewGizmo(vsg::ref_ptr<vsg::Camera> camera,
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

vsgQt::Window* createWindow(vsg::ref_ptr<vsgQt::Viewer> viewer,
                            vsg::ref_ptr<vsg::WindowTraits> traits,
                            vsg::ref_ptr<vsg::Node> vsg_scene,
                            QWindow* parent, const QString& title = {})
{
    auto window = new vsgQt::Window(viewer, traits, parent);

    window->setTitle(title);

    window->initializeWindow();

    // if this is the first window to be created, use its device for future window creation.
    if (!traits->device) traits->device = window->windowAdapter->getOrCreateDevice();

    // compute the bounds of the scene graph to help position camera
    vsg::ComputeBounds computeBounds;
    vsg_scene->accept(computeBounds);
    vsg::dvec3 centre = (computeBounds.bounds.min + computeBounds.bounds.max) * 0.5;
    double radius = vsg::length(computeBounds.bounds.max - computeBounds.bounds.min) * 0.6;
    double nearFarRatio = 0.001;

    uint32_t width = window->traits->width;
    uint32_t height = window->traits->height;
    double aspectRatio = 1.0*width/height;

    vsg::ref_ptr<vsg::EllipsoidModel> ellipsoidModel(vsg_scene->getObject<vsg::EllipsoidModel>("EllipsoidModel"));
    vsg::ref_ptr<vsg::Camera> camera;
    {
        // set up the camera
        auto lookAt = vsg::LookAt::create(centre + vsg::dvec3(radius, -radius * 2.5, radius),
                                          centre, vsg::dvec3(0.0, 0.0, 1.0));

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
                nearFarRatio * radius, radius * 4.5);
        }

        camera = vsg::Camera::create(perspective, lookAt, vsg::ViewportState::create(VkExtent2D{width, height}));
    }


    trackball = vsg::Trackball::create(camera, ellipsoidModel);
    trackball->addWindow(*window);

    viewer->addEventHandler(trackball);
    auto scene = vsg::StateGroup::create();
    scene->addChild(vsg::createHeadlight());
    scene->addChild(vsg_scene);

    auto commandGraph = vsg::CommandGraph::create(*window);
    auto renderGraph = vsg::RenderGraph::create(*window);
    auto view = vsg::View::create(camera);
    view->addChild(scene);

    //    auto commandGraph = vsg::createCommandGraphForView(*window, camera, scene);
    renderGraph->addChild(view);

    renderGraph->addChild(createViewGizmo(camera, aspectRatio));

    commandGraph->addChild(renderGraph);
    viewer->addRecordAndSubmitTaskAndPresentation({commandGraph});

    return window;
}


int main(int argc, char *argv[])
{
    if(spnav_open() == -1)
    {
        std::cerr << "failed to connect to the space navigator daemon" << std::endl;
        return -1;
    }

    QApplication app(argc, argv);

    vsg::CommandLine arguments(&argc, argv);

    // set up vsg::Options to pass in filepaths, ReaderWriters and other IO
    // related options to use when reading and writing files.
    auto options = vsg::Options::create();
    options->fileCache = vsg::getEnv("VSG_FILE_CACHE");
    options->paths = vsg::getEnvPaths("VSG_FILE_PATH");
#ifdef vsgXchange_FOUND
    options->add(vsgXchange::all::create());
#endif

    arguments.read(options);

    auto windowTraits = vsg::WindowTraits::create();
    windowTraits->windowTitle = "vsgQt viewer";
    windowTraits->debugLayer = arguments.read({"--debug", "-d"});
    windowTraits->apiDumpLayer = arguments.read({"--api", "-a"});
    arguments.read("--samples", windowTraits->samples);
    arguments.read({"--window", "-w"}, windowTraits->width, windowTraits->height);
    if (arguments.read({"--fullscreen", "--fs"})) windowTraits->fullscreen = true;

    // continuous update is fixed off to save cpu. It is assumed that qtvsgviewer
    // is mainly used for static models
    bool continuousUpdate = false;
    auto interval = arguments.value<int>(20, "--interval");

    if (arguments.errors())
        return arguments.writeErrorMessages(std::cerr);

    if (argc <= 1)
    {
        std::cout << "Please specify a 3d model or image file on the command line."
                  << std::endl;
        return 1;
    }

    // Read the filename
    vsg::Path filename = arguments[1];
    auto node = vsg::read_cast<vsg::Node>(filename, options);
    if (!node)
    {
        std::cout << "Failed to load a valid file"
                  << std::endl;
        return 1;
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

    // create the viewer that will manage all the rendering of the views
    auto viewer = vsgQt::Viewer::create();

    auto window = createWindow(viewer, windowTraits, vsg_scene, nullptr, "First Window");

    QMainWindow *mainWindow = new QMainWindow();
    auto widget = QWidget::createWindowContainer(window, mainWindow);

    mainWindow->resize(800, 600);

    mainWindow->setCentralWidget(widget);

    QMenuBar *menuBar = mainWindow->menuBar();
    QMenu *fileMenu = menuBar->addMenu("File");
    QAction *quitAction = fileMenu->addAction("Quit");
    QObject::connect(quitAction, &QAction::triggered, &app, &QApplication::quit);

    QStatusBar *statusBar = mainWindow->statusBar();
    QLabel *statusLabel = new QLabel("Ready");
    statusBar->addWidget(statusLabel);

    mainWindow->show();

    if (interval >= 0) viewer->setInterval(interval);
    viewer->continuousUpdate = continuousUpdate;

    viewer->addEventHandler(vsg::CloseHandler::create(viewer));
    viewer->compile();

    // Poll for spnav events and forward them to the trackball
    QTimer *timer = new QTimer();
    QObject::connect(timer, &QTimer::timeout, [&]() {
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

            trackball->pan({dx,dy});
            trackball->zoom(dz);
            trackball->rotate(xrot, vsg::dvec3(1,0,0));
            trackball->rotate(yrot, vsg::dvec3(0,1,0));
            trackball->rotate(zrot, vsg::dvec3(0,0,1));

            viewer->update();
            viewer->request();
            viewer->render();
  
        }
  
    });
    timer->start(20);

    app.exec();
    trackball = nullptr;
    
    exit(0);
}
 
