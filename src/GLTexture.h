#ifndef GLTEXTURE_H
#define GLTEXTURE_H

#include "common.h"
#include <QImage>

class GLTexture {
public:
    GLTexture(QOpenGLFunctionsType *context, int w, int h, Option &options);
    ~GLTexture();

    void bind(int unit = 0);
    void unbind(int unit = 0);
    bool canDrawTo();
    void drawTo(std::function< void(void) >& callback, int clear_flag);
    void swapWith(GLTexture *that);
    // void saveToFile(const std::string &path);
    void toQImage(QImage &qimg, bool flip_y);

    void preDrawTo(GLint v[4], int clear_flag);
    void postDrawTo(GLint v[4]);

    GLuint id;

    int width, height;

    GLenum format;
    GLenum type;

    GLint mag_filter, min_filter;
    static GLTexture *fromImage(QOpenGLFunctionsType *context, const std::string &path, Option &options);
    static GLTexture *fromQImage(QOpenGLFunctionsType *context, const QImage &qimg, Option &options);

protected:
    GLuint framebuffer, renderbuffer;
    QOpenGLFunctionsType *context;
};

#endif // GLTEXTURE_H
