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
 
    BuildRootSignature();
    BuildShadersAndInputLayout();
    BuildGeometry();
    BuildRenderItems();
    BuildFrameResources();
    BuildDescriptorHeaps();
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

void App::BuildDescriptorHeaps()
{
    UINT objCount = m_renderItems.size();
    UINT numberOfTextures = 1;
    UINT numberOfPassDescriptors = 1;
    UINT numDescriptors = (objCount + numberOfPassDescriptors + numberOfTextures) * NumFrames;
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
        auto objectCB = m_frameResources[frameIndex]->m_objectCb->Resource();
        for (UINT i = 0; i < objCount; ++i)
        {
            D3D12_GPU_VIRTUAL_ADDRESS cbAddress = objectCB->GetGPUVirtualAddress();
            m_frameResources[frameIndex]->objectIndex = m_cbvHeap.CreateCBV(cbAddress, objCBByteSize);
        }
    }

    UINT passCBByteSize = d3dUtil::CalcConstantBufferByteSize(sizeof(PassConstants));

    for (int frameIndex = 0; frameIndex < NumFrames; ++frameIndex)
    {
        auto passCB = m_frameResources[frameIndex]->m_passCb->Resource();
        D3D12_GPU_VIRTUAL_ADDRESS cbAddress = passCB->GetGPUVirtualAddress();
        m_frameResources[frameIndex]->passIndex = m_cbvHeap.CreateCBV(cbAddress, passCBByteSize);
    }
}

