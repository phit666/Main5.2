#include "stdafx.h"
#include "mu_2d_renderer.h"
#include "ZzzOpenglUtil.h"

extern bool TextureEnable;

void MU_DrawTexturedQuad(
    int texID,
    const MU2DVertex* v,
    GLubyte r,
    GLubyte g,
    GLubyte b,
    GLubyte a)
{
    if (!v || texID < 0)
        return;

    if (!TextureEnable)
    {
        TextureEnable = true;
        glEnable(GL_TEXTURE_2D);
    }

    BindTexture(texID);

    glColor4ub(r, g, b, a);

    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);

    glVertexPointer(2, GL_FLOAT, sizeof(MU2DVertex), &v[0].x);
    glTexCoordPointer(2, GL_FLOAT, sizeof(MU2DVertex), &v[0].u);

    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    glDisableClientState(GL_VERTEX_ARRAY);
}

void MU_DrawColorQuad(
    const MU2DVertex* v,
    GLubyte r,
    GLubyte g,
    GLubyte b,
    GLubyte a)
{
    if (!v)
        return;

    if (TextureEnable)
    {
        TextureEnable = false;
        glDisable(GL_TEXTURE_2D);
    }

    glColor4ub(r, g, b, a);

    glEnableClientState(GL_VERTEX_ARRAY);

    glVertexPointer(2, GL_FLOAT, sizeof(MU2DVertex), &v[0].x);

    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

    glDisableClientState(GL_VERTEX_ARRAY);

    glColor4ub(255, 255, 255, 255);
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
    MU2DVertex vtx[4];

    vtx[0] = { x,     y,     u,      v };
    vtx[1] = { x,     y + h, u,      v + vh };
    vtx[2] = { x + w, y + h, u + uw, v + vh };
    vtx[3] = { x + w, y,     u + uw, v };

    MU_DrawTexturedQuad(texID, vtx, r, g, b, a);
}