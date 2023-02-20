#ifndef PSO_H
#define PSO_H

#include<d3d12.h>
#include "Framework/include/common.h"
#include <d3d12.h>
#include "Framework/include/d3dx12.h"
#include <string>
#include <vector>
#include <unordered_map>
#include "PipelineStates.h"
#include "RootSignature.h"

enum class ShaderType
{
	VERTEX,
	PIXEL,
	COMPUTE
};
std::string GetShaderTypeString(ShaderType type);

struct ShaderInfo
{
	std::string shaderName;
	std::string path;
	std::string entryPoint;
	ShaderType  type;
};


class Shader
{
public:
	Shader() = default;
	Shader(ShaderInfo);
	static void CreateShader(ShaderInfo info);
	static ID3DBlob* GetShader(std::string name)
	{
		if (FindShader(name))
			return m_shaders[name].Get();
		return nullptr;
	}
	static void Clear()
	{
		m_shaders.clear();
	}

private:
	static bool FindShader(std::string name)
	{
		return m_shaders.find(name) != m_shaders.end();
	}

public:
	static inline std::unordered_map < std::string, ComPtr<ID3DBlob>> m_shaders;
	std::string m_name;
	
	ComPtr<ID3DBlob> m_shader;
	
};

struct ShaderInputGroup
{
	std::string vertexShader;
	std::string pixelShader;
	InputLayout* layout;
	RootSignature* rootSignature;

	void SetVS(std::string name) { vertexShader = name;  }
	void SetPS(std::string name) { pixelShader= name;  }
	void SetLayout(InputLayout& l) { layout = &l;  }
	void SetRootSignature(RootSignature& r) { rootSignature = &r;  }
};


class PSO
{
public:
	PSO()=default;
	PSO(ID3D12Device* device, ShaderInputGroup shader , BlendState blendState , DepthStencilState dsState , RasterizerState rasterState);
	operator ID3D12PipelineState* ()
	{
		return m_pso.Get();
	}

	void Reset() { m_pso.Reset(); }
	~PSO() { Reset(); }

	static D3D12_SHADER_BYTECODE GetShader(std::string name);

public:
	ComPtr<ID3D12PipelineState> m_pso;
	D3D12_GRAPHICS_PIPELINE_STATE_DESC m_psoDesc{};

	DXGI_FORMAT backBufferFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
	DXGI_FORMAT dsvBufferFormat  = DXGI_FORMAT_D24_UNORM_S8_UINT;
	uint msaaQuality = 0;
	bool msaaState = false;
	
};

class RenderState
{
public:
	RenderState()=default;
	PSO Compile();
	void SetBlendState       (BlendState blend=BlendState());
	void SetDepthStencilState(DepthStencilState ds =DepthStencilState());
	void SetRasterizerState  (RasterizerState raster = RasterizerState());
	void SetShaderInputGroup (ShaderInputGroup&);

public:
	BlendState        m_blend;
	DepthStencilState m_ds;
	RasterizerState   m_rast;
	ShaderInputGroup  m_shader;
};

struct ComputeShaderInputGroup
{
	std::string computeShader;
	RootSignature* rootSignature;

	void SetCS(std::string name) { computeShader= name;  }
	void SetRootSignature(RootSignature& r) { rootSignature = &r;  }
};


class ComputePSO
{
public:
	ComputePSO() = default;
	ComputePSO(ComputeShaderInputGroup shaderGroup);
	operator ID3D12PipelineState* ()
	{
		return m_pso.Get();
	}


private:
	ComPtr<ID3D12PipelineState> m_pso=nullptr;
};

#endif