void App::BuildRootSignature()
{
	// Root parameter can be a table, root descriptor or root constants.
	CD3DX12_ROOT_PARAMETER slotRootParameter[3];

	// Create a single descriptor table of CBVs.
	CD3DX12_DESCRIPTOR_RANGE cbvTable0;
	cbvTable0.Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0);

	CD3DX12_DESCRIPTOR_RANGE cbvTable1;
	cbvTable1.Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 1);

	CD3DX12_DESCRIPTOR_RANGE cbvTable2;
    cbvTable2.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);


	slotRootParameter[0].InitAsDescriptorTable(1, &cbvTable0);
	slotRootParameter[1].InitAsDescriptorTable(1, &cbvTable1);
	slotRootParameter[2].InitAsDescriptorTable(1, &cbvTable2, D3D12_SHADER_VISIBILITY_PIXEL);

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


	CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc(3, slotRootParameter, 1, &sampler, 
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

void App::BuildGeometry()
{
    UINT countOfMeshes = 2;
    GeometryGenerator generator;
    std::vector<GeometryGenerator::MeshData> mesh(countOfMeshes);
    std::vector<SubmeshGeometry> submeshes(countOfMeshes);

    std::vector<std::string> submeshNames = {
        "box",
        "cylinder"
    };

    mesh[0] = generator.CreateBox(10, 5, 10, 3);
    mesh[1] = generator.CreateCylinder(0.5f, 0.3f, 3.0f, 20, 20);

    UINT vertexOffset = 0;
    UINT indexOffset = 0;
    UINT totalVertexCount = 0;

    for (int i=0;i<mesh.size();i++)
    {
        UINT meshElementVertexOffset = vertexOffset;
        UINT meshElementIndexOffset = indexOffset;

        submeshes[i].BaseVertexLocation = meshElementVertexOffset;
        submeshes[i].StartIndexLocation = meshElementIndexOffset;
        submeshes[i].IndexCount = mesh[i].Indices32.size();

        vertexOffset += mesh[i].Vertices.size();
        indexOffset += mesh[i].Indices32.size();
        totalVertexCount += mesh[i].Vertices.size();
    }

	std::vector<Vertex> vertices(totalVertexCount);
    UINT k = 0;

    for (auto& i : mesh)
    {
		for (size_t j = 0; j < i.Vertices.size(); ++j, ++k)
		{
			vertices[k].Pos = i.Vertices[j].Position;
			vertices[k].Normal = i.Vertices[j].Normal;
            vertices[k].Tex = i.Vertices[j].TexC;
		}
    }

    std::vector<std::uint16_t> indices;
    for (auto& i : mesh)
		indices.insert(indices.end(), i.Indices32.begin(), i.Indices32.end());

    const UINT vbByteSize = vertices.size() * sizeof(Vertex);
    const UINT ibByteSize = indices.size() * sizeof(std::uint16_t);

	mBoxGeo = std::make_unique<MeshGeometry>();
	mBoxGeo->Name = "boxGeo";
    mBoxGeo->Init(Device::GetDevice(), mCommandList,
        vertices.data(), vbByteSize, sizeof(Vertex),
        indices.data(), ibByteSize, DXGI_FORMAT_R16_UINT);

    for(int i=0;i<submeshes.size();i++)
		mBoxGeo->DrawArgs[submeshNames[i]] = submeshes[i];
}

void App::CreateRenderItem(const char* renderItemName, XMMATRIX& matrix)
{
    auto rItem = std::make_unique<RenderItem>();
    XMStoreFloat4x4(&rItem->objectParam.World, matrix);
    rItem->m_objCbIndex = 0;
    rItem->Geo = mBoxGeo.get();
    rItem->m_indexCount = mBoxGeo->DrawArgs[renderItemName].IndexCount;
    rItem->m_startIndexLocation= mBoxGeo->DrawArgs[renderItemName].StartIndexLocation;
    rItem->m_baseVertexLocation= mBoxGeo->DrawArgs[renderItemName].BaseVertexLocation;
    rItem->m_name = std::string(renderItemName);
    m_renderItems.push_back(std::move(rItem));
}

void App::CreateRenderItem(const char* renderItemName, XMFLOAT4X4& matrix)
{
    auto rItem = std::make_unique<RenderItem>();
    rItem->objectParam.World = matrix;
    rItem->m_objCbIndex = 0;
    rItem->Geo = mBoxGeo.get();
    rItem->m_indexCount = mBoxGeo->DrawArgs[renderItemName].IndexCount;
    rItem->m_startIndexLocation= mBoxGeo->DrawArgs[renderItemName].StartIndexLocation;
    rItem->m_baseVertexLocation= mBoxGeo->DrawArgs[renderItemName].BaseVertexLocation;
    rItem->m_name = std::string(renderItemName);
    m_renderItems.push_back(std::move(rItem));
}

void App::BuildRenderItems()
{
    CreateRenderItem("box",XMMatrixScaling(2.0f, 2.0f, 2.0f) * XMMatrixTranslation(0.0f, 0.5f, 0.0f));

    for (auto& e : m_renderItems)
        m_opaqueItems.push_back(e.get());
}

void App::BuildPSO()
{
    D3D12_GRAPHICS_PIPELINE_STATE_DESC opaquePsoDesc;

    //
    // PSO for opaque objects.
    //
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
        m_frameResources.push_back(std::make_unique<FrameResource>(Device::GetDevice(), 1, m_renderItems.size()));
    }
}

void App::InitImgui()
{
    ImGuiSettings::Init(mhMainWnd, Device::GetDevice(), NumFrames);

}

void App::CreateTextures()
{

    for (int i = 0; i < NumFrames; i++)
    {
        m_textures.push_back(Texture("textures/wall.jpg", Device::GetDevice(), mCommandList, m_cbvHeap));
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
    auto currObjectCb = m_currentFrameResource->m_objectCb.get();
    for (auto& e : m_renderItems)
    {
        if (e->numFramesDirty > 0)
        {
            XMMATRIX world = XMLoadFloat4x4(&e->objectParam.World);
            ObjectConstants constants;
            XMStoreFloat4x4(&constants.World, XMMatrixTranspose(world));
            currObjectCb->CopyData(e->m_objCbIndex, constants);
            e->numFramesDirty--;
        }
    }
}

void App::UpdateCamera(const GameTimer& gt)
{
    mEyePos.x = mRadius * sinf(mPhi) * cosf(mTheta);
    mEyePos.z = mRadius * sinf(mPhi) * sinf(mTheta);
    mEyePos.y = mRadius * cosf(mPhi);

    // Build the view matrix.
    XMVECTOR pos = XMVectorSet(mEyePos.x, mEyePos.y, mEyePos.z, 1.0f);
    XMVECTOR target = XMVectorZero();
    XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

    XMMATRIX view = XMMatrixLookAtLH(pos, target, up);
    XMStoreFloat4x4(&mView, view);
}

void App::UpdateMainPassCB(const GameTimer& gt)
{
    XMMATRIX view = XMLoadFloat4x4(&mView);
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
    m_mainPassCB.EyePosW = mEyePos;

    m_mainPassCB.lightPos = XMFLOAT3(2.0f, 3.0, 10);
    m_mainPassCB.lightColor = XMFLOAT3( 1.0f, 1.0f, 1.0f );
    m_mainPassCB.boxColor =  XMFLOAT3(0.5f, 0.6, 0.7);
    auto currPassCB = m_currentFrameResource->m_passCb.get();
    currPassCB->CopyData(0, m_mainPassCB);

}

void App::Update(const GameTimer& gt)
{
    UpdateCamera(gt);
    m_frameIndex = (m_frameIndex + 1) % NumFrames;
    m_currentFrameResource = m_frameResources[m_frameIndex].get();

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
        renderItem->OnImGuiRender();
    }
}

