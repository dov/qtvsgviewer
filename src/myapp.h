//======================================================================
//  myapp.h - 
//
//  Dov Grobgeld <dov.grobgeld@gmail.com>
//  Sun Jun 23 09:58:51 2024
//----------------------------------------------------------------------
#ifndef MYAPP_H
#define MYAPP_H

#ifndef _MY_APP_H_
#define _MY_APP_H_

#include <QApplication>
#include "MainWindow.h"

class MyApp : public QApplication
{
  Q_OBJECT

  private:
    MainWindow m_MainWin;

  public:
    // Constructor
    MyApp(int argc, char *argv[]);
};

#endif /* MY_APP */
