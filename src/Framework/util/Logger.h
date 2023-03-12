#pragma once
#include<iostream>
#include <format>
#include <string>
#include <vector>

namespace engine::util
{
	extern std::vector<std::string> log_info;

	void logInfo(std::string info);

	struct ScopeInfo
	{
		ScopeInfo(std::string n): name(n)
		{
			std::cout << std::string( log_info.size(), '\t')<< ' ' << name << " started\n";
			log_info.push_back(name);
		}
		~ScopeInfo()
		{
			log_info.pop_back();
			std::cout << std::string( log_info.size(), '\t') << ' ' << name << " ended\n";
		}
		std::string name;
	};
};

 