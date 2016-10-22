#include "GLTexture.h"
// #include "SOIL/SOIL.h"

GLTexture::GLTexture(QOpenGLFunctionsType *context, int w, int h, Option &options)
    : context(context), width(w), height(h)
{
    context->glGenTextures(1, &id);

    format = GL_RGBA;
    type = GL_UNSIGNED_BYTE;
    mag_filter = GL_LINEAR;
    min_filter = GL_LINEAR;
    GLenum wrap = GL_REPEAT, wrapS = GL_REPEAT, wrapT = GL_REPEAT;

    if (options.count("format") > 0) format = parseTo<GLenum>(options["format"]);
    if (options.count("type") > 0) type = parseTo<GLenum>(options["type"]);

    if (options.count("wrapS") > 0) wrapS = parseTo<GLenum>(options["wrapS"]);
    if (options.count("wrapT") > 0) wrapT = parseTo<GLenum>(options["wrapT"]);
    if (options.count("wrap") > 0) wrap = wrapS = wrapT = parseTo<GLenum>(options["wrap"]);

    if (options.count("magFilter") > 0) mag_filter = parseTo<GLint>(options["magFilter"]);
    if (options.count("minFilter") > 0) min_filter = parseTo<GLint>(options["minFilter"]);
    if (options.count("filter") > 0) mag_filter = min_filter = parseTo<GLint>(options["filter"]);

    //glGenFramebuffers(1, &framebuffer);
    //glGenRenderbuffers(1, &renderbuffer);
    framebuffer = 0;
    renderbuffer = 0;

    context->glBindTexture(GL_TEXTURE_2D, id);
    // TODO gl.pixelStorei(gl.UNPACK_FLIP_Y_WEBGL, 1);
    context->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, mag_filter);
    context->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, min_filter);
    context->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrapS);
    context->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrapT);

    //  NOTICE this sentence
    if (type == GL_FLOAT) {
        float *f = new float[width*height*4];
        for (int i = 0; i < width*height*4; i++) {
            f[i] = 0.f;
        }
        context->glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, width, height, 0, format, type, f);
        delete [] f;
    }
    else {
        unsigned char *uc = new unsigned char[width*height*4];
        for (int i = 0; i < width*height*4; i++) {
            uc[i] = 0;
        }
        context->glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, type, uc);
        delete [] uc;
    }

}

void GLTexture::bind(int unit /* = 0 */) {
    context->glActiveTexture(GL_TEXTURE0 + unit);
    context->glBindTexture(GL_TEXTURE_2D, id);
}

void GLTexture::unbind(int unit /* = 0 */) {
    context->glActiveTexture(GL_TEXTURE0 + unit);
    context->glBindTexture(GL_TEXTURE_2D, 0);
}

bool GLTexture::canDrawTo() {
    context->glBindFramebuffer(GL_FRAMEBUFFER_EXT, framebuffer);
    context->glFramebufferTexture2D(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, id, 0);
    bool result = context->glCheckFramebufferStatus(GL_FRAMEBUFFER_EXT) == GL_FRAMEBUFFER_COMPLETE_EXT;
    context->glBindFramebuffer(GL_FRAMEBUFFER_EXT, 0);
    return result;
}

void GLTexture::drawTo(std::function< void(void) >& callback, int clear_flag) {
    GLint v[4];
    preDrawTo(v, clear_flag);
    callback();
    postDrawTo(v);
}

void GLTexture::preDrawTo(GLint v[4], int clear_flag) {
    // note: try glPushAttrib( GL_VIEWPORT_BIT );
    context->glGetIntegerv(GL_VIEWPORT, v);
    if (!framebuffer) context->glGenFramebuffers(1, &framebuffer);
    if (!renderbuffer) context->glGenRenderbuffers(1, &renderbuffer);
    context->glBindFramebuffer(GL_FRAMEBUFFER_EXT, framebuffer);
    context->glBindRenderbuffer(GL_RENDERBUFFER_EXT, renderbuffer);
    GLint renderbuffer_width, renderbuffer_height;
    context->glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_WIDTH, &renderbuffer_width);
    context->glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_HEIGHT, &renderbuffer_height);
    if (width != renderbuffer_width || height != renderbuffer_height) {
        // verify this
        context->glRenderbufferStorage(GL_RENDERBUFFER_EXT, GL_DEPTH_COMPONENT24, width, height);
    }
    context->glFramebufferTexture2D(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, id, 0);
    context->glFramebufferRenderbuffer(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, GL_RENDERBUFFER_EXT, renderbuffer);
    if (context->glCheckFramebufferStatus(GL_FRAMEBUFFER_EXT) != GL_FRAMEBUFFER_COMPLETE_EXT) {
        std::cerr << "Rendering to this texture is not supported (incomplete framebuffer)" << std::endl;
    }
    context->glViewport(0, 0, width, height);
    if (clear_flag & 1)
        context->glClear(GL_COLOR_BUFFER_BIT);
    if (clear_flag & 2)
        context->glClear(GL_DEPTH_BUFFER_BIT);

}

