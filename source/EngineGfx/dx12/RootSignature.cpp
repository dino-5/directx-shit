#include "RootSignature.h"
#include "Device.h"
#include "EngineCommon/util/Util.h"
#include "EngineCommon/util/Logger.h"
#include <format>

namespace engine::graphics
{

DescriptorRange::DescriptorRange(DescriptorRangeType type, uint numberOfDescriptors, uint baseShaderRegister,
    uint registerSpace, uint offset)
{
    m_range.BaseShaderRegister = baseShaderRegister;
    m_range.NumDescriptors = numberOfDescriptors;
    m_range.RegisterSpace = registerSpace;
    m_range.RangeType = static_cast<D3D12_DESCRIPTOR_RANGE_TYPE>(type);
    m_range.OffsetInDescriptorsFromTableStart = offset;
}

RootParameter CreateTable(uint numberOfDescriptorRanges,
                          DescriptorRange& range,
                          ShaderVisibility vis)
{
    RootParameter val;
    val.argument.ParameterType = CastType(RootParameterType::TABLE);
    val.argument.ShaderVisibility = CastType(vis);
    val.argument.DescriptorTable.NumDescriptorRanges = 1;
    val.argument.DescriptorTable.pDescriptorRanges = &range.m_range;
    return val;
}

RootParameter CreateConstants(UINT num32BitValues,
                              UINT shaderRegister,
    UINT registerSpace,
    ShaderVisibility visibility)
{
    RootParameter val;
    val.argument.Constants.ShaderRegister= shaderRegister;
    val.argument.Constants.RegisterSpace = registerSpace;
    val.argument.Constants.Num32BitValues = num32BitValues;
    val.argument.ShaderVisibility = CastType(visibility);
    val.argument.ParameterType = CastType(RootParameterType::CONSTANT);
    return val;
}

RootParameter CreateDescriptor(
                    uint shaderRegister,
                    uint registerSpace,
                    RootParameterType type,
                    ShaderVisibility vis)
{
    RootParameter val;
    val.argument.ParameterType = CastType(type);
    val.argument.Descriptor.RegisterSpace = registerSpace;
    val.argument.Descriptor.ShaderRegister = shaderRegister;
    val.argument.ShaderVisibility = CastType(vis);
    return val;
}

std::vector<D3D12_ROOT_PARAMETER> GetParameters(RootParameters& vec)
{
    std::vector<D3D12_ROOT_PARAMETER> result(vec.size());
    for (int i = 0; i < vec.size(); i++)
        result[i] = vec[i];
    return result;
}

D3D12_STATIC_SAMPLER_DESC GetSampler()
{
    D3D12_STATIC_SAMPLER_DESC sampler = {};
    sampler.Filter = D3D12_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR;
    sampler.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    sampler.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    sampler.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    sampler.MipLODBias = 0;
    sampler.MaxAnisotropy = 0;
    sampler.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
    sampler.BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
    sampler.MinLOD = 0.0f;
    sampler.MaxLOD = D3D12_FLOAT32_MAX;
    sampler.ShaderRegister = 0;
    sampler.RegisterSpace = 0;
    sampler.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
    return sampler;
}

RootSignature::RootSignature(ID3D12Device* device, RootParameters& parameters, RootSignatureFlags flags)
{
    auto sampler = GetSampler();
    auto rootArguments = GetParameters(parameters);
    CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc(rootArguments.size(), rootArguments.data(), 1, &sampler,
        static_cast<D3D12_ROOT_SIGNATURE_FLAGS>(flags));

    ComPtr<ID3DBlob> serializedRootSig = nullptr;
    ComPtr<ID3DBlob> errorBlob = nullptr;
    HRESULT hr = D3D12SerializeRootSignature(&rootSigDesc, D3D_ROOT_SIGNATURE_VERSION_1,
        serializedRootSig.GetAddressOf(), errorBlob.GetAddressOf());

    if(errorBlob != nullptr)
    {
        ::OutputDebugStringA((char*)errorBlob->GetBufferPointer());
    }
    ThrowIfFailed(hr);

    ThrowIfFailed(device->CreateRootSignature(
        0,
        serializedRootSig->GetBufferPointer(),
        serializedRootSig->GetBufferSize(),
        IID_PPV_ARGS(&m_rootSignature)));
}

RootSignature::RootSignature(ID3D12Device* device)
{
    CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc(0, nullptr, 0, nullptr,
        D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

    ComPtr<ID3DBlob> serializedRootSig = nullptr;
    ComPtr<ID3DBlob> errorBlob = nullptr;
    HRESULT hr = D3D12SerializeRootSignature(&rootSigDesc, D3D_ROOT_SIGNATURE_VERSION_1,
        serializedRootSig.GetAddressOf(), errorBlob.GetAddressOf());

    if(errorBlob != nullptr)
    {
        ::OutputDebugStringA((char*)errorBlob->GetBufferPointer());
    }
    ThrowIfFailed(hr);

    ThrowIfFailed(device->CreateRootSignature(
        0,
        serializedRootSig->GetBufferPointer(),
        serializedRootSig->GetBufferSize(),
        IID_PPV_ARGS(&m_rootSignature)));
}

};

