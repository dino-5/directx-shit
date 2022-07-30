#include "DescriptorHeap.h"
#include "Device.h"
#include "../include/d3dx12.h"

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


ViewID DescriptorHeap::CreateSRV(D3D12_SHADER_RESOURCE_VIEW_DESC& desc, ID3D12Resource* res)
{
    auto handle = GetCPUHandle(currentDescriptorIndex); 
    Device::GetDevice()->CreateShaderResourceView(res, &desc, handle);
    return currentDescriptorIndex++;
}

ViewID DescriptorHeap::CreateCBV(D3D12_GPU_VIRTUAL_ADDRESS address, unsigned int size)
{
	D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc;
	cbvDesc.BufferLocation = address;
	cbvDesc.SizeInBytes = size;

    auto handle = GetCPUHandle(currentDescriptorIndex);
	Device::GetDevice()->CreateConstantBufferView(&cbvDesc, handle);
    return currentDescriptorIndex++;
}

ViewID DescriptorHeap::CreateRTV(ID3D12Resource* resource)
{
    auto handle = GetCPUHandle(currentDescriptorIndex);
    Device::GetDevice()->CreateRenderTargetView(resource, nullptr, handle);
    return currentDescriptorIndex++;
}

ViewID DescriptorHeap::CreateDSV(ID3D12Resource* res, D3D12_DEPTH_STENCIL_VIEW_DESC desc)
{
    auto handle = GetCPUHandle(currentDescriptorIndex);
	Device::GetDevice()->CreateDepthStencilView(res, &desc, handle);
    return currentDescriptorIndex++;
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
