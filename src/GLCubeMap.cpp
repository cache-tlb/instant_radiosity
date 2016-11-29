#include "GLCubeMap.h"

GLCubeMap::GLCubeMap(QOpenGLFunctionsType *context, int width, int height, Option &options) :
    context(context)
{
    context->glGenTextures(1, &id);

    GLenum format = GL_RGBA;
    GLenum type = GL_UNSIGNED_BYTE;
    if (options.count("format") > 0) format = parseTo<GLenum>(options["format"]);
    if (options.count("type") > 0) type = parseTo<GLenum>(options["type"]);


    context->glBindTexture(GL_TEXTURE_CUBE_MAP, id);
    //gl.pixelStorei(gl.UNPACK_FLIP_Y_WEBGL, 1);
    context->glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    context->glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    context->glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    context->glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    if ( type == GL_FLOAT){
        float *f = new float[width*height*4];
        for (int i = 0; i < width*height*4; i++) {
            f[i] = 0.f;
        }
        format = GL_RGBA;
        context->glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_X, 0, GL_RGBA32F, width, height, 0, format, type, f);
        context->glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X, 0, GL_RGBA32F, width, height, 0, format, type, f);
        context->glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, 0, GL_RGBA32F, width, height, 0, format, type, f);
        context->glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Y, 0, GL_RGBA32F, width, height, 0, format, type, f);
        context->glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, 0, GL_RGBA32F, width, height, 0, format, type, f);
        context->glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Z, 0, GL_RGBA32F, width, height, 0, format, type, f);
        delete [] f;
    } else {
        unsigned char *uc = new unsigned char[width*height*4];
        for (int i = 0; i < width*height*4; i++) {
            uc[i] = 0;
        }
        context->glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_X, 0, format, width, height, 0, format, type, uc);
        context->glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X, 0, format, width, height, 0, format, type, uc);
        context->glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, 0, format, width, height, 0, format, type, uc);
        context->glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Y, 0, format, width, height, 0, format, type, uc);
        context->glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, 0, format, width, height, 0, format, type, uc);
        context->glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Z, 0, format, width, height, 0, format, type, uc);
        delete [] uc;
    }

    framebuffer = 0;
    renderbuffer = 0;
    width_vec.resize(6);
    height_vec.resize(6);
    for (int i = 0; i < 6; i++) {
        width_vec[i] = width;
        height_vec[i] = height;
    }
}

GLCubeMap::GLCubeMap(QOpenGLFunctionsType *context, int width[6], int height[6], GLvoid *xn, GLvoid *xp, GLvoid *yn, GLvoid *yp, GLvoid *zn, GLvoid *zp)
  : context(context)
{
    context->glGenTextures(1, &id);
    context->glBindTexture(GL_TEXTURE_CUBE_MAP, id);
    //gl.pixelStorei(gl.UNPACK_FLIP_Y_WEBGL, 1);
    context->glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    context->glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    context->glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    context->glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    // !!! TODO: fix here
    context->glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_X, 0, GL_RGB, width[0], height[0], 0, GL_RGB, GL_UNSIGNED_BYTE, xn);
    context->glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X, 0, GL_RGB, width[1], height[1], 0, GL_RGB, GL_UNSIGNED_BYTE, xp);
    context->glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, 0, GL_RGB, width[2], height[2], 0, GL_RGB, GL_UNSIGNED_BYTE, yn);
    context->glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Y, 0, GL_RGB, width[3], height[3], 0, GL_RGB, GL_UNSIGNED_BYTE, yp);
    context->glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, 0, GL_RGB, width[4], height[4], 0, GL_RGB, GL_UNSIGNED_BYTE, zn);
    context->glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Z, 0, GL_RGB, width[5], height[5], 0, GL_RGB, GL_UNSIGNED_BYTE, zp);

    framebuffer = 0;
    renderbuffer = 0;
    width_vec.resize(6);
    height_vec.resize(6);
    for (int i = 0; i < 6; i++) {
        width_vec[i] = width[i];
        height_vec[i] = height[i];
    }
}

void GLCubeMap::bind(int unit /* = 0 */) {
    context->glActiveTexture(GL_TEXTURE0 + unit);
    context->glBindTexture(GL_TEXTURE_CUBE_MAP, id);
}

