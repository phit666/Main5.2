#pragma once

#ifdef _WIN32

  #include <winsock2.h>
  #include <ws2tcpip.h>
#else

#include "win_compat.h"
#include <stdint.h>

using SOCKET = int;
#define INVALID_SOCKET (-1)
#define SOCKET_ERROR   (-1)

#ifndef AF_INET
#define AF_INET 2
#endif
#ifndef PF_INET
#define PF_INET AF_INET
#endif
#define SOCK_STREAM 1
#define SOCK_DGRAM  2
#define IPPROTO_TCP 6
#define IPPROTO_UDP 17
#define SD_RECEIVE 0
#define SD_SEND    1
#define SD_BOTH    2

#define WSAEWOULDBLOCK 10035
#define WSAEINPROGRESS 10036
#define WSAENOTSOCK    10038
#define WSAECONNRESET  10054
#define WSAECONNREFUSED 10061

#define FD_READ    0x01
#define FD_WRITE   0x02
#define FD_OOB     0x04
#define FD_ACCEPT  0x08
#define FD_CONNECT 0x10
#define FD_CLOSE   0x20

#define WSAGETSELECTEVENT(lParam) LOWORD(lParam)
#define WSAGETSELECTERROR(lParam) HIWORD(lParam)
#define WSA_MAKE_SELECT_LPARAM(event,error) MAKELPARAM(event,error)

struct WSADATA {
    WORD wVersion;
    WORD wHighVersion;
    char szDescription[257];
    char szSystemStatus[129];
};

int WSAStartup(WORD requested, WSADATA* data);
int WSACleanup();
int WSAGetLastError();
void WSASetLastError(int e);
int closesocket(SOCKET s);
int ioctlsocket(SOCKET s, long cmd, unsigned long* argp);
int WSAAsyncSelect(SOCKET s, HWND hwnd, UINT msg, long events);

// You must call this once after creating your libevent base.
struct event_base;
void WinCompat_SetEventBase(event_base* base);
void WinCompat_RemoveSocket(SOCKET s);

#ifndef FIONBIO
#define FIONBIO 0x8004667e
#endif

#endif
