#include "SimplePass.h"
#include "EngineGfx/dx12/PSO.h"
#include "EngineGfx/dx12/RootSignature.h"
#include "EngineGfx/RenderContext.h"

SimplePass::SimplePass(ID3D12Device* device, i32 aspectRatio):m_constantBuffer(sizeof(ConstandBufferData))
{
    Initialize(device, aspectRatio);
}

void SimplePass::Initialize(ID3D12Device* device, i32 aspectRatio)
{
    Pass::SetPSO("default");
    Pass::SetRootSignature(graphics::ROOT_SIG_ONE_CONST);
    
    Vertex triangleVertices[] =
    {
         math::Vector3({0.0f, 0.25f * aspectRatio,    10.f}),
         math::Vector3({0.25f, -0.25f * aspectRatio,  10.f }),
         math::Vector3({-0.25f, -0.25f * aspectRatio, 10.f }),
    };
    m_data.color = 0.8;
    m_data.perspective = math::PerspectiveProjection(90, 1, 0, 0);
    m_vertexBuffer.Init(device, triangleVertices, sizeof(triangleVertices));
    m_constantBuffer.Init(device, 1, &m_data);
}

void SimplePass::Draw(ID3D12GraphicsCommandList* commandList, u32 frameNumber)
{
    Pass::SetPipeline(commandList);

    commandList->SetGraphicsRootConstantBufferView(0, m_constantBuffer.GetAddress(frameNumber));
    commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    commandList->IASetVertexBuffers(0, 1, &m_vertexBuffer.GetView());
    commandList->DrawInstanced(3, 1, 0, 0);
}


