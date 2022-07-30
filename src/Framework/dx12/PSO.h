#ifndef PSO_H
#define PSO_H

#include<d3d12.h>
#include "../include/common.h"
#include <d3d12.h>
#include "../include/d3dx12.h"
#include <string>
#include <vector>

class InputLayout;
class BlendState;
class DepthStencilState;
class Shader;
class RootSignature;

struct ShaderInfo
{
	std::wstring vertexShader;
	std::wstring pixelShader;
};
enum class ShaderType
{
	VERTEX,
	PIXEL
};

class PSO
{
public:
	PSO()=default;
	PSO(ShaderInfo shaders, InputLayout layout, RootSignature rootSignature ,
		BlendState blendState , DepthStencilState dsState );
	void SetBlendState       (BlendState&);
	void SetDepthStencilState(DepthStencilState&);
	void SetInputLayout      (InputLayout&);
	void SetVertexShader     (Shader&);
	void SetPixelShader      (Shader&);
	void SetRootSignature    (RootSignature&);

private:
	D3D12_SHADER_BYTECODE GetShader(ComPtr<ID3D10Blob> blob,std::wstring info, std::string name);
	D3D12_SHADER_BYTECODE GetVertexShader(ShaderInfo);
	D3D12_SHADER_BYTECODE GetPixelShader(ShaderInfo);

public:
	ComPtr<ID3D12PipelineState> m_pso;
	ComPtr<ID3DBlob> VS;
	ComPtr<ID3DBlob> PS;
	DXGI_FORMAT backBufferFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
	DXGI_FORMAT dsvBufferFormat  = DXGI_FORMAT_D24_UNORM_S8_UINT;
	uint msaaQuality = 0;
	bool msaaState = false;
	
};

#endif
