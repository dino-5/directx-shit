#include "PSO.h"
#include "RootSignature.h"
#include "Device.h"
#include "Framework/util/Util.h"

namespace engine::graphics
{

    D3D12_SHADER_BYTECODE PSO::GetShader(std::string name)
    {
        auto shader = Shader::GetShader(name);
        return
        {
            reinterpret_cast<BYTE*>(shader->GetBufferPointer()),
            shader->GetBufferSize()
        };
    }

    PSO::PSO(ID3D12Device* device, ShaderInputGroup shader, BlendState blendState, DepthStencilState dsState, RasterizerState rasterState)
    {
        m_psoDesc.InputLayout = *shader.layout;
        m_psoDesc.pRootSignature = *shader.rootSignature;
        m_psoDesc.VS = GetShader(shader.vertexShader);
        m_psoDesc.PS = GetShader(shader.pixelShader);
        m_psoDesc.RasterizerState = rasterState;
        m_psoDesc.BlendState = blendState;
        m_psoDesc.DepthStencilState = dsState;
        m_psoDesc.SampleMask = UINT_MAX;
        m_psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
        m_psoDesc.NumRenderTargets = 1;
        m_psoDesc.RTVFormats[0] = backBufferFormat;
        m_psoDesc.SampleDesc.Count =  1;
        m_psoDesc.SampleDesc.Quality = 0;
        m_psoDesc.DSVFormat = dsvBufferFormat;
        ThrowIfFailed(device->CreateGraphicsPipelineState(&m_psoDesc, IID_PPV_ARGS(&m_pso)));
    }

    PSO* PSO::CreatePSO(std::string name, ID3D12Device* device, ShaderInputGroup shader, BlendState blendState, DepthStencilState dsState, RasterizerState rasterState)
    {
        if (GetPSO(name) == nullptr)
            allPSO.push_back({ name, PSO(device, shader, blendState, dsState, rasterState) });
        return &(allPSO.back().second);
    }
    void RenderState::SetBlendState(BlendState blendState)
    {
        m_blend = blendState;
    }

    void RenderState::SetDepthStencilState(DepthStencilState ds)
    {
        m_ds = ds;
    }

    void RenderState::SetRasterizerState(RasterizerState rast)
    {
        m_rast = rast;
    }

    void RenderState::SetShaderInputGroup(ShaderInputGroup& shader)
    {
        m_shader = shader;
    }

    PSO* RenderState::Compile(std::string name)
    {
        return PSO::CreatePSO(name, Device::device->GetDevice(), m_shader, m_blend, m_ds, m_rast);
    }

    std::string GetShaderTypeString(ShaderType type)
    {
        switch (type)
        {
        case ShaderType::VERTEX:
            return "vs_5_1";
        case ShaderType::PIXEL:
            return "ps_5_1";
        case ShaderType::COMPUTE:
            return "cs_5_1";
        default:
            return "wrong shit";
        }
    }

    void Shader::CreateShader(ShaderInfo info)
    {
        if (GetShader(info.shaderName)==nullptr) 
        {
            auto shaderByteCode = engine::util::d3dUtil::CompileShader(
                util::to_wstring(info.shaderName), nullptr, info.entryPoint, GetShaderTypeString(info.type));
            allShaders.push_back({ info.shaderName, shaderByteCode});
        }
    }

    Shader::Shader(ShaderInfo info) :m_name(info.shaderName)
    {
        CreateShader(info);
        m_shader = GetShader(info.shaderName);
    }

    ComputePSO::ComputePSO(ComputeShaderInputGroup shaderGroup)
    {
        D3D12_COMPUTE_PIPELINE_STATE_DESC desc{};
        desc.CS = PSO::GetShader(shaderGroup.computeShader);
        desc.pRootSignature = *shaderGroup.rootSignature;

        ThrowIfFailed(Device::device->GetDevice()->CreateComputePipelineState(&desc, IID_PPV_ARGS(&m_pso)));
    }
};
