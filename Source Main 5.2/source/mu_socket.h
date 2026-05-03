#pragma once
#define MU_USE_LIBEVENT 0

#if MU_USE_LIBEVENT == 1
#include <event2/bufferevent.h>
#include <event2/buffer.h>
#include <event2/listener.h>
#include <event2/util.h>
#include <event2/event.h>
#include <event2/thread.h>
#include <vector>
#include <string>
#include <mutex>
#include <algorithm>
#include <functional>
#include <random>

#define MAX_BUFFER_DATA 8192
#define MAX_EPOLL_BUFFER_DATA (MAX_BUFFER_DATA * 4)
#define MAX_USER_PACKET_DATA (MAX_BUFFER_DATA * 10)

struct _PACKET_DATA
{
	void clear()
	{
		bev = NULL;
		bufferlen = 0;
		memset(buffer, 0, MAX_EPOLL_BUFFER_DATA);
		isconnected = false;
	}
	char buffer[MAX_EPOLL_BUFFER_DATA];
	int bufferlen;
	struct bufferevent* bev;
	bool isconnected;
};

int le_start(void* p);
int le_connect(char* serverip, unsigned short port);
int DataSend(unsigned char* pMsg, int len);
void parsedata();
void eventbreak();
void disconnect();
void le_alive();
void le_close();

extern std::recursive_mutex gMutex;
extern _PACKET_DATA gcon;
#endif


