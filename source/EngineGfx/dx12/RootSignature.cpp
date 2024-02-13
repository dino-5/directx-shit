#include "RootSignature.h"
#include "Device.h"
#include "EngineCommon/util/Util.h"
#include "EngineCommon/util/Logger.h"
#include <format>

namespace engine::graphics
{

	DescriptorRange::DescriptorRange(DescriptorRangeType type, uint numberOfDescriptors, uint baseShaderRegister,
		uint registerSpace, uint offset)
	{
		m_range.BaseShaderRegister = baseShaderRegister;
		m_range.NumDescriptors = numberOfDescriptors;
		m_range.RegisterSpace = registerSpace;
		m_range.RangeType = static_cast<D3D12_DESCRIPTOR_RANGE_TYPE>(type);
		m_range.OffsetInDescriptorsFromTableStart = offset;
	}
	RootArgument RootArgument::CreateTable(uint numberOfDescriptorRanges, DescriptorRange& range,
		ShaderVisibility vis)
	{
		RootArgument val;
		val.m_argument.ParameterType = CastType(RootArgumentType::TABLE);
		val.m_argument.ShaderVisibility = CastType(vis);
		val.m_argument.DescriptorTable.NumDescriptorRanges = 1;
		val.m_argument.DescriptorTable.pDescriptorRanges = &range.m_range;
		return val;
	}

	RootArgument RootArgument::CreateConstants(UINT num32BitValues,
		UINT shaderRegister,
		UINT registerSpace,
		ShaderVisibility visibility)
	{
		RootArgument val;
		val.m_argument.Constants.Num32BitValues = num32BitValues;
		val.m_argument.Constants.ShaderRegister= shaderRegister;
		val.m_argument.Constants.RegisterSpace = registerSpace;
		val.m_argument.ShaderVisibility = CastType(visibility);
		val.m_argument.ParameterType = CastType(RootArgumentType::CONSTANT);
		return val;
	}

	RootArgument RootArgument::CreateCBV(uint shaderRegister, uint registerSpace ,
		ShaderVisibility vis)
	{
		RootArgument val;
		val.m_argument.ParameterType = CastType(RootArgumentType::CBV);
		val.m_argument.Descriptor.RegisterSpace = registerSpace;
		val.m_argument.Descriptor.ShaderRegister = shaderRegister;
		val.m_argument.ShaderVisibility = CastType(vis);
		return val;
	}

	std::vector<D3D12_ROOT_PARAMETER> GetArguments(RootArguments& vec)
	{
		std::vector<D3D12_ROOT_PARAMETER> result(vec.size());
		for (int i = 0; i < vec.size(); i++)
			result[i] = vec[i];
		return result;
	}

	D3D12_STATIC_SAMPLER_DESC GetSampler()
	{
		D3D12_STATIC_SAMPLER_DESC sampler = {};
		sampler.Filter = D3D12_FILTER_MIN_MAG_MIP_POINT;
		sampler.AddressU = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
		sampler.AddressV = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
		sampler.AddressW = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
		sampler.MipLODBias = 0;
		sampler.MaxAnisotropy = 0;
		sampler.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
		sampler.BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
		sampler.MinLOD = 0.0f;
		sampler.MaxLOD = D3D12_FLOAT32_MAX;
		sampler.ShaderRegister = 0;
		sampler.RegisterSpace = 0;
		sampler.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
		return sampler;
	}

	RootSignature::RootSignature(ID3D12Device* device, RootArguments& arguments)
	{
		auto sampler = GetSampler();
		auto rootArguments = GetArguments(arguments);
		CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc(rootArguments.size(), rootArguments.data(), 1, &sampler,
			D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

		ComPtr<ID3DBlob> serializedRootSig = nullptr;
		ComPtr<ID3DBlob> errorBlob = nullptr;
		HRESULT hr = D3D12SerializeRootSignature(&rootSigDesc, D3D_ROOT_SIGNATURE_VERSION_1,
			serializedRootSig.GetAddressOf(), errorBlob.GetAddressOf());

		if(errorBlob != nullptr)
		{
			::OutputDebugStringA((char*)errorBlob->GetBufferPointer());
		}
		ThrowIfFailed(hr);

		ThrowIfFailed(device->CreateRootSignature(
			0,
			serializedRootSig->GetBufferPointer(),
			serializedRootSig->GetBufferSize(),
			IID_PPV_ARGS(&m_rootSignature)));
	}

	RootSignature::RootSignature(ID3D12Device* device)
	{
		CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc(0, nullptr, 0, nullptr,
			D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

		ComPtr<ID3DBlob> serializedRootSig = nullptr;
		ComPtr<ID3DBlob> errorBlob = nullptr;
		HRESULT hr = D3D12SerializeRootSignature(&rootSigDesc, D3D_ROOT_SIGNATURE_VERSION_1,
			serializedRootSig.GetAddressOf(), errorBlob.GetAddressOf());

		if(errorBlob != nullptr)
		{
			::OutputDebugStringA((char*)errorBlob->GetBufferPointer());
		}
		ThrowIfFailed(hr);

		ThrowIfFailed(device->CreateRootSignature(
			0,
			serializedRootSig->GetBufferPointer(),
			serializedRootSig->GetBufferSize(),
			IID_PPV_ARGS(&m_rootSignature)));
	}

	void RootSignature::AddEntry(RootSignatureType type, RootSignature obj)
	{
		static bool fl = false;
		if (!fl)
		{
			allRootSignatures.resize(ROOT_SIG_COUNT);
			fl = true;
		}
		allRootSignatures[type] = obj;
	}

	void PopulateRootSignatures(ID3D12Device* device)
	{
		LogScope("RootSignature");
		// empty
		{
			RootSignature::AddEntry(ROOT_SIG_EMPTY, RootSignature(device));
		}
		// one constant buffers
		{
			RootArguments arguments;
			arguments.push_back(RootArgument::CreateCBV(0, 0, ShaderVisibility::VERTEX));
			RootSignature::AddEntry(ROOT_SIG_ONE_CONST, RootSignature(device, arguments));
		}
        engine::util::PrintInfo("successfuly created root signatures");

	}


};

