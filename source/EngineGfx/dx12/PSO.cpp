#include "PSO.h"
#include "RootSignature.h"
#include "Device.h"
#include "EngineCommon/util/Util.h"
#include "EngineCommon/util/Logger.h"
#include "EngineCommon/System/Filesystem.h"
#include "EngineCommon/System/config.h"
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

    D3D12_SHADER_BYTECODE getShader(DxBlob* blob)
    {
        D3D12_SHADER_BYTECODE ret
        {
            reinterpret_cast<BYTE*>(blob->GetBufferPointer()),
            blob->GetBufferSize()
        };
        return ret;
    }

    D3D12_SHADER_BYTECODE GetShader(std::wstring name)
    {
        auto shader = ShaderManager::GetShader(name);
        return getShader(shader);
    }

    PSO::PSO(ID3D12Device* device, ShaderInputGroup shader, BlendState blendState, DepthStencilState dsState, 
        RasterizerState rasterState, const std::vector<DXGI_FORMAT> renderTargets)
    {
        m_psoDesc.InputLayout = shader.desc;
        m_psoDesc.pRootSignature = *shader.rootSignature;
        m_psoDesc.VS = shader.vertexShader;
        m_psoDesc.PS = shader.pixelShader;
        m_psoDesc.RasterizerState = rasterState;
        m_psoDesc.BlendState = blendState;
        m_psoDesc.DepthStencilState = dsState;
        m_psoDesc.SampleMask = UINT_MAX;
        m_psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
        if (renderTargets.size() >= 1)
        {
            m_psoDesc.NumRenderTargets = renderTargets.size();
            uint index = 0;
            for(auto& format : renderTargets)
                m_psoDesc.RTVFormats[index++] = format;
        }
        else
        {
            m_psoDesc.NumRenderTargets = 1;
            m_psoDesc.RTVFormats[0] = backBufferFormat;
        }
        m_psoDesc.SampleDesc.Count =  1;
        m_psoDesc.SampleDesc.Quality = 0;
        m_psoDesc.DSVFormat = dsvBufferFormat;
        ThrowIfFailed(device->CreateGraphicsPipelineState(&m_psoDesc, IID_PPV_ARGS(&m_pso)));
    }

    PSO* PSO::CreatePSO(std::wstring name, ID3D12Device* device, ShaderInputGroup shader, BlendState blendState, DepthStencilState dsState,
        RasterizerState rasterState, const std::vector<DXGI_FORMAT> renderTargets)
    {
        if (GetPSO(name) == nullptr)
            allPSO.push_back({ name, PSO(device, shader, blendState, dsState, rasterState, renderTargets) });
        return &(allPSO.back().second);
    }

    void RenderState::setBlendState(BlendState blendState)
    {
        m_blend = blendState;
    }

    void RenderState::setDepthStencilState(DepthStencilState ds)
    {
        m_ds = ds;
    }

    void RenderState::setRasterizerState(RasterizerState rast)
    {
        m_rast = rast;
    }

    void RenderState::setShaderInputGroup(ShaderInputGroup& shader)
    {
        m_shader = shader;
    }

    PSO PSO::CreatePSO(const RenderState& state)
    {
        return PSO(Device::device->getDevice(), state.m_shader, state.m_blend, state.m_ds, state.m_rast, state.renderTargets);
    }

    PSO* RenderState::compile(std::wstring name)
    {
        return PSO::CreatePSO(name, Device::device->getDevice(), m_shader, m_blend, m_ds, m_rast);
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

    ShaderInfo::~ShaderInfo()
    {
        auto obj = ShaderManager::CreateShader(*this);
        onDestoy(obj);
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


		TableEntry< DxBlob*> CreateShader(const ShaderInfo& info)
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
            
            auto path = config::g_state.shaderDir / L"pdb" / system::Filepath(info.path).filename();
            system::Filepath pdbPath(std::filesystem::absolute(path.getPath()));
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
                static_cast<UINT32>(args.size()),
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
                    engine::util::printError("{}",std::string(str));
                    delete[] str;
                }
                return { L"", nullptr };
            }
            else
            {
                DxBlob* blob;
                result->GetOutput(DXC_OUT_OBJECT, IID_PPV_ARGS(&blob), nullptr);

                std::vector<std::byte> pdbData;
                DxBlob* pdbBlob;
                result->GetOutput(DXC_OUT_PDB, IID_PPV_ARGS(&pdbBlob), nullptr);
                auto* data = reinterpret_cast<std::byte*>(pdbBlob->GetBufferPointer());
                size_t size = pdbBlob->GetBufferSize();
                pdbData.assign(data, data + size);

                std::ofstream file(pdbPathWstr.data(), std::ios::binary | std::ios::trunc);
                file.write(reinterpret_cast<const char*>(pdbData.data()), pdbData.size());
                file.close();
                return { info.shaderName, blob};
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
        desc.CS = GetShader(shaderGroup.computeShader);
        desc.pRootSignature = *shaderGroup.rootSignature;

        ThrowIfFailed(Device::device->getDevice()->CreateComputePipelineState(&desc, IID_PPV_ARGS(&m_pso)));
    }

    void PopulateShaders()
    {
        LogScope("Shaders");
        ShaderManager::InitializeCompiler();
        ShaderManager::allShaders.reserve(10);

        std::vector<D3D12_INPUT_ELEMENT_DESC> desc = {
            {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
            {"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
            {"UV", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
        };
        ShaderManager::allDescriptions.push_back({ L"VS_Basic", desc });

        auto deleter = [](TableEntry<DxBlob*>& tableEntry)
        {
            if(tableEntry.second != nullptr)
                ShaderManager::allShaders.push_back(tableEntry);
        };
        {
            ShaderInfo info(deleter);
            info.entryPoint = L"VS_Basic";
            info.path = L"Shaders/basic_shader.hlsl";
            info.shaderName = L"VS_Basic";
            info.type = ShaderType::VERTEX;

        }
        {
            ShaderInfo info(deleter);
			info.entryPoint = L"PS_Basic";
			info.path = L"Shaders/basic_shader.hlsl";
			info.shaderName = L"PS_Basic";
			info.type = ShaderType::PIXEL;
        }
        engine::util::printInfo("successfuly loaded shaders");
    }

    void PopulatePSO(ID3D12Device* dev)
    {
        LogScope("PSO");
        PSO::allPSO.reserve(0);
        auto descElements = *util::FindElement(ShaderManager::allDescriptions, L"VS_Basic");
        ShaderInputGroup shaderIG{ {descElements.data(), descElements.size()}, 
            GetShader(L"VS_Basic"), GetShader(L"PS_Basic"),
            RootSignature::GetRootSignature(RootSignatureType::ROOT_SIG_VERTEX) };
        RenderState state;
        DepthState depthState;
        depthState.depthFunc = ComparisonFunc::LE; 
        DepthStencilState depthStencilState(depthState, StencilState());
        state.setDepthStencilState(depthStencilState);
        state.setShaderInputGroup(shaderIG);
        state.setRasterizerState(RasterizerState(CullMode::BACK, false));
        state.compile(L"default");
        engine::util::printInfo("successfuly created pso");
    }
};
