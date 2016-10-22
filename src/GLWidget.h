#ifndef GLWIDGET_H
#define GLWIDGET_H

#include "Mat.h"
#include "GLMesh.h"
#include "Shader.h"
#include "GLTexture.h"
#include "Renderer.h"
#include "CameraController.h"
#include "GLCubeMap.h"
#include <QtOpenGL>

class GLWidget : public QGLWidget, protected QOpenGLFunctionsType
{
    Q_OBJECT
public:
    GLWidget(QWidget *parent);

    void InitScene();

    void UpdateStatusMsg();

    void UpdateCamera();

public slots:
    void animate();
    void initializeGL();
    void paintGL();
    void resizeGL(int w, int h);

protected:
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void keyPressEvent(QKeyEvent *event);
    void keyReleaseEvent(QKeyEvent *event);
    void wheelEvent(QWheelEvent *event);

protected:
    QWidget *parent_;

    VSMathLibQT *vsml_;

    Renderer *renderer_;

    CameraController camera_controller_;
    Vec2i mouse_xy_;
};

#endif // GLWIDGET_H
