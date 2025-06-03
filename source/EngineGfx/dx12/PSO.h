#ifndef PSO_H
#define PSO_H

#include <d3d12.h>
#include <dxgiformat.h>
#include <string>
#include <vector>
#include <functional>
#include <unordered_map>
#include "PipelineStates.h"
#include "RootSignature.h"
#include "EngineCommon/util/Util.h"

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
    ShaderInfo(Table<DxBlob*>* aShaderTable=nullptr) : shaderTable(aShaderTable) {}
    Table<DxBlob*>* shaderTable;
    std::wstring shaderName{};
    std::wstring path{};
    std::wstring entryPoint{};
    ShaderType   type{};
    ~ShaderInfo();
};

namespace ShaderManager
{
    extern DxCompiler* s_compiler;
    extern DxUtils* s_utils;
    extern DxIncludeHandler* s_includer;
    void InitializeCompiler();
    TableEntry< DxBlob*> CreateShader(const ShaderInfo& info);
    void Clear();
};

struct ShaderInputGroup
{
    D3D12_INPUT_LAYOUT_DESC desc = { nullptr, 0 };
    D3D12_SHADER_BYTECODE vertexShader;
    D3D12_SHADER_BYTECODE pixelShader;
    RootSignature* rootSignature = nullptr;
};

class PSO;
struct RenderState
{
    RenderState()=default;
    PSO compile(std::wstring name);
    void setBlendState       (BlendState blend=BlendState());
    void setDepthStencilState(DepthStencilState ds =DepthStencilState());
    void setRasterizerState  (RasterizerState raster = RasterizerState());
    void setShaderInputGroup (ShaderInputGroup&);

    BlendState        blend;
    DepthStencilState ds;
    RasterizerState   rast;
    ShaderInputGroup  shader;
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

    void reset() {
        if(m_pso)
            m_pso->Release();
        m_pso = nullptr;
    }

public:
    ID3D12PipelineState* m_pso;
    D3D12_GRAPHICS_PIPELINE_STATE_DESC m_psoDesc{};

    DXGI_FORMAT backBufferFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
    DXGI_FORMAT dsvBufferFormat  = DXGI_FORMAT_D24_UNORM_S8_UINT;
};


struct ComputeShaderInputGroup
{
    std::wstring computeShader;
    RootSignature* rootSignature;

    void setCS(std::wstring name) { computeShader= name;  }
    void setRootSignature(RootSignature& r) { rootSignature = &r;  }
};

class ComputePSO
{
public:
    ComputePSO() = default;
    ComputePSO(ComputeShaderInputGroup shaderGroup);
    operator ID3D12PipelineState* ()
    {
        return m_pso;
    }
private:
    ID3D12PipelineState* m_pso=nullptr;
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
