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

namespace engine::graphics
{

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

	namespace ShaderManager
	{
		extern std::vector< TableEntry< ID3DBlob*>> allShaders;
		void CreateShader(ShaderInfo info);
		ID3DBlob* GetShader(std::string name);
		void Clear();

		extern std::vector< TableEntry< InputLayout>> inputLayouts;
		void CreateInputLayout(std::string name, std::vector<InputLayoutElement> layout);
		InputLayout* GetInputLayout(std::string name);
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

	class PSO;
	class RenderState
	{
	public:
		RenderState()=default;
		PSO* Compile(std::string name);
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

	class PSO
	{
	public:
		PSO()=default;
		operator ID3D12PipelineState* ()
		{
			return m_pso.Get();
		}

		// Warning: pointer can have dangling memory after adding new element
		static PSO* CreatePSO(std::string name, ID3D12Device* device, ShaderInputGroup shader , BlendState blendState , DepthStencilState dsState , RasterizerState rasterState);
		static PSO* GetPSO(std::string name)
		{
			return util::FindElement(allPSO, name);
		}
		static D3D12_SHADER_BYTECODE GetShader(std::string name);
		static inline std::vector<TableEntry<PSO>> allPSO;

		void Reset() { m_pso.Reset(); }


	private:
		PSO(ID3D12Device* device, ShaderInputGroup shader , BlendState blendState , DepthStencilState dsState , RasterizerState rasterState);

	public:
		ComPtr<ID3D12PipelineState> m_pso;
		D3D12_GRAPHICS_PIPELINE_STATE_DESC m_psoDesc{};

		DXGI_FORMAT backBufferFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
		DXGI_FORMAT dsvBufferFormat  = DXGI_FORMAT_D24_UNORM_S8_UINT;
		
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

	void PopulateShaders();
	void PopulatePSO(ID3D12Device* dev);
};
#endif
