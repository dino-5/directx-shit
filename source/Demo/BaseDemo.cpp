#include "BaseDemo.h"
#include "EngineCommon/util/Logger.h"
#include "EngineCommon/util/CommandLine.h"
#include "EngineCommon/util/ImGuiSettings.h"
#include "third_party/imgui/imgui.h"
#include "third_party/imgui/backends/imgui_impl_dx12.h"

using namespace std;
using namespace gfx;
using namespace util;
using namespace DirectX;

BaseDemo::BaseDemo(u32 width, u32 height, std::string name) :
	WindowApp(width, height, name),
	m_cmdList(m_device),
	m_cmdQueue(m_device.native()),
	m_swapChain(getCurrentWindowSettings(), m_device.getFactory(), m_cmdQueue.getQueue())
{
	m_inputManager = &system::InputManager::GetInputManager();
	m_device.createFence(&m_fence);

	DescriptorHeapManager::CreateDSVHeap(10);
	DescriptorHeapManager::CreateRTVHeap(20);
	DescriptorHeapManager::CreateSRVHeap(200);
	
	m_swapChain.onResize();
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
        val.DepthStencil = { 1.0f ,0 };
        val.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
        m_depthStencil.initResource(m_device.getDevice(), desc, viewProps, &val);
    }
    D3D12_CLEAR_VALUE val;
    val.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
    val.Color[0] = 1.f;
    val.Color[1] = 0.f;
    val.Color[2] = 1.f;
    val.Color[3] = 0.f;

    for(int i = 0; i < config::NumFrames; ++i)
    {
        DescriptorProperties viewProps{
            .descriptor = DescriptorFlags::RenderTarget |DescriptorFlags::ShaderResource ,
            .viewDimension = D3D12_SRV_DIMENSION_TEXTURE2D,
        };

        ResourceDescription descPosition{
                .format = DXGI_FORMAT_R32G32B32A32_FLOAT,
                .width= width,
                .height = height,
                .depthOrArraySize = 1,
                .dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D,
                .flags = ResourceFlags::RENDER_TARGET,
                .createState = ResourceState::RENDER_TARGET ,
                .heapType = D3D12_HEAP_TYPE_DEFAULT,
                .name = "deferred position"
       };
        m_positionRT[i].initResource(m_device.getDevice(), descPosition, viewProps, &val);

        ResourceDescription descAlbedo{
                .format = DXGI_FORMAT_R32G32B32A32_FLOAT,
                .width= width,
                .height = height,
                .depthOrArraySize = 1,
                .dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D,
                .flags = ResourceFlags::RENDER_TARGET,
                .createState = ResourceState::RENDER_TARGET ,
                .heapType = D3D12_HEAP_TYPE_DEFAULT,
                .name = "deferred albedo"
       };
        m_albedoRT[i].initResource(m_device.getDevice(), descAlbedo, viewProps, &val);

        ResourceDescription descNormal{
                .format = DXGI_FORMAT_R32G32B32A32_FLOAT,
                .width= width,
                .height = height,
                .depthOrArraySize = 1,
                .dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D,
                .flags = ResourceFlags::RENDER_TARGET,
                .createState = ResourceState::RENDER_TARGET ,
                .heapType = D3D12_HEAP_TYPE_DEFAULT,
                .name = "deferred normal"
       };
        m_normalRT[i].initResource(m_device.getDevice(), descNormal, viewProps, &val);
    }

    m_fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
    if (m_fenceEvent == nullptr)
    {
        ThrowIfFailed(HRESULT_FROM_WIN32(GetLastError()));
    }

    math::ProjectionProps props;
    props.type = math::ProjectionType::Perspective;
    props.perspective.fov = 90;
    props.perspective.aspectRatio = m_swapChain.getAspectRatio();
    props.perspective.nearZ = 0.1f;
    props.perspective.farZ = 10000.f;
    m_camera.initialize({ 0, 100, 0 }, { 0.f, 0.f, 1.f }, props);
}

SwapChainSettings BaseDemo::getCurrentWindowSettings()
{
	return { getWidth(), getHeight(), DXGI_FORMAT_R8G8B8A8_UNORM, getWindowHandle()};
}

