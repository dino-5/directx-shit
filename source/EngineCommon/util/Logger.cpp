#pragma once
#include "EngineCommon/util/Logger.h"

namespace engine::util
{
	std::vector<std::string> log_info;

	void logInfo(std::string info)
	{
		std::cout << std::string(log_info.size(), '\t') << ' ' << info << std::endl;
	}
};
