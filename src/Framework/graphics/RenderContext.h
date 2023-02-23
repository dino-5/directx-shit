#pragma once

#include "Framework/graphics/dx12/Device.h"
#include "Framework/graphics/dx12/SwapChain.h"
#include "Framework/graphics/dx12/PSO.h"
#include "Framework/graphics/dx12/CommandQueue.h"
#include "Framework/graphics/dx12/Resource.h"
#include "Framework/graphics/dx12/DescriptorHeap.h"
#include "Framework/include/defines.h"
#include "Framework/include/types.h"

namespace engine::graphics
{


	struct RenderSettings
	{
		int width;
		int height;
	};

	class RenderContext
	{
	public:
		RenderContext() = default;
		SHIT_ENGINE_NON_COPYABLE(RenderContext);
		void Initialize(ComPtr<ID3D12CommandAllocator>&);
		void ResetSwapChain(SwapChainSettings set);

		void ResetCommandAllocator(ID3D12CommandAllocator*);
		void FlushCommandQueue();

		void Reset() {
			m_swapChain.Reset();
			m_dsvBuffer.Reset();
			m_rtvHeap.Reset();
			m_dsvHeap.Reset();
		}

	private:
		Device m_device;
		CommandQueue m_graphicsQueue;
		SwapChain m_swapChain;
		Resource m_dsvBuffer;

		DescriptorHeap m_rtvHeap;
		DescriptorHeap m_dsvHeap;

		PSO* m_currentPSO;
		ComPtr<ID3D12Fence> m_fence;
		uint m_currentFence = 0;
		ComPtr<ID3D12GraphicsCommandList> m_commandList;
	};
};