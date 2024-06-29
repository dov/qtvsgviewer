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
#include <QDateTime>
#include <QFileInfo>
#include <spdlog/spdlog.h>
#include <QFileDialog>


using namespace std;

// Returns a timestamp as milliseconds since the epoch. Note this time may jump
// around subject to adjustments by the system, to measure elapsed time use
// Timer instead.
static int64_t GetTimeInMillis() {
  return std::chrono::duration_cast<std::chrono::milliseconds>(
             std::chrono::system_clock::now() -
             std::chrono::system_clock::from_time_t(0))
      .count();
}

// constructor
MainWindow::MainWindow(vsg::CommandLine& arguments,
                       vsg::ref_ptr<vsg::Options> options_,
                       shared_ptr<QSettings> settings,
                       QApplication& app
    )
    : options(options_),
      m_settings(settings)

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
    std::string filename = arguments[1];

    this->modelContainer = vsg::MatrixTransform::create();

    auto vsg_scene = vsg::Group::create();
    vsg_scene->addChild(this->modelContainer);

    this->resize(800, 600);

    // Create a timer but don't start it
    this->autoloadTimer = new QTimer(this);
    connect(this->autoloadTimer, SIGNAL(timeout()), this, SLOT(reload(void)));

    // Why do Ιneed to add a node to the child even if I later do
    // loadfile()?
    //    auto node = vsg::read_cast<vsg::Node>(filename, options);
    //this->modelContainer->addChild(node);

    loadfile(filename);
    m_widget3d = new Widget3D(this, vsg_scene, windowTraits);
    m_widget3d->show();
    m_widget3d->setFocus();

    this->setCentralWidget(m_widget3d);

    auto viewAutoloadAct = new QAction(tr("Auto load"), this);
    viewAutoloadAct->setCheckable(true);
    viewAutoloadAct->setStatusTip(tr("Toggle autoloading of a file"));
    connect(viewAutoloadAct, SIGNAL(toggled(bool)), this, SLOT(toggleAutoload(bool)));
    viewAutoloadAct->setChecked(m_settings->value("autoload").toBool());

    auto openAct = new QAction(tr("&Open..."), this);
    openAct->setShortcuts(QKeySequence::Open);
    openAct->setStatusTip(tr("Open an existing file"));
    connect(openAct, SIGNAL(triggered()), this, SLOT(open()));


    // Build the gui

    QMenuBar *menuBar = this->menuBar();
    QMenu *fileMenu = menuBar->addMenu("&File");
    fileMenu->addAction(openAct);
    fileMenu->addSeparator();
    QAction *quitAction = fileMenu->addAction("&Quit");

    QMenu *viewMenu = menuBar->addMenu(tr("&View"));
    viewMenu->addAction(viewAutoloadAct);


    QObject::connect(quitAction, &QAction::triggered, &app, &QApplication::quit);

    this->statusBar = new QStatusBar(this);
    this->setStatusBar(this->statusBar);

    // Read the filename
    m_widget3d->compile();

    setStatusMessage("Ready");
}

// Update the trackball with the new values, e.g. from a spnav device
void MainWindow::updateTrackball(double dx, double dy, double dz,
                                 double xrot, double yrot, double zrot)
{
    m_widget3d->updateTrackball(dx, dy, dz, xrot, yrot, zrot);
}

void MainWindow::toggleAutoload(bool doAutoload)
{
  if (doAutoload)
    this->autoloadTimer->start(200);
  else
    this->autoloadTimer->stop();
  m_settings->setValue("autoload", doAutoload);
}

void MainWindow::reload()
{
    // Check if the modified date changed
    QDateTime last_modified = QFileInfo(QString::fromStdString(this->currentFilename)).lastModified();
    bool wasmodified = (last_modified != this->currentFilenameLastModified);

    // Make sure that the modification was at least 100ms ago so that
    // we don't try to read a file that is in the middle of being written.
    wasmodified &= (last_modified < QDateTime::currentDateTime().addMSecs(-100));

    if (wasmodified)
        this->loadfile(this->currentFilename);
}

void MainWindow::loadfile(const std::string& filename)
{
    int64_t lf_t0 = GetTimeInMillis();
    // convert a std string to a qString:
    QFile Fout(QString::fromStdString(filename));

    spdlog::info("Loading file {}", filename);
    setStatusMessage(fmt::format("Loading {}", filename));

    auto node = vsg::read_cast<vsg::Node>(filename, options);
    if (!node)
    {
        std::cout << "Failed to load a valid file"
                  << std::endl;
        exit(-1);
    }

    // I don't know why, but read swaps the y and the z-axis. This transform node
    // transforms it back
    if (filename.find(".stl") != string::npos)
        this->modelContainer->matrix = 
            vsg::dmat4 {{ 1.0f, 0.0f, 0.0f, 0.0f },
                        { 0.0f, 0.0f, -1.0f, 0.0f },
                        { 0.0f, 1.0f, 0.0f, 0.0f },
                        { 0.0f, 0.0f, 0.0f, 1.0f }};
    else
        this->modelContainer->matrix = vsg::dmat4(1.0);
      
    this->modelContainer->children.clear();
    this->modelContainer->addChild(node);

    int64_t time0 = GetTimeInMillis();
    spdlog::info("Loaded xjsf. Duration = {} ms", GetTimeInMillis()-time0);

    this->currentFilename = filename;
    this->currentFilenameLastModified = QFileInfo(QString::fromStdString(filename)).lastModified();

    QFileInfo fi(QString::fromStdString(filename));
    setWindowTitle("qtvsgviewer: " + fi.fileName());

    spdlog::info("Total load file duration = {} ms", GetTimeInMillis()-lf_t0);
    if (m_widget3d) {
        m_widget3d->compile();
        m_widget3d->autoScale(); // TBD: make this conditional
    }
}

void MainWindow::setStatusMessage(const std::string& message)
{
    spdlog::debug("setStatusMessage(message=\"{}\")", message);
    if (this->statusBar)
        this->statusBar->showMessage(QString::fromStdString(message), 2500);
    
}

void MainWindow::open()
{
   QString filename = QFileDialog::getOpenFileName(this,
                                                  tr("Open Image"),
                                                  nullptr,
                                                  tr("Model Files (*.stl *.fern *.xjsf *.obj)"));

   // convert a qstring to a stdstring
   loadfile(filename.toStdString());
}
