// mu_win_compat.h
#pragma once

#ifndef _WIN32

#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#include <SDL.h>
#include "mu_sdl.h"


#include <algorithm>

#ifndef min
#define min(a,b) ((a) < (b) ? (a) : (b))
#endif

#ifndef max
#define max(a,b) ((a) > (b) ? (a) : (b))
#endif

// ======================================================
// Basic Win32 types
// ======================================================

typedef int                 BOOL;
typedef uint8_t             BYTE;
typedef uint8_t*             PBYTE;
typedef uint16_t            WORD;
typedef unsigned long            DWORD;
typedef long             LONG;
typedef unsigned long            ULONG;
//typedef uint64_t            QWORD;
typedef uint64_t            ULONGLONG;
typedef char CHAR;
typedef int INT;
typedef void VOID;
typedef unsigned long* LPDWORD;
typedef void* LPOVERLAPPED;
typedef void* CONTEXT;

typedef unsigned int        UINT;
typedef uintptr_t           UINT_PTR;
typedef intptr_t            LONG_PTR;
typedef uintptr_t           WPARAM;
typedef intptr_t            LPARAM;
typedef intptr_t            LRESULT;
typedef float   FLOAT;
typedef const char*         LPCSTR;
typedef char*               LPSTR;
typedef const wchar_t*      LPCWSTR;
typedef wchar_t*            LPWSTR;
typedef const void*         LPCVOID;
typedef void*               LPVOID;
typedef void*               PVOID;
typedef char                TCHAR;
typedef const TCHAR*        LPCTSTR;
typedef TCHAR*              LPTSTR;
typedef void* PVOID;
typedef const void* PCVOID;
typedef void*               HANDLE;
typedef void*               HWND;
typedef void* HHOOK;
typedef void*               HINSTANCE;
typedef void*               HMODULE;
typedef void*               HDC;
typedef void*               HGLRC;
typedef void*               HBITMAP;
typedef void*               HFONT;
typedef void*               HICON;
typedef void*               HCURSOR;
typedef void*               HMENU;
typedef void*               HGDIOBJ;
typedef void*               HBRUSH;
typedef void*               HPEN;
typedef void*               HFILE;
typedef short               SHORT;
typedef int64_t             __int64;

typedef unsigned int        SOCKET;
typedef LRESULT (*WNDPROC)(HWND, UINT, WPARAM, LPARAM);

#ifdef __ANDROID__
#include <android/log.h>

#ifndef MB_OK
#define MB_OK 0x00000000
#endif

#ifndef MB_ICONERROR
#define MB_ICONERROR 0x00000010
#endif

#ifndef MB_ICONWARNING
#define MB_ICONWARNING 0x00000030
#endif

#ifndef MB_ICONINFORMATION
#define MB_ICONINFORMATION 0x00000040
#endif

inline int MessageBox(HWND, const char* text, const char* caption, unsigned int)
{
    __android_log_print(ANDROID_LOG_INFO, "MU", "[MessageBox] %s : %s",
                        caption ? caption : "",
                        text ? text : "");
    return 0;
}
#endif

#ifndef GetCurrentThreadId
inline DWORD GetCurrentThreadId()
{
    return 0;
}
#endif

#ifndef CALLBACK
#define CALLBACK
#endif

typedef void* HHOOK;
typedef LRESULT (CALLBACK *HOOKPROC)(int code, WPARAM wParam, LPARAM lParam);

#ifndef WH_CBT
#define WH_CBT 5
#endif

#ifndef HCBT_MOVESIZE
#define HCBT_MOVESIZE 0
#endif

#ifndef HCBT_MINMAX
#define HCBT_MINMAX 1
#endif

#ifndef HCBT_QS
#define HCBT_QS 2
#endif

#ifndef HCBT_CREATEWND
#define HCBT_CREATEWND 3
#endif

#ifndef HCBT_DESTROYWND
#define HCBT_DESTROYWND 4
#endif

#ifndef HCBT_ACTIVATE
#define HCBT_ACTIVATE 5
#endif

#ifndef HCBT_CLICKSKIPPED
#define HCBT_CLICKSKIPPED 6
#endif

#ifndef HCBT_KEYSKIPPED
#define HCBT_KEYSKIPPED 7
#endif

