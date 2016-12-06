#include "GLWidget.h"
#include "Vec2.h"
#include "Mat.h"
#include "util.h"
#include <QTimer>
#include "MainWindow.h"

GLWidget::GLWidget(QWidget *parent)
    :  QGLWidget(QGLFormat(QGL::SampleBuffers), parent),
       parent_(parent),
       renderer_(NULL)
{
    setMouseTracking(true);
    camera_controller_.is_static_moving_ = false;
    QGLFormat format;
    format.setSamples(16);
    format.setSampleBuffers(true);
    this->setFormat(format);
}


void GLWidget::InitScene() {
    vsml_ = VSMathLibQT::getInstance();
    vsml_->loadIdentity(VSMathLibQT::MODEL);
    vsml_->loadIdentity(VSMathLibQT::VIEW);
    vsml_->loadIdentity(VSMathLibQT::PROJECTION);

    camera_controller_.SetScreenParameter(this->width(), this->height());

    renderer_= new Renderer(this, vsml_);
    renderer_->Init();

    QTimer *timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), (GLWidget*)(this), SLOT(animate()));
    timer->setSingleShot(false);
    timer->start(25);

    Vec3f light_pos(278/20., 548/20., 279.5/20.);
    camera_controller_.eye_pos_ = Vec3f(274.8/20, 274.4/20, -680/20);
    camera_controller_.look_at_ = Vec3f(274.8/20, 274.4/20, 0);
    camera_controller_.eye_up_ = Vec3f(0, 1, 0);
    camera_controller_.eye_distance_ = (camera_controller_.eye_pos_ - camera_controller_.look_at_).length();
}

void GLWidget::initializeGL() {
    // load opengl functions
    initializeOpenGLFunctions();

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClearDepth(1.0f);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glShadeModel(GL_SMOOTH);
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
    glClampColor(GL_CLAMP_READ_COLOR, GL_FALSE);

    glEnable(GL_POINT_SMOOTH);
    glEnable(GL_LINE_SMOOTH);
    glEnable(GL_POLYGON_SMOOTH);
    glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST);
    glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
    glHint(GL_POINT_SMOOTH_HINT, GL_NICEST);
    glEnable(GL_MULTISAMPLE);
    glEnable(GL_BLEND);
//    glBlendFunc(GL_SRC_ALPHA_SATURATE, GL_ONE);
//    glClampColor(GL_CLAMP_VERTEX_COLOR, GL_FALSE);
//    glClampColor(GL_CLAMP_FRAGMENT_COLOR, GL_FALSE);

    InitScene();
}

void GLWidget::animate() {
    update();           // update() calls paintGL();
}

void GLWidget::paintGL() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    camera_controller_.Update(0);
    UpdateCamera();
    renderer_->Render();
    {
        // save
        QImage qimg = this->grabFrameBuffer();
        QString save_path;
        save_path.sprintf("./save.png");
        qimg.save(save_path);
    }
}

void GLWidget::resizeGL(int w, int h) {
    if(h == 0) h = 1;
    renderer_->set_window_dim(w, h);
    camera_controller_.SetScreenParameter(this->width(), this->height());
    glViewport(0, 0, w, h);
    // set the projection matrix
    float asp = (1.0f * w) / h;
    vsml_->loadIdentity(VSMathLibQT::PROJECTION);
    double fov = 45;
    vsml_->perspective(fov, asp, 1e-2, 1e4);
}


void GLWidget::UpdateCamera() {
    renderer_->SetCamera(camera_controller_.eye_pos_, camera_controller_.look_at_, camera_controller_.eye_up_);
}

void GLWidget::mousePressEvent(QMouseEvent *event) {
    float x = event->x();
    float y = event->y();
    MouseEventArg arg;
    if (event->buttons() == Qt::LeftButton) {
        arg.mouse_button = MouseEventArg::LEFT;
    }
    else if(event->buttons() == Qt::RightButton) {
        arg.mouse_button = MouseEventArg::RIGHT;
    } else if (event->buttons() == Qt::MiddleButton) {
        arg.mouse_button = MouseEventArg::MIDDLE;
    }
    arg.mouse_type = MouseEventArg::PRESS;
    arg.position = Vec3f(x, y, 0.0);
    camera_controller_.HandleMousePressEvent(arg);
}

void GLWidget::mouseReleaseEvent(QMouseEvent *event) {
    MouseEventArg arg;
    arg.mouse_type = MouseEventArg::RELEASE;
    camera_controller_.HandleMouseReleaseEvent(arg);
}

void GLWidget::mouseMoveEvent(QMouseEvent *event) {
    // for parent
    mouse_xy_ = Vec2i(event->x(), event->y());
    UpdateStatusMsg();

    // for camera controller
    float x = event->x();
    float y = event->y();
    MouseEventArg arg;
    arg.mouse_type = MouseEventArg::MOVE;
    arg.position = Vec3f(x, y, 0);
    camera_controller_.HandleMouseMoveEvent(arg);
}

void GLWidget::keyPressEvent(QKeyEvent *event) {
    KeyEventArg arg;
    arg.key_code = event->key();
    arg.key_type = KeyEventArg::PRESS;
    camera_controller_.HandleKeyPressEvent(arg);
}

void GLWidget::keyReleaseEvent(QKeyEvent *event) {
    KeyEventArg arg;
    arg.key_code = event->key();
    arg.key_type = KeyEventArg::RELEASE;
    camera_controller_.HandleKeyReleaseEvent(arg);
}

void GLWidget::wheelEvent(QWheelEvent *event) {
    MouseEventArg arg;
    arg.mouse_type = MouseEventArg::WHEEL;
    arg.degree = event->delta() / 100.f;
    camera_controller_.HandleWheelEvent(arg);
}

void GLWidget::UpdateStatusMsg() {

}

