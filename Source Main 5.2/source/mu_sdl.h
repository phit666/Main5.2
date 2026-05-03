#pragma once

#include <SDL.h>
#include "w_nuklear.h"
#include "mu_file.h"
#include "mu_gl.h"
#include "mu_2d_renderer.h"
#include <event2/bufferevent.h>
#include <event2/buffer.h>
#include <event2/event.h>
#include <thread>
#include <chrono>

extern SDL_Window* gSDLWindow;
extern SDL_GLContext gGLContext;

extern bool gSDLRunning;
extern nk_context* g_nk_ctx;

extern event_base* g_eventBase;
extern event* g_socketReadEvent;
extern event* g_socketWriteEvent;
extern SOCKET g_socket;

bool MU_InitSDL(int width, int height);
void MU_ShutdownSDL();
void MU_ProcessSDLEvents();
bool MU_RegisterSocketEvent(SOCKET s);
void MU_EnableSocketWrite();
void MU_DisableSocketWrite();
void MU_Close();