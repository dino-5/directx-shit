#include "SwapChain.h"
#include "Device.h"
#include "EngineCommon/util/Util.h"
#include <dxgi.h>

namespace engine::graphics
{
	SwapChain::SwapChain(SwapChainSettings settings, Device& device, ComPtr<ID3D12CommandQueue> queue)
	{
		init(settings, device, queue);
	}

	void SwapChain::init(SwapChainSettings settings, Device& device, ComPtr<ID3D12CommandQueue> queue)
	{
		LogScope("SwapChain::Initialize");
		m_currentSettings = settings;
		if(m_swapChain)
		    m_swapChain->Release();

		DXGI_SWAP_CHAIN_DESC sd;
		sd.BufferDesc.Width = m_currentSettings.width;
		sd.BufferDesc.Height = m_currentSettings.height;
		sd.BufferDesc.RefreshRate.Numerator = 600;
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
		if(device.checkForFeatureSupport())
		{
			sd.Flags |= DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING;
			util::printInfo("tearing supported");
		}
		m_flags = (DXGI_SWAP_CHAIN_FLAG)sd.Flags;

		ThrowIfFailed(device.getFactory()->CreateSwapChain(
			queue.Get(),
			&sd,
			&m_swapChain));
	}

	// call only when descriptor sets are already created
	void SwapChain::onResize(SwapChainSettings settings)
	{
		m_currentSettings = settings;
		for (int i = 0; i < engine::config::NumFrames; ++i)
			m_resources[i].reset();

		ThrowIfFailed(m_swapChain->ResizeBuffers(
			engine::config::NumFrames,
			m_currentSettings.width, m_currentSettings.height,
			m_currentSettings.format,
			m_flags));

		m_currentBuffer = 0;

		for (UINT i = 0; i < engine::config::NumFrames; i++)
		{
			ThrowIfFailed(m_swapChain->GetBuffer(i, IID_PPV_ARGS(m_resources[i].getResourceAddress())));
			m_resources[i].createViews(Device::device->getDevice(), 
				DescriptorProperties(DescriptorFlags::RenderTarget));
		}
	}
};
