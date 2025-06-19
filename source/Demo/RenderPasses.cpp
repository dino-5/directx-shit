#include "RenderPasses.h"
#include "EngineGfx/Model.h"
#include "EngineGfx/dx12/Buffers.h"
#include <cstring>


RootSignatureFlags getDefaultRSFlags()
{
    return RootSignatureFlags::ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
        RootSignatureFlags::SBV_SRV_HEAP_DIRECT_INDEX;
}

struct PassResource
{
    u8 resourceCount;
    u8 offset;
};


std::vector<PassResource> PassResourcesDesc(PassResourcesCount);

void initRenderPassResources(GfxContext& context)
{
    PassResourcesDesc[RTXPass_ConstantBufferData] = {1, 0};
    PassResourcesDesc[RTXPass_OutputTexture] = {config::NumFrames, 0};
    PassResourcesDesc[RTXPass_SphereBuffer] = {1, 0};

    u32 totalResourceCount = 0;
    for(auto& resDesc : PassResourcesDesc)
    {
        resDesc.offset = totalResourceCount;
        totalResourceCount += resDesc.resourceCount;
    }

    context.resources.resize(totalResourceCount);
}

Resource*& getResource(GfxContext& context, u8 index)
{
    PassResource desc = PassResourcesDesc[index];
    u8 offset = 0;
    if(desc.resourceCount > 1)
        offset = globalContext.currentFrameIndex;

    return context.resources[desc.offset + offset];
}

std::span<Resource*> getPassResources(GfxContext& context, u8 index)
{
    return {&context.resources[PassResourcesDesc[index].offset], 
            PassResourcesDesc[index].resourceCount};
}

void forwardPassInit(GfxContext& context,
                     RenderPass& pass)
{

    ShaderInputGroup sig;
    pass.addShader("forwardVS",
                   "VertexMain",
                   "Shaders/forward_render.hlsl",
                    ShaderType::VERTEX);

    pass.addShader("forwardPS",
                   "PixelMain",
                   "Shaders/forward_render.hlsl",
                    ShaderType::PIXEL);

    RootParameters parameters = {
        CreateDescriptor(0),
        CreateConstants(1, 0, 10) };

    pass.rs = RootSignature(globalContext.device(), parameters,
                            getDefaultRSFlags());
    u32 offset = 0;
    std::vector<D3D12_INPUT_ELEMENT_DESC> inputElements = {
        getInputElement("POSITION", offset, 3),
        getInputElement("NORMAL", offset, 3),
        getInputElement("TANGENT", offset, 4),
        getInputElement("UV", offset, 2),
    };

    sig.desc = inputElements;
    pass.renderState.sig = sig;

    pass.compilePSO();
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

    ForwardPassData data = *(ForwardPassData*)aData;

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
                   ShaderType::VERTEX);

    pass.addShader("debugBVH_PS",
                   "PixelMain",
                   "Shaders/debug_BVHdraw.hlsl",
                    ShaderType::PIXEL);
    
    RootParameters parameters = {
        CreateConstants(1, 0, 10)
    };

    pass.rs = RootSignature(globalContext.device(),
                            parameters, getDefaultRSFlags());
    u32 offset = 0;
    std::vector<D3D12_INPUT_ELEMENT_DESC> inputElements ={
        getInputElement("POSITION", offset, 3)
    };

    sig.desc = inputElements;
    pass.renderState.sig = sig;

    pass.compilePSO();
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


struct BindlessRTXTable
{
    u32 outputTextureIndex;
    u32 sphereArrayIndex;
};

void computeRTXPassResize(GfxContext& context,
                     RenderPass& pass)
{
    std::span<Resource*> outputTexture = getPassResources(context,
                                                          RTXPass_OutputTexture);
    for(auto& texture : outputTexture)
    {
        if(texture)
        {
            Texture* tex = (Texture*)texture;
            if(tex->width != context.viewPort.Width 
                || tex->height != context.viewPort.Height)
            {
                delete tex;
            }
            else
                continue;
        }

        texture = new Texture(
            {(int)context.viewPort.Width, (int)context.viewPort.Height, 4 },
            context,
            DescriptorFlags::ShaderResource | DescriptorFlags::UnorderedAccess,
            ResourceState::UNORDERED_ACCESS
        );
    }
}

