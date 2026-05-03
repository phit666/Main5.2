#pragma once

#include "stdafx.h"

struct MU2DVertex
{
    GLfloat x, y;
    GLfloat u, v;
};

void MU_DrawTexturedQuad(
    int texID,
    const MU2DVertex* v,
    GLubyte r = 255,
    GLubyte g = 255,
    GLubyte b = 255,
    GLubyte a = 255);

void MU_DrawColorQuad(
    const MU2DVertex* v,
    GLubyte r,
    GLubyte g,
    GLubyte b,
    GLubyte a);

void MU_DrawRect(
    float x,
    float y,
    float w,
    float h,
    GLubyte r,
    GLubyte g,
    GLubyte b,
    GLubyte a);

void MU_DrawBitmapQuad(
    int texID,
    float x,
    float y,
    float w,
    float h,
    float u,
    float v,
    float uw,
    float vh,
    GLubyte r = 255,
    GLubyte g = 255,
    GLubyte b = 255,
    GLubyte a = 255);