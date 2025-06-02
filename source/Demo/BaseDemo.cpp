#include <dxgiformat.h>
#include <format>
#include <functional>
#include <ostream>
#include <string_view>
#include "BaseDemo.h"
#include "EngineCommon/util/Logger.h"
#include "EngineCommon/util/CommandLine.h"
#include "EngineCommon/util/ImGuiSettings.h"
#include "EngineCommon/util/Timer.h"
#include "EngineGfx/tiny_bvh.h"

using namespace std;
using namespace util;
using namespace DirectX;

void locGenerateTinyBVHCompatibleGeometry(
    std::vector<tinybvh::bvhvec4>& outVertices,
    std::vector<u32>& outIndices,
    const std::vector<Vertex>& vertices,
    const std::vector<u32>& indices,
    const std::vector<Submesh>& submeshes)
{
    u32 currentIndex = 0;
    auto convertV3_to_V4 = [](const math::Vector3& vec) -> tinybvh::bvhvec4
    {
        tinybvh::bvhvec4 result;
        result.x = vec[0];
        result.y = vec[1];
        result.z = vec[2];
        result.w = 1;
        return result;
    };
    for(auto& vertex : vertices)
        outVertices[currentIndex++] = convertV3_to_V4(vertex.position);

    currentIndex = 0;
    {
        for(auto& submesh : submeshes)
        {
            u32 start = submesh.StartIndexLocation,
            end = start + submesh.IndexCount;
            for(u32 i = start; i < end; i+=3)
            {
                auto ind = &indices[i];
                outIndices[currentIndex++] = ind[0] + submesh.BaseVertexLocation;
                outIndices[currentIndex++] = ind[1] + submesh.BaseVertexLocation;
                outIndices[currentIndex++] = ind[2] + submesh.BaseVertexLocation;
            }
        }
    }
}


Light::Light(
    math::Vector3 vec)
    : m_position(vec)
{
}

BaseDemo::BaseDemo(
    u32 width,
    u32 height,
    std::string_view name) :
    WindowApp(width, height, name),
    m_renderModel(*this, true, "render model"),
    m_drawBVHDebugView(*this, true, "draw BVH debug view")
{
    initGfxContext(width, height);
    m_swapChain = SwapChain(
        getCurrentWindowSettings(),
        globalContext.device,
        globalContext.cmdQueue.queue),
    m_inputManager = &system::InputManager::GetInputManager();

    m_camera.initialize({ 0, 100, 0 }, { 0.f, 0.f, 1.f });
    onResize(width, height);
}

void BaseDemo::onResize(
    uint width,
    uint height)
{
    flushGPU();
    setWindowSize(width, height);
    if(width==0 && height==0)
        return;

    WindowApp::onResize(width, height);
    m_swapChain.onResize(getCurrentWindowSettings());
    // depth
    {
        DescriptorProperties viewProps{
            .descriptor = DescriptorFlags::DepthStencil,
            .viewDimension = D3D12_SRV_DIMENSION_TEXTURE2D,
        };
        ResourceDescription desc{
                .format = DXGI_FORMAT_D24_UNORM_S8_UINT,
                .width= width,
                .height = height,
                .depthOrArraySize = 1,
                .dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D,
                .flags = ResourceFlags::DEPTH_STENCIL,
                .createState = ResourceState::DEPTH_WRITE ,
                .heapType = D3D12_HEAP_TYPE_DEFAULT,
                .name = "depthStencil"
        };
        D3D12_CLEAR_VALUE val;
        val.DepthStencil = { 1.0f, 0 };
        val.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
        m_depthStencil.initResource(
            globalContext.device,
            desc, viewProps, &val);
    }

    math::ProjectionProps perspectiveProps;
    perspectiveProps.perspective.fov = 90;
    perspectiveProps.perspective.aspectRatio = m_swapChain.getAspectRatio();
    perspectiveProps.perspective.nearZ = 0.1f;
    perspectiveProps.perspective.farZ = 10000.f;
    perspectiveProps.type = math::ProjectionType::Perspective;

    m_camera.setProjectionProperties(perspectiveProps);
    m_camera.processUpdate();

}

SwapChainSettings BaseDemo::getCurrentWindowSettings()
{
    return { getWidth(), getHeight(), DXGI_FORMAT_R8G8B8A8_UNORM,
             getWindowHandle(), false, false};
}

