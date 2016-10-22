#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "GLWidget.h"
#include <QMainWindow>

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = 0);
    ~MainWindow();

    void Init();

protected:
    GLWidget *gl_widget_;
};

#endif // MAINWINDOW_H
