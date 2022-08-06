#include "framework/dx12/Device.h"
#include "include/App.h"
#include "framework/dx12/PSO.h"
#include "external/imgui/imgui.h"
#include "external/imgui/imgui_impl_dx12.h"
#include "Framework/dx12/RootSignature.h"
#include "Framework/dx12/PipelineStates.h"
#include "Framework/dx12/PSO.h"


using namespace DirectX;


App::App(HINSTANCE hInstance)
: D3DApp(hInstance) , mainPassImGui(m_mainPassCB)
{
}

App::~App()
{
    Destroy();
}

bool App::Initialize()
{
    if(!D3DApp::Initialize())
		return false;
		
    // Reset the command list to prep for initialization commands.
    ThrowIfFailed(mCommandList->Reset(mDirectCmdListAlloc.Get(), nullptr));
    InitCamera();
    BuildRootSignature();
    BuildDescriptorHeaps();
    BuildRenderItems();
    BuildFrameResources();
    BuildPSO();
    InitImgui();
	ThrowIfFailed(mCommandList->Close());
	ID3D12CommandList* cmdsLists[] = { mCommandList.Get() };
	mCommandQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);
	FlushCommandQueue();

	return true;
}

void App::InitCamera()
{
    m_camera.SetPosition(mEyePos);
    m_camera.SetWindowSize(mClientWidth, mClientHeight);
}

void App::BuildDescriptorHeaps()
{
    UINT objCount = m_renderItems.size();
    UINT numberOfPassDescriptors = 1;
    UINT numDescriptors = 20;// (objCount + numberOfPassDescriptors + numberOfTextures)* NumFrames;
    m_cbvHeap.Init(numDescriptors, DescriptorHeapType::CBV_SRV_UAV, D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE);
}

void App::BuildRootSignature()
{
    const int numberOfRootArguments = 4;
    RootArguments rootArguments(numberOfRootArguments);
    rootArguments[0] = RootArgument::CreateCBV(0);
    rootArguments[1] = RootArgument::CreateCBV(1);
    auto srvRange1 = DescriptorRange(DescriptorRangeType::SRV, 1, 0);
    rootArguments[2] = RootArgument::CreateTable(1, srvRange1,
        ShaderVisibility::PIXEL);

    auto srvRange2 = DescriptorRange(DescriptorRangeType::SRV, 1, 1);
    rootArguments[3] = RootArgument::CreateTable(1, srvRange2,
        ShaderVisibility::PIXEL);
    m_rootSignatures["default"] = RootSignature(rootArguments);
}

void App::BuildRenderItems()
{
    DescriptorHeapManager::SetSRVHeap(m_cbvHeap);
    UINT countOfMeshes = 1;

    if(0)
    {
		std::vector<Geometry::MeshData> mesh(countOfMeshes);
        mesh[0] = Geometry::CreateQuad();
        std::vector<TextureHandle> grassTexture(2);
        grassTexture[0] =
            TextureColection::CreateTexture("textures/grass.png", Device::GetDevice(), mCommandList, "grass");
        grassTexture[1] = grassTexture[0];
        std::vector<std::pair< std::vector<TextureHandle>, std::vector<Geometry::MeshData> >> val =
        { { grassTexture, mesh } };
        Mesh quad = Mesh(val, mCommandList, "quad");
		m_renderItems.push_back(RenderItem(quad, "quad", RenderItem::RenderItemState::TRANSPARENT_STATE));
    }
    Model model; 
    model.Init("D:/dev/projects/learning/directx-shit/textures/models/backpack/backpack.obj", mCommandList);
    RenderItem model_item(model, "model");
    m_renderItems.push_back(model_item);
    model_item.m_state = RenderItem::RenderItemState::REFLECTED;
    model_item.m_transformation.m_state = ObjectConstants::ObjectConstantsState::REFLECTED;
	model_item.m_objCbIndex = RenderItem::g_objectCBIndex++;
    m_renderItems.push_back(model_item);

    m_renderItems.reserve(3);
    for (auto& e : m_renderItems)
        if (e.m_state == RenderItem::RenderItemState::OPAQUE_STATE)
            m_opaqueItems.push_back(&e);
        else if (e.m_state == RenderItem::RenderItemState::REFLECTED)
			m_reflectedItems.push_back(&e);
        else
            m_transparentItems.push_back(&e);
   {
		std::vector<Geometry::MeshData> mesh(countOfMeshes);
        mesh[0] = Geometry::CreateMirror();
        RenderItem mirror(mesh, mCommandList, "mirrror");
		m_renderItems.push_back(mirror);
        m_mirror.push_back(&m_renderItems.back());
    }
}

