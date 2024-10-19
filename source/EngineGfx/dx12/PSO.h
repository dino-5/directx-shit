#ifndef PSO_H
#define PSO_H

#include<d3d12.h>
#include <d3d12.h>
#include "EngineGfx/dx12/d3dx12.h"
#include <string>
#include <vector>
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

	enum class ShaderType
	{
		VERTEX,
		PIXEL,
		COMPUTE
	};
	std::wstring GetShaderTypeString(ShaderType type);

	struct ShaderInfo
	{
		std::wstring shaderName;
		std::wstring path;
		std::wstring entryPoint;
		D3D12_INPUT_LAYOUT_DESC desc;
		ShaderType  type;
	};

	namespace ShaderManager
	{
		extern std::vector< TableEntry< DxBlob*>> allShaders;
		extern std::vector< TableEntry<std::vector<D3D12_INPUT_ELEMENT_DESC> >> allDescriptions;
		extern DxCompiler* s_compiler;
		extern DxUtils* s_utils;
		extern DxIncludeHandler* s_includer;
        void InitializeCompiler();
		void CreateShader(ShaderInfo info);
		DxBlob* GetShader(std::wstring name);
		void Clear();
	};

	struct ShaderInputGroup
	{
		std::wstring vertexShader;
		std::wstring pixelShader;
		RootSignature* rootSignature = nullptr;

		void setVS(std::wstring name) { vertexShader = name;  }
		void setPS(std::wstring name) { pixelShader= name;  }
		void setRootSignature(RootSignature& r) { rootSignature = &r;  }
	};

	class PSO;
	class RenderState
	{
	public:
		RenderState()=default;
		PSO* compile(std::wstring name);
		void setBlendState       (BlendState blend=BlendState());
		void setDepthStencilState(DepthStencilState ds =DepthStencilState());
		void setRasterizerState  (RasterizerState raster = RasterizerState());
		void setShaderInputGroup (ShaderInputGroup&);

	public:
		BlendState        m_blend;
		DepthStencilState m_ds;
		RasterizerState   m_rast;
		ShaderInputGroup  m_shader;
	};

	class PSO
	{
	public:
		PSO()=default;
		operator ID3D12PipelineState* ()
		{
			return m_pso;
		}

		// Warning: pointer can have dangling memory after adding new element
		static PSO* CreatePSO(std::wstring name, ID3D12Device* device, ShaderInputGroup shader , BlendState blendState , DepthStencilState dsState , RasterizerState rasterState);
		static PSO* GetPSO(std::wstring name)
		{
			return util::FindElement(allPSO, name);
		}
		static D3D12_SHADER_BYTECODE GetShader(std::wstring name);
		static inline std::vector<TableEntry<PSO>> allPSO;

		void reset() {
			if(m_pso)
                m_pso->Release();
			m_pso = nullptr;
		}


	private:
		PSO(ID3D12Device* device, ShaderInputGroup shader , BlendState blendState , DepthStencilState dsState , RasterizerState rasterState);

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

	void PopulateShaders();
	void PopulatePSO(ID3D12Device* dev);
};
#endif
