#pragma once

#ifdef _WIN32

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <windows.h>

//windows
#include <WinSock2.h>
#include <mmsystem.h>
#include <shellapi.h>
#include <process.h>
#include <ws2tcpip.h>
#include <tchar.h>
#ifndef MU_USE_SDL
#include <Wininet.h>
#endif
#include <crtdbg.h>
#include <strsafe.h>

#else
#include "Compats/win_compat.h"
#endif
#include "mu_wininet_curl_compat.h"

