//
// Created by preye on 5/10/2026.
//
#ifndef MU_MOBILE_WT_H
#define MU_MOBILE_WT_H
#pragma once

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


#ifdef __ANDROID__
#include <android/keycodes.h>

#ifndef VK_LCONTROL
#define VK_LCONTROL AKEYCODE_CTRL_LEFT
#endif

#ifndef VK_RCONTROL
#define VK_RCONTROL AKEYCODE_CTRL_RIGHT
#endif

#ifndef VK_SNAPSHOT
#define VK_SNAPSHOT AKEYCODE_SYSRQ  // System Request / Print Screen
#endif

#endif

#define IME_SMODE_AUTOMATIC NULL

#include <ctype.h>

/**
 * Android NDK wrapper for _strupr
 */
static inline char* _strupr(char* s) {
    if (!s) return nullptr;
    char* p = s;
    while (*p) {
        *p = (char)toupper((unsigned char)*p);
        p++;
    }
    return s;
}

#include <time.h>


// Replacement for GetLocalTime
inline void GetLocalTime(SYSTEMTIME* st) {
    struct timespec ts;
    struct tm t;

    clock_gettime(CLOCK_REALTIME, &ts); // Get current time and nanoseconds
    localtime_r(&ts.tv_sec, &t);        // Convert to local time components

    st->wYear         = t.tm_year + 1900;
    st->wMonth        = t.tm_mon + 1;
    st->wDayOfWeek    = t.tm_wday;
    st->wDay          = t.tm_mday;
    st->wHour         = t.tm_hour;
    st->wMinute       = t.tm_min;
    st->wSecond       = t.tm_sec;
    st->wMilliseconds = ts.tv_nsec / 1000000; // Nanoseconds to milliseconds
}


#include <cwchar>

#ifndef lstrlen
#define lstrlen(s) (int)strlen(s)
#endif

#ifndef lstrlenW
#define lstrlenW(s) (int)wcslen(s)
#endif

#include <time.h>

#ifndef _tzset
#define _tzset tzset
#endif

#include <stdint.h>

// Force 2-byte alignment to match Windows BMP structure padding
#pragma pack(push, 2)

typedef struct {
    uint16_t bfType;      // "BM" magic number
    uint32_t bfSize;      // Size of the file
    uint16_t bfReserved1;
    uint16_t bfReserved2;
    uint32_t bfOffBits;   // Offset to start of pixel data
} BITMAPFILEHEADER;

typedef struct {
    uint32_t biSize;          // Size of this header (40)
    int32_t  biWidth;
    int32_t  biHeight;
    uint16_t biPlanes;
    uint16_t biBitCount;
    uint32_t biCompression;
    uint32_t biSizeImage;
    int32_t  biXPelsPerMeter;
    int32_t  biYPelsPerMeter;
    uint32_t biClrUsed;
    uint32_t biClrImportant;
} BITMAPINFOHEADER;

#pragma pack(pop)


#endif
#endif //MU_MOBILE_WT_H

#include <filesystem>
#include <string>
#include <cstring>

inline void MU_splitpath(
    const char* path,
    char* drive,
    char* dir,
    char* fname,
    char* ext)
{
    namespace fs = std::filesystem;

    if (!path)
        return;

    fs::path p(path);

    // DRIVE
    if (drive)
    {
#ifdef _WIN32
        std::string root = p.root_name().string();
        std::snprintf(drive, _MAX_DRIVE, "%s", root.c_str());
#else
        drive[0] = '\0';
#endif
    }

    // DIRECTORY
    if (dir)
    {
        std::string d = p.parent_path().string();

        // mimic old _splitpath behavior
        if (!d.empty())
        {
            char last = d[d.size() - 1];

            if (last != '/' && last != '\\')
                d += "/";
        }

        std::snprintf(dir, _MAX_DIR, "%s", d.c_str());
    }

    // FILENAME
    if (fname)
    {
        std::string f = p.filename().string();
        std::snprintf(fname, _MAX_FNAME, "%s", f.c_str());
    }

    // EXTENSION
    if (ext)
    {
        std::string e = p.extension().string();
        std::snprintf(ext, _MAX_EXT, "%s", e.c_str());
    }
}

