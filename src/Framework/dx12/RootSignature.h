#ifndef ROOT_SIGNATURE_H
#define ROOT_SIGNATURE_H

#include "../include/d3dx12.h"
#include <d3d12.h>
#include <variant>
#include "../include/common.h"

class DescriptorRange
{
public:
	enum class DescriptorRangeType
	{
		SRV = D3D12_DESCRIPTOR_RANGE_TYPE_SRV,
		UAV = D3D12_DESCRIPTOR_RANGE_TYPE_UAV,
		CBV = D3D12_DESCRIPTOR_RANGE_TYPE_CBV,
		SAMPLER = D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER,
	};
	DescriptorRange() = default;
	DescriptorRange(DescriptorRangeType type, uint numberOfDescriptors, uint baseShaderRegister, 
		uint registerSpace=0, uint offset= D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND);

	operator D3D12_DESCRIPTOR_RANGE()const { return m_range; }

public:
	D3D12_DESCRIPTOR_RANGE m_range;
};

class RootArgument
{
public:
	enum class RootArgumentType
	{
        TABLE    = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE,
        CONSTANT = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS,
        CBV      = D3D12_ROOT_PARAMETER_TYPE_CBV	,
        SRV      = D3D12_ROOT_PARAMETER_TYPE_SRV	,
        UAV      = D3D12_ROOT_PARAMETER_TYPE_UAV	,
	};

	enum class ShaderVisibility
	{
        ALL      = D3D12_SHADER_VISIBILITY_ALL	,
        VERTEX   = D3D12_SHADER_VISIBILITY_VERTEX,
        GEOMETRY = D3D12_SHADER_VISIBILITY_GEOMETRY,
        PIXEL    = D3D12_SHADER_VISIBILITY_PIXEL,
	};
	RootArgument() =default;
    static RootArgument CreateTable(uint numberOfDescriptorRanges, DescriptorRange& range,
		ShaderVisibility vis = ShaderVisibility::PIXEL);
	static RootArgument CreateCBV(uint shaderRegiste, uint registerSpace = 0, 
		ShaderVisibility vis = ShaderVisibility::ALL);
	operator D3D12_ROOT_PARAMETER() const { return m_argument; }

private:
	static D3D12_ROOT_PARAMETER_TYPE CastType(RootArgumentType type)
	{
		return static_cast<D3D12_ROOT_PARAMETER_TYPE>(type);
	}

	
	static D3D12_SHADER_VISIBILITY CastType(ShaderVisibility vis)
	{
		return static_cast<D3D12_SHADER_VISIBILITY>(vis);
	}

public:
	D3D12_ROOT_PARAMETER m_argument{};
};

using RootArguments = std::vector<RootArgument>;
std::vector<D3D12_ROOT_PARAMETER> GetArguments(RootArguments& vec);

class RootSignature
{
public:
	RootSignature();
	RootSignature(RootArguments& arguments);
	void Init(RootArguments& arguments);
	operator ID3D12RootSignature* ()
	{
		return m_rootSignature.Get();
	}

public:
	ComPtr<ID3D12RootSignature> m_rootSignature;
};

#endif