#pragma once
#include <string.h>
#include <errno.h>
#include <stdio.h>
#include <signal.h>
#include <cstdint>
#ifndef _WIN32
#include <netinet/in.h>
# ifdef _XOPEN_SOURCE_EXTENDED
#  include <arpa/inet.h>
# endif
#include <sys/socket.h>
#endif
#include "XLog.h"

#include <event2/bufferevent.h>
#include <event2/buffer.h>
#include <event2/util.h>
#include <event2/event.h>
class Timer
{
	std::thread m_thread;
	event_base* pEventBase;
	event* timer_event;

public:
	Timer();
	~Timer();
	//libevent 初始化
	int64_t Init();
	//创建定时器事件
	int64_t Create();
	bool Close();
	void run();
	static void timer_cb(evutil_socket_t sig, short events, void* user_data);
};

