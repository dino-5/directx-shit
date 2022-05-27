#include "DescriptorHeap.h"
#include"Device.h"

DescriptorHeap::DescriptorHeap(UINT numDesc, DescriptorHeapType type, D3D12_DESCRIPTOR_HEAP_FLAGS flags)
{
    D3D12_DESCRIPTOR_HEAP_DESC cbvHeapDesc;
    cbvHeapDesc.NumDescriptors = numDesc;
    cbvHeapDesc.Type = static_cast<D3D12_DESCRIPTOR_HEAP_TYPE>(type);
    cbvHeapDesc.Flags = flags;
	cbvHeapDesc.NodeMask = 0;
    ThrowIfFailed(Device::GetDevice()->CreateDescriptorHeap(&cbvHeapDesc,
        IID_PPV_ARGS(&m_heap)));
}


