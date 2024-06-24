//======================================================================
//  myapp.h - 
//
//  Dov Grobgeld <dov.grobgeld@gmail.com>
//  Sun Jun 23 09:58:51 2024
//----------------------------------------------------------------------
#ifndef MYAPP_H
#define MYAPP_H

#include <QApplication>
#include "mainwindow.h"

class MyApp : public QApplication
{
  Q_OBJECT

  private:
    MainWindow *m_mainwindow = nullptr;

  public:
    // Constructor
    MyApp(int argc, char *argv[]);
};

#endif /* MY_APP */
