#pragma once

// MU client portability shim. Include this instead of <windows.h> on non-Windows builds.
// Goal: compile large legacy code first, then replace subsystems progressively.

#ifdef _WIN32
  #include <windows.h>
  #include <winsock2.h>
  #include <ws2tcpip.h>
#else

#include <stdint.h>
#include <stddef.h>

using BYTE   = uint8_t;
using WORD   = uint16_t;
using DWORD  = uint32_t;
using UINT   = unsigned int;
using BOOL   = int;
using LONG   = long;
using ULONG  = unsigned long;
using INT    = int;
using CHAR   = char;
using WCHAR  = wchar_t;
using HANDLE = void*;
using HINSTANCE = void*;
using HMODULE = void*;
using HWND   = void*;
using HDC    = void*;
using HBRUSH = void*;
using HKEY   = void*;
using HMENU  = void*;
using HICON  = void*;
using HCURSOR= void*;
using HBITMAP= void*;
using WPARAM = uintptr_t;
using LPARAM = intptr_t;
using LRESULT= intptr_t;
using LPVOID = void*;
using LPCVOID= const void*;
using LPSTR  = char*;
using LPCSTR = const char*;
using LPBYTE = BYTE*;
using LPCTSTR= const char*;
using LPTSTR = char*;
using HRESULT= long;

#ifndef TRUE
#define TRUE 1
#define FALSE 0
#endif
#ifndef NULL
#define NULL 0
#endif
#ifndef MAX_PATH
#define MAX_PATH 260
#endif

#define WINAPI
#define CALLBACK
#define PASCAL
#define FAR
#define NEAR
#define __stdcall
#define __cdecl
#ifndef __forceinline
#define __forceinline inline
#endif
#define CONST const

#define S_OK 0
#define E_FAIL 0x80004005L
#define FAILED(hr) (((HRESULT)(hr)) < 0)
#define SUCCEEDED(hr) (((HRESULT)(hr)) >= 0)

#define ERROR_SUCCESS 0L
#define ERROR_FILE_NOT_FOUND 2L
#define ERROR_INVALID_PARAMETER 87L
#define ERROR_MORE_DATA 234L

#define LOWORD(l) ((WORD)((uintptr_t)(l) & 0xffff))
#define HIWORD(l) ((WORD)((uintptr_t)(l) >> 16))
#define LOBYTE(w) ((BYTE)((uintptr_t)(w) & 0xff))
#define HIBYTE(w) ((BYTE)(((uintptr_t)(w) >> 8) & 0xff))
#define MAKELONG(a,b) ((LONG)(((WORD)(a)) | ((DWORD)((WORD)(b))) << 16))
#define MAKELPARAM(l,h) ((LPARAM)(intptr_t)MAKELONG(l,h))
#define MAKEWPARAM(l,h) ((WPARAM)(uintptr_t)MAKELONG(l,h))

#define CopyMemory  memcpy
#define ZeroMemory(p,n) memset((p),0,(n))
#define FillMemory(p,n,v) memset((p),(v),(n))

// Common Win32 messages/constants used by MU client.
#define WM_USER             0x0400
#define WM_NULL             0x0000
#define WM_CREATE           0x0001
#define WM_DESTROY          0x0002
#define WM_MOVE             0x0003
#define WM_SIZE             0x0005
#define WM_ACTIVATE         0x0006
#define WM_SETFOCUS         0x0007
#define WM_KILLFOCUS        0x0008
#define WM_PAINT            0x000F
#define WM_CLOSE            0x0010
#define WM_QUIT             0x0012
#define WM_ERASEBKGND       0x0014
#define WM_SETCURSOR        0x0020
#define WM_TIMER            0x0113
#define WM_CHAR             0x0102
#define WM_MOUSEMOVE        0x0200
#define WM_LBUTTONDOWN      0x0201
#define WM_LBUTTONUP        0x0202
#define WM_LBUTTONDBLCLK    0x0203
#define WM_RBUTTONDOWN      0x0204
#define WM_RBUTTONUP        0x0205
#define WM_MBUTTONDOWN      0x0207
#define WM_MBUTTONUP        0x0208
#define WM_MOUSEWHEEL       0x020A
#define WM_IME_NOTIFY       0x0282
#define WM_CTLCOLOREDIT     0x0133

#define WA_INACTIVE         0
#define SIZE_RESTORED       0
#define SIZE_MINIMIZED      1
#define WHEEL_DELTA         120
#define VK_RETURN           0x0D
#define MB_OK               0x00000000L
#define BLACK_BRUSH         4

#define WS_CHILD            0x40000000L
#define WS_VISIBLE          0x10000000L
#define WS_BORDER           0x00800000L
#define WS_VSCROLL          0x00200000L
#define WS_HSCROLL          0x00100000L
#define ES_LEFT             0x0000L
#define ES_MULTILINE        0x0004L
#define GWL_WNDPROC         (-4)

struct PAINTSTRUCT { HDC hdc; BOOL fErase; };
struct RECT { LONG left, top, right, bottom; };
struct POINT { LONG x, y; };
using WNDPROC = LRESULT (CALLBACK *)(HWND, UINT, WPARAM, LPARAM);

