#include "EngineGfx/Texture.h"
#include "EngineCommon/util/Util.h"
#include "EngineCommon/include/common.h"
#include "EngineGfx/dx12/DescriptorHeap.h"
#include "EngineGfx/dx12/Device.h"
#include "third_party/stb/stb_image.h"

namespace engine::graphics
{
    void ImageData::Init(system::Filepath path)
    {
        SetData((stbi_load(path.str().c_str(), &width, &height, &channels, 4)));
        needToDestruct = true;
    }

    Texture::Texture(ImageData imData, ID3D12Device* device, ID3D12GraphicsCommandList* commandList, std::string s)
    {
        Init(imData, device, commandList, s);
    }

    void Texture::Init(ImageData imData , ID3D12Device* device, ID3D12GraphicsCommandList* commandList, std::string s)
    {
        ResourceDescription desc;
        desc.format = DXGI_FORMAT_R8G8B8A8_UNORM;
        desc.width = imData.width;
        desc.height = imData.height;
        desc.dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
        desc.flags = ResourceFlags::NONE;
        desc.createState = ResourceState::COPY_DEST;
        desc.descriptor.descriptor = DescriptorFlags::ShaderResource;
        desc.descriptor.viewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
        desc.name = imData.name;

        Resource::init(Device::device->GetDevice(), desc);

        const UINT64 uploadBufferSize = GetRequiredIntermediateSize(resource(), 0, 1);

        ResourceDescription uploadDesc;
        uploadDesc.format = DXGI_FORMAT_UNKNOWN;
        uploadDesc.width = uploadBufferSize;
        uploadDesc.height = 1;
        uploadDesc.dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
        uploadDesc.flags = ResourceFlags::NONE;
        uploadDesc.heapType = D3D12_HEAP_TYPE_UPLOAD;
        uploadDesc.createState = ResourceState::GENERIC_READ_STATE;
        uploadDesc.descriptor.descriptor = DescriptorFlags::None;
        textureUploadHeap.init(Device::device->GetDevice(), uploadDesc);

        D3D12_SUBRESOURCE_DATA textureData = {};
        textureData.pData = imData.data;
        int size = 4;
        textureData.RowPitch = imData.width * size;
        textureData.SlicePitch = textureData.RowPitch * imData.height;

        UpdateSubresources(commandList, resource(), textureUploadHeap, 0, 0, 1, &textureData);
        Resource::transition(commandList, ResourceState::PIXEL_SHADER_RESOURCE);
        m_name = s;
    }

    Texture* Texture::CreateTexture(ImageData imData, ID3D12Device* device, ID3D12GraphicsCommandList* cmdList,
        std::string s )
    {
        if(s_textures.find(std::string(imData.name))==s_textures.end())
            s_textures[imData.name] = Texture(imData, device, cmdList, s);
        return &s_textures[imData.name];
    }

};
