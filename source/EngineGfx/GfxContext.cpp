#include "EngineGfx/GfxContext.h"
#include "EngineGfx/dx12/Buffers.h"
#include "EngineCommon/util/Logger.h"
#include "dx12/dx12_includes.hpp"


GfxContext globalContext;

Fence createFence(Device device)
{
    Fence fence;
    device.createFence(&fence.fence);
    fence.fenceEvent = CreateEvent(nullptr,
                                           FALSE,
                                           FALSE, nullptr);
    if (fence.fenceEvent == nullptr)
    {
        ThrowIfFailed(HRESULT_FROM_WIN32(GetLastError()));
    }
    return fence;
}

void initGfxContext(u32 width, u32 height)
{
    memset(&globalContext.fenceValues, 0, sizeof(globalContext.fenceValues));
    Device& device = globalContext.device;
    globalContext.cmdList = CommandList(device);
    globalContext.cmdQueue = CommandQueue(device);
    
    DescriptorHeapManager::CreateDSVHeap(20);
    DescriptorHeapManager::CreateRTVHeap(100);
    DescriptorHeapManager::CreateSRVHeap(400);

    setWindowSize(width, height);

    globalContext.fence = createFence(globalContext.device);
}

void setWindowSize(u32 width, u32 height)
{
    globalContext.viewPort.TopLeftX = 0;
    globalContext.viewPort.TopLeftY = 0;
    globalContext.viewPort.Width = (float)width;
    globalContext.viewPort.Height = (float)height;
    globalContext.viewPort.MaxDepth = 1.0;
    globalContext.viewPort.MinDepth = .0;

    globalContext.scissorRect = { 0, 0, (long)(width), (long)(height) };
}

void flushGPU()
{
    globalContext.fence.waitForFence();
}

void executeAll()
{
    ID3D12GraphicsCommandList* list = globalContext.cmdList.list;
    ID3D12CommandQueue* queue = globalContext.cmdQueue.queue;

    ThrowIfFailed(list->Close());

    ID3D12CommandList* ppCommandLists[] = { list };
    queue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);
}

void waitForFrame(u32 index)
{
    ID3D12Fence* fence = globalContext.fence();
    u64 fenceValue = fence->GetCompletedValue();
    if (fenceValue < globalContext.fenceValues[index])
        waitForFence(index);
}

void waitForFence(u32 index)
{
    globalContext.fence.waitForFence(globalContext.fenceValues[index]);
}

void signal(u32 index)
{
    u32 value = globalContext.fenceValues[index] 
        = ++globalContext.fenceValue;
    ThrowIfFailed(globalContext.cmdQueue->Signal(
        globalContext.fence(), value));
}

void nextFrame(u32 nextFrameIndex)
{
    globalContext.currentFrameIndex = nextFrameIndex;
    waitForFrame();
}

void createView(GfxViewData data)
{
    globalContext.view.data = data;
    globalContext.view.buffer = ConstantBuffer(globalContext.device,
                                   globalContext.cmdList, 
                                   sizeof(GfxViewData), &data);
}

void updateView(GfxViewData data)
{
    globalContext.view.buffer.update(&data);
}

void startFrame()
{
    auto cmdList = globalContext.currentCmdList;
    cmdList->RSSetViewports(1, &globalContext.viewPort);
    cmdList->RSSetScissorRects(1, &globalContext.scissorRect);

    const float clearColor[] = { .0f, 0.0f, .0f, 1.0f };
    auto renderTarget = globalContext.currentRenderTarget->rtv;
    auto depthStencil = globalContext.currentDepthStencil->dsv;
    cmdList->ClearRenderTargetView(renderTarget.HandleCPU,
                                   clearColor, 0, nullptr);
    cmdList->ClearDepthStencilView(depthStencil.HandleCPU, 
                   D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL,
                   1.f, 0, 0, nullptr);

    cmdList->OMSetRenderTargets(1, &renderTarget.HandleCPU,
                                true, &depthStencil.HandleCPU);
}