// Registry constants.
#define HKEY_CURRENT_USER   ((HKEY)(intptr_t)0x80000001)
#define HKEY_LOCAL_MACHINE  ((HKEY)(intptr_t)0x80000002)
#define REG_OPTION_NON_VOLATILE 0
#define KEY_ALL_ACCESS      0xF003F
#define REG_DWORD           4
#define REG_SZ              1

// INI/path/time.
UINT  GetCurrentDirectoryA(UINT nBufferLength, LPSTR lpBuffer);
UINT  GetPrivateProfileStringA(LPCSTR section, LPCSTR key, LPCSTR def, LPSTR out, UINT size, LPCSTR file);
UINT  GetPrivateProfileIntA(LPCSTR section, LPCSTR key, int def, LPCSTR file);
LPSTR GetCommandLineA();
DWORD GetModuleFileNameA(HMODULE hModule, LPSTR lpFilename, DWORD nSize);
DWORD GetTickCount();
DWORD timeGetTime();
void  Sleep(DWORD ms);

#define GetCurrentDirectory GetCurrentDirectoryA
#define GetPrivateProfileString GetPrivateProfileStringA
#define GetPrivateProfileInt GetPrivateProfileIntA
#define GetCommandLine GetCommandLineA
#define GetModuleFileName GetModuleFileNameA

// Registry-as-file emulation.
LONG RegCreateKeyExA(HKEY root, LPCSTR subkey, DWORD reserved, LPSTR cls, DWORD options, DWORD sam, LPVOID sec, HKEY* result, DWORD* disp);
LONG RegOpenKeyExA(HKEY root, LPCSTR subkey, DWORD options, DWORD sam, HKEY* result);
LONG RegQueryValueExA(HKEY key, LPCSTR valueName, DWORD* reserved, DWORD* type, LPBYTE data, DWORD* size);
LONG RegSetValueExA(HKEY key, LPCSTR valueName, DWORD reserved, DWORD type, const BYTE* data, DWORD size);
LONG RegDeleteKeyA(HKEY root, LPCSTR subkey);
LONG RegDeleteValueA(HKEY key, LPCSTR valueName);
LONG RegCloseKey(HKEY key);
#define RegCreateKeyEx RegCreateKeyExA
#define RegOpenKeyEx RegOpenKeyExA
#define RegQueryValueEx RegQueryValueExA
#define RegSetValueEx RegSetValueExA
#define RegDeleteKey RegDeleteKeyA
#define RegDeleteValue RegDeleteValueA

// Dynamic library subset.
HMODULE LoadLibraryA(LPCSTR path);
LPVOID  GetProcAddress(HMODULE mod, LPCSTR name);
BOOL    FreeLibrary(HMODULE mod);
#define LoadLibrary LoadLibraryA

// Window/GDI stubs for compile-first pass. Replace with SDL later.
LRESULT DefWindowProcA(HWND, UINT, WPARAM, LPARAM);
BOOL    PostMessageA(HWND, UINT, WPARAM, LPARAM);
void    PostQuitMessage(int code);
UINT    SetTimer(HWND, UINT, UINT, void*);
BOOL    KillTimer(HWND, UINT);
int     MessageBoxA(HWND, LPCSTR text, LPCSTR caption, UINT type);
HDC     BeginPaint(HWND, PAINTSTRUCT*);
BOOL    EndPaint(HWND, const PAINTSTRUCT*);
BOOL    ShowCursor(BOOL show);
HWND    GetFocus();
HWND    SetFocus(HWND);
HWND    SetCapture(HWND);
BOOL    ReleaseCapture();
void    SetBkColor(HDC, DWORD);
void    SetTextColor(HDC, DWORD);
HBRUSH  GetStockObject(int obj);
LONG    SetWindowLongA(HWND, int, LONG);
LONG    GetWindowLongA(HWND, int);
HWND    CreateWindowA(LPCSTR, LPCSTR, DWORD, int, int, int, int, HWND, HMENU, HINSTANCE, LPVOID);
HWND    CreateWindowExA(DWORD, LPCSTR, LPCSTR, DWORD, int, int, int, int, HWND, HMENU, HINSTANCE, LPVOID);
#define DefWindowProc DefWindowProcA
#define PostMessage PostMessageA
#define MessageBox MessageBoxA
#define SetWindowLong SetWindowLongA
#define GetWindowLong GetWindowLongA
#define CreateWindow CreateWindowA
#define CreateWindowEx CreateWindowExA
#define RGB(r,g,b) ((DWORD)(((BYTE)(r)|((WORD)((BYTE)(g))<<8))|(((DWORD)(BYTE)(b))<<16)))

// Critical section.
struct CRITICAL_SECTION;
void InitializeCriticalSection(CRITICAL_SECTION*);
void EnterCriticalSection(CRITICAL_SECTION*);
void LeaveCriticalSection(CRITICAL_SECTION*);
void DeleteCriticalSection(CRITICAL_SECTION*);

#endif // !_WIN32
