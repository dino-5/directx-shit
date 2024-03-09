#include "EngineGfx/Mesh.h"
#include "EngineGfx/RenderContext.h"
#include "EngineGfx/dx12/Device.h"
#include "EngineGfx/dx12/DescriptorHeap.h"

namespace engine::graphics
{
	void Submesh::Draw(ID3D12GraphicsCommandList* cmList)
	{
		if (IndexCount)
		{
			cmList->DrawIndexedInstanced(IndexCount, 1, StartIndexLocation, BaseVertexLocation, 0);
		}
	}

	std::vector<std::pair< Material, std::vector<Submesh> >> Submesh::GetSubmeshes(std::vector<util::Geometry::MeshData> mesh)
	{
		std::vector<Submesh> submeshes(mesh.size());
		UINT vertexOffset = 0;
		UINT indexOffset = 0;
		for (int i=0;i<mesh.size();i++)
		{
			submeshes[i].BaseVertexLocation = vertexOffset;
			submeshes[i].StartIndexLocation = indexOffset;
			submeshes[i].IndexCount = mesh[i].Indices32.size();

			vertexOffset += mesh[i].Vertices.size();
			indexOffset += mesh[i].Indices32.size();
		}
		return { { Material(), submeshes}};
	}

	std::vector<std::pair< Material, std::vector<Submesh> >>
	Submesh::GetSubmeshes(std::vector<std::pair< Material, std::vector<util::Geometry::MeshData> >>& mesh)
	{
		std::vector<std::pair< Material, std::vector<Submesh> >> submeshes;
		UINT vertexOffset = 0;
		UINT indexOffset = 0;
		for (auto& i : mesh)
		{
			for (int j = 0; j < i.second.size(); j++)
			{
				Submesh submesh;
				submesh.BaseVertexLocation = vertexOffset;
				submesh.StartIndexLocation = indexOffset;
				submesh.IndexCount = i.second[j].Indices32.size();

				vertexOffset += i.second[j].Vertices.size();
				indexOffset += i.second[j].Indices32.size();
				if (j == 0)
					submeshes.push_back({ i.first, {submesh} });
				else
					submeshes.back().second.push_back(submesh);
			}
		}
		return submeshes;
	}

	void Mesh::Init(
		RenderContext& context,
		const void* vertexData, UINT vertexDataSize, UINT structSize,
		const void* indexData, UINT indexDataSize)
	{
		m_vertexBuffer.Init(context, vertexData, vertexDataSize, structSize, vertexDataSize / structSize);
		if (indexData != nullptr)
		{
            m_indexBuffer.Init(context, indexData, indexDataSize, structSize, indexDataSize / sizeof(u32));
			IndexBufferByteSize = indexDataSize;
		}
		VertexByteStride = structSize;
		VertexBufferByteSize = vertexDataSize;
	}
};
