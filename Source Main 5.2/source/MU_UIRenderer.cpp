#include "stdafx.h"
#include "MU_UIRenderer.h"
#include "Utilities/Log/ErrorReport.h"
#include <cstring>
#include "wt.h"

static TTF_Font* g_fonts[MU_FONT_MAX] = { nullptr, nullptr, nullptr, nullptr };

static int g_screenW = 0;
static int g_screenH = 0;
static bool g_ready = false;

static MU_DrawTexturedQuadFn g_drawTexturedQuad = nullptr;
static MU_FillRectFn g_fillRect = nullptr;

static int g_activeFont = MU_FONT_NORMAL;

void MU_SetActiveFont(int fontSlot)
{
    if (fontSlot < 0 || fontSlot >= MU_FONT_MAX)
        return;

    g_activeFont = fontSlot;
}

int MU_GetActiveFont()
{
    return g_activeFont;
}

static TTF_Font* MU_GetFontSafe(int slot)
{
    if (slot < 0 || slot >= MU_FONT_MAX)
        slot = MU_FONT_NORMAL;

    if (g_fonts[slot])
        return g_fonts[slot];

    return g_fonts[MU_FONT_NORMAL];
}

static GLuint MU_CreateTextureFromSurface(SDL_Surface* src)
{
    if (!src)
        return 0;

    /*
        ABGR8888 gives byte order compatible with GL_RGBA on SDL.
    */
    SDL_Surface* surface =
        SDL_ConvertSurfaceFormat(src, SDL_PIXELFORMAT_RGBA32, 0);

    if (!surface)
        return 0;

    GLint oldTexture = 0;
    glGetIntegerv(GL_TEXTURE_BINDING_2D, &oldTexture);

    GLuint tex = 0;
    glGenTextures(1, &tex);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, tex);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glTexImage2D(
        GL_TEXTURE_2D,
        0,
        GL_RGBA,
        surface->w,
        surface->h,
        0,
        GL_RGBA,
        GL_UNSIGNED_BYTE,
        surface->pixels
    );

    glBindTexture(GL_TEXTURE_2D, oldTexture);

#ifdef GL_CLAMP_TO_EDGE
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
#endif

  /*  struct nk_sdl_device* dev = &sdl.ogl;
    glGenTextures(1, &dev->font_tex);
    glBindTexture(GL_TEXTURE_2D, dev->font_tex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, (GLsizei)width, (GLsizei)height, 0,
        GL_RGBA, GL_UNSIGNED_BYTE, image);*/


    SDL_FreeSurface(surface);
    return tex;
}

void MU_2DRenderer_SetCallbacks(MU_DrawTexturedQuadFn drawTexturedQuad,
                                MU_FillRectFn fillRect)
{
    g_drawTexturedQuad = drawTexturedQuad;
    g_fillRect = fillRect;
}

bool MU_2DRenderer_Init(int screenW, int screenH)
{
    if (g_ready)
        return true;

    if (TTF_WasInit() == 0)
    {
        if (TTF_Init() != 0)
        {
            g_ErrorReport.Write(">TTF_Init failed: %s", TTF_GetError());
            return false;
        }
    }

    g_screenW = screenW;
    g_screenH = screenH;
    g_ready = true;


    MU_2DRenderer_SetCallbacks(MU_DrawTexturedQuadCallback, MU_FillRectCallback);

    return true;
}

bool MU_2DRenderer_InitDefaultFont(const char* fontPath, int fontSize, int screenW, int screenH)
{
    if (!MU_2DRenderer_Init(screenW, screenH))
        return false;

    return MU_SetFontSlot(MU_FONT_NORMAL, fontPath, fontSize);
}

void MU_2DRenderer_Destroy()
{
    for (int i = 0; i < MU_FONT_MAX; ++i)
    {
        if (g_fonts[i])
        {
            TTF_CloseFont(g_fonts[i]);
            g_fonts[i] = nullptr;
        }
    }

    if (TTF_WasInit())
        TTF_Quit();

    g_drawTexturedQuad = nullptr;
    g_fillRect = nullptr;
    g_screenW = 0;
    g_screenH = 0;
    g_ready = false;
}

void MU_2DRenderer_SetScreenSize(int screenW, int screenH)
{
    g_screenW = screenW;
    g_screenH = screenH;
}

bool MU_2DRenderer_IsReady()
{
    return g_ready;
}

bool MU_SetFontSlot(int slot, const char* fontPath, int fontSize)
{
    if (slot < 0 || slot >= MU_FONT_MAX)
        return false;

    if (!fontPath || fontSize <= 0)
        return false;

    if (TTF_WasInit() == 0)
    {
        if (TTF_Init() != 0)
        {
            g_ErrorReport.Write(">TTF_Init failed: %s", TTF_GetError());
            return false;
        }
    }

    TTF_Font* newFont = TTF_OpenFont(fontPath, fontSize);
    if (!newFont)
    {
        g_ErrorReport.Write(">TTF_OpenFont failed: %s", TTF_GetError());
        return false;
    }

    if (g_fonts[slot])
        TTF_CloseFont(g_fonts[slot]);

    g_fonts[slot] = newFont;
    return true;
}

