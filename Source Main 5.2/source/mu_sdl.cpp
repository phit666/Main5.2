#include "stdafx.h"
//#define NK_IMPLEMENTATION
//#ifdef _WIN32
//#include "w_nuklear.h"
//#else
//#include "nuklear.h"
//#endif
//#define NK_SDL_GLES2_IMPLEMENTATION
//#include "nuklear_sdl_gles2.h"
#include "MU_UIRenderer.h"

#include "mu_sdl.h"
#include "WSclient.h"
#include "wsctlc.h"
#include "wsctlc_addon.h"
#include "mu_gles2_matrix.h"
#include "wt.h"
#include "ZzzInterface.h"

#ifndef _WIN32
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <clocale>
#endif

#include "Winmain.h"
#include "MU_EditControl.h"
#include "ComplexModulus.h"

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

static std::map<uintptr_t, SDL_TimerID> g_ActiveTimers;

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
            g_ErrorReport.Write("> MU_OnSocketEvent - Close(), SocketClient.nRecv() == 1");
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
        g_ErrorReport.Write("> conn_eventcb - BEV_EVENT_EOF");
        SocketClient.Close();

#ifdef ENHANCE_ENCDEC
        g_CryptoSessionCS.Close(0);
        g_CryptoSessionSC.Close(0);
#endif

    }
    else if (events & BEV_EVENT_ERROR)
    {
        bufferevent_free(g_bev);
        g_bev = NULL;
        g_ErrorReport.Write("> conn_eventcb - BEV_EVENT_ERROR");
        SocketClient.Close();

#ifdef ENHANCE_ENCDEC
        g_CryptoSessionCS.Close(0);
        g_CryptoSessionSC.Close(0);
#endif

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

#ifdef ENHANCE_ENCDEC
    g_CryptoSessionCS.Open(0);
    g_CryptoSessionSC.Open(0);
#endif


    g_bev = bev;
    bufferlen = 0;
    return 1;
}

int MU_BevSend(const void* data, int len)
{
    if (!g_bev || !data || len <= 0)
        return FALSE;

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


#ifdef ENHANCE_ENCDEC
    g_CryptoSessionCS.Close(0);
    g_CryptoSessionSC.Close(0);
#endif

    bufferlen = 0;
}

evutil_socket_t MU_GetFD() {
    if (g_bev == nullptr)
        return -1;
    return bufferevent_getfd(g_bev);
}

// The background callback that pushes the event to the main loop
Uint32 SDL_TimerCallbackWrapper(Uint32 interval, void* param) {
    SDL_Event event;
    SDL_zero(event);
    event.type = SDL_USEREVENT;
    event.user.code = (intptr_t)param; // This is your CHATCONNECT_TIMER ID

    SDL_PushEvent(&event);
    return interval; // Return interval to keep it recurring
}

// Wrapper to mimic Windows SetTimer
uintptr_t MU_SetTimer(void* hWnd, uintptr_t nIDEvent, unsigned int uElapse, void* lpTimerFunc) {
    // If timer already exists for this ID, kill it first (Windows behavior)
    if (g_ActiveTimers.count(nIDEvent)) {
        SDL_RemoveTimer(g_ActiveTimers[nIDEvent]);
    }

    // Start the SDL timer
    SDL_TimerID sdlID = SDL_AddTimer(uElapse, SDL_TimerCallbackWrapper, (void*)nIDEvent);

    // Save it to our map
    g_ActiveTimers[nIDEvent] = sdlID;

    return nIDEvent;
}

bool MU_KillTimer(void* hWnd, uintptr_t nIDEvent) {
    auto it = g_ActiveTimers.find(nIDEvent);
    if (it != g_ActiveTimers.end()) {
        SDL_RemoveTimer(it->second);
        g_ActiveTimers.erase(it);
        return true;
    }
    return false;
}


static void* fontrawdata;
static Sint64 fontrawsize;
struct nk_font* font[MAX_FONTS];
static bool iscustomfontfailed = false;

