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
	using onLightChangeCallback = std::function<void()>;
	Light(math::Vector3 vec, std::string name, float range, onLightChangeCallback call);
	Light(const Light&)=delete;
	Light(const Light&& other) 
		: m_position(std::move(other.m_position)),
		  m_name(std::move(other.m_name)),
		  m_positionRange(other.m_positionRange),
		  m_callback(std::move(other.m_callback))
	{}
	void onImgui()
	{
		if(m_position.onImGui(m_name, -m_positionRange, m_positionRange))
			m_callback();
	}

	math::Vector3 m_position;
	std::string m_name;
	float m_positionRange;
	onLightChangeCallback m_callback;
};

struct ObjectData
{
	uint materialIndex;
};

struct LightSettings
{
	math::Vector3 cameraPosition;
	math::Vector3 viewDirection;
};

namespace gfx = engine::graphics;

struct BindlessTable
{
    u32 passCBIndex;
    u32 materialArrayIndex;
};

struct ConstandBufferData
{
    math::Matrix4 view;
    math::Matrix4 projection;
};

class BaseDemo : public WindowApp
{
public:
	BaseDemo(u32 width, u32 height, std::string name);
	BaseDemo()=default;
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
	system::InputManager* m_inputManager;
	Table<DxBlob*> m_shaders;
	graphics::GfxContext m_graphicsContext;

	// resources
	gfx::Camera m_camera;

	gfx::Buffer m_buffer;
	gfx::BufferObject<Light> m_lightBuffer;
	gfx::ConstantBuffer m_constBuffer;

	gfx::ConstantBuffer m_lightSettingsResource;

	// deferred rendering - geometry pass
	gfx::PSO m_pso;
	gfx::RootSignature m_rootSignature;
	gfx::Model m_model;

	// deferred rendering - lighting pass
	gfx::PSO m_lightingPSO;
	gfx::RootSignature m_lightingRS;

	// visualize light
	gfx::PSO m_lightBoxPSO;
	gfx::RootSignature m_lightBoxRS;
	gfx::Model m_lightCube;
	math::Vector3 m_lightCubePosition;

	//deferred rendering
	gfx::Resource m_positionRT[config::NumFrames];
	gfx::Resource m_albedoRT[config::NumFrames];
	gfx::Resource m_normalRT[config::NumFrames];
};

