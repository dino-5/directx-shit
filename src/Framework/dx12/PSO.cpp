#include "PSO.h"
#include "PipelineStates.h"
#include "RootSignature.h"
#include "Device.h"

D3D12_SHADER_BYTECODE PSO::GetShader(ComPtr<ID3D10Blob> blob,std::wstring info, std::string name)
{
    blob= d3dUtil::CompileShader(info, nullptr, name, "vs_5_1");
    return 
    {
        reinterpret_cast<BYTE*>(blob->GetBufferPointer()),
        VS->GetBufferSize()
    };
}
D3D12_SHADER_BYTECODE PSO::GetPixelShader(ShaderInfo info)
{
    return GetShader(PS, info.pixelShader, "PS");
}

D3D12_SHADER_BYTECODE PSO::GetVertexShader(ShaderInfo info)
{
    return GetShader(VS, info.vertexShader, "VS");
}

PSO::PSO(ShaderInfo shaders, InputLayout layout, RootSignature rootSignature, BlendState blendState,
	DepthStencilState dsState)
{

    D3D12_GRAPHICS_PIPELINE_STATE_DESC opaquePsoDesc;

    ZeroMemory(&opaquePsoDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
    opaquePsoDesc.InputLayout = layout;
    opaquePsoDesc.pRootSignature = rootSignature;
    opaquePsoDesc.VS = GetVertexShader(shaders);
    opaquePsoDesc.PS = GetPixelShader(shaders);
    opaquePsoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
    opaquePsoDesc.BlendState = blendState;
    opaquePsoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
    opaquePsoDesc.SampleMask = UINT_MAX;
    opaquePsoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    opaquePsoDesc.NumRenderTargets = 1;
    opaquePsoDesc.RTVFormats[0] = backBufferFormat;
    opaquePsoDesc.SampleDesc.Count = msaaState ? 4 : 1;
    opaquePsoDesc.SampleDesc.Quality = msaaState ? (msaaQuality - 1) : 0;
    opaquePsoDesc.DSVFormat = dsvBufferFormat;
    ThrowIfFailed(Device::GetDevice()->CreateGraphicsPipelineState(&opaquePsoDesc, IID_PPV_ARGS(&m_pso)));

   }
