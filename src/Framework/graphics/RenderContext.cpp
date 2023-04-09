#include "Framework/graphics/RenderContext.h"
#include "Framework/util/Logger.h"
#include <format>

namespace engine::graphics
{
	void LoadPipeline(ID3D12Device* dev)
	{
		LogScope("Graphics");
		PopulateRootSignatures(dev);
		PopulateShaders();
		PopulatePSO(dev);
	}

	void RenderContext::Initialize()
	{
		LogScope("RenderContext");
		m_device.Initialize();
		engine::util::logInfo("device initialized");
		m_graphicsQueue.Init(m_device.GetDevice(), {});
		engine::util::logInfo("queue initialized");
		m_graphicsCommandList.Initialize(m_device);
		engine::util::logInfo("list initialized");
		m_device.CreateFence(m_fence);

		DescriptorHeapManager::CreateRTVHeap(engine::config::NumFrames);
		DescriptorHeapManager::CreateDSVHeap(1);
		engine::util::logInfo("heaps created");
		LoadPipeline(m_device.GetDevice());
		{
			m_currentFence=1;

			// Create an event handle to use for frame synchronization.
			if (m_fenceEvent == nullptr)
			{
				ThrowIfFailed(HRESULT_FROM_WIN32(GetLastError()));
			}
			FlushCommandQueue();

		}
	}

	void RenderContext::SetupViewport(SwapChainSettings& set)
	{
		m_viewport.TopLeftX = 0;
		m_viewport.TopLeftY = 0;
		m_viewport.Width    = static_cast<float>(set.width);
		m_viewport.Height   = static_cast<float>(set.height);
		m_viewport.MinDepth = 0.0f;
		m_viewport.MaxDepth = 1.0f;

		m_scissorRect= { 0, 0, set.width, set.height};
	}

	void RenderContext::ResetSwapChain(SwapChainSettings set)
	{
		if (m_graphicsQueue.GetQueue() != nullptr)
			m_swapChain.Init(set, m_device.GetFactory(), m_graphicsQueue.GetQueue());
		SetupViewport(set);

		D3D12_CLEAR_VALUE optClear;
		optClear.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
		optClear.DepthStencil.Depth = 1.0f;
		optClear.DepthStencil.Stencil = 0;

		ResourceDescription desc;
		desc.createState = ResourceState::DEPTH_WRITE;
		desc.descriptor = ResourceDescriptorFlags::DepthStencil;
		desc.dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
		desc.flags = ResourceFlags::DEPTH_STENCIL;
		desc.format = DXGI_FORMAT_R24G8_TYPELESS;
		desc.width = set.width;
		desc.height = set.height;
		m_dsvBuffer.Init(Device::device->GetDevice(), desc, &optClear);
	}

	void RenderContext::FlushCommandQueue()
	{
		auto value = m_fence->GetCompletedValue();
		ThrowIfFailed(m_graphicsQueue->Signal(m_fence.Get(), m_currentFence));

		// Wait until the previous frame is finished.
		if (m_swapChain.m_fence[m_currentFrame]!=0 && value < m_swapChain.m_fence[m_currentFrame] )
		{
			m_fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
			ThrowIfFailed(m_fence->SetEventOnCompletion(m_swapChain.m_fence[m_currentFrame], m_fenceEvent));
			WaitForSingleObject(m_fenceEvent, INFINITE);
			CloseHandle(m_fenceEvent);
		}
	}

	void RenderContext::NextFrame()
	{
		FlushCommandQueue();
	}

	void RenderContext::ResetCommandAllocator()
	{
		m_graphicsCommandList.Reset(0);

	}

	void RenderContext::StartFrame()
	{
		m_graphicsCommandList.Reset(m_currentFrame);
		m_graphicsCommandList->RSSetViewports(1, &m_viewport);
		m_graphicsCommandList->RSSetScissorRects(1, &m_scissorRect);
		m_swapChain.ChangeState(m_graphicsCommandList.GetList(), m_currentFrame, ResourceState::RENDER_TARGET);
		auto rtvHandle = m_swapChain.GetView(m_currentFrame);
		auto dsvHandle = m_dsvBuffer.dsv;
		const float clearColor[] = { 0.0f, 0.2f, 0.4f, 1.0f };
		m_graphicsCommandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);
		m_graphicsCommandList->ClearDepthStencilView(dsvHandle, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);
		m_graphicsCommandList->OMSetRenderTargets(1, &rtvHandle.HandleCPU, true, &dsvHandle.HandleCPU);
	}

	void RenderContext::EndFrame()
	{
		m_swapChain.ChangeState(m_graphicsCommandList.GetList(), m_currentFrame, ResourceState::PRESENT);
		ThrowIfFailed(m_graphicsCommandList->Close());
		ID3D12CommandList* ppCommandLists[] = { m_graphicsCommandList.GetList()};
		m_graphicsQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);
		// Present the frame.
		m_swapChain->Present(0, 0);
		m_swapChain.m_fence[m_currentFrame] = ++m_currentFence;
		FlushCommandQueue();
		m_currentFrame = (m_currentFrame + 1) % (engine::config::NumFrames);
		ThrowIfFailed(m_device.GetDevice()->GetDeviceRemovedReason());
	}

};