void App::BuildPSO()
{
    std::wstring homeDir = L"D:/dev/projects/learning/directx-shit/";
    Shader::CreateShader({"colorVertex", "D:/dev/projects/learning/directx-shit/src/Shaders/color.hlsl", "VS", ShaderType::VERTEX});
    Shader::CreateShader({"colorPixel", "D:/dev/projects/learning/directx-shit/src/Shaders/color.hlsl", "PS", ShaderType::PIXEL});

    std::vector<InputLayoutElement> inputLayout =
    {
        InputLayoutElement("POSITION", Format::float3),
        InputLayoutElement("NORMAL",   Format::float3),
        InputLayoutElement("TEXTURE",  Format::float2),
    };
    InputLayout layout(inputLayout);
    ShaderInputGroup shaderBase{ "colorVertex", "colorPixel", &layout, &m_rootSignatures["default"] };

    PSO base;
    base.SetShaderInputGroup(shaderBase);
    
    m_pso["default"] = base.Compile();

    {
		StencilOpDesc op(StencilOP::KEEP, StencilOP::KEEP, StencilOP::REPLACE, ComparisonFunc::ALWAYS);
		DepthStencilState dsState(DepthState(true, DepthWriteMask::ZERO, ComparisonFunc::LESS), StencilState(true, 0xff, 0xff, op, op));
        base.SetDepthStencilState(dsState);
        base.SetBlendState(BlendState(WriteEnable::NONE));
        m_pso["stencilMirror"] = base.Compile();
    }

    {
		StencilOpDesc op(StencilOP::KEEP, StencilOP::KEEP, StencilOP::KEEP, ComparisonFunc::EQUAL);
		DepthStencilState dsState(DepthState(true, DepthWriteMask::ALL, ComparisonFunc::LESS), StencilState(true, 0xff, 0xff, op, op));
        base.SetDepthStencilState(dsState);
        base.SetBlendState();
        base.SetRasterizerState(RasterizerState(CullMode::BACK, true));
        m_pso["drawStencilReflections"] = base.Compile();
    }

    {
        ColorBlendEquation eq(BlendColorFactor::SRC_ALPHA, BlendColorFactor::INV_SRC_ALPHA, BlendOP::ADD);
        BlendState blend(eq, AlphaBlendEquation{});
        base.SetDepthStencilState();
        base.SetBlendState(blend);
        base.SetRasterizerState();
        m_pso["transparent"] = base.Compile();
    }
}

void App::BuildFrameResources()
{
    for (int i = 0; i < NumFrames; ++i)
    {
        m_frameResources.push_back(std::move(FrameResource(Device::GetDevice(), 2, m_renderItems.size())));
    }
}

void App::InitImgui()
{
    ImGuiSettings::Init(mhMainWnd, Device::GetDevice(), NumFrames);

}


void App::OnResize()
{
	D3DApp::OnResize();

    XMMATRIX P = XMMatrixPerspectiveFovLH(0.25f*MathHelper::Pi, AspectRatio(), 1.0f, 1000.0f);
    XMStoreFloat4x4(&mProj, P);
}

void App::UpdateObjectCB(const GameTimer& gt)
{
    for (auto& renderItem : m_renderItems)
    {
        if (renderItem.m_transformation.m_framesToUpdate)
        {
            XMMATRIX world = XMLoadFloat4x4(&renderItem.m_transformation.properties.World);
			ObjectConstants::ObjectProperties constants;
            XMStoreFloat4x4(&constants.World, XMMatrixTranspose(world));
            m_currentFrameResource->m_objectCb.CopyData(renderItem.m_objCbIndex, constants);
            renderItem.m_transformation.m_framesToUpdate--;
        }
    }
}

