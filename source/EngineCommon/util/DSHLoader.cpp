#include "DSHLoader.h"
#include "EngineCommon/util/Logger.h"
#include <fstream>
#include <sstream>

bool isNumber(char ch)
{
    return ch == '0' || ch == '1' || ch == '2' || ch == '3' || ch == '4' || 
        ch == '5' || ch == '6' || ch == '7' || ch == '8' || ch == '9';
}

DSH_Data loadDSH(system::Filepath path)
{
    std::string data = path.readFile();
    std::istringstream stream(data);
    
    // 9 index is start of number of elements in vertex
    DSH_Data result;

    stream.seekg(6);

    stream >> result.color[0];
    stream >> result.color[1];
    stream >> result.color[2];

    u32 verticesStart = data.find("vertices");
    stream.seekg(verticesStart + 9);
    u32 dim;
    stream >> dim;
    if (dim != 3)
        engine::util::printError("failed to get correct number of dimensions, got {}", dim);

    u32 index = stream.tellg();
    while (true)
        if (isNumber(data[++index]))
            break;
    stream.seekg(index);

    u32 indicesStart = data.find("indices");
    u32 vertexStop = indicesStart;
    while (true)
        if (isNumber(data[--vertexStop]))
            break;

    while (stream.tellg() < vertexStop)
    {
        float value;
        stream >> value;
        u32 index = stream.tellg();
        result.vertexData.push_back(value);
    }

    stream.seekg(indicesStart + 8);

    while (!stream.eof())
    {
        u32 value;
        u32 index = stream.tellg();
        stream >> value;
        index = stream.tellg();
        result.indices.push_back(value);
    }

    return result;
}
