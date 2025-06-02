#ifndef DEVICE_H
#define DEVICE_H
#include <d3d12.h>
#include <windows.h>
#include <dxgi1_6.h>
#include <D3Dcompiler.h>
#include <DirectXMath.h>
#include "EngineCommon/include/defines.h"
#include "EngineCommon/include/types.h"
#include "EngineGfx/dx12/d3dx12.h"
#include "EngineGfx/dx12/dx12_includes.hpp"

namespace engine::graphics {

class Device
{
public:
    static inline Device* s_device = nullptr;
    Device() ;
    void createCommandList(ID3D12GraphicsCommandList*& list,
                           ID3D12CommandAllocator*& allocator);
    void createCommandAllocator(ID3D12CommandAllocator*&);
    void createFence(ID3D12Fence**);

    bool checkForFeatureSupport(DXGI_FEATURE feature=
                                DXGI_FEATURE_PRESENT_ALLOW_TEARING);

    void reset() { 
        factory->Release(); factory = nullptr;
        device->Release(); device = nullptr;
    }

    ID3D12Device* operator()() { return device; }

    void getHardwareAdapter(
        IDXGIAdapter1** ppAdapter,
        bool requestHighPerformanceAdapter = true);

    IDXGIFactory4* factory = nullptr;
    ID3D12Device* device = nullptr;
};

};

#endif