bool loadfont(char* fontpath)
{
    SDL_RWops* rw = SDL_RWFromFile(fontpath, "rb");
    if (!rw) {
        g_ErrorReport.Write("SDL_RWFromFile failed: %s", SDL_GetError());
        return false;
    }
    else {
        fontrawsize = SDL_RWsize(rw);
        fontrawdata = SDL_malloc((size_t)fontrawsize);
        if (!fontrawdata) {
            SDL_RWclose(rw);
            return false;
        }
        else {
            SDL_RWread(rw, fontrawdata, 1, (size_t)fontrawsize);
            SDL_RWclose(rw);
        }
    }
    return true;
}

static int fontsize = 0;

void setfont(int size) {
    if (iscustomfontfailed)
        return;
    if (size < 0 || size >= (MAX_FONTS))
        return;
    if (font[size] == nullptr)
        return;
    //nk_style_set_font(g_nk_ctx, &font[size]->handle);
    fontsize = size;
}

int pushfont(int size) {
    if (iscustomfontfailed)
        return 0;
    if (size < 0 || size >= (MAX_FONTS))
        return 0;
    if (font[size] == nullptr)
        return 0;
    //nk_style_set_font(g_nk_ctx, &font[size]->handle);
    return size * g_fScreenRate_y;
}

void popfont() {
    //nk_style_set_font(g_nk_ctx, &font[fontsize]->handle);
}

GLuint g_meshVBO = 0;

void InitMeshVBO()
{
    glGenBuffers(1, &g_meshVBO);
}

void DestroyMeshVBO()
{
    if (g_meshVBO)
    {
        glDeleteBuffers(1, &g_meshVBO);
        g_meshVBO = 0;
    }
}

bool MU_InitSDL(int width, int height)
{
    g_ErrorReport.Write( "MU_InitSDL > SDL_Init\r\n");

#ifndef _WIN32
    setlocale(LC_ALL, "");
#endif

     if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_EVENTS) != 0)
        return false;

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

    g_ErrorReport.Write( "MU_InitSDL > SDL_CreateWindow\r\n");

#ifdef __ANDROID__
    SDL_SetHint(SDL_HINT_ORIENTATIONS, "LandscapeLeft LandscapeRight");
    width = 0;
    height = 0;
#endif

    gSDLWindow = SDL_CreateWindow(
        "MU",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        width,
        height,
#ifndef _WIN32
        SDL_WINDOW_FULLSCREEN | SDL_WINDOW_OPENGL
#else
        SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL
#endif
    );

    if (!gSDLWindow)
        return false;

    g_ErrorReport.Write( "MU_InitSDL > SDL_GL_CreateContext\r\n");

    gGLContext = SDL_GL_CreateContext(gSDLWindow);

    if (!gGLContext)
        return false;

    SDL_GL_MakeCurrent(gSDLWindow, gGLContext);
    
    SDL_GL_SetSwapInterval(1);

#ifdef _WIN32
    if (!gladLoadGLES2Loader((GLADloadproc)SDL_GL_GetProcAddress))
    {
        //OutputDebugStringA("[SDL-DEBUG] gladLoadGLES2Loader failed.");
        return false;
    }
#endif

    InitMeshVBO();

//#ifdef __ANDROID__
    int _screen_w, _screen_h;
    SDL_GL_GetDrawableSize(gSDLWindow, &_screen_w, &_screen_h);
    float scale = (std::max)(WindowWidth / 640.0f, WindowHeight / 480.0f);

    WindowWidth = _screen_w;
    WindowHeight = _screen_h;

    g_fScreenRate_x = WindowWidth / 640.0f;
    g_fScreenRate_y = WindowHeight / 480.0f;

    g_ErrorReport.Write("> SDL_GL_GetDrawableSize, Screen Width %d Height %d", WindowWidth, WindowHeight);
//#endif

    InitShader();

    g_ErrorReport.Write( "MU_InitSDL > nk_sdl_init\r\n");


    g_hFont = 0;
    g_hFontBold = 1;
    g_hFontBig = 2;
    g_hFixFont = 3;


#ifdef _WIN32
    switch (WindowWidth)
    {
    case 640:FontHeight = 12; break;
    case 800:FontHeight = 13; break;
    case 1024:FontHeight = 14; break;
    case 1280:FontHeight = 15; break;
    }
    int nFixFontHeight = 13;
#endif

#ifdef __ANDROID__
    FontHeight = 15 * g_fScreenRate_y;
    int nFixFontHeight = 13 * g_fScreenRate_y;