void BaseDemo::compileShaders()
{
    // shaders
    m_shaders.clear();
    auto getType = [](u32 type) -> DXGI_FORMAT
        {
            if(type == 3)
                return DXGI_FORMAT_R32G32B32_FLOAT;
            if(type == 4)
                return DXGI_FORMAT_R32G32B32A32_FLOAT;
            return DXGI_FORMAT_R32G32_FLOAT;
        };
    auto getInputElement = [getType](const char* name, 
                              u32& offset, 
                              u32 type) -> D3D12_INPUT_ELEMENT_DESC
        {
            u32 oldOffset = offset * sizeof(float);
            offset += type;
            return {
                name,
                0,
                getType(type), 0, oldOffset,  
                D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0};    };
    auto createShader = [this](TableEntry<DxBlob*>& entry)
    {
        if (entry.second != nullptr)
            m_shaders.push_back(entry);
    };
    {
        ShaderInfo info(createShader);
        info.shaderName = L"forwardVS";
        info.entryPoint = L"VertexMain";
        info.path       = L"Shaders/forward_render.hlsl";
        info.type       = ShaderType::VERTEX;
    }
    {
        ShaderInfo info(createShader);
        info.shaderName = L"forwardPS";
        info.entryPoint = L"PixelMain";
        info.path       = L"Shaders/forward_render.hlsl";
        info.type       = ShaderType::PIXEL;
    }
    {
        ShaderInfo info(createShader);
        info.shaderName = L"debugBVH_VS";
        info.entryPoint = L"VertexMain";
        info.path       = L"Shaders/debug_BVHdraw.hlsl";
        info.type       = ShaderType::VERTEX;
    }
    {
        ShaderInfo info(createShader);
        info.shaderName = L"debugBVH_PS";
        info.entryPoint = L"PixelMain";
        info.path       = L"Shaders/debug_BVHdraw.hlsl";
        info.type       = ShaderType::PIXEL;
    }

    {
        RootParameters parameters = {
            RootParameter::CreateDescriptor(0),
            RootParameter::CreateConstants(1, 0, 10) };
          /*RootParameter::CreateConstants(1, 1, 10) };*/

        auto rootSignFlags = 
            RootSignatureFlags::ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
            RootSignatureFlags::SBV_SRV_HEAP_DIRECT_INDEX;

        m_rootSignature.init(globalContext.device(), parameters, rootSignFlags);
        u32 offset = 0;
        D3D12_INPUT_ELEMENT_DESC inputElements[] ={
            getInputElement("POSITION", offset, 3),
            getInputElement("NORMAL", offset, 3),
            getInputElement("TANGENT", offset, 4),
            getInputElement("UV", offset, 2),
        };
        D3D12_INPUT_LAYOUT_DESC inputLayout {inputElements, 4};

        ShaderInputGroup sig;
        sig.desc = inputLayout;
        sig.vertexShader = getShader(*util::FindElement(m_shaders, L"forwardVS"));
        sig.pixelShader = getShader(*util::FindElement(m_shaders, L"forwardPS"));
        sig.rootSignature = &m_rootSignature;

        RenderState renderState;
        renderState.setShaderInputGroup(sig);
        m_pso = PSO(renderState);
    }
    {
        RootParameters parameters = { RootParameter::CreateConstants(1, 0, 10) };

        auto rootSignFlags = 
            RootSignatureFlags::ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
            RootSignatureFlags::SBV_SRV_HEAP_DIRECT_INDEX;
        m_bvhDebugDrawRS.init(globalContext.device(), parameters, rootSignFlags);
        u32 offset = 0;
        D3D12_INPUT_ELEMENT_DESC inputElements[] ={
            getInputElement("POSITION", offset, 3)
        };
        D3D12_INPUT_LAYOUT_DESC inputLayout {inputElements, 1};

        ShaderInputGroup sig;
        sig.desc = inputLayout;
        sig.vertexShader = getShader(*util::FindElement(m_shaders, L"debugBVH_VS"));
        sig.pixelShader = getShader(*util::FindElement(m_shaders, L"debugBVH_PS"));
        sig.rootSignature = &m_bvhDebugDrawRS;

        RenderState renderState;
        renderState.setShaderInputGroup(sig);
        renderState.topology = D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE;
        m_bvhDebugDrawPSO = PSO(renderState);
    }

}

