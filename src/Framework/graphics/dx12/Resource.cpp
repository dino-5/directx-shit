#pragma once
#include "Resource.h"
#include "Device.h"
#include "Framework/util/Util.h"
#include "Framework/System/config.h"

namespace engine::graphics
{
	D3D12_RESOURCE_STATES CastType(ResourceState state)
	{
		return static_cast<D3D12_RESOURCE_STATES>(state);
	}

	Resource::Resource(ID3D12Device* device, ResourceDescription desc, D3D12_CLEAR_VALUE* val)
	{
		Init(device, desc, val);
	}

	Resource::Resource(ID3D12Device* device, CD3DX12_RESOURCE_DESC desc, ResourceState createState, CD3DX12_HEAP_PROPERTIES heap_type)
	{
		Init(device, desc, createState, heap_type);
	}

	void Resource::Init(ID3D12Device* device, ResourceDescription desc, D3D12_CLEAR_VALUE* val)
	{
		m_currentState = desc.createState;
		D3D12_RESOURCE_DESC resourceDesc = {};
		resourceDesc.MipLevels = 1;
		resourceDesc.Format = desc.format;
		resourceDesc.Width = desc.width;
		resourceDesc.Height = desc.height;
		resourceDesc.Flags = CastType(desc.flags);
		resourceDesc.DepthOrArraySize = desc.depthOrArraySize;
		resourceDesc.SampleDesc.Count = 1;
		resourceDesc.SampleDesc.Quality = 0;
		resourceDesc.Dimension = desc.dimension;

		ThrowIfFailed(device->CreateCommittedResource(
			&desc.heap_type,
			D3D12_HEAP_FLAG_NONE,
			&resourceDesc,
			CastType(desc.createState),
			val,
			IID_PPV_ARGS(&m_resource)));
		CreateViews(device, desc.descriptor);
	}

	void Resource::CreateViews(ID3D12Device* device, ResourceDescriptorFlags descriptors)
	{
		if ((descriptors & ResourceDescriptorFlags::RenderTarget) == ResourceDescriptorFlags::RenderTarget)
		{
			DescriptorHeapManager::CurrentRTVHeap->CreateRTV(device, *this);
		}
		if ((descriptors & ResourceDescriptorFlags::DepthStencil) == ResourceDescriptorFlags::DepthStencil)
		{
			D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc;
			dsvDesc.Flags = D3D12_DSV_FLAG_NONE;
			dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
			dsvDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
			dsvDesc.Texture2D.MipSlice = 0;
			DescriptorHeapManager::CurrentDSVHeap->CreateDSV(device, *this, dsvDesc);
		}

		if ((descriptors & ResourceDescriptorFlags::ShaderResource) == ResourceDescriptorFlags::ShaderResource)
		{
			D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
			srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
			srvDesc.Format = m_resource->GetDesc().Format;
			srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
			srvDesc.Texture2D.MipLevels = 1;
			DescriptorHeapManager::CurrentSRVHeap->CreateSRV(device, *this, srvDesc);
		}
		if ((descriptors & ResourceDescriptorFlags::UnorderedAccess) == ResourceDescriptorFlags::UnorderedAccess)
		{
			D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
			uavDesc.Format = m_resource->GetDesc().Format;
			uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
			uavDesc.Texture2D.MipSlice = 0;
			DescriptorHeapManager::CurrentSRVHeap->CreateUAV(device, *this, uavDesc);
		}
	}

	void Resource::Init(ID3D12Device* device, CD3DX12_RESOURCE_DESC desc, ResourceState createState, CD3DX12_HEAP_PROPERTIES heap_type)
	{
		m_currentState = createState;
		ThrowIfFailed(device->CreateCommittedResource(
			&heap_type,
			D3D12_HEAP_FLAG_NONE,
			&desc,
			CastType(createState),
			nullptr,
			IID_PPV_ARGS(&m_resource)));
	}

	void Resource::InitAsConstantBuffer(ID3D12Device* device, uint sizeOfBuffer)
	{
		CD3DX12_RESOURCE_DESC resourceDesc = CD3DX12_RESOURCE_DESC::Buffer(sizeOfBuffer);
		CD3DX12_HEAP_PROPERTIES heap_type = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
		device->CreateCommittedResource(&heap_type,
			D3D12_HEAP_FLAG_NONE,
			&resourceDesc,
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&m_resource));

		// todo add creation of view descriptor

	}

	void Resource::Transition(ID3D12GraphicsCommandList* cmdList, ResourceState state)
	{
		if (state == m_currentState)
			return;
		auto transitionDesc = CD3DX12_RESOURCE_BARRIER::Transition(m_resource,
			CastType(m_currentState), CastType(state));
		cmdList->ResourceBarrier(1, &transitionDesc);
		m_currentState = state;
	}

	void Resource::PopulateResources(ID3D12Device* device)
	{
		allResources.reserve(3);
		// for two constants
		{
			uint size = 4 * 4 * sizeof(float);
			ResourceVector vec;
			vec.reserve(6);
			for (int i = 0; i < engine::config::NumFrames; i++)
			{
				ResourceDescription desc;
				for (uint i = 0; i < engine::config::NumFrames; i++)
					vec[i].Init(device, CD3DX12_RESOURCE_DESC::Buffer(engine::util::d3dUtil::CalcConstantBufferByteSize(size)),
						ResourceState::GENERIC_READ_STATE, CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD));
			}
			allResources.push_back({ "twoConst", vec });
		}
	}
};