void GLTexture::postDrawTo(GLint v[4]) {
    context->glBindFramebuffer(GL_FRAMEBUFFER_EXT, 0);
    context->glBindRenderbuffer(GL_RENDERBUFFER_EXT, 0);
    context->glViewport(v[0], v[1], v[2], v[3]);

}

/*GLTexture *GLTexture::fromImage(const std::string &path, Option &options) {
    int w, h;
    unsigned char* image = SOIL_load_image(path.c_str(), &w, &h, 0, SOIL_LOAD_RGB);

    Texture *texture = new Texture(w, h, options);
    glBindTexture(GL_TEXTURE_2D, texture->id);
    // not sure for this line
    glTexImage2D(GL_TEXTURE_2D, 0, texture->format, texture->width, texture->height, 0, texture->format, texture->type, image);

    if (texture->min_filter != GL_NEAREST && texture->min_filter != GL_LINEAR) {
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    SOIL_free_image_data(image);
    return texture;
}
*/

GLTexture *GLTexture::fromQImage(QOpenGLFunctionsType *context, const QImage &qimg, Option &options) {
    int w = qimg.width(), h = qimg.height();
    uchar *data = new uchar[w*h*4];
    for (int i = 0; i < h; i++) {
        for (int j = 0; j < w; j++) {
            QRgb c = qimg.pixel(j, i);
            data[i*w*4+j*4+0] = qRed(c);
            data[i*w*4+j*4+1] = qGreen(c);
            data[i*w*4+j*4+2] = qBlue(c);
            data[i*w*4+j*4+3] = 1;
        }
    }

    GLTexture *texture = new GLTexture(context, w, h, options);
    context->glBindTexture(GL_TEXTURE_2D, texture->id);
    context->glTexImage2D(GL_TEXTURE_2D, 0, texture->format, texture->width, texture->height, 0, texture->format, texture->type, data);

    if (texture->min_filter != GL_NEAREST && texture->min_filter != GL_LINEAR) {
        context->glGenerateMipmap(GL_TEXTURE_2D);
    }

    delete []data;

    return texture;
}

/*void GLTexture::saveToFile(const std::string &path) {

    unsigned char *img = new unsigned char[width*height*3];
    glBindTexture(GL_TEXTURE_2D, id);
    glGetTexImage(GL_TEXTURE_2D, 0, GL_RGB, GL_UNSIGNED_BYTE, img);
    SOIL_save_image(path.c_str(), SOIL_SAVE_TYPE_BMP, width, height, 3, img);
    delete [] img;
}
*/

void GLTexture::toQImage(QImage &qimg, bool flip_y) {
    unsigned char *data = new unsigned char[width*height*3];
    context->glBindTexture(GL_TEXTURE_2D, id);
    context->glGetTexImage(GL_TEXTURE_2D, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
    qimg = QImage(width, height, QImage::Format_RGB888);
    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            int r = data[i*width*3+j*3+0];
            int g = data[i*width*3+j*3+1];
            int b = data[i*width*3+j*3+2];
            if (flip_y) qimg.setPixel(j, height - 1 - i, qRgb(r,g,b));
            else qimg.setPixel(j, i, qRgb(r,g,b));
        }
    }
}

GLTexture::~GLTexture() {}

void GLTexture::swapWith(GLTexture *that) {
    GLuint tempId = that->id;
    that->id = this->id;
    this->id = tempId;

    int tempW = that->width;
    that->width = this->width;
    this->width = tempW;

    int tempH = that->height;
    that->height = this->height;
    this->height = tempH;
}

