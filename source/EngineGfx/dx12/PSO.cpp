#include "PSO.h"
#include "RootSignature.h"
#include "Device.h"
#include "EngineCommon/util/Util.h"
#include "EngineCommon/util/Logger.h"
#include "EngineCommon/System/Filesystem.h"
#include "EngineCommon/System/config.h"
#include <dxcapi.h>
#include "d3d12shader.h"
#include <format>
#include <cstring>
#include <string>
#include <locale>
#include <codecvt>
#undef memcpy

namespace engine::graphics
{

D3D12_SHADER_BYTECODE getShader(DxBlob* blob)
{
    D3D12_SHADER_BYTECODE ret
    {
        reinterpret_cast<BYTE*>(blob->GetBufferPointer()),
        blob->GetBufferSize()
    };
    return ret;
}

PSO::PSO(const RenderState& state)
{
    ID3D12Device* device = Device::s_device->device;
    
    m_psoDesc.InputLayout = state.shader.desc;
    m_psoDesc.pRootSignature = *state.shader.rootSignature;
    m_psoDesc.VS = state.shader.vertexShader;
    m_psoDesc.PS = state.shader.pixelShader;
    m_psoDesc.RasterizerState = +state.rast;
    m_psoDesc.BlendState = state.blend;
    m_psoDesc.DepthStencilState = +state.ds;
    m_psoDesc.SampleMask = UINT_MAX;
    m_psoDesc.PrimitiveTopologyType = state.topology;
    if (state.renderTargets.size() >= 1)
    {
        m_psoDesc.NumRenderTargets = (u32) state.renderTargets.size();
        uint index = 0;
        for(auto& format : state.renderTargets)
            m_psoDesc.RTVFormats[index++] = format;
    }
    else
    {
        m_psoDesc.NumRenderTargets = 1;
        m_psoDesc.RTVFormats[0] = backBufferFormat;
    }
    m_psoDesc.SampleDesc.Count =  1;
    m_psoDesc.SampleDesc.Quality = 0;
    m_psoDesc.DSVFormat = dsvBufferFormat;
    ThrowIfFailed(device->CreateGraphicsPipelineState(
        &m_psoDesc, IID_PPV_ARGS(&m_pso)));
}

void RenderState::setBlendState(BlendState blendState)
{
    blend = blendState;
}

void RenderState::setDepthStencilState(DepthStencilState d)
{
    ds = d;
}

void RenderState::setRasterizerState(RasterizerState r)
{
    rast = r;
}

void RenderState::setShaderInputGroup(ShaderInputGroup& s)
{
    shader = s;
}


PSO RenderState::compile(std::wstring name)
{
    return PSO(*this);
}

std::wstring GetShaderTypeString(ShaderType type)
{
    switch (type)
    {
    case ShaderType::VERTEX:
        return L"vs_6_6";
    case ShaderType::PIXEL:
        return L"ps_6_6";
    case ShaderType::COMPUTE:
        return L"cs_6_6";
    default:
        return L"wrong shit";
    }
}

ShaderInfo::~ShaderInfo()
{
    auto obj = ShaderManager::CreateShader(*this);
    onDestoy(obj);
}

namespace ShaderManager
{
    DxCompiler* s_compiler = nullptr;
    DxUtils* s_utils = nullptr;
    DxIncludeHandler* s_includer = nullptr;
    void InitializeCompiler()
    {
        ThrowIfFailed(::DxcCreateInstance(CLSID_DxcCompiler,
                                          IID_PPV_ARGS(&s_compiler)));
        ThrowIfFailed(::DxcCreateInstance(CLSID_DxcUtils,
                                          IID_PPV_ARGS(&s_utils)));
        s_utils->CreateDefaultIncludeHandler(&s_includer);
    }


    TableEntry< DxBlob*> CreateShader(const ShaderInfo& info)
    {
        IDxcBlobEncoding* sourceBlob;
        s_utils->LoadFile(info.path.c_str(), nullptr, &sourceBlob);

        DxcBuffer sourceBuffer;
        sourceBuffer.Ptr = sourceBlob->GetBufferPointer();
        sourceBuffer.Size = sourceBlob->GetBufferSize();

        BOOL fl;
        sourceBlob->GetEncoding(&fl, &sourceBuffer.Encoding);
        std::wstring type = GetShaderTypeString(info.type);
        
        auto path = config::g_state.shaderDir / L"pdb" /
                    system::Filepath(info.path).filename();
        system::Filepath pdbPath(std::filesystem::absolute(path.getPath()));
        std::wstring pdbPathWstr = pdbPath.wstr() + info.entryPoint + L".pdb";
        std::vector<const wchar_t*> args= 
        {
            info.shaderName.c_str(),
            L"-E", info.entryPoint.c_str(),
            L"-T", type.c_str(),
            DXC_ARG_DEBUG
        };
        IDxcResult* result;
        HRESULT hr = s_compiler->Compile(
            &sourceBuffer,
            args.data(),
            static_cast<UINT32>(args.size()),
            s_includer,
            IID_PPV_ARGS(&result)
        );
        if(SUCCEEDED(hr) && result)
            result->GetStatus(&hr);

        if (FAILED(hr))
        {
            if (result)
            {
                IDxcBlobEncoding* errorBuffer;
                result->GetErrorBuffer(&errorBuffer);
                char* str = new char[errorBuffer->GetBufferSize()];

                std::memcpy(
                    str,
                    errorBuffer->GetBufferPointer(),
                    errorBuffer->GetBufferSize());

                engine::util::printError("{}",std::string(str));
                delete[] str;
            }
            return { L"", nullptr };
        }
        else
        {
            DxBlob* blob;
            result->GetOutput(DXC_OUT_OBJECT, IID_PPV_ARGS(&blob), nullptr);

            std::vector<std::byte> pdbData;
            DxBlob* pdbBlob;
            result->GetOutput(DXC_OUT_PDB, IID_PPV_ARGS(&pdbBlob), nullptr);
            auto* data = reinterpret_cast<std::byte*>(pdbBlob->GetBufferPointer());
            size_t size = pdbBlob->GetBufferSize();
            pdbData.assign(data, data + size);

            std::ofstream file(pdbPathWstr.data(), std::ios::binary | std::ios::trunc);
            file.write(reinterpret_cast<const char*>(pdbData.data()), pdbData.size());
            file.close();
            return { info.shaderName, blob};
        }

    }
}

ComputePSO::ComputePSO(ComputeShaderInputGroup shaderGroup)
{
    D3D12_COMPUTE_PIPELINE_STATE_DESC desc{};
    //desc.CS = GetShader(shaderGroup.computeShader);
    desc.pRootSignature = *shaderGroup.rootSignature;

    ID3D12Device* device = Device::s_device->device;
    ThrowIfFailed(device->CreateComputePipelineState(&desc,
                                                     IID_PPV_ARGS(&m_pso)));
}

};
