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
	DescriptorHeapManager::CreateRTVHeap(10);
	DescriptorHeapManager::CreateSRVHeap(200);
	
	m_swapChain.onResize();
    m_viewPort.TopLeftX = 0;
    m_viewPort.TopLeftY = 0;
    m_viewPort.Width = width;
    m_viewPort.Height = height;
    m_viewPort.MaxDepth = 1.0;
    m_viewPort.MinDepth = .0;

    m_scissorRect = { 0, 0, static_cast<long>(width), static_cast<long>(height) };

	DescriptorProperties viewProps{
		.descriptor = DescriptorFlags::DepthStencil,
		.viewDimension = D3D12_SRV_DIMENSION_TEXTURE2D,
		.bufferStride = 0,
		.numElements = 0
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

    m_fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
    if (m_fenceEvent == nullptr)
    {
        ThrowIfFailed(HRESULT_FROM_WIN32(GetLastError()));
    }
}

SwapChainSettings BaseDemo::getCurrentWindowSettings()
{
	return { getWidth(), getHeight(), DXGI_FORMAT_R8G8B8A8_UNORM, getWindowHandle()};
}

bool BaseDemo::initialize()
{
	LogScope("BaseDemo");

    ImGuiSettings::Init(getWindowHandle(), m_device.getDevice(), config::NumFrames);
    //root signature
    RootParameters parameters = { RootParameter::CreateDescriptor(0, 10), RootParameter::CreateConstants(1, 1, 10) };
    auto rootSignFlags = RootSignatureFlags::ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT | RootSignatureFlags::SBV_SRV_HEAP_DIRECT_INDEX;
    m_rootSignature.init(m_device.getDevice(), parameters, rootSignFlags);

    // shaders
    ShaderManager::InitializeCompiler();
    auto createShader = [this](TableEntry<DxBlob*>& entry)
    {
        if (entry.second != nullptr)
            this->m_shaders.push_back(entry);
    };
    {
        ShaderInfo info(createShader);
        info.entryPoint = L"VS_Basic";
        info.path = L"Shaders/basic_shader.hlsl";
        info.shaderName = L"VS_Basic";
        info.type = ShaderType::VERTEX;
    }
    {
        ShaderInfo info(createShader);
        info.entryPoint = L"PS_Basic";
        info.path = L"Shaders/basic_shader.hlsl";
        info.shaderName = L"PS_Basic";
        info.type = ShaderType::PIXEL;
    }

    std::vector<D3D12_INPUT_ELEMENT_DESC> desc = {
        {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
        {"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
        {"UV", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
    };
    ShaderInputGroup shaderIG;
    shaderIG.desc = { desc.data(), static_cast<u32>(desc.size()) };
    shaderIG.vertexShader = getShader(*util::FindElement(m_shaders, L"VS_Basic"));
    shaderIG.pixelShader = getShader(*util::FindElement(m_shaders, L"PS_Basic"));
    shaderIG.rootSignature = &m_rootSignature;

    RenderState state;
    state.m_shader = shaderIG;

    m_pso = PSO::CreatePSO(state);
    GfxContext context;
    context.cmdList = m_cmdList.reset(0);
    context.device = m_device.getDevice();

    // setup data 
    ConstandBufferData data;
    data.perspective = math::PerspectiveProjection(90, m_swapChain.getAspectRatio(), .1f, 10000.f);
    data.view = m_camera.getViewMatrix();
    m_constBuffer.init(context, &data, 1);

    m_model.init(config::g_state.homeDir/ "textures/models/Sponza/gltf/Sponza.gltf", context);

    BindlessTable table{ m_constBuffer.getDescriptorHeapIndex(), m_model.m_materialBuffer.getDescriptorHeapIndex() };
    m_bindlessTable.init(context, &table, 1);

    m_camera.addChangeCallback([this](const Camera* camera)
    {
        ConstandBufferData data;
        data.perspective = math::PerspectiveProjection(90, m_swapChain.getAspectRatio(), .1f, 10000.f);
        data.view = camera->getViewMatrix();
        this->m_constBuffer.update(&data);
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
    cmdList->RSSetViewports(1, &m_viewPort);
    cmdList->RSSetScissorRects(1, &m_scissorRect);
    auto renderTarget = m_swapChain.getView(m_swapChain.changeState(cmdList, ResourceState::RENDER_TARGET));

    const float clearColor[] = { 0.0f, 0.2f, 0.4f, 1.0f };
    cmdList->ClearRenderTargetView(renderTarget.HandleCPU, clearColor, 0, nullptr);
    cmdList->ClearDepthStencilView(m_depthStencil.dsv.HandleCPU, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);
    cmdList->OMSetRenderTargets(1, &renderTarget.HandleCPU, true, &m_depthStencil.dsv.HandleCPU);

    cmdList->SetDescriptorHeaps(1, engine::graphics::DescriptorHeapManager::CurrentSRVHeap.getHeapAddress());
	cmdList->SetGraphicsRootSignature( m_rootSignature );
	cmdList->SetPipelineState( m_pso );

    cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    cmdList->SetGraphicsRootConstantBufferView(0, m_bindlessTable.getAddress());

    auto vertexBuffer = GetVertexBufferView(m_model.m_mesh.m_vertexBuffer);
    auto indexBuffer = GetIndexBufferView(m_model.m_mesh.m_indexBuffer);
    cmdList->IASetVertexBuffers(0, 1, &vertexBuffer);
    cmdList->IASetIndexBuffer(&indexBuffer);

    for (auto& submesh : m_model.m_submeshes)
    {
        cmdList->SetGraphicsRoot32BitConstant(1, submesh.materialIndex, 0);
        submesh.draw(cmdList);
    }

    ImGuiSettings::StartFrame();
    {
        static float f = 0.0f;
        ImGuiSettings::Begin("Hello, world!");
        ImGuiSettings::SliderFloat("float", &f, 0.0f, 1.0f); 
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
}
void BaseDemo::destroy()
{
    for (int i = 0; i < config::NumFrames; ++i)
        waitForFrame(i);
    m_model.reset();
    m_buffer.reset();
    m_constBuffer.reset();
    m_bindlessTable.reset();
}

LRESULT BaseDemo::processInput(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    return system::InputManager::GetInputManager().processInput(hwnd, msg, wParam, lParam);
}
