#include "dx12/Device.h"
#include "include/App.h"

using namespace DirectX;


App::App(HINSTANCE hInstance)
: D3DApp(hInstance) 
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
    BuildShadersAndInputLayout();
    BuildDescriptorHeaps();
    BuildRenderItems();
    BuildFrameResources();
	BuildConstantBuffers();
    BuildPSO();
    InitImgui();
    CreateTextures();
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
    UINT numberOfTextures = 2;
    UINT numberOfPassDescriptors = 1;
    UINT numDescriptors = 20;// (objCount + numberOfPassDescriptors + numberOfTextures)* NumFrames;
    m_passCbvOffset = objCount * NumFrames;
    m_textureCbvOffset = m_passCbvOffset + numberOfPassDescriptors * NumFrames;
    m_cbvHeap.Init(numDescriptors, DescriptorHeapType::CBV_SRV_UAV, D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE);
}

void App::BuildConstantBuffers()
{
    UINT objCBByteSize = d3dUtil::CalcConstantBufferByteSize(sizeof(ObjectConstants));

    UINT objCount = (UINT)m_opaqueItems.size();

    for (int frameIndex = 0; frameIndex < NumFrames; ++frameIndex)
    {
        auto objectCB = m_frameResources[frameIndex].m_objectCb.Resource();
		D3D12_GPU_VIRTUAL_ADDRESS cbAddress = objectCB->GetGPUVirtualAddress();
        for (UINT i = 0; i < objCount; ++i)
        {
            m_frameResources[frameIndex].objectIndexes[i] = m_cbvHeap.CreateCBV(cbAddress, objCBByteSize);
            cbAddress += objCBByteSize;
        }
    }

    UINT passCBByteSize = d3dUtil::CalcConstantBufferByteSize(sizeof(PassConstants));

    for (int frameIndex = 0; frameIndex < NumFrames; ++frameIndex)
    {
        auto passCB = m_frameResources[frameIndex].m_passCb.Resource();
        D3D12_GPU_VIRTUAL_ADDRESS cbAddress = passCB->GetGPUVirtualAddress();
        m_frameResources[frameIndex].passIndex = m_cbvHeap.CreateCBV(cbAddress, passCBByteSize);
    }
}

void App::BuildRootSignature()
{
    const int numberOfRootArguments = 4;
	CD3DX12_ROOT_PARAMETER slotRootParameter[numberOfRootArguments];

	CD3DX12_DESCRIPTOR_RANGE cbvTable0;
	cbvTable0.Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0);

	CD3DX12_DESCRIPTOR_RANGE cbvTable1;
	cbvTable1.Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 1);

	CD3DX12_DESCRIPTOR_RANGE cbvTable2;
    cbvTable2.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);

	CD3DX12_DESCRIPTOR_RANGE cbvTable3;
    cbvTable3.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 1);


	slotRootParameter[0].InitAsDescriptorTable(1, &cbvTable0);
	slotRootParameter[1].InitAsDescriptorTable(1, &cbvTable1);
	slotRootParameter[2].InitAsDescriptorTable(1, &cbvTable2, D3D12_SHADER_VISIBILITY_PIXEL);
	slotRootParameter[3].InitAsDescriptorTable(1, &cbvTable3, D3D12_SHADER_VISIBILITY_PIXEL);

    D3D12_STATIC_SAMPLER_DESC sampler = {};
    sampler.Filter = D3D12_FILTER_MIN_MAG_MIP_POINT;
    sampler.AddressU = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
    sampler.AddressV = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
    sampler.AddressW = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
    sampler.MipLODBias = 0;
    sampler.MaxAnisotropy = 0;
    sampler.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
    sampler.BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
    sampler.MinLOD = 0.0f;
    sampler.MaxLOD = D3D12_FLOAT32_MAX;
    sampler.ShaderRegister = 0;
    sampler.RegisterSpace = 0;
    sampler.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc(numberOfRootArguments, slotRootParameter, 1, &sampler, 
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

	ComPtr<ID3DBlob> serializedRootSig = nullptr;
	ComPtr<ID3DBlob> errorBlob = nullptr;
	HRESULT hr = D3D12SerializeRootSignature(&rootSigDesc, D3D_ROOT_SIGNATURE_VERSION_1,
		serializedRootSig.GetAddressOf(), errorBlob.GetAddressOf());

	if(errorBlob != nullptr)
	{
		::OutputDebugStringA((char*)errorBlob->GetBufferPointer());
	}
	ThrowIfFailed(hr);

	ThrowIfFailed(Device::GetDevice()->CreateRootSignature(
		0,
		serializedRootSig->GetBufferPointer(),
		serializedRootSig->GetBufferSize(),
		IID_PPV_ARGS(&mRootSignature)));
}

