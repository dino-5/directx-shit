#include <dxgiformat.h>
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
using namespace gfx;
using namespace util;
using namespace DirectX;

void locGenerateTinyBVHCompatibleGeometry(std::vector<tinybvh::bvhvec4>& outVertices, std::vector<u32>& outIndices,
                                          const std::vector<Vertex>& vertices, const std::vector<u32>& indices, const std::vector<Submesh>& submeshes)
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
            for(u32 i = submesh.StartIndexLocation; i < submesh.StartIndexLocation + submesh.IndexCount; i+=3)
            {
                auto ind = &indices[i];
                outIndices[currentIndex++] = ind[0] + submesh.BaseVertexLocation;
                outIndices[currentIndex++] = ind[1] + submesh.BaseVertexLocation;
                outIndices[currentIndex++] = ind[2] + submesh.BaseVertexLocation;
            }
        }
    }
}


Light::Light(math::Vector3 vec)
    : m_position(vec)
{
}

BaseDemo::BaseDemo(u32 width, u32 height, std::string_view name) :
    WindowApp(width, height, name),
    m_cmdList(m_device),
    m_cmdQueue(m_device.native()),
    m_swapChain(getCurrentWindowSettings(), m_device, m_cmdQueue.getQueue()),
    m_renderModel(*this, true, "render model"),
    m_drawBVHDebugView(*this, true, "draw BVH debug view")
{
    m_inputManager = &system::InputManager::GetInputManager();
    m_device.createFence(&m_fence);

    DescriptorHeapManager::CreateDSVHeap(20);
    DescriptorHeapManager::CreateRTVHeap(100);
    DescriptorHeapManager::CreateSRVHeap(200);

    m_fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
    if (m_fenceEvent == nullptr)
    {
        ThrowIfFailed(HRESULT_FROM_WIN32(GetLastError()));
    }

    m_camera.initialize({ 0, 100, 0 }, { 0.f, 0.f, 1.f });
    onResize(width, height);
}

void BaseDemo::onResize(uint width, uint height)
{
    flushGPU();
    if(width==0 && height==0)
        return;

    WindowApp::onResize(width, height);
    m_swapChain.onResize(getCurrentWindowSettings());

    m_viewPort.TopLeftX = 0;
    m_viewPort.TopLeftY = 0;
    m_viewPort.Width = (float)width;
    m_viewPort.Height = (float)height;
    m_viewPort.MaxDepth = 1.0;
    m_viewPort.MinDepth = .0;

    m_scissorRect = { 0, 0, static_cast<long>(width), static_cast<long>(height) };

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
        m_depthStencil.initResource(m_device.getDevice(), desc, viewProps, &val);
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
    return { getWidth(), getHeight(), DXGI_FORMAT_R8G8B8A8_UNORM, getWindowHandle(), m_device.checkForFeatureSupport(), false};
}

