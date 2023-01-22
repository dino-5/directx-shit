#pragma once
#include <vector>
#include "Texture.h"
#include "Util.h"
#include "GeometryGenerator.h"

struct Submesh
{
	UINT IndexCount = 0;
	UINT StartIndexLocation = 0;
	INT BaseVertexLocation = 0;
	Submesh(UINT indexCount, UINT startIndex, UINT baseVertexLoc) : IndexCount(indexCount), StartIndexLocation(startIndex),
		BaseVertexLocation(baseVertexLoc) {}
	Submesh() = default;

	void Draw(ID3D12GraphicsCommandList* cmList);
	static std::vector<std::pair< Material, std::vector<Submesh> >> GetSubmeshes(std::vector<Geometry::MeshData> mesh);
	static std::vector<std::pair< Material, std::vector<Submesh> >> GetSubmeshes(std::vector<std::pair< Material, std::vector<Geometry::MeshData> >>& mesh);
};

struct Mesh
{
public:
	Mesh() = default;
	Mesh(std::vector < Geometry::MeshData>& mesh, ComPtr<ID3D12GraphicsCommandList> cmdList, std::string name);
	Mesh(std::vector<std::pair<Material, std::vector<Geometry::MeshData>> >& mesh,
		ComPtr<ID3D12GraphicsCommandList> cmdList, std::string name);
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

	ComPtr<ID3DBlob> VertexBufferCPU = nullptr;
	ComPtr<ID3DBlob> IndexBufferCPU  = nullptr;

	ComPtr<ID3D12Resource> VertexBufferGPU = nullptr;
	ComPtr<ID3D12Resource> IndexBufferGPU = nullptr;

	ComPtr<ID3D12Resource> VertexBufferUploader = nullptr;
	ComPtr<ID3D12Resource> IndexBufferUploader = nullptr;

    // Data about the buffers.
	UINT VertexByteStride = 0;
	UINT VertexBufferByteSize = 0;
	DXGI_FORMAT IndexFormat = DXGI_FORMAT_R16_UINT;
	UINT IndexBufferByteSize = 0;

	std::vector<std::pair< Material, std::vector<Submesh> >> DrawArgs;

};
