#include "EngineGfx/Texture.h"
#include "third_party/stb/stb_image.h"
#include "EngineCommon/util/Util.h"
#include "EngineCommon/include/common.h"
#include "EngineGfx/dx12/DescriptorHeap.h"
#include "EngineGfx/dx12/Device.h"

namespace engine::graphics
{
    Texture::Texture(system::Filepath path, ID3D12Device* device, ID3D12GraphicsCommandList* commandList, std::string s)
    {
        Init(path, device, commandList, s);
    }

    void Texture::Init(system::Filepath path , ID3D12Device* device, ID3D12GraphicsCommandList* commandList, std::string s)
    {
		Resource textureUploadHeap;
        std::uint8_t* data = stbi_load(path.str().c_str(), &width, &height, &nrChannels, 4);
        ResourceDescription desc;
        desc.format = DXGI_FORMAT_R8G8B8A8_UNORM;
        desc.width = width;
        desc.height = height;
        desc.dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
        desc.flags = ResourceFlags::NONE;
        desc.createState = ResourceState::COPY_DEST;
        desc.descriptor = ResourceDescriptorFlags::ShaderResource;

        Resource::init(Device::device->GetDevice(), desc);

        const UINT64 uploadBufferSize = GetRequiredIntermediateSize(getResource(), 0, 1);

        ResourceDescription uploadDesc;
        uploadDesc.format = DXGI_FORMAT_UNKNOWN;
        uploadDesc.width = uploadBufferSize;
        uploadDesc.height = 1;
        uploadDesc.dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
        uploadDesc.flags = ResourceFlags::NONE;
        uploadDesc.heapType = D3D12_HEAP_TYPE_UPLOAD;
        uploadDesc.createState = ResourceState::GENERIC_READ_STATE;
        uploadDesc.descriptor = ResourceDescriptorFlags::None;
        textureUploadHeap.init(Device::device->GetDevice(), uploadDesc);

        D3D12_SUBRESOURCE_DATA textureData = {};
        textureData.pData = &data[0];
        int size = 4;
        textureData.RowPitch = width * size;
        textureData.SlicePitch = textureData.RowPitch * height;

        UpdateSubresources(commandList, getResource(), textureUploadHeap, 0, 0, 1, &textureData);
        Resource::transition(commandList, ResourceState::PIXEL_SHADER_RESOURCE);
        index = srv.HeapIndex;
        m_name = s;
    }

    bool operator==(Material mat1, Material mat2)
    {
        return (mat1.diffuseTexture == mat2.diffuseTexture) && (mat1.specularTexture == mat2.specularTexture);
    }
};
