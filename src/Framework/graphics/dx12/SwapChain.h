#pragma once
#include <d3d12.h>
#include <dxgi.h>
#include "include/common.h"
#include "Framework/System/config.h"
#include "Resource.h"

struct SwapChainSettings
{
	int width;
	int height;
	DXGI_FORMAT format = DXGI_FORMAT_R8G8B8A8_UNORM;
	HWND window;
};


class SwapChain
{
public:
	SwapChain() = default;
	SwapChain(SwapChainSettings settings, ComPtr<IDXGIFactory4> factory, ComPtr<ID3D12CommandQueue> queue);
	void Init(SwapChainSettings settings, ComPtr<IDXGIFactory4> factory, ComPtr<ID3D12CommandQueue> queue);
	IDXGISwapChain* GetSwapChain() { return m_swapChain.Get(); }
	void OnResize();

	IDXGISwapChain* operator->() { return m_swapChain.Get(); }

	void Reset() {
		m_swapChain.Reset(); 
		for (int i = 0; i < engine::config::NumFrames; i++)
			m_swapChainBuffer[i].Reset();
	}

private:
	uint m_currentBuffer=0;
	ComPtr<IDXGISwapChain> m_swapChain;
	Resource m_swapChainBuffer[engine::config::NumFrames];
	SwapChainSettings m_currentSettings;
};

