#ifndef GLCUBEMAP_H
#define GLCUBEMAP_H

#include "common.h"

#include <QImage>

class GLCubeMap {
public:
    GLCubeMap(QOpenGLFunctionsType *context, int width, int height, Option &options);
    GLCubeMap(QOpenGLFunctionsType *context, int width[6], int height[6], GLvoid *xn, GLvoid *xp, GLvoid *yn, GLvoid *yp, GLvoid *zn, GLvoid *zp);

    void bind(int unit = 0);
    void unbind(int unit = 0);

    GLuint id;

    // static Cubemap* fromImages(const std::string &xneg, const std::string &xpos, const std::string &yneg, const std::string &ypos, const std::string &zneg, const std::string &zpos);
    static GLCubeMap* fromQImages(QOpenGLFunctionsType *context, const QImage &xneg, const QImage &xpos, const QImage &yneg, const QImage &ypos, const QImage &zneg, const QImage &zpos);

protected:
    QOpenGLFunctionsType *context;
};

#endif // GLCUBEMAP_H
