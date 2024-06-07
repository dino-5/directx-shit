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
    Pass::SetRootSignature(graphics::ROOT_SIG_TWO_CONSTANTS);
    ID3D12Device* device = context.GetDevice().native();
    graphics::CommandList& commandList = context.GetList();
    float aspectRatio = context.GetAspectRatio();
    
    Vertex triangleVertices[] =
    {
         math::Vector3({-0.3f, -0.2f * aspectRatio,    .5f}),
         math::Vector3({0.2f, 0.8f * aspectRatio,  .5f }),
         math::Vector3({0.75f, -0.20f * aspectRatio, .9f }),
    };
    u16 indexData[] = { 0, 1, 2  };

    m_vertexBuffer.Init(context, triangleVertices, 3);
    m_indexBuffer.Init(context, indexData, 3);
    m_data.perspective = math::PerspectiveProjection(90, aspectRatio, 0.f, 100.f);
    m_constantBuffer.Init(context, &m_data, 1);

    m_texture.Init(graphics::ImageData(g_homeDir / "textures" / "wall.jpg"), device, commandList.GetList());

    m_rootIndexData.vertexBufferIndex = m_vertexBuffer.GetDescriptorHeapIndex();
    m_rootIndexData.constantBufferIndex = m_constantBuffer.GetDescriptorHeapIndex();
    m_rootIndexData.textureIndex = m_texture.GetDescriptorHeapIndex();
    m_rootStructure.Init(context, &m_rootIndexData, 1);
    //m_model.Init(g_homeDir / "textures/models/Sponza/gltf/Sponza.gltf", context);
}

void SimplePass::Draw(ID3D12GraphicsCommandList* commandList, u32 frameNumber)
{
    commandList->SetDescriptorHeaps(1, engine::graphics::DescriptorHeapManager::CurrentSRVHeap.getHeapAddress());
	commandList->SetGraphicsRootSignature( *m_rootSignature );
	commandList->SetPipelineState( *m_pso );

    commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    commandList->DrawInstanced(3, 1, 0, 0);
}


