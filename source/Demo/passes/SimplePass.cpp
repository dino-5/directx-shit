#include "SimplePass.h"
#include "EngineGfx/dx12/PSO.h"
#include "EngineGfx/dx12/RootSignature.h"
#include "EngineGfx/RenderContext.h"

SimplePass::SimplePass(ID3D12Device* device, int aspectRatio):m_constantBuffer(sizeof(float))
{
    Initialize(device, aspectRatio);
}

void SimplePass::Initialize(ID3D12Device* device, int aspectRatio)
{
    Pass::SetPSO("default");
    Pass::SetRootSignature("oneConst");
    
    Vertex triangleVertices[] =
    {
         math::Vector3({0.0f, 0.25f * aspectRatio, 0.0f}),
         math::Vector3({0.25f, -0.25f * aspectRatio, 0.0f }),
         math::Vector3({-0.25f, -0.25f * aspectRatio, 0.0f }),
    };
    m_vertexBuffer.Init(device, triangleVertices, sizeof(triangleVertices));
    m_constantBuffer.Init(device, 1, &m_data);
}

void SimplePass::Draw(ID3D12GraphicsCommandList* commandList, uint frameNumber)
{
    Pass::SetPipeline(commandList);

    commandList->SetGraphicsRootConstantBufferView(0, m_constantBuffer.GetAddress(frameNumber));
    commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    commandList->IASetVertexBuffers(0, 1, &m_vertexBuffer.GetView());
    commandList->DrawInstanced(3, 1, 0, 0);
}


