#pragma once
#include <vector>
#include "Texture.h"
#include "d3dUtil.h"

struct Submesh
{
	UINT IndexCount = 0;
	UINT StartIndexLocation = 0;
	INT BaseVertexLocation = 0;

    // Bounding box of the geometry defined by this submesh. 
    // This is used in later chapters of the book.
	DirectX::BoundingBox Bounds;
};

struct Mesh
{
	// Give it a name so we can look it up by name.
	std::string Name;

	// System memory copies.  Use Blobs because the vertex/index format can be generic.
	// It is up to the client to cast appropriately.  
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

	Mesh() = default;
	Mesh(
		ID3D12Device* device, ComPtr<ID3D12GraphicsCommandList>& cmList,
		const void* vertexData, UINT vertexDataSize, UINT structSize,
		const void* indexData, UINT indexDataSize, DXGI_FORMAT format);

	void Init(
		ID3D12Device* device, ComPtr<ID3D12GraphicsCommandList>& cmList,
		const void* vertexData, UINT vertexDataSize, UINT structSize,
		const void* indexData, UINT indexDataSize, DXGI_FORMAT format);

	static void CreateCPUBuffer(
		ID3D12Device* device, ComPtr<ID3D12GraphicsCommandList>& cmList,
		ComPtr<ID3DBlob>& cpuMemory, const void* data, UINT dataSize,
		ComPtr<ID3D12Resource>& uploadBuffer, ComPtr<ID3D12Resource>& gpuMemory);

	// A MeshGeometry may store multiple geometries in one vertex/index buffer.
	// Use this container to define the Submesh geometries so we can draw
	// the Submeshes individually.
	std::unordered_map<std::string, Submesh> DrawArgs;
	std::unordered_map<std::string, TextureHandle> m_textures;

	D3D12_VERTEX_BUFFER_VIEW VertexBufferView()const;
	D3D12_INDEX_BUFFER_VIEW IndexBufferView()const;
	void SetVertexBuffer(ID3D12GraphicsCommandList* cmList);
	void SetIndexBuffer(ID3D12GraphicsCommandList* cmList);
	void DisposeUploaders();
};
