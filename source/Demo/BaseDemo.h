#pragma once

#include <WindowsX.h>
#include "EngineCommon/include/defines.h"
#include "EngineCommon/System/Window.h"
#include "EngineCommon/System/InputManager.h"

#include "EngineGfx/dx12/Device.h"
#include "EngineGfx/dx12/SwapChain.h"
#include "EngineGfx/dx12/PSO.h"
#include "EngineGfx/dx12/Buffers.h"
#include "EngineGfx/dx12/CommandList.h"
#include "EngineGfx/dx12/CommandQueue.h"
#include "EngineGfx/dx12/DescriptorHeap.h"
#include "EngineGfx/util/Camera.h"
#include "EngineGfx/Model.h"

class CommandLine;
using namespace engine;
struct IDxcBlob;
using DxBlob = IDxcBlob;

// ?
struct DemoSettings
{
	graphics::SwapChainSettings m_settings; 
};

struct Light
{
	math::Vector3 position;
};

struct ObjectData
{
	uint materialIndex;
};

namespace gfx = engine::graphics;

struct BindlessTable
{
	u32 cbIndex;
	u32 submeshDataArray;
	uint lightArrayIndex;
};

struct ConstandBufferData
{
    math::Matrix4 view;
    math::Matrix4 perspective;
};

class BaseDemo : public WindowApp
{
public:
	BaseDemo(u32 width, u32 height, std::string name);
	bool initialize()override;
	void compileShaders();
	SHIT_ENGINE_SINGLETONE(BaseDemo);

	void waitForFrame(u32 index);

protected:
	void onResize()override {}
	void update()override;
	void draw()override;
	void destroy()override;
    LRESULT processInput(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)override;

private:
	gfx::SwapChainSettings getCurrentWindowSettings();

private:
	DemoSettings m_currentSettings;
	u32 m_currentFrameIndex = 0;
	
	// render infrastracture
	gfx::Device m_device;
	gfx::CommandList m_cmdList;
	gfx::CommandQueue m_cmdQueue;
	gfx::DescriptorHeap m_descriptorHeaps[static_cast<uint>(gfx::DescriptorHeapType::Count)];
	gfx::SwapChain m_swapChain;
    gfx::Resource m_depthStencil;
	ID3D12Fence* m_fence = nullptr;
	u64 m_fenceValue = 0;
    HANDLE m_fenceEvent;
	D3D12_VIEWPORT m_viewPort{};
	D3D12_RECT m_scissorRect{};

	// resources
	gfx::Model m_model;
	gfx::Buffer m_buffer;
	gfx::BufferObject<Light> m_lightBuffer;
	gfx::PSO m_pso;
	gfx::RootSignature m_rootSignature;
	Table<DxBlob*> m_shaders;
	gfx::Camera m_camera;
	gfx::ConstantBuffer m_constBuffer;
	gfx::ConstantBuffer m_bindlessTable;

	system::InputManager* m_inputManager;
};

