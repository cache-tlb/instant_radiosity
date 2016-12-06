#include "MainWindow.h"
#include <QApplication>

void test();

int main(int argc, char *argv[])
{
//    test();
    QApplication a(argc, argv);

    QSurfaceFormat format;
    format.setDepthBufferSize(24);
    format.setStencilBufferSize(8);
    format.setProfile(QSurfaceFormat::CoreProfile);
    format.setSamples(16);
    QSurfaceFormat::setDefaultFormat(format);

    QGLFormat glf = QGLFormat::defaultFormat();
    glf.setSampleBuffers(true);
    glf.setSamples(16);
    QGLFormat::setDefaultFormat(glf);

    MainWindow w;
    w.Init();
    w.show();

    return a.exec();
}
