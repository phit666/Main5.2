//
// Created by preye on 5/10/2026.
//

#ifndef MU_MOBILE_WT_H
#define MU_MOBILE_WT_H
#ifndef _WIN32
#ifndef LOWORD
#define LOWORD(l) ((unsigned short)((unsigned long)(l) & 0xffff))
#endif

#ifndef HIWORD
#define HIWORD(l) ((unsigned short)((unsigned long)(l) >> 16))
#endif


#include <assert.h>
// Basic replacement using standard C assert
#define _ASSERT(expr) assert(expr)
#ifdef __ANDROID__
#include <android/log.h>
// Advanced version that logs specifically to Android Logcat
#define _ASSERTE(expr) \
    ((expr) ? (void)0 : __android_log_assert(#expr, "NativeAssert", "Assertion failed: %s", #expr))
#endif
#ifndef UCHAR
#define UCHAR unsigned char
#endif
#include <string.h>

#ifndef lstrlen
#define lstrlen(s) (int)strlen(s)
#endif

#ifndef lstrlenW
#define lstrlenW(s) (int)wcslen(s)
#endif

inline nk_color RGB(int r, int g, int b){
    return nk_rgb(r,g,b);
}

inline void SetBkColor(HDC hdc, nk_color color) {
     g_nk_ctx->style.window.fixed_background = nk_style_item_color(color);
}

inline void SetTextColor(HDC hdc, nk_color color) {
     g_nk_ctx->style.text.color = color;
}

#define IME_CMODE_ALPHANUMERIC NULL
#define IME_SMODE_NONE NULL



// Simple NDK wrapper for MultiByteToWideChar
int MultiByteToWideChar(
        uint32_t CodePage,    // Ignored or used to setlocale()
        uint32_t dwFlags,     // Typically ignored
        const char* lpMBStr,
        int cbMB,
        wchar_t* lpWCStr,
        int cchWC)
{
    if (cbMB == 0) return 0;

    // If cchWC is 0, return the required size
    if (cchWC == 0) {
        return mbstowcs(NULL, lpMBStr, 0);
    }

    // Perform the actual conversion
    size_t result = mbstowcs(lpWCStr, lpMBStr, cchWC);

    if (result == (size_t)-1) return 0; // Conversion failed
    return (int)result;
}

int WideCharToMultiByte(
        uint32_t CodePage,       // Ignored (Android defaults to UTF-8)
        uint32_t dwFlags,        // Ignored
        const wchar_t* lpWCStr,
        int cchWC,               // Number of chars in input (-1 for null-terminated)
        char* lpMBStr,
        int cbMB,                // Size of output buffer
        const char* lpDefChar,   // Ignored
        bool* lpUsedDefChar)     // Ignored
{
    if (lpWCStr == nullptr) return 0;

    // If cbMB is 0, return the required buffer size
    if (cbMB == 0) {
        return (int)wcstombs(NULL, lpWCStr, 0);
    }

    size_t result = wcstombs(lpMBStr, lpWCStr, cbMB);

    if (result == (size_t)-1) return 0; // Conversion error
    return (int)result;
}

BOOL TextOutW(HDC hdc, int nXStart, int nYStart, LPCWSTR lpString, int cbString){
    if (nk_begin(g_nk_ctx, "Canvas", nk_rect(0, 0, WindowWidth, WindowHeight), NK_WINDOW_NO_SCROLLBAR)) {
        struct nk_command_buffer *canvas = nk_window_get_canvas(g_nk_ctx);
        //struct nk_user_font *font = g_nk_ctx->style.font;
        struct nk_rect text_rect = nk_rect(nXStart, nYStart, 200, 20);
        nk_draw_text(canvas, text_rect, "Hello", 5, g_nk_ctx->style.font,
                     nk_rgb(255,255,255), nk_rgba(0,0,0,0));
    }
    nk_end(g_nk_ctx);
    return 1;
}

bool GetTextExtentPoint32W(HDC hdc, LPCWSTR text, int len, SIZE *lpsz) {
    if (!g_nk_ctx || !text || !lpsz) return false;

    const struct nk_user_font *font = g_nk_ctx->style.font; // Currently selected font

    // Calculate width using Nuklear's internal font callback
    lpsz->cx = (long)font->width(font->userdata, font->height, reinterpret_cast<const char *>(text), len);

    // Height is constant for the font in Nuklear
    lpsz->cy = (long)font->height;

    return true;
}

#include <algorithm>

// Android NDK replacement for Windows IntersectRect
bool IntersectRect(RECT* lprcDst, const RECT* lprcSrc1, const RECT* lprcSrc2) {
    if (!lprcDst || !lprcSrc1 || !lprcSrc2) return false;

    // Convert Windows RECT (L,T,R,B) to SDL_Rect (X,Y,W,H)
    SDL_Rect s1 = { (int)lprcSrc1->left, (int)lprcSrc1->top,
                    (int)(lprcSrc1->right - lprcSrc1->left),
                    (int)(lprcSrc1->bottom - lprcSrc1->top) };

    SDL_Rect s2 = { (int)lprcSrc2->left, (int)lprcSrc2->top,
                    (int)(lprcSrc2->right - lprcSrc2->left),
                    (int)(lprcSrc2->bottom - lprcSrc2->top) };

    SDL_Rect res;

    if (SDL_IntersectRect(&s1, &s2, &res)) {
        // Convert back to Windows RECT format
        lprcDst->left   = res.x;
        lprcDst->top    = res.y;
        lprcDst->right  = res.x + res.w;
        lprcDst->bottom = res.y + res.h;
        return true;
    }

    // Windows behavior: clear the rect if no intersection
    lprcDst->left = lprcDst->top = lprcDst->right = lprcDst->bottom = 0;
    return false;
}

uintptr_t MU_SetTimer(void* hWnd, uintptr_t nIDEvent, unsigned int uElapse, void* lpTimerFunc);
bool MU_KillTimer(void* hWnd, uintptr_t nIDEvent);

#define SetTimer MU_SetTimer
#define KillTimer MU_KillTimer

#include <stdio.h>
#include <errno.h>

/**
 * Android NDK wrapper for Microsoft's _ultoa_s
 */
inline int _ultoa_s(unsigned long value, char* buffer, int radix) {

    if (buffer == nullptr) return EINVAL;

    size_t size = strlen(buffer);

    if (size == 0) return EINVAL;

    int result = 0;
    if (radix == 10) {
        result = snprintf(buffer, size, "%lu", value);
    } else if (radix == 16) {
        result = snprintf(buffer, size, "%lx", value);
    } else if (radix == 8) {
        result = snprintf(buffer, size, "%lo", value);
    } else {
        // radix 2 or others aren't natively supported by snprintf
        // You would need a custom loop for binary
        return EINVAL;
    }

    // snprintf returns the number of characters that WOULD have been written.
    // If result >= size, the output was truncated.
    if (result < 0 || (size_t)result >= size) {
        buffer[0] = '\0';
        return ERANGE;
    }

    return 0; // Success
}


#ifndef strnicmp
#define strnicmp strncasecmp
#endif


#endif
#endif //MU_MOBILE_WT_H
