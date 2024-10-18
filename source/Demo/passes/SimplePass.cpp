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
    Pass::SetRootSignature(graphics::ROOT_SIG_VERTEX);

    ID3D12Device* device = context.GetDevice().native();
    graphics::CommandList& commandList = context.GetList();
    graphics::Camera& camera = context.GetCamera();
    
    float aspectRatio = context.GetAspectRatio();
    m_data.perspective = math::PerspectiveProjection(90, aspectRatio, .1f, 10000.f);
    m_data.view = camera.GetViewMatrix();
    m_constantBuffer.Init(context, &m_data, 1);

    m_rootIndexData.constantBufferIndex = m_constantBuffer.GetDescriptorHeapIndex();
    m_rootIndexData.textureIndex = m_texture.GetDescriptorHeapIndex();
    m_rootStructure.Init(context, &m_rootIndexData, 1);

    context.GetCamera().AddChangeCallback([this, &camera]()
        {
            this->m_data.view = camera.GetViewMatrix();
            m_constantBuffer.Update(&this->m_data);
        });

    m_model.Init(g_homeDir / "textures/models/Sponza/gltf/Sponza.gltf", context);
}

void SimplePass::Draw(ID3D12GraphicsCommandList* commandList, u32 frameNumber)
{
    commandList->SetDescriptorHeaps(1, engine::graphics::DescriptorHeapManager::CurrentSRVHeap.getHeapAddress());
	commandList->SetGraphicsRootSignature( *m_rootSignature );
	commandList->SetPipelineState( *m_pso );

    commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    commandList->SetGraphicsRootConstantBufferView(0, m_rootStructure->GetGPUVirtualAddress());
    m_model.DrawModel(commandList);
}


