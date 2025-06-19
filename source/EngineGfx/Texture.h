#ifndef TEXTURE_H
#define TEXTURE_H

#include "EngineCommon/include/types.h"
#include "EngineCommon/system/Filesystem.h"
#include "EngineCommon/util/Logger.h"
#include "EngineGfx/dx12/Resource.h"
#include "EngineGfx/dx12/d3dx12.h"
#include "EngineGfx/GfxContext.h"
#include "EngineGfx/dx12/Device.h"
#include <d3d12.h>
#include <string>
#include <unordered_map>

using namespace engine;

namespace engine::graphics 
{
class DescriptorHeap;
struct TextureDescription
{
    TextureDescription(const TextureDescription& imageData) = default;
    TextureDescription(TextureDescription& imageData)
    {
        data = imageData.data;
        width = imageData.width;
        height = imageData.height;
        channels = imageData.channels;
        name = imageData.name;
        imageData.data = nullptr;
        needToDestruct = imageData.needToDestruct;
    }
    ~TextureDescription()
    {
        if (needToDestruct && data)
            delete[] data;
    }
    void setData(void* newData) { data = reinterpret_cast<u8*>(newData); }

    TextureDescription() = default;
    TextureDescription(system::Filepath path) { init(path); }
    TextureDescription(int w, int h, int chn) :
    width(w), height(h), channels(chn), data(nullptr)
    {}

    void init(engine::system::Filepath path);

    int width;
    int height;
    int channels;
    u8* data;
    const char* name = nullptr;
    bool needToDestruct = false;
};


class Texture : public Resource
{
public:
    Texture() = default;
    Texture (TextureDescription imData,
             const GfxContext& ctx,
             DescriptorFlags flags = DescriptorFlags::ShaderResource,
             ResourceState state = ResourceState::PIXEL_SHADER_RESOURCE);
    u32 getDescriptorHeapIndex()
    {
        return srv.getDescriptorIndex();
    }

public:
    Resource textureUploadHeap;
    uint width = 0;
    uint height = 0;
};

};
#endif