#endif

    int nFixFontSize;
    int iFontSize;

    iFontSize = FontHeight - 1;
    nFixFontSize = nFixFontHeight - 1;

    MU_2DRenderer_Init(WindowWidth, WindowHeight);
    MU_SetFontSlot(g_hFont, "data/fonts/gulim.ttf", iFontSize);
    MU_SetFontSlot(g_hFontBold, "data/fonts/verdana.ttf", iFontSize);
    MU_SetFontSlot(g_hFixFont, "data/fonts/gulim.ttf", nFixFontHeight);
    MU_SetFontSlot(g_hFontBig, "data/fonts/verdana.ttf", iFontSize * 2);

    MU_SetActiveFont(g_hFont);

    g_ErrorReport.Write( "MU_InitSDL > MU_InitNetworkEvent\r\n");

    if (!MU_InitNetworkEvent()) {
        //OutputDebugStringA("[SDL-DEBUG] MU_InitNetworkEvent failed.");
        return false;
    }

    return true;
}

void MU_ShutdownSDL()
{
    SDL_StopTextInput();
    
    DestroyMeshVBO();

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

    MU_2DRenderer_Destroy();
    //nk_sdl_shutdown();
    SDL_Quit();
    MU_ShutdownNetworkEvent();

    //OutputDebugStringA("[SDL-DEBUG] MU_ShutdownSDL");
}

#ifdef VK_LBUTTON
#undef VK_LBUTTON
#endif

#ifdef VK_RBUTTON
#undef VK_RBUTTON
#endif

#ifdef VK_MBUTTON
#undef VK_MBUTTON
#endif

#ifdef VK_BACK
#undef VK_BACK
#endif

#ifdef VK_TAB
#undef VK_TAB
#endif

#ifdef VK_RETURN
#undef VK_RETURN
#endif

#ifdef VK_SHIFT
#undef VK_SHIFT
#endif

#ifdef VK_CONTROL
#undef VK_CONTROL
#endif

#ifdef VK_MENU
#undef VK_MENU
#endif

#ifdef VK_PAUSE
#undef VK_PAUSE
#endif

#ifdef VK_ESCAPE
#undef VK_ESCAPE
#endif

#ifdef VK_SPACE
#undef VK_SPACE
#endif

#ifdef VK_PRIOR
#undef VK_PRIOR
#endif

#ifdef VK_NEXT
#undef VK_NEXT
#endif

#ifdef VK_END
#undef VK_END
#endif

#ifdef VK_HOME
#undef VK_HOME
#endif

#ifdef VK_LEFT
#undef VK_LEFT
#endif

#ifdef VK_UP
#undef VK_UP
#endif

#ifdef VK_RIGHT
#undef VK_RIGHT
#endif

#ifdef VK_DOWN
#undef VK_DOWN
#endif

#ifdef VK_INSERT
#undef VK_INSERT
#endif

#ifdef VK_DELETE
#undef VK_DELETE
#endif

#ifdef VK_0
#undef VK_0
#endif

#ifdef VK_1
#undef VK_1
#endif

#ifdef VK_2
#undef VK_2
#endif

#ifdef VK_3
#undef VK_3
#endif

#ifdef VK_4
#undef VK_4
#endif

#ifdef VK_5
#undef VK_5
#endif

#ifdef VK_6
#undef VK_6
#endif

#ifdef VK_7
#undef VK_7
#endif

#ifdef VK_8
#undef VK_8
#endif

#ifdef VK_9
#undef VK_9
#endif

#ifdef VK_A
#undef VK_A
#endif

#ifdef VK_B
#undef VK_B
#endif

#ifdef VK_C
#undef VK_C
#endif

#ifdef VK_D
#undef VK_D
#endif

#ifdef VK_E
#undef VK_E
#endif

#ifdef VK_F
#undef VK_F
#endif

#ifdef VK_G
#undef VK_G
#endif

#ifdef VK_H
#undef VK_H
#endif

#ifdef VK_I
#undef VK_I
#endif

#ifdef VK_J
#undef VK_J
#endif

#ifdef VK_K
#undef VK_K
#endif

#ifdef VK_L
#undef VK_L
#endif

