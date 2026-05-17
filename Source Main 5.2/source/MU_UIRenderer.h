#pragma once

/*
    MU_TextRenderer.h

    SDL_ttf text + 2D helper layer WITHOUT its own shader.

    This version is designed for your existing MU GLES2/OpenGL shader renderer.

    It does:
        - Load up to 4 font slots
        - Measure actual text width/height using TTF_SizeUTF8
        - Render text to temporary OpenGL texture
        - Call YOUR existing renderer through callbacks
        - Provide MU_FillRect / MU_DrawRect through callbacks

    You must provide callbacks:

        MU_2DRenderer_SetCallbacks(MyDrawTexturedQuad, MyFillRect);

    Your MyDrawTexturedQuad should use your existing:
        myShader
        g_aPosLoc
        g_aTexLoc
        g_aColorLoc
        g_uColorLoc
        MU_ApplyMatrices()
        SpriteVertexFull
*/

#include <SDL.h>
#include <SDL_ttf.h>

#if defined(__ANDROID__) || defined(__EMSCRIPTEN__)
    #include <GLES2/gl2.h>
#else
    #if defined(MU_USE_GLAD)
        #include <glad/glad.h>
    #elif defined(MU_USE_GLEW)
        #include <GL/glew.h>
    #else
        #include "glad.h"
    #endif
#endif

#ifdef __cplusplus
extern "C" {
#endif

enum MU_FontSlot
{
    MU_FONT_NORMAL = 0,
    MU_FONT_BIG    = 1,
    MU_FONT_BOLD   = 2,
    MU_FONT_SMALL  = 3,
    MU_FONT_MAX    = 4
};

struct MU_TextSize
{
    int w;
    int h;
};

/*
    Draw callbacks.

    drawTexturedQuad:
        texture = OpenGL texture created from SDL_ttf text surface.
        x,y,w,h = screen position.
        r,g,b,a = color multiplier 0-255.

    fillRect:
        x,y,w,h = screen position.
        r,g,b,a = fill color 0-255.

    You can set fillRect to nullptr if you only need text.
*/
typedef void (*MU_DrawTexturedQuadFn)(
    GLuint texture,
    float x, float y, float w, float h,
    unsigned char r, unsigned char g, unsigned char b, unsigned char a);

typedef void (*MU_FillRectFn)(
    float x, float y, float w, float h,
    unsigned char r, unsigned char g, unsigned char b, unsigned char a);

void MU_2DRenderer_SetCallbacks(MU_DrawTexturedQuadFn drawTexturedQuad,
                                MU_FillRectFn fillRect);

/*
    Init only initializes SDL_ttf and internal font table.
    It does NOT create shaders.
*/
bool MU_2DRenderer_Init(int screenW, int screenH);
bool MU_2DRenderer_InitDefaultFont(const char* fontPath, int fontSize, int screenW, int screenH);
void MU_2DRenderer_Destroy();

void MU_2DRenderer_SetScreenSize(int screenW, int screenH);
bool MU_2DRenderer_IsReady();

bool MU_SetFontSlot(int slot, const char* fontPath, int fontSize);
void MU_ClearFontSlot(int slot);
bool MU_HasFontSlot(int slot);

/*
    Begin/End are now lightweight.
    They exist so your edit control code can keep the same structure.
*/
void MU_2DRenderer_Begin(int screenW, int screenH);
void MU_2DRenderer_End();

void MU_FillRect(float x, float y, float w, float h,
                 unsigned char r, unsigned char g, unsigned char b, unsigned char a);

void MU_DrawRect(float x, float y, float w, float h,
                 unsigned char r, unsigned char g, unsigned char b, unsigned char a);

void MU_DrawRectEx(float x, float y, float w, float h, float thickness,
                   unsigned char r, unsigned char g, unsigned char b, unsigned char a);

void MU_TextOut(float x, float y, const char* text,
                unsigned char r, unsigned char g, unsigned char b, unsigned char a);

void MU_TextOutEx(int fontSlot, float x, float y, const char* text,
                  unsigned char r, unsigned char g, unsigned char b, unsigned char a);

int MU_GetTextWidth(const char* text);
int MU_GetTextHeight(const char* text);
MU_TextSize MU_GetTextSize(const char* text);

int MU_GetTextWidthEx(int fontSlot, const char* text);
int MU_GetTextHeightEx(int fontSlot, const char* text);
MU_TextSize MU_GetTextSizeEx(int fontSlot, const char* text);

bool MU_MeasureText(const char* text, int* outW, int* outH);
bool MU_MeasureTextEx(int fontSlot, const char* text, int* outW, int* outH);

void MU_SetActiveFont(int fontSlot);
int MU_GetActiveFont();


void MU_DrawTexturedQuadCallback(
    GLuint texture,
    float x,
    float y,
    float w,
    float h,
    unsigned char r,
    unsigned char g,
    unsigned char b,
    unsigned char a);

void MU_FillRectCallback(
    float x,
    float y,
    float w,
    float h,
    unsigned char r,
    unsigned char g,
    unsigned char b,
    unsigned char a);
#ifdef __cplusplus
}
#endif
