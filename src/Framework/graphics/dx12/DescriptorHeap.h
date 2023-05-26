#ifndef DESCRIPTOR_HEAP_H
#define DESCRIPTOR_HEAP_H
#include <d3d12.h>
#include <memory>
#include "Framework/include/d3dx12.h"
#include "Framework/include/common.h"
#include "Framework/include/types.h"

namespace engine::graphics
{

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
			CurrentSRVHeap.reset(&heap);
		}
		static void CreateSRVHeap(uint n);
		static void CreateRTVHeap(uint n);
		static void CreateDSVHeap(uint n);
		static inline std::unique_ptr<DescriptorHeap> CurrentSRVHeap = nullptr;
		static inline std::unique_ptr<DescriptorHeap> CurrentRTVHeap = nullptr;
		static inline std::unique_ptr<DescriptorHeap> CurrentDSVHeap = nullptr;
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
		DescriptorHeap(ID3D12Device* device, UINT numDesc, DescriptorHeapType type);
		DescriptorHeap() = default;
		void Init(ID3D12Device* device, UINT numDesc, DescriptorHeapType type);

		void CreateSRV(ID3D12Device* device, Resource& res, D3D12_SHADER_RESOURCE_VIEW_DESC& desc);
		void CreateUAV(ID3D12Device* device, Resource& res, D3D12_UNORDERED_ACCESS_VIEW_DESC& desc);

		void CreateRTV(ID3D12Device* device, Resource& resource);
		void CreateDSV(ID3D12Device* device, Resource& res, D3D12_DEPTH_STENCIL_VIEW_DESC desc);

		D3D12_CPU_DESCRIPTOR_HANDLE GetCPUHandle(ViewID index)const;
		D3D12_GPU_DESCRIPTOR_HANDLE GetGPUHandle(ViewID index)const;

		ID3D12DescriptorHeap* GetHeap();

		void Reset()
		{
			m_heap.Reset();
			heapSize = 0;
			descriptorSize = 0;
			currentDescriptorIndex = 0;
		}

	private:
		ComPtr<ID3D12DescriptorHeap> m_heap;
		unsigned int heapSize;
		unsigned int descriptorSize;
		unsigned int currentDescriptorIndex=0;
	};

	void PopulateDescriptorHeaps();

};
#endif 

