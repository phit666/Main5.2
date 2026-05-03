#include "stdafx.h"

#ifndef _WIN32
#include "win_compat.h"
#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>
#include <unordered_map>
#include <cstring>
#include <cstdlib>
#include <cerrno>
#include <chrono>
#include <thread>
#include <mutex>
#include <dlfcn.h>
#include <iostream>

struct CRITICAL_SECTION { std::recursive_mutex m; };

static std::string trim(std::string s) {
    while (!s.empty() && (s.back()=='\r'||s.back()=='\n'||s.back()==' '||s.back()=='\t')) s.pop_back();
    size_t i=0; while (i<s.size() && (s[i]==' '||s[i]=='\t')) ++i;
    return s.substr(i);
}

UINT GetCurrentDirectoryA(UINT nBufferLength, LPSTR lpBuffer) {
    std::string path = std::filesystem::current_path().string();
    UINT len = (UINT)path.size();
    if (nBufferLength == 0) return len + 1;
    if (!lpBuffer) return 0;
    UINT copyLen = (len >= nBufferLength) ? nBufferLength - 1 : len;
    std::memcpy(lpBuffer, path.c_str(), copyLen);
    lpBuffer[copyLen] = '\0';
    return len;
}

UINT GetPrivateProfileStringA(LPCSTR section, LPCSTR key, LPCSTR def, LPSTR out, UINT size, LPCSTR filename) {
    if (!out || size == 0) return 0;
    std::string result = def ? def : "";
    std::ifstream file(filename ? filename : "");
    std::string line, cur;
    if (file) {
        while (std::getline(file, line)) {
            line = trim(line);
            if (line.empty() || line[0] == ';' || line[0] == '#') continue;
            if (line.front() == '[' && line.back() == ']') { cur = trim(line.substr(1, line.size()-2)); continue; }
            if (section && cur == section) {
                size_t eq = line.find('=');
                if (eq != std::string::npos) {
                    std::string k = trim(line.substr(0, eq));
                    std::string v = trim(line.substr(eq+1));
                    if (key && k == key) { result = v; break; }
                }
            }
        }
    }
    UINT len = (UINT)result.size();
    UINT copyLen = (len >= size) ? size - 1 : len;
    std::memcpy(out, result.c_str(), copyLen);
    out[copyLen] = '\0';
    return copyLen;
}

UINT GetPrivateProfileIntA(LPCSTR section, LPCSTR key, int def, LPCSTR file) {
    char buf[64] = {};
    GetPrivateProfileStringA(section, key, "", buf, sizeof(buf), file);
    if (!buf[0]) return (UINT)def;
    errno = 0; char* end = nullptr; long v = std::strtol(buf, &end, 10);
    if (end == buf || errno == ERANGE) return (UINT)def;
    return (UINT)v;
}

LPSTR GetCommandLineA() {
    static char cmd[MAX_PATH] = {};
    if (!cmd[0]) {
        std::string p = std::filesystem::current_path().string() + "/muclient";
        std::snprintf(cmd, sizeof(cmd), "\"%s\"", p.c_str());
    }
    return cmd;
}

DWORD GetModuleFileNameA(HMODULE, LPSTR out, DWORD size) {
    if (!out || size == 0) return 0;
    std::string p = std::filesystem::current_path().string() + "/muclient";
    DWORD copyLen = (p.size() >= size) ? size - 1 : (DWORD)p.size();
    std::memcpy(out, p.c_str(), copyLen); out[copyLen] = 0; return copyLen;
}

DWORD GetTickCount() {
    using namespace std::chrono;
    return (DWORD)duration_cast<milliseconds>(steady_clock::now().time_since_epoch()).count();
}
DWORD timeGetTime() { return GetTickCount(); }
void Sleep(DWORD ms) { std::this_thread::sleep_for(std::chrono::milliseconds(ms)); }

