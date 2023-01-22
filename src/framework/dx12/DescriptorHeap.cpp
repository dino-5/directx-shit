#include "DescriptorHeap.h"
#include "Device.h"
#include "../include/d3dx12.h"
#include "dx12/Resource.h"
#include "include/Util.h"

DescriptorHeap::DescriptorHeap(UINT numDesc, DescriptorHeapType type, D3D12_DESCRIPTOR_HEAP_FLAGS flags):heapSize(numDesc)
{
    D3D12_DESCRIPTOR_HEAP_DESC cbvHeapDesc;
    cbvHeapDesc.NumDescriptors = numDesc;
    cbvHeapDesc.Type = static_cast<D3D12_DESCRIPTOR_HEAP_TYPE>(type);
    cbvHeapDesc.Flags = flags;
	cbvHeapDesc.NodeMask = 0;
    ID3D12Device* device = Device::GetDevice();
    ThrowIfFailed(device->CreateDescriptorHeap(&cbvHeapDesc,
        IID_PPV_ARGS(&m_heap)));
    descriptorSize = device->GetDescriptorHandleIncrementSize(cbvHeapDesc.Type);
}

void DescriptorHeap::Init(UINT numDesc, DescriptorHeapType type, D3D12_DESCRIPTOR_HEAP_FLAGS flags)
{
    heapSize = numDesc;
    D3D12_DESCRIPTOR_HEAP_DESC cbvHeapDesc;
    cbvHeapDesc.NumDescriptors = numDesc;
    cbvHeapDesc.Type = static_cast<D3D12_DESCRIPTOR_HEAP_TYPE>(type);
    cbvHeapDesc.Flags = flags;
	cbvHeapDesc.NodeMask = 0;
    ID3D12Device* device = Device::GetDevice();
    ThrowIfFailed(device->CreateDescriptorHeap(&cbvHeapDesc,
        IID_PPV_ARGS(&m_heap)));
    descriptorSize = device->GetDescriptorHandleIncrementSize(cbvHeapDesc.Type);
}


void DescriptorHeap::CreateSRV(Resource& res, D3D12_SHADER_RESOURCE_VIEW_DESC& desc)
{
    res.srv.HandleCPU = GetCPUHandle(currentDescriptorIndex); 
    res.srv.HandleGPU = GetGPUHandle(currentDescriptorIndex);
    res.srv.HeapIndex = currentDescriptorIndex;
    Device::GetDevice()->CreateShaderResourceView(res, &desc, res.srv.HandleCPU);
    currentDescriptorIndex++;
}

void DescriptorHeap::CreateUAV(Resource& res, D3D12_UNORDERED_ACCESS_VIEW_DESC& desc)
{
    res.uav.HandleCPU = GetCPUHandle(currentDescriptorIndex); 
    res.uav.HandleGPU = GetGPUHandle(currentDescriptorIndex);
    Device::GetDevice()->CreateUnorderedAccessView(res, nullptr, &desc, res.uav.HandleCPU);
    currentDescriptorIndex++;
}


void DescriptorHeap::CreateRTV(Resource& res)
{
    res.rtv.HandleCPU = GetCPUHandle(currentDescriptorIndex); 
    Device::GetDevice()->CreateRenderTargetView(res, nullptr, res.rtv.HandleCPU);
    currentDescriptorIndex++;
}

void DescriptorHeap::CreateDSV(Resource& res, D3D12_DEPTH_STENCIL_VIEW_DESC desc)
{
    res.dsv.HandleCPU = GetCPUHandle(currentDescriptorIndex); 
	Device::GetDevice()->CreateDepthStencilView(res, &desc, res.dsv.HandleCPU);
    currentDescriptorIndex++;
}

D3D12_CPU_DESCRIPTOR_HANDLE DescriptorHeap::GetCPUHandle(ViewID index)const
{
	auto handle = CD3DX12_CPU_DESCRIPTOR_HANDLE(m_heap->GetCPUDescriptorHandleForHeapStart());
	handle.Offset(index, descriptorSize);
    return handle;
}

D3D12_GPU_DESCRIPTOR_HANDLE DescriptorHeap::GetGPUHandle(ViewID index)const
{
	auto handle = CD3DX12_GPU_DESCRIPTOR_HANDLE(m_heap->GetGPUDescriptorHandleForHeapStart());
	handle.Offset(index, descriptorSize);
    return handle;
}

ID3D12DescriptorHeap* DescriptorHeap::GetHeap()
{
    return m_heap.Get();
}
