#pragma once
#ifndef _WIN32
#include <GLES2/gl2.h>
#else
#include <GL/glew.h>
#include <GL/gl.h>
#endif
#include <math.h>
#include <string.h>

#define USE_GLES2_PORT

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

extern int  CachTexture;
