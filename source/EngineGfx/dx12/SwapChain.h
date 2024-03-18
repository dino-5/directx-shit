#pragma once
#include <d3d12.h>
#include <dxgi.h>
#include "EngineCommon/include/common.h"
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
		void Init(SwapChainSettings settings, ComPtr<IDXGIFactory4> factory, ComPtr<ID3D12CommandQueue> queue);
		IDXGISwapChain* GetSwapChain() { return m_swapChain; }
		void OnResize();

		IDXGISwapChain* operator->() { return m_swapChain; }
		int GetAspectRatio()const { return m_currentSettings.width / m_currentSettings.height; }

		void Reset() {
			m_swapChain->Release();
			for (int i = 0; i < engine::config::NumFrames; i++)
				m_swapChainBuffer[i].reset();
		}

		u32 ChangeState(ID3D12GraphicsCommandList* cmdList, ResourceState state)
		{
			IDXGISwapChain3* swapChain;
			m_swapChain->QueryInterface(IID_PPV_ARGS(&swapChain));
			u32 index = swapChain->GetCurrentBackBufferIndex();
			m_swapChainBuffer[index].transition(cmdList, state);
			swapChain->QueryInterface(IID_PPV_ARGS(&m_swapChain));
			return index;
		}

		DescriptorRTV GetView(uint index) { return m_swapChainBuffer[index].rtv; }

		uint m_fence[engine::config::NumFrames] = {};
		SwapChainSettings m_currentSettings;
	private:
		uint m_currentBuffer = 0;
		IDXGISwapChain* m_swapChain = nullptr;
		Resource m_swapChainBuffer[engine::config::NumFrames] = {};
	};
};
