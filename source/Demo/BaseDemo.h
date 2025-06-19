#pragma once

#include <WindowsX.h>
#include "EngineCommon/include/defines.h"
#include "EngineCommon/System/Window.h"
#include "EngineCommon/System/InputManager.h"
#include "EngineCommon/Scene/Camera.h"

#include "EngineGfx/dx12/PSO.h"
#include "EngineGfx/dx12/Buffers.h"
#include "EngineGfx/Model.h"
#include "EngineGfx/BVH_Builder.h"
#include "EngineGfx/GfxContext.h"
#include "EngineGfx/RenderPass.h"
#include "RenderPasses.h"

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
    {}

    bool isPoint() const {
        return bool(m_flags & LightFlags::POINT); }
    bool isDirectional()const {
        return bool(m_flags & LightFlags::DIRECTIONAL); }
    bool isSpotlight() const{
        return bool(m_flags & LightFlags::SPOTLIGHT); }

    // shader data
    math::Vector3    m_position;
    math::Vector3    m_direction;
    LightFlags       m_flags;
};

using namespace engine::graphics;
using namespace engine::math;
using namespace engine::util;
using namespace engine::system;

struct ObjectData
{
    uint materialIndex;
};

struct LightSettings
{
    Vector3 cameraPosition;
    Vector3 viewDirection;
};

struct BindlessTable
{
    u32 passCBIndex;
    u32 materialArrayIndex;
};

struct ConstandBufferData
{
    Matrix4 view;
    Matrix4 projection;
};

class BaseDemo : public WindowApp
{
public:
    BaseDemo(u32 width, u32 height, std::string_view name);
    BaseDemo():
        m_renderModel(*this, true, "render model"),
        m_drawBVHDebugView(*this, true, "draw BVH debug view"),
        m_outputColor(*this, Vector3({1.f, 1.f, 0}), "color", 1.f)
    {}
    bool initialize()override;
    void createRenderPasses();
    SHIT_ENGINE_SINGLETONE(BaseDemo);

protected:
    void onResize(uint width, uint height) override;
    void update()override;
    void draw()override;
    void destroy()override;
    LRESULT processInput(HWND hwnd,
                         UINT msg,
                         WPARAM wParam, LPARAM lParam) override;

private:
    SwapChainSettings getCurrentWindowSettings();

public:
    GfxContext m_graphicsContext;

private:
    DemoSettings m_currentSettings;
    
    // render infrastracture
    Resource m_depthStencil;
    SwapChain m_swapChain;

    Table<DxBlob*> m_shaders;

    // resources
    Camera m_camera;

    Buffer m_buffer;

    BVHBuilder m_bvhBuilder;

    Model m_model;
    Model m_bvhModel;
    Model m_tinybvhModel;

    enum {
        ForwardPass,
        BVHDebugPass,
        RTXComputePass,
        RenderPassCount
    };

    RenderPass m_renderPasses[RenderPassCount];

    RTXPassData m_rtxData;

    UI_Vector<BaseDemo> m_outputColor;
    UI_CheckBox<BaseDemo> m_renderModel;
    UI_CheckBox<BaseDemo> m_drawBVHDebugView;

    InputManager* m_inputManager;
};

