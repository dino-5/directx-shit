#pragma once
#include "EngineCommon/util/Logger.h"

namespace engine::util
{
	extern u32 log_info = 0;
    LoggerState g_loggerState;

	void logInfo(std::string info)
	{
		std::cout << std::string(log_info, '\t') << ' ' << info << '\n';
	}
};
