#include "SimplePass.h"
#include "EngineGfx/dx12/PSO.h"
#include "EngineGfx/dx12/RootSignature.h"
#include "EngineGfx/dx12/DescriptorHeap.h"
#include "EngineGfx/RenderContext.h"
#include "EngineCommon/System/Filesystem.h"

using namespace engine;

SimplePass::SimplePass(graphics::RenderContext& context)
{
    Initialize(context);
}

void SimplePass::Initialize(graphics::RenderContext& context)
{
    Pass::SetPSO(L"default");
    Pass::SetRootSignature(graphics::ROOT_SIG_BINDLESS);
    ID3D12Device* device = context.GetDevice().native();
    graphics::CommandList& commandList = context.GetList();
    graphics::CommandQueue& queue = context.GetQueue();
    float aspectRatio = context.GetAspectRatio();
    
    Vertex triangleVertices[] =
    {
         math::Vector3({0.0f, 0.25f * aspectRatio,    .9f}),
         math::Vector3({0.25f, 0.95f * aspectRatio,  .9f }),
         math::Vector3({0.95f, 0.05f * aspectRatio, .9f }),
    };
    m_data.color = 0.8;
    m_data.perspective = math::PerspectiveProjection(90, 1, 0, 0);

    m_vertexBuffer.Init(context, triangleVertices, sizeof(triangleVertices), sizeof(math::Vector3), 3);
    m_constantBuffer.Init(device, 1, &m_data, sizeof(m_data));

    m_texture.Init(g_homeDir / "textures" / "wall.jpg", device, commandList.GetList());

    m_rootIndexData.vertexBufferIndex = m_vertexBuffer.GetDescriptorHeapIndex();
    m_rootIndexData.constantBufferIndex = m_constantBuffer.getDescriptorHeapIndex();
    m_rootIndexData.textureIndex = m_texture.getDescriptorHeapIndex();
    m_rootStructure.Init(device, 1, &m_rootIndexData, sizeof(m_rootIndexData));
    m_model.Init(g_homeDir/"textures/models/Sponza/gltf/Sponza.gltf", commandList.GetList());
}

void SimplePass::Draw(ID3D12GraphicsCommandList* commandList, u32 frameNumber)
{
    commandList->SetDescriptorHeaps(1, engine::graphics::DescriptorHeapManager::CurrentSRVHeap.getHeapAddress());
	commandList->SetGraphicsRootSignature( *m_rootSignature );
	commandList->SetPipelineState( *m_pso );

    commandList->SetGraphicsRootConstantBufferView(0, m_rootStructure.getAddress());
    commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    commandList->DrawInstanced(3, 1, 0, 0);
}


