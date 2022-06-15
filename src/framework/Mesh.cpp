#include "include/Mesh.h"
#include "dx12/Device.h"


Mesh::Mesh(
	ID3D12Device* device, ComPtr<ID3D12GraphicsCommandList>& cmList,
	const void* vertexData, UINT vertexDataSize, UINT structSize,
	const void* indexData, UINT indexDataSize, DXGI_FORMAT format)
{
	Init(device, cmList, vertexData, vertexDataSize, structSize, indexData, indexDataSize, format);
}

Mesh::Mesh(std::vector < Geometry::MeshData>& mesh, ComPtr<ID3D12GraphicsCommandList> cmdList, std::string name) :
	Name(name)
{
    DrawArgs = Submesh::GetSubmeshes(mesh);
    std::vector<Geometry::Vertex> vertices;
    std::vector<std::uint16_t> indices;
    for (auto& i : mesh)
    {
        vertices.insert(vertices.end(), i.Vertices.begin(), i.Vertices.end());
		indices.insert(indices.end(), i.GetIndices16().begin(), i.GetIndices16().end());
    }

    Init(Device::GetDevice(), cmdList,
        vertices.data(), GetVectorSize(vertices), sizeof(Geometry::Vertex),
        indices.data(), GetVectorSize(indices));
}

void Mesh::Init(
	ID3D12Device* device, ComPtr<ID3D12GraphicsCommandList>& cmList,
	const void* vertexData, UINT vertexDataSize, UINT structSize,
	const void* indexData, UINT indexDataSize, DXGI_FORMAT format)
{
	CreateCPUBuffer(device, cmList,
					VertexBufferCPU, vertexData, vertexDataSize,
					VertexBufferUploader, VertexBufferGPU);
	CreateCPUBuffer(device, cmList,
					IndexBufferCPU, indexData, indexDataSize,
					IndexBufferUploader, IndexBufferGPU);

	VertexByteStride = structSize;
	VertexBufferByteSize = vertexDataSize;
	//IndexFormat = format;
	IndexBufferByteSize = indexDataSize;
}

void Mesh::CreateCPUBuffer(
			ID3D12Device* device, ComPtr<ID3D12GraphicsCommandList>& cmList,
			ComPtr<ID3DBlob>& cpuMemory, const void* data, UINT dataSize, 
			ComPtr<ID3D12Resource>& uploadBuffer, ComPtr<ID3D12Resource>& gpuMemory)
{
	ThrowIfFailed(D3DCreateBlob(dataSize, &cpuMemory));
	CopyMemory(cpuMemory->GetBufferPointer(), data, dataSize);
	gpuMemory = d3dUtil::CreateDefaultBuffer(device, cmList.Get(), data, dataSize, uploadBuffer);

}


D3D12_VERTEX_BUFFER_VIEW Mesh::VertexBufferView()const
{
	D3D12_VERTEX_BUFFER_VIEW vbv;
	vbv.BufferLocation = VertexBufferGPU->GetGPUVirtualAddress();
	vbv.StrideInBytes = VertexByteStride;
	vbv.SizeInBytes = VertexBufferByteSize;

	return vbv;
}

D3D12_INDEX_BUFFER_VIEW Mesh::IndexBufferView()const
{
	D3D12_INDEX_BUFFER_VIEW ibv;
	ibv.BufferLocation = IndexBufferGPU->GetGPUVirtualAddress();
	ibv.Format = IndexFormat;
	ibv.SizeInBytes = IndexBufferByteSize;

	return ibv;
}

void Mesh::SetVertexBuffer(ID3D12GraphicsCommandList* cmList)
{
	cmList->IASetVertexBuffers(0, 1, &VertexBufferView());
}

void Mesh::SetIndexBuffer(ID3D12GraphicsCommandList* cmList)
{
	cmList->IASetIndexBuffer(&IndexBufferView());
}

// We can free this memory after we finish upload to the GPU.
void Mesh::DisposeUploaders()
{
	VertexBufferUploader = nullptr;
	IndexBufferUploader = nullptr;
}

