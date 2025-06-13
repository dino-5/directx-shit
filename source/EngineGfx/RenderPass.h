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

struct RenderPass
{
    PSO pso;
    RootSignature rs;
    std::array<Input, (u32)InputType::Count> inputs;
    Table<DxBlob*> shaders;
    RenderPassDraw _execute;
    RenderPassInit _init;
    bool enabled = true;
    void* data;

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
    void addShader(std::string_view name,
                   std::string_view entryPoint,
                   std::string_view path,
                   ShaderType type,
                   ShaderInputGroup* sig = nullptr);

};

inline RenderPass CreateRenderPass(RenderPassInit init,
                            RenderPassDraw exec,
                            bool enable)
{
    RenderPass pass;
    pass._init = init;
    pass._execute = exec;
    pass.enabled = enable;
    return pass;
}

