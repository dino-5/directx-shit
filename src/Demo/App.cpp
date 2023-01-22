#include "framework/dx12/Device.h"
#include "include/App.h"
#include "framework/dx12/PSO.h"
#include "external/imgui/imgui.h"
#include "external/imgui/imgui_impl_dx12.h"
#include "Framework/dx12/RootSignature.h"
#include "Framework/dx12/PipelineStates.h"
#include "Framework/dx12/PSO.h"
#include "external/stb/stb_image.h"
#include "Framework/math/Matrix.h"


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
		
    ThrowIfFailed(mCommandList->Reset(mDirectCmdListAlloc.Get(), nullptr));
    InitCamera();
    BuildRootSignature();
    BuildDescriptorHeaps();
    BuildSkyBox();
    BuildRenderItems();
    BuildFrameResources();
    BuildPSO();
    InitImgui();
    InitBlur();
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
    DescriptorHeapManager::SetSRVHeap(m_cbvHeap);
}

void App::BuildRootSignature()
{
    {
        const int numberOfRootArguments = 5;
        RootArguments rootArguments(numberOfRootArguments);
        rootArguments[0] = RootArgument::CreateCBV(0);
        rootArguments[1] = RootArgument::CreateCBV(1);
        rootArguments[2] = RootArgument::CreateCBV(2);
        auto srvRange1 = DescriptorRange(DescriptorRangeType::SRV, 1, 0);
        rootArguments[3] = RootArgument::CreateTable(1, srvRange1,
            ShaderVisibility::PIXEL);

        auto srvRange2 = DescriptorRange(DescriptorRangeType::SRV, 1, 1);
        rootArguments[4] = RootArgument::CreateTable(1, srvRange2,
            ShaderVisibility::PIXEL);
        m_rootSignatures["default"] = RootSignature(rootArguments);
    }

    {
		const int numberOfRootArgBlur = 3;
		RootArguments blurRootArg(numberOfRootArgBlur);
        blurRootArg[0] = RootArgument::CreateConstants(12, 0);
        auto srvRange0 = DescriptorRange(DescriptorRangeType::SRV, 1, 0);
        blurRootArg[1] = RootArgument::CreateTable(1, srvRange0, ShaderVisibility::ALL);
        
        auto uavRange1 = DescriptorRange(DescriptorRangeType::UAV, 1, 0);
        blurRootArg[2] = RootArgument::CreateTable(1, uavRange1, ShaderVisibility::ALL);

        m_rootSignatures["blur"] = RootSignature(blurRootArg);
    }

    {
        const int numberOfRooArgSkyBox = 3;
        RootArguments skyBoxArg(numberOfRooArgSkyBox);
        skyBoxArg[0] = RootArgument::CreateCBV(0);
        skyBoxArg[1] = RootArgument::CreateCBV(1);
        auto srvRange1 = DescriptorRange(DescriptorRangeType::SRV, 1, 2);
        skyBoxArg[2] = RootArgument::CreateTable(1, srvRange1, ShaderVisibility::PIXEL);
        m_rootSignatures["SkyBox"] = RootSignature(skyBoxArg);
    }
}

