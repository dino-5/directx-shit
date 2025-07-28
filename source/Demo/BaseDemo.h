#pragma once

#include <WindowsX.h>
#include "EngineCommon/System/Window.h"
#include "EngineCommon/System/InputManager.h"
#include "EngineCommon/Scene/Camera.h"
#include "EngineCommon/util/ImGuiSettings.h"

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

using namespace engine::graphics;
using namespace engine::math;
using namespace engine::util;
using namespace engine::system;

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

struct RTX_BVHNode
{
    AABB aabb;
    // NOTE:: if count = 0 then first is index of the left node,
        // first + 1 is right node, and if count > 0 then first is actual index of
        // primitives and count is telling us how many primitives is related
        // to our node
    u32 first = 0, count = 0; 
};

// NOTE  : 
// 1. calculate the AABB of the node 
// 2. if it is already small enough or node has only one element the return
// 3. calculate the bins for partition
// 4. select the best partition
// 5. sort indices of node according where center of the aabb lying

struct RTX_BVH
{
    std::vector<u32> indices;
    std::vector<AABB> aabbs;
    std::vector<RTX_BVHNode> nodes;

    u32 count = 0;

    void createBVH(Sphere* array, u32 arrayCount);
};

using SphereUI = std::function<bool(std::string_view name, Sphere* sphere, int i)>;
using uiSphereActionCallback = std::function<void(BaseDemo&, Sphere, int)>;

inline bool locSphereUI(std::string_view str, Sphere* sphere, int index)
{
    imgui::PushID(index);
    imgui::Text(str);
    bool res = false;
    res |= imgui::SliderFloat3("Pos", sphere->center.data(), -100, 100);
    res |= imgui::SliderFloat3("Color", sphere->mat.color.data(), 0, 1);
    res |= imgui::SliderFloat("Radius", &sphere->radius, 1,100);

    if(sphere->mat.type == Metal)
        res |= imgui::SliderFloat("Fuzy", &sphere->mat.metalFuzy, 0,1);

    if(sphere->mat.type == Dielectric)
        res |= imgui::SliderFloat("Refraction bending", &sphere->mat.refractionIndex, 0,2);

    int material = (int)sphere->mat.type;
    if (imgui::Combo("Material Type", &material, MaterialTypeNames, MaterialCount)) 
    {
            sphere->mat.type = (MaterialType)material;
    }
    imgui::PopID();
    return res;
}

struct UI_Sphere : public UI_ElementGenericInterface<BaseDemo, 
                            SphereUI, Sphere, uiSphereActionCallback>
{
    using Super = UI_ElementGenericInterface<BaseDemo, SphereUI, Sphere,
                                            uiSphereActionCallback>;
    superFunctions()
    void onUIAction() override
    {
        if (!Super::object)
            return;
        if (function(getStringView(), getDataPtr(), index) && call)
            call(getObject(), getData(), index);
    }
    UI_Sphere()
    {
        setFunction(locSphereUI);
    }
    
    UI_Sphere(BaseDemo& obj, Sphere aData, std::string_view aName):
        Super(obj, aData, aName)
    {
        setFunction(locSphereUI);
    }

    int index = 0;
};

class BaseDemo : public WindowApp
{
public:
    BaseDemo(u32 width, u32 height, std::string_view name);
    BaseDemo():
        m_renderModel(*this, true, "render model"),
        m_drawBVHDebugView(*this, true, "draw BVH debug view"),
        m_drawRTXBVHDebugView(*this, false, "draw RTX BVH debug view"),
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
    Model m_rtxbvhModel;

    enum {
        ForwardPass,
        BVHDebugPass,
        RTXComputePass,
        RenderPassCount
    };

    RenderPass m_renderPasses[RenderPassCount];
    constexpr static uint sphereCount = 500;
    u32 m_currentSphereCount = 0;

    UI_Sphere m_sphereUI[sphereCount];

    RTXPassData m_rtxData;
    RTX_BVH m_bvh;

    UI_Vector<BaseDemo> m_outputColor;
    UI_CheckBox<BaseDemo> m_renderModel;
    UI_CheckBox<BaseDemo> m_drawBVHDebugView;
    UI_CheckBox<BaseDemo> m_drawRTXBVHDebugView;

    InputManager* m_inputManager;
};

