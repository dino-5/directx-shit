#include "EngineGfx/dx12/DescriptorHeap.h"
#include "EngineGfx/dx12/Device.h"
#include "EngineGfx/dx12/d3dx12.h"
#include "EngineGfx/dx12/Resource.h"
#include "EngineCommon/util/Util.h"

namespace engine::graphics
{
	DescriptorHeap::DescriptorHeap(ID3D12Device* device, UINT numDesc, DescriptorHeapType type)
	{
		init(device,numDesc, type);
	}

	void DescriptorHeap::init(ID3D12Device* device, UINT numDesc, DescriptorHeapType type)
	{
		heapSize = numDesc;
		D3D12_DESCRIPTOR_HEAP_DESC cbvHeapDesc;
		cbvHeapDesc.NumDescriptors = numDesc;
		cbvHeapDesc.Type = static_cast<D3D12_DESCRIPTOR_HEAP_TYPE>(type);
		cbvHeapDesc.Flags = (type==DescriptorHeapType::CBV_SRV_UAV) ? D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE : D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		cbvHeapDesc.NodeMask = 0;
		ThrowIfFailed(device->CreateDescriptorHeap(&cbvHeapDesc,
			IID_PPV_ARGS(&m_heap)));
		descriptorSize = device->GetDescriptorHandleIncrementSize(cbvHeapDesc.Type);
	}

	void DescriptorHeap::createCBV(ID3D12Device* device, Resource& res, D3D12_CONSTANT_BUFFER_VIEW_DESC& desc)
	{
		res.srv.HandleCPU = getCPUHandle(currentDescriptorIndex); 
		res.srv.HandleGPU = getGPUHandle(currentDescriptorIndex);
		res.srv.HeapIndex = currentDescriptorIndex++;
		device->CreateConstantBufferView(&desc, res.srv.HandleCPU);
	}

	void DescriptorHeap::createSRV(ID3D12Device* device, Resource& res, D3D12_SHADER_RESOURCE_VIEW_DESC& desc)
	{
		res.srv.HandleCPU = getCPUHandle(currentDescriptorIndex); 
		res.srv.HandleGPU = getGPUHandle(currentDescriptorIndex);
		res.srv.HeapIndex = currentDescriptorIndex++;
		device->CreateShaderResourceView(res, &desc, res.srv.HandleCPU);
	}

	void DescriptorHeap::createUAV(ID3D12Device* device, Resource& res, D3D12_UNORDERED_ACCESS_VIEW_DESC& desc)
	{
		res.uav.HandleCPU = getCPUHandle(currentDescriptorIndex); 
		res.uav.HandleGPU = getGPUHandle(currentDescriptorIndex);
		res.uav.HeapIndex = currentDescriptorIndex++;
		device->CreateUnorderedAccessView(res, nullptr, &desc, res.uav.HandleCPU);
    }

	void DescriptorHeap::createRTV(ID3D12Device* device, Resource& res)
	{
		res.rtv.HandleCPU = getCPUHandle(currentDescriptorIndex++); 
		device->CreateRenderTargetView(res, nullptr, res.rtv.HandleCPU);
	}

	void DescriptorHeap::createDSV(ID3D12Device* device, Resource& res, D3D12_DEPTH_STENCIL_VIEW_DESC desc)
	{
		res.dsv.HandleCPU = getCPUHandle(currentDescriptorIndex++); 
		device->CreateDepthStencilView(res, &desc, res.dsv.HandleCPU);
	}

	D3D12_CPU_DESCRIPTOR_HANDLE DescriptorHeap::getCPUHandle(ViewID index)const
	{
		auto handle = CD3DX12_CPU_DESCRIPTOR_HANDLE(m_heap->GetCPUDescriptorHandleForHeapStart());
		handle.Offset(index, descriptorSize);
		return handle;
	}

	D3D12_GPU_DESCRIPTOR_HANDLE DescriptorHeap::getGPUHandle(ViewID index)const
	{
		auto handle = CD3DX12_GPU_DESCRIPTOR_HANDLE(m_heap->GetGPUDescriptorHandleForHeapStart());
		handle.Offset(index, descriptorSize);
		return handle;
	}

	ID3D12DescriptorHeap* DescriptorHeap::getHeap()
	{
		return m_heap.Get();
	}

	void DescriptorHeapManager::CreateSRVHeap(uint n)
	{
		CurrentSRVHeap.init(Device::device->GetDevice(), n, DescriptorHeapType::CBV_SRV_UAV);
	}

	void DescriptorHeapManager::CreateRTVHeap(uint n)
	{
		CurrentRTVHeap.init(Device::device->GetDevice(), n, DescriptorHeapType::RTV);
	}

	void DescriptorHeapManager::CreateDSVHeap(uint n)
	{
		CurrentDSVHeap.init(Device::device->GetDevice(), n, DescriptorHeapType::DSV);
	}
	
	void PopulateDescriptorHeaps()
	{
		DescriptorHeapManager::CreateSRVHeap(10);
	}
};
