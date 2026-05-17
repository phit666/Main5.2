#pragma once

#include "stdafx.h"

struct MU2DVertex
{
    GLfloat x, y;
    GLfloat u, v;
};

struct MU3DVertex
{
    GLfloat x, y, z;
    GLfloat u, v;
};

struct MU3DColorVertex
{
    GLfloat x, y, z;
    GLfloat u, v;
    GLubyte r, g, b, a;
};

struct MUClothVertex
{
    GLfloat x, y, z;
    GLfloat u, v;
};

struct MUWaterVertex
{
    GLfloat x, y, z;
    GLfloat u, v;
};

struct MU2DColorVertex
{
    GLfloat x, y;
    GLfloat u, v;
    GLubyte r, g, b, a;
};

inline GLubyte MU_FloatToColorByte(float v)
{
    if (v < 0.0f) v = 0.0f;
    if (v > 1.0f) v = 1.0f;
    return (GLubyte)(v * 255.0f);
}



