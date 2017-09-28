/** ----------------------------------------------------------
 * \class VSShaderLib
 *
 * Lighthouse3D
 *
 * VSShaderLib - Very Simple Shader Library
 *
 * Full documentation at
 * http://www.lighthouse3d.com/very-simple-libs
 *
 * This class aims at making life simpler
 * when using shaders and uniforms
 *
 * \version 0.2.2
 *		Added the possibility ot use instance names in a
 *			uniform block
 * \version 0.2.1
 *		Added more attrib defs, namely
 *			tangents, bi tangents, and 4 custom
 *
 * version 0.2.0
 *		Added methods to set uniforms
 *		Added methods to set blocks
 *		Renamed to VSShaderLib
 *
 * version 0.1.0
 * Initial Release
 *
 * This lib requires:
 *
 * GLEW (http://glew.sourceforge.net/)
 *
 ---------------------------------------------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include "VSShaderLibQT.h"

// pre conditions are established with asserts
// if having errors using the lib switch to Debug mode
#include <assert.h>

GLenum
VSShaderLibQT::spGLShaderTypes[VSShaderLibQT::COUNT_SHADER_TYPE] = {
                                GL_VERTEX_SHADER,
                                GL_GEOMETRY_SHADER,
                                GL_TESS_CONTROL_SHADER,
                                GL_TESS_EVALUATION_SHADER,
                                GL_FRAGMENT_SHADER,
                                GL_COMPUTE_SHADER};


std::string
VSShaderLibQT::spStringShaderTypes[VSShaderLibQT::COUNT_SHADER_TYPE] = {
                                "Vertex Shader",
                                "Geometry Shader",
                                "Tesselation Control Shader",
                                "Tesselation Evaluation Shader",
                                "Fragment Shader",
                                "Compute Shader"};


std::map<std::string, VSShaderLibQT::UniformBlock> VSShaderLibQT::spBlocks;


int VSShaderLibQT::spBlockCount = 1;

VSShaderLibQT::VSShaderLibQT(QOpenGLFunctionsType *env)
    : pProgram(0), pInited(false), context(env)
{
    for (int i = 0; i < VSShaderLibQT::COUNT_SHADER_TYPE; ++i) {
        pShader[i] = 0;
    }
}


VSShaderLibQT::~VSShaderLibQT() {

    if (pProgram)
        context->glDeleteProgram(pProgram);

    for (int i = 0; i < VSShaderLibQT::COUNT_SHADER_TYPE; ++i) {
        if (pShader[i])
            context->glDeleteShader(pShader[i]);
    }
    pUniforms.clear();
}


void
VSShaderLibQT::init() {

    pInited = true;
    pProgram = context->glCreateProgram();
}


void
VSShaderLibQT::loadShader(VSShaderLibQT::ShaderType st, std::string fileName) {

    // init should always be called first
    assert(pInited == true);

    char *s = NULL;

    s = textFileRead(fileName);

    if (s != NULL) {
        const char * ss = s;

        pShader[st] = context->glCreateShader(spGLShaderTypes[st]);
        context->glShaderSource(pShader[st], 1, &ss,NULL);
        context->glAttachShader(pProgram, pShader[st]);
        context->glCompileShader(pShader[st]);

        free(s);
    }
}


void
VSShaderLibQT::prepareProgram() {

    context->glLinkProgram(pProgram);
    addUniforms();
    addBlocks();
}


void
VSShaderLibQT::setProgramOutput(int index, std::string name) {

    context->glBindFragDataLocation(pProgram, index, name.c_str());
}


GLint
VSShaderLibQT::getProgramOutput(std::string name) {

    return context->glGetFragDataLocation(pProgram, name.c_str());
}


void
VSShaderLibQT::setVertexAttribName(VSShaderLibQT::AttribType at, std::string name) {

    context->glBindAttribLocation(pProgram,at,name.c_str());
}


GLuint
VSShaderLibQT::getProgramIndex() {

    return pProgram;
}


GLuint
VSShaderLibQT::getShaderIndex(VSShaderLibQT::ShaderType aType) {

    return pShader[aType];
}


void
VSShaderLibQT::setBlock(QOpenGLFunctionsType *context, std::string name, void *value) {

    if (spBlocks.count(name) != 0) {

        context->glBindBuffer(GL_UNIFORM_BUFFER, spBlocks[name].buffer);
        context->glBufferSubData(GL_UNIFORM_BUFFER, 0, spBlocks[name].size, value);
        context->glBindBuffer(GL_UNIFORM_BUFFER,0);
    }

}


void
VSShaderLibQT::setBlockUniform(QOpenGLFunctionsType *context, std::string blockName,
                        std::string uniformName,
                        void *value) {

    if (!spBlocks.count(blockName))
        return;

    std::string uniformComposed = blockName + "." + uniformName;
    std::string finalUniName;

    if (spBlocks[blockName].uniformOffsets.count(uniformName))
        finalUniName = uniformName;
    else if (spBlocks[blockName].uniformOffsets.count(uniformComposed))
        finalUniName = uniformComposed;
    else
        return;
//	if (!(spBlocks.count(blockName) &&
//		   spBlocks[blockName].uniformOffsets.count(uniformName)))
//		   return;

    UniformBlock b;
    b = spBlocks[blockName];

    myBlockUniform bUni;
//	bUni = b.uniformOffsets[uniformName];
    bUni = b.uniformOffsets[finalUniName];
    context->glBindBuffer(GL_UNIFORM_BUFFER, b.buffer);
    context->glBufferSubData(GL_UNIFORM_BUFFER, bUni.offset, bUni.size, value);
    context->glBindBuffer(GL_UNIFORM_BUFFER,0);
}


void
VSShaderLibQT::setBlockUniformArrayElement(QOpenGLFunctionsType *context, std::string blockName,
                                std::string uniformName,
                                int arrayIndex,
                                void * value) {

    assert(spBlocks.count(blockName) &&
           spBlocks[blockName].uniformOffsets.count(uniformName));

    UniformBlock b;
    b = spBlocks[blockName];

    myBlockUniform bUni;
    bUni = b.uniformOffsets[uniformName];

    context->glBindBuffer(GL_UNIFORM_BUFFER, b.buffer);
    context->glBufferSubData(GL_UNIFORM_BUFFER,
                        bUni.offset + bUni.arrayStride * arrayIndex,
                        bUni.arrayStride, value);
    context->glBindBuffer(GL_UNIFORM_BUFFER,0);
}


void
VSShaderLibQT::setUniform(std::string name, int value) {

//	assert(pUniforms.count(name) != 0);
    if (pUniforms.count(name) == 0) {
        std::cout << "fail to set uniform: " << name << std::endl;
        return;
    }

    int val = value;
    myUniforms u = pUniforms[name];
    context->glProgramUniform1i(pProgram, u.location, val);

}


void
VSShaderLibQT::setUniform(std::string name, float value) {

//	assert(pUniforms.count(name) != 0);
    if (pUniforms.count(name) == 0) {
        std::cout << "fail to set uniform: " << name << std::endl;
        return;
    }

    float val = value;
    myUniforms u = pUniforms[name];
    context->glProgramUniform1f(pProgram, u.location, val);
}


void
VSShaderLibQT::setUniform(std::string name, void *value) {

//	assert(pUniforms.count(name) != 0);
    if (pUniforms.count(name) == 0) {
        std::cout << "fail to set uniform: " << name << std::endl;
        return;
    }

    myUniforms u = pUniforms[name];
    switch (u.type) {

        // Floats
        case GL_FLOAT:
            context->glProgramUniform1fv(pProgram, u.location, u.size, (const GLfloat *)value);
            break;
        case GL_FLOAT_VEC2:
            context->glProgramUniform2fv(pProgram, u.location, u.size, (const GLfloat *)value);
            break;
        case GL_FLOAT_VEC3:
            context->glProgramUniform3fv(pProgram, u.location, u.size, (const GLfloat *)value);
            break;
        case GL_FLOAT_VEC4:
            context->glProgramUniform4fv(pProgram, u.location, u.size, (const GLfloat *)value);
            break;

        // Doubles
        case GL_DOUBLE:
            context->glProgramUniform1dv(pProgram, u.location, u.size, (const GLdouble *)value);
            break;
        case GL_DOUBLE_VEC2:
            context->glProgramUniform2dv(pProgram, u.location, u.size, (const GLdouble *)value);
            break;
        case GL_DOUBLE_VEC3:
            context->glProgramUniform3dv(pProgram, u.location, u.size, (const GLdouble *)value);
            break;
        case GL_DOUBLE_VEC4:
            context->glProgramUniform4dv(pProgram, u.location, u.size, (const GLdouble *)value);
            break;

        // Samplers, Ints and Bools
        case GL_IMAGE_1D :
        case GL_IMAGE_2D :
        case GL_IMAGE_3D :
        case GL_IMAGE_2D_RECT :
        case GL_IMAGE_CUBE :
        case GL_IMAGE_BUFFER :
        case GL_IMAGE_1D_ARRAY :
        case GL_IMAGE_2D_ARRAY :
        case GL_IMAGE_CUBE_MAP_ARRAY :
        case GL_IMAGE_2D_MULTISAMPLE :
        case GL_IMAGE_2D_MULTISAMPLE_ARRAY :
        case GL_INT_IMAGE_1D :
        case GL_INT_IMAGE_2D :
        case GL_INT_IMAGE_3D :
        case GL_INT_IMAGE_2D_RECT :
        case GL_INT_IMAGE_CUBE :
        case GL_INT_IMAGE_BUFFER :
        case GL_INT_IMAGE_1D_ARRAY :
        case GL_INT_IMAGE_2D_ARRAY :
        case GL_INT_IMAGE_CUBE_MAP_ARRAY :
        case GL_INT_IMAGE_2D_MULTISAMPLE :
        case GL_INT_IMAGE_2D_MULTISAMPLE_ARRAY :
        case GL_UNSIGNED_INT_IMAGE_1D :
        case GL_UNSIGNED_INT_IMAGE_2D :
        case GL_UNSIGNED_INT_IMAGE_3D :
        case GL_UNSIGNED_INT_IMAGE_2D_RECT :
        case GL_UNSIGNED_INT_IMAGE_CUBE :
        case GL_UNSIGNED_INT_IMAGE_BUFFER :
        case GL_UNSIGNED_INT_IMAGE_1D_ARRAY :
        case GL_UNSIGNED_INT_IMAGE_2D_ARRAY :
        case GL_UNSIGNED_INT_IMAGE_CUBE_MAP_ARRAY :
        case GL_UNSIGNED_INT_IMAGE_2D_MULTISAMPLE :
        case GL_UNSIGNED_INT_IMAGE_2D_MULTISAMPLE_ARRAY :
        case GL_SAMPLER_1D:
        case GL_SAMPLER_2D:
        case GL_SAMPLER_3D:
        case GL_SAMPLER_CUBE:
        case GL_SAMPLER_1D_SHADOW:
        case GL_SAMPLER_2D_SHADOW:
        case GL_SAMPLER_1D_ARRAY:
        case GL_SAMPLER_2D_ARRAY:
        case GL_SAMPLER_1D_ARRAY_SHADOW:
        case GL_SAMPLER_2D_ARRAY_SHADOW:
        case GL_SAMPLER_2D_MULTISAMPLE:
        case GL_SAMPLER_2D_MULTISAMPLE_ARRAY:
        case GL_SAMPLER_CUBE_SHADOW:
        case GL_SAMPLER_BUFFER:
        case GL_SAMPLER_2D_RECT:
        case GL_SAMPLER_2D_RECT_SHADOW:
        case GL_INT_SAMPLER_1D:
        case GL_INT_SAMPLER_2D:
        case GL_INT_SAMPLER_3D:
        case GL_INT_SAMPLER_CUBE:
        case GL_INT_SAMPLER_1D_ARRAY:
        case GL_INT_SAMPLER_2D_ARRAY:
        case GL_INT_SAMPLER_2D_MULTISAMPLE:
        case GL_INT_SAMPLER_2D_MULTISAMPLE_ARRAY:
        case GL_INT_SAMPLER_BUFFER:
        case GL_INT_SAMPLER_2D_RECT:
        case GL_UNSIGNED_INT_SAMPLER_1D:
        case GL_UNSIGNED_INT_SAMPLER_2D:
        case GL_UNSIGNED_INT_SAMPLER_3D:
        case GL_UNSIGNED_INT_SAMPLER_CUBE:
        case GL_UNSIGNED_INT_SAMPLER_1D_ARRAY:
        case GL_UNSIGNED_INT_SAMPLER_2D_ARRAY:
        case GL_UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE:
        case GL_UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE_ARRAY:
        case GL_UNSIGNED_INT_SAMPLER_BUFFER:
        case GL_UNSIGNED_INT_SAMPLER_2D_RECT:
        case GL_BOOL:
        case GL_INT :
            context->glProgramUniform1iv(pProgram, u.location, u.size, (const GLint *)value);
            break;
        case GL_BOOL_VEC2:
        case GL_INT_VEC2:
            context->glProgramUniform2iv(pProgram, u.location, u.size, (const GLint *)value);
            break;
        case GL_BOOL_VEC3:
        case GL_INT_VEC3:
            context->glProgramUniform3iv(pProgram, u.location, u.size, (const GLint *)value);
            break;
        case GL_BOOL_VEC4:
        case GL_INT_VEC4:
            context->glProgramUniform4iv(pProgram, u.location, u.size, (const GLint *)value);
            break;

        // Unsigned ints
        case GL_UNSIGNED_INT:
            context->glProgramUniform1uiv(pProgram, u.location, u.size, (const GLuint *)value);
            break;
        case GL_UNSIGNED_INT_VEC2:
            context->glProgramUniform2uiv(pProgram, u.location, u.size, (const GLuint *)value);
            break;
        case GL_UNSIGNED_INT_VEC3:
            context->glProgramUniform3uiv(pProgram, u.location, u.size, (const GLuint *)value);
            break;
        case GL_UNSIGNED_INT_VEC4:
            context->glProgramUniform4uiv(pProgram, u.location, u.size, (const GLuint *)value);
            break;

        // Float Matrices
        case GL_FLOAT_MAT2:
            context->glProgramUniformMatrix2fv(pProgram, u.location, u.size, false, (const GLfloat *)value);
            break;
        case GL_FLOAT_MAT3:
            context->glProgramUniformMatrix3fv(pProgram, u.location, u.size, false, (const GLfloat *)value);
            break;
        case GL_FLOAT_MAT4:
            context->glProgramUniformMatrix4fv(pProgram, u.location, u.size, false, (const GLfloat *)value);
            break;
        case GL_FLOAT_MAT2x3:
            context->glProgramUniformMatrix2x3fv(pProgram, u.location, u.size, false, (const GLfloat *)value);
            break;
        case GL_FLOAT_MAT2x4:
            context->glProgramUniformMatrix2x4fv(pProgram, u.location, u.size, false, (const GLfloat *)value);
            break;
        case GL_FLOAT_MAT3x2:
            context->glProgramUniformMatrix3x2fv(pProgram, u.location, u.size, false, (const GLfloat *)value);
            break;
        case GL_FLOAT_MAT3x4:
            context->glProgramUniformMatrix3x4fv(pProgram, u.location, u.size, false, (const GLfloat *)value);
            break;
        case GL_FLOAT_MAT4x2:
            context->glProgramUniformMatrix4x2fv(pProgram, u.location, u.size, false, (const GLfloat *)value);
            break;
        case GL_FLOAT_MAT4x3:
            context->glProgramUniformMatrix4x3fv(pProgram, u.location, u.size, false, (const GLfloat *)value);
            break;

        // Double Matrices
        case GL_DOUBLE_MAT2:
            context->glProgramUniformMatrix2dv(pProgram, u.location, u.size, false, (const GLdouble *)value);
            break;
        case GL_DOUBLE_MAT3:
            context->glProgramUniformMatrix3dv(pProgram, u.location, u.size, false, (const GLdouble *)value);
            break;
        case GL_DOUBLE_MAT4:
            context->glProgramUniformMatrix4dv(pProgram, u.location, u.size, false, (const GLdouble *)value);
            break;
        case GL_DOUBLE_MAT2x3:
            context->glProgramUniformMatrix2x3dv(pProgram, u.location, u.size, false, (const GLdouble *)value);
            break;
        case GL_DOUBLE_MAT2x4:
            context->glProgramUniformMatrix2x4dv(pProgram, u.location, u.size, false, (const GLdouble *)value);
            break;
        case GL_DOUBLE_MAT3x2:
            context->glProgramUniformMatrix3x2dv(pProgram, u.location, u.size, false, (const GLdouble *)value);
            break;
        case GL_DOUBLE_MAT3x4:
            context->glProgramUniformMatrix3x4dv(pProgram, u.location, u.size, false, (const GLdouble *)value);
            break;
        case GL_DOUBLE_MAT4x2:
            context->glProgramUniformMatrix4x2dv(pProgram, u.location, u.size, false, (const GLdouble *)value);
            break;
        case GL_DOUBLE_MAT4x3:
            context->glProgramUniformMatrix4x3dv(pProgram, u.location, u.size, false, (const GLdouble *)value);
            break;
    }
}


std::string
VSShaderLibQT::getShaderInfoLog(VSShaderLibQT::ShaderType st) {

    int infologLength = 0;
    int charsWritten  = 0;
    char *infoLog;

    pResult = "";

    if (pShader[st]) {
        context->glGetShaderiv(pShader[st], GL_INFO_LOG_LENGTH,&infologLength);

        if (infologLength > 0)
        {
            infoLog = (char *)malloc(infologLength);
            context->glGetShaderInfoLog(pShader[st], infologLength, &charsWritten, infoLog);
            if (charsWritten)
                pResult = infoLog;
            else
                pResult= "OK";
            free(infoLog);
        }
    }
    else
        pResult = "Shader not loaded";
    return pResult;
}


std::string
VSShaderLibQT::getProgramInfoLog() {

    int infologLength = 0;
    int charsWritten  = 0;
    char *infoLog;

    pResult = "";

    if (pProgram) {

        context->glGetProgramiv(pProgram, GL_INFO_LOG_LENGTH,&infologLength);

        if (infologLength > 0)
        {
            infoLog = (char *)malloc(infologLength);
            context->glGetProgramInfoLog(pProgram, infologLength, &charsWritten, infoLog);
            pResult = infoLog;
            if (charsWritten)
                pResult = infoLog;
            else
                pResult= "OK";
            free(infoLog);
        }
    }
    return pResult;
}


bool
VSShaderLibQT::isProgramValid() {

    GLint b = GL_FALSE;

    if (pProgram) {

        context->glValidateProgram(pProgram);
        context->glGetProgramiv(pProgram, GL_VALIDATE_STATUS,&b);
    }

    return (b != GL_FALSE);
}


bool
VSShaderLibQT::isShaderCompiled(VSShaderLibQT::ShaderType aType) {

    GLint b = GL_FALSE;

    if (pShader[aType]) {

        context->glGetShaderiv(pShader[aType], GL_INFO_LOG_LENGTH, &b);
    }

    return (b != GL_FALSE);
}


bool
VSShaderLibQT::isProgramLinked() {

    GLint b = GL_FALSE;

    if (pProgram) {

        context->glGetProgramiv(pProgram, GL_LINK_STATUS, &b);
    }

    return (b != GL_FALSE);
}


std::string
VSShaderLibQT::getAllInfoLogs() {

    std::string s;

    for (int i = 0; i < VSShaderLibQT::COUNT_SHADER_TYPE; ++i) {
        if (pShader[i]) {
            getShaderInfoLog((VSShaderLibQT::ShaderType)i);
            s += VSShaderLibQT::spStringShaderTypes[i] + ": " + pResult + "\n";
        }
    }

    if (pProgram) {
        getProgramInfoLog();
        s += "Program: " + pResult;
        if (isProgramValid())
            s += " - Valid\n";
        else
            s += " - Not Valid\n";
    }

    pResult = s;
    return pResult;
}


// PRIVATE METHODS

char *
VSShaderLibQT::textFileRead(std::string fileName) {


    FILE *fp;
    char *content = NULL;

    int count=0;

    if (fileName != "") {
        fp = fopen(fileName.c_str(),"rt");

        if (fp != NULL) {

            fseek(fp, 0, SEEK_END);
            count = ftell(fp);
            rewind(fp);

            if (count > 0) {
                content = (char *)malloc(sizeof(char) * (count+1));
                count = fread(content,sizeof(char),count,fp);
                content[count] = '\0';
            }
            fclose(fp);
        }
    }
    return content;
}





void
VSShaderLibQT::addBlocks() {

    int count, dataSize, actualLen, activeUnif, maxUniLength;
    int uniType, uniSize, uniOffset, uniMatStride, uniArrayStride, auxSize;
    char *name, *name2;

    UniformBlock block;

    context->glGetProgramiv(pProgram, GL_ACTIVE_UNIFORM_BLOCKS, &count);

    for (int i = 0; i < count; ++i) {
        // Get buffers name
        context->glGetActiveUniformBlockiv(pProgram, i, GL_UNIFORM_BLOCK_NAME_LENGTH, &actualLen);
        name = (char *)malloc(sizeof(char) * actualLen);
        context->glGetActiveUniformBlockName(pProgram, i, actualLen, NULL, name);

        bool newBlock=true;
        if (spBlocks.count(name)) {
            newBlock = false;
            block = spBlocks[name];
        }

        /*if (!spBlocks.count(name))*/ {
            // Get buffers size
            //block = spBlocks[name];
            context->glGetActiveUniformBlockiv(pProgram, i, GL_UNIFORM_BLOCK_DATA_SIZE, &dataSize);
            //printf("DataSize:%d\n", dataSize);

            if (newBlock) {
                context->glGenBuffers(1, &block.buffer);
                context->glBindBuffer(GL_UNIFORM_BUFFER, block.buffer);
                context->glBufferData(GL_UNIFORM_BUFFER, dataSize, NULL, GL_DYNAMIC_DRAW);
                context->glUniformBlockBinding(pProgram, i, spBlockCount);
                context->glBindBufferRange(GL_UNIFORM_BUFFER, spBlockCount, block.buffer, 0, dataSize);
            }
            else {
                context->glBindBuffer(GL_UNIFORM_BUFFER, block.buffer);
                context->glUniformBlockBinding(pProgram, i, block.bindingIndex);
            }
            context->glGetActiveUniformBlockiv(pProgram, i, GL_UNIFORM_BLOCK_ACTIVE_UNIFORMS, &activeUnif);

            unsigned int *indices;
            indices = (unsigned int *)malloc(sizeof(unsigned int) * activeUnif);
            context->glGetActiveUniformBlockiv(pProgram, i, GL_UNIFORM_BLOCK_ACTIVE_UNIFORM_INDICES, (int *)indices);

            context->glGetProgramiv(pProgram, GL_ACTIVE_UNIFORM_MAX_LENGTH, &maxUniLength);
            name2 = (char *)malloc(sizeof(char) * maxUniLength);

            for (int k = 0; k < activeUnif; ++k) {

                myBlockUniform bUni;

                context->glGetActiveUniformName(pProgram, indices[k], maxUniLength, &actualLen, name2);
                context->glGetActiveUniformsiv(pProgram, 1, &indices[k], GL_UNIFORM_TYPE, &uniType);
                context->glGetActiveUniformsiv(pProgram, 1, &indices[k], GL_UNIFORM_SIZE, &uniSize);
                context->glGetActiveUniformsiv(pProgram, 1, &indices[k], GL_UNIFORM_OFFSET, &uniOffset);
                context->glGetActiveUniformsiv(pProgram, 1, &indices[k], GL_UNIFORM_MATRIX_STRIDE, &uniMatStride);
                context->glGetActiveUniformsiv(pProgram, 1, &indices[k], GL_UNIFORM_ARRAY_STRIDE, &uniArrayStride);

                if (uniArrayStride > 0)
                    auxSize = uniArrayStride * uniSize;

                else if (uniMatStride > 0) {

                    switch(uniType) {
                        case GL_FLOAT_MAT2:
                        case GL_FLOAT_MAT2x3:
                        case GL_FLOAT_MAT2x4:
                        case GL_DOUBLE_MAT2:
                        case GL_DOUBLE_MAT2x3:
                        case GL_DOUBLE_MAT2x4:
                            auxSize = 2 * uniMatStride;
                            break;
                        case GL_FLOAT_MAT3:
                        case GL_FLOAT_MAT3x2:
                        case GL_FLOAT_MAT3x4:
                        case GL_DOUBLE_MAT3:
                        case GL_DOUBLE_MAT3x2:
                        case GL_DOUBLE_MAT3x4:
                            auxSize = 3 * uniMatStride;
                            break;
                        case GL_FLOAT_MAT4:
                        case GL_FLOAT_MAT4x2:
                        case GL_FLOAT_MAT4x3:
                        case GL_DOUBLE_MAT4:
                        case GL_DOUBLE_MAT4x2:
                        case GL_DOUBLE_MAT4x3:
                            auxSize = 4 * uniMatStride;
                            break;
                    }
                }
                else
                    auxSize = typeSize(uniType);

                bUni.offset = uniOffset;
                bUni.type = uniType;
                bUni.size = auxSize;
                bUni.arrayStride = uniArrayStride;

                block.uniformOffsets[name2] = bUni;


            }
            free(name2);
            if (newBlock) {
            block.size = dataSize;
            block.bindingIndex = spBlockCount;
            spBlockCount++;
            }
            spBlocks[name] = block;
        }
        //else
        //	context->glUniformBlockBinding(pProgram, i, spBlocks[name].bindingIndex);

    }

}
//void
//VSShaderLibQT::addBlocks() {
//
//	int count, dataSize, actualLen, activeUnif, maxUniLength;
//	int uniType, uniSize, uniOffset, uniMatStride, uniArrayStride, auxSize;
//	char *name, *name2;
//
//	UniformBlock block;
//
//	context->glGetProgramiv(pProgram, GL_ACTIVE_UNIFORM_BLOCKS, &count);
//
//	for (int i = 0; i < count; ++i) {
//		// Get buffers name
//		context->glGetActiveUniformBlockiv(pProgram, i, GL_UNIFORM_BLOCK_NAME_LENGTH, &actualLen);
//		name = (char *)malloc(sizeof(char) * actualLen);
//		context->glGetActiveUniformBlockName(pProgram, i, actualLen, NULL, name);
//
//		/*if (!spBlocks.count(name))*/ {
//			// Get buffers size
//			block = spBlocks[name];
//			context->glGetActiveUniformBlockiv(pProgram, i, GL_UNIFORM_BLOCK_DATA_SIZE, &dataSize);
//			//printf("DataSize:%d\n", dataSize);
//			context->glGenBuffers(1, &block.buffer);
//			context->glBindBuffer(GL_UNIFORM_BUFFER, block.buffer);
//			context->glBufferData(GL_UNIFORM_BUFFER, dataSize, NULL, GL_DYNAMIC_DRAW);
//			context->glUniformBlockBinding(pProgram, i, spBlockCount);
//			context->glBindBufferRange(GL_UNIFORM_BUFFER, spBlockCount, block.buffer, 0, dataSize);
//
//			context->glGetActiveUniformBlockiv(pProgram, i, GL_UNIFORM_BLOCK_ACTIVE_UNIFORMS, &activeUnif);
//
//			unsigned int *indices;
//			indices = (unsigned int *)malloc(sizeof(unsigned int) * activeUnif);
//			context->glGetActiveUniformBlockiv(pProgram, i, GL_UNIFORM_BLOCK_ACTIVE_UNIFORM_INDICES, (int *)indices);
//
//			context->glGetProgramiv(pProgram, GL_ACTIVE_UNIFORM_MAX_LENGTH, &maxUniLength);
//			name2 = (char *)malloc(sizeof(char) * maxUniLength);
//
//			for (int k = 0; k < activeUnif; ++k) {
//
//				myBlockUniform bUni;
//
//				context->glGetActiveUniformName(pProgram, indices[k], maxUniLength, &actualLen, name2);
//				context->glGetActiveUniformsiv(pProgram, 1, &indices[k], GL_UNIFORM_TYPE, &uniType);
//				context->glGetActiveUniformsiv(pProgram, 1, &indices[k], GL_UNIFORM_SIZE, &uniSize);
//				context->glGetActiveUniformsiv(pProgram, 1, &indices[k], GL_UNIFORM_OFFSET, &uniOffset);
//				context->glGetActiveUniformsiv(pProgram, 1, &indices[k], GL_UNIFORM_MATRIX_STRIDE, &uniMatStride);
//				context->glGetActiveUniformsiv(pProgram, 1, &indices[k], GL_UNIFORM_ARRAY_STRIDE, &uniArrayStride);
//
//				if (uniArrayStride > 0)
//					auxSize = uniArrayStride * uniSize;
//
//				else if (uniMatStride > 0) {
//
//					switch(uniType) {
//						case GL_FLOAT_MAT2:
//						case GL_FLOAT_MAT2x3:
//						case GL_FLOAT_MAT2x4:
//						case GL_DOUBLE_MAT2:
//						case GL_DOUBLE_MAT2x3:
//						case GL_DOUBLE_MAT2x4:
//							auxSize = 2 * uniMatStride;
//							break;
//						case GL_FLOAT_MAT3:
//						case GL_FLOAT_MAT3x2:
//						case GL_FLOAT_MAT3x4:
//						case GL_DOUBLE_MAT3:
//						case GL_DOUBLE_MAT3x2:
//						case GL_DOUBLE_MAT3x4:
//							auxSize = 3 * uniMatStride;
//							break;
//						case GL_FLOAT_MAT4:
//						case GL_FLOAT_MAT4x2:
//						case GL_FLOAT_MAT4x3:
//						case GL_DOUBLE_MAT4:
//						case GL_DOUBLE_MAT4x2:
//						case GL_DOUBLE_MAT4x3:
//							auxSize = 4 * uniMatStride;
//							break;
//					}
//				}
//				else
//					auxSize = typeSize(uniType);
//
//				bUni.offset = uniOffset;
//				bUni.type = uniType;
//				bUni.size = auxSize;
//				bUni.arrayStride = uniArrayStride;
//
//				block.uniformOffsets[name2] = bUni;
//
//
//			}
//			free(name2);
//
//			block.size = dataSize;
//			block.bindingIndex = spBlockCount;
//			spBlocks[name] = block;
//			spBlockCount++;
//		}
//		//else
//			context->glUniformBlockBinding(pProgram, i, spBlocks[name].bindingIndex);
//
//	}
//
//}
void
VSShaderLibQT::addUniforms() {

    int count;
    GLsizei actualLen;
    GLint size;
    GLint uniArrayStride;
    GLenum type;
    char *name;

    int maxUniLength;
    context->glGetProgramiv(pProgram, GL_ACTIVE_UNIFORMS, &count);

    context->glGetProgramiv(pProgram, GL_ACTIVE_UNIFORM_MAX_LENGTH, &maxUniLength);

    name = (char *)malloc(sizeof(char) * maxUniLength);

    unsigned int loc;
    for (int i = 0; i < count; ++i) {

        context->glGetActiveUniform(pProgram, i, maxUniLength, &actualLen, &size, &type, name);
        // -1 indicates that is not an active uniform, although it may be present in a
        // uniform block
        loc = context->glGetUniformLocation(pProgram, name);
        context->glGetActiveUniformsiv(pProgram, 1, &loc, GL_UNIFORM_ARRAY_STRIDE, &uniArrayStride);
        if (loc != -1)
            addUniform(name, type, size);
    }
    free(name);
}