#ifdef VK_M
#undef VK_M
#endif

#ifdef VK_N
#undef VK_N
#endif

#ifdef VK_O
#undef VK_O
#endif

#ifdef VK_P
#undef VK_P
#endif

#ifdef VK_Q
#undef VK_Q
#endif

#ifdef VK_R
#undef VK_R
#endif

#ifdef VK_S
#undef VK_S
#endif

#ifdef VK_T
#undef VK_T
#endif

#ifdef VK_U
#undef VK_U
#endif

#ifdef VK_V
#undef VK_V
#endif

#ifdef VK_W
#undef VK_W
#endif

#ifdef VK_X
#undef VK_X
#endif

#ifdef VK_Y
#undef VK_Y
#endif

#ifdef VK_Z
#undef VK_Z
#endif

#ifndef VK_LBUTTON
#define VK_LBUTTON 0x01
#endif

#ifndef VK_RBUTTON
#define VK_RBUTTON 0x02
#endif

#ifndef VK_MBUTTON
#define VK_MBUTTON 0x04
#endif

#ifndef VK_BACK
#define VK_BACK 0x08
#endif

#ifndef VK_TAB
#define VK_TAB 0x09
#endif

#ifndef VK_RETURN
#define VK_RETURN 0x0D
#endif

#ifndef VK_SHIFT
#define VK_SHIFT 0x10
#endif

#ifndef VK_CONTROL
#define VK_CONTROL 0x11
#endif

#ifndef VK_MENU
#define VK_MENU 0x12
#endif

#ifndef VK_PAUSE
#define VK_PAUSE 0x13
#endif

#ifndef VK_ESCAPE
#define VK_ESCAPE 0x1B
#endif

#ifndef VK_SPACE
#define VK_SPACE 0x20
#endif

#ifndef VK_PRIOR
#define VK_PRIOR 0x21
#endif

#ifndef VK_NEXT
#define VK_NEXT 0x22
#endif

#ifndef VK_END
#define VK_END 0x23
#endif

#ifndef VK_HOME
#define VK_HOME 0x24
#endif

#ifndef VK_LEFT
#define VK_LEFT 0x25
#endif

#ifndef VK_UP
#define VK_UP 0x26
#endif

#ifndef VK_RIGHT
#define VK_RIGHT 0x27
#endif

#ifndef VK_DOWN
#define VK_DOWN 0x28
#endif

#ifndef VK_INSERT
#define VK_INSERT 0x2D
#endif

#ifndef VK_DELETE
#define VK_DELETE 0x2E
#endif

#ifndef VK_0
#define VK_0 0x30
#endif

#ifndef VK_1
#define VK_1 0x31
#endif

#ifndef VK_2
#define VK_2 0x32
#endif

#ifndef VK_3
#define VK_3 0x33
#endif

#ifndef VK_4
#define VK_4 0x34
#endif

#ifndef VK_5
#define VK_5 0x35
#endif

#ifndef VK_6
#define VK_6 0x36
#endif

#ifndef VK_7
#define VK_7 0x37
#endif

#ifndef VK_8
#define VK_8 0x38
#endif

#ifndef VK_9
#define VK_9 0x39
#endif

#ifndef VK_A
#define VK_A 0x41
#endif

#ifndef VK_B
#define VK_B 0x42
#endif

#ifndef VK_C
#define VK_C 0x43
#endif

#ifndef VK_D
#define VK_D 0x44
#endif

#ifndef VK_E
#define VK_E 0x45
#endif

#ifndef VK_F
#define VK_F 0x46
#endif

#ifndef VK_G
#define VK_G 0x47
#endif

#ifndef VK_H
#define VK_H 0x48
#endif

#ifndef VK_I
#define VK_I 0x49
#endif

#ifndef VK_J
#define VK_J 0x4A
#endif

#ifndef VK_K
#define VK_K 0x4B
#endif

#ifndef VK_L
#define VK_L 0x4C
#endif

#ifndef VK_M
#define VK_M 0x4D
#endif

#ifndef VK_N
#define VK_N 0x4E
#endif

#ifndef VK_O
#define VK_O 0x4F
#endif

#ifndef VK_P
#define VK_P 0x50
#endif

