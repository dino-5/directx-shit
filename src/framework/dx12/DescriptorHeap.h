#ifndef DESCRIPTOR_HEAP_H
#define DESCRIPTOR_HEAP_H
#include <d3d12.h>
#include "../include/d3dx12.h"
#include "../include/common.h"

enum class DescriptorHeapType
{
	CBV_SRV_UAV = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV,
	SAMPLER     = D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER,
	RTV         = D3D12_DESCRIPTOR_HEAP_TYPE_RTV,
	DSV         = D3D12_DESCRIPTOR_HEAP_TYPE_DSV,	
};


class DescriptorHeap
{
public:
	DescriptorHeap(UINT numDesc, DescriptorHeapType type, D3D12_DESCRIPTOR_HEAP_FLAGS flags=D3D12_DESCRIPTOR_HEAP_FLAG_NONE);

private:
	ComPtr<ID3D12DescriptorHeap> m_heap;
};

#endif 

