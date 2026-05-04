#include "stdafx.h"
#define NK_IMPLEMENTATION
#include "w_nuklear.h"
#define NK_SDL_GL2_IMPLEMENTATION
#include "nuklear_sdl_gl2.h"
#include "mu_sdl.h"
#include "WSclient.h"
#include "wsctlc.h"


SDL_Window* gSDLWindow = nullptr;
SDL_GLContext gGLContext = nullptr;

bool gSDLRunning = true;
static BYTE buffer[MAX_RECVBUF];
static int bufferlen = 0;

event_base* g_eventBase = nullptr;
bufferevent* g_bev;
event* g_socketReadEvent = nullptr;
event* g_socketWriteEvent = nullptr;
SOCKET g_socket = INVALID_SOCKET;

nk_context* g_nk_ctx = nullptr;

// LIBEVENT

void MU_EnableSocketWrite()
{
#ifdef MU_USE_SDL
    if (g_socketWriteEvent)
        event_add(g_socketWriteEvent, nullptr);
#endif
}

void MU_DisableSocketWrite()
{
#ifdef MU_USE_SDL
    if (g_socketWriteEvent)
        event_del(g_socketWriteEvent);
#endif
}

bool MU_InitNetworkEvent()
{
    memset(buffer, 0, MAX_RECVBUF);
    g_eventBase = event_base_new();

    if (!g_eventBase)
        return false;

    return true;
}

void MU_Close()
{
#ifdef MU_USE_SDL
    if (g_socketReadEvent)
    {
        event_free(g_socketReadEvent);
        g_socketReadEvent = nullptr;
    }

    if (g_socketWriteEvent)
    {
        event_free(g_socketWriteEvent);
        g_socketWriteEvent = nullptr;
    }
    g_socket = INVALID_SOCKET;
#endif
}

void MU_ShutdownNetworkEvent()
{
    if (g_socketReadEvent)
    {
        event_free(g_socketReadEvent);
        g_socketReadEvent = nullptr;
    }

    if (g_socketWriteEvent)
    {
        event_free(g_socketWriteEvent);
        g_socketWriteEvent = nullptr;
    }

    if (g_eventBase)
    {
        event_base_free(g_eventBase);
        g_eventBase = nullptr;
    }
}

void MU_OnSocketEvent(evutil_socket_t fd, short events, void*)
{
    if (events & EV_READ)
    {
        OutputDebugStringA("[SDL-DEBUG] MU_OnSocketEvent, EV_READ");
        if (SocketClient.nRecv() == 1) {
            SocketClient.Close();
            return;
        }
    }

    if (events & EV_WRITE)
    {
        OutputDebugStringA("[SDL-DEBUG] MU_OnSocketEvent, EV_WRITE");
        if (SocketClient.FDWriteSend())
        {
            MU_DisableSocketWrite();
        }
    }
}

bool MU_RegisterSocketEvent(SOCKET s)
{
    if (!g_eventBase || s == INVALID_SOCKET)
        return false;

    g_socket = s;

    evutil_make_socket_nonblocking(s);

    g_socketReadEvent = event_new(
        g_eventBase,
        s,
        EV_READ | EV_PERSIST,
        MU_OnSocketEvent,
        nullptr
    );

    g_socketWriteEvent = event_new(
        g_eventBase,
        s,
        EV_WRITE | EV_PERSIST,
        MU_OnSocketEvent,
        nullptr
    );

    if (!g_socketReadEvent || !g_socketWriteEvent)
        return false;

    event_add(g_socketReadEvent, nullptr);

    return true;
}

DWORD host2ip(const char* hostname)
{
    struct hostent* h = gethostbyname(hostname);
    return (h != NULL) ? ntohl(*(DWORD*)h->h_addr) : 0;
}


static void
conn_readcb(struct bufferevent* bev, void* user_data)
{
    size_t len;
   len = bufferevent_read(bev, (char*)buffer + bufferlen,
        MAX_RECVBUF - bufferlen);

    if (len < 3)
    {
        bufferlen += len;
        return;
    }

    bufferlen += len;
    SocketClient.PushPacket(buffer, bufferlen);
    bufferlen = 0;
    memset(buffer, 0, MAX_RECVBUF);
}

