#include "PSO.h"
#include "RootSignature.h"
#include "Device.h"
#include "EngineCommon/util/Util.h"
#include "EngineCommon/util/Logger.h"
#include <format>

namespace engine::graphics
{

    D3D12_SHADER_BYTECODE PSO::GetShader(std::string name)
    {
        auto shader = ShaderManager::GetShader(name);
        D3D12_SHADER_BYTECODE ret
        {
            reinterpret_cast<BYTE*>(shader->GetBufferPointer()),
            shader->GetBufferSize()
        };
        return ret;
    }

    PSO::PSO(ID3D12Device* device, ShaderInputGroup shader, BlendState blendState, DepthStencilState dsState, RasterizerState rasterState)
    {
        m_psoDesc.InputLayout = *shader.layout;
        m_psoDesc.pRootSignature = *shader.rootSignature;
        m_psoDesc.VS = GetShader(shader.vertexShader);
        m_psoDesc.PS = GetShader(shader.pixelShader);
        m_psoDesc.RasterizerState = rasterState;
        m_psoDesc.BlendState = blendState;
        m_psoDesc.DepthStencilState = dsState;
        m_psoDesc.SampleMask = UINT_MAX;
        m_psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
        m_psoDesc.NumRenderTargets = 1;
        m_psoDesc.RTVFormats[0] = backBufferFormat;
        m_psoDesc.SampleDesc.Count =  1;
        m_psoDesc.SampleDesc.Quality = 0;
        m_psoDesc.DSVFormat = dsvBufferFormat;
        ThrowIfFailed(device->CreateGraphicsPipelineState(&m_psoDesc, IID_PPV_ARGS(&m_pso)));
    }

    PSO* PSO::CreatePSO(std::string name, ID3D12Device* device, ShaderInputGroup shader, BlendState blendState, DepthStencilState dsState, RasterizerState rasterState)
    {
        if (GetPSO(name) == nullptr)
            allPSO.push_back({ name, PSO(device, shader, blendState, dsState, rasterState) });
        return &(allPSO.back().second);
    }
    void RenderState::SetBlendState(BlendState blendState)
    {
        m_blend = blendState;
    }

    void RenderState::SetDepthStencilState(DepthStencilState ds)
    {
        m_ds = ds;
    }

    void RenderState::SetRasterizerState(RasterizerState rast)
    {
        m_rast = rast;
    }

    void RenderState::SetShaderInputGroup(ShaderInputGroup& shader)
    {
        m_shader = shader;
    }

    PSO* RenderState::Compile(std::string name)
    {
        return PSO::CreatePSO(name, Device::device->GetDevice(), m_shader, m_blend, m_ds, m_rast);
    }

    std::string GetShaderTypeString(ShaderType type)
    {
        switch (type)
        {
        case ShaderType::VERTEX:
            return "vs_5_0";
        case ShaderType::PIXEL:
            return "ps_5_0";
        case ShaderType::COMPUTE:
            return "cs_5_0";
        default:
            return "wrong shit";
        }
    }

    namespace ShaderManager
    {

		std::vector< TableEntry< ID3DBlob*>> allShaders;
		std::vector< TableEntry< InputLayout>> inputLayouts;
		void CreateShader(ShaderInfo info)
		{
            UINT compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;

            ID3DBlob* blob;
            ThrowIfFailed(D3DCompileFromFile(
                engine::util::to_wstring(info.path).c_str(), nullptr, nullptr, info.entryPoint.c_str(),
                GetShaderTypeString(info.type).c_str(), compileFlags, 0, &blob, nullptr)
            );
            allShaders.push_back({ info.shaderName, blob});
		}

		ID3DBlob* GetShader(std::string name)
		{
            return *util::FindElement(allShaders, name);
		}

		void Clear()
		{
			allShaders.clear();
		}

        void CreateInputLayout(std::string name,std::vector<InputLayoutElement> layout)
        {

			if (GetInputLayout(name)==nullptr) 
			{
				inputLayouts.push_back({ name, InputLayout(layout)});
			}
        }

        InputLayout* GetInputLayout(std::string name)
        {
			return util::FindElement(inputLayouts, name);
        }
    }


    ComputePSO::ComputePSO(ComputeShaderInputGroup shaderGroup)
    {
        D3D12_COMPUTE_PIPELINE_STATE_DESC desc{};
        desc.CS = PSO::GetShader(shaderGroup.computeShader);
        desc.pRootSignature = *shaderGroup.rootSignature;

        ThrowIfFailed(Device::device->GetDevice()->CreateComputePipelineState(&desc, IID_PPV_ARGS(&m_pso)));
    }

    void PopulateShaders()
    {
        LogScope("Shaders");
        ShaderManager::allShaders.reserve(10);
        ShaderManager::inputLayouts.reserve(5);
        {
            ShaderInfo info;
            info.entryPoint = "VS_Basic";
            info.path = "Shaders/basic_shader.hlsl";
            info.shaderName = "VS_Basic";
            info.type = ShaderType::VERTEX;
            ShaderManager::CreateShader(info);

            std::vector<InputLayoutElement> layout = {
                {"POSITION", Format::float3}
            };
            ShaderManager::CreateInputLayout("defaultLayout", layout);
        }

        {
			ShaderInfo info;
			info.entryPoint = "PS_Basic";
			info.path = "Shaders/basic_shader.hlsl";
			info.shaderName = "PS_Basic";
			info.type = ShaderType::PIXEL;
			ShaderManager::CreateShader(info);
        }
        engine::util::logInfo("successfuly loaded shaders");
    }

    void PopulatePSO(ID3D12Device* dev)
    {
        LogScope("PSO");
        PSO::allPSO.reserve(0);
        ShaderInputGroup shaderIG{ "VS_Basic", "PS_Basic", ShaderManager::GetInputLayout("defaultLayout"),
            RootSignature::GetRootSignature("oneConst") };
        RenderState state;
        state.SetShaderInputGroup(shaderIG);
        state.Compile("default");
        engine::util::logInfo("successfuly created pso");
    }
};
