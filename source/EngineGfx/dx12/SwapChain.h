#pragma once
#include <d3d12.h>
#include <dxgi.h>
#include "EngineCommon/System/config.h"
#include "Resource.h"

namespace engine::graphics
{
	struct SwapChainSettings
	{
		int width{};
		int height{};
		DXGI_FORMAT format = DXGI_FORMAT_R8G8B8A8_UNORM;
		HWND window{};
	};


	class SwapChain
	{
	public:
		SwapChain() = default;
		SwapChain(SwapChainSettings settings, ComPtr<IDXGIFactory4> factory, ComPtr<ID3D12CommandQueue> queue);
		void init(SwapChainSettings settings, ComPtr<IDXGIFactory4> factory, ComPtr<ID3D12CommandQueue> queue);
		IDXGISwapChain* getSwapChain() { return m_swapChain; }
		void onResize();

		IDXGISwapChain* operator->() { return m_swapChain; }
		f32 getAspectRatio()const { 
			return static_cast<f32>(m_currentSettings.width) / static_cast<f32>(m_currentSettings.height); 
		}

		void reset() {
			m_swapChain->Release();
			for (int i = 0; i < engine::config::NumFrames; i++)
				m_resources[i].reset();
		}

		u32 changeState(ID3D12GraphicsCommandList* cmdList, ResourceState state)
		{
			IDXGISwapChain3* swapChain;
			m_swapChain->QueryInterface(IID_PPV_ARGS(&swapChain));
			u32 index = swapChain->GetCurrentBackBufferIndex();
			m_resources[index].transition(cmdList, state);
			swapChain->QueryInterface(IID_PPV_ARGS(&m_swapChain));
			return index;
		}

		DescriptorCPU getView(uint index) { return m_resources[index].rtv; }

		uint m_fence[engine::config::NumFrames] = {};
		SwapChainSettings m_currentSettings;

		Resource m_resources[engine::config::NumFrames] = {};
	private:
		uint m_currentBuffer = 0;
		IDXGISwapChain* m_swapChain = nullptr;
	};
};
