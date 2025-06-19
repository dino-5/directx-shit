#include "RenderPass.h"
#include "dx12/PSO.h"
#include <string>

void RenderPass::addShader(std::string_view name,
               std::string_view entryPoint,
               std::string_view path,
               ShaderType type)
{
    ShaderInfo info;
    info.shaderName = util::to_wstring(name);
    info.entryPoint = util::to_wstring(entryPoint);
    info.path       = util::to_wstring(path);
    info.type       = type;
    shaders[(u32)type] = info;
}

void RenderPass::compilePSO()
{
    if(!shaders[0].isChanged() && !shaders[1].isChanged() && !shaders[2].isChanged())
        return;

    renderState.sig.rootSignature = &rs;
    if (!shaders[0].shaderName.empty())
    {
        renderState.sig.vs = shaders[0].createShader();
        renderState.sig.ps = shaders[1].createShader();
        pso = PSO(renderState);
    }
    else
    {
        renderState.sig.cs = shaders[2].createShader();
        pso = PSO(renderState.sig);
    }
   

}
