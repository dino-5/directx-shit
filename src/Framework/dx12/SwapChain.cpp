#include "SwapChain.h"
#include "Device.h"
#include "Framework/include/common.h"
#include "Framework/include/Util.h"

SwapChain::SwapChain(SwapChainSettings settings, ComPtr<IDXGIFactory4> factory, ComPtr<ID3D12CommandQueue> queue)
{
	Init(settings, factory, queue);
}

void SwapChain::Init(SwapChainSettings settings, ComPtr<IDXGIFactory4> factory, ComPtr<ID3D12CommandQueue> queue)
{
    m_swapChain.Reset();

    DXGI_SWAP_CHAIN_DESC sd;
    sd.BufferDesc.Width = settings.width;
    sd.BufferDesc.Height = settings.height;
    sd.BufferDesc.RefreshRate.Numerator = 60;
    sd.BufferDesc.RefreshRate.Denominator = 1;
    sd.BufferDesc.Format = settings.format;
    sd.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
    sd.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
    sd.SampleDesc.Count = 1;
    sd.SampleDesc.Quality = 0;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.BufferCount = NumFrames;
    sd.OutputWindow = settings.window;
    sd.Windowed = true;
	sd.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

    ThrowIfFailed(factory->CreateSwapChain(
		queue.Get(),
		&sd, 
		m_swapChain.GetAddressOf()));
}

void SwapChain::OnResize()
{
	for (int i = 0; i < NumFrames; ++i)
		m_swapChainBuffer[i].Reset();
	
	
	// Resize the swap chain.
 //   ThrowIfFailed(mSwapChain->ResizeBuffers(
	//	NumFrames, 
	//	mClientWidth, mClientHeight, 
	//	mBackBufferFormat, 
	//	DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH));

	////mCurrBackBuffer = 0;
 //
	//for (UINT i = 0; i < NumFrames; i++)
	//{
	//	ThrowIfFailed(mSwapChain->GetBuffer(i, IID_PPV_ARGS(m_swapChainBuffer[i].GetAddress() )));
	//	m_rtvHeap.CreateRTV(m_swapChainBuffer[i]);
	//}
}