#ifndef HCBT_SYSCOMMAND
#define HCBT_SYSCOMMAND 8
#endif

#ifndef HCBT_SETFOCUS
#define HCBT_SETFOCUS 9
#endif

#define TRUE 1
#define FALSE 0

inline HHOOK SetWindowsHookExA(int, HOOKPROC, HINSTANCE, DWORD)
{
    return nullptr;
}

inline BOOL UnhookWindowsHookEx(HHOOK)
{
    return TRUE;
}

inline LRESULT CallNextHookEx(HHOOK, int, WPARAM, LPARAM)
{
    return 0;
}

#define SetWindowsHookEx SetWindowsHookExA
//#endif

#define WH_CBT 5

#ifndef GetDesktopWindow
inline HWND GetDesktopWindow()
{
    return nullptr;
}
#endif

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

#ifndef NULL
#define NULL 0
#endif

#ifndef INVALID_SOCKET
#define INVALID_SOCKET ((SOCKET)(~0))
#endif

#ifndef SOCKET_ERROR
#define SOCKET_ERROR (-1)
#endif

#ifndef CALLBACK
#define CALLBACK
#endif

#ifndef WINAPI
#define WINAPI
#endif

#ifndef APIENTRY
#define APIENTRY
#endif

#ifndef PASCAL
#define PASCAL
#endif

#ifndef FAR
#define FAR
#endif

#ifndef NEAR
#define NEAR
#endif

#ifndef CONST
#define CONST const
#endif

#ifndef MAX_PATH
#define MAX_PATH 260
#endif

#ifndef S_OK
#define S_OK ((HRESULT)0)
#endif

#ifndef E_FAIL
#define E_FAIL ((HRESULT)0x80004005L)
#endif

#ifndef __forceinline
#define __forceinline inline __attribute__((always_inline))
#endif

#ifndef __inline
#define __inline inline
#endif

#ifndef __int64
#define __int64 long long
#endif

#ifndef ZeroMemory
#define ZeroMemory(Destination, Length) memset((Destination), 0, (Length))
#endif

#ifndef RtlSecureZeroMemory
#define RtlSecureZeroMemory(Destination, Length) memset((Destination), 0, (Length))
#endif

#define CP_UTF8 65001

typedef LONG HRESULT;

// ======================================================
// Structs
// ======================================================

struct POINT
{
    LONG x;
    LONG y;
};

struct SIZE
{
    LONG cx;
    LONG cy;
};

struct RECT
{
    LONG left;
    LONG top;
    LONG right;
    LONG bottom;
};

inline void OffsetRect(RECT* prc, int dx, int dy) {
    if (!prc) return;
    prc->left += dx;
    prc->right += dx;
    prc->top += dy;
    prc->bottom += dy;
}

typedef POINT* LPPOINT;
typedef const POINT* LPCPOINT;

typedef SIZE* LPSIZE;
typedef const SIZE* LPCSIZE;

typedef RECT* LPRECT;
typedef const RECT* LPCRECT;

struct MSG
{
    HWND hwnd;
    UINT message;
    WPARAM wParam;
    LPARAM lParam;
    DWORD time;
    POINT pt;
};

// ======================================================
// Windows messages
// ======================================================

#define WM_NULL             0x0000
#define WM_CREATE           0x0001
#define WM_DESTROY          0x0002
#define WM_MOVE             0x0003
#define WM_SIZE             0x0005
#define WM_ACTIVATE         0x0006
#define WM_SETFOCUS         0x0007
#define WM_KILLFOCUS        0x0008
#define WM_ENABLE           0x000A
#define WM_SETREDRAW        0x000B
#define WM_SETTEXT          0x000C
#define WM_GETTEXT          0x000D
#define WM_GETTEXTLENGTH    0x000E
#define WM_PAINT            0x000F
#define WM_CLOSE            0x0010
#define WM_QUIT             0x0012
#define WM_ERASEBKGND       0x0014

#define WM_KEYDOWN          0x0100
#define WM_KEYUP            0x0101
#define WM_CHAR             0x0102

#define WM_MOUSEMOVE        0x0200
#define WM_LBUTTONDOWN      0x0201
#define WM_LBUTTONUP        0x0202
#define WM_LBUTTONDBLCLK    0x0203
#define WM_RBUTTONDOWN      0x0204
#define WM_RBUTTONUP        0x0205
#define WM_RBUTTONDBLCLK    0x0206
#define WM_MBUTTONDOWN      0x0207
#define WM_MBUTTONUP        0x0208
#define WM_MOUSEWHEEL       0x020A

