#pragma once

#include "d3dApp.h"
#include "MathHelper.h"
#include "UploadBuffer.h"
#include "common.h"
#include <unordered_map>
#include "FrameResource.h"
#include "RenderItem.h"
#include "PassConstants.h"
#include "GeometryGenerator.h"
#include "ImGuiSettings.h" 
#include "Texture.h"

using namespace DirectX;
using namespace DirectX::PackedVector;


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

    virtual void OnMouseDown(WPARAM btnState, int x, int y)override;
    virtual void OnMouseUp(WPARAM btnState, int x, int y)override;
    virtual void OnMouseMove(WPARAM btnState, int x, int y)override;

    void BuildDescriptorHeaps();
	void BuildConstantBuffers();
    void BuildRootSignature();
    void BuildShadersAndInputLayout();
    void BuildGeometry();
    void BuildPSO();
    void BuildFrameResources();
    void BuildRenderItems();
    void CreateTextures();
    void CreateRenderItem(const char* submeshName, XMMATRIX& matrix);
	void CreateRenderItem(const char* submeshName, XMFLOAT4X4& matrix);

    void InitImgui();
    void DrawImgui();

    void UpdateObjectCB(const GameTimer& gt);
    void UpdateMainPassCB(const GameTimer& gt);
    void UpdateCamera(const GameTimer& gt);

private:

    std::vector<std::unique_ptr<RenderItem>> m_renderItems;
    std::vector<RenderItem*> m_opaqueItems;
    std::vector<RenderItem*> m_transparentItems;

    std::vector<std::unique_ptr<FrameResource>> m_frameResources;
    FrameResource* m_currentFrameResource = nullptr;
    int m_currentFrameResourceIndex = 0;
    
    ComPtr<ID3D12RootSignature> mRootSignature = nullptr;
    ComPtr<ID3D12DescriptorHeap> mCbvHeap = nullptr;

    std::unique_ptr<UploadBuffer<ObjectConstants>> mObjectCB = nullptr;

	std::unique_ptr<MeshGeometry> mBoxGeo = nullptr;

    ComPtr<ID3DBlob> mvsByteCode = nullptr;
    ComPtr<ID3DBlob> mpsByteCode = nullptr;

    std::vector<D3D12_INPUT_ELEMENT_DESC> mInputLayout;
    std::unordered_map < std::string, ComPtr<ID3DBlob>> m_shaders;

    std::unordered_map<std::string, ComPtr<ID3D12PipelineState>> mPSO ;
    float m_isWireFrame = false;

    XMFLOAT3 mEyePos = { 0.0f, 0.0f, 0.0f };
    XMFLOAT4X4 mView = MathHelper::Identity4x4();
    XMFLOAT4X4 mProj = MathHelper::Identity4x4();

    PassConstants m_mainPassCB;

    float mTheta = 1.5f*XM_PI;
    float mPhi = XM_PIDIV4;
    float mRadius = 15.0f;
    UINT m_passCbvOffset = 0;
    UINT m_textureCbvOffset = 0;

    float clear_color[4] = { 0.6f, 0.7f, 0.3f, 1.0f };


    std::vector<Texture> m_textures;
    ComPtr<ID3D12Resource> m_texture[3];


    POINT mLastMousePos;
};
