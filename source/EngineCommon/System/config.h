#pragma once 
#include "EngineCommon/include/types.h"
#include "EngineCommon/System/Filesystem.h"

namespace engine::config
{
	const uint NumFrames = 2;

    using namespace system;
    struct GlobalState
    {
        Filepath homeDir;
        Filepath demoDir;
        Filepath shaderDir;
    };

    extern GlobalState g_state;
};