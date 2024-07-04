#include "Reactor.h"

XReactor::XReactor():conf(nullptr),base(nullptr), listener(nullptr), signal_event(nullptr) {
	Init();
}
XReactor::~XReactor() {

}

int64_t XReactor::Init() {
	LOG(INFO)("start initialize libevent!");
#ifdef _WIN32
	WSADATA wsa_data;
	WSAStartup(0x0201, &wsa_data);
#endif
	//忽略管道信号，防止发送数据给已关闭的socket导致程序dump
	if (signal(SIGPIPE, SIG_IGN) == SIG_ERR) {
		return 1;
	}
	//创建配置上下文
	conf = event_config_new();

	//显示支持的网络模式
	const char** methods=event_get_supported_methods();
	LOG(INFO)("XReactor support methods:!");
	for (int i = 0; methods[i] != NULL; i++) {
		LOG(INFO)("({}): {}",i,methods[i]);
	}
	//设置特征
	event_config_require_features(conf, EV_FEATURE_FDS);
	//设置网络模型
	event_config_avoid_method(conf, "epoll");
    //配置IOCP
	event_config_set_flag(conf, EVENT_BASE_FLAG_STARTUP_IOCP);
	evthread_use
	
	base = event_base_new_with_config();
	if (!base) {
		LOG(ERROR)("Could not initialize libevent!");
		return 1;
	}
	return 0;
}
int64_t XReactor::connectCreate(int port) {
	sin.sin_family = AF_INET;
	sin.sin_port = htons(port);

	listener = evconnlistener_new_bind(base, listener_cb, (void*)base,
		LEV_OPT_REUSEABLE | LEV_OPT_CLOSE_ON_FREE, -1,
		(sockaddr*)&sin,
		sizeof(sin));

	if (!listener) {
		LOG(ERROR)("Could not create a listener!");
		return 1;
	}

	signal_event = evsignal_new(base, SIGINT, signal_cb, (void*)base);

	if (!signal_event || event_add(signal_event, NULL) < 0) {
		LOG(ERROR)("Could not create/add a signal event!");
		return 1;
	}
	event_base_dispatch(base);
	return 0;
}
bool XReactor::connectClose(int handle) {
	evconnlistener_free(listener);
	event_free(signal_event);
	event_base_free(base);
}

void XReactor::listener_cb(evconnlistener* listener, evutil_socket_t fd,
	struct sockaddr* sa, int socklen, void* user_data)
{
	struct event_base* base = static_cast<event_base*>(user_data);
	struct bufferevent* bev;

	bev = bufferevent_socket_new(base, fd, BEV_OPT_CLOSE_ON_FREE);
	if (!bev) {
		LOG(ERROR)("Error constructing bufferevent!");
		event_base_loopbreak(base);
		return;
	}
	bufferevent_setcb(bev, NULL, conn_writecb, conn_eventcb, NULL);
	bufferevent_enable(bev, EV_WRITE);
	bufferevent_disable(bev, EV_READ);
	std::string message= "I get the data!";
	bufferevent_write(bev, message.c_str(), message.size());
}

void XReactor::conn_writecb(bufferevent* bev, void* user_data)
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

void XReactor::signal_cb(evutil_socket_t sig, short events, void* user_data)
{
	struct event_base* base = static_cast<event_base*>(user_data);
	struct timeval delay = { 2, 0 };

	LOG(INFO)("Caught an interrupt signal; exiting cleanly in two seconds.\n");

	event_base_loopexit(base, &delay);
}