bool BaseDemo::initialize()
{
    LogScope("BaseDemo");

    imgui::Init(getWindowHandle(), globalContext.device(), config::NumFrames);

    ShaderManager::InitializeCompiler();
    compileShaders();
    
    resetList(0);

    // setup data 
    ConstandBufferData data;
    data.projection = m_camera.getProjectionMatrix();
    data.view = m_camera.getViewMatrix();
    m_constBuffer = ConstantBuffer(globalContext.device,
                                   globalContext.cmdList, &data, 1);

    LightSettings lightSettings;
    lightSettings.cameraPosition = m_camera.getPos();
    lightSettings.viewDirection = m_camera.getDir();
    m_lightSettingsResource= ConstantBuffer(globalContext.device,
                                   globalContext.cmdList, &lightSettings, 1);

    if(1)
        m_model.initGLTF(config::g_state.homeDir/"data/Sponza/glTF/Sponza.gltf",
                         globalContext);
    else if (0)
        m_model.initOBJ(config::g_state.homeDir/ "data/teapot.obj", globalContext);

    if(m_model.isInitialized())
    {
        Profiler::StartProfiling();
        m_bvhBuilder.build(m_model);
        Profiler::EndProfiling();

        using func = std::function<Vector3(const BVHNode&)>;
        func diagonalF = [](const BVHNode& node) {
            return node.aabb.diagonal(); };

        std::function<Vector3(const BVHNode&)> aabbF = [](const BVHNode& node){
            return node.aabb.aabbMin;};
        m_bvhModel = BVHBuilder::generateDrawData(globalContext,
                                                  m_bvhBuilder.getRootNode(),
                                                  m_bvhBuilder.getNodeCount(),
                                                  diagonalF, aabbF);
    }

    if(m_model.isInitialized())
    {
        auto& geometry = m_model.m_geometry;
        std::vector<tinybvh::bvhvec4> vertices(geometry.vertices.size());
        std::vector<u32> indices(geometry.indices.size());
        locGenerateTinyBVHCompatibleGeometry(vertices, indices, geometry.vertices, geometry.indices, m_model.m_submeshes);

        tinybvh::BVH bvh;
        Profiler::StartProfiling();
        {
            PROFILER("tiny BVH state of art");
            bvh.Build(vertices.data(), indices.data(),
                      (u32)geometry.indices.size() / 3);
        }
        Profiler::EndProfiling();
        util::printInfo("tiny bvh node number {}", bvh.NodeCount());

        using func = std::function<Vector3(const tinybvh::BVH::BVHNode&)>;
        auto from_tinyV3_to_mathV3 = [](tinybvh::bvhvec3 vec) -> Vector3 
            { return Vector3({vec.x, vec.y, vec.z}); };

        func diagonalF =
            [from_tinyV3_to_mathV3](const tinybvh::BVH::BVHNode& node) {
            return from_tinyV3_to_mathV3(node.aabbMax - node.aabbMin);};

        func aabbF = 
            [from_tinyV3_to_mathV3](const tinybvh::BVH::BVHNode& node)  {
            return from_tinyV3_to_mathV3(node.aabbMin);};

        m_tinybvhModel = BVHBuilder::generateDrawData(globalContext,
                                                      bvh.bvhNode,
                                                      bvh.NodeCount(),
                                                      diagonalF, aabbF);
    }

    m_camera.addChangeCallback([this](const Camera* camera)
    {
        ConstandBufferData data;
        data.projection = camera->getProjectionMatrix();
        data.view = camera->getViewMatrix();
        this->m_constBuffer.update(&data);

        LightSettings lightSettings;
        lightSettings.cameraPosition = m_camera.getPos();
        lightSettings.viewDirection = m_camera.getDir();
        this->m_lightSettingsResource.update(&lightSettings);
    });

    executeAll();

    signal(0);
    waitForFence(0);

    return true;
}

