#pragma once
#include <d3d12.h>
#include <dxgi.h>
#include "include/common.h"
#include "Resource.h"

class SwapChain
{
public:
	SwapChain() = default;
	SwapChain(int widht, int height, DXGI_FORMAT format, HWND window, ComPtr<ID3D12CommandQueue> queue);
	void OnResize();
private:
	ComPtr<IDXGISwapChain> m_swapChain;
	Resource m_swapChainBuffer[NumFrames];
};

