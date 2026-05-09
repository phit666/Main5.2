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
#endif IME_SMODE_NONE NULL
#endif //MU_MOBILE_WT_H
