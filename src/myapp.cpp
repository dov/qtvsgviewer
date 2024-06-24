//======================================================================
//  myapp.h - 
//
//  Dov Grobgeld <dov.grobgeld@gmail.com>
//  Sun Jun 23 09:58:51 2024
//----------------------------------------------------------------------
#include "myapp.h"
#include "mainwindow.h"

// Constructor
MyApp::MyApp(int argc, char *argv[])
  : QApplication(argc, argv)
{
  QCoreApplication::setOrganizationName("DovProgramming");
  QCoreApplication::setApplicationName("Qt VSG Viewer");
  QCoreApplication::setApplicationVersion(QT_VERSION_STR);

  m_MainWin.show();

  int argp = 1;
  if (argp < argc)
  {
    QString Filename = argv[argp++];
    qDebug() << "Loading " << Filename << "\n";
    m_MainWin.loadFile(Filename);
  }
}


