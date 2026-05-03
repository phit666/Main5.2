#include "stdafx.h"

#ifndef _WIN32
#include "winsock_compat.h"
#include <event2/event.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <unordered_map>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

static thread_local int g_wsa_error = 0;
static event_base* g_base = nullptr;

struct AsyncSelectEntry {
    SOCKET s = INVALID_SOCKET;
    HWND hwnd = nullptr;
    UINT msg = 0;
    long requested = 0;
    event* ev = nullptr;
};
static std::unordered_map<SOCKET, AsyncSelectEntry*> g_entries;

extern void WinCompat_DispatchMessage(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

static int map_errno(int e) {
    switch (e) {
        case EWOULDBLOCK:
#if EAGAIN != EWOULDBLOCK
        case EAGAIN:
#endif
            return WSAEWOULDBLOCK;
        case ECONNRESET: return WSAECONNRESET;
        case ECONNREFUSED: return WSAECONNREFUSED;
        case EINPROGRESS: return WSAEINPROGRESS;
        case EBADF: return WSAENOTSOCK;
        default: return e;
    }
}

void WSASetLastError(int e) { g_wsa_error = e; }
int WSAGetLastError() { return g_wsa_error; }

int WSAStartup(WORD, WSADATA* data) {
    if (data) {
        memset(data,0,sizeof(*data));
        data->wVersion = 0x0202;
        data->wHighVersion = 0x0202;
        strcpy(data->szDescription, "MU WinSock compat/libevent");
        strcpy(data->szSystemStatus, "Running");
    }
    return 0;
}
int WSACleanup() { return 0; }

int closesocket(SOCKET s) {
    WinCompat_RemoveSocket(s);
    int r = close(s);
    if (r < 0) { g_wsa_error = map_errno(errno); return SOCKET_ERROR; }
    return 0;
}

int ioctlsocket(SOCKET s, long cmd, unsigned long* argp) {
    if (cmd != FIONBIO || !argp) { g_wsa_error = EINVAL; return SOCKET_ERROR; }
    int flags = fcntl(s, F_GETFL, 0);
    if (flags < 0) { g_wsa_error = map_errno(errno); return SOCKET_ERROR; }
    flags = *argp ? (flags | O_NONBLOCK) : (flags & ~O_NONBLOCK);
    if (fcntl(s, F_SETFL, flags) < 0) { g_wsa_error = map_errno(errno); return SOCKET_ERROR; }
    return 0;
}

void WinCompat_SetEventBase(event_base* base) { g_base = base; }

static void async_cb(evutil_socket_t fd, short ev, void* arg) {
    auto* e = static_cast<AsyncSelectEntry*>(arg);
    if (!e) return;
    if ((ev & EV_READ) && (e->requested & FD_READ))
        WinCompat_DispatchMessage(e->hwnd, e->msg, (WPARAM)fd, WSA_MAKE_SELECT_LPARAM(FD_READ, 0));
    if ((ev & EV_WRITE) && (e->requested & FD_CONNECT))
        WinCompat_DispatchMessage(e->hwnd, e->msg, (WPARAM)fd, WSA_MAKE_SELECT_LPARAM(FD_CONNECT, 0));
    if ((ev & EV_WRITE) && (e->requested & FD_WRITE))
        WinCompat_DispatchMessage(e->hwnd, e->msg, (WPARAM)fd, WSA_MAKE_SELECT_LPARAM(FD_WRITE, 0));
}

int WSAAsyncSelect(SOCKET s, HWND hwnd, UINT msg, long events) {
    if (!g_base) { g_wsa_error = EINVAL; return SOCKET_ERROR; }
    unsigned long nb = 1;
    if (ioctlsocket(s, FIONBIO, &nb) == SOCKET_ERROR) return SOCKET_ERROR;
    WinCompat_RemoveSocket(s);
    short ev = EV_PERSIST;
    if (events & (FD_READ | FD_CLOSE)) ev |= EV_READ;
    if (events & (FD_WRITE | FD_CONNECT)) ev |= EV_WRITE;
    auto* ent = new AsyncSelectEntry;
    ent->s=s; ent->hwnd=hwnd; ent->msg=msg; ent->requested=events;
    ent->ev = event_new(g_base, s, ev, async_cb, ent);
    if (!ent->ev || event_add(ent->ev, nullptr) != 0) {
        if (ent->ev) event_free(ent->ev); delete ent; g_wsa_error = EINVAL; return SOCKET_ERROR;
    }
    g_entries[s] = ent;
    return 0;
}

void WinCompat_RemoveSocket(SOCKET s) {
    auto it = g_entries.find(s);
    if (it == g_entries.end()) return;
    if (it->second->ev) { event_del(it->second->ev); event_free(it->second->ev); }
    delete it->second;
    g_entries.erase(it);
}

#endif
