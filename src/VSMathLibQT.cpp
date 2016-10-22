/* --------------------------------------------------

Lighthouse3D

VSMathLib - Very Simple Matrix Library

http://www.lighthouse3d.com/very-simple-libs

----------------------------------------------------*/

#include "VSMathLibQT.h"
#include "VSShaderLibQT.h"
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

// This var keeps track of the single instance of VSMathLib
VSMathLibQT* VSMathLibQT::gInstance = 0;

#ifdef _WIN32
#ifndef M_PI
#define M_PI       3.14159265358979323846
#endif
#endif

static inline float
DegToRad(float degrees)
{
    return (float)(degrees * (M_PI / 180.0f));
};


// Singleton implementation
// use this function to get the instance of VSMathLib
VSMathLibQT*
VSMathLibQT::getInstance (void) {

    if (0 != gInstance)
        return gInstance;
    else
        gInstance = new VSMathLibQT();

    return gInstance;
}


// VSMathLib constructor
VSMathLibQT::VSMathLibQT():
        mInit(false),
        mBlocks(false)
{
    // set all uniform names to ""
    for (int i = 0; i < COUNT_MATRICES; ++i) {
        mUniformName[i] = "";
        mUniformArrayIndex[i] = 0;
    }
    for (int i = 0; i < COUNT_COMPUTED_MATRICES; ++i) {
        mComputedMatUniformName[i] = "";
        mComputedMatUniformArrayIndex[i] = 0;
    }
}


// VSMathLib destructor
VSMathLibQT::~VSMathLibQT()
{
}


void
VSMathLibQT::setUniformBlockName(std::string blockName) {

    mInit = true;
    // We ARE using blocks
    mBlocks = true;
    mBlockName = blockName;
}


void
VSMathLibQT::setUniformName(MatrixTypes matType, std::string uniformName) {

    mInit = true;
    mUniformName[matType] = uniformName;
    mUniformArrayIndex[matType] = 0;
}


void
VSMathLibQT::setUniformName(ComputedMatrixTypes matType, std::string uniformName) {

    mInit = true;
    mComputedMatUniformName[matType] = uniformName;
    mComputedMatUniformArrayIndex[matType] = 0;
}


void
VSMathLibQT::setUniformArrayIndexName(MatrixTypes matType,
                            std::string uniformName, int index) {

    mInit = true;
    mUniformName[matType] = uniformName;
    mUniformArrayIndex[matType] = index;
}


void
VSMathLibQT::setUniformArrayIndexName(ComputedMatrixTypes matType,
                            std::string uniformName, int index) {

    mInit = true;
    mComputedMatUniformName[matType] = uniformName;
    mComputedMatUniformArrayIndex[matType] = index;
}


// glPushMatrix implementation
void
VSMathLibQT::pushMatrix(MatrixTypes aType) {

    float *aux = (float *)malloc(sizeof(float) * 16);
    memcpy(aux, mMatrix[aType], sizeof(float) * 16);
    mMatrixStack[aType].push_back(aux);
}


// glPopMatrix implementation
void
VSMathLibQT::popMatrix(MatrixTypes aType) {

    if (mMatrixStack[aType].size()-1 >= 0) {
        float *m = mMatrixStack[aType][mMatrixStack[aType].size()-1];
        memcpy(mMatrix[aType], m, sizeof(float) * 16);
        mMatrixStack[aType].pop_back();
        free(m);
    }
}


// glLoadIdentity implementation
void
VSMathLibQT::loadIdentity(MatrixTypes aType)
{
    setIdentityMatrix(mMatrix[aType]);
}


// glMultMatrix implementation
void
VSMathLibQT::multMatrix(MatrixTypes aType, float *aMatrix)
{

    float *a, *b, res[16];
    a = mMatrix[aType];
    b = aMatrix;

    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) {
            res[j*4 + i] = 0.0f;
            for (int k = 0; k < 4; ++k) {
                res[j*4 + i] += a[k*4 + i] * b[j*4 + k];
            }
        }
    }
    memcpy(mMatrix[aType], res, 16 * sizeof(float));
}




// glLoadMatrix implementation
void
VSMathLibQT::loadMatrix(MatrixTypes aType, float *aMatrix)
{
    memcpy(mMatrix[aType], aMatrix, 16 * sizeof(float));
}


// glTranslate implementation with matrix selection
void
VSMathLibQT::translate(MatrixTypes aType, float x, float y, float z)
{
    float mat[16];

    setIdentityMatrix(mat);
    mat[12] = x;
    mat[13] = y;
    mat[14] = z;

    multMatrix(aType,mat);
}


// glTranslate on the MODEL matrix
void
VSMathLibQT::translate(float x, float y, float z)
{
    translate(MODEL, x,y,z);
}


// glScale implementation with matrix selection
void
VSMathLibQT::scale(MatrixTypes aType, float x, float y, float z)
{
    float mat[16];

    setIdentityMatrix(mat,4);
    mat[0] = x;
    mat[5] = y;
    mat[10] = z;

    multMatrix(aType,mat);
}