void GLCubeMap::unbind(int unit /* = 0 */) {
    context->glActiveTexture(GL_TEXTURE0 + unit);
    context->glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
}

static unsigned char *qimg2uchar(const QImage &qimg, int &w, int &h) {
    w = qimg.width();
    h = qimg.height();
    unsigned char *ret = new unsigned char[w*h*3];
    for (int i = 0; i < h; i++) {
        for (int j = 0; j < w; j++) {
            QRgb c = qimg.pixel(j, i);
            ret[i*w*3+j*3+0] = qRed(c);
            ret[i*w*3+j*3+1] = qGreen(c);
            ret[i*w*3+j*3+2] = qBlue(c);
        }
    }
    return ret;
}

GLCubeMap* GLCubeMap::fromQImages(QOpenGLFunctionsType *context, const QImage &xneg, const QImage &xpos, const QImage &yneg, const QImage &ypos, const QImage &zneg, const QImage &zpos) {
    int w[6], h[6];
    unsigned char* xn = qimg2uchar(xneg, w[0], h[0]);
    unsigned char* xp = qimg2uchar(xpos, w[1], h[1]);
    unsigned char* yn = qimg2uchar(yneg, w[2], h[2]);
    unsigned char* yp = qimg2uchar(ypos, w[3], h[3]);
    unsigned char* zn = qimg2uchar(zneg, w[4], h[4]);
    unsigned char* zp = qimg2uchar(zpos, w[5], h[5]);
    GLCubeMap *cubemap = new GLCubeMap(context, w, h, xn, xp, yn, yp, zn, zp);
    delete [] xn;
    delete [] xp;
    delete [] yn;
    delete [] yp;
    delete [] zn;
    delete [] zp;
    return cubemap;
}

void GLCubeMap::drawTo(std::function<void ()> &callback, int face_index, int clear_flag) {
    GLint v[4];
    preDrawTo(v, face_index, clear_flag);
    callback();
    postDrawTo(v);
}

void GLCubeMap::preDrawTo(GLint v[], int face_index, int clear_flag) {
    // note: try glPushAttrib( GL_VIEWPORT_BIT );
    context->glGetIntegerv(GL_VIEWPORT, v);
    if (!framebuffer) context->glGenFramebuffers(1, &framebuffer);
    if (!renderbuffer) context->glGenRenderbuffers(1, &renderbuffer);
    context->glBindFramebuffer(GL_FRAMEBUFFER_EXT, framebuffer);
    context->glBindRenderbuffer(GL_RENDERBUFFER_EXT, renderbuffer);
    GLint renderbuffer_width, renderbuffer_height;
    context->glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_WIDTH, &renderbuffer_width);
    context->glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_HEIGHT, &renderbuffer_height);
    if (width_vec[face_index] != renderbuffer_width || height_vec[face_index] != renderbuffer_height) {
        // verify this
        context->glRenderbufferStorage(GL_RENDERBUFFER_EXT, GL_DEPTH_COMPONENT24, width_vec[face_index], height_vec[face_index]);
    }
    context->glFramebufferTexture2D(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_CUBE_MAP_POSITIVE_X + face_index, id, 0);
    context->glFramebufferRenderbuffer(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, GL_RENDERBUFFER_EXT, renderbuffer);
    if (context->glCheckFramebufferStatus(GL_FRAMEBUFFER_EXT) != GL_FRAMEBUFFER_COMPLETE_EXT) {
        std::cerr << "Rendering to this texture is not supported (incomplete framebuffer)" << std::endl;
    }
    context->glViewport(0, 0, width_vec[face_index], height_vec[face_index]);
    if (clear_flag & 1)
        context->glClear(GL_COLOR_BUFFER_BIT);
    if (clear_flag & 2)
        context->glClear(GL_DEPTH_BUFFER_BIT);
}

void GLCubeMap::postDrawTo(GLint v[]) {
    context->glBindFramebuffer(GL_FRAMEBUFFER_EXT, 0);
    context->glBindRenderbuffer(GL_RENDERBUFFER_EXT, 0);
    context->glViewport(v[0], v[1], v[2], v[3]);
}
