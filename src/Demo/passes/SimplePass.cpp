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
    Pass::SetRootSignature("empty");
    
    Vertex triangleVertices[] =
    {
        //{ { 0.0f, 0.25f * renderContext.GetAspectRatio(), 0.0f}, {1.0f, 0.0f, 0.0f, 1.0f}},
        //{ { 0.25f, -0.25f * renderContext.GetAspectRatio(), 0.0f }, { 0.0f, 1.0f, 0.0f, 1.0f } },
        //{ { -0.25f, -0.25f * renderContext.GetAspectRatio(), 0.0f }, { 0.0f, 0.0f, 1.0f, 1.0f } }
         math::Vector3({0.0f, 0.25f * aspectRatio, 0.0f}),
         math::Vector3({0.25f, -0.25f * aspectRatio, 0.0f }),
         math::Vector3({-0.25f, -0.25f * aspectRatio, 0.0f }),
    };

    const UINT vertexBufferSize = sizeof(triangleVertices);

    auto heapType = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
    auto bufferDesc =CD3DX12_RESOURCE_DESC::Buffer(vertexBufferSize); 
    ThrowIfFailed(device->CreateCommittedResource(
        &heapType,
        D3D12_HEAP_FLAG_NONE,
        &bufferDesc,
        D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr,
        IID_PPV_ARGS(&m_vertexBuffer)));

    UINT8* pVertexDataBegin;
    CD3DX12_RANGE readRange(0, 0);        // We do not intend to read from this resource on the CPU.
    ThrowIfFailed(m_vertexBuffer->Map(0, &readRange, reinterpret_cast<void**>(&pVertexDataBegin)));
    memcpy(pVertexDataBegin, triangleVertices, sizeof(triangleVertices));
    m_vertexBuffer->Unmap(0, nullptr);

    m_vertexBufferView.BufferLocation = m_vertexBuffer->GetGPUVirtualAddress();
    m_vertexBufferView.StrideInBytes = sizeof(Vertex);
    m_vertexBufferView.SizeInBytes = vertexBufferSize;
}

void SimplePass::Draw(ID3D12GraphicsCommandList* commandList)
{
    Pass::SetPipeline(commandList);
    commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    commandList->IASetVertexBuffers(0, 1, &m_vertexBufferView);
    commandList->DrawInstanced(3, 1, 0, 0);
}


