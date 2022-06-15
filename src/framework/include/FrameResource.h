#pragma once
#include "UploadBuffer.h"
#include "PassConstants.h"
#include "Texture.h"
#include "../dx12/DescriptorHeap.h"
#include "RenderItem.h"

class FrameResource
{
public:
	FrameResource(ID3D12Device* device, UINT passCount, UINT objectCount);
	void Init(ID3D12Device* device, UINT passCount, UINT objectCount);

	FrameResource() = default;
	FrameResource(FrameResource&&) ;
	FrameResource& operator=(const FrameResource&) = delete;

	ComPtr<ID3D12CommandAllocator> m_cmdAlloc = nullptr;
	UploadBuffer<PassConstants> m_passCb;
	UploadBuffer<ObjectConstants::ObjectProperties> m_objectCb;
	ViewID passIndex=-1;
	ViewID objectIndex=-1;

	UINT m_fence = 0;

};

