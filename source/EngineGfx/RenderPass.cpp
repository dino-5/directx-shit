#include "RenderPass.h"
#include "dx12/PSO.h"
#include <string>

void RenderPass::addShader(std::string_view name,
               std::string_view entryPoint,
               std::string_view path,
               ShaderType type,
               ShaderInputGroup* sig)
{
    ShaderInfo info(&shadersBin);
    info.shaderName = util::to_wstring(name);
    info.entryPoint = util::to_wstring(entryPoint);
    info.path       = util::to_wstring(path);
    info.type       = type;
    shadersDesc[(u32)type] = info;

    if(!sig)
        return;

    if(type == ShaderType::PIXEL)
        sig->pixelShader = info.createShader();
    else if(type == ShaderType::VERTEX)
        sig->vertexShader = info.createShader();
    else 
        sig->computeShader = info.createShader();
}

void RenderPass::compilePSO()
{
    ShaderInputGroup sig;
    if (!shadersDesc[0].shaderName.empty())
    {
        sig.vertexShader = shadersDesc[0].createShader();
        sig.pixelShader = shadersDesc[1].createShader();
    }
    else
    {
        sig.computeShader = shadersDesc[2].createShader();
    }
   
    sig.rootSignature = &rs;
    pso = PSO(sig);

}