void App::UpdateMainPassCB(const GameTimer& gt)
{
    XMMATRIX view = m_camera.GetView();
    XMMATRIX proj = XMLoadFloat4x4(&mProj);

    XMMATRIX viewProj = XMMatrixMultiply(view, proj);
    auto viewDeterm = XMMatrixDeterminant(view);
    XMMATRIX invView = XMMatrixInverse(&viewDeterm, view);
    auto projDeterm = XMMatrixDeterminant(proj);
    XMMATRIX invProj = XMMatrixInverse(&projDeterm, proj);
    auto viewProjDeterm = XMMatrixDeterminant(viewProj);
    XMMATRIX invViewProj = XMMatrixInverse(&viewProjDeterm, viewProj);
    XMMATRIX shadowView{};
    {
		XMMATRIX shadowview = Camera::LookAt(m_mainPassCB.lightPos, m_mainPassCB.lightDir);
        shadowView = shadowview;
        shadowView = XMMatrixMultiply(shadowview, proj);
    }

    XMStoreFloat4x4(&m_mainPassCB.View,        XMMatrixTranspose(view));
    XMStoreFloat4x4(&m_mainPassCB.InvView,     XMMatrixTranspose(invView));
    XMStoreFloat4x4(&m_mainPassCB.Proj,        XMMatrixTranspose(proj));
    XMStoreFloat4x4(&m_mainPassCB.InvProj,     XMMatrixTranspose(invProj));
    XMStoreFloat4x4(&m_mainPassCB.ViewProj,    XMMatrixTranspose(viewProj));
    XMStoreFloat4x4(&m_mainPassCB.InvViewProj, XMMatrixTranspose(invViewProj));
    XMStoreFloat4x4(&m_mainPassCB.shadowView,  XMMatrixTranspose(shadowView));
    XMStoreFloat4  (&m_mainPassCB.EyePosW,     m_camera.GetPosition());

    m_currentFrameResource->m_passCb.CopyData(0, m_mainPassCB);

}

void App::Update(const GameTimer& gt)
{
    OnMouseMove();
    m_frameIndex = (m_frameIndex + 1) % NumFrames;
    m_currentFrameResource = &m_frameResources[m_frameIndex];

    // Convert Spherical to Cartesian coordinates.
    auto temp = mFence->GetCompletedValue();
    if (m_currentFrameResource->m_fence != 0 && mFence->GetCompletedValue() < m_currentFrameResource->m_fence)
    {
        HANDLE eventHandle = CreateEventEx(nullptr, nullptr, false, EVENT_ALL_ACCESS);
        ThrowIfFailed(mFence->SetEventOnCompletion(m_currentFrameResource->m_fence, eventHandle));
        WaitForSingleObject(eventHandle, INFINITE);
        CloseHandle(eventHandle);
    }
    UpdateObjectCB(gt);
    UpdateMainPassCB(gt);
}

void App::DrawImgui()
{
    for (auto& renderItem : m_renderItems)
    {
        renderItem.OnImGuiRender();
    }
    m_camera.OnImGui();
    mainPassImGui.OnImGui();
}

void App::DrawRenderItems(ID3D12GraphicsCommandList* cmdList, const std::vector<RenderItem*>& ritems)
{
    UINT objCBByteSize = d3dUtil::CalcConstantBufferByteSize(sizeof(ObjectConstants));

    ID3D12Resource* objectCB = m_currentFrameResource->m_objectCb.Resource();

    for (size_t i = 0; i < ritems.size(); ++i)
    {
        auto ri = ritems[i];
        auto cbvHandle = m_cbvHeap.GetGPUHandle(m_frameResources[m_frameIndex].objectIndexes[i]);
        auto cbvHandle1 = m_frameResources[m_frameIndex].m_objectCb.Resource()->GetGPUVirtualAddress() +
            ri->m_objCbIndex * m_frameResources[m_frameIndex].m_objectCb.mElementByteSize;
        cmdList->SetGraphicsRootConstantBufferView(0, cbvHandle1);
        // todo: get rid of this useless true 
        ri->Draw(cmdList, true);
    }
}

