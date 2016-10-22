#include "Shader.h"

Shader::Shader(QOpenGLFunctionsType *context, VSMathLibQT *vsml, const std::string &vertexSource, const std::string &fragmentSource)
    : context(context), vsml(vsml), vsshader(context)
{
    vsshader.init();
    vsshader.loadShader(VSShaderLibQT::VERTEX_SHADER, vertexSource);
    vsshader.loadShader(VSShaderLibQT::FRAGMENT_SHADER, fragmentSource);
    vsshader.setProgramOutput(0,"outputF");

    vsshader.setVertexAttribName(VSShaderLibQT::VERTEX_COORD_ATTRIB, "position");
    vsshader.setVertexAttribName(VSShaderLibQT::TEXTURE_COORD_ATTRIB, "uv");
    vsshader.setVertexAttribName(VSShaderLibQT::NORMAL_ATTRIB, "normal");

    vsshader.prepareProgram();

}

void Shader::draw(GLMesh *mesh) {
    // set the uniforms
    context->glUseProgram(vsshader.getProgramIndex());
    vsml->setUniformName(VSMathLibQT::NORMAL, "m_normal");
    vsml->setUniformName(VSMathLibQT::MODEL, "m_model");
    vsml->setUniformName(VSMathLibQT::VIEW, "m_view");
    vsml->setUniformName(VSMathLibQT::VIEW_MODEL, "m_viewModel");
    vsml->setUniformName(VSMathLibQT::PROJECTION, "m_projection");
    vsml->setUniformName(VSMathLibQT::PROJ_VIEW_MODEL, "m_pvm");
    vsml->matricesToGL(context);
    context->glBindVertexArray(mesh->vao);
    context->glDrawElements(GL_TRIANGLES, mesh->triangles.size(), GL_UNSIGNED_INT, 0);
    context->glBindVertexArray(0);
}

void Shader::draw_mesh(GLMesh *mesh) {
    context->glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    draw(mesh);
}

void Shader::draw_wireframe(GLMesh *mesh, float line_width) {
    float old_width = 1;
    context->glGetFloatv(GL_LINE_WIDTH, &old_width);

    context->glLineWidth(line_width);
    context->glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    draw(mesh);

    context->glLineWidth(old_width);
}

void Shader::draw_point(GLMesh *mesh, float point_size) {
    float old_size = 1;
    context->glGetFloatv(GL_POINT_SIZE, &old_size);

    context->glPointSize(point_size);
    context->glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);
    draw(mesh);

    context->glPointSize(old_size);
}

/*void Shader::renderTo(Mesh *mesh, Texture *texture) {
    std::function<void(void)> callback = [this, mesh]() {
        this->draw(mesh);
    };
    texture->drawTo(callback);
}*/

void Shader::uniforms(const std::string &name, void *val) {
    context->glUseProgram(vsshader.getProgramIndex());
    vsshader.setUniform(name, val);
}

void Shader::uniforms(const std::string &name, float val) {
    context->glUseProgram(vsshader.getProgramIndex());
    vsshader.setUniform(name, val);
}

void Shader::uniforms(const std::string &name, int val) {
    context->glUseProgram(vsshader.getProgramIndex());
    vsshader.setUniform(name, val);
}

Shader::~Shader() {}
