#ifndef PSO_H
#define PSO_H

#include<d3d12.h>
#include "../include/common.h"
#include <d3d12.h>
#include "../include/d3dx12.h"
#include <string>
#include <vector>
#include <unordered_map>

class InputLayout;
class BlendState;
class DepthStencilState;
class Shader;
class RootSignature;
class RasterizerState;

enum class ShaderType
{
	VERTEX,
	PIXEL
};

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

class PSO
{
public:
	PSO()=default;
	PSO(std::string vertexShader, std::string pixelShader, InputLayout layout, RootSignature rootSignature ,
		BlendState blendState , DepthStencilState dsState , RasterizerState rasterState);
	void SetBlendState       (BlendState&);
	void SetDepthStencilState(DepthStencilState&);
	void SetInputLayout      (InputLayout&);
	void SetVertexShader     (Shader&);
	void SetPixelShader      (Shader&);
	void SetRootSignature    (RootSignature&);

	operator ID3D12PipelineState* ()
	{
		return m_pso.Get();
	}

	operator D3D12_GRAPHICS_PIPELINE_STATE_DESC()
	{
		return m_psoDesc;
	}
		

private:
	D3D12_SHADER_BYTECODE GetShader(std::string name);

public:
	ComPtr<ID3D12PipelineState> m_pso;
	D3D12_GRAPHICS_PIPELINE_STATE_DESC m_psoDesc{};
	ComPtr<ID3DBlob> VS;
	ComPtr<ID3DBlob> PS;
	DXGI_FORMAT backBufferFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
	DXGI_FORMAT dsvBufferFormat  = DXGI_FORMAT_D24_UNORM_S8_UINT;
	uint msaaQuality = 0;
	bool msaaState = false;
	
};

#endif
