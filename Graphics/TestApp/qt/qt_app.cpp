
#include <QSurfaceFormat>
#include <QApplication>
#include <QMainWindow>


#include "viewer.h"
#include "geometry_descriptor.h"


int main(int argc, char *argv[])
{
    QApplication app(argc, argv);   
    
    QMainWindow window;
    window.setWindowTitle("Qt Viewer");
    window.setCentralWidget(new Viewer(&window));
    window.show();
    return app.exec();
}