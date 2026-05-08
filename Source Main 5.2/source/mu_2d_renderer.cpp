#include "stdafx.h"
#include "mu_2d_renderer.h"
#include "ZzzOpenglUtil.h"
#include "mu_gles2_matrix.h"

extern bool TextureEnable;

bool MU_ShouldDiscardBlack(int texID)
{
    switch (texID) {
    //case (BITMAP_LOG_IN + 16):
    case (BITMAP_LOG_IN + 17):
        return true;
    }
    return false;
}


void MU_DrawColorQuad(const MU2DVertex* v,
    GLubyte r, GLubyte g, GLubyte b, GLubyte a)
{

}

void MU_DrawTexturedQuad(int texID, const MU2DVertex* v,
    GLubyte r, GLubyte g, GLubyte b, GLubyte a)
{

}

void MU_DrawRect(
    float x,
    float y,
    float w,
    float h,
    GLubyte r,
    GLubyte g,
    GLubyte b,
    GLubyte a)
{
    MU2DVertex vtx[4];

    vtx[0] = { x,     y,     0.0f, 0.0f };
    vtx[1] = { x,     y + h, 0.0f, 1.0f };
    vtx[2] = { x + w, y + h, 1.0f, 1.0f };
    vtx[3] = { x + w, y,     1.0f, 0.0f };

    MU_DrawColorQuad(vtx, r, g, b, a);
}

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
    GLubyte r,
    GLubyte g,
    GLubyte b,
    GLubyte a)
{

}

void MU_DrawBoundTexturedColorQuad2D(const MU2DColorVertex* v)
{

}

void MU_DrawBoundTexturedQuad2D(const MU2DVertex* v, int texID)
{

}

void MU_DrawTexturedQuad3D(int texID, const MU3DVertex* v,
    GLubyte r, GLubyte g, GLubyte b, GLubyte a)
{

}

void MU_DrawTexturedColorQuad3D(int texID, const MU3DColorVertex* v)
{

}


void MU_DrawBoundQuad3D(
    const MU3DVertex* v,
    GLubyte r,
    GLubyte g,
    GLubyte b,
    GLubyte a)
{

}

void MU_DrawTexturedColorQuad3D_Bound(const MU3DColorVertex* v)
{

}

void MU_DrawLine3D(const vec3_t a, const vec3_t b)
{

}



