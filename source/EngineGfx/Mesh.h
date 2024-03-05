#pragma once
#include <vector>
#include "Texture.h"
#include "EngineGfx/dx12/Buffers.h"
#include "EngineCommon/util/Util.h"
#include "EngineCommon/util/GeometryGenerator.h"
#include "EngineCommon/include/types.h"

namespace engine::graphics
{
	class RenderContext;
	struct Submesh
	{
		UINT IndexCount = 0;
		UINT StartIndexLocation = 0;
		INT BaseVertexLocation = 0;
		Submesh(UINT indexCount, UINT startIndex, UINT baseVertexLoc) : IndexCount(indexCount), StartIndexLocation(startIndex),
			BaseVertexLocation(baseVertexLoc) {}
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

		void Init(
			RenderContext& context,
			const void* vertexData, UINT vertexDataSize, UINT structSize,
			const void* indexData, UINT indexDataSize, DXGI_FORMAT format = DXGI_FORMAT_R16_UINT);

	public:

		std::string Name;

		Buffer m_vertexBuffer;
		Buffer m_indexBuffer;
		// Data about the buffers.
		UINT VertexByteStride = 0;
		UINT VertexBufferByteSize = 0;
		UINT IndexBufferByteSize = 0;

		std::vector<std::pair< Material, std::vector<Submesh> >> DrawArgs;

	};
};
