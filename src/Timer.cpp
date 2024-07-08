#include "Timer.h"

Timer::Timer() :base(nullptr), timer_event(nullptr) {
	Init();
}
Timer::~Timer() {
	event_base_free(base);
	if (m_thread.joinable()) {
		m_thread.join();
	}
}

int64_t Timer::Init()
{
    base = event_base_new();
    return 0;
}

int64_t Timer::Create()
{
	timeval tv = { 1,0 };
	timer_event = event_new(base, -1, EV_PERSIST, timer_cb, base);

	if (!timer_event || event_add(timer_event, &tv) < 0) {
		LOG(ERROR)("Could not create/add a signal event!");
		return 1;
	}
	LOG(INFO)("Add a timer event success!");
	return 0;
}

bool Timer::Close()
{
	if (timer_event != nullptr) {
		event_free(timer_event);
		LOG(INFO)(" a timer event removed");
	}
	return true;
}

void Timer::run()
{
	//由于event_base_dispatch是一个loop循环，所以要创建线程来保证本方法继续执行
	m_thread = std::thread(event_base_dispatch, base);
	LOG(INFO)("Timer events start Looping!");
	return;
}

void Timer::timer_cb(evutil_socket_t sig, short events, void* user_data)
{
	LOG(INFO)("Timer reach.\n");

}