// ======================================================
// ShowWindow flags
// ======================================================

#define SW_HIDE             0
#define SW_SHOWNORMAL       1
#define SW_SHOW             5

// ======================================================
// Virtual keys
// ======================================================

#define VK_BACK      0x08
#define VK_TAB       0x09
#define VK_RETURN    0x0D
#define VK_SHIFT     0x10
#define VK_CONTROL   0x11
#define VK_MENU      0x12
#define VK_PAUSE     0x13
#define VK_ESCAPE    0x1B
#define VK_SPACE     0x20
#define VK_PRIOR     0x21
#define VK_NEXT      0x22
#define VK_END       0x23
#define VK_HOME      0x24
#define VK_LEFT      0x25
#define VK_UP        0x26
#define VK_RIGHT     0x27
#define VK_DOWN      0x28
#define VK_INSERT    0x2D
#define VK_DELETE    0x2E

#define VK_LBUTTON   0x01
#define VK_RBUTTON   0x02
#define VK_MBUTTON   0x04

#define VK_0         0x30
#define VK_1         0x31
#define VK_2         0x32
#define VK_3         0x33
#define VK_4         0x34
#define VK_5         0x35
#define VK_6         0x36
#define VK_7         0x37
#define VK_8         0x38
#define VK_9         0x39

#define VK_A         0x41
#define VK_B         0x42
#define VK_C         0x43
#define VK_D         0x44
#define VK_E         0x45
#define VK_F         0x46
#define VK_G         0x47
#define VK_H         0x48
#define VK_I         0x49
#define VK_J         0x4A
#define VK_K         0x4B
#define VK_L         0x4C
#define VK_M         0x4D
#define VK_N         0x4E
#define VK_O         0x4F
#define VK_P         0x50
#define VK_Q         0x51
#define VK_R         0x52
#define VK_S         0x53
#define VK_T         0x54
#define VK_U         0x55
#define VK_V         0x56
#define VK_W         0x57
#define VK_X         0x58
#define VK_Y         0x59
#define VK_Z         0x5A

#define GET_X_LPARAM(lp) ((int)(short)((lp) & 0xffff))
#define GET_Y_LPARAM(lp) ((int)(short)(((lp) >> 16) & 0xffff))

// ======================================================
// RECT helpers
// ======================================================

inline BOOL PtInRect(const RECT* rc, POINT pt)
{
    if (!rc)
        return FALSE;

    return (pt.x >= rc->left &&
            pt.x <  rc->right &&
            pt.y >= rc->top &&
            pt.y <  rc->bottom) ? TRUE : FALSE;
}

inline void SetRect(RECT* rc, LONG left, LONG top, LONG right, LONG bottom)
{
    if (!rc)
        return;

    rc->left   = left;
    rc->top    = top;
    rc->right  = right;
    rc->bottom = bottom;
}

inline BOOL IsRectEmpty(const RECT* rc)
{
    if (!rc)
        return TRUE;

    return (rc->right <= rc->left || rc->bottom <= rc->top) ? TRUE : FALSE;
}

// ======================================================
// Timing
// ======================================================

inline void Sleep(DWORD ms)
{
    SDL_Delay(ms);
}

inline DWORD GetTickCount()
{
    return SDL_GetTicks();
}

inline ULONGLONG GetTickCount64()
{
    return (ULONGLONG)SDL_GetTicks64();
}

// ======================================================
// Dummy window/message API
// ======================================================

inline LRESULT SendMessage(HWND, UINT, WPARAM, LPARAM)
{
    return 0;
}

inline BOOL PostMessage(HWND, UINT, WPARAM, LPARAM)
{
    return TRUE;
}

inline BOOL PeekMessage(MSG*, HWND, UINT, UINT, UINT)
{
    return FALSE;
}

inline BOOL GetMessage(MSG*, HWND, UINT, UINT)
{
    return FALSE;
}

inline BOOL TranslateMessage(const MSG*)
{
    return TRUE;
}

inline LRESULT DispatchMessage(const MSG*)
{
    return 0;
}

