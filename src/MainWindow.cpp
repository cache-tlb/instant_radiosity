#include "MainWindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
}

MainWindow::~MainWindow()
{}

void MainWindow::Init() {
    // TODO
    gl_widget_ = new GLWidget(this);
    this->setCentralWidget(gl_widget_);
    gl_widget_->setFixedSize(512, 512);
}
