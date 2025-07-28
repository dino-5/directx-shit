#include <dxgiformat.h>
#include <format>
#include <functional>
#include <ios>
#include <iterator>
#include <ostream>
#include <string_view>
#include "BaseDemo.h"
#include "EngineCommon/util/Logger.h"
#include "EngineCommon/util/CommandLine.h"
#include "EngineCommon/util/ImGuiSettings.h"
#include "EngineCommon/util/Timer.h"
#include "EngineGfx/tiny_bvh.h"
#include "EngineGfx/dx12/GfxHelper.h"
#include "RenderPasses.h"
#include <random>

using namespace std;
using namespace util;
using namespace DirectX;

inline float random_float() {
    static std::uniform_real_distribution<float> distribution(0.0, 1.0);
    static std::mt19937 generator;
    return distribution(generator);
}

inline float random_float(float min, float max) {
    // Returns a random real in [min,max).
    return min + (max-min)*random_float();
}

Vector3 randomColor(float min = 0.f, float max = 1.f)
{
    return Vector3 { random_float(), random_float(),random_float() };
}

inline void locCreateViewData(GfxViewData& data, const Camera& camera)
{
    data.projectionMatrix = camera.getProjectionMatrix();
    data.viewMatrix = camera.getViewMatrix();
    data.cameraPos = camera.getPos();
    data.cameraViewDir = camera.getViewDir();
    data.cameraRightDir = camera.getRightDir();
    data.cameraUpDir = camera.getUpDir();
    data.fov = 90.f;
}

void RTX_BVH::createBVH(Sphere* array, u32 arrayCount)
{
    indices.resize(arrayCount);
    aabbs.resize(arrayCount);
    nodes.resize(2*arrayCount-1);

    u32 i = 0;
    for(auto& index : indices)
    {
        index = i++;
    }

    for(int i=0; i<arrayCount; i++)
    {
        Vector3 center = array[i].center;
        float r = array[i].radius;
        aabbs[i] = {center - r, center + r};
    }

    nodes[0].first = 0;
    nodes[0].count = arrayCount;
    count++;

    for(auto& aabb : aabbs) // NOTE:: valid only in this case in all other use indices
    {
        nodes[0].aabb.grow(aabb);
    }

    uint32_t task[256], taskCount = 0, nodeIdx = 0;

    while(1)
    {
        while(1)
        {
            auto& node = nodes[nodeIdx];
            if(node.count<=2)
                break;

            Vector3 diag = node.aabb.diagonal();
            u32 longestAxis = 0;
            if(diag[1] > diag[longestAxis]) longestAxis = 1;
            if(diag[2] > diag[longestAxis]) longestAxis = 2;

            float split = node.aabb.min[longestAxis] + diag[longestAxis]/2; 
            u32 leftEnd = node.first;

            for(u32 idx=leftEnd, end=idx+node.count; idx<end; idx++)
            {
                int i = indices[idx];
                float pos = aabbs[i].middle()[longestAxis];
                if(pos <= split)
                    std::swap(indices[leftEnd++], indices[idx]);
            }

            if(leftEnd == node.first || leftEnd == node.first+node.count)
                break;

            u32 leftIdx = count++, rightIdx = count++;
            auto& leftNode = nodes[leftIdx];
            auto& rightNode = nodes[rightIdx];

            leftNode.first = node.first;
            leftNode.count = leftEnd - node.first;
            rightNode.first = leftEnd;
            rightNode.count = node.count - leftNode.count;

            node.first = leftIdx;
            node.count = 0;
            nodeIdx = leftIdx;
            task[taskCount++] = rightIdx;

            for(u32 idx=leftNode.first, end=idx+leftNode.count; idx<end; idx++)
            {
                leftNode.aabb.grow(aabbs[indices[idx]]);
            }

            for(u32 idx=rightNode.first, end=idx+rightNode.count; idx<end; idx++)
            {
                rightNode.aabb.grow(aabbs[indices[idx]]);
            }
        }
        if(taskCount==0)
            break;
        nodeIdx = task[--taskCount];
    }

}