inline HWND CreateWindowA(...)
{
    return nullptr;
}

inline HWND CreateWindowExA(...)
{
    return nullptr;
}

inline BOOL DestroyWindow(HWND)
{
    return TRUE;
}

inline BOOL ShowWindow(HWND, int)
{
    return TRUE;
}

inline BOOL SetForegroundWindow(HWND)
{
    return TRUE;
}

inline HWND GetFocus()
{
    return nullptr;
}

inline HWND SetFocus(HWND)
{
    return nullptr;
}

inline LONG_PTR SetWindowLongPtrA(HWND, int, LONG_PTR)
{
    return 0;
}

inline LONG_PTR GetWindowLongPtrA(HWND, int)
{
    return 0;
}

inline LONG SetWindowLongA(HWND, int, LONG)
{
    return 0;
}

inline LONG GetWindowLongA(HWND, int)
{
    return 0;
}

inline char* itoa(int value, char* str, int base)
{
    if (base == 10)
    {
        sprintf(str, "%d", value);
    }
    else if (base == 16)
    {
        sprintf(str, "%x", value);
    }
    else
    {
        // simple fallback
        sprintf(str, "%d", value);
    }
    return str;
}

#define SetWindowLongPtr SetWindowLongPtrA
#define GetWindowLongPtr GetWindowLongPtrA
#define SetWindowLong    SetWindowLongA
#define GetWindowLong    GetWindowLongA

// ======================================================
// Cursor/input dummy API
// ======================================================

inline BOOL GetCursorPos(POINT*)
{
    return FALSE;
}

inline BOOL ScreenToClient(HWND, POINT*)
{
    return FALSE;
}

inline short GetAsyncKeyState(int)
{
    return 0;
}

inline short GetKeyState(int)
{
    return 0;
}

inline BOOL GetKeyboardState(BYTE*)
{
    return FALSE;
}

inline int ShowCursor(BOOL)
{
    return 0;
}

inline HCURSOR SetCursor(HCURSOR)
{
    return nullptr;
}

inline HCURSOR LoadCursorA(HINSTANCE, LPCSTR)
{
    return nullptr;
}

// ======================================================
// GDI dummy API
// ======================================================

inline HDC GetDC(HWND)
{
    return nullptr;
}

inline int ReleaseDC(HWND, HDC)
{
    return 1;
}

inline HDC CreateCompatibleDC(HDC)
{
    return nullptr;
}

inline HBITMAP CreateCompatibleBitmap(HDC, int, int)
{
    return nullptr;
}

inline HGDIOBJ SelectObject(HDC, HGDIOBJ)
{
    return nullptr;
}

inline BOOL DeleteObject(HGDIOBJ)
{
    return TRUE;
}

inline BOOL DeleteDC(HDC)
{
    return TRUE;
}

inline BOOL BitBlt(...)
{
    return FALSE;
}

inline BOOL StretchBlt(...)
{
    return FALSE;
}

inline int SetBkMode(HDC, int)
{
    return 0;
}

inline DWORD SetTextColor(HDC, DWORD)
{
    return 0;
}

inline DWORD SetBkColor(HDC, DWORD)
{
    return 0;
}

inline BOOL TextOutA(HDC, int, int, LPCSTR, int)
{
    return TRUE;
}

inline BOOL GetTextExtentPointA(HDC, LPCSTR, int, LPSIZE lpSize)
{
    if (lpSize)
    {
        lpSize->cx = 0;
        lpSize->cy = 0;
    }

    return TRUE;
}

inline BOOL GetTextExtentPoint32A(HDC, LPCSTR, int, LPSIZE lpSize)
{
    return GetTextExtentPointA(nullptr, nullptr, 0, lpSize);
}

inline HFONT CreateFontA(...)
{
    return nullptr;
}

inline HFONT CreateFontIndirectA(...)
{
    return nullptr;
}

// ======================================================
// File/path dummy API
// ======================================================

#define INVALID_HANDLE_VALUE ((HANDLE)(intptr_t)-1)

inline HANDLE CreateFileA(...)
{
    return INVALID_HANDLE_VALUE;
}

inline BOOL ReadFile(...)
{
    return FALSE;
}

inline DWORD GetModuleFileNameA(HMODULE, LPSTR buffer, DWORD size)
{
    if (buffer && size > 0)
        buffer[0] = '\0';

    return 0;
}

