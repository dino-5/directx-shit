#pragma once
#include <d3d12.h>
#include "Framework/include/d3dx12.h"
#include "Framework/include/types.h"
#include "Framework/include/common.h"

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

};