void BaseDemo::draw()
{
    Timer timer("draw");
    startFrame();
    ID3D12GraphicsCommandList* cmdList = globalContext.currentCmdList;

    u32 swapChainBufferIndex = m_swapChain.changeState(
                                cmdList,
                                ResourceState::RENDER_TARGET);
    // forward rendering 
    {
        const float clearColor[] = { .0f, 0.0f, .0f, 1.0f };
        auto renderTarget = m_swapChain.getView(swapChainBufferIndex);

        cmdList->OMSetRenderTargets(1, &renderTarget.HandleCPU,
                                    true, &m_depthStencil.dsv.HandleCPU);
        cmdList->ClearRenderTargetView(renderTarget.HandleCPU,
                                       clearColor, 0, nullptr);
        cmdList->ClearDepthStencilView(m_depthStencil.dsv.HandleCPU, 
                           D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL,
                           1.f, 0, 0, nullptr);

        cmdList->SetDescriptorHeaps(1, 
                DescriptorHeapManager::CurrentSRVHeap.getHeapAddress());
        cmdList->SetGraphicsRootSignature(m_rootSignature);
        cmdList->SetPipelineState(m_pso);

        cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

        /*struct */
        /*{*/
        /*    int materialArrayIndex;*/
        /*} passIndices;*/
        u32 passIndices =
            m_model.m_materialBuffer.getDescriptorHeapIndex();
        cmdList->SetGraphicsRootConstantBufferView(
            0,
            m_constBuffer->GetGPUVirtualAddress());
        cmdList->SetGraphicsRoot32BitConstant(1, passIndices, 0);

        if(m_model.isInitialized())
        {
            auto vertexBuffer = GetVertexBufferView(m_model.m_mesh.m_vertexBuffer);
            auto indexBuffer = GetIndexBufferView(m_model.m_mesh.m_indexBuffer);
            cmdList->IASetVertexBuffers(0, 1, &vertexBuffer);
            cmdList->IASetIndexBuffer(&indexBuffer);

            if(m_renderModel.getData())
            for (auto& submesh : m_model.m_submeshes)
            {
                cmdList->SetGraphicsRoot32BitConstant(1, submesh.materialIndex, 0);
                submesh.draw(cmdList);
            }
        }
    }
    // BVH debug draw
    if(m_drawBVHDebugView.getData() && m_tinybvhModel.isInitialized())
    {
        cmdList->SetGraphicsRootSignature(m_bvhDebugDrawRS);
        cmdList->SetPipelineState(m_bvhDebugDrawPSO);
        cmdList->SetGraphicsRoot32BitConstant(
            0, m_constBuffer.getDescriptorHeapIndex(), 0);
        cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_LINELIST);

        auto mesh = m_bvhModel.m_mesh;
        auto submesh = m_bvhModel.m_submeshes[0];
        auto vertexBuffer = GetVertexBufferView(mesh.m_vertexBuffer);
        auto indexBuffer = GetIndexBufferView(mesh.m_indexBuffer);
        cmdList->IASetVertexBuffers(0, 1, &vertexBuffer);
        cmdList->IASetIndexBuffer(&indexBuffer);
        submesh.draw(cmdList);
    }
    else if(m_tinybvhModel.isInitialized())
    {
        cmdList->SetGraphicsRootSignature(m_bvhDebugDrawRS);
        cmdList->SetPipelineState(m_bvhDebugDrawPSO);
        cmdList->SetGraphicsRoot32BitConstant(0,
                                              m_constBuffer.getDescriptorHeapIndex(),
                                              0);
        cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_LINELIST);

        auto mesh = m_tinybvhModel.m_mesh;
        auto submesh = m_tinybvhModel.m_submeshes[0];
        auto vertexBuffer = GetVertexBufferView(mesh.m_vertexBuffer);
        auto indexBuffer = GetIndexBufferView(mesh.m_indexBuffer);
        cmdList->IASetVertexBuffers(0, 1, &vertexBuffer);
        cmdList->IASetIndexBuffer(&indexBuffer);
        submesh.draw(cmdList);
    }

    imgui::StartFrame();
    {
        imgui::Begin("Settings");

        // light UI
        auto& uiElements = UI_ElementInterface::s_uiElements;
        for(UI_ElementInterface* uiElement: uiElements)
            uiElement->onUIAction();

        imgui::End();
    }
    imgui::EndFrame(cmdList);

    m_swapChain.changeState(cmdList, ResourceState::PRESENT);

    executeAll();
    m_swapChain.Present();

    // sync
    signal();
}

void BaseDemo::update()
{
    Timer timer("update");

    nextFrame(m_swapChain.getCurrentIndex());
    waitForFrame();

    m_camera.update();

    if (m_inputManager->getKeyState(system::Key::C).isPressed())
        compileShaders();
}
void BaseDemo::destroy()
{
    for (int i = 0; i < config::NumFrames; ++i)
        waitForFrame(i);
    m_model.reset();
    m_buffer.reset();
    m_constBuffer.reset();
}

LRESULT BaseDemo::processInput(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    return system::InputManager::GetInputManager().processInput(hwnd, msg, wParam, lParam);
}
