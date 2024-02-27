#include "EngineGfx/Texture.h"
#include "third_party/stb/stb_image.h"
#include "EngineCommon/util/Util.h"
#include "EngineCommon/include/common.h"
#include "EngineGfx/dx12/DescriptorHeap.h"
#include "EngineGfx/dx12/Device.h"

namespace engine::graphics
{
    void BindTexture(ID3D12GraphicsCommandList* cmdList, TextureHandle handle, uint bindSlot)
    {
        auto textureHandle = DescriptorHeapManager::CurrentSRVHeap.getGPUHandle(handle);
        cmdList->SetGraphicsRootDescriptorTable(bindSlot, textureHandle);
    }

    void BindTextures(Material& material, ID3D12GraphicsCommandList* cmdList)
    {
        if (material.diffuseTexture > 0)
            BindTexture(cmdList, material.diffuseTexture, 3);
        if (material.specularTexture > 0)
            BindTexture(cmdList, material.specularTexture, 4);
    }

    Texture::Texture(const char* path, ID3D12Device* device, ComPtr<ID3D12GraphicsCommandList>& commandList, std::string s)
    {
        Init(path, device, commandList, s);
    }

    void Texture::Init(const char* path, ID3D12Device* device, ComPtr<ID3D12GraphicsCommandList>& commandList, std::string s)
    {
        std::uint8_t* data = stbi_load(path, &width, &height, &nrChannels, 4);
        ResourceDescription desc;
        desc.format = DXGI_FORMAT_R8G8B8A8_UNORM;
        desc.width = width;
        desc.height = height;
        desc.dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
        desc.flags = ResourceFlags::NONE;
        desc.createState = ResourceState::COPY_DEST;
        desc.descriptor = ResourceDescriptorFlags::ShaderResource;

        m_texture.init(Device::device->GetDevice(), desc);

        const UINT64 uploadBufferSize = GetRequiredIntermediateSize(m_texture, 0, 1);

        textureUploadHeap.init(Device::device->GetDevice(), CD3DX12_RESOURCE_DESC::Buffer(uploadBufferSize), ResourceState::GENERIC_READ_STATE,
            CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD));

        D3D12_SUBRESOURCE_DATA textureData = {};
        textureData.pData = &data[0];
        int size = 4;
        textureData.RowPitch = width * size;
        textureData.SlicePitch = textureData.RowPitch * height;

        UpdateSubresources(commandList.Get(), m_texture, textureUploadHeap, 0, 0, 1, &textureData);
        m_texture.transition(commandList.Get(), ResourceState::PIXEL_SHADER_RESOURCE);
        index = m_texture.srv.HeapIndex;
        m_name = s;
    }
    TextureHandle TextureColection::CreateTexture(const char* path, ID3D12Device* device, ComPtr<ID3D12GraphicsCommandList>& cmdList, std::string s)
    {
        if (static_textures.find(path) == static_textures.end())
            static_textures[path] = Texture(path, device, cmdList, s);
        return static_textures[path].GetHandle();
    }

    bool operator==(Material mat1, Material mat2)
    {
        return (mat1.diffuseTexture == mat2.diffuseTexture) && (mat1.specularTexture == mat2.specularTexture);
    }
};
