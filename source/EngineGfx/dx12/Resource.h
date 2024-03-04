#pragma once
#include "EngineCommon/include/common.h"
#include "DescriptorHeap.h"
#include "third_party/magic_enum/include/magic_enum.hpp"
#include "EngineCommon/include/types.h"
#include "EngineCommon/System/config.h"

#include <array>

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
	D3D12_RESOURCE_STATES CastEnum(ResourceState state);

	enum class ResourceDescriptorFlags : std::uint32_t
	{
		None = 0,
		RenderTarget = 1 << 0,
		DepthStencil = 1 << 1,
		ShaderResource = 1 << 2,
		UnorderedAccess = 1 << 3,
		ConstantBuffer = 1 << 4,
	};
	using namespace magic_enum::bitwise_operators;


	struct ResourceDescription
	{
		DXGI_FORMAT format;
		u32 width;
		u32 height;
		u32 depthOrArraySize = 1;
		D3D12_RESOURCE_DIMENSION dimension;
		ResourceFlags flags = ResourceFlags::NONE;
		ResourceState createState = ResourceState::COMMON;
		D3D12_HEAP_TYPE  heapType = D3D12_HEAP_TYPE_DEFAULT;
		ResourceDescriptorFlags descriptor;
		D3D12_SRV_DIMENSION viewDimension{};
	};

	//ResourceDescription desc{
	//		.format = DXGI_FORMAT_,
	//		.width= ,
	//		.height = ,
	//		.depthOrArraySize = 1,
	//		.dimension = D3D12_RESOURCE_DIMENSION_,
	//		.flags = ResourceFlags::None,
	//		.createState = ResourceState::,
	//		.heapType = D3D12_HEAP_TYPE_DEFAULT,
	//		.descriptor = ResourceDescriptorFlags::,
	//		.viewDimension = D3D12_SRV_DIMENSION_
   //};

	class Resource
	{
	public:
		Resource() = default;

		Resource(ID3D12Device* device, ResourceDescription desc, D3D12_CLEAR_VALUE* val = nullptr);
		void init(ID3D12Device* device, ResourceDescription desc, D3D12_CLEAR_VALUE* val = nullptr);
		void transition(ID3D12GraphicsCommandList* cmdList, ResourceState state);

		void reset()
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

		ID3D12Resource* resource() { return m_resource; }
		ID3D12Resource** getResourceAddress() { return &m_resource; }

		void createViews(ID3D12Device* device, ResourceDescriptorFlags descriptors);

		DescriptorDSV dsv;
		DescriptorRTV rtv;
		DescriptorSRV srv;
		DescriptorUAV uav;

	private:
		ID3D12Resource* m_resource;
		ResourceState m_currentState = ResourceState::COMMON;
		D3D12_SRV_DIMENSION m_viewDimension{};
		u32 m_bufferSize = 0;
	};

	template<typename T>
	class FrameResourceHandler
	{
	public:
		T& operator[](u16 index) { return m_resources[index]; }
	private:
		std::array<T, engine::config::NumFrames> m_resources;
	};

};