inline DWORD GetCurrentDirectoryA(DWORD size, LPSTR buffer)
{
    if (buffer && size > 0)
        buffer[0] = '\0';

    return 0;
}

inline BOOL SetCurrentDirectoryA(LPCSTR)
{
    return FALSE;
}

// ======================================================
// Shell / registry dummy API
// ======================================================

inline void* ShellExecuteA(...)
{
    return nullptr;
}

inline LONG RegOpenKeyExA(...)
{
    return -1;
}

inline LONG RegSetValueExA(...)
{
    return -1;
}

inline LONG RegQueryValueExA(...)
{
    return -1;
}

inline LONG RegCloseKey(...)
{
    return 0;
}

// ======================================================
// Thread / sync dummy API
// ======================================================

typedef unsigned int (__stdcall *PTHREAD_START)(void*);

#ifndef __stdcall
#define __stdcall
#endif

inline uintptr_t _beginthreadex(
        void*,
        unsigned,
        unsigned int (__stdcall *)(void*),
        void*,
        unsigned,
        unsigned*)
{
    return 0;
}

inline HANDLE CreateThread(...)
{
    return nullptr;
}

inline DWORD WaitForSingleObject(HANDLE, DWORD)
{
    return 0;
}

inline void InitializeCriticalSection(void*)
{
}

inline void EnterCriticalSection(void*)
{
}

inline void LeaveCriticalSection(void*)
{
}

inline void DeleteCriticalSection(void*)
{
}

// ======================================================
// Winsock compatibility
// ======================================================

#define SD_BOTH 2
#define WSAEWOULDBLOCK 10035

inline int WSAStartup(WORD, void*)
{
    return 0;
}

inline int WSACleanup()
{
    return 0;
}

inline int WSAGetLastError()
{
    return 0;
}

inline int closesocket(SOCKET)
{
    return 0;
}

// ======================================================
// TCHAR compatibility
// ======================================================

#define _T(x) x
#define TEXT(x) x

#define strcpy_s(dst, size, src)        strncpy((dst), (src), (size))
#define strcat_s(dst, size, src)        strncat((dst), (src), (size) - strlen(dst) - 1)
#define sprintf_s                      snprintf
#define _stricmp                       strcasecmp
#define _strnicmp                      strncasecmp

// OTHER
#define IN
#define OUT

#ifndef HWND_TOPMOST
#define HWND_TOPMOST ((HWND)-1)
#endif

#ifndef HWND_NOTOPMOST
#define HWND_NOTOPMOST ((HWND)-2)
#endif

#ifndef HWND_TOP
#define HWND_TOP ((HWND)0)
#endif

#ifndef HWND_BOTTOM
#define HWND_BOTTOM ((HWND)1)
#endif

#ifndef SWP_NOSIZE
#define SWP_NOSIZE         0x0001
#endif

#ifndef SWP_NOMOVE
#define SWP_NOMOVE         0x0002
#endif

#ifndef SWP_NOZORDER
#define SWP_NOZORDER       0x0004
#endif

#ifndef SWP_NOREDRAW
#define SWP_NOREDRAW       0x0008
#endif

#ifndef SWP_NOACTIVATE
#define SWP_NOACTIVATE     0x0010
#endif

#ifndef SWP_FRAMECHANGED
#define SWP_FRAMECHANGED   0x0020
#endif

#ifndef SWP_SHOWWINDOW
#define SWP_SHOWWINDOW     0x0040
#endif

#ifndef SWP_HIDEWINDOW
#define SWP_HIDEWINDOW     0x0080
#endif

#ifndef SWP_NOCOPYBITS
#define SWP_NOCOPYBITS     0x0100
#endif

#ifndef SWP_NOOWNERZORDER
#define SWP_NOOWNERZORDER  0x0200
#endif

#ifndef SWP_NOSENDCHANGING
#define SWP_NOSENDCHANGING 0x0400
#endif

#ifndef FW_NORMAL
#define FW_NORMAL 400
#endif

#ifndef FW_BOLD
#define FW_BOLD 700
#endif

// Font / GDI constants
#ifndef DEFAULT_CHARSET
#define DEFAULT_CHARSET 1
#endif

