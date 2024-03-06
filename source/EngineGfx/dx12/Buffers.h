#pragma once
#include <d3d12.h>
#include "EngineGfx/RenderContext.h"
#include "EngineGfx/dx12/d3dx12.h"
#include "EngineGfx/dx12/Resource.h"
#include "EngineCommon/System/config.h"
#include "EngineCommon/include/types.h"
#include "EngineCommon/include/common.h"

namespace engine::graphics
{
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
                    .descriptor = DescriptorFlags::None
            };
			Resource::init(device, desc);
            ThrowIfFailed(resource()->Map(0, nullptr, reinterpret_cast<void**>(&m_MappedData)));
        }

        UploadBuffer() = default;
        UploadBuffer& operator=(const UploadBuffer& rhs) = delete;
        ~UploadBuffer()
        {
            resource()->Unmap(0, nullptr);

            m_MappedData = nullptr;
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

	class Buffer : public Resource
	{
	public:
		Buffer() = default;
		template<typename T>
		void Init(RenderContext& context, T* data, uint bufferSize, u32 bufferStride, u32 numElements)
		{
            ID3D12Device* device = context.GetDevice().GetDevice();
            ID3D12GraphicsCommandList* commandList = context.GetList().GetList();
            ResourceDescription desc{
                    .format = DXGI_FORMAT_UNKNOWN,
                    .width = bufferSize,
                    .height = 1,
                    .depthOrArraySize = 1,
                    .dimension = D3D12_RESOURCE_DIMENSION_BUFFER,
                    .flags = ResourceFlags::NONE,
                    .createState = ResourceState::COMMON,
                    .heapType = D3D12_HEAP_TYPE_DEFAULT,
                    .descriptor = {
                        .descriptor = DescriptorFlags::ShaderResource,
                        .viewDimension = D3D12_SRV_DIMENSION_BUFFER,
                        .bufferStride = bufferStride,
                        .numElements = numElements
                    }
            };
            Resource::init(device, desc);

            UploadBuffer buffer;
            buffer.Init(device, 1, bufferSize, false);
            D3D12_SUBRESOURCE_DATA subresData = {};
            subresData.pData = data;
            subresData.RowPitch = bufferSize;
            subresData.SlicePitch = 1;
            transition(commandList, ResourceState::COPY_DEST);
            UpdateSubresources(commandList, resource(), buffer.resource(), 0, 0, 1, &subresData);
            transition(commandList, ResourceState::GENERIC_READ_STATE);

		}
		u32 GetDescriptorHeapIndex()
		{
			return srv.getDescriptorIndex();
		}
	private:
	};

	class ConstantBuffer : public Resource
	{
	public:
        ConstantBuffer() = default;
		void Init(ID3D12Device* device, uint numberOfElements, const void* data, u32 size) 
		{
            m_structSize = CalcConstantBufferByteSize(size);
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
                    .descriptor = DescriptorFlags::ConstantBuffer
            };
			Resource::init(device, desc);
			CD3DX12_RANGE readRange(0, 0);       
            ThrowIfFailed(resource()->Map(0, &readRange, reinterpret_cast<void**>(&m_buffer)));
            Update(data);
		}

		D3D12_GPU_VIRTUAL_ADDRESS getAddress(u32 element=0) 		{
			return resource()->GetGPUVirtualAddress() + m_structSize * element;
		}
		u32 getDescriptorHeapIndex()
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