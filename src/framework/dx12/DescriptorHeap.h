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

class DescriptorHeap;

class DescriptorHeapManager
{
public:
	static void SetSRVHeap(DescriptorHeap& heap)
	{
		CurrentSRVHeap = &heap;
	}

	static inline DescriptorHeap* CurrentSRVHeap = nullptr;
	static inline DescriptorHeap* CurrentRTVHeap = nullptr;
	static inline DescriptorHeap* CurrentDSVHeap = nullptr;
};

struct DescriptorRTV
{
	D3D12_CPU_DESCRIPTOR_HANDLE HandleCPU;

	operator bool() const { return HandleCPU.ptr != 0; }
	operator D3D12_CPU_DESCRIPTOR_HANDLE() const { return HandleCPU; }
};
using DescriptorDSV = DescriptorRTV;

struct DescriptorSRV
{
	uint32_t HeapIndex;
	D3D12_CPU_DESCRIPTOR_HANDLE HandleCPU;
	D3D12_GPU_DESCRIPTOR_HANDLE HandleGPU;

	operator bool() const { return HandleCPU.ptr != 0; }
	operator D3D12_CPU_DESCRIPTOR_HANDLE() const { return HandleCPU; }
	operator D3D12_GPU_DESCRIPTOR_HANDLE() const { return HandleGPU; }
};
using DescriptorUAV = DescriptorSRV;
using DescriptorSampler = DescriptorSRV;


class Resource;

class DescriptorHeap
{
public:
	DescriptorHeap(UINT numDesc, DescriptorHeapType type, D3D12_DESCRIPTOR_HEAP_FLAGS flags=D3D12_DESCRIPTOR_HEAP_FLAG_NONE);
	DescriptorHeap() = default;
	void Init(UINT numDesc, DescriptorHeapType type, D3D12_DESCRIPTOR_HEAP_FLAGS flags=D3D12_DESCRIPTOR_HEAP_FLAG_NONE);

	void CreateSRV(Resource& res, D3D12_SHADER_RESOURCE_VIEW_DESC& desc);
	void CreateUAV(Resource& res, D3D12_UNORDERED_ACCESS_VIEW_DESC& desc);

	void CreateRTV(Resource& resource);
	void CreateDSV(Resource& res, D3D12_DEPTH_STENCIL_VIEW_DESC desc);

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