void App::BuildRenderItems()
{
    UINT countOfMeshes = 1;

    if(0)
    {
		std::vector<Geometry::MeshData> mesh(countOfMeshes);
        mesh[0] = Geometry::CreateQuad();
        Material grassTexture;
        grassTexture.diffuseTexture =
            TextureColection::CreateTexture("textures/grass.png", Device::GetDevice(), mCommandList, "grass");
        grassTexture.specularTexture = grassTexture.diffuseTexture;
        std::vector<std::pair< Material, std::vector<Geometry::MeshData> >> val =
        { { grassTexture, mesh } };
        Mesh quad = Mesh(val, mCommandList, "quad");
		m_renderItems.push_back(RenderItem(quad, "quad", RenderItem::RenderItemState::TRANSPARENT_STATE));
    }

    //box for light source
    {
        std::vector<Geometry::MeshData> mesh(1);
        mesh[0] = Geometry::CreateBox();
        Mesh lightBox(mesh, mCommandList, "Box");
        RenderItem ritem(lightBox, "Box", RenderItem::RenderItemState::BOX);
        float scale = 5.0f;
        ritem.m_transformation = XMMatrixScaling(scale, scale, scale);
        m_renderItems.push_back(ritem);
    }

    Model model; 
    model.Init("D:/dev/projects/learning/directx-shit/textures/models/backpack/backpack.obj", mCommandList);
    RenderItem model_item(model, "model");
    m_renderItems.push_back(model_item);
    model_item.m_state = RenderItem::RenderItemState::REFLECTED;
    model_item.m_transformation.m_state = ObjectConstants::ObjectConstantsState::REFLECTED;
	model_item.m_objCbIndex = RenderItem::g_objectCBIndex++;
    m_renderItems.push_back(model_item);

    m_renderItems.reserve(4);
    for (auto& e : m_renderItems)
        if (e.m_state == RenderItem::RenderItemState::OPAQUE_STATE)
            m_opaqueItems.push_back(&e);
        else if (e.m_state == RenderItem::RenderItemState::REFLECTED)
            m_reflectedItems.push_back(&e);
        else if (e.m_state == RenderItem::RenderItemState::BOX)
            m_box.push_back(&e);
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
    Shader::CreateShader({"colorPixel",  "D:/dev/projects/learning/directx-shit/src/Shaders/color.hlsl", "PS", ShaderType::PIXEL});
    Shader::CreateShader({"WhitePixel",  "D:/dev/projects/learning/directx-shit/src/Shaders/color.hlsl", "WhiteBox", ShaderType::PIXEL});

    Shader::CreateShader({"SkyBoxVS",  "D:/dev/projects/learning/directx-shit/src/Shaders/color.hlsl", "SkyBoxVS", ShaderType::VERTEX});
    Shader::CreateShader({"SkyBoxPS",  "D:/dev/projects/learning/directx-shit/src/Shaders/color.hlsl", "SkyBoxPS", ShaderType::PIXEL});

    Shader::CreateShader({"blurVertCS",  "D:/dev/projects/learning/directx-shit/src/Shaders/Blur.hlsl",  "VertBlurCS", ShaderType::COMPUTE});
    Shader::CreateShader({"blurHorCS",  "D:/dev/projects/learning/directx-shit/src/Shaders/Blur.hlsl",  "HorzBlurCS", ShaderType::COMPUTE});

    std::vector<InputLayoutElement> inputLayout =
    {
        InputLayoutElement("POSITION", Format::float3),
        InputLayoutElement("NORMAL",   Format::float3),
        InputLayoutElement("TEXTURE",  Format::float2),
    };
    InputLayout layout(inputLayout);
    ShaderInputGroup shaderBase{ "colorVertex", "colorPixel", &layout, &m_rootSignatures["default"] };

    RenderState base;
    base.SetShaderInputGroup(shaderBase);
    
    m_pso["default"] = base.Compile();
    // TODO change all setting from the objects to the pointers;

    {
        shaderBase.SetPS("WhitePixel");
        base.SetShaderInputGroup(shaderBase);
        m_pso["whiteBox"] = base.Compile();
        shaderBase.SetPS("colorPixel");
        base.SetShaderInputGroup(shaderBase);
    }

    {
		StencilOpDesc op(StencilOP::KEEP, StencilOP::KEEP, StencilOP::REPLACE, ComparisonFunc::ALWAYS);
		DepthStencilState dsState(DepthState(true, DepthWriteMask::ZERO, ComparisonFunc::LESS), StencilState(true, 0xff, 0xff, op, op));
        base.SetDepthStencilState(dsState);
        base.SetBlendState(BlendState(WriteEnable::NONE));
        m_pso["stencilMirror"] = base.Compile();
        base.SetBlendState();
    }

    {
		StencilOpDesc op(StencilOP::KEEP, StencilOP::KEEP, StencilOP::KEEP, ComparisonFunc::EQUAL);
		DepthStencilState dsState(DepthState(true, DepthWriteMask::ALL, ComparisonFunc::LESS), StencilState(true, 0xff, 0xff, op, op));
        base.SetDepthStencilState(dsState);
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

    // blur pso
	ComputeShaderInputGroup cs_ig{"blurVertCS", &m_rootSignatures["blur"]};
    {
        m_CSpso["blurVert"] = ComputePSO(cs_ig);
    }

    {
        cs_ig.SetCS("blurHorCS");
        m_CSpso["blurHor"] = ComputePSO(cs_ig);
    }

    // SkyBox PSO
    {
		std::vector<InputLayoutElement> inputLayout =
		{
			InputLayoutElement("POSITION", Format::float3),
			InputLayoutElement("NORMAL",   Format::float3),
			InputLayoutElement("TEXTURE",  Format::float2),
		};
		ShaderInputGroup shaderBase{ "SkyBoxVS", "SkyBoxPS", &layout, &m_rootSignatures["SkyBox"] };
        base.SetShaderInputGroup(shaderBase);
        base.SetRasterizerState(RasterizerState(CullMode::NONE, true));
        base.SetDepthStencilState(DepthStencilState(DepthState(true, DepthWriteMask::ALL, ComparisonFunc::LE), StencilState()));
        m_pso["SkyBox"] = base.Compile();
    }
}

void App::BuildFrameResources()
{
    const int frameResBuffers = 3;
    std::vector<FrameResourceDescription> frameResDesc(frameResBuffers);
    frameResDesc[0] = { sizeof(PassConstants), 1 };
    frameResDesc[1] = { sizeof(ObjectConstants::ObjectProperties), static_cast<uint>(m_renderItems.size()) };
    frameResDesc[2] = { sizeof(PointLight), 1};
    for (int i = 0; i < NumFrames; ++i)
    {
        m_frameResources.push_back(std::move(FrameResource<frameResBuffers>(frameResDesc)));
    }
}

void App::InitImgui()
{
    ImGuiSettings::Init(mhMainWnd, Device::GetDevice(), NumFrames);

}

void App::InitBlur()
{
    m_blur.Init(mClientWidth, mClientHeight, DXGI_FORMAT_R8G8B8A8_UNORM);
}

void App::BuildSkyBox()
{
    std::vector<std::string> faces;

    faces.push_back("right.jpg");
    faces.push_back("left.jpg");
    faces.push_back("top.jpg");
    faces.push_back("bottom.jpg");
    faces.push_back("back.jpg");
    faces.push_back("front.jpg");

    std::string rootDir = "D:/dev/projects/learning/directx-shit/textures/skybox/";
    int width, height;
    unsigned char* ptr = stbi_load((rootDir + faces[0]).c_str(), &width, &height, nullptr, 4);
    int a = sizeof(ptr);
    stbi_image_free(ptr);
    ptr = new unsigned char[width * height * 4 * 6];
    for (int i = 0; i < faces.size(); i++)
    {
		unsigned char* ptr1 = stbi_load((rootDir + faces[i]).c_str(), &width, &height, nullptr, 4);
        memcpy(&ptr[width * height * 4 * i], ptr1, width * height * 4);
    }
    m_cubeMap.Init(DXGI_FORMAT_R8G8B8A8_UNORM, width, height, 6, D3D12_RESOURCE_DIMENSION_TEXTURE2D, ResourceFlags::NONE, ResourceState::COPY_DEST);
    const UINT64 uploadBufferSize = GetRequiredIntermediateSize(m_cubeMap, 0, 6);

    textureUploadHeap.Init(CD3DX12_RESOURCE_DESC::Buffer(uploadBufferSize), ResourceState::GENERIC_READ_STATE,
        CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD));

    D3D12_SUBRESOURCE_DATA textureData[6];
	int size = 4;
    for (int i = 0; i < 6; i++)
    {
		textureData[i].pData = ptr+width*height*size*i;
		textureData[i].RowPitch = width * size;
		textureData[i].SlicePitch = textureData[i].RowPitch * height;

    }

    UpdateSubresources(mCommandList.Get(), m_cubeMap, textureUploadHeap, 0, 0, 6, textureData);
    m_cubeMap.Transition(mCommandList, ResourceState::PIXEL_SHADER_RESOURCE);

    D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
    srvDesc.TextureCube.MostDetailedMip = 0;
    srvDesc.TextureCube.MipLevels = m_cubeMap->GetDesc().MipLevels;
    srvDesc.TextureCube.ResourceMinLODClamp = 0.0f;
    srvDesc.Format = m_cubeMap->GetDesc().Format;
	DescriptorHeapManager::CurrentSRVHeap->CreateSRV(m_cubeMap, srvDesc);
}


