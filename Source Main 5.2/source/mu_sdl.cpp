#include "stdafx.h"
#define NK_IMPLEMENTATION
#include "w_nuklear.h"
#define NK_SDL_GLES2_IMPLEMENTATION
#include "nuklear_sdl_gles2.h"
#include "mu_sdl.h"
#include "WSclient.h"
#include "wsctlc.h"
#include "wsctlc_addon.h"
#include "mu_gles2_matrix.h"

#ifndef _WIN32
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#endif

typedef struct
{
    BYTE c;
    BYTE size;
    BYTE headcode1;
} MU_WSCTLC_PBMSG_HEAD, * MU_WSCTLC_LPPBMSG_HEAD;

typedef struct
{
    BYTE c;
    BYTE sizeH;
    BYTE sizeL;
    BYTE headcode1;
} MU_WSCTLC_PWMSG_HEAD, * MU_WSCTLC_LPPWMSG_HEAD;

SDL_Window* gSDLWindow = nullptr;
SDL_GLContext gGLContext = nullptr;

bool gIsMixerInit = false;

bool gSDLRunning = true;
static BYTE buffer[MAX_RECVBUF];
static int bufferlen = 0;

event_base* g_eventBase = nullptr;
bufferevent* g_bev;
event* g_socketReadEvent = nullptr;
event* g_socketWriteEvent = nullptr;
SOCKET g_socket = INVALID_SOCKET;

nk_context* g_nk_ctx = nullptr;


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
#if USE_LIBEVENT == 0
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
#endif
}

void MU_ShutdownNetworkEvent()
{
#if USE_LIBEVENT == 1
    MU_CloseBev();
#else
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
#endif
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
        //OutputDebugStringA("[SDL-DEBUG] MU_OnSocketEvent, EV_READ");
        if (SocketClient.nRecv() == 1) {
            SocketClient.Close();
            return;
        }
    }

    if (events & EV_WRITE)
    {
       // OutputDebugStringA("[SDL-DEBUG] MU_OnSocketEvent, EV_WRITE");
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

    if (bufferlen >= MAX_RECVBUF)
        return;

    len = bufferevent_read(bev, (char*)buffer + bufferlen,
        MAX_RECVBUF - bufferlen);

    bufferlen += len;

    if (bufferlen < 3)
    {
        return;
    }

    int lOfs = 0;
    int size = 0;

    while (1)
    {
        if (buffer[lOfs] == 0xC1 || buffer[lOfs] == 0xC3)
        {
            MU_WSCTLC_LPPBMSG_HEAD lphead = (MU_WSCTLC_LPPBMSG_HEAD)(buffer + lOfs);
            size = (int)lphead->size;
        }
        else if (buffer[lOfs] == 0xC2 || buffer[lOfs] == 0xC4)
        {
            MU_WSCTLC_LPPWMSG_HEAD lphead = (MU_WSCTLC_LPPWMSG_HEAD)(buffer + lOfs);
            size = (((int)(lphead->sizeH)) << 8) + lphead->sizeL;
        }
        else {
            bufferlen = 0;
            return;
        }


        if (size <= 0)
        {
            return;
        }
        else if (size <= bufferlen)
        {
            SocketClient.PushPacket(buffer + lOfs, size);

            lOfs += size;
            bufferlen -= size;
            if (bufferlen <= 0)
            {
                break;
            }
        }
        else
        {
            if (lOfs > 0)
            {
                if (bufferlen < 1)
                {
                    break;
                }
                else
                {
                    memcpy(buffer, buffer + lOfs, bufferlen);
                }
            }
            break;
        }
    }

    SocketClient.ClearGarbage();

}

static void
conn_eventcb(struct bufferevent* bev, short events, void* user_data)
{
    if (events & BEV_EVENT_EOF)
    {
        bufferevent_free(g_bev);
        g_bev = NULL;
        SocketClient.Close();
    }
    else if (events & BEV_EVENT_ERROR)
    {
        bufferevent_free(g_bev);
        g_bev = NULL;
        SocketClient.Close();
    }
    else if (events & BEV_EVENT_CONNECTED)
    {
        //SocketClient.OnConnect();
    }
}

