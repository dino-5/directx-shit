#pragma once

#include "EngineCommon/include/defines.h"

#include "EngineCommon/math/Matrix.h"
#include "EngineCommon/System/config.h"
#include "EngineGfx/dx12/Device.h"
#include "EngineGfx/dx12/SwapChain.h"
#include "EngineGfx/dx12/PSO.h"
#include "EngineGfx/dx12/CommandQueue.h"
#include "EngineGfx/dx12/CommandList.h"
#include "EngineGfx/dx12/Resource.h"
#include "EngineGfx/dx12/DescriptorHeap.h"
#include "EngineGfx/dx12/d3dx12.h"
#include "EngineGfx/dx12/Buffers.h"

using namespace engine::math;
using namespace engine::graphics;

struct RenderSettings
{
    int width;
    int height;
};

struct GfxViewData
{
    Matrix4 viewMatrix;
    Matrix4 projectionMatrix;
    Vector3 cameraPos;
    float pad0;
    Vector3 cameraViewDir;
    float pad1;
    Vector3 cameraRightDir;
    float pad2;
    Vector3 cameraUpDir;
    float pad3;
    float fov;
};

struct GfxView
{
    GfxViewData data;
    ConstantBuffer buffer;
};

struct Fence
{
    ID3D12Fence* fence = nullptr;
    HANDLE fenceEvent;

    ID3D12Fence* operator()() { return fence; }

    void waitForFence(u64 fenceValue = 0)
    {
        fence->SetEventOnCompletion(fenceValue, fenceEvent);
        WaitForSingleObjectEx(fenceEvent, INFINITE, FALSE);
    }
};

Fence createFence();

struct GfxContext
{
    Device device;
    CommandList cmdList;
    CommandQueue cmdQueue;
    DescriptorHeap descriptorHeaps[(u32)(DescriptorHeapType::Count)];

    D3D12_VIEWPORT viewPort{};
    D3D12_RECT scissorRect{};
    Resource* currentRenderTarget;
    Resource* currentDepthStencil;
    GfxView view;
    std::vector<Resource*> resources;

    Fence fence;
    u32 fenceValues [config::NumFrames];
    u32 fenceValue = 0;

    u32 currentFrameIndex = 0;
    ID3D12GraphicsCommandList* currentCmdList = nullptr;

};

extern GfxContext globalContext;
void initGfxContext(u32 width, u32 height);
void setWindowSize(u32 width, u32 height);
void flushGPU();
void executeAll();
void nextFrame(u32 nextFrameIndex);
void waitForFrame(u32 index);
void waitForFence(u32 index);
void signal(u32 index);
void createView(GfxViewData data);
void updateView(GfxViewData data);

inline void resetList(u32 index)
{
    globalContext.currentCmdList = globalContext.cmdList.reset(index);
}

void startFrame();

inline void signal()
{
    signal(globalContext.currentFrameIndex);
}

inline void waitForFrame()
{
    waitForFrame(globalContext.currentFrameIndex);
}


