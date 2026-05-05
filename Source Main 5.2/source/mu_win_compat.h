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
inline void MU_glColor3f(float r, float g, float b)
{
    glDisableVertexAttribArray(2);
    glVertexAttrib4f(2, r, g, b, 1.0f);
}

inline void MU_glColor4f(float r, float g, float b, float a)
{
    glDisableVertexAttribArray(2);
    glVertexAttrib4f(2, r, g, b, a);
}

inline void MU_glColor3fv(const float* v)
{
    glDisableVertexAttribArray(2);
    glVertexAttrib4f(2, v[0], v[1], v[2], 1.0f);
}

inline void MU_glColor4fv(const float* v)
{
    glDisableVertexAttribArray(2);
    glVertexAttrib4f(2, v[0], v[1], v[2], v[3]);
}

inline void MU_glColor4ub(GLubyte r, GLubyte g, GLubyte b, GLubyte a)
{
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

#define glEnable_TEXTURE_2D()  MU_glEnable_TEXTURE_2D()
#define glDisable_TEXTURE_2D() MU_glDisable_TEXTURE_2D()
#define glColor4ub MU_glColor4ub
#define glColor3ub MU_glColor3ub
#define glColor3f  MU_glColor3f
#define glColor4f  MU_glColor4f
#define glColor3fv MU_glColor3fv
#define glColor4fv MU_glColor4fv
#endif

