#include "Framework/graphics/RenderContext.h"

namespace engine::graphics
{

	void RenderContext::Initialize(ComPtr<ID3D12CommandAllocator>& alloc)
	{
		//assert(m_device.GetDevice() != nullptr);
		m_device.Initialize();
		m_graphicsQueue.Init(m_device.GetDevice(), {});
		m_device.CreateCommandAllocator(alloc);
		m_device.CreateCommandList(m_commandList, alloc);
		m_device.CreateFence(m_fence);

		DescriptorHeapManager::CreateRTVHeap(engine::config::NumFrames);
		DescriptorHeapManager::CreateDSVHeap(1);
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
		m_dsvBuffer.Init(Device::device->GetDevice(), DXGI_FORMAT_R24G8_TYPELESS, set.width, set.height, 1, D3D12_RESOURCE_DIMENSION_TEXTURE2D,
			ResourceFlags::DEPTH_STENCIL, ResourceState::COMMON, heapProp, ResourceDescriptorFlags::DepthStencil, &optClear);
	}

	void RenderContext::FlushCommandQueue()
	{
		m_currentFence++;
		ThrowIfFailed(m_graphicsQueue->Signal(m_fence.Get(), m_currentFence));

		if (m_fence->GetCompletedValue() < m_currentFence)
		{
			HANDLE eventHandle = CreateEventEx(nullptr, nullptr, false, EVENT_ALL_ACCESS);

			ThrowIfFailed(m_fence->SetEventOnCompletion(m_currentFence, eventHandle));

			WaitForSingleObject(eventHandle, INFINITE);
			CloseHandle(eventHandle);
		}
	}

	void RenderContext::ResetCommandAllocator(ID3D12CommandAllocator* alloc)
	{
		m_commandList->Reset(alloc, nullptr);
	}

};
