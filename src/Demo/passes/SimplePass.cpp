#include "SimplePass.h"
#include "Framework/graphics/dx12/PSO.h"
#include "Framework/graphics/dx12/RootSignature.h"
#include "Framework/graphics/RenderContext.h"

SimplePass::SimplePass(ID3D12Device* device, int aspectRatio)
{
    Initialize(device, aspectRatio);
}

void SimplePass::Initialize(ID3D12Device* device, int aspectRatio)
{

    Pass::SetPSO("default");
    Pass::SetRootSignature("twoConst");
    
    Vertex triangleVertices[] =
    {
         math::Vector3({0.0f, 0.25f * aspectRatio, 0.0f}),
         math::Vector3({0.25f, -0.25f * aspectRatio, 0.0f }),
         math::Vector3({-0.25f, -0.25f * aspectRatio, 0.0f }),
    };
    m_vertexBuffer.Init(device, triangleVertices, sizeof(triangleVertices));
}

void SimplePass::Draw(ID3D12GraphicsCommandList* commandList)
{
    Pass::SetPipeline(commandList);
    commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    commandList->IASetVertexBuffers(0, 1, &m_vertexBuffer.GetView());
    commandList->DrawInstanced(3, 1, 0, 0);
}


