#pragma once
#include "include/common.h"
#include "DescriptorHeap.h"
#include "external/magic_enum/include/magic_enum.hpp"
#include "Framework/include/types.h"

namespace engine::graphics
{
	enum class ResourceState
	{
		COMMON = D3D12_RESOURCE_STATE_COMMON,
		VERTEX_CONSTANT_BUFFER = D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER,
		INDEX_BUFFER = D3D12_RESOURCE_STATE_INDEX_BUFFER,
		RENDER_TARGET = D3D12_RESOURCE_STATE_RENDER_TARGET,
		UNORDERED_ACCESS = D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
		DEPTH_WRITE = D3D12_RESOURCE_STATE_DEPTH_WRITE,
		DEPTH_READ = D3D12_RESOURCE_STATE_DEPTH_READ,
		COPY_DEST = D3D12_RESOURCE_STATE_COPY_DEST,
		COPY_SOURCE = D3D12_RESOURCE_STATE_COPY_SOURCE,
		PIXEL_SHADER_RESOURCE = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
		GENERIC_READ_STATE = D3D12_RESOURCE_STATE_GENERIC_READ,
		PRESENT = D3D12_RESOURCE_STATE_PRESENT,
	};

	enum class ResourceDescriptorFlags : std::uint32_t
	{
		RenderTarget = 1 << 0,
		DepthStencil = 1 << 1,
		ShaderResource = 1 << 2,
		UnorderedAccess = 1 << 3,
	};
	using namespace magic_enum::bitwise_operators;

	D3D12_RESOURCE_STATES CastType(ResourceState state);

	struct ResourceDescription
	{
		DXGI_FORMAT format;
		uint width;
		uint height;
		uint depthOrArraySize = 1;
		D3D12_RESOURCE_DIMENSION dimension;
		ResourceFlags flags;
		ResourceState createState = ResourceState::COMMON;
		CD3DX12_HEAP_PROPERTIES heap_type = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
		ResourceDescriptorFlags descriptor;
	};

	class Resource
	{
	private:
		using ResourceVector = std::vector<Resource>;
		static inline std::vector<TableEntry<ResourceVector>> allResources;
	public:
		static void CreateResource(ID3D12Device* device);
		static void PopulateResources(ID3D12Device* device);
		static Resource& GetResource(std::string name, uint numberOfFrame)
		{
			ResourceVector* res = engine::util::FindElement(allResources, name);
			return (*res)[numberOfFrame];
		}

		Resource() = default;

		void InitAsConstantBuffer(ID3D12Device* device, uint sizeOfBuffer);
		Resource(ID3D12Device* device, ResourceDescription desc, D3D12_CLEAR_VALUE* val = nullptr);
		void Init(ID3D12Device* device, ResourceDescription desc, D3D12_CLEAR_VALUE* val = nullptr);

		Resource(ID3D12Device* device, CD3DX12_RESOURCE_DESC desc,
			ResourceState createState = ResourceState::COMMON,
			CD3DX12_HEAP_PROPERTIES heap_type = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT));
		void Init(ID3D12Device* device, CD3DX12_RESOURCE_DESC desc,
			ResourceState createState = ResourceState::COMMON,
			CD3DX12_HEAP_PROPERTIES heap_type = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT));

		void Transition(ID3D12GraphicsCommandList* cmdList, ResourceState state);

		void Reset()
		{
			if (m_resource)
				m_resource->Release();
		}

		operator ID3D12Resource* ()
		{
			return m_resource;
		}

		ID3D12Resource* operator->()
		{
			return m_resource;
		}

		ID3D12Resource** GetAddress() { return &m_resource; }
		ID3D12Resource* GetResource() { return m_resource; }

		void CreateViews(ID3D12Device* device, ResourceDescriptorFlags descriptors);

		DescriptorDSV dsv;
		DescriptorRTV rtv;
		DescriptorSRV srv;
		DescriptorUAV uav;

	private:
		ID3D12Resource* m_resource;
		ResourceState m_currentState = ResourceState::COMMON;
	};

};
