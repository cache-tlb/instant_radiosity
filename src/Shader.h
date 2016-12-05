#ifndef SHADER_H
#define SHADER_H

#include "VSShaderLibQT.h"
#include "VSMathLibQT.h"
#include "GLMesh.h"
#include "GLTexture.h"
#include "OpenGLVersion.h"

class Shader{
public:
    // now the vertex and fragment source mean the file path
    Shader(QOpenGLFunctionsType *context, VSMathLibQT *vsml, const std::string &vertexSource, const std::string &fragmentSource, const Option &extra_attributes = Option());
    ~Shader();

    //GLuint program;
    VSShaderLibQT vsshader;

    void draw_mesh(GLMesh *mesh);
    void draw_wireframe(GLMesh *mesh, float line_width = 1);
    void draw_point(GLMesh *mesh, float point_size = 1);
    void draw(GLMesh *mesh);

    void draw_instance(GLMesh *mesh, int amount);

    // before using renderTo, the uniforms should be set first.
    //void renderTo(Mesh *mesh, Texture *texture);

    void uniforms(const std::string &name, void *val);
    void uniforms(const std::string &name, float val);
    void uniforms(const std::string &name, int val);

private:
    QOpenGLFunctionsType *context;
    VSMathLibQT *vsml;
};

#endif // SHADER_H