void App::Destroy()
{
	HANDLE eventHandle = CreateEventEx(nullptr, nullptr, false, EVENT_ALL_ACCESS);
	ThrowIfFailed(mFence->SetEventOnCompletion(m_currentFrameResource->m_fence, eventHandle));
	WaitForSingleObject(eventHandle, INFINITE);
	CloseHandle(eventHandle);
}

void App::Draw(const GameTimer& gt)
{

    ComPtr<ID3D12CommandAllocator> currentCmdAlloc = m_currentFrameResource->m_cmdAlloc;
	ThrowIfFailed(currentCmdAlloc->Reset());
	ID3D12DescriptorHeap* descriptorHeaps[] = { m_cbvHeap.GetHeap()};
    ThrowIfFailed(mCommandList->Reset(currentCmdAlloc.Get(), nullptr));


    if (true)
    {
        // main pass 
        auto resourceTransition = CD3DX12_RESOURCE_BARRIER::Transition(CurrentBackBuffer(),
            D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
		mCommandList->ResourceBarrier(1, &resourceTransition);
		mCommandList->SetPipelineState(m_pso["default"]);

		mCommandList->RSSetViewports(1, &mScreenViewport);
		mCommandList->RSSetScissorRects(1, &mScissorRect);
		mCommandList->ClearDepthStencilView(DepthStencilView(), D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);
		mCommandList->ClearRenderTargetView(CurrentBackBufferView(), clear_color, 0, nullptr);
        auto backBuffer = CurrentBackBufferView();
        auto dsvBuffer = DepthStencilView();
		mCommandList->OMSetRenderTargets(1, &backBuffer, true, &dsvBuffer);
		mCommandList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);
		mCommandList->SetGraphicsRootSignature(m_rootSignatures["default"]);

		auto passCbvAddress = m_frameResources[m_frameIndex].m_passCb.Resource()->GetGPUVirtualAddress();
		mCommandList->SetGraphicsRootConstantBufferView(1, passCbvAddress);

		UINT objCBByteSize = d3dUtil::CalcConstantBufferByteSize(sizeof(ObjectConstants));

        mCommandList->OMSetStencilRef(0);

		DrawRenderItems(mCommandList.Get(), m_opaqueItems);

        mCommandList->SetPipelineState(m_pso["stencilMirror"]);
        mCommandList->OMSetStencilRef(1);
		DrawRenderItems(mCommandList.Get(), m_mirror);

        mCommandList->SetPipelineState(m_pso["drawStencilReflections"]);
        DrawRenderItems(mCommandList.Get(), m_reflectedItems);

        // transparent items

		mCommandList->SetPipelineState(m_pso["transparent"]);
		DrawRenderItems(mCommandList.Get(), m_transparentItems);
        //imgui shit 		
		ImGuiSettings::StartFrame();
		DrawImgui();
		ImGuiSettings::EndFrame();

		auto lv = ImGuiSettings::GetDescriptorHeap();
		mCommandList->SetDescriptorHeaps(1, &lv);
		ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), mCommandList.Get());

        resourceTransition = CD3DX12_RESOURCE_BARRIER::Transition(CurrentBackBuffer(),
            D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
		mCommandList->ResourceBarrier(1, &resourceTransition);


    }
  	ThrowIfFailed(mCommandList->Close());
 
	ID3D12CommandList* cmdsLists[] = { mCommandList.Get() };
	mCommandQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);
	
	ThrowIfFailed(mSwapChain->Present(0, 0));
	mCurrBackBuffer = (mCurrBackBuffer + 1) % SwapChainBufferCount;
    m_currentFrameResource->m_fence = ++mCurrentFence;

    ThrowIfFailed(mCommandQueue->Signal(mFence.Get(), mCurrentFence));
}

void App::OnMouseDown(WPARAM btnState, int x, int y)
{
    SetCapture(mhMainWnd);
}

void App::OnMouseUp(WPARAM btnState, int x, int y)
{
    ReleaseCapture();
}

void App::OnMouseMove()
{
    ShowCursor(!m_camera.IsCameraOn());
    POINT p{};
    GetCursorPos(&p);
    ScreenToClient(mhMainWnd, &p);
	m_camera.OnMouseMove(p.x, p.y);
}

void App::OnKeyDown(Key key)
{
    m_camera.OnKeyDown(key);
}
