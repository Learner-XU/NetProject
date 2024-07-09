#include "SignalManager.h"

SignalManager::SignalManager():pEventBase(nullptr), signal_event(nullptr)
{
	Init();
}

SignalManager::~SignalManager()
{
	event_base_free(pEventBase);
	if (m_thread.joinable()) {
		m_thread.join();
	}
}

int64_t SignalManager::Init()
{
	pEventBase = event_base_new();
	return 0;
}

int64_t SignalManager::Create()
{
	//持久事件
	//signal_event = evsignal_new(base, SIGINT, signal_cb, (void*)base);
	//非持久事件 只进入一次
	signal_event = event_new(pEventBase, SIGTERM, EV_SIGNAL, signal_cb, event_self_cbarg());
	if (!signal_event || event_add(signal_event, NULL) < 0) {
		LOG(ERROR)("Could not create/add a signal event!");
		return 1;
	}
	LOG(INFO)("Add a signal event success!");
	return 0;
}

bool SignalManager::Close()
{
	if (signal_event != nullptr) {
		event_free(signal_event);
		LOG(INFO)(" a signal event removed");
	}
	return true;
}

void SignalManager::run()
{
	//由于event_base_dispatch是一个loop循环，所以要创建线程来保证本方法继续执行
	m_thread = std::thread(event_base_dispatch, pEventBase);
	LOG(INFO)("signal events start Looping!");
	return;
}

void SignalManager::signal_cb(evutil_socket_t sig, short events, void* user_data)
{
	timeval delay = { 2, 0 };

	LOG(INFO)("Caught an kill signal; exiting cleanly in two seconds.\n");

	event_base_loopexit(static_cast<event_base*>(user_data), &delay);
}
