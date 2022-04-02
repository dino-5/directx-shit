#include "FrameResource.h"

FrameResource::FrameResource(ComPtr<ID3D12Device>& device, UINT passCount, UINT objectCount)
{
	ThrowIfFailed(device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(m_cmdAlloc.GetAddressOf())));
	m_passCb = std::make_unique<UploadBuffer<PassConstants>>(device.Get(), passCount, true);
	m_objectCb = std::make_unique<UploadBuffer<ObjectConstants>>(device.Get(), objectCount, true);
}


