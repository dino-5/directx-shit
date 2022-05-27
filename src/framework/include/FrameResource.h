#pragma once
#include "UploadBuffer.h"
#include "PassConstants.h"
#include "Texture.h"
#include "../dx12/DescriptorHeap.h"

class FrameResource
{
public:
	FrameResource(ID3D12Device* device, UINT passCount, UINT objectCount);
	FrameResource(const FrameResource&) = delete;
	FrameResource& operator=(const FrameResource&) = delete;

	ComPtr<ID3D12CommandAllocator> m_cmdAlloc;
	std::unique_ptr<UploadBuffer<PassConstants>> m_passCb = nullptr;
	std::unique_ptr<UploadBuffer<ObjectConstants>> m_objectCb = nullptr;
	ViewID passIndex;
	ViewID objectIndex;

	UINT m_fence = 0;

};

