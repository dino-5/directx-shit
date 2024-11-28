#ifndef PSO_H
#define PSO_H

#include <d3d12.h>
#include <dxgiformat.h>
#include "EngineGfx/dx12/d3dx12.h"
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
		ShaderInfo() = default;
		ShaderInfo(std::function<void(TableEntry<DxBlob*>&)> deleter) : onDestoy(deleter) {}
		std::function<void(TableEntry<DxBlob*>&)> onDestoy;
		std::wstring shaderName{};
		std::wstring path{};
		std::wstring entryPoint{};
		D3D12_INPUT_LAYOUT_DESC desc{};
		ShaderType  type{};
		~ShaderInfo();
	};

	namespace ShaderManager
	{
		extern std::vector< TableEntry< DxBlob*>> allShaders;
		extern std::vector< TableEntry<std::vector<D3D12_INPUT_ELEMENT_DESC> >> allDescriptions;
		extern DxCompiler* s_compiler;
		extern DxUtils* s_utils;
		extern DxIncludeHandler* s_includer;
        void InitializeCompiler();
		inline void Reset()
		{
			allShaders.clear();
			allDescriptions.clear();
		}
		TableEntry< DxBlob*> CreateShader(const ShaderInfo& info);
		DxBlob* GetShader(std::wstring name);
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
		PSO* compile(std::wstring name);
		void setBlendState       (BlendState blend=BlendState());
		void setDepthStencilState(DepthStencilState ds =DepthStencilState());
		void setRasterizerState  (RasterizerState raster = RasterizerState());
		void setShaderInputGroup (ShaderInputGroup&);

		BlendState        m_blend;
		DepthStencilState m_ds;
		RasterizerState   m_rast;
		ShaderInputGroup  m_shader;
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
		PSO(ID3D12Device* device, ShaderInputGroup shader , BlendState blendState , DepthStencilState dsState , 
			RasterizerState rasterState, std::vector<DXGI_FORMAT> renderTargets = {});
		static PSO CreatePSO(const RenderState& state);
		static PSO* CreatePSO(std::wstring name, ID3D12Device* device, ShaderInputGroup shader , BlendState blendState , DepthStencilState dsState , 
			RasterizerState rasterState, std::vector<DXGI_FORMAT> renderTargets = {});
		static PSO* GetPSO(std::wstring name)
		{
			return util::FindElement(allPSO, name);
		}
		static inline Table<PSO> allPSO;

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

	void PopulateShaders();
	void PopulatePSO(ID3D12Device* dev);
};
#endif
