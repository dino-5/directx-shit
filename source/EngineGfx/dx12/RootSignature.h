#ifndef ROOT_SIGNATURE_H
#define ROOT_SIGNATURE_H

#include <d3d12.h>
#include <variant>
#include <array>
#include "EngineCommon/include/common.h"
#include "EngineCommon/include/types.h"
#include "EngineGfx/dx12/d3dx12.h"
#include "third_party/magic_enum/include/magic_enum.hpp"

using namespace magic_enum::bitwise_operators;

namespace engine::graphics
{
	enum RootSignatureType {
		ROOT_SIG_EMPTY,
		ROOT_SIG_ONE_CONST,
		ROOT_SIG_BINDLESS,
		ROOT_SIG_TWO_CONSTANTS,
		ROOT_SIG_COUNT
	};
	constexpr const u32 rootSignatureCount = castEnum(ROOT_SIG_COUNT);

	enum class DescriptorRangeType
	{
		SRV = D3D12_DESCRIPTOR_RANGE_TYPE_SRV,
		UAV = D3D12_DESCRIPTOR_RANGE_TYPE_UAV,
		CBV = D3D12_DESCRIPTOR_RANGE_TYPE_CBV,
		SAMPLER = D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER,
	};

	class DescriptorRange
	{
	public:
		DescriptorRange() = default;
		DescriptorRange(DescriptorRangeType type, uint numberOfDescriptors, uint baseShaderRegister = 0,
			uint registerSpace = 0, uint offset = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND);

		operator D3D12_DESCRIPTOR_RANGE()const { return m_range; }

	public:
		D3D12_DESCRIPTOR_RANGE m_range;
	};

	enum class RootParameterType
	{
		TABLE = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE,
		CONSTANT = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS,
		CBV = D3D12_ROOT_PARAMETER_TYPE_CBV,
		SRV = D3D12_ROOT_PARAMETER_TYPE_SRV,
		UAV = D3D12_ROOT_PARAMETER_TYPE_UAV,
	};

	enum class ShaderVisibility
	{
		ALL = D3D12_SHADER_VISIBILITY_ALL,
		VERTEX = D3D12_SHADER_VISIBILITY_VERTEX,
		GEOMETRY = D3D12_SHADER_VISIBILITY_GEOMETRY,
		PIXEL = D3D12_SHADER_VISIBILITY_PIXEL,
	};

	class RootParameter
	{
	public:
		RootParameter() = default;
		static RootParameter CreateTable(uint numberOfDescriptorRanges, DescriptorRange& range,
			ShaderVisibility vis = ShaderVisibility::PIXEL);
		static RootParameter CreateDescriptor(uint shaderRegiste, uint registerSpace = 0, RootParameterType type = RootParameterType::CBV,
			ShaderVisibility vis = ShaderVisibility::ALL);
		static RootParameter CreateConstants(UINT num32BitValues,
			UINT shaderRegister,
			UINT registerSpace = 0,
			ShaderVisibility visibility = ShaderVisibility::ALL);
		operator D3D12_ROOT_PARAMETER() const { return m_argument; }

	private:
		static D3D12_ROOT_PARAMETER_TYPE CastType(RootParameterType type)
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

	using RootParameters = std::vector<RootParameter>;
	std::vector<D3D12_ROOT_PARAMETER> GetParameters(RootParameters& vec);

	enum class RootSignatureFlags{
        ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT,
        SBV_SRV_HEAP_DIRECT_INDEX = D3D12_ROOT_SIGNATURE_FLAG_CBV_SRV_UAV_HEAP_DIRECTLY_INDEXED,
        SAMPLER_HEAP_DIRECTLY_INDEXED = D3D12_ROOT_SIGNATURE_FLAG_SAMPLER_HEAP_DIRECTLY_INDEXED,
	};

	class RootSignature
	{
	public:
		RootSignature()=default;
		RootSignature(ID3D12Device* device);
		RootSignature(ID3D12Device* device, RootParameters& arguments, RootSignatureFlags flags = RootSignatureFlags::ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);
		operator ID3D12RootSignature* ()
		{
			return m_rootSignature;
		}

		static inline std::vector<RootSignature> allRootSignatures;
		static void AddEntry(RootSignatureType, RootSignature);
		static RootSignature* GetRootSignature(RootSignatureType type)
		{
			return &allRootSignatures[type];
		}

	public:
		ID3D12RootSignature* m_rootSignature;
	};

	void PopulateRootSignatures(ID3D12Device* device);

};
#endif