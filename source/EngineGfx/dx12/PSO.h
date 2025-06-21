#ifndef PSO_H
#define PSO_H

#include <d3d12.h>
#include <dxgiformat.h>
#include <filesystem>
#include <string>
#include <string_view>
#include <vector>
#include <functional>
#include <unordered_map>
#include "PipelineStates.h"
#include "RootSignature.h"
#include "EngineCommon/util/Util.h"
#include "EngineCommon/System/Filesystem.h"

struct IDxcCompiler3;
using DxCompiler = IDxcCompiler3;
struct IDxcUtils;
using DxUtils= IDxcUtils;
struct IDxcBlob;
using DxBlob = IDxcBlob;
struct IDxcIncludeHandler;
using DxIncludeHandler = IDxcIncludeHandler;

namespace engine::graphics
{
D3D12_SHADER_BYTECODE getShader(DxBlob* blob);

enum class ShaderType
{
    VERTEX,
    PIXEL,
    COMPUTE
};
std::wstring GetShaderTypeString(ShaderType type);

struct ShaderInfo
{
    // TODO : Add dependencies on the include files
    DxBlob* bin = nullptr;
    std::wstring shaderName{};
    std::wstring path{};
    std::wstring entryPoint{};
    ShaderType   type{};
    D3D12_SHADER_BYTECODE createShader();
    bool isChanged()
    {
        if(shaderName.empty())
            return false;

        std::string orPath = util::to_string(path);
        std::string tempPath = util::to_string(getTempPath());
        if(!fs::exists(tempPath))
        {
            copyFile(orPath, tempPath);
            return true;
        }

        error_code code;
        auto original = GetLastEditTime(orPath, code);
        auto temp = GetLastEditTime(tempPath, code);
        if((!code && original > temp) || !bin)
        {
            copyFile(orPath, tempPath);
            return true;
        }

        return false;
    }

    std::wstring getTempPath() const
    {
        return path + L"t";
    }
};

namespace ShaderManager
{
    extern DxCompiler* s_compiler;
    extern DxUtils* s_utils;
    extern DxIncludeHandler* s_includer;
    void InitializeCompiler();
    DxBlob* CreateShader(const ShaderInfo& info);
    void Clear();
};

struct ShaderInputGroup
{
    D3D12_SHADER_BYTECODE vs;
    D3D12_SHADER_BYTECODE ps;
    D3D12_SHADER_BYTECODE cs;
    RootSignature* rootSignature = nullptr;
    std::vector<D3D12_INPUT_ELEMENT_DESC> desc;
};

class PSO;
struct RenderState
{
    RenderState()=default;
    PSO compile(std::wstring name);

    BlendState        blend;
    DepthStencilState ds;
    RasterizerState   rast;
    ShaderInputGroup  sig;
    D3D12_PRIMITIVE_TOPOLOGY_TYPE topology = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    std::vector<DXGI_FORMAT> renderTargets = {};
};

D3D12_SHADER_BYTECODE GetShader(std::wstring name);

class PSO
{
public:
    PSO()=default;
    operator ID3D12PipelineState* ()
    {
        return m_pso;
    }

    // Warning: pointer can have dangling memory after adding new element
    PSO(const RenderState& state);
    PSO(const ShaderInputGroup& sig);

    void reset() {
        if(m_pso)
            m_pso->Release();
        m_pso = nullptr;
    }

public:
    ID3D12PipelineState* m_pso = nullptr;
    D3D12_GRAPHICS_PIPELINE_STATE_DESC m_psoDesc{};
};

inline DXGI_FORMAT getType(u32 type)
{
    if(type == 3)
        return DXGI_FORMAT_R32G32B32_FLOAT;
    if(type == 4)
        return DXGI_FORMAT_R32G32B32A32_FLOAT;
    return DXGI_FORMAT_R32G32_FLOAT;
}

inline D3D12_INPUT_ELEMENT_DESC getInputElement(const char* name, 
                              u32& offset, 
                              u32 type)
{
    u32 oldOffset = offset * sizeof(float);
    offset += type;
    return {
        name,
        0,
        getType(type), 0, oldOffset,  
        D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0};
}    

};
#endif
