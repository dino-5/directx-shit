#pragma once
#include <d3d12.h>
#include "EngineCommon/System/config.h"
#include "EngineGfx/dx12/d3dx12.h"
#include "EngineCommon/include/types.h"
#include "EngineCommon/include/common.h"
#include "EngineGfx/dx12/Resource.h"

namespace engine::graphics
{
	class VertexBuffer
	{
	public:
		VertexBuffer() = default;
		template<typename T>
		void Init(ID3D12Device* device, T* data, uint sizeOfBuffer)
		{
			auto heapType = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
			auto bufferDesc =CD3DX12_RESOURCE_DESC::Buffer(sizeOfBuffer); 
			ThrowIfFailed(device->CreateCommittedResource(
				&heapType,
				D3D12_HEAP_FLAG_NONE,
				&bufferDesc,
				D3D12_RESOURCE_STATE_GENERIC_READ,
				nullptr,
				IID_PPV_ARGS(&m_vertexBuffer)));

			UINT8* pVertexDataBegin;
			CD3DX12_RANGE readRange(0, 0); 
			ThrowIfFailed(m_vertexBuffer->Map(0, &readRange, reinterpret_cast<void**>(&pVertexDataBegin)));
			memcpy(pVertexDataBegin, data, sizeOfBuffer);
			m_vertexBuffer->Unmap(0, nullptr);

			m_vertexBufferView.BufferLocation = m_vertexBuffer->GetGPUVirtualAddress();
			m_vertexBufferView.StrideInBytes = sizeof(T);
			m_vertexBufferView.SizeInBytes = sizeOfBuffer;
		}
		D3D12_VERTEX_BUFFER_VIEW& GetView(){ return m_vertexBufferView; }
	private:
		ID3D12Resource* m_vertexBuffer;
		D3D12_VERTEX_BUFFER_VIEW m_vertexBufferView;

	};

	class ConstantBuffer 
	{
	public:
		ConstantBuffer(uint size) : sizeOfStruct(engine::util::d3dUtil::CalcConstantBufferByteSize(size))
		{}
		void Init(ID3D12Device* device, uint numberOfElements, const void* data) 
		{
			CD3DX12_RANGE readRange(0, 0);       
			uint size = sizeOfStruct*numberOfElements;
			for (uint i = 0; i < engine::config::NumFrames; i++)
			{
				m_buffer[i] = nullptr;
				m_resouces[i].InitAsConstantBuffer(device, size);
				ThrowIfFailed(m_resouces[i]->Map(0, &readRange, reinterpret_cast<void**>(&m_buffer[i])));
				Update(data, i);
			}
		}
		// frame selects which resource we should use
		// and element selects which object data we should use from this resource
		D3D12_GPU_VIRTUAL_ADDRESS GetAddress(uint frame, uint element=0) 		{
			return (m_resouces[frame])->GetGPUVirtualAddress() + sizeOfStruct * element;
		}

		void Update(const void* data, uint frameNumber, uint numberOfElement = 0)
		{
			memcpy(&m_buffer[frameNumber][numberOfElement * sizeOfStruct], data, sizeOfStruct);
		}
	private:
		// we have resource per frame, and in one frame on one resource we can have any posible number of same objects data 
		// holding in one constant buffer
		char* m_buffer[engine::config::NumFrames];
		uint sizeOfStruct;
		Resource m_resouces[engine::config::NumFrames];
	};

};