// glScale on the MODELVIEW matrix
void
VSMathLibQT::scale(float x, float y, float z)
{
    scale(MODEL, x, y, z);
}


// glRotate implementation with matrix selection
void
VSMathLibQT::rotate(MatrixTypes aType, float angle, float x, float y, float z)
{
    float mat[16];
    float v[3];

    v[0] = x;
    v[1] = y;
    v[2] = z;

    float radAngle = DegToRad(angle);
    float co = cos(radAngle);
    float si = sin(radAngle);
    normalize(v);
    float x2 = v[0]*v[0];
    float y2 = v[1]*v[1];
    float z2 = v[2]*v[2];

//	mat[0] = x2 + (y2 + z2) * co;
    mat[0] = co + x2 * (1 - co);// + (y2 + z2) * co;
    mat[4] = v[0] * v[1] * (1 - co) - v[2] * si;
    mat[8] = v[0] * v[2] * (1 - co) + v[1] * si;
    mat[12]= 0.0f;

    mat[1] = v[0] * v[1] * (1 - co) + v[2] * si;
//	mat[5] = y2 + (x2 + z2) * co;
    mat[5] = co + y2 * (1 - co);
    mat[9] = v[1] * v[2] * (1 - co) - v[0] * si;
    mat[13]= 0.0f;

    mat[2] = v[0] * v[2] * (1 - co) - v[1] * si;
    mat[6] = v[1] * v[2] * (1 - co) + v[0] * si;
//	mat[10]= z2 + (x2 + y2) * co;
    mat[10]= co + z2 * (1 - co);
    mat[14]= 0.0f;

    mat[3] = 0.0f;
    mat[7] = 0.0f;
    mat[11]= 0.0f;
    mat[15]= 1.0f;

    multMatrix(aType,mat);
}


// glRotate implementation in the MODELVIEW matrix
void
VSMathLibQT::rotate(float angle, float x, float y, float z)
{
    rotate(MODEL,angle,x,y,z);
}


// gluLookAt implementation
void
VSMathLibQT::lookAt(float xPos, float yPos, float zPos,
                    float xLook, float yLook, float zLook,
                    float xUp, float yUp, float zUp)
{
    float dir[3], right[3], up[3];

    up[0] = xUp;	up[1] = yUp;	up[2] = zUp;

    dir[0] =  (xLook - xPos);
    dir[1] =  (yLook - yPos);
    dir[2] =  (zLook - zPos);
    normalize(dir);

    crossProduct(dir,up,right);
    normalize(right);

    crossProduct(right,dir,up);
    normalize(up);

    float m1[16],m2[16];

    m1[0]  = right[0];
    m1[4]  = right[1];
    m1[8]  = right[2];
    m1[12] = 0.0f;

    m1[1]  = up[0];
    m1[5]  = up[1];
    m1[9]  = up[2];
    m1[13] = 0.0f;

    m1[2]  = -dir[0];
    m1[6]  = -dir[1];
    m1[10] = -dir[2];
    m1[14] =  0.0f;

    m1[3]  = 0.0f;
    m1[7]  = 0.0f;
    m1[11] = 0.0f;
    m1[15] = 1.0f;

    setIdentityMatrix(m2,4);
    m2[12] = -xPos;
    m2[13] = -yPos;
    m2[14] = -zPos;

    multMatrix(VIEW, m1);
    multMatrix(VIEW, m2);
}


// gluPerspective implementation
void
VSMathLibQT::perspective(float fov, float ratio, float nearp, float farp)
{
    float projMatrix[16];

    float f = 1.0f / tan (fov * (M_PI / 360.0f));

    setIdentityMatrix(projMatrix,4);

    projMatrix[0] = f / ratio;
    projMatrix[1 * 4 + 1] = f;
    projMatrix[2 * 4 + 2] = (farp + nearp) / (nearp - farp);
    projMatrix[3 * 4 + 2] = (2.0f * farp * nearp) / (nearp - farp);
    projMatrix[2 * 4 + 3] = -1.0f;
    projMatrix[3 * 4 + 3] = 0.0f;

    multMatrix(PROJECTION, projMatrix);
}


// glOrtho implementation
void
VSMathLibQT::ortho(float left, float right,
            float bottom, float top,
            float nearp, float farp)
{
    float m[16];

    setIdentityMatrix(m,4);

    m[0 * 4 + 0] = 2 / (right - left);
    m[1 * 4 + 1] = 2 / (top - bottom);
    m[2 * 4 + 2] = -2 / (farp - nearp);
    m[3 * 4 + 0] = -(right + left) / (right - left);
    m[3 * 4 + 1] = -(top + bottom) / (top - bottom);
    m[3 * 4 + 2] = -(farp + nearp) / (farp - nearp);

    multMatrix(PROJECTION, m);
}