#ifndef OUT_DEFAULT_PRECIS
#define OUT_DEFAULT_PRECIS 0
#endif

#ifndef CLIP_DEFAULT_PRECIS
#define CLIP_DEFAULT_PRECIS 0
#endif

#ifndef DEFAULT_QUALITY
#define DEFAULT_QUALITY 0
#endif

#ifndef NONANTIALIASED_QUALITY
#define NONANTIALIASED_QUALITY 3
#endif

#ifndef ANTIALIASED_QUALITY
#define ANTIALIASED_QUALITY 4
#endif

#ifndef DEFAULT_PITCH
#define DEFAULT_PITCH 0
#endif

#ifndef FIXED_PITCH
#define FIXED_PITCH 1
#endif

#ifndef VARIABLE_PITCH
#define VARIABLE_PITCH 2
#endif

#ifndef FF_DONTCARE
#define FF_DONTCARE 0
#endif

#ifndef GL_ALPHA_TEST
#define GL_ALPHA_TEST 0
#endif

#ifndef wsprintf
#define wsprintf sprintf
#endif

#ifndef MAKEWORD
#define MAKEWORD(a, b) ((unsigned short)(((unsigned char)(a)) | ((unsigned short)((unsigned char)(b)) << 8)))
#endif

#ifdef __ANDROID__
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>

#define MU_ATTR_POS    0
#define MU_ATTR_UV     1
#define MU_ATTR_COLOR  2

// ===== Legacy GL constants (fixed pipeline) =====
#ifndef GL_CURRENT_COLOR
#define GL_CURRENT_COLOR 0x0B00
#endif

#ifndef GL_VERTEX_ARRAY
#define GL_VERTEX_ARRAY 0x8074
#endif

#ifndef GL_TEXTURE_COORD_ARRAY
#define GL_TEXTURE_COORD_ARRAY 0x8078
#endif

#ifndef GL_COLOR_ARRAY
#define GL_COLOR_ARRAY 0x8076
#endif

#ifndef GL_NORMAL_ARRAY
#define GL_NORMAL_ARRAY 0x8075
#endif

#ifndef GL_ALPHA_TEST
#define GL_ALPHA_TEST 0x0BC0
#endif

#ifndef GL_LIGHTING
#define GL_LIGHTING 0x0B50
#endif

#ifndef GL_LIGHT0
#define GL_LIGHT0 0x4000
#endif

#ifndef GL_FOG
#define GL_FOG 0x0B60
#endif

#ifndef GL_CLIP_PLANE0
#define GL_CLIP_PLANE0 0x3000
#endif

#ifndef GL_MODELVIEW
#define GL_MODELVIEW 0x1700
#endif

#ifndef GL_PROJECTION
#define GL_PROJECTION 0x1701
#endif

#ifndef GL_TEXTURE
#define GL_TEXTURE 0x1702
#endif

inline void glEnableClientState(GLenum array)
{
    if (array == GL_VERTEX_ARRAY)
        glEnableVertexAttribArray(MU_ATTR_POS);
    else if (array == GL_TEXTURE_COORD_ARRAY)
        glEnableVertexAttribArray(MU_ATTR_UV);
    else if (array == GL_COLOR_ARRAY)
        glEnableVertexAttribArray(MU_ATTR_COLOR);
}

inline void glDisableClientState(GLenum array)
{
    if (array == GL_VERTEX_ARRAY)
        glDisableVertexAttribArray(MU_ATTR_POS);
    else if (array == GL_TEXTURE_COORD_ARRAY)
        glDisableVertexAttribArray(MU_ATTR_UV);
    else if (array == GL_COLOR_ARRAY)
        glDisableVertexAttribArray(MU_ATTR_COLOR);
}

inline void glVertexPointer(GLint size, GLenum type, GLsizei stride, const void* pointer)
{
    glVertexAttribPointer(
            MU_ATTR_POS,
            size,
            type,
            GL_FALSE,
            stride,
            pointer
    );
}

inline void glTexCoordPointer(GLint size, GLenum type, GLsizei stride, const void* pointer)
{
    glVertexAttribPointer(
            MU_ATTR_UV,
            size,
            type,
            GL_FALSE,
            stride,
            pointer
    );
}

