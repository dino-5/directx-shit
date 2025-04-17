#pragma once
#include <d3d12.h>
#include <vector>
#include "EngineGfx/dx12/Device.h"
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

    class UploadBuffer : public Resource
    {
    public:
        UploadBuffer(ID3D12Device* device, uint elementCount, uint sizeOfType , bool isConstantBuffer)  
        {
            init(device, elementCount, sizeOfType, isConstantBuffer);
        }

        void init(ID3D12Device* device, uint elementCount, uint typeSize, bool isConstantBuffer) 
        {
            m_IsConstantBuffer = isConstantBuffer;
            m_ElementByteSize = isConstantBuffer ? CalcConstantBufferByteSize(typeSize) : typeSize;

            ResourceDescription desc{
                    .format = DXGI_FORMAT_UNKNOWN,
                    .width = m_ElementByteSize * elementCount,
                    .height = 1,
                    .depthOrArraySize = 1,
                    .dimension = D3D12_RESOURCE_DIMENSION_BUFFER,
                    .flags = ResourceFlags::NONE,
                    .createState = ResourceState::GENERIC_READ_STATE,
                    .heapType = D3D12_HEAP_TYPE_UPLOAD,
                    .name = "Upload Buffer"
            };
			Resource::initResource(device, desc, DescriptorProperties(DescriptorFlags::None));
            ThrowIfFailed(resource()->Map(0, nullptr, reinterpret_cast<void**>(&m_MappedData)));
        }

        UploadBuffer() = default;
        UploadBuffer& operator=(const UploadBuffer& rhs) = delete;

        void CopyData(int elementIndex, const void* data)
        {
            memcpy(&m_MappedData[elementIndex*m_ElementByteSize], data, m_ElementByteSize);
        }

    public:
        BYTE* m_MappedData = nullptr;

        UINT m_ElementByteSize = 0;
        bool m_IsConstantBuffer = false;
    };

    enum class BufferType : u16
    {
        NONE,
        VERTEX,
        INDEX,
        CONSTANT,
        UPLOAD,
        CUSTOM
    };

    template<typename T>
    struct BufferDescription
    {
        const T* data = nullptr;
        u32 elementCount{};
        std::string_view name;
        ResourceState state{};
        BufferType type{};
    };

	class Buffer : public Resource
	{
	public:
		Buffer() = default;

		template<typename T>
        Buffer(GfxContext& context, T* data, uint numberOfElements, std::string_view name);

		template<typename T>
        static Buffer CreateVertexBuffer(GfxContext& context, T* data, uint numberOfElements);
		template<typename T>
        static Buffer CreateIndexBuffer(GfxContext& context, T* data, uint numberOfElements);

        template<typename T>
        void init(GfxContext& context, const BufferDescription<T>& desc);
    
		u32 getDescriptorHeapIndex()
		{
			return srv.getDescriptorIndex();
		}
        u32 getBufferSize() const { return m_bufferSize; }
        u32 getElementSize() const { return m_elementSize; }

        void copyData(const void* data, ID3D12GraphicsCommandList* commandList, ResourceState state);

	private:
        UploadBuffer m_uploadBuffer;
        uint m_elementSize;
        uint m_bufferSize;
        BufferType m_type;
	};
    #include "Buffer.hpp"

    template<typename T>
    struct BufferObject
    {
        void create(GfxContext& ctx)
        {
            assert(desc.type != BufferType::NONE);
            desc.data = data.data();
            desc.elementCount = data.size();
            buffer.init(ctx, desc);
        }
        void update(GfxContext& ctx)
        {
            buffer.copyData(data.data(), ctx.cmdList, buffer.getCurrentState());
        }

        Buffer buffer;
        std::vector<T> data;
        BufferDescription<T> desc;
    };

	class ConstantBuffer : public Resource
	{
	public:
        ConstantBuffer() = default;
        template<typename T>
		void init(GfxContext& context, T* data, u32 elementCount, bool createDescriptors = true)
		{
            m_structSize = sizeof(T);
            m_elementSize = CalcConstantBufferByteSize(m_structSize);
            m_bufferSize = m_elementSize * elementCount; 
            ResourceDescription desc{
                    .format = DXGI_FORMAT_UNKNOWN,
                    .width = m_bufferSize,
                    .height = 1,
                    .depthOrArraySize = 1,
                    .dimension = D3D12_RESOURCE_DIMENSION_BUFFER,
                    .flags = ResourceFlags::NONE,
                    .createState = ResourceState::GENERIC_READ_STATE,
                    .heapType = D3D12_HEAP_TYPE_UPLOAD,
                    .name = "Constant Buffer"
            };

            DescriptorProperties descProps{
                .descriptor = createDescriptors ? DescriptorFlags::ConstantBuffer : DescriptorFlags::None,
                .viewDimension = {},
                .bufferStride = m_bufferSize,
                .numElements = elementCount 
            };
			Resource::initResource(context.device, desc, descProps);
			CD3DX12_RANGE readRange(0, 0);       
            ThrowIfFailed(resource()->Map(0, &readRange, reinterpret_cast<void**>(&m_buffer)));

            if(sizeof(T) * elementCount == m_bufferSize)
                memcpy(m_buffer, data, sizeof(T) * elementCount);
            else
            {
                for(u32 i = 0; i < elementCount; ++i)
                    memcpy(m_buffer + i * m_elementSize, &data[i], sizeof(T));
            }

            // TODO : maybe transition to the constant state
		}

		D3D12_GPU_VIRTUAL_ADDRESS getAddress(u32 element=0) 		{
			return resource()->GetGPUVirtualAddress() + m_elementSize * element;
		}
		u32 getDescriptorHeapIndex()
		{
			return srv.getDescriptorIndex();
		}

        template<typename T>
		void update(T* data, uint elementNumber = 0)
		{
            memcpy(&m_buffer[elementNumber * m_elementSize], data, sizeof(T));
		}

	private:
		char* m_buffer=nullptr;
		uint m_bufferSize = 0;
		uint m_structSize = 0;
        uint m_elementSize = 0;
	};

};
