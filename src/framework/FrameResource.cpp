#include "include/FrameResource.h"

FrameResource::FrameResource(ID3D12Device* device, UINT passCount, UINT objectCount)
{
	ThrowIfFailed(device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(m_cmdAlloc.GetAddressOf())));
	m_passCb.Init(device, passCount, true);
	m_objectCb.Init(device, objectCount, true);
}

void FrameResource::Init(ID3D12Device* device, UINT passCount, UINT objectCount)
{
	ThrowIfFailed(device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(m_cmdAlloc.GetAddressOf())));
	m_passCb.Init(device, passCount, true);
	m_objectCb.Init(device, objectCount, true);
}

FrameResource::FrameResource(FrameResource&& obj) :
	m_passCb(std::move(obj.m_passCb)),
	m_objectCb(std::move(obj.m_objectCb)),
	m_cmdAlloc(obj.m_cmdAlloc),
	passIndex(obj.passIndex),
	objectIndex(obj.passIndex),
	m_fence(obj.m_fence)
{}

