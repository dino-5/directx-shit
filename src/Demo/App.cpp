#include "framework/dx12/Device.h"
#include "include/App.h"
#include "framework/dx12/PSO.h"
#include "external/imgui/imgui.h"
#include "external/imgui/imgui_impl_dx12.h"

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
    BuildShadersAndInputLayout();
    BuildDescriptorHeaps();
    BuildRenderItems();
    BuildFrameResources();
    BuildPSO();
    InitImgui();
    CreateTextures();
    //PrepareForShadows();
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
    m_cbvHeap.Init(numDescriptors, DescriptorHeapType::CBV_SRV_UAV, D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE);
}

void App::BuildRootSignature()
{
    const int numberOfRootArguments = 4;
	CD3DX12_ROOT_PARAMETER slotRootParameter[numberOfRootArguments];


	CD3DX12_DESCRIPTOR_RANGE srvTable2;
    srvTable2.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);

	CD3DX12_DESCRIPTOR_RANGE srvTable3;
    srvTable3.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 1);
    
    slotRootParameter[0].InitAsConstantBufferView(0);
    slotRootParameter[1].InitAsConstantBufferView(1);
	slotRootParameter[2].InitAsDescriptorTable(1, &srvTable2, D3D12_SHADER_VISIBILITY_PIXEL);
	slotRootParameter[3].InitAsDescriptorTable(1, &srvTable3, D3D12_SHADER_VISIBILITY_PIXEL);

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
    
    std::wstring homeDir = L"D:/dev/projects/learning/directx-shit/";
    m_shaders["standardVS"] = d3dUtil::CompileShader(L"D:/dev/projects/learning/directx-shit/src/Shaders/color.hlsl", nullptr, "VS", "vs_5_1");
    m_shaders["opaquePS"] = d3dUtil::CompileShader(L"D:/dev/projects/learning/directx-shit/src/Shaders/color.hlsl", nullptr, "PS", "ps_5_1");
    m_shaders["shadowsPS"] = d3dUtil::CompileShader(L"D:/dev/projects/learning/directx-shit/src/Shaders/shadows.hlsl", nullptr, "PS", "ps_5_1");

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
    if(0)
    {
		std::vector<Geometry::MeshData> mesh(countOfMeshes);
		mesh[0] = Geometry::CreateBox(10, 5, 10, 3);
        RenderItem box(mesh, mCommandList, "box");
		m_renderItems.push_back(box);
        box.m_state = RenderItem::RenderItemState::REFLECTED;
		box.m_transformation.m_state = ObjectConstants::ObjectConstantsState::REFLECTED;
		box.m_objCbIndex = RenderItem::g_objectCBIndex++;
		m_renderItems.push_back(box);
    }

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
    m_model.Init("D:/dev/projects/learning/directx-shit/textures/models/backpack/backpack.obj", mCommandList);
    RenderItem model(m_model, "model");
    m_renderItems.push_back(model);
    model.m_state = RenderItem::RenderItemState::REFLECTED;
    model.m_transformation.m_state = ObjectConstants::ObjectConstantsState::REFLECTED;
	model.m_objCbIndex = RenderItem::g_objectCBIndex++;
    m_renderItems.push_back(model);

    m_renderItems.reserve(4); 
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

    D3D12_DEPTH_STENCIL_DESC mirrorDepthStencilDesc;
    mirrorDepthStencilDesc.DepthEnable = true;
    mirrorDepthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
    mirrorDepthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
    mirrorDepthStencilDesc.StencilEnable = true;
    mirrorDepthStencilDesc.StencilReadMask = 0xff;
    mirrorDepthStencilDesc.StencilWriteMask = 0xff;

    mirrorDepthStencilDesc.FrontFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
    mirrorDepthStencilDesc.FrontFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
    mirrorDepthStencilDesc.FrontFace.StencilPassOp = D3D12_STENCIL_OP_REPLACE;
    mirrorDepthStencilDesc.FrontFace.StencilFunc = D3D12_COMPARISON_FUNC_ALWAYS;

    mirrorDepthStencilDesc.BackFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
    mirrorDepthStencilDesc.BackFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
    mirrorDepthStencilDesc.BackFace.StencilPassOp = D3D12_STENCIL_OP_REPLACE;
    mirrorDepthStencilDesc.BackFace.StencilFunc = D3D12_COMPARISON_FUNC_ALWAYS;

    D3D12_GRAPHICS_PIPELINE_STATE_DESC stencilMirrorPsoDesc = opaquePsoDesc;
    stencilMirrorPsoDesc.DepthStencilState = mirrorDepthStencilDesc;
    stencilMirrorPsoDesc.BlendState.RenderTarget[0].RenderTargetWriteMask = 0;
    ThrowIfFailed(Device::GetDevice()->CreateGraphicsPipelineState(&stencilMirrorPsoDesc,
        IID_PPV_ARGS(&mPSO["stencilMirror"])));

    D3D12_DEPTH_STENCIL_DESC reflectionsDSS;
    reflectionsDSS.DepthEnable = true;
    reflectionsDSS.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
    reflectionsDSS.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
    reflectionsDSS.StencilEnable = true;
    reflectionsDSS.StencilReadMask = 0xff;
    reflectionsDSS.StencilWriteMask = 0xff;

    reflectionsDSS.FrontFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
    reflectionsDSS.FrontFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
    reflectionsDSS.FrontFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
    reflectionsDSS.FrontFace.StencilFunc = D3D12_COMPARISON_FUNC_EQUAL;

    // We are not rendering backfacing polygons, so these settings do not matter.
    reflectionsDSS.BackFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
    reflectionsDSS.BackFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
    reflectionsDSS.BackFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
    reflectionsDSS.BackFace.StencilFunc = D3D12_COMPARISON_FUNC_EQUAL;

    D3D12_GRAPHICS_PIPELINE_STATE_DESC drawReflectionsPsoDesc = opaquePsoDesc;
    drawReflectionsPsoDesc.DepthStencilState = reflectionsDSS;
    //drawReflectionsPsoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_BACK;
    //drawReflectionsPsoDesc.RasterizerState.FrontCounterClockwise = true;
    ThrowIfFailed(Device::GetDevice()->CreateGraphicsPipelineState(&drawReflectionsPsoDesc,
        IID_PPV_ARGS(&mPSO["drawStencilReflections"])));


    D3D12_GRAPHICS_PIPELINE_STATE_DESC transparentPsoDesc = opaquePsoDesc;

    D3D12_RENDER_TARGET_BLEND_DESC blendStateDesc{0};
	blendStateDesc.BlendEnable = true;
	blendStateDesc.LogicOpEnable = false;
	blendStateDesc.SrcBlend = D3D12_BLEND_SRC_ALPHA;
	blendStateDesc.DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
	blendStateDesc.BlendOp = D3D12_BLEND_OP_ADD;
    blendStateDesc.SrcBlendAlpha = D3D12_BLEND_ONE;
    blendStateDesc.DestBlendAlpha = D3D12_BLEND_ZERO;
	blendStateDesc.BlendOpAlpha = D3D12_BLEND_OP_ADD;
    blendStateDesc.LogicOp = D3D12_LOGIC_OP_NOOP;
    blendStateDesc.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

    transparentPsoDesc.BlendState.RenderTarget[0] = blendStateDesc;
    ThrowIfFailed(Device::GetDevice()->CreateGraphicsPipelineState(&transparentPsoDesc,
        IID_PPV_ARGS(&mPSO["transparent"])));
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