#define FONT_SIZE12 12
#define FONT_SIZE13 13
#define FONT_SIZE14 14
#define FONT_SIZE15 15
#define DEFAULT_FONT_SIZE FONT_SIZE13
#define MAX_FONTS 20


struct _TexScaleMap {
    GLuint TexID;
    float scale;
    bool iscopy;
    bool isscale;
    std::string note;
};

void setfont(int size);
int pushfont(int size);
void popfont();

_TexScaleMap getTexScale(GLuint TexID);
GLuint getScaleTexID(GLuint TexID);
int32_t getScaleNewSize(GLuint TexID, int32_t size);

extern int TextNum;
extern char TextList[30][100];
extern int TextListColor[30];
extern int  TextBold[30];
//extern int g_iItemInfo[12][17];
extern float g_fScreenRate_x;
extern float g_fScreenRate_y;

extern int   ScreenCenterX;
extern int   ScreenCenterY;
extern int   ScreenCenterYFlip;
extern float Distance;

extern int g_WorldViewX;
extern int g_WorldViewY;
extern int g_WorldViewW;
extern int g_WorldViewH;


void BeginWorldOpenGL(int x, int y, int w, int h);
void RestoreWorldPickingViewport();
bool CreateScreenRayGLM(
    int mouseX,
    int mouseY,
    int viewportX,
    int viewportY,
    int viewportW,
    int viewportH,
    glm::vec3& rayStart,
    glm::vec3& rayEnd);

struct PickState
{
    int ScreenCenterX;
    int ScreenCenterY;
    int ScreenCenterYFlip;
    float PerspectiveX;
    float PerspectiveY;
    float CameraMatrix[3][4];
};

inline PickState SavePickState()
{
    PickState s;
    s.ScreenCenterX = ScreenCenterX;
    s.ScreenCenterY = ScreenCenterY;
    s.ScreenCenterYFlip = ScreenCenterYFlip;
    s.PerspectiveX = PerspectiveX;
    s.PerspectiveY = PerspectiveY;
    memcpy(s.CameraMatrix, CameraMatrix, sizeof(CameraMatrix));
    return s;
}

inline void RestorePickState(const PickState& s)
{
    ScreenCenterX = s.ScreenCenterX;
    ScreenCenterY = s.ScreenCenterY;
    ScreenCenterYFlip = s.ScreenCenterYFlip;
    PerspectiveX = s.PerspectiveX;
    PerspectiveY = s.PerspectiveY;
    memcpy(CameraMatrix, s.CameraMatrix, sizeof(CameraMatrix));
}

extern bool  SelectFlag;

struct WorldPickState
{
    bool valid;

    int ScreenCenterX;
    int ScreenCenterY;
    int ScreenCenterYFlip;

    float PerspectiveX;
    float PerspectiveY;

    float CameraMatrix[3][4];
};

extern WorldPickState g_LastWorldPickState;
void SaveWorldPickState();
void RestoreWorldPickState();
extern bool g_PendingTouchMove;
extern int  g_PendingTouchMoveFrames;

#include "UIControls.h"

extern std::vector<CUITextInputBox*> vUITextInputs;
extern bool overlayblocktouch;
extern int g_focusinputresize;
extern std::string g_focusinputwindow;
extern int g_refocusinputresize;

void RenderNuklearSafe(struct nk_context* ctx);

bool SaveQuickLogin(const char* username, const char* password);
bool LoadQuickLogin(char* usernameOut, int usernameOutSize,
    char* passwordOut, int passwordOutSize);

extern GLuint g_meshVBO;

#define ENHANCE_ENCDEC
