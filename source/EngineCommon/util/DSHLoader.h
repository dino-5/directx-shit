#pragma once
#include "EngineCommon/include/types.h"
#include "EngineCommon/System/Filesystem.h"
#include <vector>

struct DSH_Data
{
    std::vector<float> vertexData;
    std::vector<u32> indices;
};

DSH_Data loadDSH(system::Filepath path);