void App::BuildShadersAndInputLayout()
{
    HRESULT hr = S_OK;
    
    m_shaders["standardVS"] = d3dUtil::CompileShader(L"Shaders\\color.hlsl", nullptr, "VS", "vs_5_1");
    m_shaders["opaquePS"] = d3dUtil::CompileShader(L"Shaders\\color.hlsl", nullptr, "PS", "ps_5_1");

    mInputLayout =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,  D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "TEXTURE",  0, DXGI_FORMAT_R32G32_FLOAT,    0, 24, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
    };
}

void App::BuildRenderItems()
{
    DescriptorHeapManager::SetSRVHeap(m_cbvHeap);
    UINT countOfMeshes = 1;
    std::vector<Geometry::MeshData> mesh(countOfMeshes);
    mesh[0] = Geometry::CreateBox(10, 5, 10, 3);
    mBoxGeo = Mesh(mesh, mCommandList, "box");
    m_renderItems.push_back(RenderItem(mesh, mCommandList, "box"));

    m_model.Init("D:/dev/projects/learning/directx-shit/textures/models/backpack/backpack.obj", mCommandList);
    m_renderItems.push_back(RenderItem(m_model, "model"));
    for (auto& e : m_renderItems)
        m_opaqueItems.push_back(&e);
}

void App::BuildPSO()
{
    D3D12_GRAPHICS_PIPELINE_STATE_DESC opaquePsoDesc;

    ZeroMemory(&opaquePsoDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
    opaquePsoDesc.InputLayout = { mInputLayout.data(), (UINT)mInputLayout.size() };
    opaquePsoDesc.pRootSignature = mRootSignature.Get();
    opaquePsoDesc.VS =
    {
        reinterpret_cast<BYTE*>(m_shaders["standardVS"]->GetBufferPointer()),
        m_shaders["standardVS"]->GetBufferSize()
    };
    opaquePsoDesc.PS =
    {
        reinterpret_cast<BYTE*>(m_shaders["opaquePS"]->GetBufferPointer()),
        m_shaders["opaquePS"]->GetBufferSize()
    };
    opaquePsoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
    opaquePsoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
    opaquePsoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
    opaquePsoDesc.SampleMask = UINT_MAX;
    opaquePsoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    opaquePsoDesc.NumRenderTargets = 1;
    opaquePsoDesc.RTVFormats[0] = mBackBufferFormat;
    opaquePsoDesc.SampleDesc.Count = m4xMsaaState ? 4 : 1;
    opaquePsoDesc.SampleDesc.Quality = m4xMsaaState ? (m4xMsaaQuality - 1) : 0;
    opaquePsoDesc.DSVFormat = mDepthStencilFormat;
    ThrowIfFailed(Device::GetDevice()->CreateGraphicsPipelineState(&opaquePsoDesc, IID_PPV_ARGS(&mPSO["opaque"])));

    D3D12_GRAPHICS_PIPELINE_STATE_DESC opaqueWireframePsoDesc = opaquePsoDesc;
    opaqueWireframePsoDesc.RasterizerState.FillMode = D3D12_FILL_MODE_WIREFRAME;
    ThrowIfFailed(Device::GetDevice()->CreateGraphicsPipelineState(&opaqueWireframePsoDesc, IID_PPV_ARGS(&mPSO["opaque_wireframe"])));
}

void App::BuildFrameResources()
{
    for (int i = 0; i < NumFrames; ++i)
    {
        m_frameResources.push_back(std::move(FrameResource(Device::GetDevice(), 1, m_renderItems.size())));
    }
}

void App::InitImgui()
{
    ImGuiSettings::Init(mhMainWnd, Device::GetDevice(), NumFrames);

}

void App::CreateTextures()
{
    DescriptorHeapManager::SetSRVHeap(m_cbvHeap);
    for (int i = 0; i < NumFrames; i++)
    {
        m_textures.push_back(Texture("textures/container2.png",          Device::GetDevice(), mCommandList, "container_diffuse"));
        m_textures.push_back(Texture("textures/container2_specular.png", Device::GetDevice(), mCommandList , "container_specular"));
    }
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
    XMMATRIX invView = XMMatrixInverse(&XMMatrixDeterminant(view), view);
    XMMATRIX invProj = XMMatrixInverse(&XMMatrixDeterminant(proj), proj);
    XMMATRIX invViewProj = XMMatrixInverse(&XMMatrixDeterminant(viewProj), viewProj);

    XMStoreFloat4x4(&m_mainPassCB.View, XMMatrixTranspose(view));
    XMStoreFloat4x4(&m_mainPassCB.InvView, XMMatrixTranspose(invView));
    XMStoreFloat4x4(&m_mainPassCB.Proj, XMMatrixTranspose(proj));
    XMStoreFloat4x4(&m_mainPassCB.InvProj, XMMatrixTranspose(invProj));
    XMStoreFloat4x4(&m_mainPassCB.ViewProj, XMMatrixTranspose(viewProj));
    XMStoreFloat4x4(&m_mainPassCB.InvViewProj, XMMatrixTranspose(invViewProj));
    XMStoreFloat4(&m_mainPassCB.EyePosW, m_camera.GetPosition());

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
        HANDLE eventHandle = CreateEventEx(nullptr, false, false, EVENT_ALL_ACCESS);
        ThrowIfFailed(mFence->SetEventOnCompletion(m_currentFrameResource->m_fence, eventHandle));
        WaitForSingleObject(eventHandle, INFINITE);
        CloseHandle(eventHandle);
    }
	// Update the constant buffer with the latest worldViewProj matrix.
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
}

