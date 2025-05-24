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
#include "EngineCommon/Scene/Camera.h"
#include "EngineGfx/Model.h"
#include "EngineGfx/BVH_Builder.h"

#include "third_party/magic_enum/include/magic_enum.hpp"

class BaseDemo;
class CommandLine;
using namespace engine;
struct IDxcBlob;
using DxBlob = IDxcBlob;

// ?
struct DemoSettings
{
	graphics::SwapChainSettings m_settings; 
};

enum class LightFlags
{
	POINT,
	DIRECTIONAL,
	SPOTLIGHT
};
using namespace magic_enum::bitwise_operators;

struct Light
{
	Light(math::Vector3 vec);
	Light(const Light&)=delete;
	Light(Light&& other) 
		:m_position(std::move(other.m_position))
	{
	}

	bool isPoint() const       { return bool(m_flags & LightFlags::POINT); }
	bool isDirectional() const { return bool(m_flags & LightFlags::DIRECTIONAL); }
	bool isSpotlight() const   { return bool(m_flags & LightFlags::SPOTLIGHT); }

	// shader data
	math::Vector3    m_position;
	math::Vector3    m_direction;
	LightFlags       m_flags;

};

namespace gfx = engine::graphics;

struct ObjectData
{
	uint materialIndex;
};

struct LightSettings
{
	math::Vector3 cameraPosition;
	math::Vector3 viewDirection;
};

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
	BaseDemo(u32 width, u32 height, std::string_view name);
	BaseDemo():m_renderModel(*this, true, "render model"), m_drawBVHDebugView(*this, true, "draw BVH debug view"){}
	bool initialize()override;
	void compileShaders();
	SHIT_ENGINE_SINGLETONE(BaseDemo);

	void waitForFrame(u32 index);
    void flushGPU();

	auto& getLightBuffer() { return m_lightBuffer; }

protected:
	void onResize(uint width, uint height) override;
	void update()override;
	void draw()override;
	void destroy()override;
	LRESULT processInput(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)override;

private:
	gfx::SwapChainSettings getCurrentWindowSettings();
	u64 signal(graphics::CommandQueue& queue, ID3D12Fence* fence, u64& value);
	void waitForFence(ID3D12Fence* fence, HANDLE& fenceEvent, u64 fenceValue);

public:
	graphics::GfxContext m_graphicsContext;

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
	Table<DxBlob*> m_shaders;

	// resources
	gfx::Camera m_camera;

	gfx::Buffer m_buffer;
	gfx::BufferObject m_lightBuffer;
	gfx::ConstantBuffer m_constBuffer;

	gfx::ConstantBuffer m_lightSettingsResource;

    gfx::BVHBuilder m_bvhBuilder;

	// forward rendering -
	gfx::PSO m_pso;
	gfx::RootSignature m_rootSignature;
	gfx::Model m_model;

    // BVH debug draw
	gfx::PSO m_bvhDebugDrawPSO;
	gfx::RootSignature m_bvhDebugDrawRS;

    gfx::Model m_bvhModel;
    gfx::Model m_tinybvhModel;

    util::UI_CheckBox<BaseDemo> m_renderModel;
    util::UI_CheckBox<BaseDemo> m_drawBVHDebugView;

	system::InputManager* m_inputManager;
};

