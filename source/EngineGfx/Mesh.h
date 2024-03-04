#pragma once
#include <vector>
#include "Texture.h"
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
		//Mesh(RenderContext& context, MeshDataVector& mesh, std::string name);
		//Mesh(RenderContext& context, std::vector<std::pair<Material, MeshDataVector> >& mesh,
		// std::string name);
		//Mesh(
		//	RenderContext& context,	
		//	const void* vertexData, UINT vertexDataSize, UINT structSize,
		//	const void* indexData, UINT indexDataSize, DXGI_FORMAT format = DXGI_FORMAT_R16_UINT);

		void Init(
			RenderContext& context,
			const void* vertexData, UINT vertexDataSize, UINT structSize,
			const void* indexData, UINT indexDataSize, DXGI_FORMAT format = DXGI_FORMAT_R16_UINT);

		static void CreateCPUBuffer(
			RenderContext& context,
			ComPtr<ID3DBlob>& cpuMemory, const void* data, UINT dataSize,
			ComPtr<ID3D12Resource>& uploadBuffer, ComPtr<ID3D12Resource>& gpuMemory);

		D3D12_VERTEX_BUFFER_VIEW VertexBufferView()const;
		D3D12_INDEX_BUFFER_VIEW IndexBufferView()const;
		void SetVertexBuffer(ID3D12GraphicsCommandList* cmList);
		void SetIndexBuffer(ID3D12GraphicsCommandList* cmList);
		void DisposeUploaders();

	public:

		std::string Name;

		ComPtr<ID3DBlob> VertexBufferCPU = nullptr;
		ComPtr<ID3D12Resource> VertexBufferGPU = nullptr;
		ComPtr<ID3D12Resource> VertexBufferUploader = nullptr;

		ComPtr<ID3DBlob> IndexBufferCPU = nullptr;
		ComPtr<ID3D12Resource> IndexBufferGPU = nullptr;
		ComPtr<ID3D12Resource> IndexBufferUploader = nullptr;

		// Data about the buffers.
		UINT VertexByteStride = 0;
		UINT VertexBufferByteSize = 0;
		DXGI_FORMAT IndexFormat = DXGI_FORMAT_R16_UINT;
		UINT IndexBufferByteSize = 0;

		std::vector<std::pair< Material, std::vector<Submesh> >> DrawArgs;

	};
};
