#pragma once
#include <iostream>
#include <format>
#include <string>
#include <vector>
#include <source_location>
#include "Util.h"

namespace engine::util
{
    struct LoggerState
    {
         #define LoggerVariable(varName) bool varName = true;\
         void set##varName(bool val) { varName = val;}
         LoggerVariable(infoEnabled)
         LoggerVariable(errorsEnabled)
    };
    // TODO move this to the one global state object
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
#define SET_RED_COLOR() ColorSetter _(redColor)

	extern std::vector<std::string> log_info;

    template<typename... Args>
    void printInfo(std::format_string<Args...> fmt, Args&&... args)
    {
        if(g_loggerState.infoEnabled)
            std::cout<< std::string(log_info.size(), '\t') <<
                     std::format(fmt, std::forward<Args>(args)...) << '\n';
    }
    template<typename... Args>
    void printError(std::format_string<Args...> fmt, Args&&... args)
    {
        if (g_loggerState.errorsEnabled)
        {
            SET_RED_COLOR();
            std::cout << std::string(log_info.size(), '\t') <<
                      std::format(fmt, std::forward<Args>(args)...) << '\n';
        }
    }


	struct ScopeInfo
	{
		ScopeInfo(const std::string_view& name = "", const std::source_location& location = std::source_location::current()) :
            m_name(GetFormattedPath(location, name))
		{
			std::cout << std::string( log_info.size(), '\t')<< ' ' << m_name << " started\n";
			log_info.push_back(m_name);
		}
		~ScopeInfo()
		{
			log_info.pop_back();
			std::cout << std::string( log_info.size(), '\t') << ' ' << m_name << " ended\n";
		}
		std::string m_name;
	};
#define LogScope(name) engine::util::ScopeInfo _objectScope(name)
};

 