void App::DrawRenderItems(ID3D12GraphicsCommandList* cmdList, const std::vector<RenderItem*>& ritems)
{
    UINT objCBByteSize = d3dUtil::CalcConstantBufferByteSize(sizeof(ObjectConstants));

    auto objectCB = m_currentFrameResource->m_objectCb->Resource();

    // For each render item...
    for (size_t i = 0; i < ritems.size(); ++i)
    {
        auto ri = ritems[i];
        if (!ri->m_indexCount)
            continue;
        cmdList->IASetVertexBuffers(0, 1, &ri->Geo->VertexBufferView());
        cmdList->IASetIndexBuffer(&ri->Geo->IndexBufferView());
        cmdList->IASetPrimitiveTopology(ri->m_primitiveType);
        auto cbvHandle = m_cbvHeap.GetGPUHandle(m_frameResources[m_frameIndex]->objectIndex);
        cmdList->SetGraphicsRootDescriptorTable(0, cbvHandle);
        cmdList->DrawIndexedInstanced(ri->m_indexCount, 1, ri->m_startIndexLocation, ri->m_baseVertexLocation, 0);
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

    auto& currentCmdAlloc = m_currentFrameResource->m_cmdAlloc;
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

    auto passCbvHandle = m_cbvHeap.GetGPUHandle(m_frameResources[m_frameIndex]->passIndex);
    mCommandList->SetGraphicsRootDescriptorTable(1, passCbvHandle);

    auto textureHandle = m_cbvHeap.GetGPUHandle(m_textures[m_frameIndex].index);
    mCommandList->SetGraphicsRootDescriptorTable(2, textureHandle);

    DrawRenderItems(mCommandList.Get(), m_opaqueItems);

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
    mLastMousePos.x = x;
    mLastMousePos.y = y;

    SetCapture(mhMainWnd);
}

void App::OnMouseUp(WPARAM btnState, int x, int y)
{
    ReleaseCapture();
}

void App::OnMouseMove(WPARAM btnState, int x, int y)
{
    if((btnState & MK_LBUTTON) != 0)
    {
        float sence = -0.35f;
        float dx = XMConvertToRadians(sence*static_cast<float>(x - mLastMousePos.x));
        float dy = XMConvertToRadians(sence*static_cast<float>(y - mLastMousePos.y));

        mTheta += dx;
        mPhi += dy;

        // Restrict the angle mPhi.
        mPhi = MathHelper::Clamp(mPhi, 0.1f, MathHelper::Pi - 0.1f);
    }
    else if((btnState & MK_RBUTTON) != 0)
    {
        // Make each pixel correspond to 0.005 unit in the scene.
        float dx = 0.005f*static_cast<float>(x - mLastMousePos.x);
        float dy = 0.005f*static_cast<float>(y - mLastMousePos.y);

        // Update the camera radius based on input.
        mRadius += dx - dy;

        // Restrict the radius.
        mRadius = MathHelper::Clamp(mRadius, 3.0f, 15.0f);
    }

    mLastMousePos.x = x;
    mLastMousePos.y = y;
}
