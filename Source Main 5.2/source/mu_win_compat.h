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
//#include "ZzzOpenglUtil.h"
#ifdef USE_GLES2_PORT

inline void MU_glColor4f(float r, float g, float b, float a)
{
    g_CurrentColor[0] = r;
    g_CurrentColor[1] = g;
    g_CurrentColor[2] = b;
    g_CurrentColor[3] = a;
    GLint isEnabled = 0;
    glGetVertexAttribiv(g_aColorLoc, GL_VERTEX_ATTRIB_ARRAY_ENABLED, &isEnabled);
    if (isEnabled) {
        glDisableVertexAttribArray(g_aColorLoc);
        glVertexAttrib4f(g_aColorLoc, 1.0f, 1.0f, 1.0f, 1.0f);
    }
    myShader.setVec4(g_uColorLoc, r, g, b, a);
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
    glDisableVertexAttribArray(g_aColorLoc);
    glVertexAttrib4f(g_aColorLoc, 1.0f, 1.0f, 1.0f, 1.0f);
    myShader.setVec4(g_uColorLoc, r / 255.0f, g / 255.0f, b / 255.0f, a / 255.0f);
}

inline void MU_glColor3ub(GLubyte r, GLubyte g, GLubyte b)
{
    g_CurrentColor[0] = r / 255.0f;
    g_CurrentColor[1] = g / 255.0f;
    g_CurrentColor[2] = b / 255.0f;
    g_CurrentColor[3] = 1.0f;
    glDisableVertexAttribArray(g_aColorLoc);
    glVertexAttrib4f(g_aColorLoc, 1.0f, 1.0f, 1.0f, 1.0f);
    myShader.setVec4(g_uColorLoc, r / 255.0f, g / 255.0f, b / 255.0f, 1.0f);
}

inline void MU_glEnable_TEXTURE_2D()
{
    myShader.setFloat(g_uTexEnabledLoc, 1.0);
}

inline void MU_glDisable_TEXTURE_2D()
{
    myShader.setFloat(g_uTexEnabledLoc, 0.0);
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

inline void MU_glAlphaFunc(GLenum func, float threshold)
{
    myShader.setFloat(g_uAlphaThresholdLoc, threshold);
}

inline void MU_glPushMatrix()
{
    // Push a copy of the current top matrix
    modelViewStack.push_back(modelViewStack.back());
}

inline void MU_glPopMatrix()
{
    if (modelViewStack.size() > 1)
    {
        modelViewStack.pop_back();
    }

    // CRITICAL: Immediately tell the shader the matrix changed.
    // This replicates the automatic hardware behavior of legacy OpenGL.
    myShader.setMat4(g_uMvpLoc, projectionStack.back() * modelViewStack.back());
    myShader.setMat4(g_uMvLoc, modelViewStack.back()); // For Fog
}

// --- GLES2 Legacy Compatibility Definitions ---

// Missing Texture Environment Constants
#ifndef GL_TEXTURE_ENV
#define GL_TEXTURE_ENV      0x2300
#endif
#ifndef GL_TEXTURE_ENV_MODE
#define GL_TEXTURE_ENV_MODE 0x2200
#endif
#ifndef GL_MODULATE
#define GL_MODULATE         0x2100
#endif
#ifndef GL_DECAL
#define GL_DECAL            0x2101
#endif
#ifndef GL_REPLACE
#define GL_REPLACE          0x1E01
#endif
#ifndef GL_ADD
#define GL_ADD              0x0104
#endif

#ifndef GL_CURRENT_COLOR
#define GL_CURRENT_COLOR                  0x0B00
#endif

// --- GLES2 Alpha Test Compatibility ---

#ifndef GL_ALPHA_TEST
#define GL_ALPHA_TEST 0x0BC0
#endif

void EnableAlphaTest(bool DepthMake = true);

// We use your existing EnableAlphaTest helper to redirect legacy calls
inline void MU_glEnable(GLenum cap) {
    if (cap == GL_ALPHA_TEST) {
        EnableAlphaTest(true);
    }
    else {
        ::glEnable(cap); // Pass standard GLES2 states through
    }
}

inline void MU_glDisable(GLenum cap) {
    if (cap == GL_ALPHA_TEST) {
       EnableAlphaTest(false);
    }
    else {
        ::glDisable(cap);
    }
}

//#define glEnable  MU_glEnable
//#define glDisable MU_glDisable


// The "Do Nothing" Wrappers
inline void MU_glTexEnvf(GLenum target, GLenum pname, GLfloat param) {}
inline void MU_glTexEnvi(GLenum target, GLenum pname, GLint param) {}
inline void MU_glTexEnvfv(GLenum target, GLenum pname, const GLfloat* params) {}

// Macros to redirect legacy calls
#define glTexEnvf   MU_glTexEnvf
#define glTexEnvi   MU_glTexEnvi
#define glTexEnvfv  MU_glTexEnvfv


#define glEnable_TEXTURE_2D()  MU_glEnable_TEXTURE_2D()
#define glDisable_TEXTURE_2D() MU_glDisable_TEXTURE_2D()
#define glColor4ub MU_glColor4ub
#define glColor3ub MU_glColor3ub
#define glColor3f  MU_glColor3f
#define glColor4f  MU_glColor4f
#define glColor3fv MU_glColor3fv
#define glColor4fv MU_glColor4fv
#define glAlphaFunc  MU_glAlphaFunc 
#define glPushMatrix MU_glPushMatrix
#define glPopMatrix  MU_glPopMatrix

#endif

