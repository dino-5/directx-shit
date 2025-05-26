#pragma once
#include <cstring>
#include <d3d12.h>
#include <string_view>
#include <vector>
#include "EngineGfx/dx12/Device.h"
#include "EngineGfx/GfxContext.h"
#include "EngineGfx/dx12/d3dx12.h"
#include "EngineGfx/dx12/Resource.h"
#include "EngineGfx/dx12/dx12_includes.hpp"
#include "EngineCommon/System/config.h"
#include "EngineCommon/include/types.h"

#include "third_party/magic_enum/include/magic_enum.hpp"

namespace engine::graphics
{
using namespace magic_enum::bitwise_operators;
using engine::util::CalcConstantBufferByteSize;

enum class BufferType : u16
{
    NONE     = 0,
    VERTEX   = 1 << 0,
    INDEX    = 1 << 1,
    CONSTANT = 1 << 2,
    UPLOAD   = 1 << 3,
    CUSTOM   = 1 << 4
};

struct BufferDescription
{
    template<typename T>
    BufferDescription(T* aData, u32 aElementCount, std::string_view aName, ResourceState aState = ResourceState::GENERIC_READ_STATE, BufferType aType = BufferType::CUSTOM) 
    : data((void*)aData), elementCount(aElementCount),
    elementSize(sizeof(T)), name(aName), state(aState), type(aType)
    {}

    BufferDescription(u32 aSize, std::string_view aName, ResourceState aState = ResourceState::GENERIC_READ_STATE, BufferType aType = BufferType::CUSTOM) 
      : data(nullptr),
        elementCount(1),
        elementSize(aSize), 
        name(aName), 
        state(aState), 
        type(aType)
    {}

    BufferDescription(u32 aElementCount, u32 aElementSize, std::string_view aName, ResourceState aState = ResourceState::GENERIC_READ_STATE, BufferType aType = BufferType::CUSTOM) 
      : data(nullptr),
        elementCount(aElementCount),
        elementSize(aElementSize), 
        name(aName), 
        state(aState), 
        type(aType)
    {}
    BufferDescription() = default;

    const void* data = nullptr;
    u32 elementCount{};
    u32 elementSize = 0;
    std::string_view name;
    ResourceState state{};
    BufferType type{};
};

class Buffer : public Resource
{
public:
    Buffer() = default;

    template<typename T>
    Buffer(GfxContext& context, T* data, u32 elementCount, std::string_view name)
    {
        init(context, BufferDescription(data, elementCount, name));
    }

    Buffer(const GfxContext& context, const BufferDescription& desc)
    {
        init(context, desc);
    }

    static Buffer CreateIndexBuffer(GfxContext& context, u32* data, u32 elementCount)
    {
        return CreateBuffer(context, BufferDescription(data, elementCount, "IndexBuffer", ResourceState::INDEX_BUFFER, BufferType::INDEX));
    }

    template<typename T>
    static Buffer CreateVertexBuffer(GfxContext& context, T* data, u32 elementCount){
        return CreateBuffer(context, BufferDescription(data, elementCount, "VertexBuffer", ResourceState::VERTEX_CONSTANT_BUFFER, BufferType::VERTEX));
    }

    template<typename T>
    static Buffer CreateCustomBuffer(GfxContext& context, T* data, u32 elementCount){
        return CreateBuffer(context, BufferDescription(data, elementCount, "CustomBuffer", ResourceState::COMMON, BufferType::CUSTOM));
    }

    static Buffer CreateUploadBuffer(const GfxContext& context, u32 bufferSize){
        return CreateBuffer(context, BufferDescription(bufferSize, "UploadBuffer", ResourceState::COPY_SOURCE, BufferType::UPLOAD));
    }

    static Buffer CreateBuffer(const GfxContext& context, const BufferDescription& bufferDesc)
    {
        return Buffer(context, bufferDesc);
    }

    void init(const GfxContext& context, const BufferDescription& desc);

