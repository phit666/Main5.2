#include "stdafx.h"
#include "mu_2d_renderer.h"
#include "ZzzOpenglUtil.h"
#include "mu_gles2_matrix.h"

extern bool TextureEnable;

void MU_DrawColorQuad(const MU2DVertex* v,
    GLubyte r, GLubyte g, GLubyte b, GLubyte a)
{
    if (!v)
        return;

    TextureEnable = false;

    glUseProgram(g_muProgram);
    MU_ApplyMatrices();

    if (g_uUseTexture >= 0)
        glUniform1i(g_uUseTexture, 0);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(MU2DVertex), &v[0].x);

    glDisableVertexAttribArray(1);
    glVertexAttrib2f(1, 0.0f, 0.0f);

    glDisableVertexAttribArray(2);
    glVertexAttrib4f(2, r / 255.0f, g / 255.0f, b / 255.0f, a / 255.0f);

    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

    glDisableVertexAttribArray(0);
}

void MU_DrawTexturedQuad(int texID, const MU2DVertex* v,
    GLubyte r, GLubyte g, GLubyte b, GLubyte a)
{
    if (!v || texID < 0)
        return;

    char t[100] = { 0 };
    sprintf(t, "[SDL-DEBUG] MU_DrawTexturedQuad %d", texID);
    OutputDebugStringA(t);

    // convert color to float
    float cr = r / 255.0f;
    float cg = g / 255.0f;
    float cb = b / 255.0f;
    float ca = a / 255.0f;

    glUseProgram(g_muProgram);
    MU_ApplyMatrices();

    if (g_uUseTexture >= 0)
        glUniform1i(g_uUseTexture, 1);

    glActiveTexture(GL_TEXTURE0);
    BindTexture(texID);

    // POSITION (attribute 0)
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE,
        sizeof(MU2DVertex), &v[0].x);

    // UV (attribute 1)
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE,
        sizeof(MU2DVertex), &v[0].u);

    // COLOR (attribute 2) — constant per draw
    glDisableVertexAttribArray(2);
    glVertexAttrib4f(2, cr, cg, cb, ca);

    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
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

void MU_DrawBoundTexturedColorQuad2D(const MU2DColorVertex* v)
{
    if (!v)
        return;

    TextureEnable = true;

    glUseProgram(g_muProgram);
    MU_ApplyMatrices();

    if (g_uUseTexture >= 0)
        glUniform1i(g_uUseTexture, 1);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(MU2DColorVertex), &v[0].x);

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(MU2DColorVertex), &v[0].u);

    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(MU2DColorVertex), &v[0].r);

    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

    glDisableVertexAttribArray(2);
    glDisableVertexAttribArray(1);
    glDisableVertexAttribArray(0);
}

void MU_DrawBoundTexturedQuad2D(const MU2DVertex* v)
{
    if (!v)
        return;

    TextureEnable = true;

    glUseProgram(g_muProgram);
    MU_ApplyMatrices();

    if (g_uUseTexture >= 0)
        glUniform1i(g_uUseTexture, 1);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(MU2DVertex), &v[0].x);

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(MU2DVertex), &v[0].u);

    glDisableVertexAttribArray(2);
    glVertexAttrib4f(2, 1.f, 1.f, 1.f, 1.f);

    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

    glDisableVertexAttribArray(1);
    glDisableVertexAttribArray(0);
}

void MU_DrawTexturedQuad3D(int texID, const MU3DVertex* v,
    GLubyte r, GLubyte g, GLubyte b, GLubyte a)
{
    if (!v || texID < 0)
        return;

    glUseProgram(g_muProgram);
    MU_ApplyMatrices();

    if (g_uUseTexture >= 0)
        glUniform1i(g_uUseTexture, 1);

    glActiveTexture(GL_TEXTURE0);
    BindTexture(texID);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(MU3DVertex), &v[0].x);

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(MU3DVertex), &v[0].u);

    glDisableVertexAttribArray(2);
    glVertexAttrib4f(2, r / 255.0f, g / 255.0f, b / 255.0f, a / 255.0f);

    static const GLubyte idx[6] = { 0, 1, 2, 0, 2, 3 };
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_BYTE, idx);

    glDisableVertexAttribArray(1);
    glDisableVertexAttribArray(0);
}

void MU_DrawTexturedColorQuad3D(int texID, const MU3DColorVertex* v)
{
    if (!v || texID < 0)
        return;

    glUseProgram(g_muProgram);
    MU_ApplyMatrices();

    if (g_uUseTexture >= 0)
        glUniform1i(g_uUseTexture, 1);

    glActiveTexture(GL_TEXTURE0);
    BindTexture(texID);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(MU3DColorVertex), &v[0].x);

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(MU3DColorVertex), &v[0].u);

    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(MU3DColorVertex), &v[0].r);

    static const GLubyte idx[6] = { 0, 1, 2, 0, 2, 3 };
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_BYTE, idx);

    glDisableVertexAttribArray(2);
    glDisableVertexAttribArray(1);
    glDisableVertexAttribArray(0);
}


void MU_DrawBoundQuad3D(const MU3DVertex* v,
    GLubyte r, GLubyte g, GLubyte b, GLubyte a)
{
    if (!v)
        return;

    TextureEnable = true;

    glUseProgram(g_muProgram);
    MU_ApplyMatrices();

    if (g_uUseTexture >= 0)
        glUniform1i(g_uUseTexture, 1);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(MU3DVertex), &v[0].x);

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(MU3DVertex), &v[0].u);

    glDisableVertexAttribArray(2);
    glVertexAttrib4f(2, r / 255.0f, g / 255.0f, b / 255.0f, a / 255.0f);

    static const GLubyte idx[6] = { 0, 1, 2, 0, 2, 3 };

    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_BYTE, idx);

    glDisableVertexAttribArray(1);
    glDisableVertexAttribArray(0);
}

void MU_DrawTexturedColorQuad3D_Bound(const MU3DColorVertex* v)
{
    if (!v)
        return;

    TextureEnable = true;

    glUseProgram(g_muProgram);
    MU_ApplyMatrices();

    if (g_uUseTexture >= 0)
        glUniform1i(g_uUseTexture, 1);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(MU3DColorVertex), &v[0].x);

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(MU3DColorVertex), &v[0].u);

    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(MU3DColorVertex), &v[0].r);

    static const GLubyte idx[6] = { 0, 1, 2, 0, 2, 3 };

    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_BYTE, idx);

    glDisableVertexAttribArray(2);
    glDisableVertexAttribArray(1);
    glDisableVertexAttribArray(0);
}

void MU_DrawLine3D(const vec3_t a, const vec3_t b)
{
    GLfloat verts[6] =
    {
        a[0], a[1], a[2],
        b[0], b[1], b[2]
    };

    glUseProgram(g_muProgram);
    MU_ApplyMatrices();

    if (g_uUseTexture >= 0)
        glUniform1i(g_uUseTexture, 0);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, verts);

    glDisableVertexAttribArray(1);
    glVertexAttrib2f(1, 0.0f, 0.0f);

    glDisableVertexAttribArray(2);
    glVertexAttrib4f(2, 1.0f, 1.0f, 1.0f, 1.0f);

    glDrawArrays(GL_LINES, 0, 2);

    glDisableVertexAttribArray(0);
}