// glFrustum implementation
void
VSMathLibQT::frustum(float left, float right,
            float bottom, float top,
            float nearp, float farp)
{
    float m[16];

    setIdentityMatrix(m,4);

    m[0 * 4 + 0] = 2 * nearp / (right-left);
    m[1 * 4 + 1] = 2 * nearp / (top - bottom);
    m[2 * 4 + 0] = (right + left) / (right - left);
    m[2 * 4 + 1] = (top + bottom) / (top - bottom);
    m[2 * 4 + 2] = - (farp + nearp) / (farp - nearp);
    m[2 * 4 + 3] = -1.0f;
    m[3 * 4 + 2] = - 2 * farp * nearp / (farp-nearp);
    m[3 * 4 + 3] = 0.0f;

    multMatrix(PROJECTION, m);
}


// returns a pointer to the requested matrix
float *
VSMathLibQT::get(MatrixTypes aType)
{
    return mMatrix[aType];
}


// returns a pointer to the requested matrix
float *
VSMathLibQT::get(ComputedMatrixTypes aType)
{
    switch (aType) {

        case NORMAL:
            computeNormalMatrix3x3();
            return mNormal3x3;
            break;
        case NORMAL_VIEW:
            computeNormalViewMatrix3x3();
            return mNormalView3x3;
            break;
        case NORMAL_MODEL:
            computeNormalModelMatrix3x3();
            return mNormalModel3x3;
            break;
        default:
            computeDerivedMatrix(aType);
            return mCompMatrix[aType];
            break;
    }
    // this should never happen!
    return NULL;
}

/* -----------------------------------------------------
             SEND MATRICES TO OPENGL
------------------------------------------------------*/




// universal
void
VSMathLibQT::matrixToGL(MatrixTypes aType, QOpenGLFunctionsType *context)
{
    if (mInit) {

        if (mBlocks) {
            if (mUniformName[aType] != "") {
                if (mUniformArrayIndex[aType]) {
                    VSShaderLibQT::setBlockUniformArrayElement(
                                                        context,
                                                        mBlockName,
                                                        mUniformName[aType],
                                                        mUniformArrayIndex[aType],
                                                        mMatrix[aType]);
                }
                else {
                    VSShaderLibQT::setBlockUniform(context,
                                            mBlockName,
                                            mUniformName[aType],
                                            mMatrix[aType]);
                }
            }
        }
        else {
            int p,loc;
            if (mUniformName[aType] != "") {
                glGetIntegerv(GL_CURRENT_PROGRAM,&p);
                loc = context->glGetUniformLocation(p, mUniformName[aType].c_str());
                context->glProgramUniformMatrix4fv(p, loc, 1, false, mMatrix[aType]);
            }
        }

    }
}

void
VSMathLibQT::matrixToGL(ComputedMatrixTypes aType, QOpenGLFunctionsType *context)
{
    if (mInit) {

        if (mBlocks) {
            if (aType == NORMAL && mComputedMatUniformName[NORMAL] != "") {
                computeNormalMatrix();
                if (mComputedMatUniformArrayIndex[NORMAL])
                    VSShaderLibQT::setBlockUniformArrayElement(
                                    context,
                                    mBlockName,
                                    mComputedMatUniformName[NORMAL],
                                    mComputedMatUniformArrayIndex[NORMAL],
                                    mNormal);
                else
                    VSShaderLibQT::setBlockUniform(
                                    context,
                                    mBlockName,
                                    mComputedMatUniformName[NORMAL],
                                    mNormal);
            }
            else if (mComputedMatUniformName[aType] != "") {
                computeDerivedMatrix(aType);
                if (mComputedMatUniformArrayIndex[aType])
                    VSShaderLibQT::setBlockUniformArrayElement(context,
                                    mBlockName,
                                    mComputedMatUniformName[aType],
                                    mComputedMatUniformArrayIndex[aType],
                                    mCompMatrix[aType]);
                else
                    VSShaderLibQT::setBlockUniform(context,
                                    mBlockName,
                                    mComputedMatUniformName[aType],
                                    mCompMatrix[aType]);
            }
            }
        }
        else {
            int p,loc;
            if (mUniformName[aType] != "") {
                context->glGetIntegerv(GL_CURRENT_PROGRAM,&p);
                loc = context->glGetUniformLocation(p, mUniformName[aType].c_str());
            if (aType == NORMAL && mComputedMatUniformName[NORMAL] != "") {
                computeNormalMatrix3x3();
                loc = context->glGetUniformLocation(p,
                                    mComputedMatUniformName[NORMAL].c_str());
                context->glProgramUniformMatrix3fv(p, loc, 1, false, mNormal3x3);
            }
            else if (mComputedMatUniformName[aType] != "") {
                computeDerivedMatrix(aType);
                loc = context->glGetUniformLocation(p,
                                    mComputedMatUniformName[aType].c_str());
                context->glProgramUniformMatrix4fv(p, loc, 1, false, mCompMatrix[aType]);

            }
        }

    }
}