void App::OnResize()
{
	D3DApp::OnResize();

    XMMATRIX P = XMMatrixPerspectiveFovLH(0.25f*MathHelper::Pi, AspectRatio(), 1.0f, 1000.0f);
    XMStoreFloat4x4(&mProj, P);
}

void App::UpdateLightCB(const GameTimer& gt)
{
	m_currentFrameResource->m_buffers[2].CopyData(0, &m_light);
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
            m_currentFrameResource->m_buffers[1].CopyData(renderItem.m_objCbIndex, &constants);
            renderItem.m_transformation.m_framesToUpdate--;
        }
    }
}

void App::UpdateMainPassCB(const GameTimer& gt)
{
    auto size = sizeof(XMMATRIX);
    XMMATRIX view = m_camera.GetView();
    XMMATRIX proj = XMLoadFloat4x4(&mProj);

    XMMATRIX viewProj = XMMatrixMultiply(view, proj);
    auto viewDeterm = XMMatrixDeterminant(view);
    XMMATRIX View = XMMatrixInverse(&viewDeterm, view);
    auto projDeterm = XMMatrixDeterminant(proj);
    auto viewProjDeterm = XMMatrixDeterminant(viewProj);

    XMStoreFloat4x4(&m_mainPassCB.View,        XMMatrixTranspose(view));
    //m_mainPassCB.Proj = Math::Matrix4::OrhographicProjection(0, 1, 0, 1, 0, 1);
    m_mainPassCB.Proj = Math::Matrix4::PerspectiveProjection(45.0f, mClientWidth / mClientHeight, 1, 100);
    //memcpy(m_mainPassCB.Proj, XMMatrixTranspose(proj));
    XMStoreFloat4x4(&m_mainPassCB.ViewProj,    XMMatrixTranspose(viewProj));

    auto ptrCameraPos = m_camera.GetPosition3f();
    m_mainPassCB.EyePosW[0] = ptrCameraPos[0];
    m_mainPassCB.EyePosW[1] = ptrCameraPos[1];
    m_mainPassCB.EyePosW[2] = ptrCameraPos[2];
    m_mainPassCB.EyePosW[3] = 1.0f;

    m_currentFrameResource->m_buffers[0].CopyData(0, &m_mainPassCB);
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
    UpdateLightCB(gt);
}

