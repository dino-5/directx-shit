#pragma once

#include "d3dApp.h"
#include "framework/include/MathHelper.h"
#include "framework/include/UploadBuffer.h"
#include "framework/include/common.h"
#include <unordered_map>
#include "framework/include/FrameResource.h"
#include "framework/include/RenderItem.h"
#include "framework/include/PassConstants.h"
#include "framework/include/GeometryGenerator.h"
#include "framework/include/ImGuiSettings.h" 
#include "framework/include/Mesh.h"
#include "framework/include/Texture.h"
#include "framework/dx12/DescriptorHeap.h"
#include "framework/include/Camera.h"
#include "framework/include/Model.h"


using namespace DirectX;
using namespace DirectX::PackedVector;

class RootSignature;
class PSO;
class ShaderInfo;

class App : public D3DApp
{
public:
	App(HINSTANCE hInstance);
    App(const App& rhs) = delete;
    App& operator=(const App& rhs) = delete;
	~App();

	virtual bool Initialize()override;

private:
    virtual void OnResize()override;
    virtual void Update(const GameTimer& gt)override;
    virtual void Draw(const GameTimer& gt)override;
    virtual void Destroy()override;
    void DrawRenderItems(ID3D12GraphicsCommandList* cmdList, const std::vector<RenderItem*>& ritems);

    void OnMouseDown(WPARAM btnState, int x, int y)override;
    void OnMouseUp(WPARAM btnState, int x, int y)override;
    void OnMouseMove();
    void OnKeyDown(Key key) override;

    void InitCamera();
    void BuildDescriptorHeaps();
    void BuildRootSignature();
    void BuildPSO();
    void BuildFrameResources();
    void BuildRenderItems();

    void InitImgui();
    void DrawImgui();

    void UpdateObjectCB(const GameTimer& gt);
    void UpdateMainPassCB(const GameTimer& gt);

private:

    std::vector<RenderItem> m_renderItems;
    std::vector<RenderItem*> m_opaqueItems;
    std::vector<RenderItem*> m_transparentItems;
    std::vector<RenderItem*> m_reflectedItems;
    std::vector<RenderItem*> m_mirror;

    std::vector<FrameResource> m_frameResources;
    FrameResource* m_currentFrameResource = nullptr;
    int m_frameIndex = 0;
    
    std::unordered_map<std::string, RootSignature> m_rootSignatures;
    ComPtr<ID3DBlob> mvsByteCode = nullptr;
    ComPtr<ID3DBlob> mpsByteCode = nullptr;

    std::unordered_map<std::string, PSO> m_pso;

    DescriptorHeap m_cbvHeap;

    XMFLOAT3 mEyePos = { 0.0f, .75f, -7.0f };
    XMFLOAT4X4 mView = MathHelper::Identity4x4();
    XMFLOAT4X4 mProj = MathHelper::Identity4x4();
    Camera m_camera;

    PassConstants m_mainPassCB;
    imguiConstants mainPassImGui;
    PassConstants m_reflectedPassCB;

    float clear_color[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
};
