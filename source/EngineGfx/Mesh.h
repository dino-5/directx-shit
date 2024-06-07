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

		void Draw(ID3D12GraphicsCommandList* cmList);
		static std::vector<std::pair< Material, std::vector<Submesh> >> GetSubmeshes(std::vector<util::Geometry::MeshData> mesh);
		static std::vector<std::pair< Material, std::vector<Submesh> >> GetSubmeshes(std::vector<std::pair< Material, std::vector<util::Geometry::MeshData> >>& mesh);
	};

	struct Mesh
	{
	public:
		using MeshDataVector = std::vector < util::Geometry::MeshData>;
		Mesh() = default;

        template<typename VertexType, typename IndexType>
		Mesh(
            RenderContext& context,
            const VertexType* vertexData, UINT vertexCount,
            const IndexType* indexData, UINT indexCount)
		{
			Init(context, vertexData, vertexCount, indexData, indexCount);
		}

        template<typename VertexType, typename IndexType>
		void Init(
			RenderContext& context,
			const VertexType* vertexData, UINT vertexCount,
			const IndexType* indexData, UINT indexCount)
		{
            m_vertexBuffer.Init(context, vertexData, vertexCount);
            if (indexData != nullptr)
            {
                m_indexBuffer.Init(context, indexData, indexCount);
				m_indexBuffer.Transition(context.GetList().GetList(), ResourceState::INDEX_BUFFER);
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
