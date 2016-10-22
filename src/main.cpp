#include "MainWindow.h"
#include <QApplication>

void test();

int main(int argc, char *argv[])
{
//    test();
    QApplication a(argc, argv);

    MainWindow w;
    w.Init();
    w.show();

    return a.exec();
}
