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
	//����event��ʼLoop
	void run();
private:
	int nPort = 1994;
	//������
	bufferevent* pBufEv;
	std::thread m_thread;
	event_config* pConf;
	event_base* pEventBase;
	evconnlistener* pListener;
	sockaddr_in sin = { 0 };


public:
	//libevent ��ʼ��
	int64_t Init();
	//����IO�¼�
	int64_t Create();
	//���ӷ�����
	int64_t connect(int port);
	bool Close(int handle);
	static void listener_cb(evconnlistener*, evutil_socket_t,sockaddr*, int socklen, void*);
	static void conn_writecb(bufferevent* bev, void* user_data);
	static void conn_eventcb(bufferevent*, short, void*);
	static void read_callback(bufferevent* pBufEv, void* pArg);
	static void write_callback(bufferevent* pBufEv, short sEvent,void* pArg);
	static void event_callback(bufferevent* pBufEv, short sEvent, void* pArg);
};