void BaseDemo::compileShaders()
{
    // shaders
    m_shaders.clear();
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
        RootParameters parameters = { RootParameter::CreateConstants(2, 0, 10) };
                                      /*RootParameter::CreateConstants(1, 1, 10) };*/

        auto rootSignFlags = RootSignatureFlags::ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT | RootSignatureFlags::SBV_SRV_HEAP_DIRECT_INDEX;
        m_rootSignature.init(m_device.getDevice(), parameters, rootSignFlags);
        D3D12_INPUT_ELEMENT_DESC inputElements[] ={
            { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,    0, 0,  D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
            { "NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT,    0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
            { "TANGENT",  0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 24, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
            { "UV",       0, DXGI_FORMAT_R32G32_FLOAT,       0, 40, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
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

        auto rootSignFlags = RootSignatureFlags::ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT | RootSignatureFlags::SBV_SRV_HEAP_DIRECT_INDEX;
        m_bvhDebugDrawRS.init(m_device.getDevice(), parameters, rootSignFlags);
        D3D12_INPUT_ELEMENT_DESC inputElements[] ={
            { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,  D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
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

u64 BaseDemo::signal(graphics::CommandQueue& queue, ID3D12Fence* fence, u64& value)
{
    u64 valueForSignal = ++value;
    ThrowIfFailed(queue->Signal(fence, valueForSignal));

    return valueForSignal;
}

void BaseDemo::waitForFence(ID3D12Fence* fence, HANDLE& fenceEvent, u64 fenceValue)
{
    fence->SetEventOnCompletion(fenceValue, fenceEvent);
    WaitForSingleObjectEx(fenceEvent, INFINITE, FALSE);
}

void BaseDemo::flushGPU()
{
    waitForFence(m_fence, m_fenceEvent, m_fenceValue);
}


bool BaseDemo::initialize()
{
    LogScope("BaseDemo");

    imgui::Init(getWindowHandle(), m_device.getDevice(), config::NumFrames);

    ShaderManager::InitializeCompiler();
    compileShaders();

    GfxContext context;
    context.cmdList = m_cmdList.reset(0);
    context.device = m_device.getDevice();

    // setup data 
    ConstandBufferData data;
    data.projection = m_camera.getProjectionMatrix();
    data.view = m_camera.getViewMatrix();
    m_constBuffer = ConstantBuffer(context, &data, 1);

    LightSettings lightSettings;
    lightSettings.cameraPosition = m_camera.getPos();
    lightSettings.viewDirection = m_camera.getDir();
    m_lightSettingsResource= ConstantBuffer(context, &lightSettings, 1);

    if(1)
        m_model.initGLTF(config::g_state.homeDir/ "data/Sponza/glTF/Sponza.gltf", context);
    else if (0)
        m_model.initOBJ(config::g_state.homeDir/ "data/teapot.obj", context);

    if(m_model.isInitialized())
    {
        Profiler::StartProfiling();
        m_bvhBuilder.build(m_model);
        Profiler::EndProfiling();
        std::function<Vector3(const BVHNode&)> diagonalF = [](const BVHNode& node) -> Vector3 { return node.aabb.diagonal();};
        std::function<Vector3(const BVHNode&)> aabbF = [](const BVHNode& node) -> Vector3 { return node.aabb.aabbMin;};
        m_bvhModel = BVHBuilder::generateDrawData(context, m_bvhBuilder.getRootNode(), m_bvhBuilder.getNodeCount(), diagonalF, aabbF);
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
            bvh.Build(vertices.data(), indices.data(), (u32)geometry.indices.size() / 3);
        }
        Profiler::EndProfiling();
        util::printInfo("tiny bvh node number {}", bvh.NodeCount());

        auto from_tinyV3_to_mathV3 = [](tinybvh::bvhvec3 vec) -> Vector3 { return Vector3({vec.x, vec.y, vec.z}); };
        std::function<Vector3(const tinybvh::BVH::BVHNode&)> diagonalF = [from_tinyV3_to_mathV3](const tinybvh::BVH::BVHNode& node) -> Vector3 {
            return from_tinyV3_to_mathV3(node.aabbMax - node.aabbMin);};
        std::function<Vector3(const tinybvh::BVH::BVHNode&)> aabbF = [from_tinyV3_to_mathV3](const tinybvh::BVH::BVHNode& node) -> Vector3 {
            return from_tinyV3_to_mathV3(node.aabbMin);};
        m_tinybvhModel = BVHBuilder::generateDrawData(context, bvh.bvhNode, bvh.NodeCount(), diagonalF, aabbF);
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

    ThrowIfFailed(context.cmdList->Close());
    ID3D12CommandList* ppCommandLists[] = { context.cmdList };
    m_cmdQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

    m_swapChain.m_fence[0] = signal(m_cmdQueue, m_fence, m_fenceValue);
    waitForFence(m_fence, m_fenceEvent, m_swapChain.m_fence[0]);

    return true;
}

void BaseDemo::draw()
{
    Timer timer("draw");
    ID3D12GraphicsCommandList* cmdList = m_cmdList.reset(m_currentFrameIndex);
    m_graphicsContext.cmdList = cmdList;
    m_graphicsContext.device = m_device.getDevice();

    u32 swapChainBufferIndex = m_swapChain.changeState(cmdList, ResourceState::RENDER_TARGET);

    // forward rendering 
    {
        const float clearColor[] = { .0f, 0.0f, .0f, 1.0f };
        auto renderTarget = m_swapChain.getView(swapChainBufferIndex);

        cmdList->RSSetViewports(1, &m_viewPort);
        cmdList->RSSetScissorRects(1, &m_scissorRect);

        cmdList->OMSetRenderTargets(1, &renderTarget.HandleCPU, true, &m_depthStencil.dsv.HandleCPU);
        cmdList->ClearRenderTargetView(renderTarget.HandleCPU, clearColor, 0, nullptr);
        cmdList->ClearDepthStencilView(m_depthStencil.dsv.HandleCPU, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.f, 0, 0, nullptr);

        cmdList->SetDescriptorHeaps(1, gfx::DescriptorHeapManager::CurrentSRVHeap.getHeapAddress());
        cmdList->SetGraphicsRootSignature(m_rootSignature);
        cmdList->SetPipelineState(m_pso);

        cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

        struct 
        {
            int viewSettingsIndex;
            int materialArrayIndex;
        } passIndices;
        passIndices.viewSettingsIndex = m_constBuffer.getDescriptorHeapIndex();
        passIndices.materialArrayIndex = m_model.m_materialBuffer.getDescriptorHeapIndex();
        cmdList->SetGraphicsRoot32BitConstants(0, 2, &passIndices, 0);

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
        cmdList->SetGraphicsRoot32BitConstant(0, m_constBuffer.getDescriptorHeapIndex(), 0);
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
        cmdList->SetGraphicsRoot32BitConstant(0, m_constBuffer.getDescriptorHeapIndex(), 0);
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
    ThrowIfFailed(cmdList->Close());
    ID3D12CommandList* ppCommandLists[] = { cmdList};
    m_cmdQueue->ExecuteCommandLists(1, ppCommandLists);

    m_swapChain.Present();

    // sync
    m_swapChain.m_fence[m_currentFrameIndex] = signal(m_cmdQueue, m_fence, m_fenceValue);
}

void BaseDemo::waitForFrame(u32 index)
{
    u64 fenceValue = m_fence->GetCompletedValue();
    if (fenceValue < m_swapChain.m_fence[index])
        waitForFence(m_fence, m_fenceEvent, m_swapChain.m_fence[index]);
}

void BaseDemo::update()
{
    Timer timer("update");

    m_currentFrameIndex = m_swapChain.getCurrentIndex();
    waitForFrame(m_currentFrameIndex);

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
