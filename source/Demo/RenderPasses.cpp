#include "RenderPasses.h"
#include "EngineGfx/Model.h"
#include "EngineGfx/dx12/Buffers.h"


RootSignatureFlags getDefaultRSFlags()
{
    return RootSignatureFlags::ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
        RootSignatureFlags::SBV_SRV_HEAP_DIRECT_INDEX;
}

void forwardPassInit(GfxContext& context,
                     RenderPass& pass)
{

    ShaderInputGroup sig;
    pass.addShader("forwardVS",
                   "VertexMain",
                   "Shaders/forward_render.hlsl",
                    ShaderType::VERTEX, &sig);

    pass.addShader("forwardPS",
                   "PixelMain",
                   "Shaders/forward_render.hlsl",
                    ShaderType::PIXEL, &sig);

    RootParameters parameters = {
        RootParameter::CreateDescriptor(0),
        RootParameter::CreateConstants(1, 0, 10) };

    pass.rs = RootSignature(globalContext.device(), parameters,
                            getDefaultRSFlags());
    u32 offset = 0;
    D3D12_INPUT_ELEMENT_DESC inputElements[] ={
        getInputElement("POSITION", offset, 3),
        getInputElement("NORMAL", offset, 3),
        getInputElement("TANGENT", offset, 4),
        getInputElement("UV", offset, 2),
    };
    D3D12_INPUT_LAYOUT_DESC inputLayout {inputElements, 4};

    sig.desc = inputLayout;
    sig.rootSignature = &pass.rs;

    RenderState renderState;
    renderState.setShaderInputGroup(sig);
    pass.pso = PSO(renderState);
}

void forwardPassExecute(GfxContext& context,
                        Model* model,
                        RenderPass& pass,
                        void* aData)
{
    auto cmdList = context.currentCmdList;
    auto renderTarget = context.currentRenderTarget->rtv;
    auto depthStencil = context.currentDepthStencil->dsv;

    cmdList->OMSetRenderTargets(1, &renderTarget.HandleCPU,
                                true, &depthStencil.HandleCPU);

    cmdList->SetDescriptorHeaps(1, 
            DescriptorHeapManager::CurrentSRVHeap.getHeapAddress());
    cmdList->SetGraphicsRootSignature(pass.rs);
    cmdList->SetPipelineState(pass.pso);

    cmdList->IASetPrimitiveTopology(
        D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    u32 passIndices =
        model->m_materialBuffer.getDescriptorHeapIndex();
    cmdList->SetGraphicsRootConstantBufferView(
        0,
        context.view.buffer->GetGPUVirtualAddress());
    cmdList->SetGraphicsRoot32BitConstant(1, passIndices, 0);

    ForwardPassData data = *(reinterpret_cast<ForwardPassData*>(
        aData
    ));

    if(model->isInitialized())
    {
        auto vertexBuffer = GetVertexBufferView(
            model->m_mesh.m_vertexBuffer);
        auto indexBuffer = GetIndexBufferView(
            model->m_mesh.m_indexBuffer);
        cmdList->IASetVertexBuffers(0, 1, &vertexBuffer);
        cmdList->IASetIndexBuffer(&indexBuffer);

        if(data.drawModel)
        for (auto& submesh : model->m_submeshes)
        {
            cmdList->SetGraphicsRoot32BitConstant(1,
                                          submesh.materialIndex, 0);
            submesh.draw(cmdList);
        }
    }
}

void debugDrawBVHPassInit(GfxContext& context,
                     RenderPass& pass)
{
    ShaderInputGroup sig;
    pass.addShader("debugBVH_VS",
                   "VertexMain",
                   "Shaders/debug_BVHdraw.hlsl",
                   ShaderType::VERTEX, &sig);

    pass.addShader("debugBVH_PS",
                   "PixelMain",
                   "Shaders/debug_BVHdraw.hlsl",
                    ShaderType::PIXEL, &sig);
    
    RootParameters parameters = { RootParameter::CreateConstants(1, 0, 10) };

    pass.rs = RootSignature(globalContext.device(), parameters, getDefaultRSFlags());
    u32 offset = 0;
    D3D12_INPUT_ELEMENT_DESC inputElements[] ={
        getInputElement("POSITION", offset, 3)
    };
    D3D12_INPUT_LAYOUT_DESC inputLayout {inputElements, 1};

    sig.desc = inputLayout;
    sig.rootSignature = &pass.rs;

    RenderState renderState;
    renderState.setShaderInputGroup(sig);
    renderState.topology = D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE;
    pass.pso = PSO(renderState);
}

void debugDrawBVHPassExecute(GfxContext& context,
                        Model* model,
                        RenderPass& pass,
                        void* data)
{
    auto cmdList = context.currentCmdList;

    auto renderTarget = context.currentRenderTarget->rtv;
    auto depthStencil = context.currentDepthStencil->dsv;

    cmdList->OMSetRenderTargets(1, &renderTarget.HandleCPU,
                                true, &depthStencil.HandleCPU);
    cmdList->SetDescriptorHeaps(1, 
            DescriptorHeapManager::CurrentSRVHeap.getHeapAddress());
    cmdList->SetGraphicsRootSignature(pass.rs);
    cmdList->SetPipelineState(pass.pso);
    cmdList->SetGraphicsRoot32BitConstant(0,
              context.view.buffer.getDescriptorHeapIndex(), 0);
    cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_LINELIST);

    auto mesh = model->m_mesh;
    auto submesh = model->m_submeshes[0];
    auto vertexBuffer = GetVertexBufferView(mesh.m_vertexBuffer);
    auto indexBuffer = GetIndexBufferView(mesh.m_indexBuffer);
    cmdList->IASetVertexBuffers(0, 1, &vertexBuffer);
    cmdList->IASetIndexBuffer(&indexBuffer);
    submesh.draw(cmdList);
}


void computeRTXPassInit(GfxContext& context,
                     RenderPass& pass)
{
    ShaderInputGroup sig;
    pass.addShader("rtxCompute",
                   "CSMain",
                   "Shaders/rtxCompute.hlsl",
                   ShaderType::COMPUTE, &sig);
    
    pass.rs = RootSignature(context.device());
    sig.rootSignature = &pass.rs;
    pass.pso = PSO(sig);

}

void computeRTXPassExecute(GfxContext& context,
                        Model* model,
                        RenderPass& pass,
                        void* data)
{

}
