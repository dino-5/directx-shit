#include "PSO.h"
#include "RootSignature.h"
#include "Device.h"
#include "EngineCommon/util/Util.h"
#include "EngineCommon/util/Logger.h"
#include "EngineCommon/System/Filesystem.h"
#include <dxcapi.h>
#include "d3d12shader.h"
#include <format>
#include <cstring>
#include <string>
#include <locale>
#include <codecvt>
#undef memcpy

namespace engine::graphics
{

    D3D12_SHADER_BYTECODE PSO::GetShader(std::wstring name)
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
        auto desc = *util::FindElement(ShaderManager::allDescriptions, shader.vertexShader);
        m_psoDesc.InputLayout = D3D12_INPUT_LAYOUT_DESC{desc.data(), static_cast<u32>(desc.size())};
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

    PSO* PSO::CreatePSO(std::wstring name, ID3D12Device* device, ShaderInputGroup shader, BlendState blendState, DepthStencilState dsState, RasterizerState rasterState)
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

    PSO* RenderState::Compile(std::wstring name)
    {
        return PSO::CreatePSO(name, Device::device->GetDevice(), m_shader, m_blend, m_ds, m_rast);
    }

    std::wstring GetShaderTypeString(ShaderType type)
    {
        switch (type)
        {
        case ShaderType::VERTEX:
            return L"vs_6_6";
        case ShaderType::PIXEL:
            return L"ps_6_6";
        case ShaderType::COMPUTE:
            return L"cs_6_6";
        default:
            return L"wrong shit";
        }
    }

    namespace ShaderManager
    {

		std::vector< TableEntry< DxBlob*>> allShaders;
		std::vector< TableEntry<std::vector<D3D12_INPUT_ELEMENT_DESC> >> allDescriptions;
		DxCompiler* s_compiler = nullptr;
		DxUtils* s_utils = nullptr;
		DxIncludeHandler* s_includer = nullptr;
        void InitializeCompiler()
        {
            ThrowIfFailed(::DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&s_compiler)));
            ThrowIfFailed(::DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&s_utils)));
            s_utils->CreateDefaultIncludeHandler(&s_includer);
        }

		void CreateShader(ShaderInfo info)
		{
            UINT compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;

            IDxcBlobEncoding* sourceBlob;
            s_utils->LoadFile(info.path.c_str(), nullptr, &sourceBlob);
            DxcBuffer sourceBuffer;
            sourceBuffer.Ptr = sourceBlob->GetBufferPointer();
            sourceBuffer.Size = sourceBlob->GetBufferSize();
            BOOL fl;
            sourceBlob->GetEncoding(&fl, &sourceBuffer.Encoding);
            std::wstring type = GetShaderTypeString(info.type);
            
            system::Filepath pdbPath(std::filesystem::absolute(g_shaderDir / L"pdb" / system::Filepath(info.path).wfilename()));
            std::wstring pdbPathWstr = pdbPath.wstr() + info.entryPoint + L".pdb";
            std::vector<const wchar_t*> args= 
            {
                info.shaderName.c_str(),
                L"-E", info.entryPoint.c_str(),
                L"-T", type.c_str(),
                DXC_ARG_DEBUG
            };
            IDxcResult* result;
            HRESULT hr = s_compiler->Compile(
                &sourceBuffer,
                args.data(),
                args.size(),
                s_includer,
                IID_PPV_ARGS(&result)
            );
            if(SUCCEEDED(hr) && result)
                result->GetStatus(&hr);

            if (FAILED(hr))
            {
                if (result)
                {
                    IDxcBlobEncoding* errorBuffer;
                    result->GetErrorBuffer(&errorBuffer);
                    char* str = new char[errorBuffer->GetBufferSize()];
                    std::memcpy(str, errorBuffer->GetBufferPointer(), errorBuffer->GetBufferSize());
                    engine::util::PrintError("{}",std::string(str));
                    delete[] str;
                }
            }
            else
            {
                DxBlob* blob;
                result->GetOutput(DXC_OUT_OBJECT, IID_PPV_ARGS(&blob), nullptr);
                allShaders.push_back({ info.shaderName, blob});

                std::vector<std::byte> pdbData;
                DxBlob* pdbBlob;
                result->GetOutput(DXC_OUT_PDB, IID_PPV_ARGS(&pdbBlob), nullptr);
                auto* data = reinterpret_cast<std::byte*>(pdbBlob->GetBufferPointer());
                size_t size = pdbBlob->GetBufferSize();
                pdbData.assign(data, data + size);

                std::ofstream file(pdbPathWstr.data(), std::ios::binary | std::ios::trunc);
                file.write(reinterpret_cast<const char*>(pdbData.data()), pdbData.size());
                file.close();
            }

		}

		DxBlob* GetShader(std::wstring name)
		{
            return *util::FindElement(allShaders, name);
		}

		void Clear()
		{
			allShaders.clear();
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
        ShaderManager::InitializeCompiler();
        ShaderManager::allShaders.reserve(10);
        {
            ShaderInfo info;
            info.entryPoint = L"VS_Basic";
            info.path = L"Shaders/basic_shader.hlsl";
            info.shaderName = L"VS_Basic";
            info.type = ShaderType::VERTEX;
            ShaderManager::CreateShader(info);

            std::vector<D3D12_INPUT_ELEMENT_DESC> desc = {
                {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0}
            };
            ShaderManager::allDescriptions.push_back({ info.shaderName, desc });
        }

        {
			ShaderInfo info;
			info.entryPoint = L"PS_Basic";
			info.path = L"Shaders/basic_shader.hlsl";
			info.shaderName = L"PS_Basic";
			info.type = ShaderType::PIXEL;
			ShaderManager::CreateShader(info);
        }
        engine::util::PrintInfo("successfuly loaded shaders");
    }

    void PopulatePSO(ID3D12Device* dev)
    {
        LogScope("PSO");
        PSO::allPSO.reserve(0);
        ShaderInputGroup shaderIG{ L"VS_Basic", L"PS_Basic",
            RootSignature::GetRootSignature(RootSignatureType::ROOT_SIG_VERTEX) };
        RenderState state;
        DepthState depthState;
        depthState.depthFunc = ComparisonFunc::LE; 
        DepthStencilState depthStencilState(depthState, StencilState());
        state.SetDepthStencilState(depthStencilState);
        state.SetShaderInputGroup(shaderIG);
        state.Compile(L"default");
        engine::util::PrintInfo("successfuly created pso");
    }
};