void App::DrawRenderItems(ID3D12GraphicsCommandList* cmdList, const std::vector<RenderItem*>& ritems)
{
    UINT objCBByteSize = d3dUtil::CalcConstantBufferByteSize(sizeof(ObjectConstants));

    ID3D12Resource* objectCB = m_currentFrameResource->m_objectCb.Resource();

    for (size_t i = 0; i < ritems.size(); ++i)
    {
        auto ri = ritems[i];
        auto cbvHandle = m_cbvHeap.GetGPUHandle(m_frameResources[m_frameIndex].objectIndexes[i]);
        cmdList->SetGraphicsRootDescriptorTable(0, cbvHandle);
        ri->Draw(cmdList);
    }
}

void App::Destroy()
{
	HANDLE eventHandle = CreateEventEx(nullptr, false, false, EVENT_ALL_ACCESS);
	ThrowIfFailed(mFence->SetEventOnCompletion(m_currentFrameResource->m_fence, eventHandle));
	WaitForSingleObject(eventHandle, INFINITE);
	CloseHandle(eventHandle);
}

void App::Draw(const GameTimer& gt)
{

    ComPtr<ID3D12CommandAllocator> currentCmdAlloc = m_currentFrameResource->m_cmdAlloc;
	ThrowIfFailed(currentCmdAlloc->Reset());
    if (m_isWireFrame)
    {
        ThrowIfFailed(mCommandList->Reset(currentCmdAlloc.Get(), mPSO["opaque_wireframe"].Get()));
    }
    else
    {
        ThrowIfFailed(mCommandList->Reset(currentCmdAlloc.Get(), mPSO["opaque"].Get()));
    }

	ImGuiSettings::StartFrame();
    DrawImgui();
	ImGuiSettings::EndFrame();

    mCommandList->RSSetViewports(1, &mScreenViewport);
    mCommandList->RSSetScissorRects(1, &mScissorRect);
	mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(CurrentBackBuffer(),
		D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));
    mCommandList->ClearRenderTargetView(CurrentBackBufferView(), clear_color, 0, nullptr);
    mCommandList->ClearDepthStencilView(DepthStencilView(), D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);
	mCommandList->OMSetRenderTargets(1, &CurrentBackBufferView(), true, &DepthStencilView());
	ID3D12DescriptorHeap* descriptorHeaps[] = { m_cbvHeap.GetHeap()};
	mCommandList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);
	mCommandList->SetGraphicsRootSignature(mRootSignature.Get());

    auto passCbvHandle = m_cbvHeap.GetGPUHandle(m_frameResources[m_frameIndex].passIndex);
    mCommandList->SetGraphicsRootDescriptorTable(1, passCbvHandle);

    auto textureHandle1 = m_cbvHeap.GetGPUHandle(m_textures[m_frameIndex*2].index);
    mCommandList->SetGraphicsRootDescriptorTable(2, textureHandle1);

    auto textureHandle2 = m_cbvHeap.GetGPUHandle(m_textures[m_frameIndex*2+1].index);
    mCommandList->SetGraphicsRootDescriptorTable(3, textureHandle2);

    DrawRenderItems(mCommandList.Get(), m_opaqueItems);
    //m_model.DrawModel(mCommandList.Get());
    
    auto lv = ImGuiSettings::GetDescriptorHeap();
    mCommandList->SetDescriptorHeaps(1, &lv);
	ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), mCommandList.Get());

	mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(CurrentBackBuffer(),
		D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));

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
	m_camera.OnMouseMove(p.x, p.y);
}

void App::OnKeyDown(Key key)
{
    m_camera.OnKeyDown(key);
}
