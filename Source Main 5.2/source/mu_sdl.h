#pragma once
//#include <SDL.h>
//#include <SDL_opengles2.h>
#include "w_nuklear.h"
#include "mu_file.h"
#include <event2/bufferevent.h>
#include <event2/buffer.h>
#include <event2/event.h>
#include <thread>
#include <chrono>

extern bool gIsMixerInit;


extern SDL_Window* gSDLWindow;
extern SDL_GLContext gGLContext;

extern bool gSDLRunning;
extern nk_context* g_nk_ctx;

extern event_base* g_eventBase;
extern event* g_socketReadEvent;
extern event* g_socketWriteEvent;
//extern SOCKET g_socket;

bool MU_InitSDL(int width, int height);
void MU_ShutdownSDL();
void MU_ProcessSDLEvents();
//bool MU_RegisterSocketEvent(SOCKET s);
void MU_EnableSocketWrite();
void MU_DisableSocketWrite();
void MU_Close();
int MU_Connect(char* serverip, unsigned short port);
void MU_CloseBev();
int MU_BevSend(const void* data, int len);
evutil_socket_t MU_GetFD();

void UpdateDebugCameraByKeyboard(float dt);

#ifndef _WIN32
bool SetCursorPos(int x, int y) {
    if (!gSDLWindow) return false;
    // SDL_WarpMouseInWindow moves the mouse to (x, y) relative to the window.
    SDL_WarpMouseInWindow(gSDLWindow, x, y);
    return true;
}
#endif


