#include "include/Texture.h"
#include "../external/stb/stb_image.h"
#include "include/d3dUtil.h"
#include "dx12/DescriptorHeap.h"

void BindTextures(std::vector<TextureHandle>& textures, ID3D12GraphicsCommandList* cmdList)
{
	for (int j=0; j<textures.size(); j++)
	{
		auto textureHandle = DescriptorHeapManager::CurrentSRVHeap->GetGPUHandle(textures[j]);
		cmdList->SetGraphicsRootDescriptorTable(2+j, textureHandle);
	}
}

Texture::Texture(const char* path, ID3D12Device* device, ComPtr<ID3D12GraphicsCommandList>& commandList, std::string s)
          //DescriptorHeap& srvHeap)
{
    Init(path, device, commandList, s);
}

void Texture::Init(const char* path, ID3D12Device* device, ComPtr<ID3D12GraphicsCommandList>& commandList, std::string s)
          //DescriptorHeap& srvHeap)
{

	std::uint8_t* data = stbi_load(path, &width, &height, &nrChannels, 4); 
    D3D12_RESOURCE_DESC textureDesc = {};
    textureDesc.MipLevels = 1;
    textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    textureDesc.Width = width;
    textureDesc.Height = height;
    textureDesc.Flags = D3D12_RESOURCE_FLAG_NONE;
    textureDesc.DepthOrArraySize = 1;
    textureDesc.SampleDesc.Count = 1;
    textureDesc.SampleDesc.Quality = 0;
    textureDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;

    auto heapType = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
    ThrowIfFailed(device->CreateCommittedResource(
        &heapType,
        D3D12_HEAP_FLAG_NONE,
        &textureDesc,
        D3D12_RESOURCE_STATE_COPY_DEST,
        nullptr,
        IID_PPV_ARGS(&m_texture)));

    const UINT64 uploadBufferSize = GetRequiredIntermediateSize(m_texture.Get(), 0, 1);

    // Create the GPU upload buffer.
    heapType = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
    auto bufferDesc = CD3DX12_RESOURCE_DESC::Buffer(uploadBufferSize);
    ThrowIfFailed(device->CreateCommittedResource(
        &heapType,
        D3D12_HEAP_FLAG_NONE,
        &bufferDesc,
        D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr,
        IID_PPV_ARGS(&textureUploadHeap)));

    // Copy data to the intermediate upload heap and then schedule a copy 
    // from the upload heap to the Texture2D.

    D3D12_SUBRESOURCE_DATA textureData = {};
    textureData.pData = &data[0];
    int size = 4;
    textureData.RowPitch = width*size;
    textureData.SlicePitch = textureData.RowPitch * height;

    UpdateSubresources(commandList.Get(), m_texture.Get(), textureUploadHeap.Get(), 0, 0, 1, &textureData);
    auto transitionDesc = CD3DX12_RESOURCE_BARRIER::Transition(m_texture.Get(),
        D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
    commandList->ResourceBarrier(1, &transitionDesc);

    // Describe and create a SRV for the texture.
    D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srvDesc.Format = textureDesc.Format;
    srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Texture2D.MipLevels = 1;
    index = DescriptorHeapManager::CurrentSRVHeap->CreateSRV(srvDesc, m_texture.Get());
    m_name = s;
}
TextureHandle TextureColection::CreateTexture(const char* path, ID3D12Device* device, ComPtr<ID3D12GraphicsCommandList>& cmdList, std::string s)
{
    if(static_textures.find(path)==static_textures.end())
		static_textures[path] = Texture(path, device, cmdList, s);
    return static_textures[path].GetHandle();
}
