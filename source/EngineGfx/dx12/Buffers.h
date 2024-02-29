#pragma once
#include <d3d12.h>
#include "EngineCommon/System/config.h"
#include "EngineGfx/dx12/d3dx12.h"
#include "EngineCommon/include/types.h"
#include "EngineCommon/include/common.h"
#include "EngineGfx/dx12/Resource.h"

namespace engine::graphics
{
    using engine::util::CalcConstantBufferByteSize;
	class VertexBuffer : public Resource
	{
	public:
		VertexBuffer() = default;
		template<typename T>
		void Init(ID3D12Device* device, T* data, uint bufferSize)
		{
            ResourceDescription desc{
                    .format = DXGI_FORMAT_UNKNOWN,
                    .width = bufferSize,
                    .height = 1,
                    .depthOrArraySize = 1,
                    .dimension = D3D12_RESOURCE_DIMENSION_BUFFER,
                    .flags = ResourceFlags::NONE,
                    .createState = ResourceState::GENERIC_READ_STATE,
                    .heapType = D3D12_HEAP_TYPE_UPLOAD,
                    .descriptor = ResourceDescriptorFlags::None
            };
            Resource::init(device, desc);
			UINT8* pVertexDataBegin;
			CD3DX12_RANGE readRange(0, 0); 
			ThrowIfFailed(getResource()->Map(0, &readRange, reinterpret_cast<void**>(&pVertexDataBegin)));
			memcpy(pVertexDataBegin, data, bufferSize);
			getResource()->Unmap(0, nullptr);

			m_vertexBufferView.BufferLocation = getResource()->GetGPUVirtualAddress();
			m_vertexBufferView.StrideInBytes = sizeof(T);
			m_vertexBufferView.SizeInBytes = bufferSize;
		}
		D3D12_VERTEX_BUFFER_VIEW& GetView(){ return m_vertexBufferView; }
	private:
		D3D12_VERTEX_BUFFER_VIEW m_vertexBufferView;
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
                    .descriptor = ResourceDescriptorFlags::ConstantBuffer
            };
			Resource::init(device, desc);
			CD3DX12_RANGE readRange(0, 0);       
            ThrowIfFailed(getResource()->Map(0, &readRange, reinterpret_cast<void**>(&m_buffer)));
            Update(data);
		}

		D3D12_GPU_VIRTUAL_ADDRESS getAddress(u32 element=0) 		{
			return getResource()->GetGPUVirtualAddress() + m_structSize * element;
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

    class UploadBuffer
    {
    public:
        UploadBuffer(ID3D12Device* device, uint elementCount, uint sizeOfType , bool isConstantBuffer)  
        {
            Init(device, elementCount, sizeOfType, isConstantBuffer);
        }

        void Init(ID3D12Device* device, uint elementCount, uint sizeOfType, bool isConstantBuffer) 
        {
            mIsConstantBuffer = isConstantBuffer;
            mElementByteSize = sizeOfType;

            if(isConstantBuffer)
                mElementByteSize = engine::util::CalcConstantBufferByteSize(sizeOfType);

            auto heapDesc = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
            auto bufferDesc = CD3DX12_RESOURCE_DESC::Buffer(mElementByteSize * elementCount); 
            ThrowIfFailed(device->CreateCommittedResource(
                &heapDesc,
                D3D12_HEAP_FLAG_NONE,
                &bufferDesc,
                D3D12_RESOURCE_STATE_GENERIC_READ,
                nullptr,
                IID_PPV_ARGS(&mUploadBuffer)));

            ThrowIfFailed(mUploadBuffer->Map(0, nullptr, reinterpret_cast<void**>(&mMappedData)));

        }

        UploadBuffer() = default;
        UploadBuffer(UploadBuffer&& rhs):
            mUploadBuffer(std::move(rhs.mUploadBuffer)),
            mMappedData(std::move(rhs.mMappedData)),
            mElementByteSize(std::move(rhs.mElementByteSize)),
            mIsConstantBuffer(std::move(rhs.mIsConstantBuffer))
        {}
        UploadBuffer& operator=(const UploadBuffer& rhs) = delete;
        ~UploadBuffer()
        {
            if(mUploadBuffer != nullptr)
                mUploadBuffer->Unmap(0, nullptr);

            mMappedData = nullptr;
        }

        ID3D12Resource* Resource()const
        {
            return mUploadBuffer.Get();
        }

        void CopyData(int elementIndex, const void* data)
        {
            memcpy(&mMappedData[elementIndex*mElementByteSize], data, mElementByteSize);
        }

    public:
        Microsoft::WRL::ComPtr<ID3D12Resource> mUploadBuffer;
        BYTE* mMappedData = nullptr;

        UINT mElementByteSize = 0;
        bool mIsConstantBuffer = false;
    };
};