#pragma once
#include<iostream>
#include <format>
#include <string>
#include <vector>

namespace engine::util
{
    struct LoggerState
    {
         #define LoggerVariable(varName) bool varName = true;\
         void set##varName(bool val) { varName = val;}
         LoggerVariable(infoEnabled)
         LoggerVariable(errorsEnabled)
    };
    extern LoggerState g_loggerState;

    // ANSI escape code for red text
    constexpr const char* redColor = "\033[1;31m";
    // ANSI escape code for resetting text color
    constexpr const char* resetColor = "\033[0m";
    struct ColorSetter
    {
        ColorSetter(const char* setColor)
        {
            std::cout<<setColor;
        }
        ~ColorSetter()
        {
            std::cout << resetColor;
        }
    };

	extern std::vector<std::string> log_info;

    template<typename... Args>
    void PrintInfo(std::format_string<Args...> fmt, Args&&... args)
    {
        if(g_loggerState.infoEnabled)
            std::cout<< std::string(log_info.size(), '\t') <<
                     std::format(fmt, std::forward<Args>(args)...) << '\n';
    }
    template<typename... Args>
    void PrintError(std::format_string<Args...> fmt, Args&&... args)
    {
        if (g_loggerState.errorsEnabled)
        {
            ColorSetter _(redColor);
            std::cout << std::string(log_info.size(), '\t') <<
                      std::format(fmt, std::forward<Args>(args)...) << '\n';
        }
    }

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

 