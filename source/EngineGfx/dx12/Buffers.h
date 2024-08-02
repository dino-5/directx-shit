#pragma once
#include <d3d12.h>
#include "EngineGfx/RenderContext.h"
#include "EngineGfx/dx12/d3dx12.h"
#include "EngineGfx/dx12/Resource.h"
#include "EngineCommon/System/config.h"
#include "EngineCommon/include/types.h"
#include "EngineCommon/include/common.h"

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
            Init(device, elementCount, sizeOfType, isConstantBuffer);
        }

        void Init(ID3D12Device* device, uint elementCount, uint typeSize, bool isConstantBuffer) 
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
			Resource::InitResource(device, desc, DescriptorProperties(DescriptorFlags::None));
            ThrowIfFailed(resource()->Map(0, nullptr, reinterpret_cast<void**>(&m_MappedData)));
        }

        UploadBuffer() = default;
        UploadBuffer& operator=(const UploadBuffer& rhs) = delete;
        ~UploadBuffer()
        {
            //if(auto res= resource())
            //    res->Unmap(0, nullptr);
            //m_MappedData = nullptr;
        }

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

	class Buffer : public Resource
	{
	public:
		Buffer() = default;

		template<typename T>
        Buffer(RenderContext& context, T* data, uint numberOfElements, std::string_view name);

		template<typename T>
        static Buffer CreateVertexBuffer(RenderContext& context, T* data, uint numberOfElements);

		template<typename T>
        void InitAsVertexBuffer(RenderContext& context, T* data, uint numberOfElements);

		template<typename T>
        static Buffer CreateConstantBuffer(RenderContext& context, T* data, uint numberOfElements);

		template<typename T>
        void InitAsConstantBuffer(RenderContext& context, T* data, uint numberOfElements);

		template<typename T>
        static Buffer CreateIndexBuffer(RenderContext& context, T* data, uint numberOfElements);

		template<typename T>
        void InitAsIndexBuffer(RenderContext& context, T* data, uint numberOfElements);
    
		u32 GetDescriptorHeapIndex()
		{
			return srv.getDescriptorIndex();
		}
        u32 GetBufferSize() const { return m_bufferSize; }
        u32 GetElementSize() const { return m_elementSize; }

    private:
        template<typename T>
        void Init(RenderContext& context, T* data, uint numberOfElements, std::string_view name);
        void CopyData(void* data, ID3D12Device* device, ID3D12GraphicsCommandList* commandList, ResourceState state);

	private:
        UploadBuffer m_uploadBuffer;
        uint m_elementSize;
        uint m_bufferSize;
        BufferType m_type;
	};

    inline void Buffer::CopyData(void* data, ID3D12Device* device, ID3D12GraphicsCommandList* commandList, ResourceState state)
    {
        m_uploadBuffer.Init(device, 1, m_bufferSize, false);

        D3D12_SUBRESOURCE_DATA subresData = {};
        subresData.pData = data;
        subresData.RowPitch = m_bufferSize;
        subresData.SlicePitch = 1;

        Transition(commandList, ResourceState::COPY_DEST);
        UpdateSubresources(commandList, resource(), m_uploadBuffer.resource(), 0, 0, 1, &subresData);
        Transition(commandList, state);
    }

    template<typename T>
    void Buffer::Init(RenderContext & context, T * data, uint numberOfElements, std::string_view name)
    {
        ID3D12Device* device = context.GetDevice().GetDevice();
        ID3D12GraphicsCommandList* commandList = context.GetList().GetList();
        ResourceDescription desc{
                .format = DXGI_FORMAT_UNKNOWN,
                .width = m_bufferSize,
                .height = 1,
                .depthOrArraySize = 1,
                .dimension = D3D12_RESOURCE_DIMENSION_BUFFER,
                .flags = ResourceFlags::NONE,
                .createState = ResourceState::COMMON,
                .heapType = D3D12_HEAP_TYPE_DEFAULT,
                .name = name.data()
        };

        // TODO remake this thing
        DescriptorProperties descriptorProps = {
            .descriptor = DescriptorFlags::ShaderResource,
            .viewDimension = D3D12_SRV_DIMENSION_BUFFER,
            .bufferStride = sizeof(T),
            .numElements = numberOfElements
        };
        Resource::InitResource(device, desc, descriptorProps);

    }

    template<typename T>
    Buffer::Buffer(RenderContext& context, T* data, uint numberOfElements, std::string_view name)
    {
        m_type = BufferType::CUSTOM;
        Init(context, data, numberOfElements, name);
    }

    template<typename T>
    Buffer Buffer::CreateVertexBuffer(RenderContext& context, T* data, uint numberOfElements)
    {
        Buffer buffer;
        buffer.InitAsVertexBuffer(context, data, numberOfElements);
        return buffer;
    }

    template<typename T>
    void Buffer::InitAsVertexBuffer(RenderContext& context, T* data, uint numberOfElements)
    {
        m_elementSize = sizeof(T);
        m_bufferSize = numberOfElements * m_elementSize;

        m_type = BufferType::VERTEX;
        ID3D12Device* device = context.GetDevice().GetDevice();
        ID3D12GraphicsCommandList* commandList = context.GetList().GetList();

        Init(context, data, numberOfElements, "VertexBuffer");

        CopyData(data, device, commandList, ResourceState::VERTEX_CONSTANT_BUFFER);
    }

    template<typename T>
    Buffer Buffer::CreateConstantBuffer(RenderContext& context, T* data, uint numberOfElements)
    {
        Buffer buffer;
        buffer.InitAsVertexBuffer(context, data, numberOfElements);
        return buffer;
    }

    template<typename T>
    void Buffer::InitAsConstantBuffer(RenderContext& context, T* data, uint numberOfElements)
    {
        m_elementSize = CalcConstantBufferByteSize(sizeof(T)); // do we need make single entry of buffer be
            // divided by 256 or all all buffer?        
        m_bufferSize = numberOfElements * m_elementSize;

        m_type = BufferType::CONSTANT;
        ID3D12Device* device = context.GetDevice().GetDevice();
        ID3D12GraphicsCommandList* commandList = context.GetList().GetList();

        Init(context, data, numberOfElements, "ConstantBuffer");
        CopyData(data, device, commandList, ResourceState::VERTEX_CONSTANT_BUFFER);
    }

    template<typename T>
    Buffer Buffer::CreateIndexBuffer(RenderContext& context, T* data, uint numberOfElements)
    {
        InitAsVertexBuffer(context, data, numberOfElements);
    }

    template<typename T>
    void Buffer::InitAsIndexBuffer(RenderContext& context, T* data, uint numberOfElements)
    {
        m_elementSize = sizeof(T);
        m_bufferSize = numberOfElements * m_elementSize;

        m_type = BufferType::INDEX;
        ID3D12Device* device = context.GetDevice().GetDevice();
        ID3D12GraphicsCommandList* commandList = context.GetList().GetList();

        Init(context, data, numberOfElements, "IndexBuffer");
        CopyData(data, device, commandList, ResourceState::INDEX_BUFFER);
    }


    inline D3D12_INDEX_BUFFER_VIEW GetIndexBufferView(Buffer& buffer) {
        D3D12_INDEX_BUFFER_VIEW view;
        view.BufferLocation = buffer.resource()->GetGPUVirtualAddress();
        view.Format = DXGI_FORMAT_R32_UINT;
        view.SizeInBytes = buffer.GetBufferSize();
        return view;
    }

    inline D3D12_VERTEX_BUFFER_VIEW GetVertexBufferView(Buffer& buffer) {
        D3D12_VERTEX_BUFFER_VIEW view;
        view.BufferLocation = buffer.resource()->GetGPUVirtualAddress();
        view.StrideInBytes = buffer.GetElementSize();
        view.SizeInBytes = buffer.GetBufferSize();
        return view;
    }
    
	class ConstantBuffer : public Resource
	{
	public:
        ConstantBuffer() = default;
		template<typename T>
		void Init(RenderContext& context, T* data, uint numberOfElements)
		{
            ID3D12Device* device = context.GetDevice().GetDevice();
            m_structSize =  CalcConstantBufferByteSize(sizeof(T)); // do we need make single entry of buffer be
            // divided by 256 or all all buffer?  
			uint bufferSize = m_structSize*numberOfElements;
            ResourceDescription desc{
                    .format = DXGI_FORMAT_UNKNOWN,
                    .width = bufferSize,
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
                .bufferStride = bufferSize,
                .numElements = numberOfElements 
            };
			Resource::InitResource(device, desc, descProps);
			CD3DX12_RANGE readRange(0, 0);       
            ThrowIfFailed(resource()->Map(0, &readRange, reinterpret_cast<void**>(&m_buffer)));
            Update(data);
		}

		D3D12_GPU_VIRTUAL_ADDRESS getAddress(u32 element=0) 		{
			return resource()->GetGPUVirtualAddress() + m_structSize * element;
		}
		u32 GetDescriptorHeapIndex()
		{
			return srv.getDescriptorIndex();
		}

		void Update(const void* data,uint numberOfElement = 0)
		{
			memcpy(&m_buffer[numberOfElement * m_structSize], data, m_structSize);
		}
	private:
		char* m_buffer=nullptr;
		uint m_structSize;
	};

};