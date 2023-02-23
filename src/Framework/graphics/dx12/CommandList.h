#pragma once

#include <d3d12.h>
#include "Framework/graphics/dx12/Device.h"
#include "Framework/include/types.h"
#include "Framework/System/config.h"

namespace engine::graphics
{
	using namespace engine;
	class CommandList
	{
	public:
		CommandList() = default;
		void Initialize(Device& dev)
		{
			for (uint i = 0; i < config::NumFrames; i++)
				dev.CreateCommandAllocator(m_alloc[i]);
			dev.CreateCommandList(m_list, m_alloc[0]);
		}

		ID3D12CommandList* operator->() { return m_list.Get(); }

	private:
		ComPtr<ID3D12GraphicsCommandList> m_list = nullptr;
		ComPtr<ID3D12CommandAllocator> m_alloc[config::NumFrames];

	};
}