// Sends all matrices whose respectived uniforms have been named
void
VSMathLibQT::matricesToGL(QOpenGLFunctionsType *context) {

    if (mInit) {

        if (mBlocks) {
            for (int i = 0 ; i < COUNT_MATRICES; ++i ) {
                if (mUniformName[i] != "")
                    if (mUniformArrayIndex[i])
                        VSShaderLibQT::setBlockUniformArrayElement(context,
                                                            mBlockName,
                                                            mUniformName[i],
                                                            mUniformArrayIndex[i],
                                                            mMatrix[i]);
                    else
                        VSShaderLibQT::setBlockUniform(context, mBlockName, mUniformName[i], mMatrix[i]);
            }
            if (mComputedMatUniformName[NORMAL] != "") {
                computeNormalMatrix();
                if (mComputedMatUniformArrayIndex[NORMAL])
                    VSShaderLibQT::setBlockUniformArrayElement(context,
                                    mBlockName,
                                    mComputedMatUniformName[NORMAL],
                                    mComputedMatUniformArrayIndex[NORMAL],
                                    mNormal);
                else
                    VSShaderLibQT::setBlockUniform(context,
                                    mBlockName,
                                    mComputedMatUniformName[NORMAL],
                                    mNormal);
            }
            if (mComputedMatUniformName[NORMAL_VIEW] != "") {
                computeNormalViewMatrix();
                if (mComputedMatUniformArrayIndex[NORMAL_VIEW])
                    VSShaderLibQT::setBlockUniformArrayElement(context,
                                    mBlockName,
                                    mComputedMatUniformName[NORMAL_VIEW],
                                    mComputedMatUniformArrayIndex[NORMAL_VIEW],
                                    mNormalView);
                else
                    VSShaderLibQT::setBlockUniform(context,
                                    mBlockName,
                                    mComputedMatUniformName[NORMAL_VIEW],
                                    mNormalView);
            }
            if (mComputedMatUniformName[NORMAL_MODEL] != "") {
                computeNormalModelMatrix();
                if (mComputedMatUniformArrayIndex[NORMAL_MODEL])
                    VSShaderLibQT::setBlockUniformArrayElement(context,
                                    mBlockName,
                                    mComputedMatUniformName[NORMAL_MODEL],
                                    mComputedMatUniformArrayIndex[NORMAL_MODEL],
                                    mNormalModel);
                else
                    VSShaderLibQT::setBlockUniform(context,
                                    mBlockName,
                                    mComputedMatUniformName[NORMAL_MODEL],
                                    mNormalModel);
            }

            for (int i = 0; i < COUNT_COMPUTED_4x4_MATRICES; ++i) {

                if (mComputedMatUniformName[i] != "") {
                    computeDerivedMatrix((VSMathLibQT::ComputedMatrixTypes)i);
                    if (mComputedMatUniformArrayIndex[i])
                        VSShaderLibQT::setBlockUniformArrayElement(context,
                                        mBlockName,
                                        mComputedMatUniformName[i],
                                        mComputedMatUniformArrayIndex[i],
                                        mCompMatrix[i]);
                    else
                    VSShaderLibQT::setBlockUniform(context,
                                        mBlockName,
                                        mComputedMatUniformName[i],
                                        mCompMatrix[i]);
                }
            }
        }
        else {
            int p,loc;
            context->glGetIntegerv(GL_CURRENT_PROGRAM,&p);
            for (int i = 0; i < COUNT_MATRICES; ++i) {
                if (mUniformName[i] != "") {
                    loc = context->glGetUniformLocation(p, mUniformName[i].c_str());
                    context->glProgramUniformMatrix4fv(p, loc, 1, false, mMatrix[i]);
                }
            }
            if (mComputedMatUniformName[NORMAL] != "") {
                computeNormalMatrix3x3();
                loc = context->glGetUniformLocation(p,
                                    mComputedMatUniformName[NORMAL].c_str());

                context->glProgramUniformMatrix3fv(p, loc, 1, false, mNormal3x3);
            }
            if (mComputedMatUniformName[NORMAL_VIEW] != "") {
                computeNormalViewMatrix3x3();
                loc = context->glGetUniformLocation(p,
                                    mComputedMatUniformName[NORMAL_VIEW].c_str());

                context->glProgramUniformMatrix3fv(p, loc, 1, false, mNormalView3x3);
            }
            if (mComputedMatUniformName[NORMAL_MODEL] != "") {
                computeNormalModelMatrix3x3();
                loc = context->glGetUniformLocation(p,
                                    mComputedMatUniformName[NORMAL_MODEL].c_str());

                context->glProgramUniformMatrix3fv(p, loc, 1, false, mNormalModel3x3);
            }
            for (int i = 0; i < COUNT_COMPUTED_4x4_MATRICES; ++i) {
                if (mComputedMatUniformName[i] != "") {
                    computeDerivedMatrix((VSMathLibQT::ComputedMatrixTypes)i);
                    loc = context->glGetUniformLocation(p,
                                    mComputedMatUniformName[i].c_str());
                    context->glProgramUniformMatrix4fv(p, loc, 1, false, mCompMatrix[i]);
                }
            }
        }

    }
}

// -----------------------------------------------------
//                      AUX functions
// -----------------------------------------------------

