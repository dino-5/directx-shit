#include "Framework/graphics/Mesh.h"
#include "Framework/graphics/dx12/Device.h"
#include "Framework/graphics/dx12/DescriptorHeap.h"

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


Mesh::Mesh(
	ID3D12Device* device, ComPtr<ID3D12GraphicsCommandList>& cmList,
	const void* vertexData, UINT vertexDataSize, UINT structSize,
	const void* indexData, UINT indexDataSize, DXGI_FORMAT format)
{
	Init(device, cmList, vertexData, vertexDataSize, structSize, indexData, indexDataSize, format);
}

Mesh::Mesh(ID3D12Device* device, std::vector<std::pair< Material, std::vector<util::Geometry::MeshData>>>& mesh,
	ComPtr<ID3D12GraphicsCommandList> cmdList, std::string name)
{
    DrawArgs = Submesh::GetSubmeshes(mesh);
    std::vector<util::Geometry::Vertex> vertices;
    std::vector<std::uint16_t> indices;
    for (auto& i : mesh)
    {
		for (int j = 0; j < i.second.size(); j++)
		{
			vertices.insert(vertices.end(), i.second[j].Vertices.begin(), i.second[j].Vertices.end());
			indices.insert(indices.end(), i.second[j].GetIndices16().begin(), i.second[j].GetIndices16().end());
		}
    }

    Init(device, cmdList,
        vertices.data(), GetVectorSize(vertices), sizeof(util::Geometry::Vertex),
        indices.data(), GetVectorSize(indices));
}

Mesh::Mesh(ID3D12Device* device, std::vector <util::Geometry::MeshData>& mesh, ComPtr<ID3D12GraphicsCommandList> cmdList, std::string name) :
	Name(name)
{
    DrawArgs = Submesh::GetSubmeshes(mesh);
    std::vector<util::Geometry::Vertex> vertices;
    std::vector<std::uint16_t> indices;
    for (auto& i : mesh)
    {
        vertices.insert(vertices.end(), i.Vertices.begin(), i.Vertices.end());
		indices.insert(indices.end(), i.GetIndices16().begin(), i.GetIndices16().end());
    }

    Init(device, cmdList,
        vertices.data(), GetVectorSize(vertices), sizeof(util::Geometry::Vertex),
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
	if (indexData != nullptr)
	{

		CreateCPUBuffer(device, cmList,
						IndexBufferCPU, indexData, indexDataSize,
						IndexBufferUploader, IndexBufferGPU);
		IndexBufferByteSize = indexDataSize;
	}
	VertexByteStride = structSize;
	VertexBufferByteSize = vertexDataSize;
	//IndexFormat = format;
}

void Mesh::CreateCPUBuffer(
			ID3D12Device* device, ComPtr<ID3D12GraphicsCommandList>& cmList,
			ComPtr<ID3DBlob>& cpuMemory, const void* data, UINT dataSize, 
			ComPtr<ID3D12Resource>& uploadBuffer, ComPtr<ID3D12Resource>& gpuMemory)
{
	ThrowIfFailed(D3DCreateBlob(dataSize, &cpuMemory));
	CopyMemory(cpuMemory->GetBufferPointer(), data, dataSize);
	gpuMemory = engine::util::d3dUtil::CreateDefaultBuffer(device, cmList.Get(), data, dataSize, uploadBuffer);

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
	auto view = VertexBufferView();
	cmList->IASetVertexBuffers(0, 1, &view);
}

void Mesh::SetIndexBuffer(ID3D12GraphicsCommandList* cmList)
{
	auto view = IndexBufferView();
	cmList->IASetIndexBuffer(&view);
}

// We can free this memory after we finish upload to the GPU.
void Mesh::DisposeUploaders()
{
	VertexBufferUploader = nullptr;
	IndexBufferUploader = nullptr;
}