void computeRTXPassInit(GfxContext& context,
                     RenderPass& pass)
{
    pass.addShader("rtxCompute",
                   "CSMain",
                   "Shaders/rtxCompute.hlsl",
                   ShaderType::COMPUTE);

    RootParameters rsParams = {
        CreateDescriptor(0, 0, RootParameterType::CBV),
        CreateDescriptor(1, 0, RootParameterType::CBV),
        CreateConstants(sizeof(BindlessRTXTable) / sizeof(u32), 2)
    };
    
    pass.rs = RootSignature(context.device(), rsParams, getDefaultRSFlags());
    pass.compilePSO();

    if(!pass.data)
        pass.data = new RTXPassData;

    Resource*& constantBuffer = getResource(context, RTXPass_ConstantBufferData);
    constantBuffer = new ConstantBuffer(globalContext.device,
                                        globalContext.cmdList,
                                        sizeof(RTXDescription));

    Resource*& sphereBuffer = getResource(context, RTXPass_SphereBuffer);
    sphereBuffer = new ConstantBuffer(globalContext.device,
                                     globalContext.cmdList,
                                     sizeof(Sphere) * MAX_NUMBER_OF_SPHERES);

}

void computeRTXPassExecute(GfxContext& context,
                        Model* model,
                        RenderPass& pass,
                        void* data)
{
    auto cmdList = context.currentCmdList;

    auto renderTarget = context.currentRenderTarget->rtv;
    auto depthStencil = context.currentDepthStencil->dsv;

    Resource*& outputTexture = getResource(context, RTXPass_OutputTexture);

    ConstantBuffer* descriptionCB = (ConstantBuffer*) getResource(context,
                                                   RTXPass_ConstantBufferData);

    ConstantBuffer* sphereArray = (ConstantBuffer*) getResource(context,
                                                   RTXPass_SphereBuffer);

    if(!outputTexture || !sphereArray)
        return;

    BindlessRTXTable passResourcesIndices;
    passResourcesIndices.outputTextureIndex = outputTexture->uav.getDescriptorIndex();
    passResourcesIndices.sphereArrayIndex = sphereArray->cbv.getDescriptorIndex();

    cmdList->OMSetRenderTargets(1, &renderTarget.HandleCPU,
                                true, &depthStencil.HandleCPU);
    cmdList->SetDescriptorHeaps(1, 
            DescriptorHeapManager::CurrentSRVHeap.getHeapAddress());
    cmdList->SetComputeRootSignature(pass.rs);
    cmdList->SetPipelineState(pass.pso);

    RTXPassData* passData = (RTXPassData*)pass.data;
    RTXPassData* newRtxData = (RTXPassData*)data;

    if(!data)
        return;

    if (memcmp(&newRtxData->description, &passData->description,
               sizeof(RTXDescription)))
    {
        memcpy(pass.data, data, sizeof(RTXDescription));
        descriptionCB->update(&passData->description);
    }

    if (memcmp(&newRtxData->sphereArray, &passData->sphereArray,
               sizeof(Sphere) * MAX_NUMBER_OF_SPHERES))
    {
        memcpy(&passData->sphereArray, &newRtxData->sphereArray,
               sizeof(Sphere) * MAX_NUMBER_OF_SPHERES);
        sphereArray->update(&passData->sphereArray);
    }

    u32 bindlessTableSize = sizeof(BindlessRTXTable) / sizeof(u32);

    cmdList->SetComputeRootConstantBufferView(0,
              context.view.buffer->GetGPUVirtualAddress());
    cmdList->SetComputeRootConstantBufferView(1,
              (*descriptionCB)->GetGPUVirtualAddress());
    cmdList->SetComputeRoot32BitConstants(2, bindlessTableSize,
                                          &passResourcesIndices,
                                          0);

    cmdList->Dispatch(passData->description.imWidth / 8,
                      passData->description.imHeight / 8, 1);

    auto rt = context.currentRenderTarget;
    rt->transition(cmdList, ResourceState::COPY_DEST);
    outputTexture->transition(cmdList, ResourceState::COPY_SOURCE);
    cmdList->CopyResource(*rt, *outputTexture);
    rt->transition(cmdList, ResourceState::RENDER_TARGET);
}
