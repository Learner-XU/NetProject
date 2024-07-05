#pragma once
#include <memory>
#include <string>
#include "spdlog/spdlog.h"
#include "spdlog/async.h"
#include "spdlog/sinks/daily_file_sink.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/common.h"

#define LOG(level) LOG_##level
#define __FILENAME__ (strchr(__FILE__,'/')?(strrchr(__FILE__,'/')+1):__FILE__)


class XLog
{
public:
	enum Level
	{
		nTrace = 0,
		nDebug,
		nInfo,
		nWarn,
		nError,
		nCritical
	};
	enum State {
		eNone = 0,
		eInit
	};
	~XLog();
	static XLog& GetInst() {
		if (m_pInster == nullptr) {
			m_pInster = new XLog();
		}
		return *m_pInster;
	}
    template <typename... Args> 
	int Log(int nLevel, std::string sFormat, const Args &... args)
	{
		if (m_state == eNone)
		{
			printf("Log not init! file:%s, line:%d\n", __FILENAME__, __LINE__);
			return -1;
		}
		//spdlog::memory_buf_t buf;
		//fmt::format_to(buf, sFormat, args...);
		//printf("CLog::Log,to print: %s\n", spdlog::string_view_t(buf.data(), buf.size()).data());
		switch (nLevel)
		{
		case nTrace:
			m_pLogger->trace(sFormat, args...);
			break;
		case nDebug:
			m_pLogger->debug(sFormat, args...);
			break;
		case nInfo:
			m_pLogger->info(sFormat, args...);
			break;
		case nWarn:
			m_pLogger->warn(sFormat, args...);
			break;
		case nError:
			m_pLogger->error(sFormat, args...);
			break;
		case nCritical:
			m_pLogger->critical(sFormat, args...);
			break;

		}
		return 0;
	}
	
	void Init(const std::string& sAppName);
	void Destory() {
		if (m_pInster != nullptr) {
			delete m_pInster;
		}
		m_pInster=nullptr;
	}

private:
	
	XLog(){}
	static XLog* m_pInster;
	State m_state = eNone;
	std::string s_LogPath="./log/";
	std::shared_ptr<spdlog::async_logger> m_pLogger = nullptr;
};

#define LOG_TRACE(fmt,args...) XLog::GetInst().Log(XLog::nTrace,"[{}:{}] {} " fmt ,__FILENAME__ ,__LINE__,__FUNCTION__,##args );
#define LOG_DEBUG(fmt,args...) XLog::GetInst().Log(XLog::nDebug,"[{}:{}] {} " fmt ,__FILENAME__ ,__LINE__,__FUNCTION__,##args );
#define LOG_INFO(fmt,args...)  XLog::GetInst().Log(XLog::nInfo,"[{}:{}] {} " fmt ,__FILENAME__ ,__LINE__,__FUNCTION__,##args );
#define LOG_WARN(fmt,args...)  XLog::GetInst().Log(XLog::nWarn,"[{}:{}] {} " fmt ,__FILENAME__ ,__LINE__,__FUNCTION__,##args );
#define LOG_ERROR(fmt,args...) XLog::GetInst().Log(XLog::nError,"[{}:{}] {} " fmt ,__FILENAME__ ,__LINE__,__FUNCTION__,##args );
#define LOG_CRITICAL(fmt,args...) XLog::GetInst().Log(XLog::nCritical,"[{}:{}] {} " fmt ,__FILENAME__ ,__LINE__,__FUNCTION__,##args );