void App::DrawImgui()
{
	ImGuiSettings::StartFrame();
    for (auto& renderItem : m_renderItems)
    {
        renderItem.OnImGuiRender();
    }
    m_camera.OnImGui();
    mainPassImGui.OnImGui();
    m_light.OnImGui();
    if (ImGui::Button("reload shaders"))
    {
        reloadShaders = true;
    }
	ImGuiSettings::EndFrame();
	auto lv = ImGuiSettings::GetDescriptorHeap();
	mCommandList->SetDescriptorHeaps(1, &lv);
	ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), mCommandList.Get());
    
}

void App::DrawRenderItems(ID3D12GraphicsCommandList* cmdList, const std::vector<RenderItem*>& ritems)
{
    UINT objCBByteSize = d3dUtil::CalcConstantBufferByteSize(sizeof(ObjectConstants));

    auto objectCB = m_currentFrameResource->m_buffers[1].Resource()->GetGPUVirtualAddress();

    for (size_t i = 0; i < ritems.size(); ++i)
    {
        auto ri = ritems[i];
        auto cbvHandle1 = objectCB + ri->m_objCbIndex * m_currentFrameResource->m_buffers[1].mElementByteSize;
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

		auto passCbvAddress = m_frameResources[m_frameIndex].m_buffers[0].Resource()->GetGPUVirtualAddress();
		mCommandList->SetGraphicsRootConstantBufferView(1, passCbvAddress);
        
		auto lightCbvAddress = m_frameResources[m_frameIndex].m_buffers[2].Resource()->GetGPUVirtualAddress();
		mCommandList->SetGraphicsRootConstantBufferView(2, lightCbvAddress);

        bool fl_drawOpaqueItems      = bool(1);
        bool fl_drawStencilMirror    = bool(0);
        bool fl_drawReflectedItems   = bool(0);
        bool fl_drawTransparentItems = bool(0);
        bool fl_drawSkyBox           = bool(0);
        bool fl_executeBlurPass      = bool(0);

        // simple draw of opaque items
        if(fl_drawOpaqueItems)
        {
			mCommandList->OMSetStencilRef(0);
			DrawRenderItems(mCommandList.Get(), m_opaqueItems);
        }

        // draw on the stencil buffer mirror
        if(fl_drawStencilMirror)
        {
			mCommandList->SetPipelineState(m_pso["stencilMirror"]);
			mCommandList->OMSetStencilRef(1);
			DrawRenderItems(mCommandList.Get(), m_mirror);
        }

        // draw reflected items in the mirror
        if(fl_drawReflectedItems)
        {
			mCommandList->SetPipelineState(m_pso["drawStencilReflections"]);
			DrawRenderItems(mCommandList.Get(), m_reflectedItems);
        }

        // transparent items
        if(fl_drawTransparentItems)
        {
			mCommandList->SetPipelineState(m_pso["transparent"]);
			DrawRenderItems(mCommandList.Get(), m_transparentItems);
        }

        if (fl_drawSkyBox)
        {
            mCommandList->SetPipelineState(m_pso["SkyBox"]);
            mCommandList->SetGraphicsRootSignature(m_rootSignatures["SkyBox"]);
			auto passCbvAddress = m_frameResources[m_frameIndex].m_buffers[0].Resource()->GetGPUVirtualAddress();
			mCommandList->SetGraphicsRootConstantBufferView(1, passCbvAddress);
            mCommandList->SetGraphicsRootDescriptorTable(2, m_cubeMap.srv);
            DrawRenderItems(mCommandList.Get(), m_box);
            //m_box[0]->Draw(mCommandList.Get(), true);
        }

        if(fl_executeBlurPass)
			m_blur.Execute(mCommandList, m_rootSignatures["blur"], m_CSpso["blurHor"], m_CSpso["blurVert"], CurrentBackBuffer(), 4);

        //resourceTransition = CD3DX12_RESOURCE_BARRIER::Transition(CurrentBackBuffer(),
        //    D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_COPY_DEST);
        //mCommandList->ResourceBarrier(1, &resourceTransition);

        //mCommandList->CopyResource(CurrentBackBuffer(), m_blur.Output());

        //resourceTransition = CD3DX12_RESOURCE_BARRIER::Transition(CurrentBackBuffer(),
        //    D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_RENDER_TARGET);
        //mCommandList->ResourceBarrier(1, &resourceTransition);

        //imgui shit 		
		DrawImgui();

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

    if (reloadShaders)
    {
        Shader::Clear();
        BuildPSO();
        reloadShaders = false;
    }
}

void App::OnMouseDown(WPARAM btnState, int x, int y)
{
    SetCapture(mhMainWnd);
    if (btnState == 1)
        m_camera.ChangeState();
}

void App::OnMouseUp(WPARAM btnState, int x, int y)
{
	m_camera.ChangeState();
    ReleaseCapture();
}

void App::OnMouseMove()
{
    ShowCursor(true);
    POINT p{};
    GetCursorPos(&p);
	m_camera.OnMouseMove(p.x, p.y);
    ScreenToClient(mhMainWnd, &p);
}

void App::OnKeyDown(Key key)
{
    m_camera.OnKeyDown(key);
}
