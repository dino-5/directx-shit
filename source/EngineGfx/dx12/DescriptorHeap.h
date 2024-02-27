#ifndef DESCRIPTOR_HEAP_H
#define DESCRIPTOR_HEAP_H
#include <d3d12.h>
#include <memory>
#include "EngineGfx/dx12/d3dx12.h"
#include "EngineCommon/include/common.h"
#include "EngineCommon/include/types.h"

namespace engine::graphics
{

	enum class DescriptorHeapType
	{
		CBV_SRV_UAV = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV,
		SAMPLER     = D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER,
		RTV         = D3D12_DESCRIPTOR_HEAP_TYPE_RTV,
		DSV         = D3D12_DESCRIPTOR_HEAP_TYPE_DSV,	
	};

	using ViewID = u32;
	class Resource;

	class DescriptorHeap
	{
	public:
		friend class DescriptorHeapManager;
		void init(ID3D12Device* device, UINT numDesc, DescriptorHeapType type);

		void createCBV(ID3D12Device* device, Resource& res, D3D12_CONSTANT_BUFFER_VIEW_DESC& desc);
		void createSRV(ID3D12Device* device, Resource& res, D3D12_SHADER_RESOURCE_VIEW_DESC& desc);
		void createUAV(ID3D12Device* device, Resource& res, D3D12_UNORDERED_ACCESS_VIEW_DESC& desc);

		void createRTV(ID3D12Device* device, Resource& resource);
		void createDSV(ID3D12Device* device, Resource& res, D3D12_DEPTH_STENCIL_VIEW_DESC desc);

		D3D12_CPU_DESCRIPTOR_HANDLE getCPUHandle(ViewID index)const;
		D3D12_GPU_DESCRIPTOR_HANDLE getGPUHandle(ViewID index)const;

		ID3D12DescriptorHeap* getHeap();
		ID3D12DescriptorHeap** getHeapAddress() { return m_heap.GetAddressOf(); }
		u32 getDescriptorSize() { return descriptorSize; }

		void reset()
		{
			m_heap.Reset();
			heapSize = 0;
			descriptorSize = 0;
			currentDescriptorIndex = 0;
		}

	private:
		DescriptorHeap(ID3D12Device* device, UINT numDesc, DescriptorHeapType type);
		DescriptorHeap() = default;

		ComPtr<ID3D12DescriptorHeap> m_heap;
		u32 heapSize;
		u32 descriptorSize;
		u32 currentDescriptorIndex=0;
	};

	class DescriptorHeapManager
	{
	public:
		static void CreateSRVHeap(uint n);
		static void CreateRTVHeap(uint n);
		static void CreateDSVHeap(uint n);
		static inline DescriptorHeap CurrentSRVHeap;
		static inline DescriptorHeap CurrentRTVHeap;
		static inline DescriptorHeap CurrentDSVHeap;
	};

	struct DescriptorSRV
	{
		uint32_t HeapIndex;
		D3D12_CPU_DESCRIPTOR_HANDLE HandleCPU;
		D3D12_GPU_DESCRIPTOR_HANDLE HandleGPU;

		operator bool() const { return HandleCPU.ptr != 0; }
		u32 getDescriptorIndex() {
			u64 heapStartPtr = DescriptorHeapManager::CurrentSRVHeap.getHeap()
				->GetGPUDescriptorHandleForHeapStart().ptr;
			u32 descSize = DescriptorHeapManager::CurrentSRVHeap.getDescriptorSize();
            return static_cast<u32>((HandleGPU.ptr - heapStartPtr)/descSize); 
        }
		operator D3D12_CPU_DESCRIPTOR_HANDLE() const { return HandleCPU; }
		operator D3D12_GPU_DESCRIPTOR_HANDLE() const { return HandleGPU; }
	};
	using DescriptorUAV = DescriptorSRV;
	using DescriptorSampler = DescriptorSRV;
	struct DescriptorRTV
	{
		D3D12_CPU_DESCRIPTOR_HANDLE HandleCPU;

		operator bool() const { return HandleCPU.ptr != 0; }
		operator D3D12_CPU_DESCRIPTOR_HANDLE() const { return HandleCPU; }
	};
	using DescriptorDSV = DescriptorRTV;
	void PopulateDescriptorHeaps();

};
#endif 

