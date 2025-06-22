#include <dxgiformat.h>
#include <format>
#include <functional>
#include <iterator>
#include <ostream>
#include <string_view>
#include "BaseDemo.h"
#include "EngineCommon/util/Logger.h"
#include "EngineCommon/util/CommandLine.h"
#include "EngineCommon/util/ImGuiSettings.h"
#include "EngineCommon/util/Timer.h"
#include "EngineGfx/tiny_bvh.h"
#include "RenderPasses.h"

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
    m_drawBVHDebugView(*this, true, "draw BVH debug view"),
    m_outputColor(*this, Vector3({1.f, 1.f, 0}), "color", 1.f)
{
    initGfxContext(width, height);
    initRenderPassResources(globalContext);
    m_swapChain = SwapChain(
        getCurrentWindowSettings(),
        globalContext.device,
        globalContext.cmdQueue.queue),
    m_inputManager = &system::InputManager::GetInputManager();

    m_camera.initialize({ 0, 100, 0 }, { 0.f, 0.f, 1.f });
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
    perspectiveProps.perspective.aspectRatio = 
        m_swapChain.getAspectRatio();
    perspectiveProps.perspective.nearZ = 0.1f;
    perspectiveProps.perspective.farZ = 10000.f;
    perspectiveProps.type = math::ProjectionType::Perspective;

    m_camera.setProjectionProperties(perspectiveProps);
    m_camera.processUpdate();

    for(auto& renderPass : m_renderPasses)
        renderPass.resize(globalContext);

}

SwapChainSettings BaseDemo::getCurrentWindowSettings()
{
    return { getWidth(), getHeight(), DXGI_FORMAT_R8G8B8A8_UNORM,
             getWindowHandle(), false, false};
}

struct RenderPassDesc
{
    RenderPassInit init;
    RenderPassResize resize;
    RenderPassDraw execute;
    bool enabled;
};

void BaseDemo::createRenderPasses()
{
    RenderPassDesc renderPassDesc[RenderPassCount] = {
        {forwardPassInit, 0, forwardPassExecute,  0} ,
        {debugDrawBVHPassInit, 0, debugDrawBVHPassExecute, 0} ,
        {computeRTXPassInit, computeRTXPassResize, computeRTXPassExecute, 1}
    };
    
    for(int i = 0; i < RenderPassCount; ++i)
        m_renderPasses[i] = CreateRenderPass(renderPassDesc[i].init,
                                             renderPassDesc[i].resize,
                                             renderPassDesc[i].execute,
                                             renderPassDesc[i].enabled,
                                             globalContext);
}

