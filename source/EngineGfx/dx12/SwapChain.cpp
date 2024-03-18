#include "SwapChain.h"
#include "Device.h"
#include "EngineCommon/include/common.h"
#include "EngineCommon/util/Util.h"

namespace engine::graphics
{
	SwapChain::SwapChain(SwapChainSettings settings, ComPtr<IDXGIFactory4> factory, ComPtr<ID3D12CommandQueue> queue)
	{
		Init(settings, factory, queue);
	}

	void SwapChain::Init(SwapChainSettings settings, ComPtr<IDXGIFactory4> factory, ComPtr<ID3D12CommandQueue> queue)
	{
		m_currentSettings = settings;
		if(m_swapChain)
            m_swapChain->Release();

		DXGI_SWAP_CHAIN_DESC sd;
		sd.BufferDesc.Width = m_currentSettings.width;
		sd.BufferDesc.Height = m_currentSettings.height;
		sd.BufferDesc.RefreshRate.Numerator = 60;
		sd.BufferDesc.RefreshRate.Denominator = 1;
		sd.BufferDesc.Format = m_currentSettings.format;
		sd.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
		sd.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
		sd.SampleDesc.Count = 1;
		sd.SampleDesc.Quality = 0;
		sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		sd.BufferCount = engine::config::NumFrames;
		sd.OutputWindow = m_currentSettings.window;
		sd.Windowed = true;
		sd.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
		sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

		ThrowIfFailed(factory->CreateSwapChain(
			queue.Get(),
			&sd,
			&m_swapChain));
		OnResize();
	}

	void SwapChain::OnResize()
	{
		for (int i = 0; i < engine::config::NumFrames; ++i)
			m_swapChainBuffer[i].reset();


		ThrowIfFailed(m_swapChain->ResizeBuffers(
			engine::config::NumFrames,
			m_currentSettings.width, m_currentSettings.height,
			m_currentSettings.format,
			DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH));

		m_currentBuffer = 0;

		for (UINT i = 0; i < engine::config::NumFrames; i++)
		{
			ThrowIfFailed(m_swapChain->GetBuffer(i, IID_PPV_ARGS(m_swapChainBuffer[i].getResourceAddress())));
			m_swapChainBuffer[i].createViews(Device::device->GetDevice(), 
				DescriptorProperties(DescriptorFlags::RenderTarget));
		}
	}
};