void
VSShaderLibQT::addUniform(std::string name, GLenum type, unsigned int size) {

    myUniforms u;
    u.type = type;
    u.location =  context->glGetUniformLocation(pProgram, name.c_str());
    u.size = size;
    pUniforms[name] = u;
}


int
VSShaderLibQT::typeSize(int type) {

    int s;

    switch(type) {

        case GL_FLOAT:
            s = sizeof(float);
            break;
        case GL_FLOAT_VEC2:
            s = sizeof(float)*2;
            break;
        case GL_FLOAT_VEC3:
            s = sizeof(float)*3;
            break;
        case GL_FLOAT_VEC4:
            s = sizeof(float)*4;
            break;

        // Doubles
        case GL_DOUBLE:
            s = sizeof(double);
            break;
        case GL_DOUBLE_VEC2:
            s = sizeof(double) * 2;
            break;
        case GL_DOUBLE_VEC3:
            s = sizeof(double) * 3;
            break;
        case GL_DOUBLE_VEC4:
            s = sizeof(double) * 4;
            break;

        // Samplers, Ints and Bools
        case GL_SAMPLER_1D:
        case GL_SAMPLER_2D:
        case GL_SAMPLER_3D:
        case GL_SAMPLER_CUBE:
        case GL_SAMPLER_1D_SHADOW:
        case GL_SAMPLER_2D_SHADOW:
        case GL_SAMPLER_1D_ARRAY:
        case GL_SAMPLER_2D_ARRAY:
        case GL_SAMPLER_1D_ARRAY_SHADOW:
        case GL_SAMPLER_2D_ARRAY_SHADOW:
        case GL_SAMPLER_2D_MULTISAMPLE:
        case GL_SAMPLER_2D_MULTISAMPLE_ARRAY:
        case GL_SAMPLER_CUBE_SHADOW:
        case GL_SAMPLER_BUFFER:
        case GL_SAMPLER_2D_RECT:
        case GL_SAMPLER_2D_RECT_SHADOW:
        case GL_INT_SAMPLER_1D:
        case GL_INT_SAMPLER_2D:
        case GL_INT_SAMPLER_3D:
        case GL_INT_SAMPLER_CUBE:
        case GL_INT_SAMPLER_1D_ARRAY:
        case GL_INT_SAMPLER_2D_ARRAY:
        case GL_INT_SAMPLER_2D_MULTISAMPLE:
        case GL_INT_SAMPLER_2D_MULTISAMPLE_ARRAY:
        case GL_INT_SAMPLER_BUFFER:
        case GL_INT_SAMPLER_2D_RECT:
        case GL_UNSIGNED_INT_SAMPLER_1D:
        case GL_UNSIGNED_INT_SAMPLER_2D:
        case GL_UNSIGNED_INT_SAMPLER_3D:
        case GL_UNSIGNED_INT_SAMPLER_CUBE:
        case GL_UNSIGNED_INT_SAMPLER_1D_ARRAY:
        case GL_UNSIGNED_INT_SAMPLER_2D_ARRAY:
        case GL_UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE:
        case GL_UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE_ARRAY:
        case GL_UNSIGNED_INT_SAMPLER_BUFFER:
        case GL_UNSIGNED_INT_SAMPLER_2D_RECT:
        case GL_BOOL:
        case GL_INT :
            s = sizeof(int);
            break;
        case GL_BOOL_VEC2:
        case GL_INT_VEC2:
            s = sizeof(int) * 2;
            break;
        case GL_BOOL_VEC3:
        case GL_INT_VEC3:
            s = sizeof(int) * 3;
            break;
        case GL_BOOL_VEC4:
        case GL_INT_VEC4:
            s = sizeof(int) * 4;
            break;

        // Unsigned ints
        case GL_UNSIGNED_INT:
            s = sizeof(unsigned int);
            break;
        case GL_UNSIGNED_INT_VEC2:
            s = sizeof(unsigned int) * 2;
            break;
        case GL_UNSIGNED_INT_VEC3:
            s = sizeof(unsigned int) * 3;
            break;
        case GL_UNSIGNED_INT_VEC4:
            s = sizeof(unsigned int) * 4;
            break;

        // Float Matrices
        case GL_FLOAT_MAT2:
            s = sizeof(float) * 4;
            break;
        case GL_FLOAT_MAT3:
            s = sizeof(float) * 9;
            break;
        case GL_FLOAT_MAT4:
            s = sizeof(float) * 16;
            break;
        case GL_FLOAT_MAT2x3:
            s = sizeof(float) * 6;
            break;
        case GL_FLOAT_MAT2x4:
            s = sizeof(float) * 8;
            break;
        case GL_FLOAT_MAT3x2:
            s = sizeof(float) * 6;
            break;
        case GL_FLOAT_MAT3x4:
            s = sizeof(float) * 12;
            break;
        case GL_FLOAT_MAT4x2:
            s = sizeof(float) * 8;
            break;
        case GL_FLOAT_MAT4x3:
            s = sizeof(float) * 12;
            break;

        // Double Matrices
        case GL_DOUBLE_MAT2:
            s = sizeof(double) * 4;
            break;
        case GL_DOUBLE_MAT3:
            s = sizeof(double) * 9;
            break;
        case GL_DOUBLE_MAT4:
            s = sizeof(double) * 16;
            break;
        case GL_DOUBLE_MAT2x3:
            s = sizeof(double) * 6;
            break;
        case GL_DOUBLE_MAT2x4:
            s = sizeof(double) * 8;
            break;
        case GL_DOUBLE_MAT3x2:
            s = sizeof(double) * 6;
            break;
        case GL_DOUBLE_MAT3x4:
            s = sizeof(double) * 12;
            break;
        case GL_DOUBLE_MAT4x2:
            s = sizeof(double) * 8;
            break;
        case GL_DOUBLE_MAT4x3:
            s = sizeof(double) * 12;
            break;
        default: return 0;
    }
    return s;
}