// sets the square matrix mat to the identity matrix,
// size refers to the number of rows (or columns)
void
VSMathLibQT::setIdentityMatrix( float *mat, int size) {

    // fill matrix with 0s
    for (int i = 0; i < size * size; ++i)
            mat[i] = 0.0f;

    // fill diagonal with 1s
    for (int i = 0; i < size; ++i)
        mat[i + i * size] = 1.0f;
}


// Compute res = M * point
void
VSMathLibQT::multMatrixPoint(MatrixTypes aType, float *point, float *res) {

    for (int i = 0; i < 4; ++i) {

        res[i] = 0.0f;

        for (int j = 0; j < 4; j++) {

            res[i] += point[j] * mMatrix[aType][j*4 + i];
        }
    }
}

// Compute res = M * point
void
VSMathLibQT::multMatrixPoint(ComputedMatrixTypes aType, float *point, float *res) {


    if (aType == NORMAL) {
        computeNormalMatrix();
        for (int i = 0; i < 3; ++i) {

            res[i] = 0.0f;

            for (int j = 0; j < 3; j++) {

                res[i] += point[j] * mNormal[j*4 + i];
            }
        }
    }
    else if (aType == NORMAL_VIEW) {
        computeNormalViewMatrix();
        for (int i = 0; i < 3; ++i) {

            res[i] = 0.0f;

            for (int j = 0; j < 3; j++) {

                res[i] += point[j] * mNormalView[j*4 + i];
            }
        }
    }
    else if (aType == NORMAL_MODEL) {
        computeNormalModelMatrix();
        for (int i = 0; i < 3; ++i) {

            res[i] = 0.0f;

            for (int j = 0; j < 3; j++) {

                res[i] += point[j] * mNormalModel[j*4 + i];
            }
        }
    }

    else {
        computeDerivedMatrix(aType);
        for (int i = 0; i < 4; ++i) {

            res[i] = 0.0f;

            for (int j = 0; j < 4; j++) {

                res[i] += point[j] * mCompMatrix[aType][j*4 + i];
            }
        }
    }
}

// res = a cross b;
void
VSMathLibQT::crossProduct( float *a, float *b, float *res) {

    res[0] = a[1] * b[2]  -  b[1] * a[2];
    res[1] = a[2] * b[0]  -  b[2] * a[0];
    res[2] = a[0] * b[1]  -  b[0] * a[1];
}


// returns a . b
float
VSMathLibQT::dotProduct(float *a, float *b) {

    float res = a[0] * b[0]  +  a[1] * b[1]  +  a[2] * b[2];

    return res;
}


// Normalize a vec3
void
VSMathLibQT::normalize(float *a) {

    float mag = sqrt(a[0] * a[0]  +  a[1] * a[1]  +  a[2] * a[2]);

    a[0] /= mag;
    a[1] /= mag;
    a[2] /= mag;
}


// res = b - a
void
VSMathLibQT::subtract(float *a, float *b, float *res) {

    res[0] = b[0] - a[0];
    res[1] = b[1] - a[1];
    res[2] = b[2] - a[2];
}


// res = a + b
void
VSMathLibQT::add(float *a, float *b, float *res) {

    res[0] = b[0] + a[0];
    res[1] = b[1] + a[1];
    res[2] = b[2] + a[2];
}


// returns |a|
float
VSMathLibQT::length(float *a) {

    return(sqrt(a[0] * a[0]  +  a[1] * a[1]  +  a[2] * a[2]));

}


static inline int
M3(int i, int j)
{
   return (i*3+j);
};


// computes the derived normal matrix
void
VSMathLibQT::computeNormalMatrix() {

    computeDerivedMatrix(VIEW_MODEL);

    mMat3x3[0] = mCompMatrix[VIEW_MODEL][0];
    mMat3x3[1] = mCompMatrix[VIEW_MODEL][1];
    mMat3x3[2] = mCompMatrix[VIEW_MODEL][2];

    mMat3x3[3] = mCompMatrix[VIEW_MODEL][4];
    mMat3x3[4] = mCompMatrix[VIEW_MODEL][5];
    mMat3x3[5] = mCompMatrix[VIEW_MODEL][6];

    mMat3x3[6] = mCompMatrix[VIEW_MODEL][8];
    mMat3x3[7] = mCompMatrix[VIEW_MODEL][9];
    mMat3x3[8] = mCompMatrix[VIEW_MODEL][10];

    float det, invDet;

    det = mMat3x3[0] * (mMat3x3[4] * mMat3x3[8] - mMat3x3[5] * mMat3x3[7]) +
          mMat3x3[1] * (mMat3x3[5] * mMat3x3[6] - mMat3x3[8] * mMat3x3[3]) +
          mMat3x3[2] * (mMat3x3[3] * mMat3x3[7] - mMat3x3[4] * mMat3x3[6]);

    invDet = 1.0f/det;

    mNormal[0] = (mMat3x3[4] * mMat3x3[8] - mMat3x3[5] * mMat3x3[7]) * invDet;
    mNormal[1] = (mMat3x3[5] * mMat3x3[6] - mMat3x3[8] * mMat3x3[3]) * invDet;
    mNormal[2] = (mMat3x3[3] * mMat3x3[7] - mMat3x3[4] * mMat3x3[6]) * invDet;
    mNormal[3] = 0.0f;
    mNormal[4] = (mMat3x3[2] * mMat3x3[7] - mMat3x3[1] * mMat3x3[8]) * invDet;
    mNormal[5] = (mMat3x3[0] * mMat3x3[8] - mMat3x3[2] * mMat3x3[6]) * invDet;
    mNormal[6] = (mMat3x3[1] * mMat3x3[6] - mMat3x3[7] * mMat3x3[0]) * invDet;
    mNormal[7] = 0.0f;
    mNormal[8] = (mMat3x3[1] * mMat3x3[5] - mMat3x3[4] * mMat3x3[2]) * invDet;
    mNormal[9] = (mMat3x3[2] * mMat3x3[3] - mMat3x3[0] * mMat3x3[5]) * invDet;
    mNormal[10] =(mMat3x3[0] * mMat3x3[4] - mMat3x3[3] * mMat3x3[1]) * invDet;
    mNormal[11] = 0.0;

}


