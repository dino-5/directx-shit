#pragma once

#include <chrono>
#include <ctime>
#include <string_view>
#include "EngineCommon/util/Logger.h"
#include "EngineCommon/System/config.h"

namespace engine::util {

    using clock = std::chrono::system_clock;
    using duration = std::chrono::duration<double>;
    using namespace config;


    class Timer
    {
    public:
        Timer(std::string_view name, bool fps = false):
            m_start(clock::now()),
            m_lastCheck(m_start),
            m_name ( name),
            m_isFps(fps)
        {
            if (!g_state.profilingEnabled)
                return;
            util::printInfo("{} started", name);
            log_info++;
        }

        void Tick(std::string_view name)
        {
            if (!g_state.profilingEnabled)
                return;
            auto now = clock::now();
            duration time_elapsed = now - m_lastCheck;
            m_lastCheck = now;
            util::printInfo("{} was {} miliseconds", name, time_elapsed.count());
        }

        void saveCurrentTime()
        {
            auto now = clock::now();
            m_lastCheck = now;
        }

        double getElapsedTime() const
        {
            auto now = clock::now();
            duration time_elapsed = now - m_lastCheck;
            return time_elapsed.count(); 
        }

        double getFps() const {
            return 1 / getElapsedTime();
        }

        ~Timer()
        {
            if (!g_state.profilingEnabled)
                return;
            log_info--;
            duration time_elapsed = clock::now() - m_start;
            if(m_isFps)
                util::printInfo("fps for {} is {}", m_name, 1.0/time_elapsed.count());
            else
                util::printInfo("{} was {} miliseconds", m_name, time_elapsed.count());
        }
    private:
        decltype(clock::now()) m_start;
        decltype(clock::now()) m_lastCheck;
        std::string m_name;
        bool m_isFps = false;
    };
};

#define TIMER(name) engine::util::Timer _(GET_PATH#name)
