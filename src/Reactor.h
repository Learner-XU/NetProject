#pragma once
#include <string.h>
#include <vector>
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
	//所有event开始Loop
	void run();
private:
	//v_message加锁
	std::mutex m_mtx;
	int nPort = 1994;
	//客户端缓冲区
	std::vector<bufferevent*> p_ClientBufEv;
	//服务端缓冲区
	std::vector<bufferevent*> p_ServerBufEv;
	std::thread m_thread;
	event_config* pConf;
	event_base* pEventBase;
	evconnlistener* pListener;
	sockaddr_in sin = { 0 };
	std::vector<std::string> v_message;
	int count = 0;

public:
	//libevent 初始化
	int64_t Init();
	//创建IO事件
	int64_t CreateServer();
	//连接服务器
	int64_t connectServer(int port);
	virtual void GetData(const char* pBody,int nLen) {
		std::string recv(pBody);
		LOG(INFO)("Client {},recv:{}", (void*)pEventBase, recv);
	};
	virtual void writeData() {
		std::unique_lock<std::mutex> mtx(m_mtx);
		count++;
		//v_message.emplace_back("send count " + std::to_string(count)+"\n");
		std::string item = "send count " + std::to_string(count) + "\n";
		for (auto& pBufEv : p_ServerBufEv) {
			if (pBufEv) {
				bufferevent_write(pBufEv, item.c_str(), item.size());
			}
		}
		
	};
	void SendData();
	bool Close();
	static void listener_cb(evconnlistener* pListener, evutil_socket_t fd, sockaddr* sa, int socklen, void* user_data);
	static void conn_writecb(bufferevent* bev, void* user_data);
	static void conn_eventcb(bufferevent*, short, void*);
	static void read_callback(bufferevent* pBufEv, void* pArg);
	static void write_callback(bufferevent* pBufEv, short sEvent,void* pArg);
	static void event_callback(bufferevent* pBufEv, short sEvent, void* pArg);
};