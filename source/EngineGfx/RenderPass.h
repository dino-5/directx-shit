#pragma once
#include "GfxContext.h"
#include "dx12/PSO.h"
#include "dx12/RootSignature.h"
#include <array>
#include <string_view>

using namespace engine::graphics;

namespace engine::graphics
{
class Model;
};

enum class InputType : u32
{ 
    // per model
    ModelTransform, // TODO : implement transforms 
    // per submesh
    Material,
    ObjectTransform, // unused
    Count
};

struct Input
{
    InputType type;
    u32 index;
};

struct RenderPass;
typedef void (*RenderPassDraw)(GfxContext&, Model*, RenderPass&,
                               void*);
typedef void (*RenderPassInit)(GfxContext&, RenderPass&);
typedef void (*RenderPassResize)(GfxContext&, RenderPass&);

struct RenderPass
{
    PSO pso;
    RootSignature rs;
    std::array<Input, (u32)InputType::Count> inputs;
    Table<DxBlob*> shadersBin;
    ShaderInfo shaders[3];
    RenderState renderState;

    bool enabled = true;
    void* data = nullptr;

    RenderPassDraw _execute;
    RenderPassInit _init;
    RenderPassResize _resize;

    void execute(GfxContext& context, Model* model,
                 void* passData = nullptr)
    {
        if(enabled && _execute)
            _execute(context, model, *this, passData);
    }
    void init(GfxContext& context)
    {
        if(_init)
            _init(context, *this);
    }
    void resize(GfxContext& context)
    {
        if(_resize)
            _resize(context, *this);
    }

    void addShader(std::string_view name,
                   std::string_view entryPoint,
                   std::string_view path,
                   ShaderType type);
    void compilePSO();

};

inline RenderPass CreateRenderPass(RenderPassInit init,
                                   RenderPassResize resize,
                                   RenderPassDraw exec,
                                   bool enable,
                                   GfxContext& context)
{
    RenderPass pass;
    pass._init = init;
    pass._resize = resize;
    pass._execute = exec;
    pass.enabled = enable;
    pass.init(context);
    return pass;
}

