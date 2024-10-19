#pragma once

#include "EngineGfx/dx12/Device.h"
#include "EngineGfx/dx12/SwapChain.h"
#include "EngineGfx/dx12/PSO.h"
#include "EngineGfx/dx12/CommandQueue.h"
#include "EngineGfx/dx12/CommandList.h"
#include "EngineGfx/dx12/Resource.h"
#include "EngineGfx/dx12/DescriptorHeap.h"
#include "EngineGfx/util/Camera.h"
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
		void initialize(SwapChainSettings set);
		void resetSwapChain(SwapChainSettings set);
		void setupViewport(SwapChainSettings& set);

		void resetCommandAllocator();
		void flushCommandQueue();
		void nextFrame();

		void update()
		{
			m_camera.update();
		}

		void reset() {
			m_swapChain.reset();
			m_dsvBuffer.reset();
		}

		void startFrame();
		void endFrame();

		f32 getAspectRatio()const { return m_swapChain.getAspectRatio(); }
		CommandList& getList() { return m_graphicsCommandList; }
		Device& getDevice() { return m_device; }
		CommandQueue& getQueue() { return m_graphicsQueue; }
		u64 getFenceValue() { return m_fence->GetCompletedValue(); }
		Camera& getCamera() { return m_camera; }

	private:
		D3D12_VIEWPORT m_viewport;
		D3D12_RECT m_scissorRect;
		Device m_device;
		CommandQueue m_graphicsQueue;
		CommandList m_graphicsCommandList;
		SwapChain m_swapChain;
		Resource m_dsvBuffer;

		Camera m_camera;

		HANDLE m_fenceEvent;

		PSO* m_currentPSO = nullptr;
		ID3D12Fence* m_fence = nullptr;
		uint m_currentFence = 0;
		uint m_currentFrame = 0;
	};
};