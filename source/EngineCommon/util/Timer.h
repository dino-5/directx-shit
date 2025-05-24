#pragma once

#include <chrono>
#include <ctime>
#include <ratio>
#include <string_view>
#include <unordered_map>
#include <algorithm>
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
    static double getMicroseconds(TimeType t1, TimeType t2) { return (double)std::chrono::duration_cast<std::chrono::microseconds>(t1 - t2).count(); }
    TimeType m_start;
    TimeType m_lastCheck;
    bool m_isFps = false;
};

class Profiler
{
public:
    struct ProfilerNode
    {
        std::string name;
        double duration = -1;
        u32 count = 0;
        u32 parrentIndex = 0;
        u32 recursionDepth = 0;
        std::vector<std::string> names;
        std::vector<u32> indices;
        void print()
        {
            util::printInfo("[{}] : total duration {}, called {} times, mean duration {}", name, duration, count, duration / count);
        }
    };


    static void StartProfiling() 
    {
        s_profilerStarted = true;
    }

    static void EndProfiling() 
    {
        s_profilerStarted = false;
        PrintReport();
        Clear();
    }

    static void Clear() 
    {
        s_currentCallstack.clear(); 
        s_currentNodeIndex = 0;
    }

    static void Push(const std::string& name)
    {
        if(!s_profilerStarted)
            return;
        if(s_currentCallstack.size() )
        {
            auto& data = s_currentCallstack[s_currentNodeIndex];
            if(data.name== name)
            {
                data.recursionDepth++;
                return;
            }
            i32 index = 0;
            for(auto& n : data.names)
                if(n == name)
                    break;
                else
                    index++;

            if (index != data.names.size())
                s_currentNodeIndex = data.indices[index];
            else
            {
                data.names.push_back(name);
                data.indices.push_back((u32)s_currentCallstack.size());
                s_currentCallstack.push_back(ProfilerNode(name, 0, 0, s_currentNodeIndex));
                s_currentNodeIndex = (u32)s_currentCallstack.size() - 1;
            }
        }
        else 
            s_currentCallstack.push_back(ProfilerNode(name, 0, 0, s_currentNodeIndex));
    }

    static void Pop(double ms)
    {
        if(!s_profilerStarted)
            return;
        auto& data = s_currentCallstack[s_currentNodeIndex];
        data.duration += ms;
        data.count++;
        if(data.recursionDepth==0)
            s_currentNodeIndex  = data.parrentIndex;
        else
            data.recursionDepth--;
    }

    static void PrintReport()
    {
        auto& node = s_currentCallstack[0];
        node.print();
        log_info++;
        u32 currentIndex = 0;
        std::vector<bool> printedNodes(s_currentCallstack.size(), false);

        if(node.indices.size())
        {
            printedNodes[currentIndex] = true;
            currentIndex = node.indices[0];
        }
        while(1)
        {
            if(!currentIndex)
                for(u32 index = 0; index < printedNodes.size(); index++)
                    if(!printedNodes[index])
                    {
                        log_info++;
                        currentIndex = index;
                    }
            if(!currentIndex)
                break;
            node = s_currentCallstack[currentIndex];
            if(!printedNodes[currentIndex])
            {
                node.print();
                printedNodes[currentIndex] = true;
            }
            u32 newIndex = 0;
            for (auto& index : node.indices)
            {
                if(!printedNodes[index])
                {
                    newIndex = index;
                    currentIndex = newIndex;
                    log_info++;
                    break;
                }
            }
            // if there is no child elements or we visited all our children go to parent
            if(!newIndex)
            {
                currentIndex = node.parrentIndex;
                log_info--;
            }
        }

    }

private:
    static inline std::vector<ProfilerNode> s_currentCallstack;
    static inline u32 s_currentNodeIndex = 0;
    static inline bool s_profilerStarted;
};

class ProfilerTimer : public Timer
{
public:
    ProfilerTimer(std::string_view name, bool fps = false):
        Timer(fps), m_name(name)
    {
        if (!g_state.profilingEnabled)
            return;
        Profiler::Push(m_name);
    }

    ~ProfilerTimer()
    {
        if (!g_state.profilingEnabled)
            return;
        double time_elapsed = Timer::getMiliseconds(clock::now(), m_start);
        Profiler::Pop(time_elapsed);
    }
private:
    std::string m_name;
};

};

#define TIMER(name) engine::util::Timer _(#name)
#define PROFILER(name) engine::util::ProfilerTimer _(#name)
