#pragma once

#include "EngineGfx/dx12/Device.h"
#include "EngineGfx/dx12/SwapChain.h"
#include "EngineGfx/dx12/PSO.h"
#include "EngineGfx/dx12/CommandQueue.h"
#include "EngineGfx/dx12/CommandList.h"
#include "EngineGfx/dx12/Resource.h"
#include "EngineGfx/dx12/DescriptorHeap.h"
#include "EngineCommon/include/defines.h"
#include "EngineCommon/include/types.h"
#include "EngineGfx/dx12/d3dx12.h"

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
		void Initialize(SwapChainSettings set);
		void ResetSwapChain(SwapChainSettings set);
		void SetupViewport(SwapChainSettings& set);

		void ResetCommandAllocator();
		void FlushCommandQueue();
		void NextFrame();

		void Reset() {
			m_swapChain.Reset();
			m_dsvBuffer.reset();
		}

		void StartFrame();
		void EndFrame();

		int GetAspectRatio()const { return m_swapChain.GetAspectRatio(); }
		CommandList& GetList() { return m_graphicsCommandList; }
		Device& GetDevice() { return m_device; }
		CommandQueue& GetQueue() { return m_graphicsQueue; }

	private:
		D3D12_VIEWPORT m_viewport;
		D3D12_RECT m_scissorRect;
		Device m_device;
		CommandQueue m_graphicsQueue;
		CommandList m_graphicsCommandList;
		SwapChain m_swapChain;
		Resource m_dsvBuffer;

		HANDLE m_fenceEvent;

		PSO* m_currentPSO;
		ComPtr<ID3D12Fence> m_fence;
		uint m_currentFence = 0;
		uint m_currentFrame = 0;
	};
};