void App::CreateTextures()
{
    DescriptorHeapManager::SetSRVHeap(m_cbvHeap);
    for (int i = 0; i < NumFrames; i++)
    {
        m_textures.push_back(Texture("textures/container2.png",          Device::GetDevice(), mCommandList, "container_diffuse"));
        m_textures.push_back(Texture("textures/container2_specular.png", Device::GetDevice(), mCommandList , "container_specular"));
    }
}

void App::PrepareForShadows()
{
    m_depthHeap.Init(3, DescriptorHeapType::DSV);
	D3D12_RESOURCE_DESC depthStencilDesc;
	depthStencilDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	depthStencilDesc.Alignment = 0;
	depthStencilDesc.Width = mClientWidth;
	depthStencilDesc.Height = mClientHeight;
	depthStencilDesc.DepthOrArraySize = 1;
	depthStencilDesc.MipLevels = 1;
	depthStencilDesc.Format = DXGI_FORMAT_R24G8_TYPELESS;

	depthStencilDesc.SampleDesc.Count = m4xMsaaState ? 4 : 1;
	depthStencilDesc.SampleDesc.Quality = m4xMsaaState ? (m4xMsaaQuality - 1) : 0;
	depthStencilDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	depthStencilDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

    for (int i = 0; i < NumFrames; i++)
    {
		D3D12_CLEAR_VALUE optClear;
		optClear.Format = mDepthStencilFormat;
		optClear.DepthStencil.Depth = 1.0f;
		optClear.DepthStencil.Stencil = 0;

        auto heapProperty = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
		ThrowIfFailed(Device::GetDevice()->CreateCommittedResource(
			&heapProperty,
			D3D12_HEAP_FLAG_NONE,
			&depthStencilDesc,
			D3D12_RESOURCE_STATE_COMMON,
			&optClear,
			IID_PPV_ARGS(shadowMaps[i].GetAddressOf())));

		D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc;
		dsvDesc.Flags = D3D12_DSV_FLAG_NONE;
		dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
		dsvDesc.Format = mDepthStencilFormat;
		dsvDesc.Texture2D.MipSlice = 0;
        depthHandle[i] = m_depthHeap.CreateDSV(shadowMaps[i].Get(), dsvDesc);
        D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
        srvDesc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
        srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
        srvDesc.Texture2D.MipLevels = 1;
		srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
        srvHandle[i] = m_cbvHeap.CreateSRV(srvDesc, shadowMaps[i].Get());

        auto resourceTransition = CD3DX12_RESOURCE_BARRIER::Transition(shadowMaps[i].Get(),
            D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_DEPTH_WRITE);
		mCommandList->ResourceBarrier(1, &resourceTransition);
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

void App::UpdateShadowPassCB()
{
    XMMATRIX view = Camera::LookAt(m_mainPassCB.lightPos, m_mainPassCB.lightDir);
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

    XMStoreFloat4x4(&m_shadowPassCB.View,        XMMatrixTranspose(view));
    XMStoreFloat4x4(&m_shadowPassCB.InvView,     XMMatrixTranspose(invView));
    XMStoreFloat4x4(&m_shadowPassCB.Proj,        XMMatrixTranspose(proj));
    XMStoreFloat4x4(&m_shadowPassCB.InvProj,     XMMatrixTranspose(invProj));
    XMStoreFloat4x4(&m_shadowPassCB.ViewProj,    XMMatrixTranspose(viewProj));
    XMStoreFloat4x4(&m_shadowPassCB.InvViewProj, XMMatrixTranspose(invViewProj));
    XMStoreFloat4x4(&m_shadowPassCB.shadowView,  XMMatrixTranspose(shadowView));
    m_shadowPassCB.EyePosW  = m_mainPassCB.lightPos;

    m_currentFrameResource->m_passCb.CopyData(1, m_shadowPassCB);
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
        ri->Draw(cmdList, load_texture);
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
    if (m_isWireFrame)
    {
        ThrowIfFailed(mCommandList->Reset(currentCmdAlloc.Get(), mPSO["opaque_wireframe"].Get()));
    }
    else
    {
        ThrowIfFailed(mCommandList->Reset(currentCmdAlloc.Get(), mPSO["shadows"].Get()));
    }

	ID3D12DescriptorHeap* descriptorHeaps[] = { m_cbvHeap.GetHeap()};
    if (true)
    {
        // main pass 
        auto resourceTransition = CD3DX12_RESOURCE_BARRIER::Transition(CurrentBackBuffer(),
            D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
		mCommandList->ResourceBarrier(1, &resourceTransition);
		mCommandList->SetPipelineState(mPSO["opaque"].Get());
		load_texture = true;

		mCommandList->RSSetViewports(1, &mScreenViewport);
		mCommandList->RSSetScissorRects(1, &mScissorRect);
		mCommandList->ClearDepthStencilView(DepthStencilView(), D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);
		mCommandList->ClearRenderTargetView(CurrentBackBufferView(), clear_color, 0, nullptr);
        auto backBuffer = CurrentBackBufferView();
        auto dsvBuffer = DepthStencilView();
		mCommandList->OMSetRenderTargets(1, &backBuffer, true, &dsvBuffer);
		mCommandList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);
		mCommandList->SetGraphicsRootSignature(mRootSignature.Get());

		auto passCbvAddress = m_frameResources[m_frameIndex].m_passCb.Resource()->GetGPUVirtualAddress();
		mCommandList->SetGraphicsRootConstantBufferView(1, passCbvAddress);

		auto textureHandle1 = m_cbvHeap.GetGPUHandle(m_textures[m_frameIndex*2].index);
		mCommandList->SetGraphicsRootDescriptorTable(2, textureHandle1);

		auto textureHandle2 = m_cbvHeap.GetGPUHandle(m_textures[m_frameIndex*2+1].index);
		mCommandList->SetGraphicsRootDescriptorTable(3, textureHandle2);

		UINT objCBByteSize = d3dUtil::CalcConstantBufferByteSize(sizeof(ObjectConstants));

        mCommandList->OMSetStencilRef(0);

		DrawRenderItems(mCommandList.Get(), m_opaqueItems);

        mCommandList->SetPipelineState(mPSO["stencilMirror"].Get());
        mCommandList->OMSetStencilRef(1);
		DrawRenderItems(mCommandList.Get(), m_mirror);

        mCommandList->SetPipelineState(mPSO["drawStencilReflections"].Get());
        DrawRenderItems(mCommandList.Get(), m_reflectedItems);

        // transparent items

		mCommandList->SetPipelineState(mPSO["transparent"].Get());
		load_texture = true;
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
