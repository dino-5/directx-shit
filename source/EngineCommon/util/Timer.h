#pragma once

#include <chrono>
#include <ctime>
#include <ratio>
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
    Timer(bool fps = false):
        m_start(clock::now()),
        m_lastCheck(m_start),
        m_isFps(fps) {}

    double Tick()
    {
        auto now = clock::now();
        duration time_elapsed = now - m_lastCheck;
        m_lastCheck = now;
        return time_elapsed.count();
    }

    void saveCurrentTime()
    {
        auto now = clock::now();
        m_lastCheck = now;
    }

    double getElapsedTime() const
    {
        auto time = clock::now();
        return getMiliseconds(time, m_lastCheck);
    }

    double getFps() const {
        return 1e6 / getMicroseconds(clock::now(), m_lastCheck);
    }

protected:
    using TimeType = decltype(clock::now());
    static double getMiliseconds(TimeType t1, TimeType t2) { return getMicroseconds(t1, t2) / 1000.0; }
    static double getMicroseconds(TimeType t1, TimeType t2) { return std::chrono::duration_cast<std::chrono::microseconds>(t1 - t2).count(); }
    TimeType m_start;
    TimeType m_lastCheck;
    bool m_isFps = false;
};

class Profiler : public Timer
{
public:
    Profiler(std::string_view name, bool fps = false):
        Timer(fps), m_name(name)
    {
        if (!g_state.profilingEnabled)
            return;
        util::printInfo("{} started", name);
        log_info++;
    }

    void Tick(std::string_view name)
    {
        double time = Timer::Tick();
        if (!g_state.profilingEnabled)
            return;
        util::printInfo("{} was {} miliseconds", name, time);
    }

    ~Profiler()
    {
        if (!g_state.profilingEnabled)
            return;
        log_info--;
        double time_elapsed = Timer::getMiliseconds(clock::now(), m_start);
        if(m_isFps)
            util::printInfo("fps for {} is {}", m_name, 1.0/time_elapsed);
        else 
        {

            if(time_elapsed > 1000)
                util::printInfo("{} was {} seconds", m_name, time_elapsed/1000);
            else
                util::printInfo("{} was {} miliseconds", m_name, time_elapsed);
        }
    }
private:
    std::string m_name;
};

};

#define TIMER(name) engine::util::Timer _(#name)
#define PROFILER(name) engine::util::Profiler _(#name)
