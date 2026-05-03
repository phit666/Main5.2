#include "stdafx.h"
#include "mu_2d_renderer.h"
#include "ZzzOpenglUtil.h"

extern bool TextureEnable;

void MU_DrawColorQuad(const MU2DVertex* v,
    GLubyte r, GLubyte g, GLubyte b, GLubyte a)
{
    if (!v)
        return;

    TextureEnable = false;
    glDisable(GL_TEXTURE_2D);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    glEnableClientState(GL_VERTEX_ARRAY);

    glColor4ub(r, g, b, a);

    glVertexPointer(2, GL_FLOAT, sizeof(MU2DVertex), &v[0].x);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

    glDisableClientState(GL_VERTEX_ARRAY);

    glColor4ub(255, 255, 255, 255);
}

void MU_DrawTexturedQuad(int texID, const MU2DVertex* v,
    GLubyte r, GLubyte g, GLubyte b, GLubyte a)
{
    if (!v || texID < 0)
        return;

    TextureEnable = true;
    glEnable(GL_TEXTURE_2D);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    BindTexture(texID);

    glColor4ub(r, g, b, a);

    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);

    glVertexPointer(2, GL_FLOAT, sizeof(MU2DVertex), &v[0].x);
    glTexCoordPointer(2, GL_FLOAT, sizeof(MU2DVertex), &v[0].u);

    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
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

void MU_DrawTexturedQuad3D(
    int texID,
    const MU3DVertex* v,
    GLubyte r,
    GLubyte g,
    GLubyte b,
    GLubyte a)
{
    if (!v || texID < 0)
        return;

    TextureEnable = true;
    glEnable(GL_TEXTURE_2D);

    BindTexture(texID);

    glColor4ub(r, g, b, a);

    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);

    glVertexPointer(3, GL_FLOAT, sizeof(MU3DVertex), &v[0].x);
    glTexCoordPointer(2, GL_FLOAT, sizeof(MU3DVertex), &v[0].u);

    static const GLubyte indices[6] =
    {
        0, 1, 2,
        0, 2, 3
    };

    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_BYTE, indices);

    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    glDisableClientState(GL_VERTEX_ARRAY);

    glColor4ub(255, 255, 255, 255);
}

void MU_DrawTexturedColorQuad3D(int texID, const MU3DColorVertex* v)
{
    if (!v || texID < 0)
        return;

    TextureEnable = true;
    glEnable(GL_TEXTURE_2D);

    BindTexture(texID);

    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glEnableClientState(GL_COLOR_ARRAY);

    glVertexPointer(3, GL_FLOAT, sizeof(MU3DColorVertex), &v[0].x);
    glTexCoordPointer(2, GL_FLOAT, sizeof(MU3DColorVertex), &v[0].u);
    glColorPointer(4, GL_UNSIGNED_BYTE, sizeof(MU3DColorVertex), &v[0].r);

    static const GLubyte indices[6] =
    {
        0, 1, 2,
        0, 2, 3
    };

    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_BYTE, indices);

    glDisableClientState(GL_COLOR_ARRAY);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    glDisableClientState(GL_VERTEX_ARRAY);

    glColor4ub(255, 255, 255, 255);
}

void MU_DrawBoundQuad3D(
    const MU3DVertex* v,
    GLubyte r,
    GLubyte g,
    GLubyte b,
    GLubyte a)
{
    if (!v)
        return;

    TextureEnable = true;
    glEnable(GL_TEXTURE_2D);

    glColor4ub(r, g, b, a);

    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);

    glVertexPointer(3, GL_FLOAT, sizeof(MU3DVertex), &v[0].x);
    glTexCoordPointer(2, GL_FLOAT, sizeof(MU3DVertex), &v[0].u);

    static const GLubyte idx[6] =
    {
        0,1,2,
        0,2,3
    };

    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_BYTE, idx);

    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    glDisableClientState(GL_VERTEX_ARRAY);

    glColor4ub(255, 255, 255, 255);
}