// Registry emulation: values are kept in memory for now. You can persist this to config/registry.ini later.
struct RegKeyHandle { std::string path; };
static std::unordered_map<std::string, std::vector<BYTE>> g_reg;
static std::string rootName(HKEY root) { return root == HKEY_LOCAL_MACHINE ? "HKLM" : "HKCU"; }
static std::string keyPath(HKEY root, LPCSTR sub) {
    if (root == HKEY_CURRENT_USER || root == HKEY_LOCAL_MACHINE) return rootName(root) + "\\" + (sub ? sub : "");
    auto* h = reinterpret_cast<RegKeyHandle*>(root); return h ? h->path : std::string();
}
LONG RegCreateKeyExA(HKEY root, LPCSTR subkey, DWORD, LPSTR, DWORD, DWORD, LPVOID, HKEY* result, DWORD* disp) {
    if (!result) return ERROR_INVALID_PARAMETER;
    *result = new RegKeyHandle{keyPath(root, subkey)}; if (disp) *disp = 0; return ERROR_SUCCESS;
}
LONG RegOpenKeyExA(HKEY root, LPCSTR subkey, DWORD, DWORD, HKEY* result) { return RegCreateKeyExA(root, subkey,0,nullptr,0,0,nullptr,result,nullptr); }
LONG RegQueryValueExA(HKEY key, LPCSTR valueName, DWORD*, DWORD* type, LPBYTE data, DWORD* size) {
    if (!size || !valueName) return ERROR_INVALID_PARAMETER;
    std::string k = keyPath(key, nullptr) + "\\" + valueName;
    auto it = g_reg.find(k); if (it == g_reg.end()) return ERROR_FILE_NOT_FOUND;
    if (type) *type = REG_SZ;
    if (!data || *size < it->second.size()) { *size = (DWORD)it->second.size(); return ERROR_MORE_DATA; }
    std::memcpy(data, it->second.data(), it->second.size()); *size = (DWORD)it->second.size(); return ERROR_SUCCESS;
}
LONG RegSetValueExA(HKEY key, LPCSTR valueName, DWORD, DWORD, const BYTE* data, DWORD size) {
    if (!valueName || (!data && size)) return ERROR_INVALID_PARAMETER;
    std::string k = keyPath(key, nullptr) + "\\" + valueName;
    g_reg[k] = std::vector<BYTE>(data, data + size); return ERROR_SUCCESS;
}
LONG RegDeleteKeyA(HKEY root, LPCSTR subkey) { (void)root; (void)subkey; return ERROR_SUCCESS; }
LONG RegDeleteValueA(HKEY key, LPCSTR valueName) { g_reg.erase(keyPath(key,nullptr)+"\\"+(valueName?valueName:"")); return ERROR_SUCCESS; }
LONG RegCloseKey(HKEY key) { if (key && key != HKEY_CURRENT_USER && key != HKEY_LOCAL_MACHINE) delete reinterpret_cast<RegKeyHandle*>(key); return ERROR_SUCCESS; }

HMODULE LoadLibraryA(LPCSTR path) { return path ? dlopen(path, RTLD_LAZY) : nullptr; }
LPVOID GetProcAddress(HMODULE mod, LPCSTR name) { return (mod && name) ? dlsym(mod, name) : nullptr; }
BOOL FreeLibrary(HMODULE mod) { return mod ? (dlclose(mod) == 0) : FALSE; }

LRESULT DefWindowProcA(HWND, UINT, WPARAM, LPARAM) { return 0; }
BOOL PostMessageA(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) { extern void WinCompat_DispatchMessage(HWND,UINT,WPARAM,LPARAM); WinCompat_DispatchMessage(hwnd,msg,wp,lp); return TRUE; }
void PostQuitMessage(int) {}
UINT SetTimer(HWND, UINT id, UINT, void*) { return id; }
BOOL KillTimer(HWND, UINT) { return TRUE; }
int MessageBoxA(HWND, LPCSTR text, LPCSTR caption, UINT) { std::cerr << (caption?caption:"MessageBox") << ": " << (text?text:"") << std::endl; return 0; }
HDC BeginPaint(HWND, PAINTSTRUCT* ps) { return ps ? ps->hdc : nullptr; }
BOOL EndPaint(HWND, const PAINTSTRUCT*) { return TRUE; }
BOOL ShowCursor(BOOL) { return TRUE; }
static HWND g_focus = nullptr, g_capture = nullptr;
HWND GetFocus() { return g_focus; }
HWND SetFocus(HWND h) { HWND old=g_focus; g_focus=h; return old; }
HWND SetCapture(HWND h) { HWND old=g_capture; g_capture=h; return old; }
BOOL ReleaseCapture() { g_capture=nullptr; return TRUE; }
void SetBkColor(HDC, DWORD) {}
void SetTextColor(HDC, DWORD) {}
HBRUSH GetStockObject(int) { return nullptr; }
LONG SetWindowLongA(HWND, int, LONG) { return 0; }
LONG GetWindowLongA(HWND, int) { return 0; }
HWND CreateWindowA(LPCSTR, LPCSTR, DWORD, int, int, int, int, HWND, HMENU, HINSTANCE, LPVOID) { return reinterpret_cast<HWND>(1); }
HWND CreateWindowExA(DWORD, LPCSTR, LPCSTR, DWORD, int, int, int, int, HWND, HMENU, HINSTANCE, LPVOID) { return reinterpret_cast<HWND>(1); }

void InitializeCriticalSection(CRITICAL_SECTION*) {}
void EnterCriticalSection(CRITICAL_SECTION* cs) { if (cs) cs->m.lock(); }
void LeaveCriticalSection(CRITICAL_SECTION* cs) { if (cs) cs->m.unlock(); }
void DeleteCriticalSection(CRITICAL_SECTION*) {}

// Weak dispatcher; your SDL/libevent platform layer can override by defining this symbol.
__attribute__((weak)) void WinCompat_DispatchMessage(HWND, UINT, WPARAM, LPARAM) {}

#endif
