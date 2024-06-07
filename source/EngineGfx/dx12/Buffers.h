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
                    //.descriptor = DescriptorFlags::None,
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

	class Buffer : public Resource
	{
	public:
		Buffer() = default;
		template<typename T>
		void Init(RenderContext& context, T* data, uint numberOfElements)
		{
            ID3D12Device* device = context.GetDevice().GetDevice();
            ID3D12GraphicsCommandList* commandList = context.GetList().GetList();
            u32 bufferSize = numberOfElements * sizeof(T);
            ResourceDescription desc{
                    .format = DXGI_FORMAT_UNKNOWN,
                    .width = bufferSize,
                    .height = 1,
                    .depthOrArraySize = 1,
                    .dimension = D3D12_RESOURCE_DIMENSION_BUFFER,
                    .flags = ResourceFlags::NONE,
                    .createState = ResourceState::COMMON,
                    .heapType = D3D12_HEAP_TYPE_DEFAULT,
                    .name = "Buffer"
            };
            DescriptorProperties descriptorProps = {
                .descriptor = DescriptorFlags::ShaderResource,
                .viewDimension = D3D12_SRV_DIMENSION_BUFFER,
                .bufferStride = sizeof(T),
                .numElements = numberOfElements 
            };
            Resource::InitResource(device, desc, descriptorProps);

            buffer.Init(device, 1, bufferSize, false);
            D3D12_SUBRESOURCE_DATA subresData = {};
            subresData.pData = data;
            subresData.RowPitch = bufferSize;
            subresData.SlicePitch = 1;
            Transition(commandList, ResourceState::COPY_DEST);
            UpdateSubresources(commandList, resource(), buffer.resource(), 0, 0, 1, &subresData);
            Transition(commandList, ResourceState::GENERIC_READ_STATE);

		}
		u32 GetDescriptorHeapIndex()
		{
			return srv.getDescriptorIndex();
		}
        D3D12_INDEX_BUFFER_VIEW GetIndexBufferView() {
            D3D12_INDEX_BUFFER_VIEW view;
            view.BufferLocation = resource()->GetGPUVirtualAddress();
            view.Format = DXGI_FORMAT_R32_UINT;
            view.SizeInBytes = resource()->GetDesc().Width * sizeof(u32);
            return view;
        }
	private:
        UploadBuffer buffer;
	};

	class ConstantBuffer : public Resource
	{
	public:
        ConstantBuffer() = default;
		template<typename T>
		void Init(RenderContext& context, T* data, uint numberOfElements)
		{
            ID3D12Device* device = context.GetDevice().GetDevice();
            m_structSize = CalcConstantBufferByteSize(sizeof(T)); // do we need make single entry of buffer be
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