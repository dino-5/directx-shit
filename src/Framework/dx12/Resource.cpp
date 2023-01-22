#pragma once
#include "Resource.h"
#include "Device.h"
#include "include/Util.h"


D3D12_RESOURCE_STATES CastType(ResourceState state)
{
    return static_cast<D3D12_RESOURCE_STATES>(state);
}

Resource::Resource(DXGI_FORMAT format, uint width, uint height, uint depthOrArraySize, uint dimension, ResourceFlags flag, ResourceState createState,
    CD3DX12_HEAP_PROPERTIES heap_type, ResourceDescriptorFlags descriptor,D3D12_CLEAR_VALUE* val)
{
    Init(format, width, height, depthOrArraySize, dimension, flag, createState, heap_type, descriptor, val);
}

Resource::Resource(CD3DX12_RESOURCE_DESC desc, ResourceState createState, CD3DX12_HEAP_PROPERTIES heap_type, D3D12_CLEAR_VALUE* val) 
{
    Init(desc, createState, heap_type, val);
}

void Resource::Init(DXGI_FORMAT format, uint width, uint height, uint depthOrArraySize, uint dimension, ResourceFlags flag, ResourceState createState ,
    CD3DX12_HEAP_PROPERTIES heap_type, ResourceDescriptorFlags descriptor, D3D12_CLEAR_VALUE* val )
{
    m_currentState = createState;
    D3D12_RESOURCE_DESC resourceDesc= {};
    resourceDesc.MipLevels = 1;
    resourceDesc.Format = format;
    resourceDesc.Width = width;
    resourceDesc.Height = height;
    resourceDesc.Flags = CastType(flag);
    resourceDesc.DepthOrArraySize = depthOrArraySize;
    resourceDesc.SampleDesc.Count = 1;
    resourceDesc.SampleDesc.Quality = 0;
    resourceDesc.Dimension = static_cast<D3D12_RESOURCE_DIMENSION>(dimension);

    ThrowIfFailed(Device::GetDevice()->CreateCommittedResource(
        &heap_type,
        D3D12_HEAP_FLAG_NONE,
        &resourceDesc,
        CastType(createState),
        val,
        IID_PPV_ARGS(&m_resource)));
    CreateViews(descriptor);
}

void Resource::CreateViews(ResourceDescriptorFlags descriptors)
{
    if ( (descriptors & ResourceDescriptorFlags::RenderTarget) == ResourceDescriptorFlags::RenderTarget)
    {
		DescriptorHeapManager::CurrentRTVHeap->CreateRTV(*this);
    }
    if ( (descriptors & ResourceDescriptorFlags::DepthStencil) == ResourceDescriptorFlags::DepthStencil)
    {
		D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc;
		dsvDesc.Flags = D3D12_DSV_FLAG_NONE;
		dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
		dsvDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
		dsvDesc.Texture2D.MipSlice = 0;
        DescriptorHeapManager::CurrentDSVHeap->CreateDSV(*this, dsvDesc);
    }

    if ( (descriptors & ResourceDescriptorFlags::ShaderResource) == ResourceDescriptorFlags::ShaderResource)
    {
		D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
		srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		srvDesc.Format = m_resource->GetDesc().Format;
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MipLevels = 1;
        DescriptorHeapManager::CurrentSRVHeap->CreateSRV(*this, srvDesc);
    }
    if ((descriptors & ResourceDescriptorFlags::UnorderedAccess) == ResourceDescriptorFlags::UnorderedAccess)
    {
		D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
		uavDesc.Format = m_resource->GetDesc().Format;
		uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
		uavDesc.Texture2D.MipSlice = 0;
        DescriptorHeapManager::CurrentSRVHeap->CreateUAV(*this, uavDesc );
    }
}

void Resource::Init(CD3DX12_RESOURCE_DESC desc, ResourceState createState , CD3DX12_HEAP_PROPERTIES heap_type, D3D12_CLEAR_VALUE* val )
{
    m_currentState = createState;
    ThrowIfFailed(Device::GetDevice()->CreateCommittedResource(
        &heap_type,
        D3D12_HEAP_FLAG_NONE,
        &desc,
        CastType(createState),
        val,
        IID_PPV_ARGS(&m_resource)));
}

void Resource::Transition(ComPtr<ID3D12GraphicsCommandList> cmdList, ResourceState state)
{
    if (state == m_currentState)
        return;
    auto transitionDesc = CD3DX12_RESOURCE_BARRIER::Transition(m_resource.Get(),
        CastType(m_currentState), CastType(state));
    cmdList->ResourceBarrier(1, &transitionDesc);
    m_currentState = state;
}
