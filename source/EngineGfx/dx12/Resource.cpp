#pragma once
#include "Resource.h"
#include "Device.h"
#include "EngineCommon/util/Util.h"
#include "EngineCommon/System/config.h"
#include "third_party/magic_enum/include/magic_enum.hpp"

void locCreateResource(ID3D12Device* device,
                       D3D12_RESOURCE_DESC desc,
                       engine::graphics::ResourceState createState,
                       D3D12_HEAP_TYPE aHeapType,
                       std::wstring_view name,
                       ID3D12Resource** resource,
                       D3D12_CLEAR_VALUE* val)
{
    CD3DX12_HEAP_PROPERTIES heapType = CD3DX12_HEAP_PROPERTIES(aHeapType);
    ThrowIfFailed(device->CreateCommittedResource(
        &heapType,
        D3D12_HEAP_FLAG_NONE,
        &desc,
        CastEnum(createState),
        val,
        IID_PPV_ARGS(resource)));
    (*resource)->SetName(name.data());
}

namespace engine::graphics
{
D3D12_RESOURCE_STATES CastEnum(ResourceState state)
{
    return static_cast<D3D12_RESOURCE_STATES>(state);
}

void Resource::initResource(const Device& device, ResourceDescription desc, DescriptorProperties descriptorDesc, D3D12_CLEAR_VALUE* val)
{
    m_id = ++s_counter;
    m_bufferSize = desc.dimension== D3D12_RESOURCE_DIMENSION_BUFFER ? desc.width : 0;
    m_currentState = desc.createState;
    m_heapType = desc.heapType;
    if (val) {
        m_clearValue = *val;
    }

    D3D12_RESOURCE_DESC resourceDesc = {};
    resourceDesc.Format = desc.format;
    resourceDesc.Width = desc.width;
    resourceDesc.Height = desc.height;
    resourceDesc.Dimension = desc.dimension;
    resourceDesc.Flags = CastType(desc.flags);
    resourceDesc.DepthOrArraySize = (u32)desc.depthOrArraySize;
    resourceDesc.MipLevels = 1;
    resourceDesc.SampleDesc.Count = 1;
    resourceDesc.SampleDesc.Quality = 0;
    resourceDesc.Layout = desc.dimension == D3D12_RESOURCE_DIMENSION_BUFFER ? 
        D3D12_TEXTURE_LAYOUT_ROW_MAJOR : D3D12_TEXTURE_LAYOUT_UNKNOWN;

    if (desc.name)
    {
        name = util::to_wstring(std::string(desc.name));
    }
    else
    {
        name = std::to_wstring(s_counter);
    }
    D3D12_CLEAR_VALUE* clearValue = m_clearValue.has_value() ? &m_clearValue.value() : nullptr;
    locCreateResource(device.device, resourceDesc, desc.createState, m_heapType, name, m_resource.ReleaseAndGetAddressOf(), clearValue);

    m_descriptorProps = descriptorDesc;
    createViews(device.device, m_descriptorProps);
}

void Resource::createViews(ID3D12Device* device, DescriptorProperties descriptorProps)
{
    if (descriptorProps.descriptor == DescriptorFlags::None)
        return;

    D3D12_RESOURCE_DESC desc = m_resource->GetDesc();
    DescriptorFlags descriptor = descriptorProps.descriptor;

    bool isRenderTarget = (descriptor & DescriptorFlags::RenderTarget) == DescriptorFlags::RenderTarget;
    bool isDepthStencil = (descriptor & DescriptorFlags::DepthStencil) == DescriptorFlags::DepthStencil;
    bool isShaderResource = (descriptor & DescriptorFlags::ShaderResource) == DescriptorFlags::ShaderResource;
    bool isUnorderedAccess = (descriptor & DescriptorFlags::UnorderedAccess) == DescriptorFlags::UnorderedAccess;
    bool isConstantBuffer = (descriptor & DescriptorFlags::ConstantBuffer) == DescriptorFlags::ConstantBuffer;

    if (isRenderTarget)
    {
        DescriptorHeapManager::CurrentRTVHeap.createRTV(device, *this);
    }
    if (isDepthStencil)
    {
        D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc;
        dsvDesc.Flags = D3D12_DSV_FLAG_NONE;
        dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
        dsvDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
        dsvDesc.Texture2D.MipSlice = 0;
        DescriptorHeapManager::CurrentDSVHeap.createDSV(device, *this, dsvDesc);
    }
    if (isShaderResource && desc.Dimension != D3D12_RESOURCE_DIMENSION_BUFFER)
    {
        D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
        srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
        srvDesc.Format = desc.Format;
        srvDesc.ViewDimension = descriptorProps.viewDimension;
        srvDesc.Texture2D.MipLevels = 1;
        DescriptorHeapManager::CurrentSRVHeap.createSRV(device, *this, srvDesc);
    }
    if (isShaderResource && desc.Dimension == D3D12_RESOURCE_DIMENSION_BUFFER)
    {
        if (descriptorProps.numElements == 0)
            DebugBreak();
        D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
        srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
        srvDesc.Format = desc.Format;
        srvDesc.ViewDimension = descriptorProps.viewDimension;
        srvDesc.Buffer.StructureByteStride = descriptorProps.bufferStride;
        srvDesc.Buffer.FirstElement = 0;
        srvDesc.Buffer.NumElements = descriptorProps.numElements;
        DescriptorHeapManager::CurrentSRVHeap.createSRV(device, *this, srvDesc);
    }
    if (isUnorderedAccess)
    {
        D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
        uavDesc.Format = desc.Format;
        uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
        uavDesc.Texture2D.MipSlice = 0;
        DescriptorHeapManager::CurrentSRVHeap.createUAV(device, *this, uavDesc);
    }
    if (isConstantBuffer)
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
