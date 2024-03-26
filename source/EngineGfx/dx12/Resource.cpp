#pragma once
#include "Resource.h"
#include "Device.h"
#include "EngineCommon/util/Util.h"
#include "EngineCommon/System/config.h"

namespace engine::graphics
{
	D3D12_RESOURCE_STATES CastEnum(ResourceState state)
	{
		return static_cast<D3D12_RESOURCE_STATES>(state);
	}

	Resource::Resource(ID3D12Device* device, ResourceDescription desc, D3D12_CLEAR_VALUE* val)
	{
		init(device, desc, val);
	}

	void Resource::init(ID3D12Device* device, ResourceDescription desc, D3D12_CLEAR_VALUE* val)
	{
		static u32 resIndex = 0;
		m_bufferSize = desc.dimension== D3D12_RESOURCE_DIMENSION_BUFFER ? desc.width : 0;
		m_currentState = desc.createState;
		D3D12_RESOURCE_DESC resourceDesc = {};
		resourceDesc.Format = desc.format;
		resourceDesc.Width = desc.width;
		resourceDesc.Height = desc.height;
		resourceDesc.Dimension = desc.dimension;
		resourceDesc.Flags = CastType(desc.flags);
		resourceDesc.DepthOrArraySize = desc.depthOrArraySize;
		resourceDesc.MipLevels = 1;
		resourceDesc.SampleDesc.Count = 1;
		resourceDesc.SampleDesc.Quality = 0;
		resourceDesc.Layout = desc.dimension == D3D12_RESOURCE_DIMENSION_BUFFER ? 
			D3D12_TEXTURE_LAYOUT_ROW_MAJOR : D3D12_TEXTURE_LAYOUT_UNKNOWN;

		CD3DX12_HEAP_PROPERTIES heapType = CD3DX12_HEAP_PROPERTIES(desc.heapType);
		ThrowIfFailed(device->CreateCommittedResource(
			&heapType,
			D3D12_HEAP_FLAG_NONE,
			&resourceDesc,
			CastEnum(desc.createState),
			val,
			IID_PPV_ARGS(&m_resource)));
		if (desc.name)
		{
            name = util::to_wstring(std::string(desc.name));
		}
		else
		{
			name = std::to_wstring(resIndex++);
		}
        m_resource->SetName(name.c_str());
		createViews(device, desc.descriptor);
        util::PrintInfo("created resource {}", !name.empty() ? util::to_string(name) : "");
	}

	void Resource::createViews(ID3D12Device* device, DescriptorProperties descriptors)
	{
		D3D12_RESOURCE_DESC desc = m_resource->GetDesc();
		DescriptorFlags descriptor = descriptors.descriptor;
		if ((descriptor & DescriptorFlags::RenderTarget) == DescriptorFlags::RenderTarget)
		{
			DescriptorHeapManager::CurrentRTVHeap.createRTV(device, *this);
		}
		if ((descriptor & DescriptorFlags::DepthStencil) == DescriptorFlags::DepthStencil)
		{
			D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc;
			dsvDesc.Flags = D3D12_DSV_FLAG_NONE;
			dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
			dsvDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
			dsvDesc.Texture2D.MipSlice = 0;
			DescriptorHeapManager::CurrentDSVHeap.createDSV(device, *this, dsvDesc);
		}

		if ((descriptor & DescriptorFlags::ShaderResource) == DescriptorFlags::ShaderResource && 
			desc.Dimension != D3D12_RESOURCE_DIMENSION_BUFFER)
		{
			D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
			srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
			srvDesc.Format = desc.Format;
			srvDesc.ViewDimension = descriptors.viewDimension;
			srvDesc.Texture2D.MipLevels = 1;
			DescriptorHeapManager::CurrentSRVHeap.createSRV(device, *this, srvDesc);
		}
		if ((descriptor & DescriptorFlags::ShaderResource) == DescriptorFlags::ShaderResource &&
			desc.Dimension == D3D12_RESOURCE_DIMENSION_BUFFER)
		{
            if (descriptors.numElements == 0)
                DebugBreak();
			D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
			srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
			srvDesc.Format = desc.Format;
			srvDesc.ViewDimension = descriptors.viewDimension;
			srvDesc.Buffer.StructureByteStride = descriptors.bufferStride;
			srvDesc.Buffer.FirstElement = 0;
			srvDesc.Buffer.NumElements = descriptors.numElements;
			DescriptorHeapManager::CurrentSRVHeap.createSRV(device, *this, srvDesc);
		}
		if ((descriptor & DescriptorFlags::UnorderedAccess) == DescriptorFlags::UnorderedAccess)
		{
			D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
			uavDesc.Format = desc.Format;
			uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
			uavDesc.Texture2D.MipSlice = 0;
			DescriptorHeapManager::CurrentSRVHeap.createUAV(device, *this, uavDesc);
		}
		if ((descriptor & DescriptorFlags::ConstantBuffer) == DescriptorFlags::ConstantBuffer)
		{
			D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
			cbvDesc.BufferLocation = m_resource->GetGPUVirtualAddress();
			cbvDesc.SizeInBytes = m_bufferSize;
			DescriptorHeapManager::CurrentSRVHeap.createCBV(device, *this, cbvDesc);
		}
	}


	void Resource::transition(ID3D12GraphicsCommandList* cmdList, ResourceState state)
	{
		if (state == m_currentState)
			return;
		auto transitionDesc = CD3DX12_RESOURCE_BARRIER::Transition(m_resource.Get(),
			CastEnum(m_currentState), CastEnum(state));
		cmdList->ResourceBarrier(1, &transitionDesc);
		m_currentState = state;
	}

};