#ifndef VK_Q
#define VK_Q 0x51
#endif

#ifndef VK_R
#define VK_R 0x52
#endif

#ifndef VK_S
#define VK_S 0x53
#endif

#ifndef VK_T
#define VK_T 0x54
#endif

#ifndef VK_U
#define VK_U 0x55
#endif

#ifndef VK_V
#define VK_V 0x56
#endif

#ifndef VK_W
#define VK_W 0x57
#endif

#ifndef VK_X
#define VK_X 0x58
#endif

#ifndef VK_Y
#define VK_Y 0x59
#endif

#ifndef VK_Z
#define VK_Z 0x5A
#endif

SDL_Scancode VKToSDLScancode(int key)
{
    if (key >= VK_A && key <= VK_Z)
        return (SDL_Scancode)(SDL_SCANCODE_A + (key - VK_A));

    if (key >= VK_0 && key <= VK_9)
        return (SDL_Scancode)(SDL_SCANCODE_0 + (key - VK_0));

    switch (key)
    {
    case VK_SPACE:   return SDL_SCANCODE_SPACE;
    case VK_ESCAPE:  return SDL_SCANCODE_ESCAPE;
    case VK_SHIFT:   return SDL_SCANCODE_LSHIFT;
    case VK_CONTROL: return SDL_SCANCODE_LCTRL;
    case VK_MENU:    return SDL_SCANCODE_LALT;
    case VK_HOME:    return SDL_SCANCODE_HOME;
    case VK_END:     return SDL_SCANCODE_END;
    case VK_INSERT:  return SDL_SCANCODE_INSERT;
    case VK_DELETE:  return SDL_SCANCODE_DELETE;
    case VK_BACK:   return SDL_SCANCODE_BACKSPACE;
    case VK_TAB:    return SDL_SCANCODE_TAB;
    case VK_RETURN: return SDL_SCANCODE_RETURN;
    case VK_LEFT:   return SDL_SCANCODE_LEFT;
    case VK_UP:     return SDL_SCANCODE_UP;
    case VK_RIGHT:  return SDL_SCANCODE_RIGHT;
    case VK_DOWN:   return SDL_SCANCODE_DOWN;
    }

    return SDL_SCANCODE_UNKNOWN;
}

short MU_GetAsyncKeyState(int key) {

    if (MU_EditAnyFocused())
        return 0;

#ifdef __ANDROID__

    if (SceneFlag == CHARACTER_SCENE && key == VK_LBUTTON) {
        if (g_PendingTouchMove && g_PendingTouchMoveFrames == 0) {
            g_PendingTouchMove = false;
            return (short)0x8000;
        }
    }

    if (SceneFlag == MAIN_SCENE && key == VK_LBUTTON) {
        if (MouseLButtonPush) {
            return (short)0x8000;
        }
    }

#endif //MouseLButtonPush

    // Mouse buttons
    Uint32 mouse = SDL_GetMouseState(nullptr, nullptr);

    switch (key)
    {
#ifdef __ANDROID__
    case VK_LBUTTON:
        return MouseLButton ? (short)0x8000 : 0;
#else
    case VK_LBUTTON:
        return (mouse & SDL_BUTTON(SDL_BUTTON_LEFT)) ? (short)0x8000 : 0;
#endif
    case VK_RBUTTON:
        return (mouse & SDL_BUTTON(SDL_BUTTON_RIGHT)) ? (short)0x8000 : 0;

    case VK_MBUTTON:
        return (mouse & SDL_BUTTON(SDL_BUTTON_MIDDLE)) ? (short)0x8000 : 0;
    }

    // Keyboard
    const Uint8* state = SDL_GetKeyboardState(nullptr);

    //SDL_Scancode scan = SDL_GetScancodeFromKey(key);
    SDL_Scancode scan = VKToSDLScancode(key);

    if (scan == SDL_SCANCODE_UNKNOWN)
        return 0;

    return state[scan] ? (short)0x8000 : 0;
}

#ifndef _WIN32
BOOL TextOutW(HDC hdc, int nXStart, int nYStart, LPCWSTR lpString, int cbString){
    return 1;
}

bool GetTextExtentPoint32W(HDC hdc, LPCWSTR text, int len, SIZE *lpsz) {
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

#endif