// computes the derived normal matrix for the view matrix
void
VSMathLibQT::computeNormalViewMatrix() {

    mMat3x3[0] = mMatrix[VIEW][0];
    mMat3x3[1] = mMatrix[VIEW][1];
    mMat3x3[2] = mMatrix[VIEW][2];

    mMat3x3[3] = mMatrix[VIEW][4];
    mMat3x3[4] = mMatrix[VIEW][5];
    mMat3x3[5] = mMatrix[VIEW][6];

    mMat3x3[6] = mMatrix[VIEW][8];
    mMat3x3[7] = mMatrix[VIEW][9];
    mMat3x3[8] = mMatrix[VIEW][10];

    float det, invDet;

    det = mMat3x3[0] * (mMat3x3[4] * mMat3x3[8] - mMat3x3[5] * mMat3x3[7]) +
          mMat3x3[1] * (mMat3x3[5] * mMat3x3[6] - mMat3x3[8] * mMat3x3[3]) +
          mMat3x3[2] * (mMat3x3[3] * mMat3x3[7] - mMat3x3[4] * mMat3x3[6]);

    invDet = 1.0f/det;

    mNormalView[0] = (mMat3x3[4] * mMat3x3[8] - mMat3x3[5] * mMat3x3[7]) * invDet;
    mNormalView[1] = (mMat3x3[5] * mMat3x3[6] - mMat3x3[8] * mMat3x3[3]) * invDet;
    mNormalView[2] = (mMat3x3[3] * mMat3x3[7] - mMat3x3[4] * mMat3x3[6]) * invDet;
    mNormalView[3] = 0.0f;
    mNormalView[4] = (mMat3x3[2] * mMat3x3[7] - mMat3x3[1] * mMat3x3[8]) * invDet;
    mNormalView[5] = (mMat3x3[0] * mMat3x3[8] - mMat3x3[2] * mMat3x3[6]) * invDet;
    mNormalView[6] = (mMat3x3[1] * mMat3x3[6] - mMat3x3[7] * mMat3x3[0]) * invDet;
    mNormalView[7] = 0.0f;
    mNormalView[8] = (mMat3x3[1] * mMat3x3[5] - mMat3x3[4] * mMat3x3[2]) * invDet;
    mNormalView[9] = (mMat3x3[2] * mMat3x3[3] - mMat3x3[0] * mMat3x3[5]) * invDet;
    mNormalView[10] =(mMat3x3[0] * mMat3x3[4] - mMat3x3[3] * mMat3x3[1]) * invDet;
    mNormalView[11] = 0.0;

}


// computes the derived normal matrix for the model matrix
void
VSMathLibQT::computeNormalModelMatrix() {

    mMat3x3[0] = mMatrix[MODEL][0];
    mMat3x3[1] = mMatrix[MODEL][1];
    mMat3x3[2] = mMatrix[MODEL][2];

    mMat3x3[3] = mMatrix[MODEL][4];
    mMat3x3[4] = mMatrix[MODEL][5];
    mMat3x3[5] = mMatrix[MODEL][6];

    mMat3x3[6] = mMatrix[MODEL][8];
    mMat3x3[7] = mMatrix[MODEL][9];
    mMat3x3[8] = mMatrix[MODEL][10];

    float det, invDet;

    det = mMat3x3[0] * (mMat3x3[4] * mMat3x3[8] - mMat3x3[5] * mMat3x3[7]) +
          mMat3x3[1] * (mMat3x3[5] * mMat3x3[6] - mMat3x3[8] * mMat3x3[3]) +
          mMat3x3[2] * (mMat3x3[3] * mMat3x3[7] - mMat3x3[4] * mMat3x3[6]);

    invDet = 1.0f/det;

    mNormalModel[0] = (mMat3x3[4] * mMat3x3[8] - mMat3x3[5] * mMat3x3[7]) * invDet;
    mNormalModel[1] = (mMat3x3[5] * mMat3x3[6] - mMat3x3[8] * mMat3x3[3]) * invDet;
    mNormalModel[2] = (mMat3x3[3] * mMat3x3[7] - mMat3x3[4] * mMat3x3[6]) * invDet;
    mNormalModel[3] = 0.0f;
    mNormalModel[4] = (mMat3x3[2] * mMat3x3[7] - mMat3x3[1] * mMat3x3[8]) * invDet;
    mNormalModel[5] = (mMat3x3[0] * mMat3x3[8] - mMat3x3[2] * mMat3x3[6]) * invDet;
    mNormalModel[6] = (mMat3x3[1] * mMat3x3[6] - mMat3x3[7] * mMat3x3[0]) * invDet;
    mNormalModel[7] = 0.0f;
    mNormalModel[8] = (mMat3x3[1] * mMat3x3[5] - mMat3x3[4] * mMat3x3[2]) * invDet;
    mNormalModel[9] = (mMat3x3[2] * mMat3x3[3] - mMat3x3[0] * mMat3x3[5]) * invDet;
    mNormalModel[10] =(mMat3x3[0] * mMat3x3[4] - mMat3x3[3] * mMat3x3[1]) * invDet;
    mNormalModel[11] = 0.0;

}


