#include "EngineGfx/RenderContext.h"
#include "EngineGfx/dx12/Buffers.h"
#include "EngineCommon/util/Logger.h"
#include <format>

namespace engine::graphics
{
	void LoadPipeline(ID3D12Device* dev)
	{
		LogScope("Graphics");
		PopulateRootSignatures(dev);
		PopulateShaders();
		PopulatePSO(dev);
		PopulateDescriptorHeaps();
	}

	void RenderContext::initialize(SwapChainSettings set)
	{
		LogScope("RenderContext");
		m_device.initialize();
		engine::util::printInfo("device initialized");
		m_graphicsQueue.init(m_device.getDevice(), {});
		engine::util::printInfo("queue initialized");
		m_graphicsCommandList.initialize(m_device);
		engine::util::printInfo("list initialized");
		m_device.createFence(&m_fence);

		DescriptorHeapManager::CreateRTVHeap(engine::config::NumFrames);
		DescriptorHeapManager::CreateDSVHeap(1);
		engine::util::printInfo("heaps created");
		LoadPipeline(m_device.getDevice());
		{

			// Create an event handle to use for frame synchronization.
			if (m_fenceEvent == nullptr)
			{
				ThrowIfFailed(HRESULT_FROM_WIN32(GetLastError()));
			}

		}
		resetSwapChain(set);

		m_camera.initialize(math::Vector3(), math::Vector3({0.f, 0.f, 1.f}));
	}

	void RenderContext::setupViewport(SwapChainSettings& set)
	{
		m_viewport.TopLeftX = 0;
		m_viewport.TopLeftY = 0;
		m_viewport.Width    = static_cast<float>(set.width);
		m_viewport.Height   = static_cast<float>(set.height);
		m_viewport.MinDepth = 0.0f;
		m_viewport.MaxDepth = 1.0f;

		m_scissorRect= { 0, 0, set.width, set.height};
	}

	void RenderContext::resetSwapChain(SwapChainSettings set)
	{
		if (m_graphicsQueue.getQueue() != nullptr)
			m_swapChain.init(set, m_device.getFactory(), m_graphicsQueue.getQueue());
		setupViewport(set);

		D3D12_CLEAR_VALUE optClear;
		optClear.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
		optClear.DepthStencil.Depth = 1.0f;
		optClear.DepthStencil.Stencil = 0;

		ResourceDescription desc;
		desc.createState = ResourceState::DEPTH_WRITE;
		desc.dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
		desc.flags = ResourceFlags::DEPTH_STENCIL;
		desc.format = DXGI_FORMAT_R24G8_TYPELESS;
		desc.width = set.width;
		desc.height = set.height;
		m_dsvBuffer.initResource(Device::device->getDevice(), desc, DescriptorProperties(DescriptorFlags::DepthStencil), &optClear);
	}

	void RenderContext::flushCommandQueue()
	{
		u64 value = m_fence->GetCompletedValue();
		m_swapChain.m_fence[m_currentFrame] = ++m_currentFence;
		ThrowIfFailed(m_graphicsQueue->Signal(m_fence, m_currentFence));

		// Wait until the previous frame is finished.
		if (m_swapChain.m_fence[m_currentFrame]!=0 && value < m_swapChain.m_fence[m_currentFrame] )
		{
			m_fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
			ThrowIfFailed(m_fence->SetEventOnCompletion(m_swapChain.m_fence[m_currentFrame], m_fenceEvent));
			WaitForSingleObject(m_fenceEvent, INFINITE);
			CloseHandle(m_fenceEvent);
		}
		m_currentFrame = (m_currentFrame + 1) % (engine::config::NumFrames);
	}

	void RenderContext::nextFrame()
	{
		flushCommandQueue();
	}

	void RenderContext::resetCommandAllocator()
	{
		m_graphicsCommandList.reset(0);

	}

	void RenderContext::startFrame()
	{
		m_graphicsCommandList.reset(m_currentFrame);
		m_graphicsCommandList->RSSetViewports(1, &m_viewport);
		m_graphicsCommandList->RSSetScissorRects(1, &m_scissorRect);
		m_currentFrame = m_swapChain.changeState(m_graphicsCommandList.getList(), ResourceState::RENDER_TARGET);
		auto rtvHandle = m_swapChain.getView(m_currentFrame);
		auto dsvHandle = m_dsvBuffer.dsv;
		const float clearColor[] = { 0.0f, 0.2f, 0.4f, 1.0f };
		m_graphicsCommandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);
		m_graphicsCommandList->ClearDepthStencilView(dsvHandle, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);
		m_graphicsCommandList->OMSetRenderTargets(1, &rtvHandle.HandleCPU, true, &dsvHandle.HandleCPU);
	}

	void RenderContext::endFrame()
	{
		m_swapChain.changeState(m_graphicsCommandList.getList(), ResourceState::PRESENT);
		ThrowIfFailed(m_graphicsCommandList->Close());
		ID3D12CommandList* ppCommandLists[] = { m_graphicsCommandList.getList()};
		m_graphicsQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);
		// Present the frame.
		m_swapChain->Present(0, 0);
		flushCommandQueue();
		ThrowIfFailed(m_device.getDevice()->GetDeviceRemovedReason());
	}

};
