#pragma once
#include <unordered_map>
#include "Texture.h"
#include "EngineGfx/dx12/Buffers.h"
#include "EngineCommon/util/Util.h"
#include "EngineCommon/util/GeometryGenerator.h"
#include "EngineCommon/include/types.h"

namespace engine::graphics
{
	struct Material
	{
		TextureHandle occlusionTexture;
		//TextureHandle emmisiveTexture;
	};
	class RenderContext;
	struct Submesh
	{
		u32 IndexCount = 0;
		u32 StartIndexLocation = 0;
		u32 BaseVertexLocation = 0;
		u32 materialIndex = 0;
		Submesh(u32 indexCount, u32 startIndex, u32 baseVertexLoc, u32 matIndex) : IndexCount(indexCount), StartIndexLocation(startIndex),
			BaseVertexLocation(baseVertexLoc), materialIndex(matIndex) {}
		Submesh() = default;

		void draw(ID3D12GraphicsCommandList* cmList);
		static std::vector<std::pair< Material, std::vector<Submesh> >> GetSubmeshes(std::vector<util::Geometry::MeshData> mesh);
		static std::vector<std::pair< Material, std::vector<Submesh> >> GetSubmeshes(std::vector<std::pair< Material, std::vector<util::Geometry::MeshData> >>& mesh);
	};

	struct Vertex
	{
		math::Vector3 position;
		math::Vector3 normal;
		math::Vector2 uv;
	};

	template<typename VertexT>
    struct Geometry
    {
        std::vector<VertexT> vertices;
        std::vector<u32> indices;
        u32 lastIndexLocation=0;
        u32 lastVertexOffset=0;
        Submesh getSubmesh(u32 index)
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
		using MeshDataVector = std::vector < util::Geometry::MeshData>;
		Mesh() = default;

		template<typename VertexType>
		Mesh(RenderContext& context, const Geometry<VertexType>& geometry)
		{
			init(context, geometry.vertices.data(), geometry.vertices.size(),
				geometry.indices.data(), geometry.indices.size());
		}

        template<typename VertexType, typename IndexType>
		Mesh(
            RenderContext& context,
            const VertexType* vertexData, UINT vertexCount,
            const IndexType* indexData, UINT indexCount)
		{
			init(context, vertexData, vertexCount, indexData, indexCount);
		}

		template<typename VertexType>
		void init(RenderContext& context, const Geometry<VertexType>& geometry)
		{
			init(context, geometry.vertices.data(), geometry.vertices.size(),
				geometry.indices.data(), geometry.indices.size());
		}

        template<typename VertexType, typename IndexType>
		void init(
			RenderContext& context,
			const VertexType* vertexData, UINT vertexCount,
			const IndexType* indexData, UINT indexCount)
		{
            m_vertexBuffer.initAsVertexBuffer(context, vertexData, vertexCount);
            if (indexData != nullptr)
            {
                m_indexBuffer.initAsIndexBuffer(context, indexData, indexCount);
				m_indexBuffer.transition(context.getList().getList(), ResourceState::INDEX_BUFFER);
                m_indexBufferByteSize = sizeof(IndexType);
				m_indexCount = indexCount;
            }
            m_vertexByteStride = sizeof(VertexType);
            m_vertexBufferByteSize = m_vertexByteStride * vertexCount;
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
