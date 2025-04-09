#include "BaseDemo.h"
#include "EngineCommon/util/Logger.h"
#include "EngineCommon/util/CommandLine.h"
#include "EngineCommon/util/ImGuiSettings.h"
#include "EngineCommon/util/Timer.h"
#include "third_party/imgui/imgui.h"
#include "third_party/imgui/backends/imgui_impl_dx12.h"
#include <dxgiformat.h>
#include <ostream>
#include <string_view>

using namespace std;
using namespace gfx;
using namespace util;
using namespace DirectX;

Light::Light(math::Vector3 vec, std::string name, float range, uiActionCallback callback)
    : m_position(vec),
      m_name(name),
      m_positionRange(range),
      m_uiElement(3, m_position.data(), m_name, m_positionRange)
{
    m_uiElement.m_callback = callback;
}

void Light::defaultCallback(BaseDemo& demo)
{
    //demo.getLightBuffer().update(demo.m_graphicsContext);
}

BaseDemo::BaseDemo(u32 width, u32 height, std::string_view name) :
    WindowApp(width, height, name),
    m_cmdList(m_device),
    m_cmdQueue(m_device.native()),
    m_swapChain(getCurrentWindowSettings(), m_device, m_cmdQueue.getQueue())
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
    WindowApp::onResize(width, height);
    m_swapChain.onResize(getCurrentWindowSettings());

    m_viewPort.TopLeftX = 0;
    m_viewPort.TopLeftY = 0;
    m_viewPort.Width = width;
    m_viewPort.Height = height;
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

    RootParameters parameters = { RootParameter::CreateConstants(2, 0, 10),
                                  RootParameter::CreateConstants(1, 1, 10) };

    auto rootSignFlags = RootSignatureFlags::ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT | RootSignatureFlags::SBV_SRV_HEAP_DIRECT_INDEX;
    m_rootSignature.init(m_device.getDevice(), parameters, rootSignFlags);

    {
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
        m_pso = PSO::CreatePSO(renderState);
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
    m_constBuffer.init(context, &data, 1);

    LightSettings lightSettings;
    lightSettings.cameraPosition = m_camera.getPos();
    lightSettings.viewDirection = m_camera.getDir();
    m_lightSettingsResource.init(context, &lightSettings, 1);

    m_model.initGLTF(config::g_state.homeDir/ "data/Sponza/gltf/Sponza.gltf", context);

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

        auto vertexBuffer = GetVertexBufferView(m_model.m_mesh.m_vertexBuffer);
        auto indexBuffer = GetIndexBufferView(m_model.m_mesh.m_indexBuffer);
        cmdList->IASetVertexBuffers(0, 1, &vertexBuffer);
        cmdList->IASetIndexBuffer(&indexBuffer);

        for (auto& submesh : m_model.m_submeshes)
        {
            cmdList->SetGraphicsRoot32BitConstant(1, submesh.materialIndex, 0);
            submesh.draw(cmdList);
        }
    }

    imgui::StartFrame();
    {
        imgui::Begin("Settings");

        // light UI
        auto& uiElements = UI_Element::s_uiElements;
        for(UI_Element* uiElement: uiElements)
            uiElement->onUIAction(*this);

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