inline void glColorPointer(GLint size, GLenum type, GLsizei stride, const void* pointer)
{
    glVertexAttribPointer(
            MU_ATTR_COLOR,
            size,
            type,
            GL_TRUE,   // important: 0-255 color becomes 0.0-1.0
            stride,
            pointer
    );
}
#endif

inline HFONT CreateFont(...)
{
    return nullptr;
}

inline BOOL SetWindowPos(...)
{
    return TRUE;
}

inline BOOL GetWindowRect(HWND, RECT* rc)
{
    if (!rc)
        return FALSE;

    int w = 0, h = 0;
    SDL_GetWindowSize(SDL_GL_GetCurrentWindow(), &w, &h);

    rc->left = 0;
    rc->top = 0;
    rc->right = w;
    rc->bottom = h;

    return TRUE;
}

#ifndef timeGetTime
inline unsigned int timeGetTime()
{
    return SDL_GetTicks();
}
#endif

typedef struct _SYSTEMTIME
{
    WORD wYear;
    WORD wMonth;
    WORD wDayOfWeek;
    WORD wDay;
    WORD wHour;
    WORD wMinute;
    WORD wSecond;
    WORD wMilliseconds;
} SYSTEMTIME;

#ifndef IsBadReadPtr
inline bool IsBadReadPtr(const void* ptr, size_t size)
{
    return ptr == nullptr;
}
#endif

typedef void* HKEY;
typedef BYTE* LPBYTE;
typedef const BYTE* LPCBYTE;
typedef uint32_t* ULONG_PTR;

#ifndef ERROR_SUCCESS
#define ERROR_SUCCESS 0L
#endif

#ifndef HKEY_CLASSES_ROOT
#define HKEY_CLASSES_ROOT ((HKEY)(intptr_t)0x80000000)
#endif
#ifndef HKEY_CURRENT_CONFIG
#define HKEY_CURRENT_CONFIG ((HKEY)(intptr_t)0x80000005)
#endif
#ifndef HKEY_CURRENT_USER
#define HKEY_CURRENT_USER ((HKEY)(intptr_t)0x80000001)
#endif
#ifndef HKEY_LOCAL_MACHINE
#define HKEY_LOCAL_MACHINE ((HKEY)(intptr_t)0x80000002)
#endif
#ifndef HKEY_USERS
#define HKEY_USERS ((HKEY)(intptr_t)0x80000003)
#endif

#ifndef REG_DWORD
#define REG_DWORD 4
#endif
#ifndef REG_SZ
#define REG_SZ 1
#endif
#ifndef REG_EXPAND_SZ
#define REG_EXPAND_SZ 2
#endif
#ifndef REG_OPTION_NON_VOLATILE
#define REG_OPTION_NON_VOLATILE 0
#endif
#ifndef KEY_ALL_ACCESS
#define KEY_ALL_ACCESS 0
#endif
#ifndef KEY_QUERY_VALUE
#define KEY_QUERY_VALUE 0
#endif

inline LONG RegCreateKeyExA(HKEY, LPCSTR, DWORD, LPSTR, DWORD, DWORD, void*, HKEY* phkResult, DWORD* lpdwDisposition)
{
    if (phkResult) *phkResult = nullptr;
    if (lpdwDisposition) *lpdwDisposition = 0;
    return -1;
}

inline LONG RegOpenKeyExA(HKEY, LPCSTR, DWORD, DWORD, HKEY* phkResult)
{
    if (phkResult) *phkResult = nullptr;
    return -1;
}

inline LONG RegQueryValueExA(HKEY, LPCSTR, DWORD*, DWORD*, LPBYTE, DWORD*)
{
    return -1;
}

inline LONG RegSetValueExA(HKEY, LPCSTR, DWORD, DWORD, const BYTE*, DWORD)
{
    return -1;
}

inline LONG RegDeleteKeyA(HKEY, LPCSTR)
{
    return -1;
}

inline LONG RegDeleteValueA(HKEY, LPCSTR)
{
    return -1;
}

inline LONG RegCloseKey(HKEY)
{
    return ERROR_SUCCESS;
}

#define RegCreateKeyEx  RegCreateKeyExA
#define RegOpenKeyEx    RegOpenKeyExA
#define RegQueryValueEx RegQueryValueExA
#define RegSetValueEx   RegSetValueExA
#define RegDeleteKey    RegDeleteKeyA
#define RegDeleteValue  RegDeleteValueA

