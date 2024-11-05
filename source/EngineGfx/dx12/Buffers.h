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

    private:
        void copyData(const void* data, ID3D12Device* device, ID3D12GraphicsCommandList* commandList, ResourceState state);

	private:
        UploadBuffer m_uploadBuffer;
        uint m_elementSize;
        uint m_bufferSize;
        BufferType m_type;
	};

    inline void Buffer::copyData(const void* data, ID3D12Device* device, ID3D12GraphicsCommandList* commandList, ResourceState state)
    {
        m_uploadBuffer.init(device, 1, m_bufferSize, false);

        D3D12_SUBRESOURCE_DATA subresData = {};
        subresData.pData = data;
        subresData.RowPitch = m_bufferSize;
        subresData.SlicePitch = 1;

        transition(commandList, ResourceState::COPY_DEST);
        UpdateSubresources(commandList, resource(), m_uploadBuffer.resource(), 0, 0, 1, &subresData);
        transition(commandList, state);
    }

    template<typename T>
    void Buffer::init(GfxContext & context, const BufferDescription<T>& bufferDesc)
    {
        m_type = bufferDesc.type;
        m_elementSize = sizeof(T);
        m_bufferSize = bufferDesc.elementCount* m_elementSize;
        ResourceDescription desc{
                .format = DXGI_FORMAT_UNKNOWN,
                .width = m_bufferSize,
                .height = 1,
                .depthOrArraySize = 1,
                .dimension = D3D12_RESOURCE_DIMENSION_BUFFER,
                .flags = ResourceFlags::NONE,
                .createState = ResourceState::COMMON,
                .heapType = D3D12_HEAP_TYPE_DEFAULT,
                .name = bufferDesc.name.data()
        };

        // TODO remake this thing
        DescriptorProperties descriptorProps = {
            .descriptor = DescriptorFlags::ShaderResource,
            .viewDimension = D3D12_SRV_DIMENSION_BUFFER,
            .bufferStride = sizeof(T),
            .numElements = bufferDesc.elementCount
        };
        Resource::initResource(context.device, desc, descriptorProps);

        if(bufferDesc.data)
            copyData(bufferDesc.data, context.device, context.cmdList, bufferDesc.state);
    }

    template<typename T>
    Buffer::Buffer(GfxContext& context, T* data, uint numberOfElements, std::string_view name)
    {
        m_type = BufferType::CUSTOM;
        init(context, data, numberOfElements, name);
    }

    template<typename T>
    Buffer Buffer::CreateVertexBuffer(GfxContext& context, T* data, uint numberOfElements)
    {
        Buffer buffer;
        buffer.init(context, data, numberOfElements, "VertexBuffer", ResourceState::VERTEX_CONSTANT_BUFFER, BufferType::VERTEX);
        return buffer;
    }

    template<typename T>
    Buffer Buffer::CreateIndexBuffer(GfxContext& context, T* data, uint numberOfElements)
    {
        Buffer buffer;
        buffer.init(context, data, numberOfElements, "IndexBuffer", ResourceState::INDEX_BUFFER, BufferType::INDEX);
        return buffer;
    }

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
    
	class ConstantBuffer : public Resource
	{
	public:
        ConstantBuffer() = default;
        template<typename T>
		void init(GfxContext& context, T* data, u32 elementCount)
		{
            m_structSize = sizeof(T);
            m_bufferSize = CalcConstantBufferByteSize(m_structSize * elementCount); 
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
                .descriptor = DescriptorFlags::ConstantBuffer,
                .viewDimension = D3D12_SRV_DIMENSION_BUFFER,
                .bufferStride = m_bufferSize,
                .numElements = elementCount 
            };
			Resource::initResource(context.device, desc, descProps);
			CD3DX12_RANGE readRange(0, 0);       
            ThrowIfFailed(resource()->Map(0, &readRange, reinterpret_cast<void**>(&m_buffer)));
            memcpy(m_buffer, data, sizeof(T) * elementCount);

            // TODO : maybe transition to the constant state
		}

		D3D12_GPU_VIRTUAL_ADDRESS getAddress(u32 element=0) 		{
			return resource()->GetGPUVirtualAddress() + m_structSize * element;
		}
		u32 getDescriptorHeapIndex()
		{
			return srv.getDescriptorIndex();
		}

        template<typename T>
		void update(T* data, uint elementNumber = 0)
		{
            memcpy(&m_buffer[elementNumber * m_structSize], data, sizeof(T));
		}

	private:
		char* m_buffer=nullptr;
		uint m_bufferSize = 0;
		uint m_structSize = 0;
	};

};