// computes the derived normal matrix
void
VSMathLibQT::computeNormalMatrix3x3() {

    computeDerivedMatrix(VIEW_MODEL);

    mMat3x3[0] = mCompMatrix[VIEW_MODEL][0];
    mMat3x3[1] = mCompMatrix[VIEW_MODEL][1];
    mMat3x3[2] = mCompMatrix[VIEW_MODEL][2];

    mMat3x3[3] = mCompMatrix[VIEW_MODEL][4];
    mMat3x3[4] = mCompMatrix[VIEW_MODEL][5];
    mMat3x3[5] = mCompMatrix[VIEW_MODEL][6];

    mMat3x3[6] = mCompMatrix[VIEW_MODEL][8];
    mMat3x3[7] = mCompMatrix[VIEW_MODEL][9];
    mMat3x3[8] = mCompMatrix[VIEW_MODEL][10];

    float det, invDet;

    det = mMat3x3[0] * (mMat3x3[4] * mMat3x3[8] - mMat3x3[5] * mMat3x3[7]) +
          mMat3x3[1] * (mMat3x3[5] * mMat3x3[6] - mMat3x3[8] * mMat3x3[3]) +
          mMat3x3[2] * (mMat3x3[3] * mMat3x3[7] - mMat3x3[4] * mMat3x3[6]);

    invDet = 1.0f/det;

    mNormal3x3[0] = (mMat3x3[4] * mMat3x3[8] - mMat3x3[5] * mMat3x3[7]) * invDet;
    mNormal3x3[1] = (mMat3x3[5] * mMat3x3[6] - mMat3x3[8] * mMat3x3[3]) * invDet;
    mNormal3x3[2] = (mMat3x3[3] * mMat3x3[7] - mMat3x3[4] * mMat3x3[6]) * invDet;
    mNormal3x3[3] = (mMat3x3[2] * mMat3x3[7] - mMat3x3[1] * mMat3x3[8]) * invDet;
    mNormal3x3[4] = (mMat3x3[0] * mMat3x3[8] - mMat3x3[2] * mMat3x3[6]) * invDet;
    mNormal3x3[5] = (mMat3x3[1] * mMat3x3[6] - mMat3x3[7] * mMat3x3[0]) * invDet;
    mNormal3x3[6] = (mMat3x3[1] * mMat3x3[5] - mMat3x3[4] * mMat3x3[2]) * invDet;
    mNormal3x3[7] = (mMat3x3[2] * mMat3x3[3] - mMat3x3[0] * mMat3x3[5]) * invDet;
    mNormal3x3[8] = (mMat3x3[0] * mMat3x3[4] - mMat3x3[3] * mMat3x3[1]) * invDet;

}

