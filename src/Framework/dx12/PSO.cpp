#include "PSO.h"
#include "RootSignature.h"
#include "Device.h"
#include "Framework/include/d3dUtil.h"

D3D12_SHADER_BYTECODE PSO::GetShader(std::string name)
{
    auto shader = Shader::GetShader(name);
    return 
    {
        reinterpret_cast<BYTE*>(shader->GetBufferPointer()),
        shader->GetBufferSize()
    };
}

PSO::PSO(ShaderInputGroup shader, BlendState blendState,DepthStencilState dsState, RasterizerState rasterState)
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
    m_psoDesc.SampleDesc.Count = msaaState ? 4 : 1;
    m_psoDesc.SampleDesc.Quality = msaaState ? (msaaQuality - 1) : 0;
    m_psoDesc.DSVFormat = dsvBufferFormat;
    ThrowIfFailed(Device::GetDevice()->CreateGraphicsPipelineState(&m_psoDesc, IID_PPV_ARGS(&m_pso)));
}

void PSO::SetBlendState(BlendState blendState)
{
    m_blend = blendState;
}

void PSO::SetDepthStencilState(DepthStencilState ds)
{
    m_ds = ds;
}

void PSO::SetRasterizerState(RasterizerState rast)
{
    m_rast = rast;
}

void PSO::SetShaderInputGroup(ShaderInputGroup& shader)
{
    m_shader = shader;
}

PSO PSO::Compile()
{
    return PSO(m_shader, m_blend, m_ds, m_rast);
}

void Shader::CreateShader(ShaderInfo info)
{
    if (FindShader(info.shaderName)) {}
    else
    {
        m_shaders[info.shaderName] = d3dUtil::CompileShader(std::wstring(info.path.begin(), info.path.end()),
            nullptr, info.entryPoint, info.type == ShaderType::VERTEX ? "vs_5_1" : "ps_5_1");
    }
}

Shader::Shader(ShaderInfo info) :m_name(info.shaderName)
{
	CreateShader(info);
	m_shader = m_shaders[info.shaderName];
}
