#pragma once
#ifndef _WIN32
#include <GLES2/gl2.h>
#else
//#include <GL/glew.h>
//#include <GL/gl.h>
#include <SDL.h>
#include "glad.h"
#include <SDL_opengles2.h>
#endif
#include <math.h>
#include <string.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#ifdef _WIN32
#ifndef max
#define max(a,b)            (((a) > (b)) ? (a) : (b))
#endif

#ifndef min
#define min(a,b)            (((a) < (b)) ? (a) : (b))
#endif
#endif

#define USE_GLES2_PORT

typedef float vec_t;
typedef vec_t vec2_t[2];
typedef vec_t vec3_t[3];
typedef vec_t vec4_t[4];
typedef vec_t vec34_t[3][4];

struct MU_Mat4
{
    float m[16];
};

extern MU_Mat4 g_muProjection;
extern MU_Mat4 g_muView;

extern GLuint g_muProgram;
extern GLint g_uProjection;
extern GLint g_uView;
extern GLint g_uTexture;
extern GLint g_uUseTexture;
extern GLint g_uDiscardBlack;
extern GLint g_uMinLight;

extern float g_CurrentColor[4];

void MU_Ortho(MU_Mat4& out, float w, float h);
void MU_LoadIdentity(MU_Mat4& out);
void MU_Perspective(MU_Mat4& out, float fovyDeg, float aspect, float zNear, float zFar);
void MU_ApplyMatrices();
void UpdateProjection();
void InitShader();

inline void MU_Multiply(MU_Mat4& out, const MU_Mat4& a, const MU_Mat4& b)
{
    MU_Mat4 r{};
    for (int row = 0; row < 4; ++row)
    {
        for (int col = 0; col < 4; ++col)
        {
            r.m[col + row * 4] =
                a.m[0 + row * 4] * b.m[col + 0 * 4] +
                a.m[1 + row * 4] * b.m[col + 1 * 4] +
                a.m[2 + row * 4] * b.m[col + 2 * 4] +
                a.m[3 + row * 4] * b.m[col + 3 * 4];
        }
    }
    out = r;
}

inline void MU_Translate(MU_Mat4& m, float x, float y, float z)
{
    MU_Mat4 t;
    MU_LoadIdentity(t);
    t.m[12] = x;
    t.m[13] = y;
    t.m[14] = z;
    MU_Multiply(m, m, t);
}

inline void MU_RotateX(MU_Mat4& m, float deg)
{
    float r = deg * 3.1415926535f / 180.0f;
    float c = cosf(r), s = sinf(r);

    MU_Mat4 t;
    MU_LoadIdentity(t);
    t.m[5] = c;
    t.m[6] = s;
    t.m[9] = -s;
    t.m[10] = c;

    MU_Multiply(m, m, t);
}

inline void MU_RotateY(MU_Mat4& m, float deg)
{
    float r = deg * 3.1415926535f / 180.0f;
    float c = cosf(r), s = sinf(r);

    MU_Mat4 t;
    MU_LoadIdentity(t);
    t.m[0] = c;
    t.m[2] = -s;
    t.m[8] = s;
    t.m[10] = c;

    MU_Multiply(m, m, t);
}

inline void MU_RotateZ(MU_Mat4& m, float deg)
{
    float r = deg * 3.1415926535f / 180.0f;
    float c = cosf(r), s = sinf(r);

    MU_Mat4 t;
    MU_LoadIdentity(t);
    t.m[0] = c;
    t.m[1] = s;
    t.m[4] = -s;
    t.m[5] = c;

    MU_Multiply(m, m, t);
}

inline void MU_CopyViewToCameraMatrix(float dst[3][4])
{
    dst[0][0] = g_muView.m[0];
    dst[0][1] = g_muView.m[4];
    dst[0][2] = g_muView.m[8];
    dst[0][3] = g_muView.m[12];

    dst[1][0] = g_muView.m[1];
    dst[1][1] = g_muView.m[5];
    dst[1][2] = g_muView.m[9];
    dst[1][3] = g_muView.m[13];

    dst[2][0] = g_muView.m[2];
    dst[2][1] = g_muView.m[6];
    dst[2][2] = g_muView.m[10];
    dst[2][3] = g_muView.m[14];
}

extern int  CachTexture;
extern MU_Mat4 g_savedViewForSprite;
void MU_TransformPoint(const MU_Mat4& m, const vec3_t in, vec3_t out);

class Shader {
public:
    GLuint ID; // Your linked Program ID

    void use() { glUseProgram(ID); }

    void setBool(GLuint iID, bool value) const {
        glUniform1i(iID, (int)value);
    }
    void setFloat(GLuint iID, float value) const {
        glUniform1f(iID, value);
    }
    void setVec4(GLuint iID, float x, float y, float z, float w) const {
        glUniform4f(iID, x, y, z, w);
    }
    void setMat4(GLuint iID, const glm::mat4& mat) const {
        glUniformMatrix4fv(iID, 1, GL_FALSE, glm::value_ptr(mat));
    }
};

struct SpriteVertex {
    float x, y; // Position
    float u, v; // Texture Coordinates
};

struct SpriteVertex3D {
    float x, y, z;
    float u, v;
};

struct SpriteVertexFull {
    float x, y, z;
    float u, v;
    float r, g, b, a;
};

struct BitmapVertex {
    float x, y;
    float u, v;
};

extern GLint g_aPosLoc;
extern GLint g_aTexLoc;
extern GLint g_aColorLoc;
extern GLint g_uTexEnabledLoc;
extern GLint g_uAlphaTestLoc;
extern GLint g_uAlphaThresholdLoc;
extern GLint g_uFogEnabledLoc;
extern GLint g_uFogColorLoc;
extern GLint g_uFogDensityLoc;
extern GLint g_uColorLoc;
extern GLint g_uMvpLoc; // projection * model view
extern GLint g_uMvLoc; // model view only
extern GLint g_uFogStartLoc, g_uFogEndLoc;
extern Shader myShader;
extern std::vector<glm::mat4> projectionStack;
extern std::vector<glm::mat4> modelViewStack;

void testprogram();


