#pragma once
#include"common.h"
#include"../../Common/UploadBuffer.h"
#include"PassConstants.h"

class FrameResource
{
public:
	FrameResource(ComPtr<ID3D12Device>& device, UINT passCount, UINT objectCount);
	FrameResource(const FrameResource&) = delete;
	FrameResource& operator=(const FrameResource&) = delete;

	ComPtr<ID3D12CommandAllocator> m_cmdAlloc;
	std::unique_ptr<UploadBuffer<PassConstants>> m_passCb = nullptr;
	std::unique_ptr<UploadBuffer<ObjectConstants>> m_objectCb = nullptr;

	UINT m_fence = 0;

};

