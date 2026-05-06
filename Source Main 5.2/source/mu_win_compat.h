#pragma once

#ifdef _WIN32

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <windows.h>

//windows
#include <WinSock2.h>
#include <mmsystem.h>
#include <shellapi.h>
#include <process.h>
#include <ws2tcpip.h>
#include <tchar.h>
#ifndef MU_USE_SDL
#include <Wininet.h>
#endif
#include <crtdbg.h>
#include <strsafe.h>

#else
#include "Compats/win_compat.h"
#endif
#include "mu_wininet_curl_compat.h"
#include "mu_gles2_matrix.h"
#ifdef USE_GLES2_PORT

inline void MU_glColor4f(float r, float g, float b, float a)
{
    g_CurrentColor[0] = r;
    g_CurrentColor[1] = g;
    g_CurrentColor[2] = b;
    g_CurrentColor[3] = a;

    glDisableVertexAttribArray(2);
    glVertexAttrib4f(2, r, g, b, a);
}

inline void MU_glColor3f(float r, float g, float b)
{
    MU_glColor4f(r, g, b, 1.f);
}

inline void MU_glColor3fv(const float* v)
{
    MU_glColor4f(v[0], v[1], v[2], 1.f);
}

inline void MU_glColor4fv(const float* v)
{
    MU_glColor4f(v[0], v[1], v[2], v[3]);
}

inline void MU_glColor4ub(GLubyte r, GLubyte g, GLubyte b, GLubyte a)
{
    g_CurrentColor[0] = r / 255.0f;
    g_CurrentColor[1] = g / 255.0f;
    g_CurrentColor[2] = b / 255.0f;
    g_CurrentColor[3] = a / 255.0f;

    glDisableVertexAttribArray(2);
    glVertexAttrib4f(
        2,
        r / 255.0f,
        g / 255.0f,
        b / 255.0f,
        a / 255.0f
    );
}

inline void MU_glColor3ub(GLubyte r, GLubyte g, GLubyte b)
{
    g_CurrentColor[0] = r / 255.0f;
    g_CurrentColor[1] = g / 255.0f;
    g_CurrentColor[2] = b / 255.0f;
    g_CurrentColor[3] = 1.0f;

    glDisableVertexAttribArray(2);
    glVertexAttrib4f(
        2,
        r / 255.0f,
        g / 255.0f,
        b / 255.0f,
        1.0f
    );
}

inline void MU_glEnable_TEXTURE_2D()
{
    glUseProgram(g_muProgram);
    if (g_uUseTexture >= 0)
        glUniform1i(g_uUseTexture, 1);
}

inline void MU_glDisable_TEXTURE_2D()
{
    glUseProgram(g_muProgram);
    if (g_uUseTexture >= 0)
        glUniform1i(g_uUseTexture, 0);
}

inline void MU_glGetColor4(int flag, vec4_t vec)
{
    vec[0] = g_CurrentColor[0];
    vec[1] = g_CurrentColor[1];
    vec[2] = g_CurrentColor[2];
    vec[3] = g_CurrentColor[3];
}

inline void MU_glGetColor3(int flag, vec3_t vec)
{
    vec[0] = g_CurrentColor[1];
    vec[1] = g_CurrentColor[2];
    vec[2] = g_CurrentColor[3];
}

#define glEnable_TEXTURE_2D()  MU_glEnable_TEXTURE_2D()
#define glDisable_TEXTURE_2D() MU_glDisable_TEXTURE_2D()
#define glColor4ub MU_glColor4ub
#define glColor3ub MU_glColor3ub
#define glColor3f  MU_glColor3f
#define glColor4f  MU_glColor4f
#define glColor3fv MU_glColor3fv
#define glColor4fv MU_glColor4fv

#endif