#ifndef _EXCEPTION_POINTERS_DEFINED
#define _EXCEPTION_POINTERS_DEFINED
typedef struct _EXCEPTION_POINTERS
{
    void* ExceptionRecord;
    void* ContextRecord;
} _EXCEPTION_POINTERS;
#endif

// ===== Memory =====
#ifndef MEMORYSTATUS
typedef struct _MEMORYSTATUS
{
    DWORD dwLength;
    DWORD dwMemoryLoad;
    DWORD dwTotalPhys;
    DWORD dwAvailPhys;
    DWORD dwTotalPageFile;
    DWORD dwAvailPageFile;
    DWORD dwTotalVirtual;
    DWORD dwAvailVirtual;
} MEMORYSTATUS, *LPMEMORYSTATUS;
#endif

#ifndef PEXCEPTION_RECORD
typedef void* PEXCEPTION_RECORD;
#endif

#ifndef LPTOP_LEVEL_EXCEPTION_FILTER
typedef LONG (*LPTOP_LEVEL_EXCEPTION_FILTER)(_EXCEPTION_POINTERS*);
#endif

// ===== Version info =====
#ifndef VS_FIXEDFILEINFO
typedef struct tagVS_FIXEDFILEINFO
{
    DWORD dwSignature;
    DWORD dwStrucVersion;
    DWORD dwFileVersionMS;
    DWORD dwFileVersionLS;
    DWORD dwProductVersionMS;
    DWORD dwProductVersionLS;
    DWORD dwFileFlagsMask;
    DWORD dwFileFlags;
    DWORD dwFileOS;
    DWORD dwFileType;
    DWORD dwFileSubtype;
    DWORD dwFileDateMS;
    DWORD dwFileDateLS;
} VS_FIXEDFILEINFO;
#endif

#ifndef StringCchCopy
inline int StringCchCopy(char* dst, size_t dstBytes, const char* src)
{
    if (!dst || dstBytes == 0)
        return -1;

    if (!src)
    {
        dst[0] = '\0';
        return 0;
    }

    strncpy(dst, src, dstBytes - 1);
    dst[dstBytes - 1] = '\0';
    return 0;
}
#endif

#ifndef _tcschr
#define _tcschr strchr
#endif

inline BOOL WriteFile(void* file, const void* buffer, DWORD size, DWORD* written, void*)
{
    size_t w = MU_fwrite(buffer, 1, size, (MU_FILE*)file);

    if (written)
        *written = (DWORD)w;

    return (w == size) ? TRUE : FALSE;
}

inline int CloseHandle(void* h)
{
    if (!h)
        return 0;

    return MU_fclose((MU_FILE*)h);
}

#ifndef _atoi64
#define _atoi64 atoll
#endif


#ifndef HIBYTE
#define HIBYTE(w) ((BYTE)(((WORD)(w) >> 8) & 0xFF))
#endif

#ifndef LOBYTE
#define LOBYTE(w) ((BYTE)((WORD)(w) & 0xFF))
#endif

#ifndef MAKELONG
#define MAKELONG(a, b) ((LONG)(((WORD)((DWORD_PTR)(a) & 0xffff)) | ((DWORD)((WORD)((DWORD_PTR)(b) & 0xffff))) << 16))
#endif

#define stricmp strcasecmp


// Wrapper to mimic the MSVC basic_string::_Copy_s member
inline size_t ndk_copy_s(const std::string& str, char* dest, size_t destSize, size_t count) {
    if (!dest || destSize == 0) return 0;

    // MSVC's _Copy_s copies 'count' characters or the string length,
    // whichever is smaller, provided it fits in destSize.
    size_t lengthToCopy = (std::min)({count, str.length(), destSize});

    size_t copied = str.copy(dest, lengthToCopy);

    // IMPORTANT: _Copy_s does NOT null-terminate.
    // If your code expects a C-string, you might need:
    // if (copied < destSize) dest[copied] = '\0';

    return copied;
}

#define _MAX_PATH   260 // max. length of full pathname
#define _MAX_DRIVE  3   // max. length of drive component
#define _MAX_DIR    256 // max. length of path component
#define _MAX_FNAME  256 // max. length of file name component
#define _MAX_EXT    256 // max. length of extension component

#endif