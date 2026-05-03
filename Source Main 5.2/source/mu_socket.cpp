#include "stdafx.h"
#if MU_USE_LIBEVENT == 1
#include "mu_socket.h"
#include <SDL.h>

static void conn_readcb(struct bufferevent*, void*);
static void conn_eventcb(struct bufferevent*, short, void*);
static void le_timercb(evutil_socket_t, short, void*);

static struct event_base* base;

std::recursive_mutex gMutex;
_PACKET_DATA gcon;

int le_start(void* p)
{
	struct timeval tv;
	struct event* timeout;

#ifdef _WIN32
	unsigned short wVersionRequested;
	WSADATA wsaData;
	wVersionRequested = MAKEWORD(2, 2);
	WSAStartup(wVersionRequested, &wsaData);

	evthread_use_windows_threads();
	event_config* pConfig = event_config_new();
	event_config_set_flag(pConfig, EVENT_BASE_FLAG_STARTUP_IOCP);
	event_config_set_num_cpus_hint(pConfig, 4);
	base = event_base_new_with_config(pConfig);
	event_config_free(pConfig);
#else
	evthread_use_pthreads();
	base = event_base_new();
#endif

	if (!base)
	{
		return -1;
	}

	gcon.clear();

	timeout = event_new(base, -1, EV_PERSIST, le_timercb, NULL);
	evutil_timerclear(&tv);
	tv.tv_sec = 1;
	event_add(timeout, &tv);

	event_base_dispatch(base);

	if (gcon.bev)
		bufferevent_free(gcon.bev);

	if (timeout != NULL) {
		event_del(timeout);
		event_free(timeout);
	}

	event_base_free(base);

	return 0;
}

unsigned int tick = 0;

static void le_timercb(evutil_socket_t fd, short event, void* arg)
{
	if ((SDL_GetTicks() - tick) > 5000) {
		tick = SDL_GetTicks();
	}
}

void le_alive()
{
	if ((SDL_GetTicks() - tick) > 5000) {
		tick = SDL_GetTicks();
	}
}

void disconnect()
{
	bufferevent_free(gcon.bev);
	gcon.bev = NULL;
}

void eventbreak()
{
	event_base_loopbreak(base);
}

static void
conn_readcb(struct bufferevent* bev, void* user_data)
{
	size_t len;

	gMutex.lock();

	len = bufferevent_read(bev, (char*)gcon.buffer + gcon.bufferlen,
		MAX_EPOLL_BUFFER_DATA - gcon.bufferlen);

	if (len <= 0)
	{
		gMutex.unlock();
		return;
	}

	gcon.bufferlen += len;

	parsedata();

	gMutex.unlock();

}

void parsedata()
{
	int n = 0, len = 0;

	while (true) {


		//if (!protocol(head, (unsigned char*)gcon.buffer)) {
			bufferevent_free(gcon.bev);
			gcon.bev = NULL;
			break;
		//}

		gcon.bufferlen -= len;

		if (gcon.bufferlen <= 0) {
			gcon.bufferlen = 0;
			break;
		}

		memcpy(&gcon.buffer[0], gcon.buffer + len, gcon.bufferlen);

	}

}

static void
conn_eventcb(struct bufferevent* bev, short events, void* user_data)
{
	if (events & BEV_EVENT_EOF)
	{
		bufferevent_free(gcon.bev);
		gcon.isconnected = false;
		gcon.bev = NULL;
	}
	else if (events & BEV_EVENT_ERROR)
	{
		bufferevent_free(gcon.bev);
		gcon.isconnected = false;
		gcon.bev = NULL;
	}
	else if (events & BEV_EVENT_CONNECTED)
	{
	}
}

DWORD host2ip(const char* hostname)
{
	struct hostent* h = gethostbyname(hostname);
	return (h != NULL) ? ntohl(*(DWORD*)h->h_addr) : 0;
}

int le_connect(char* serverip, unsigned short port)
{
	struct bufferevent* bev;
	struct sockaddr_in remote_address;
	int result;

	if (!base)
		return -1;

	bev = bufferevent_socket_new(base, -1,
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

	gcon.bev = bev;

	bufferevent_setcb(bev, conn_readcb, NULL, conn_eventcb, NULL);
	bufferevent_enable(bev, EV_WRITE);
	bufferevent_enable(bev, EV_READ);

	gcon.isconnected = true;
	return 1;
}

int DataSend(unsigned char* pMsg, int len) // only function here that is being invoked in main thread, rest are invoked in 2nd thread
{
	if (gcon.bev == NULL)
		return 0;

	gMutex.lock();

	if (bufferevent_write(gcon.bev, pMsg, len) == 0) {
		gMutex.unlock();
		return 1;
	}

	gMutex.unlock();
	return 0;
}

static void defer_free_cb(evutil_socket_t, short, void* arg)
{
	intptr_t aIndex = (intptr_t)arg;

	gMutex.lock();

	if (gcon.bev != NULL)
	{
		bufferevent_free(gcon.bev);
		gcon.bev = NULL;
	}

	gMutex.unlock();
}

void le_close()
{
	if (gcon.bev == NULL)
		return;
	int arg = 0;
	timeval tv = { 0, 0 };
	bufferevent_disable(gcon.bev, EV_WRITE || EV_READ);
	bufferevent_setcb(gcon.bev, nullptr, nullptr, nullptr, nullptr);

	evutil_socket_t fd = bufferevent_getfd(gcon.bev);

	if (fd != EVUTIL_INVALID_SOCKET) {
		shutdown(fd, SD_BOTH);
	}

	event_base_once(base, -1, EV_TIMEOUT, defer_free_cb, (LPVOID)static_cast<int>(arg), &tv);
}
#endif