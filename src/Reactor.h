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
#include <event2/listener.h>
#include <event2/util.h>
#include <event2/event.h>


class XReactor {
public:
	XReactor();
	~XReactor();
private:
	event_config* conf;
	event_base* base;
	evconnlistener* listener;
	event* signal_event;
	sockaddr_in sin = { 0 };


public:
	int64_t Init();
	int64_t connectCreate(int port);
	bool connectClose(int handle);

	static void listener_cb(evconnlistener*, evutil_socket_t,sockaddr*, int socklen, void*);
	static void conn_writecb(bufferevent*, void*);
	static void conn_eventcb(bufferevent*, short, void*);
	static void signal_cb(evutil_socket_t, short, void*);
};