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
    
    Vertex triangleVertices[] =
    {
         math::Vector3({-0.5f,  -0.5f,  1.5f}),
         math::Vector3({-0.5f,   0.5f,  1.5f }),
         math::Vector3({ 0.5f,   0.5f,  1.5f }),
         math::Vector3({ 0.5f,  -0.5f,  1.5f }),

         math::Vector3({ 0.5f,  -0.5f,  1.5f}),
         math::Vector3({ 0.5f,   0.5f,  1.5f }),
         math::Vector3({ 0.5f,   0.5f,  2.5f }),
         math::Vector3({ 0.5f,  -0.5f,  2.5f }),

         math::Vector3({-0.5f,  -0.5f,  2.5f}),
         math::Vector3({-0.5f,   0.5f,  2.5f }),
         math::Vector3({-0.5f,   0.5f,  1.5f }),
         math::Vector3({-0.5f,  -0.5f,  1.5f }),

         math::Vector3({-0.5f,  -0.5f,  1.5f}),
         math::Vector3({-0.5f,   0.5f,  1.5f }),
         math::Vector3({ 0.5f,   0.5f,  1.5f }),
         math::Vector3({ 0.5f,   0.5f,  1.5f }),

         math::Vector3({-0.5f,  -0.5f,  1.5f}),
         math::Vector3({-0.5f,   0.5f,  1.5f }),
         math::Vector3({ 0.5f,   0.5f,  1.5f }),
         math::Vector3({ 0.5f,   0.5f,  1.5f }),

         math::Vector3({-0.5f,  -0.5f,  1.5f}),
         math::Vector3({-0.5f,   0.5f,  1.5f }),
         math::Vector3({ 0.5f,   0.5f,  1.5f }),
         math::Vector3({ 0.5f,   0.5f,  1.5f }),

    };
    u32 indexData[36]; 
    for (uint i = 0; i < 6; i++)
    {
        indexData[i*6 + 0] = i * 4 + 0;
        indexData[i*6 + 1] = i * 4 + 1;
        indexData[i*6 + 2] = i * 4 + 2;
        indexData[i*6 + 3] = i * 4 + 2;
        indexData[i*6 + 4] = i * 4 + 3;
        indexData[i*6 + 5] = i * 4 + 0;
    }
                        

    m_vertexBuffer.InitAsVertexBuffer(context, triangleVertices, sizeof(triangleVertices) / sizeof(Vertex));
    m_indexBuffer.InitAsIndexBuffer(context, indexData, sizeof(indexData) / sizeof(u32));
    numberOfIndices = sizeof(indexData) / sizeof(u32);
    m_data.perspective = math::PerspectiveProjection(90, aspectRatio, 1.f, 100.f);
    m_data.view = camera.GetViewMatrix();
    m_constantBuffer.Init(context, &m_data, 1);

    m_texture.Init(graphics::ImageData(g_homeDir / "textures" / "wall.jpg"), device, commandList.GetList());

    m_rootIndexData.constantBufferIndex = m_constantBuffer.GetDescriptorHeapIndex();
    m_rootIndexData.textureIndex = m_texture.GetDescriptorHeapIndex();
    m_rootStructure.Init(context, &m_rootIndexData, 1);

    context.GetCamera().AddChangeCallback([this, &camera]()
        {
            this->m_data.view = camera.GetViewMatrix();
            m_constantBuffer.Update(&this->m_data);
        });
}

void SimplePass::Draw(ID3D12GraphicsCommandList* commandList, u32 frameNumber)
{
    commandList->SetDescriptorHeaps(1, engine::graphics::DescriptorHeapManager::CurrentSRVHeap.getHeapAddress());
	commandList->SetGraphicsRootSignature( *m_rootSignature );
	commandList->SetPipelineState( *m_pso );

    auto vertexView = GetVertexBufferView(m_vertexBuffer);
    auto indexBuffer = GetIndexBufferView(m_indexBuffer);

    commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    commandList->IASetVertexBuffers(0, 1, &vertexView);
    commandList->SetGraphicsRootConstantBufferView(0, m_rootStructure.getAddress());
    commandList->IASetIndexBuffer(&indexBuffer);
    commandList->DrawIndexedInstanced(numberOfIndices, 1, 0, 0, 0);
}