int MU_Connect(char* serverip, unsigned short port)
{
    if (!g_eventBase)
        return -1;

    struct bufferevent* bev = bufferevent_socket_new(
        g_eventBase,
        -1,
        BEV_OPT_CLOSE_ON_FREE
    );

    if (!bev)
        return -1;

    sockaddr_in remote_address;
    memset(&remote_address, 0, sizeof(remote_address));

    remote_address.sin_family = AF_INET;
    remote_address.sin_addr.s_addr = inet_addr(serverip);
    remote_address.sin_port = htons(port);

    if (bufferevent_socket_connect(
        bev,
        (struct sockaddr*)&remote_address,
        sizeof(remote_address)) < 0)
    {
        bufferevent_free(bev);
        return -1;
    }

    bufferevent_setcb(bev, conn_readcb, NULL, conn_eventcb, NULL);
    bufferevent_enable(bev, EV_READ | EV_WRITE);

    g_bev = bev;
    bufferlen = 0;
    return 1;
}

int MU_BevSend(const void* data, int len)
{
    if (!g_bev || !data || len <= 0)
        return FALSE;

    //char t[100] = { 0 };
    //sprintf(t, "[SDL-DEBUG] MU_BevSend, len %d", len);
    //OutputDebugStringA(t);


    return bufferevent_write(g_bev, data, len) == 0 ? TRUE : FALSE;
}

void MU_CloseBev()
{
    if (!g_bev)
        return;

    struct bufferevent* bev = g_bev;
    g_bev = NULL;

    bufferevent_disable(bev, EV_READ | EV_WRITE);
    bufferevent_setcb(bev, NULL, NULL, NULL, NULL);

    evutil_socket_t fd = bufferevent_getfd(bev);
    if (fd != EVUTIL_INVALID_SOCKET)
        shutdown(fd, SD_BOTH);

    bufferevent_free(bev);

    bufferlen = 0;
}

evutil_socket_t MU_GetFD() {
    return bufferevent_getfd(g_bev);
}

/*
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
}*/

// SDL

bool MU_InitSDL(int width, int height)
{
    OutputDebugStringA("[SDL-DEBUG] SDL_Init");

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_EVENTS) != 0)
        return false;

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

    OutputDebugStringA("[SDL-DEBUG] SDL_CreateWindow");

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

    OutputDebugStringA("[SDL-DEBUG] SDL_GL_CreateContext");

    gGLContext = SDL_GL_CreateContext(gSDLWindow);

    if (!gGLContext)
        return false;

    SDL_GL_MakeCurrent(gSDLWindow, gGLContext);

    if (!gladLoadGLES2Loader((GLADloadproc)SDL_GL_GetProcAddress))
    {
        OutputDebugStringA("[SDL-DEBUG] gladLoadGLES2Loader failed.");
        return false;
    }

    SDL_GL_SetSwapInterval(1);
    SDL_StartTextInput();


    /*glewExperimental = GL_TRUE;
    GLenum err = glewInit();

    if (err != GLEW_OK)
    {
        OutputDebugStringA("[SDL-DEBUG] glewInit failed");
        return false;
    }*/

    OutputDebugStringA("[SDL-DEBUG] InitShader");

    InitShader();

    //OutputDebugStringA("[SDL-DEBUG] nk_sdl_init");

    //g_nk_ctx = nk_sdl_init(gSDLWindow);

    //struct nk_font_atlas* atlas;
    //nk_sdl_font_stash_begin(&atlas);
    //nk_sdl_font_stash_end();

    OutputDebugStringA("[SDL-DEBUG] MU_InitNetworkEvent");

    if (!MU_InitNetworkEvent()) {
        //OutputDebugStringA("[SDL-DEBUG] MU_InitNetworkEvent failed.");
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

    //nk_sdl_shutdown();
    SDL_Quit();
    MU_ShutdownNetworkEvent();

    //OutputDebugStringA("[SDL-DEBUG] MU_ShutdownSDL");
}



