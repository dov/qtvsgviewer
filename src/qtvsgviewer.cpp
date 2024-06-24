//======================================================================
// A user friendly 3D model viewer with a qt interface.
//
// 2024-06-22 Sat
// Dov Grobgeld <dov.grobgeld@gmail.com>
//
// License:
//   This program is licensed under the GPL v2 license
//----------------------------------------------------------------------

#include "mainwindow.h"
#include "myapp.h"

using namespace std;


int main(int argc, char *argv[])
{
    MyApp app(argc, argv);

    app.exec();
    
    exit(0);
}
 
