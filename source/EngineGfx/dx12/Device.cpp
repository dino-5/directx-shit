#include <functional>
#include "Device.h"
#include "third_party/stb/stb_image.h"
#include "EngineCommon/util/Util.h"
#include "EngineCommon/util/Logger.h"
#include "EngineCommon/util/CommandLine.h"


void Device::GetHardwareAdapter(
    IDXGIAdapter1** ppAdapter,
    bool requestHighPerformanceAdapter)
{
    *ppAdapter = nullptr;

    IDXGIAdapter1* adapter;

    IDXGIFactory6* factory6;
    if (SUCCEEDED(m_factory->QueryInterface(IID_PPV_ARGS(&factory6))))
    {
        DXGI_GPU_PREFERENCE preference = requestHighPerformanceAdapter == true ?
             DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE : DXGI_GPU_PREFERENCE_UNSPECIFIED;
        CommandLine& cmdLine = CommandLine::GetCommandLine();
        bool listDevices = cmdLine.getValue(CommandLineOption::LIST_ADAPTER) == -1 ? false : true;
        i32 deviceIndex = cmdLine.getValue(CommandLineOption::ADAPTER);
        std::function<bool(u32)> selector;
        DXGI_ADAPTER_DESC1 desc;

        if (deviceIndex != -1)
        {
            auto adapterSelector = [deviceIndex, &adapter, &desc, this](u32 i) -> bool
            {
               if (i == static_cast<u32>(deviceIndex))
               {
                   if (!SUCCEEDED(D3D12CreateDevice(adapter, D3D_FEATURE_LEVEL_12_0,
                                 __uuidof(ID3D12Device), reinterpret_cast<void**>(&m_device))))
                   {
                        engine::util::PrintError("can't create D3D12 device");
                        return false;
                   }
                   engine::util::PrintInfo("Selected GPU -> {}", engine::util::to_string(desc.Description));
                   return true;
               }
               return false;
            };
            selector = adapterSelector;
        }
        else
        {
            auto adapterSelector = [&adapter, this, &desc](u32 i) -> bool
            {
                   if (!SUCCEEDED(D3D12CreateDevice(adapter, D3D_FEATURE_LEVEL_12_0,
                             __uuidof(ID3D12Device), reinterpret_cast<void**>(&m_device))))
                   {
                        return false;
                   }
                   engine::util::PrintInfo("Selected GPU -> {}", engine::util::to_string(desc.Description));
                   return true;
            };
            selector = adapterSelector;
        }

        bool deviceSelected = false;
        for (u32 i=0;SUCCEEDED(factory6->EnumAdapterByGpuPreference(
                i, preference, IID_PPV_ARGS(&adapter)));++i)
        {
            adapter->GetDesc1(&desc);
            if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
            {
                continue;
            }

            if (listDevices)
            {
               engine::util::PrintInfo("[{}] : {}", i, engine::util::to_string(desc.Description));
               if (!deviceSelected)
               {
                    deviceSelected = selector(i);
               }
            }
            else if (deviceSelected = selector(i))
            {
                break;
            }

        }
    }

    *ppAdapter = adapter;
}

void Device::Initialize()
{
    UINT dxgiFactoryFlags = 0;
    LogScope("Device::Initialize");

    {
        ComPtr<ID3D12Debug> debugController;
        if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController))))
        {
            debugController->EnableDebugLayer();

            // Enable additional debug layers.
            dxgiFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
        }
    }

    ThrowIfFailed(CreateDXGIFactory2(dxgiFactoryFlags, IID_PPV_ARGS(&m_factory)));
    {
        IDXGIAdapter1* adapter;
        GetHardwareAdapter(&adapter);
    }
    device = this;
    engine::util::PrintInfo("device initialized");
}

void Device::CreateCommandAllocator(ID3D12CommandAllocator* &alloc)
{
    ThrowIfFailed(m_device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&alloc)));
}

void Device::CreateCommandList(ID3D12GraphicsCommandList* &list, ID3D12CommandAllocator* &allocator)
{
	ThrowIfFailed(m_device->CreateCommandList(
		0,
		D3D12_COMMAND_LIST_TYPE_DIRECT,
        allocator,
		nullptr,                   // Initial PipelineStateObject
		IID_PPV_ARGS(&list)) );
    ThrowIfFailed(list->Close());
}

void Device::CreateFence(ID3D12Fence** fence)
{
	ThrowIfFailed(m_device->CreateFence(0, D3D12_FENCE_FLAG_NONE, 
        __uuidof(**fence), reinterpret_cast<void**>(fence)));
    u32 value = (*fence)->GetCompletedValue();
}
