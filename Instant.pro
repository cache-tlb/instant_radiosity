#-------------------------------------------------
#
# Project created by QtCreator 2016-07-13T13:39:11
#
#-------------------------------------------------

QT       += core gui opengl

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = Instant
TEMPLATE = app

INCLUDEPATH += ./3rdparty

SOURCES += \
    src/CameraController.cpp \
    src/GLCubeMap.cpp \
    src/GLMesh.cpp \
    src/GLTexture.cpp \
    src/GLWidget.cpp \
    src/main.cpp \
    src/Mat.cpp \
    src/Shader.cpp \
    src/util.cpp \
    src/Vec2.cpp \
    src/Vec3.cpp \
    src/Vec4.cpp \
    src/VSMathLibQT.cpp \
    src/VSShaderLibQT.cpp \
    src/MainWindow.cpp \
    src/Renderer.cpp \
    3rdparty/tiny_obj_loader/tiny_obj_loader.cc \
    src/test.cpp \
    src/Material.cpp \
    src/BBox.cpp \
    src/BVH.cpp \
    src/Triangle.cpp \
    src/Shape.cpp

HEADERS  += \
    src/CameraController.h \
    src/common.h \
    src/GLCubeMap.h \
    src/GLMesh.h \
    src/GLTexture.h \
    src/GLWidget.h \
    src/Mat.h \
    src/OpenGLVersion.h \
    src/Shader.h \
    src/util.h \
    src/Vec2.h \
    src/Vec3.h \
    src/Vec4.h \
    src/VSMathLibQT.h \
    src/VSShaderLibQT.h \
    src/MainWindow.h \
    src/Renderer.h \
    src/IntersectionInfo.h \
    3rdparty/tiny_obj_loader/tiny_obj_loader.h \
    src/Material.h \
    src/Shape.h \
    src/BBox.h \
    src/Ray.h \
    src/BVH.h \
    src/Triangle.h

win32:LIBS += opengl32.lib

DISTFILES += \
    Shaders/shadow_depth_map.fs.glsl \
    Shaders/shadow_depth_map.vs.glsl \
    Shaders/draw_with_shadow.fs.glsl \
    Shaders/draw_with_shadow.vs.glsl \
    Shaders/show_texture.fs.glsl \
    Shaders/show_texture.vs.glsl \
    Shaders/acc_texture.fs.glsl \
    Shaders/acc_texture.vs.glsl \
    Shaders/cubemap.fs.glsl \
    Shaders/cubemap.vs.glsl \
    Shaders/envmap.fs.glsl \
    Shaders/envmap.vs.glsl \
    Shaders/gbuffer.fs.glsl \
    Shaders/gbuffer.vs.glsl \
    Shaders/ssao.fs.glsl \
    Shaders/ssao.vs.glsl \
    Shaders/volumetric_scattering.fs.glsl