void locGenerateTinyBVHCompatibleGeometry(
    std::vector<tinybvh::bvhvec4>& outVertices,
    std::vector<u32>& outIndices,
    const std::vector<Vertex>& vertices,
    const std::vector<u32>& indices,
    const std::vector<Submesh>& submeshes)
{
    u32 currentIndex = 0;
    auto convertV3_to_V4 = [](const math::Vector3& vec) 
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
                outIndices[currentIndex++] =
                        ind[0] + submesh.BaseVertexLocation;
                outIndices[currentIndex++] =
                        ind[1] + submesh.BaseVertexLocation;
                outIndices[currentIndex++] =
                        ind[2] + submesh.BaseVertexLocation;
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
    m_drawRTXBVHDebugView(*this, false, "draw RTX BVH debug view"),
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

    math::ProjectionProps perspectiveProps;
    perspectiveProps.perspective.fov = 90;
    perspectiveProps.perspective.aspectRatio = m_swapChain.getAspectRatio();
    perspectiveProps.perspective.nearZ = 0.1f;
    perspectiveProps.perspective.farZ = 10000.f;
    perspectiveProps.type = math::ProjectionType::Perspective;

    m_camera.setProjectionProperties(perspectiveProps);
    m_camera.processUpdate();
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
        D3D12_CLEAR_VALUE val;
        val.DepthStencil = { 1.0f, 0 };
        val.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
        m_depthStencil.initResource(
            globalContext.device,
            getDepthStencilDesc(width, height),
            viewProps, &val);
    }

    for(auto& renderPass : m_renderPasses)
        renderPass.resize(globalContext);

}

SwapChainSettings BaseDemo::getCurrentWindowSettings()
{
    return { getWidth(), getHeight(), DXGI_FORMAT_R8G8B8A8_UNORM,
             getWindowHandle(), false, false};
}


