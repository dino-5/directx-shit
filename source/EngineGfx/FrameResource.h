#pragma once
#include "EngineGfx/dx12/DescriptorHeap.h"
#include "EngineGfx/dx12/Buffers.h"
#include "EngineGfx/dx12/Device.h"
#include "EngineGfx/PassConstants.h"
#include "EngineGfx/Texture.h"
#include "EngineGfx/RenderItem.h"
#include <array>
#include <vector>

namespace engine::graphics
{
	struct FrameResourceDescription
	{
		uint bufferSize;
		uint bufferCount;
	};

	template<uint N>
	class FrameResource
	{
	public:
		FrameResource(std::vector<FrameResourceDescription>& resourceDesc);
		void Init(std::vector<FrameResourceDescription>& resourceDesc);

		FrameResource() = default;
		FrameResource(FrameResource&&);
		FrameResource& operator=(const FrameResource&) = delete;

		ComPtr<ID3D12CommandAllocator> m_cmdAlloc = nullptr;
		//UploadBuffer m_passCb;
		//UploadBuffer m_objectCb;
		std::array<UploadBuffer, N> m_buffers;
		ViewID passIndex = -1;
		ViewID objectIndex = -1;

		UINT m_fence = 0;

	};

	template<uint N>
	FrameResource<N>::FrameResource(std::vector<FrameResourceDescription>& resourceDesc)
	{
		Init(resourceDesc);
	}

	template<uint N>
	void FrameResource<N>::Init(std::vector<FrameResourceDescription>& resourceDesc)
	{
		auto device = Device::GetDevice();
		for (int i = 0; i < resourceDesc.size(); i++)
		{
			m_buffers[i].Init(resourceDesc[i].bufferCount, resourceDesc[i].bufferSize, true);
		}
		//m_passCb.Init(passCount, sizeof(PassConstants), true);
		//m_objectCb.Init(objectCount, sizeof(ObjectConstants::ObjectProperties), true);
		ThrowIfFailed(device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(m_cmdAlloc.GetAddressOf())));
	}

	template<uint N>
	FrameResource<N>::FrameResource(FrameResource<N>&& obj) :
		m_buffers(std::move(obj.m_buffers)),
		m_cmdAlloc(obj.m_cmdAlloc),
		passIndex(obj.passIndex),
		objectIndex(obj.passIndex),
		m_fence(obj.m_fence)
	{}
};
