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

    ComPtr<IDXGIAdapter1> adapter;

    ComPtr<IDXGIFactory6> factory6;
    if (SUCCEEDED(m_factory->QueryInterface(IID_PPV_ARGS(&factory6))))
    {
        DXGI_GPU_PREFERENCE preference = requestHighPerformanceAdapter == true ?
             DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE : DXGI_GPU_PREFERENCE_UNSPECIFIED;
        CommandLine& cmdLine = CommandLine::GetCommandLine();
        bool listDevices = cmdLine.getValue(CommandLineOption::LIST_ADAPTER) == -1 ? false : true;
        i32 deviceIndex = cmdLine.getValue(CommandLineOption::ADAPTER);
        std::function<bool(u32)> selector;

        if (deviceIndex != -1)
        {
            auto adapterSelector = [deviceIndex, &adapter](u32 i) -> bool
            {
               if (i == static_cast<u32>(deviceIndex))
               {
                   if (!SUCCEEDED(D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_11_0, _uuidof(ID3D12Device), nullptr)))
                   {
                        engine::util::PrintError("can't create D3D12 device");
                        return false;
                   }
                   return true;
               }
               return false;
            };
            selector = adapterSelector;
        }
        else
        {
            auto adapterSelector = [&adapter](u32 i) -> bool
            {
                   if (!SUCCEEDED(D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_11_0, _uuidof(ID3D12Device), nullptr)))
                   {
                        return false;
                   }
                   return true;
            };
            selector = adapterSelector;
        }

        bool deviceSelected = false;
        for (u32 i=0;SUCCEEDED(factory6->EnumAdapterByGpuPreference(
                i, preference, IID_PPV_ARGS(&adapter)));++i)
        {
            DXGI_ADAPTER_DESC1 desc;
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
                    if(deviceSelected = selector(i))
                        engine::util::PrintInfo(
                        "Selected GPU -> {}", engine::util::to_string(desc.Description));
               }
            }
            else if (deviceSelected = selector(i))
            {
                engine::util::PrintInfo("Selected GPU -> {}", engine::util::to_string(desc.Description));
                break;
            }

        }
    }

    *ppAdapter = adapter.Detach();
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
        ThrowIfFailed(D3D12CreateDevice(
            adapter,
            D3D_FEATURE_LEVEL_12_0,
            IID_PPV_ARGS(&m_device)
        ));
    }
    device = this;
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

void Device::CreateFence(ComPtr<ID3D12Fence> &fence)
{
	ThrowIfFailed(m_device->CreateFence(0, D3D12_FENCE_FLAG_NONE,
		IID_PPV_ARGS(&fence)));
}
