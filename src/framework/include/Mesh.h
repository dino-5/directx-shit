#pragma once
#include <vector>
#include "Texture.h"
#include "d3dUtil.h"
#include "GeometryGenerator.h"

struct Submesh
{
	UINT IndexCount = 0;
	UINT StartIndexLocation = 0;
	INT BaseVertexLocation = 0;
	Submesh(UINT indexCount, UINT startIndex, UINT baseVertexLoc) : IndexCount(indexCount), StartIndexLocation(startIndex),
		BaseVertexLocation(baseVertexLoc) {}
	Submesh() = default;

	void Draw(ID3D12GraphicsCommandList* cmList)
	{
		if(IndexCount)
			cmList->DrawIndexedInstanced(IndexCount, 1, StartIndexLocation, BaseVertexLocation, 0);
	}

	static std::vector<Submesh> GetSubmeshes(std::vector<Geometry::MeshData> mesh)
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
		return submeshes;
	}
};

struct Mesh
{
public:
	Mesh() = default;
	Mesh(
		ID3D12Device* device, ComPtr<ID3D12GraphicsCommandList>& cmList,
		const void* vertexData, UINT vertexDataSize, UINT structSize,
		const void* indexData, UINT indexDataSize, DXGI_FORMAT format=DXGI_FORMAT_R16_UINT);

	void Init(
		ID3D12Device* device, ComPtr<ID3D12GraphicsCommandList>& cmList,
		const void* vertexData, UINT vertexDataSize, UINT structSize,
		const void* indexData, UINT indexDataSize, DXGI_FORMAT format=DXGI_FORMAT_R16_UINT);

	static void CreateCPUBuffer(
		ID3D12Device* device, ComPtr<ID3D12GraphicsCommandList>& cmList,
		ComPtr<ID3DBlob>& cpuMemory, const void* data, UINT dataSize,
		ComPtr<ID3D12Resource>& uploadBuffer, ComPtr<ID3D12Resource>& gpuMemory);

	D3D12_VERTEX_BUFFER_VIEW VertexBufferView()const;
	D3D12_INDEX_BUFFER_VIEW IndexBufferView()const;
	void SetVertexBuffer(ID3D12GraphicsCommandList* cmList);
	void SetIndexBuffer(ID3D12GraphicsCommandList* cmList);
	void DisposeUploaders();

public:

	std::string Name;

	Microsoft::WRL::ComPtr<ID3DBlob> VertexBufferCPU = nullptr;
	Microsoft::WRL::ComPtr<ID3DBlob> IndexBufferCPU  = nullptr;

	Microsoft::WRL::ComPtr<ID3D12Resource> VertexBufferGPU = nullptr;
	Microsoft::WRL::ComPtr<ID3D12Resource> IndexBufferGPU = nullptr;

	Microsoft::WRL::ComPtr<ID3D12Resource> VertexBufferUploader = nullptr;
	Microsoft::WRL::ComPtr<ID3D12Resource> IndexBufferUploader = nullptr;

    // Data about the buffers.
	UINT VertexByteStride = 0;
	UINT VertexBufferByteSize = 0;
	DXGI_FORMAT IndexFormat = DXGI_FORMAT_R16_UINT;
	UINT IndexBufferByteSize = 0;

	std::unordered_map<std::string, Submesh> DrawArgs;
	std::unordered_map<std::string, TextureHandle> m_textures;

};
