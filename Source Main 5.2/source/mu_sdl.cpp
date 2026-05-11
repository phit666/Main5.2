#include "stdafx.h"
#define NK_IMPLEMENTATION
#ifdef _WIN32
#include "w_nuklear.h"
#else
#include "nuklear.h"
#endif
#define NK_SDL_GLES2_IMPLEMENTATION
#include "nuklear_sdl_gles2.h"

#include "mu_sdl.h"
#include "WSclient.h"
#include "wsctlc.h"
#include "wsctlc_addon.h"
#include "mu_gles2_matrix.h"
#include "wt.h"

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
    }
    else if (events & BEV_EVENT_ERROR)
    {
        bufferevent_free(g_bev);
        g_bev = NULL;
        g_ErrorReport.Write("> conn_eventcb - BEV_EVENT_ERROR");
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



bool MU_InitSDL(int width, int height)
{
    g_ErrorReport.Write( "MU_InitSDL > SDL_Init\r\n");

     if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_EVENTS) != 0)
        return false;

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

    g_ErrorReport.Write( "MU_InitSDL > SDL_CreateWindow\r\n");

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

#ifdef _WIN32
    if (!gladLoadGLES2Loader((GLADloadproc)SDL_GL_GetProcAddress))
    {
        //OutputDebugStringA("[SDL-DEBUG] gladLoadGLES2Loader failed.");
        return false;
    }
#endif

    SDL_GL_SetSwapInterval(1);
    SDL_StartTextInput();

    InitShader();

    g_ErrorReport.Write( "MU_InitSDL > nk_sdl_init\r\n");

    g_nk_ctx = nk_sdl_init(gSDLWindow);

    struct nk_font_atlas* atlas;
    nk_sdl_font_stash_begin(&atlas);
    nk_sdl_font_stash_end();

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

    //OutputDebugStringA("[SDL-DEBUG] MU_ShutdownSDL");
}


#ifndef _WIN32
// A wrapper to mimic Win32 GetAsyncKeyState using SDL2
short GetAsyncKeyState_SDL(int vKey) {
    // Ensure the SDL event queue is up to date before checking state
    SDL_PumpEvents();

    // 1. Handle Mouse Buttons (VK_LBUTTON, VK_RBUTTON, etc.)
    if (vKey == VK_LBUTTON || vKey == VK_RBUTTON || vKey == VK_MBUTTON) {
        Uint32 mouseState = SDL_GetMouseState(NULL, NULL);
        bool isDown = false;

        if (vKey == VK_LBUTTON) isDown = (mouseState & SDL_BUTTON(SDL_BUTTON_LEFT));
        else if (vKey == VK_RBUTTON) isDown = (mouseState & SDL_BUTTON(SDL_BUTTON_RIGHT));
        else if (vKey == VK_MBUTTON) isDown = (mouseState & SDL_BUTTON(SDL_BUTTON_MIDDLE));

        return isDown ? (short)0x8000 : 0;
    }

    // 2. Handle Keyboard Keys
    const Uint8* keyboardState = SDL_GetKeyboardState(NULL);

    // Map Win32 Virtual Key codes to SDL Scancodes
    SDL_Scancode scancode = SDL_SCANCODE_UNKNOWN;

    switch (vKey) {
    case VK_SHIFT:   scancode = SDL_SCANCODE_LSHIFT; break; // Or RSHIFT
    case VK_CONTROL: scancode = SDL_SCANCODE_LCTRL; break;
    case VK_MENU:    scancode = SDL_SCANCODE_LALT; break;
    case VK_ESCAPE:  scancode = SDL_SCANCODE_ESCAPE; break;
    case VK_SPACE:   scancode = SDL_SCANCODE_SPACE; break;
    case VK_RETURN:  scancode = SDL_SCANCODE_RETURN; break;
        // Add other VK mappings as needed for your project...
    default:
        // Generic mapping for alphanumeric keys (rough estimation)
        if (vKey >= 'A' && vKey <= 'Z')
            scancode = (SDL_Scancode)(SDL_SCANCODE_A + (vKey - 'A'));
        else if (vKey >= '0' && vKey <= '9')
            scancode = (SDL_Scancode)(SDL_SCANCODE_1 + (vKey - '1'));
        break;
    }

    if (scancode != SDL_SCANCODE_UNKNOWN && keyboardState[scancode]) {
        return (short)0x8000;
    }

    return 0;
}

BOOL TextOutW(HDC hdc, int nXStart, int nYStart, LPCWSTR lpString, int cbString){
    if (nk_begin(g_nk_ctx, "Canvas", nk_rect(0, 0, WindowWidth, WindowHeight), NK_WINDOW_NO_SCROLLBAR)) {
        struct nk_command_buffer *canvas = nk_window_get_canvas(g_nk_ctx);
        //struct nk_user_font *font = g_nk_ctx->style.font;
        struct nk_rect text_rect = nk_rect(nXStart, nYStart, 200, 20);
        nk_draw_text(canvas, text_rect, "Hello", 5, g_nk_ctx->style.font,
                     nk_rgb(255,255,255), nk_rgba(0,0,0,0));
    }
    nk_end(g_nk_ctx);
    return 1;
}

bool GetTextExtentPoint32W(HDC hdc, LPCWSTR text, int len, SIZE *lpsz) {
    if (!g_nk_ctx || !text || !lpsz) return false;

    const struct nk_user_font *font = g_nk_ctx->style.font; // Currently selected font

    // Calculate width using Nuklear's internal font callback
    lpsz->cx = (long)font->width(font->userdata, font->height, reinterpret_cast<const char *>(text), len);

    // Height is constant for the font in Nuklear
    lpsz->cy = (long)font->height;

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


