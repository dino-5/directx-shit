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

using ViewID = int;

class DescriptorHeap
{
public:
	DescriptorHeap(UINT numDesc, DescriptorHeapType type, D3D12_DESCRIPTOR_HEAP_FLAGS flags=D3D12_DESCRIPTOR_HEAP_FLAG_NONE);
	DescriptorHeap() = default;
	void Init(UINT numDesc, DescriptorHeapType type, D3D12_DESCRIPTOR_HEAP_FLAGS flags=D3D12_DESCRIPTOR_HEAP_FLAG_NONE);
	ViewID CreateSRV(D3D12_SHADER_RESOURCE_VIEW_DESC& desc, ID3D12Resource* res);
	ViewID CreateCBV(D3D12_GPU_VIRTUAL_ADDRESS, unsigned int size);
	ViewID CreateRTV(ID3D12Resource* resource);
	ViewID CreateDSV();

	D3D12_CPU_DESCRIPTOR_HANDLE GetCPUHandle(ViewID index)const;
	D3D12_GPU_DESCRIPTOR_HANDLE GetGPUHandle(ViewID index)const;

	ID3D12DescriptorHeap* GetHeap();

private:
	ComPtr<ID3D12DescriptorHeap> m_heap;
	unsigned int heapSize;
	unsigned int descriptorSize;
	unsigned int currentDescriptorIndex=0;
};

#endif 

