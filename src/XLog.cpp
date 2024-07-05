#include "XLog.h"

XLog* XLog::m_pInster =nullptr;

XLog::~XLog()
{
	if (m_pLogger != nullptr)
	{
		m_pLogger->flush_on(spdlog::level::trace);
	}
	spdlog::shutdown();
	m_state = eNone;
}

void XLog::Init(const std::string& sAppName)
{
	try
	{
		spdlog::init_thread_pool(2, 1);
		spdlog::set_pattern("[%Y-%m-%d %H:%M:%S][%L][%t] %v");
		std::string sFileName = s_LogPath + sAppName + ".log";
		printf("Log file path: %s\n", sFileName.c_str());
		auto pFileSink = std::make_shared<spdlog::sinks::daily_file_sink_mt>(sFileName, 0, 0);
		auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();

		spdlog::sinks_init_list ls{ pFileSink ,console_sink };

		m_pLogger = std::make_shared<spdlog::async_logger>("log", ls, spdlog::thread_pool());

		spdlog::flush_every(std::chrono::milliseconds(100));
		m_state = eInit;
	}
	catch (const spdlog::spdlog_ex& e)
	{
		printf("logger init fail! %s\n", e.what());
	}
}