void BaseDemo::compileShaders()
{
    // shaders
    m_shaders.clear();
    auto createShader = [this](TableEntry<DxBlob*>& entry)
    {
        if (entry.second != nullptr)
            this->m_shaders.push_back(entry);
    };
    {
        ShaderInfo info(createShader);
        info.entryPoint = L"VS_Basic";
        info.path = L"Shaders/deferred_geometry.hlsl";
        info.shaderName = L"VS_Color";
        info.type = ShaderType::VERTEX;
    }
    {
        ShaderInfo info(createShader);
        info.entryPoint = L"PS_Basic";
        info.path = L"Shaders/deferred_geometry.hlsl";
        info.shaderName = L"PS_Color";
        info.type = ShaderType::PIXEL;
    }
    {
        ShaderInfo info(createShader);
        info.entryPoint = L"VSMain";
        info.path = L"Shaders/deferred_lighting.hlsl";
        info.shaderName = L"VS_Deferred";
        info.type = ShaderType::VERTEX;
    }
    {
        ShaderInfo info(createShader);
        info.entryPoint = L"PSMain";
        info.path = L"Shaders/deferred_lighting.hlsl";
        info.shaderName = L"PS_Deferred";
        info.type = ShaderType::PIXEL;
    }
    {
        ShaderInfo info(createShader);
        info.entryPoint = L"VSMain";
        info.path = L"Shaders/visualizeLight.hlsl";
        info.shaderName = L"VS_LightBox";
        info.type = ShaderType::VERTEX;
    }
    {
        ShaderInfo info(createShader);
        info.entryPoint = L"PSMain";
        info.path = L"Shaders/visualizeLight.hlsl";
        info.shaderName = L"PS_LightBox";
        info.type = ShaderType::PIXEL;
    }

    // TODO rename and structure pass elements
    // deferred geometry pass
    {
        std::vector<D3D12_INPUT_ELEMENT_DESC> desc = {
            {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
            {"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
            {"TANGENT", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 24, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
            {"UV", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 40, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
        };
        ShaderInputGroup shaderIG;
        shaderIG.desc = { desc.data(), static_cast<u32>(desc.size()) };
        shaderIG.vertexShader = getShader(*util::FindElement(m_shaders, L"VS_Color"));
        shaderIG.pixelShader = getShader(*util::FindElement(m_shaders, L"PS_Color"));
        shaderIG.rootSignature = &m_rootSignature;

        RenderState state;
        state.m_shader = shaderIG;
        state.renderTargets = { DXGI_FORMAT_R32G32B32A32_FLOAT, DXGI_FORMAT_R32G32B32A32_FLOAT,DXGI_FORMAT_R32G32B32A32_FLOAT };
        m_pso = PSO::CreatePSO(state);
    }

    // deferred lighting pass
    {
        ShaderInputGroup shaderIG;
        shaderIG.vertexShader = getShader(*util::FindElement(m_shaders, L"VS_Deferred"));
        shaderIG.pixelShader = getShader(*util::FindElement(m_shaders, L"PS_Deferred"));
        shaderIG.rootSignature = &m_lightingRS;

        RenderState state;
        DepthStencilState depth;
        depth.m_desc.DepthEnable = FALSE;
        depth.m_desc.StencilEnable = FALSE;

        state.setRasterizerState(RasterizerState(CullMode::FRONT, true));
        state.setDepthStencilState(depth);
        state.m_shader = shaderIG;
        m_lightingPSO = PSO::CreatePSO(state);
    }

    // light box
    {
        std::vector<D3D12_INPUT_ELEMENT_DESC> desc = {
            {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
        };
        ShaderInputGroup shaderIG;
        shaderIG.desc = { desc.data(), static_cast<u32>(desc.size()) };
        shaderIG.vertexShader = getShader(*util::FindElement(m_shaders, L"VS_LightBox"));
        shaderIG.pixelShader = getShader(*util::FindElement(m_shaders, L"PS_LightBox"));
        shaderIG.rootSignature = &m_lightBoxRS;

        RenderState state;
        state.setRasterizerState(RasterizerState(CullMode::NONE, true));
        state.m_shader = shaderIG;
        m_lightBoxPSO = PSO::CreatePSO(state);
    }

}

bool BaseDemo::initialize()
{
	LogScope("BaseDemo");

    ImGuiSettings::Init(getWindowHandle(), m_device.getDevice(), config::NumFrames);

    //root signature
    {
        // deferred geometry
        RootParameters parameters = { RootParameter::CreateConstants(2, 0, 10),
                                      RootParameter::CreateConstants(1, 1, 10) };
        auto rootSignFlags = RootSignatureFlags::ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT | RootSignatureFlags::SBV_SRV_HEAP_DIRECT_INDEX;
        m_rootSignature.init(m_device.getDevice(), parameters, rootSignFlags);
    }
    {
        // deferred lighting
        RootParameters parameters = { RootParameter::CreateConstants(4, 0, 10),
            RootParameter::CreateDescriptor(1, 10)};
        auto rootSignFlags = RootSignatureFlags::ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT | RootSignatureFlags::SBV_SRV_HEAP_DIRECT_INDEX;
        m_lightingRS.init(m_device.getDevice(), parameters, rootSignFlags);
    }
    {
        // visualize light 
        RootParameters parameters = { RootParameter::CreateDescriptor(0, 0),
            RootParameter::CreateDescriptor(0, 0, RootParameterType::SRV, ShaderVisibility::ALL) };
        auto rootSignFlags = RootSignatureFlags::ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT ;
        m_lightBoxRS.init(m_device.getDevice(), parameters, rootSignFlags);
    }

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

    m_lightBuffer.data.push_back(Light{ math::Vector3{1.f, 1.f, 1.f} });
    m_lightBuffer.desc.state = ResourceState::GENERIC_READ_STATE;
    m_lightBuffer.desc.type = BufferType::CUSTOM;
    m_lightBuffer.create(context);

    m_lightCube.initDSH(config::g_state.homeDir/ "textures/models/cube.dsh", context);
    m_model.initGLTF(config::g_state.homeDir/ "textures/models/Sponza/gltf/Sponza.gltf", context);

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
    ID3D12CommandList* ppCommandLists[] = { context.cmdList};
    m_cmdQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

    u64 fence = m_fence->GetCompletedValue();
    m_swapChain.m_fence[0] = ++m_fenceValue;
    m_cmdQueue->Signal(m_fence, m_fenceValue);
    m_fence->SetEventOnCompletion(m_fenceValue, m_fenceEvent);
    WaitForSingleObjectEx(m_fenceEvent, INFINITE, FALSE);
    fence = m_fence->GetCompletedValue();

	return true;
}

void BaseDemo::draw()
{
    ID3D12GraphicsCommandList* cmdList = m_cmdList.reset(m_currentFrameIndex);
    auto& positionRT = m_positionRT[m_currentFrameIndex];
    auto& albedoRT = m_albedoRT[m_currentFrameIndex];
    auto& normalRT = m_normalRT[m_currentFrameIndex];

    // deferred
    {
        positionRT.transition(cmdList, ResourceState::RENDER_TARGET);
        albedoRT.transition(cmdList, ResourceState::RENDER_TARGET);
        normalRT.transition(cmdList, ResourceState::RENDER_TARGET);

        cmdList->RSSetViewports(1, &m_viewPort);
        cmdList->RSSetScissorRects(1, &m_scissorRect);

        const float clearColor[] = { 1.0f, .0f, 1.0f, .0f };
        cmdList->ClearRenderTargetView(positionRT.rtv.HandleCPU, clearColor, 0, nullptr);
        cmdList->ClearRenderTargetView(albedoRT.rtv.HandleCPU, clearColor, 0, nullptr);
        cmdList->ClearRenderTargetView(normalRT.rtv.HandleCPU, clearColor, 0, nullptr);

        constexpr const u32 rtHandlesNumber = 3;
        D3D12_CPU_DESCRIPTOR_HANDLE rtHandles[rtHandlesNumber] = {
            positionRT.rtv.HandleCPU, 
            albedoRT.rtv.HandleCPU, 
            normalRT.rtv.HandleCPU, 
        };
        cmdList->ClearDepthStencilView(m_depthStencil.dsv.HandleCPU, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);
        cmdList->OMSetRenderTargets(rtHandlesNumber, rtHandles, true, &m_depthStencil.dsv.HandleCPU);

        cmdList->SetDescriptorHeaps(1, engine::graphics::DescriptorHeapManager::CurrentSRVHeap.getHeapAddress());
        cmdList->SetGraphicsRootSignature(m_rootSignature);
        cmdList->SetPipelineState(m_pso);

        cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

        BindlessTable table{ m_constBuffer.getDescriptorHeapIndex(),
            m_model.m_materialBuffer.getDescriptorHeapIndex()};
        cmdList->SetGraphicsRoot32BitConstants(0, 2, &table, 0);

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

    // deferred lighting
    {
        auto renderTarget = m_swapChain.getView(m_swapChain.changeState(cmdList, ResourceState::RENDER_TARGET));
        positionRT.transition(cmdList, ResourceState::PIXEL_SHADER_RESOURCE);
        albedoRT.transition(cmdList, ResourceState::PIXEL_SHADER_RESOURCE);
        normalRT.transition(cmdList, ResourceState::PIXEL_SHADER_RESOURCE);

        const float clearColor[] = { .0f, .0f, .0f, .0f };
        cmdList->ClearRenderTargetView(renderTarget.HandleCPU, clearColor, 0, nullptr);
        cmdList->OMSetRenderTargets(1, &renderTarget.HandleCPU, true, nullptr);
        struct 
        {
            int positionIndex;
            int albedoIndex;
            int normalIndex;
            int lightBufferIndex;
        }deferredTable;
        deferredTable.positionIndex = positionRT.srv.getDescriptorIndex();
        deferredTable.albedoIndex = albedoRT.srv.getDescriptorIndex();
        deferredTable.normalIndex = normalRT.srv.getDescriptorIndex();
        deferredTable.lightBufferIndex = m_lightBuffer.buffer.getDescriptorHeapIndex();

        cmdList->SetGraphicsRootSignature(m_lightingRS);
        cmdList->SetPipelineState(m_lightingPSO);
        cmdList->IASetVertexBuffers(0, 1, nullptr);
        cmdList->IASetIndexBuffer(nullptr);
        cmdList->SetGraphicsRoot32BitConstants(0, 4, &deferredTable, 0);
        cmdList->SetGraphicsRootConstantBufferView(1, m_lightSettingsResource.getGPUAdress());
        cmdList->DrawInstanced(6, 1, 0, 0);

        //draw light cube
        cmdList->SetPipelineState(m_lightBoxPSO);
        cmdList->SetGraphicsRootSignature(m_lightBoxRS);
        cmdList->OMSetRenderTargets(1, &renderTarget.HandleCPU, true, &m_depthStencil.dsv.HandleCPU);

        cmdList->SetGraphicsRootConstantBufferView(0, m_constBuffer.getAddress());
        cmdList->SetGraphicsRootShaderResourceView(1, m_lightBuffer.buffer.getGPUAdress());

        auto vertexBuffer = GetVertexBufferView(m_lightCube.m_mesh.m_vertexBuffer);
        auto indexBuffer = GetIndexBufferView(m_lightCube.m_mesh.m_indexBuffer);
        cmdList->IASetVertexBuffers(0, 1, &vertexBuffer);
        cmdList->IASetIndexBuffer(&indexBuffer);

        for (auto& submesh : m_lightCube.m_submeshes)
        {
            submesh.draw(cmdList);
        }
    }

    ImGuiSettings::StartFrame();
    {
        static math::Vector3 lightPos;
        ImGuiSettings::Begin("Settings");
        if (m_lightBuffer.data[0].position.onImGui("light position", -1000, 1000))
        {
            GfxContext context;
            context.cmdList = cmdList;
            context.device = m_device.getDevice();
            m_lightBuffer.update(context);
        }
        ImGuiSettings::End();
    }
    ImGuiSettings::EndFrame(cmdList);

    m_swapChain.changeState(cmdList, ResourceState::PRESENT);
    ThrowIfFailed(cmdList->Close());
    ID3D12CommandList* ppCommandLists[] = { cmdList};
    m_cmdQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);
    m_swapChain->Present(1, 0);

    // sync
    m_cmdQueue->Signal(m_fence, ++m_fenceValue);
    m_swapChain.m_fence[m_currentFrameIndex] = m_fenceValue;

}

void BaseDemo::waitForFrame(u32 index)
{
    if (m_fence->GetCompletedValue() < m_swapChain.m_fence[index])
    {
        m_fence->SetEventOnCompletion(m_swapChain.m_fence[index], m_fenceEvent);
        WaitForSingleObjectEx(m_fenceEvent, INFINITE, FALSE);
    }
}

void BaseDemo::update()
{
    m_currentFrameIndex = (m_currentFrameIndex + 1) % config::NumFrames;
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
