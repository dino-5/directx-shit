#include "RenderContext.h"

void RenderContext::Initialize()
{
	assert(m_device.GetDevice() != nullptr);
	m_graphicsQueue = CommandQueue(m_device.GetDevice(), {});
	m_device.CreateCommandList(m_commandList.Get());

	m_rtvHeap.Init(NumFrames, DescriptorHeapType::RTV);
	m_dsvHeap.Init(1, DescriptorHeapType::DSV);
	DescriptorHeapManager::CurrentDSVHeap = &m_dsvHeap;
	DescriptorHeapManager::CurrentRTVHeap = &m_rtvHeap;

}

void RenderContext::ResetSwapChain(SwapChainSettings set)
{
	if (m_graphicsQueue.GetQueue() != nullptr)
		m_swapChain.Init(set, m_device.GetFactory(), m_graphicsQueue.GetQueue());


    D3D12_CLEAR_VALUE optClear;
    optClear.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    optClear.DepthStencil.Depth = 1.0f;
    optClear.DepthStencil.Stencil = 0;
	auto heapProp = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
	m_dsvBuffer.Init(DXGI_FORMAT_R24G8_TYPELESS, set.width, set.height, 1, D3D12_RESOURCE_DIMENSION_TEXTURE2D,
		ResourceFlags::DEPTH_STENCIL, ResourceState::COMMON, heapProp, ResourceDescriptorFlags::DepthStencil, &optClear);
}
