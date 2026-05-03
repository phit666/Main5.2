#pragma once

#ifdef __APPLE__
#include <TargetConditionals.h>
#endif

#if defined(__ANDROID__) || (defined(__APPLE__) && TARGET_OS_IPHONE)
    #define MU_OPENGL_ES 1
#else
    #define MU_OPENGL_ES 0
#endif

#if MU_OPENGL_ES
    #define MU_GL_RGB_INTERNAL  GL_RGB
    #define MU_GL_RGBA_INTERNAL GL_RGBA
#else
    #define MU_GL_RGB_INTERNAL  GL_RGB
    #define MU_GL_RGBA_INTERNAL GL_RGBA
#endif

#ifndef GL_CLAMP_TO_EDGE
#define GL_CLAMP_TO_EDGE 0x812F
#endif