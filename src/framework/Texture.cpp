#include "Framework/graphics/Texture.h"
#include "Framework/../external/stb/stb_image.h"
#include "Framework/util/Util.h"
#include "Framework/include/common.h"
#include "Framework/graphics/dx12/DescriptorHeap.h"
#include "Framework/graphics/dx12/Device.h"

void BindTexture(ID3D12GraphicsCommandList* cmdList, TextureHandle handle, uint bindSlot)
{
	auto textureHandle = DescriptorHeapManager::CurrentSRVHeap->GetGPUHandle(handle);
	cmdList->SetGraphicsRootDescriptorTable(bindSlot, textureHandle);
}

void BindTextures(Material& material, ID3D12GraphicsCommandList* cmdList)
{
    if(material.diffuseTexture>0)
		BindTexture(cmdList, material.diffuseTexture, 3);
    if(material.specularTexture>0)
		BindTexture(cmdList, material.specularTexture, 4);
}

Texture::Texture(const char* path, ID3D12Device* device, ComPtr<ID3D12GraphicsCommandList>& commandList, std::string s)
{
    Init(path, device, commandList, s);
}

void Texture::Init(const char* path, ID3D12Device* device, ComPtr<ID3D12GraphicsCommandList>& commandList, std::string s)
{
	std::uint8_t* data = stbi_load(path, &width, &height, &nrChannels, 4); 
    m_texture.Init(Device::device->GetDevice(), DXGI_FORMAT_R8G8B8A8_UNORM, width, height, 1, D3D12_RESOURCE_DIMENSION_TEXTURE2D,
        ResourceFlags::NONE, ResourceState::COPY_DEST, CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
        ResourceDescriptorFlags::ShaderResource);

    const UINT64 uploadBufferSize = GetRequiredIntermediateSize(m_texture, 0, 1);

    textureUploadHeap.Init(Device::device->GetDevice(), CD3DX12_RESOURCE_DESC::Buffer(uploadBufferSize), ResourceState::GENERIC_READ_STATE,
        CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD));

    D3D12_SUBRESOURCE_DATA textureData = {};
    textureData.pData = &data[0];
    int size = 4;
    textureData.RowPitch = width*size;
    textureData.SlicePitch = textureData.RowPitch * height;

    UpdateSubresources(commandList.Get(), m_texture, textureUploadHeap, 0, 0, 1, &textureData);
    m_texture.Transition(commandList.Get(), ResourceState::PIXEL_SHADER_RESOURCE);
    index = m_texture.srv.HeapIndex;
    m_name = s;
}
TextureHandle TextureColection::CreateTexture(const char* path, ID3D12Device* device, ComPtr<ID3D12GraphicsCommandList>& cmdList, std::string s)
{
    if(static_textures.find(path)==static_textures.end())
		static_textures[path] = Texture(path, device, cmdList, s);
    return static_textures[path].GetHandle();
}

bool operator==(Material mat1, Material mat2)
{
    return (mat1.diffuseTexture == mat2.diffuseTexture) && (mat1.specularTexture == mat2.specularTexture);
}