static void
conn_eventcb(struct bufferevent* bev, short events, void* user_data)
{
    if (events & BEV_EVENT_EOF)
    {
        bufferevent_free(g_bev);
        g_bev = NULL;
    }
    else if (events & BEV_EVENT_ERROR)
    {
        bufferevent_free(g_bev);
        g_bev = NULL;
    }
    else if (events & BEV_EVENT_CONNECTED)
    {
    }
}

int MU_Connect(char* serverip, unsigned short port)
{
    struct bufferevent* bev;
    struct sockaddr_in remote_address;
    int result;

    if (!g_eventBase)
        return -1;

    bev = bufferevent_socket_new(g_eventBase, -1,
        BEV_OPT_CLOSE_ON_FREE
        | BEV_OPT_THREADSAFE
        | BEV_OPT_UNLOCK_CALLBACKS
        | BEV_OPT_DEFER_CALLBACKS
    );

    if (!bev)
    {
        return -1;
    }

    remote_address.sin_family = AF_INET;
    remote_address.sin_addr.s_addr = htonl(host2ip(serverip));
    remote_address.sin_port = htons(port);

    result = bufferevent_socket_connect(bev, (struct sockaddr*)&remote_address, sizeof(remote_address));

    if (result == -1) {
        return -1;
    }

    g_bev = bev;

    bufferevent_setcb(bev, conn_readcb, NULL, conn_eventcb, NULL);
    bufferevent_enable(bev, EV_WRITE);
    bufferevent_enable(bev, EV_READ);

    return 1;
}

static void defer_free_cb(evutil_socket_t, short, void* arg)
{
    if (g_bev != NULL)
    {
        bufferevent_free(g_bev);
        g_bev = NULL;
    }
}

void MU_CloseBev()
{
    if (g_bev == NULL)
        return;
    int arg = 0;
    timeval tv = { 0, 0 };
    bufferevent_disable(g_bev, EV_WRITE || EV_READ);
    bufferevent_setcb(g_bev, nullptr, nullptr, nullptr, nullptr);

    evutil_socket_t fd = bufferevent_getfd(g_bev);

    if (fd != EVUTIL_INVALID_SOCKET) {
        shutdown(fd, SD_BOTH);
    }

    event_base_once(g_eventBase, -1, EV_TIMEOUT, defer_free_cb, (LPVOID)static_cast<int>(arg), &tv);
}

// SDL

bool MU_InitSDL(int width, int height)
{
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_EVENTS) != 0)
        return false;

    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

    gSDLWindow = SDL_CreateWindow(
        "MU",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        width,
        height,
        SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL
    );

    if (!gSDLWindow)
        return false;

    gGLContext = SDL_GL_CreateContext(gSDLWindow);

    if (!gGLContext)
        return false;

    SDL_GL_MakeCurrent(gSDLWindow, gGLContext);
    SDL_GL_SetSwapInterval(1);
    SDL_StartTextInput();

    g_nk_ctx = nk_sdl_init(gSDLWindow);

    struct nk_font_atlas* atlas;
    nk_sdl_font_stash_begin(&atlas);
    nk_sdl_font_stash_end();

    if (!MU_InitNetworkEvent()) {
        OutputDebugStringA("[SDL-DEBUG] MU_InitNetworkEvent failed.");
        return false;
    }

    return true;
}

void MU_ShutdownSDL()
{
    SDL_StopTextInput();

    if (gGLContext)
    {
        SDL_GL_DeleteContext(gGLContext);
        gGLContext = nullptr;
    }

    if (gSDLWindow)
    {
        SDL_DestroyWindow(gSDLWindow);
        gSDLWindow = nullptr;
    }

    nk_sdl_shutdown();
    SDL_Quit();
    MU_ShutdownNetworkEvent();

    OutputDebugStringA("[SDL-DEBUG] MU_ShutdownSDL");
}
