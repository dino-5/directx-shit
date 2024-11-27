#pragma once
#include <unordered_map>
#include "Texture.h"
#include "EngineGfx/dx12/Buffers.h"
#include "EngineCommon/util/Util.h"
#include "EngineCommon/include/types.h"
#include "EngineCommon/math/Vector.h"

namespace engine::graphics
{
	struct Material
	{
		TextureHandle occlusionTexture;
		//TextureHandle emmisiveTexture;
	};
	class GfxContext;
	struct Submesh
	{
		u32 IndexCount = 0;
		u32 StartIndexLocation = 0;
		u32 BaseVertexLocation = 0;
		i32 materialIndex = 0;
		Submesh(u32 indexCount, u32 startIndex, u32 baseVertexLoc, i32 matIndex) : IndexCount(indexCount), StartIndexLocation(startIndex),
			BaseVertexLocation(baseVertexLoc), materialIndex(matIndex) {}
		Submesh() = default;

		void draw(ID3D12GraphicsCommandList* cmList);
	};

	struct Vertex
	{
		math::Vector3 position;
		math::Vector3 normal;
		math::Vector4 tangent;
		math::Vector2 uv;
	};

	template<typename VertexT>
    struct Geometry
    {
        std::vector<VertexT> vertices;
        std::vector<u32> indices;
        u32 lastIndexLocation=0;
        u32 lastVertexOffset=0;
        Submesh getSubmesh(i32 index)
        {
            Submesh result;
            result.IndexCount = indices.size() - lastIndexLocation;
            result.StartIndexLocation = lastIndexLocation;
            result.BaseVertexLocation = lastVertexOffset;
            lastIndexLocation = indices.size();
            lastVertexOffset = vertices.size();
			result.materialIndex = index;
            return result;
        }
    };

	struct Mesh
	{
	public:
		Mesh() = default;

		template<typename VertexType>
		Mesh(GfxContext& context, const Geometry<VertexType>& geometry)
		{
			init(context, geometry.vertices.data(), geometry.vertices.size(),
				geometry.indices.data(), geometry.indices.size());
		}

        template<typename VertexType, typename IndexType>
		Mesh(
            GfxContext& context,
            const VertexType* vertexData, UINT vertexCount,
            const IndexType* indexData, UINT indexCount)
		{
			init(context, vertexData, vertexCount, indexData, indexCount);
		}

		template<typename VertexType>
		void init(GfxContext& context, const Geometry<VertexType>& geometry)
		{
			init(context, geometry.vertices.data(), geometry.vertices.size(),
				geometry.indices.data(), geometry.indices.size());
		}

        template<typename VertexType, typename IndexType>
		void init(
			GfxContext& context,
			const VertexType* vertexData, UINT vertexCount,
			const IndexType* indexData, UINT indexCount)
		{
            m_vertexByteStride = sizeof(VertexType);
            m_vertexBufferByteSize = m_vertexByteStride * vertexCount;

			BufferDescription<VertexType> desc;
			desc.data = vertexData;
			desc.elementCount = vertexCount;
			desc.name = "VertexBuffer";
			desc.state = ResourceState::VERTEX_CONSTANT_BUFFER;
			desc.type = BufferType::VERTEX;
			m_vertexBuffer.init(context, desc);

            if (indexData != nullptr)
            {
                BufferDescription<IndexType> desc;
                desc.data = indexData;
                desc.elementCount = indexCount;
                desc.name = "IndexBuffer";
                desc.state = ResourceState::INDEX_BUFFER;
                desc.type = BufferType::INDEX;
				m_indexBuffer.init(context, desc);

                m_indexBufferByteSize = sizeof(IndexType);
				m_indexCount = indexCount;
            }
		}

		void reset()
		{
			m_vertexBuffer.reset();
			m_indexBuffer.reset();
		}

	public:

		std::string Name;
		Buffer m_vertexBuffer;
		Buffer m_indexBuffer;
		// Data about the buffers.
		u32 m_vertexByteStride = 0;
		u32 m_vertexBufferByteSize = 0;
		u32 m_indexBufferByteSize = 0;
		u32 m_indexCount = 0;

		//std::unordered_map<Material, std::vector<Submesh>> DrawArgs;
		Material m_material;

	};
};