void BaseDemo::createRenderPasses()
{
    RenderPassDesc renderPassDesc[RenderPassCount] = {
        {forwardPassInit, 0, forwardPassExecute,  0} ,
        {debugDrawBVHPassInit, 0, debugDrawBVHPassExecute, 1} ,
        {computeRTXPassInit, computeRTXPassResize, computeRTXPassExecute, 1}
    };
    
    for(int i = 0; i < RenderPassCount; ++i)
        m_renderPasses[i] = CreateRenderPass(renderPassDesc[i], globalContext);
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
    locCreateViewData(data, m_camera);
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
        using leaf = std::function<bool(const BVHNode&)>;
        func diagonalF = [](const BVHNode& node) { return node.aabb.diagonal(); };
        func aabbF = [](const BVHNode& node) { return node.aabb.min;};

        leaf leafF=[](const BVHNode& node){return node.isLeaf();};

        m_bvhModel = BVHBuilder::generateDrawData(
                        globalContext,
                        m_bvhBuilder.getRootNode(),
                        m_bvhBuilder.getNodeCount(),
                        diagonalF, aabbF, leafF);
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

        using func = std::function<Vector3( const tinybvh::BVH::BVHNode&)>;
        using leaf = std::function<bool( const tinybvh::BVH::BVHNode&)>;
        auto from_tinyV3_to_mathV3 = [](tinybvh::bvhvec3 vec) 
            { return Vector3({vec.x, vec.y, vec.z}); };

        func diagonalF =
        [from_tinyV3_to_mathV3](const tinybvh::BVH::BVHNode& node) 
            { return from_tinyV3_to_mathV3(node.aabbMax - node.aabbMin);};

        func aabbF = 
        [from_tinyV3_to_mathV3](const tinybvh::BVH::BVHNode& node)  
        { return from_tinyV3_to_mathV3(node.aabbMin);};

        leaf leafF=[](const tinybvh::BVH::BVHNode& node){return node.isLeaf();};
        m_tinybvhModel = BVHBuilder::generateDrawData(globalContext,
                                                      bvh.bvhNode,
                                                      bvh.NodeCount(),
                                                      diagonalF, aabbF, leafF);
    }

    m_camera.addChangeCallback([](const Camera* camera)
    {
        GfxViewData data;
        locCreateViewData(data, *camera);

        updateView(data);
    });

    int i = 0;
    Material ground_material = MakeLamberian(Vector3{0.5f, 0.5f, 0.5f});
    m_rtxData.sphereArray[i++] = {
        {.0f, -1000.f, .0f}, 1000, ground_material
    };

    int sphereCount = 9;

    if(1)
    for (int a = -sphereCount; a < sphereCount; a++) 
    {
        for (int b = -sphereCount; b < sphereCount; b++) 
        {
            float choose_mat = random_float();
            Vector3 center = Vector3(
                        a + 0.9f*random_float(),
                        0.2f,
                        b + 0.9f*random_float());

            if ((center - Vector3(4.f, 0.2f, 0.f)).length() > 0.9) 
            {
                Material sphere_material;

                if (choose_mat < 0.8) 
                {
                    // diffuse
                    auto albedo = randomColor() * randomColor();
                    sphere_material = MakeLamberian(albedo);

                }
                else if (choose_mat < 0.95) 
                {
                    // metal
                    auto albedo = randomColor(0.5, 1);
                    auto fuzz = random_float(0, 0.5);
                    sphere_material = MakeMetal(albedo, fuzz);
                } else 
                    // glass
                    sphere_material = MakeDielectric(1.5);

                m_rtxData.sphereArray[i++] = { center, 0.2f, sphere_material };
            }
        }
    }

    Material mat1 = MakeDielectric(1.5);
    m_rtxData.sphereArray[i++] = { Vector3(0.f, 1.f, 0.f), 1.0f, mat1 };

    Material mat2 = MakeLamberian(Vector3(0.4f, 0.2f, 0.1f));
    m_rtxData.sphereArray[i++] = { Vector3(-4.f, 1.f, 0.f), 1.0f, mat2 };

    Material mat3 = MakeMetal(Vector3(0.7f, 0.6f, 0.5f), 0);
    m_rtxData.sphereArray[i++] = { Vector3(4.f, 1.f, 0.f), 1.0f, mat3 };

    m_currentSphereCount = i;

    m_bvh.createBVH(m_rtxData.sphereArray, m_currentSphereCount);

    {
        using func = std::function<Vector3(const RTX_BVHNode&)>;
        using leaf = std::function<bool(const RTX_BVHNode&)>;
        func diagonalF = [](const RTX_BVHNode& node) { return node.aabb.diagonal(); };
        func aabbF = [](const RTX_BVHNode& node) { return node.aabb.min;};
        leaf leafF = [](const RTX_BVHNode& node) { return node.count > 0;};

        m_rtxbvhModel = BVHBuilder::generateDrawData(
                            globalContext,
                            m_bvh.nodes.data(),
                            m_bvh.count,
                            diagonalF,
                            aabbF,
                            leafF);
    }

    Resource*& bvhNodeBuffer = getResource(globalContext, RTXPass_BVHNode_Buffer);
    bvhNodeBuffer = new Buffer(globalContext.device,
                               globalContext.cmdList,
                               getCustomBufferDescription(m_bvh.nodes.data(),
                                                          m_bvh.count));

    Resource*& bvhAABBBuffer = getResource(globalContext, RTXPass_BVHAABB_Buffer);
    bvhAABBBuffer = new Buffer(globalContext.device,
                               globalContext.cmdList,
                               getCustomBufferDescription(m_bvh.aabbs.data(),
                                                          m_bvh.aabbs.size()));

    Resource*& bvhIndicesBuffer = getResource(globalContext, RTXPass_BVHIndices_Buffer);
    bvhIndicesBuffer = new Buffer(globalContext.device,
                               globalContext.cmdList,
                               getCustomBufferDescription(m_bvh.indices.data(),
                                                          m_bvh.indices.size()));

    struct BVHDescription
    {
        u32 bvhNodeBufferIndex;
        u32 bvhIndicesBufferIndex;
        u32 bvhNodeCount;
    }bvhDesc;

    bvhDesc.bvhNodeCount = m_bvh.count;
    bvhDesc.bvhNodeBufferIndex = bvhNodeBuffer->srv.getDescriptorIndex();
    bvhDesc.bvhIndicesBufferIndex = bvhIndicesBuffer->srv.getDescriptorIndex();

    Resource*& bvhDescBuffer = getResource(globalContext,
                                           RTXPass_BVHDescription_Buffer);
    bvhDescBuffer = new ConstantBuffer(globalContext.device,
                                     globalContext.cmdList,
                                     sizeof(BVHDescription));
    ((ConstantBuffer*)bvhDescBuffer)->update(&bvhDesc);

    RTXDescription rtxDesc;
    rtxDesc.sphereCount = m_currentSphereCount;
    rtxDesc.imWidth = getWidth();
    rtxDesc.imHeight = getHeight();
    rtxDesc.color = m_outputColor.getData();

    m_rtxData.description = rtxDesc;

    for(int i = 0; i < m_currentSphereCount; i++)
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
    if(m_drawRTXBVHDebugView.getData())
        m_renderPasses[BVHDebugPass].execute(globalContext, &m_rtxbvhModel);

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
    rtxDesc.sphereCount = m_currentSphereCount;
    rtxDesc.imWidth = getWidth();
    rtxDesc.imHeight = getHeight();
    rtxDesc.color = m_outputColor.getData();

    for(u32 i = 0; i < m_currentSphereCount; ++i)
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
