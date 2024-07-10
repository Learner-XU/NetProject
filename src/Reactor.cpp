#include "Reactor.h"

XReactor::XReactor():pConf(nullptr),pEventBase(nullptr), pListener(nullptr) {
	Init();
}
XReactor::~XReactor() {
	event_base_free(pEventBase);
	if (m_thread.joinable()) {
		m_thread.join();
	}
}

int64_t XReactor::Init() {
	if (pEventBase!=nullptr) {
		LOG(INFO)("libevent has init!");
		if (pBufEv != nullptr) {
			LOG(INFO)("bufferevent has init!");
		}
		else {
			pBufEv= bufferevent_socket_new(pEventBase, -1, 0);
			LOG(INFO)("bufferevent is init!");
		} 
		
		return 0;
	}
	
	//���Թܵ��źţ���ֹ�������ݸ��ѹرյ�socket���³���dump
	if (signal(SIGPIPE, SIG_IGN) == SIG_ERR) {
		return 1;
	}
	//��������������
	pConf = event_config_new();

	//��������
	event_config_require_features(pConf, EV_FEATURE_FDS);
	//��������ģ��
	event_config_avoid_method(pConf, "epoll");
    //����IOCP
	//event_config_set_flag(conf, EVENT_BASE_FLAG_STARTUP_IOCP);
	
	pEventBase = event_base_new_with_config(pConf);
	if (pEventBase==nullptr) {
		LOG(ERROR)("Could not initialize libevent!");
		return 1;
	}
	else {
		LOG(INFO)("libevent is init!");
	}
	pBufEv = bufferevent_socket_new(pEventBase, -1, 0);
	if (pBufEv == nullptr) {
		LOG(ERROR)("Could not initialize bufferevent!");
		return 1;
	}
	else {
		LOG(INFO)("bufferevent is init!");
	}
	LOG(INFO)("Reactor is init!");
	return 0;
}
int64_t XReactor::Create() {
	if (pListener!=nullptr) {
		evconnlistener_free(pListener);
		LOG(INFO)("last listener is free!");
	}
	sin.sin_family = AF_INET;
	sin.sin_port = htons(nPort);

	pListener = evconnlistener_new_bind(pEventBase, listener_cb, (void*)pEventBase,
		LEV_OPT_REUSEABLE | LEV_OPT_CLOSE_ON_FREE, -1,
		(sockaddr*)&sin,
		sizeof(sin));

	if (!pListener) {
		LOG(ERROR)("Could not create a listener!");
		return 1;
	}
	LOG(INFO)("Create a listener success!");
	return 0;
}

void XReactor::run() {
	//����event_base_dispatch��һ��loopѭ��������Ҫ�����߳�����֤����������ִ��
	m_thread = std::thread(event_base_dispatch, pEventBase);
	LOG(INFO)("All events start Looping!");
	return;
}


int64_t XReactor::connect(int port)
{
	//���ûص�����, ���ص������Ĳ���
	bufferevent_setcb(pBufEv, read_callback, conn_writecb, event_callback, NULL);
	memset(&sin, 0, sizeof(sin));
	sin.sin_family = AF_INET;
	sin.sin_addr.s_addr = inet_addr("172.17.201.213");
	sin.sin_port = htons(nPort);

	//���ӷ�����
	if (bufferevent_socket_connect(pBufEv, (struct sockaddr*)&sin, sizeof(sin)) ==-1) {
		LOG(ERROR)("connect server failed!");
		return -1;
	}
	LOG(INFO)("connect server success!");
	return 0;
}

bool XReactor::Close(int handle) {
	if (pListener != nullptr) {
		evconnlistener_free(pListener);
		LOG(INFO)(" a listener removed");
	}
	return true;
	
	
}


void XReactor::listener_cb(evconnlistener* pListener, evutil_socket_t fd,
	struct sockaddr* sa, int socklen, void* user_data)
{
	struct event_base* pEventBase = static_cast<event_base*>(user_data);
	struct bufferevent* bev;

	bev = bufferevent_socket_new(pEventBase, fd, BEV_OPT_CLOSE_ON_FREE);
	if (!bev) {
		LOG(ERROR)("Error constructing bufferevent!");
		event_base_loopbreak(pEventBase);
		return;
	}
	bufferevent_setcb(bev, NULL, conn_writecb, conn_eventcb, NULL);
	bufferevent_enable(bev, EV_WRITE);
	bufferevent_disable(bev, EV_READ);
	std::string message= "I get the data!";
	bufferevent_write(bev, message.c_str(), message.size());
	LOG(INFO)("Send message:{}\n",message);
}

void XReactor::conn_writecb(bufferevent* bev,  void* user_data)
{
	evbuffer* output = bufferevent_get_output(bev);
	if (evbuffer_get_length(output) == 0) {
		LOG(INFO)("flushed answer\n");
		bufferevent_free(bev);
	}
}

void XReactor::conn_eventcb(bufferevent* bev, short events, void* user_data)
{
	if (events & BEV_EVENT_EOF) {
		LOG(INFO)("Connection closed.\n");
	}
	else if (events & BEV_EVENT_ERROR) {
		LOG(ERROR)("Got an error on the connection: {}",
			strerror(errno));/*XXX win32*/
	}
	/* None of the other events can happen here, since we haven't enabled
	 * timeouts */
	bufferevent_free(bev);
}

//���ص�����
void XReactor::read_callback(bufferevent* pBufEv, void* pArg) {
	//��ȡ���뻺��
	evbuffer* pInput = bufferevent_get_input(pBufEv);
	//��ȡ���뻺�����ݵĳ���
	int nLen = evbuffer_get_length(pInput);
	//��ȡ���ݵĵ�ַ
	const char* pBody = (const char*)evbuffer_pullup(pInput, nLen);
	//�������ݴ���
	std::string recv(pBody);
	LOG(INFO)("Client recv:{}", pBody);
	//д���������,��bufferevent�Ŀ�д�¼���ȡ��ͨ��fd����
	//bufferevent_write(pBufEv, pResponse, nResLen);
	return;
}

//д�ص�����
void XReactor::write_callback(bufferevent* pBufEv,short sEvent, void* pArg) {
	return;
}

//�¼��ص�����
void XReactor::event_callback(bufferevent* pBufEv, short sEvent, void* pArg) {
	//�ɹ�����֪ͨ�¼�
	if (BEV_EVENT_CONNECTED == sEvent) {
		bufferevent_enable(pBufEv, EV_READ);
	}
	return;
}