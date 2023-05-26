#pragma once

#include "Framework/graphics/dx12/Device.h"
#include "Framework/graphics/dx12/SwapChain.h"
#include "Framework/graphics/dx12/PSO.h"
#include "Framework/graphics/dx12/CommandQueue.h"
#include "Framework/graphics/dx12/CommandList.h"
#include "Framework/graphics/dx12/Resource.h"
#include "Framework/graphics/dx12/DescriptorHeap.h"
#include "Framework/include/defines.h"
#include "Framework/include/types.h"
#include "Framework/include/d3dx12.h"

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
		void Initialize();
		void ResetSwapChain(SwapChainSettings set);
		void SetupViewport(SwapChainSettings& set);

		void ResetCommandAllocator();
		void FlushCommandQueue();
		void NextFrame();

		void Reset() {
			m_swapChain.Reset();
			m_dsvBuffer.Reset();
			m_rtvHeap.Reset();
			m_dsvHeap.Reset();
		}

		void StartFrame();
		void EndFrame();

		int GetAspectRatio()const { return m_swapChain.GetAspectRatio(); }
		ID3D12GraphicsCommandList* GetList() { return m_graphicsCommandList.GetList(); }
		ID3D12Device* GetDevice() { return m_device.GetDevice(); }

	private:
		D3D12_VIEWPORT m_viewport;
		D3D12_RECT m_scissorRect;
		Device m_device;
		CommandQueue m_graphicsQueue;
		CommandList m_graphicsCommandList;
		SwapChain m_swapChain;
		Resource m_dsvBuffer;

		HANDLE m_fenceEvent;

		DescriptorHeap m_rtvHeap;
		DescriptorHeap m_dsvHeap;

		PSO* m_currentPSO;
		ComPtr<ID3D12Fence> m_fence;
		uint m_currentFence = 0;
		uint m_currentFrame = 0;
	};
};