    u32 getDescriptorHeapIndex()
    {
        return srv.getDescriptorIndex();
    }
    u32 getBufferSize() const { return m_bufferSize; }
    u32 getElementSize() const { return m_elementSize; }

    void copyDataToGPU(const void* data, const GfxContext& context, ResourceState state);

protected:
    struct UploadBufferInfo
    {
        u32 size;
        bool isUsed;
    };
    using BufferHandle = u32;
    static BufferHandle GetUploadBufferHandle(const GfxContext& ctx, u32 bufferSize)
    {
        for(u32 i = 0; i < (u32)s_uploadBufferInfo.size(); ++i)
        {
            auto& buffer = s_uploadBufferInfo[i];
            if(buffer.size >= bufferSize && !buffer.isUsed)
                return i;
        }
        s_uploadBuffers.push_back(CreateUploadBuffer(ctx, bufferSize));
        s_uploadBufferInfo.push_back({bufferSize, true});
        return (u32)s_uploadBuffers.size()-1;
    }
    static void ReleaseUploadBuffer(BufferHandle handle)
    {
        s_uploadBufferInfo[handle].isUsed = false;
    }
    static Buffer& GetUploadBuffer(BufferHandle handle) 
    {
        return s_uploadBuffers[handle];
    }

    static std::vector<Buffer> s_uploadBuffers;
    static std::vector<UploadBufferInfo> s_uploadBufferInfo;
    u32 m_elementSize;
    u32 m_bufferSize;
    BufferType m_type;
};

class ConstantBuffer : public Buffer
{
public:
    template<typename T>
    ConstantBuffer(GfxContext& context, T* data, u32 elementCount) : Buffer(CreateConstantBuffer(context, elementCount, sizeof(T))) 
    {
        CD3DX12_RANGE readRange(0, 0);       
        ThrowIfFailed(resource()->Map(0, &readRange, reinterpret_cast<void**>(&m_buffer)));
        update(data);
    }
    ~ConstantBuffer() 
    {
        CD3DX12_RANGE readRange(0, 0);       
        resource()->Unmap(0, &readRange);
    }
    ConstantBuffer() = default;

    template<typename T>
    void update(T* data, u32 elementNumber = 0)
    {
        memcpy(&m_buffer[elementNumber * m_elementSize], data, sizeof(T));
    }
private:
    static Buffer CreateConstantBuffer(GfxContext& context, u32 elementCount, u32 elementSize){
        return CreateBuffer(context, BufferDescription(elementCount, elementSize, "ConstantBuffer", ResourceState::VERTEX_CONSTANT_BUFFER, BufferType::CONSTANT));
    }
    char* m_buffer = nullptr;
};

struct BufferObject
{
    template<typename T>
    void create(GfxContext& ctx, const BufferDescription& aDesc)
    {
        assert(aDesc.type != BufferType::NONE);

        data.resize(aDesc.elementSize * aDesc.elementCount);
        memcpy(aDesc.data, data.data(), aDesc.elementSize * aDesc.elementCount);

        buffer.init(ctx, aDesc);
    }
    void update(GfxContext& ctx)
    {
        buffer.copyDataToGPU(data.data(), ctx, buffer.getCurrentState());
    }

    Buffer buffer;
    std::vector<char*> data;
};

inline D3D12_INDEX_BUFFER_VIEW GetIndexBufferView(Buffer& buffer) {
    D3D12_INDEX_BUFFER_VIEW view;
    view.BufferLocation = buffer.resource()->GetGPUVirtualAddress();
    view.Format = DXGI_FORMAT_R32_UINT;
    view.SizeInBytes = buffer.getBufferSize();
    return view;
}

inline D3D12_VERTEX_BUFFER_VIEW GetVertexBufferView(Buffer& buffer) {
    D3D12_VERTEX_BUFFER_VIEW view;
    view.BufferLocation = buffer.resource()->GetGPUVirtualAddress();
    view.StrideInBytes = buffer.getElementSize();
    view.SizeInBytes = buffer.getBufferSize();
    return view;
}

};