void MU_ClearFontSlot(int slot)
{
    if (slot < 0 || slot >= MU_FONT_MAX)
        return;

    if (g_fonts[slot])
    {
        TTF_CloseFont(g_fonts[slot]);
        g_fonts[slot] = nullptr;
    }
}

bool MU_HasFontSlot(int slot)
{
    if (slot < 0 || slot >= MU_FONT_MAX)
        return false;

    return g_fonts[slot] != nullptr;
}

void MU_2DRenderer_Begin(int screenW, int screenH)
{
    g_screenW = screenW;
    g_screenH = screenH;

    /*
        Intentionally no shader/state setup here.
        Your renderer owns the GL state.
    */
}

void MU_2DRenderer_End()
{
    /*
        Intentionally empty.
        Your renderer owns cleanup/state restore.
    */
}

void MU_FillRect(float x, float y, float w, float h,
                 unsigned char r, unsigned char g, unsigned char b, unsigned char a)
{
    if (!g_ready || !g_fillRect)
        return;

    if (w <= 0.0f || h <= 0.0f)
        return;

    g_fillRect(x, y, w, h, r, g, b, a);
}

void MU_DrawRectEx(float x, float y, float w, float h, float thickness,
                   unsigned char r, unsigned char g, unsigned char b, unsigned char a)
{
    if (thickness <= 0.0f)
        thickness = 1.0f;

    MU_FillRect(x, y, w, thickness, r, g, b, a);
    MU_FillRect(x, y + h - thickness, w, thickness, r, g, b, a);
    MU_FillRect(x, y, thickness, h, r, g, b, a);
    MU_FillRect(x + w - thickness, y, thickness, h, r, g, b, a);
}

void MU_DrawRect(float x, float y, float w, float h,
                 unsigned char r, unsigned char g, unsigned char b, unsigned char a)
{
    MU_DrawRectEx(x, y, w, h, 1.0f, r, g, b, a);
}

void MU_TextOut(float x, float y, const char* text,
    unsigned char r,
    unsigned char g,
    unsigned char b,
    unsigned char a)
{
    MU_TextOutEx(g_activeFont, x, y, text, r, g, b, a);
}

void MU_TextOutEx(int fontSlot, float x, float y, const char* text,
                  unsigned char r, unsigned char g, unsigned char b, unsigned char a)
{
    if (!g_ready || !g_drawTexturedQuad)
        return;

    if (!text || text[0] == '\0')
        return;

    TTF_Font* font = MU_GetFontSafe(fontSlot);
    if (!font)
        return;

    SDL_Color white = { 255, 255, 255, 255 };

    SDL_Surface* surf = TTF_RenderUTF8_Blended(font, text, white);
    if (!surf)
        return;

    int tw = surf->w;
    int th = surf->h;

    SDL_Surface* surface =
        SDL_ConvertSurfaceFormat(surf, SDL_PIXELFORMAT_RGBA32, 0);

    if (!surface)
        return;

    GLint oldTexture = 0;
    glGetIntegerv(GL_TEXTURE_BINDING_2D, &oldTexture);

    GLuint tex = 0;
    glGenTextures(1, &tex);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, tex);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glTexImage2D(
        GL_TEXTURE_2D,
        0,
        GL_RGBA,
        surface->w,
        surface->h,
        0,
        GL_RGBA,
        GL_UNSIGNED_BYTE,
        surface->pixels
    );

    SDL_FreeSurface(surf);

    if (!tex)
       return;

    g_drawTexturedQuad(tex, x, y, (float)tw, (float)th, r, g, b, a);

    glDeleteTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, oldTexture);

}

bool MU_MeasureTextEx(int fontSlot, const char* text, int* outW, int* outH)
{
    if (outW)
        *outW = 0;

    if (outH)
        *outH = 0;

    if (!text)
        return false;

    TTF_Font* font = MU_GetFontSafe(fontSlot);
    if (!font)
        return false;

    int w = 0;
    int h = 0;

    if (TTF_SizeUTF8(font, text, &w, &h) != 0)
        return false;

    if (outW)
        *outW = w;

    if (outH)
        *outH = h;

    return true;
}

bool MU_MeasureText(const char* text, int* outW, int* outH)
{
    return MU_MeasureTextEx(MU_FONT_NORMAL, text, outW, outH);
}

MU_TextSize MU_GetTextSizeEx(int fontSlot, const char* text)
{
    MU_TextSize s;
    s.w = 0;
    s.h = 0;

    MU_MeasureTextEx(fontSlot, text, &s.w, &s.h);
    return s;
}

MU_TextSize MU_GetTextSize(const char* text)
{
    return MU_GetTextSizeEx(MU_FONT_NORMAL, text);
}

int MU_GetTextWidthEx(int fontSlot, const char* text)
{
    int w = 0;
    MU_MeasureTextEx(fontSlot, text, &w, nullptr);
    return w;
}

int MU_GetTextHeightEx(int fontSlot, const char* text)
{
    int h = 0;
    MU_MeasureTextEx(fontSlot, text, nullptr, &h);
    return h;
}

int MU_GetTextWidth(const char* text)
{
    return MU_GetTextWidthEx(MU_FONT_NORMAL, text);
}

int MU_GetTextHeight(const char* text)
{
    return MU_GetTextHeightEx(MU_FONT_NORMAL, text);
}