bool BaseDemo::initialize()
{
    LogScope("BaseDemo");

    imgui::Init(getWindowHandle(),
                globalContext.device(), config::NumFrames);

    ShaderManager::InitializeCompiler();

    resetList(0);
    createRenderPasses();
    onResize(getWidth(), getHeight());

    // setup data 
    GfxViewData data;
    data.projectionMatrix = m_camera.getProjectionMatrix();
    data.viewMatrix = m_camera.getViewMatrix();
    data.cameraPos = m_camera.getPos();
    data.cameraViewDir = m_camera.getViewDir();
    data.cameraRightDir = m_camera.getRightDir();
    data.cameraUpDir = m_camera.getUpDir();
    data.fov = 90.f;
    createView(data);

    if(0)
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

        std::function<Vector3(const BVHNode&)> aabbF =
            [](const BVHNode& node)
            {
            return node.aabb.aabbMin;};
        m_bvhModel = BVHBuilder::generateDrawData(
                        globalContext,
                        m_bvhBuilder.getRootNode(),
                        m_bvhBuilder.getNodeCount(),
                        diagonalF, aabbF);
    }

    if(m_model.isInitialized())
    {
        auto& geometry = m_model.m_geometry;
        std::vector<tinybvh::bvhvec4> vertices(
            geometry.vertices.size());
        std::vector<u32> indices(geometry.indices.size());
        locGenerateTinyBVHCompatibleGeometry(
                        vertices,
                        indices,
                        geometry.vertices,
                        geometry.indices,
                        m_model.m_submeshes);

        tinybvh::BVH bvh;
        Profiler::StartProfiling();
        {
            PROFILER("tiny BVH state of art");
            bvh.Build(vertices.data(), indices.data(),
                      (u32)geometry.indices.size() / 3);
        }
        Profiler::EndProfiling();
        util::printInfo("tiny bvh node number {}", bvh.NodeCount());

        using func = std::function<Vector3(
            const tinybvh::BVH::BVHNode&)>;
        auto from_tinyV3_to_mathV3 = [](tinybvh::bvhvec3 vec) 
            { return Vector3({vec.x, vec.y, vec.z}); };

        func diagonalF =
        [from_tinyV3_to_mathV3](const tinybvh::BVH::BVHNode& node) 
            {
        return from_tinyV3_to_mathV3(node.aabbMax - node.aabbMin);};

        func aabbF = 
        [from_tinyV3_to_mathV3](const tinybvh::BVH::BVHNode& node)  
        {
            return from_tinyV3_to_mathV3(node.aabbMin);};

        m_tinybvhModel = BVHBuilder::generateDrawData(globalContext,
                                                      bvh.bvhNode,
                                                      bvh.NodeCount(),
                                                  diagonalF, aabbF);
    }

    m_camera.addChangeCallback([](const Camera* camera)
    {
        GfxViewData data;
        data.projectionMatrix = camera->getProjectionMatrix();
        data.viewMatrix = camera->getViewMatrix();

        data.cameraPos = camera->getPos();
        data.cameraViewDir = camera->getViewDir();
        data.cameraRightDir = camera->getRightDir();
        data.cameraUpDir = camera->getUpDir();
        data.fov = 90.f;
        updateView(data);
    });

    RTXDescription rtxDesc;
    rtxDesc.sphereCount = sphereCount;
    rtxDesc.imWidth = getWidth();
    rtxDesc.imHeight = getHeight();
    rtxDesc.color = m_outputColor.getData();

    m_rtxData.description = rtxDesc;
    m_rtxData.sphereArray[0] = {
        {0.f, 0.f, -1.f},
        10.f,
        {{1.f, 0.f, 1.f, 1.f}, Lambertian}
    };

    m_rtxData.sphereArray[1] = {
        {0.f, -1010.f, -1.f},
        1000.f,
        {{1.f, 1.f, 0.f, 1.f}, Lambertian}
    };

    for(int i = 0; i < sphereCount; i++)
    {
        m_sphereUI[i].init(*this, m_rtxData.sphereArray[i], 
                           std::format("Sphere {}", i));
        m_sphereUI[i].index = i;
    }

    executeAll();

    signal(0);
    waitForFence(0);

    return true;
}

void BaseDemo::draw()
{
    Timer timer("draw");
    resetList(globalContext.currentFrameIndex); 
    ID3D12GraphicsCommandList* cmdList = globalContext.currentCmdList;
    cmdList->RSSetViewports(1, &globalContext.viewPort);

    u32 swapChainBufferIndex = m_swapChain.changeState(
                                cmdList,
                                ResourceState::RENDER_TARGET);
    globalContext.currentRenderTarget = 
        &m_swapChain.getRenderTarget(swapChainBufferIndex);
    globalContext.currentDepthStencil = &m_depthStencil;

    startFrame();
    
    // forward rendering 
    ForwardPassData data;
    data.drawModel = m_renderModel.getData();
    m_renderPasses[ForwardPass].execute(globalContext, &m_model, &data);
    
    // BVH debug draw
    if(m_drawBVHDebugView.getData() && m_tinybvhModel.isInitialized())
    {
        m_renderPasses[BVHDebugPass].execute(globalContext, &m_bvhModel);
    }
    else if(m_tinybvhModel.isInitialized())
    {
        m_renderPasses[BVHDebugPass].execute(globalContext, &m_tinybvhModel);
    }


    m_renderPasses[RTXComputePass].execute(globalContext, nullptr, &m_rtxData);

    imgui::StartFrame();
    {
        imgui::Begin("Settings");

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

    RTXDescription rtxDesc;
    rtxDesc.sphereCount = 2;
    rtxDesc.imWidth = getWidth();
    rtxDesc.imHeight = getHeight();
    rtxDesc.color = m_outputColor.getData();

    for(u32 i = 0; i < sphereCount; ++i)
        m_rtxData.sphereArray[i] = m_sphereUI[i].getData();

    m_rtxData.description = rtxDesc;

    for(auto& pass : m_renderPasses)
        pass.compilePSO();
}
void BaseDemo::destroy()
{
    for (int i = 0; i < config::NumFrames; ++i)
        waitForFrame(i);
    m_model.reset();
}

LRESULT BaseDemo::processInput(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    return system::InputManager::GetInputManager().processInput(
        hwnd, msg, wParam, lParam);
}