// computes the derived normal matrix for the view matrix only
void
VSMathLibQT::computeNormalViewMatrix3x3() {


    mMat3x3[0] = mMatrix[VIEW][0];
    mMat3x3[1] = mMatrix[VIEW][1];
    mMat3x3[2] = mMatrix[VIEW][2];

    mMat3x3[3] = mMatrix[VIEW][4];
    mMat3x3[4] = mMatrix[VIEW][5];
    mMat3x3[5] = mMatrix[VIEW][6];

    mMat3x3[6] = mMatrix[VIEW][8];
    mMat3x3[7] = mMatrix[VIEW][9];
    mMat3x3[8] = mMatrix[VIEW][10];

    float det, invDet;

    det = mMat3x3[0] * (mMat3x3[4] * mMat3x3[8] - mMat3x3[5] * mMat3x3[7]) +
          mMat3x3[1] * (mMat3x3[5] * mMat3x3[6] - mMat3x3[8] * mMat3x3[3]) +
          mMat3x3[2] * (mMat3x3[3] * mMat3x3[7] - mMat3x3[4] * mMat3x3[6]);

    invDet = 1.0f/det;

    mNormalView3x3[0] = (mMat3x3[4] * mMat3x3[8] - mMat3x3[5] * mMat3x3[7]) * invDet;
    mNormalView3x3[1] = (mMat3x3[5] * mMat3x3[6] - mMat3x3[8] * mMat3x3[3]) * invDet;
    mNormalView3x3[2] = (mMat3x3[3] * mMat3x3[7] - mMat3x3[4] * mMat3x3[6]) * invDet;
    mNormalView3x3[3] = (mMat3x3[2] * mMat3x3[7] - mMat3x3[1] * mMat3x3[8]) * invDet;
    mNormalView3x3[4] = (mMat3x3[0] * mMat3x3[8] - mMat3x3[2] * mMat3x3[6]) * invDet;
    mNormalView3x3[5] = (mMat3x3[1] * mMat3x3[6] - mMat3x3[7] * mMat3x3[0]) * invDet;
    mNormalView3x3[6] = (mMat3x3[1] * mMat3x3[5] - mMat3x3[4] * mMat3x3[2]) * invDet;
    mNormalView3x3[7] = (mMat3x3[2] * mMat3x3[3] - mMat3x3[0] * mMat3x3[5]) * invDet;
    mNormalView3x3[8] = (mMat3x3[0] * mMat3x3[4] - mMat3x3[3] * mMat3x3[1]) * invDet;

}


// computes the derived normal matrix for the model matrix only
void
VSMathLibQT::computeNormalModelMatrix3x3() {


    mMat3x3[0] = mMatrix[MODEL][0];
    mMat3x3[1] = mMatrix[MODEL][1];
    mMat3x3[2] = mMatrix[MODEL][2];

    mMat3x3[3] = mMatrix[MODEL][4];
    mMat3x3[4] = mMatrix[MODEL][5];
    mMat3x3[5] = mMatrix[MODEL][6];

    mMat3x3[6] = mMatrix[MODEL][8];
    mMat3x3[7] = mMatrix[MODEL][9];
    mMat3x3[8] = mMatrix[MODEL][10];

    float det, invDet;

    det = mMat3x3[0] * (mMat3x3[4] * mMat3x3[8] - mMat3x3[5] * mMat3x3[7]) +
          mMat3x3[1] * (mMat3x3[5] * mMat3x3[6] - mMat3x3[8] * mMat3x3[3]) +
          mMat3x3[2] * (mMat3x3[3] * mMat3x3[7] - mMat3x3[4] * mMat3x3[6]);

    invDet = 1.0f/det;

    mNormalModel3x3[0] = (mMat3x3[4] * mMat3x3[8] - mMat3x3[5] * mMat3x3[7]) * invDet;
    mNormalModel3x3[1] = (mMat3x3[5] * mMat3x3[6] - mMat3x3[8] * mMat3x3[3]) * invDet;
    mNormalModel3x3[2] = (mMat3x3[3] * mMat3x3[7] - mMat3x3[4] * mMat3x3[6]) * invDet;
    mNormalModel3x3[3] = (mMat3x3[2] * mMat3x3[7] - mMat3x3[1] * mMat3x3[8]) * invDet;
    mNormalModel3x3[4] = (mMat3x3[0] * mMat3x3[8] - mMat3x3[2] * mMat3x3[6]) * invDet;
    mNormalModel3x3[5] = (mMat3x3[1] * mMat3x3[6] - mMat3x3[7] * mMat3x3[0]) * invDet;
    mNormalModel3x3[6] = (mMat3x3[1] * mMat3x3[5] - mMat3x3[4] * mMat3x3[2]) * invDet;
    mNormalModel3x3[7] = (mMat3x3[2] * mMat3x3[3] - mMat3x3[0] * mMat3x3[5]) * invDet;
    mNormalModel3x3[8] = (mMat3x3[0] * mMat3x3[4] - mMat3x3[3] * mMat3x3[1]) * invDet;

}


// Computes derived matrices
void
VSMathLibQT::computeDerivedMatrix(ComputedMatrixTypes aType) {

    memcpy(mCompMatrix[VIEW_MODEL], mMatrix[VIEW], 16 * sizeof(float));
    multMatrix(mCompMatrix[VIEW_MODEL], mMatrix[MODEL]);

    if (aType == PROJ_VIEW_MODEL) {
        memcpy(mCompMatrix[PROJ_VIEW_MODEL], mMatrix[PROJECTION], 16 * sizeof(float));
        multMatrix(mCompMatrix[PROJ_VIEW_MODEL], mCompMatrix[VIEW_MODEL]);
    }
}


// aux function resMat = resMat * aMatrix
void
VSMathLibQT::multMatrix(float *resMat, float *aMatrix)
{

    float *a, *b, res[16];
    a = resMat;
    b = aMatrix;

    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) {
            res[j*4 + i] = 0.0f;
            for (int k = 0; k < 4; ++k) {
                res[j*4 + i] += a[k*4 + i] * b[j*4 + k];
            }
        }
    }
    memcpy(a, res, 16 * sizeof(float));
}
