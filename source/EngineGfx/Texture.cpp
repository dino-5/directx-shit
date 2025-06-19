#include "EngineGfx/Texture.h"
#include "EngineCommon/util/Util.h"
#include "EngineGfx/dx12/DescriptorHeap.h"
#include "EngineGfx/dx12/Device.h"
#define STBI_MALOC ArenaAlloc
#include "third_party/stb/stb_image.h"

namespace engine::graphics
{
void TextureDescription::init(system::Filepath path)
{
    setData((stbi_load(path.str().c_str(), &width, &height, &channels, 4)));
    needToDestruct = true;
}

Texture::Texture(TextureDescription imData,
                 const GfxContext& ctx,
                 DescriptorFlags flags,
                 ResourceState state)
: width(imData.width), height(imData.height)
{
    ResourceDescription desc;
    desc.format = DXGI_FORMAT_R8G8B8A8_UNORM;
    desc.width = width;
    desc.height = height;
    desc.dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    desc.flags = ResourceFlags::NONE;
    desc.createState = imData.data ? ResourceState::COPY_DEST : state;
    desc.name = imData.name;

    DescriptorProperties descriptorProps{
        .descriptor = flags,
        .viewDimension = D3D12_SRV_DIMENSION_TEXTURE2D
    };

    Resource::initResource(ctx.device, desc, descriptorProps);

    if(imData.data)
    {

        const UINT64 uploadBufferSize = GetRequiredIntermediateSize(resource(), 0, 1);

        ResourceDescription uploadDesc;
        uploadDesc.format = DXGI_FORMAT_UNKNOWN;
        uploadDesc.width = (u32)uploadBufferSize;
        uploadDesc.height = 1;
        uploadDesc.dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
        uploadDesc.flags = ResourceFlags::NONE;
        uploadDesc.heapType = D3D12_HEAP_TYPE_UPLOAD;
        uploadDesc.createState = ResourceState::GENERIC_READ_STATE;
        textureUploadHeap.initResource(
            ctx.device,
            uploadDesc,
            DescriptorProperties(DescriptorFlags::None));

        D3D12_SUBRESOURCE_DATA textureData = {};
        textureData.pData = imData.data;
        int size = 4;
        textureData.RowPitch = imData.width * size;
        textureData.SlicePitch = textureData.RowPitch * imData.height;

        UpdateSubresources(
            ctx.cmdList.list,
            resource(),
            textureUploadHeap, 0, 0, 1,
            &textureData);

        Resource::transition(ctx.cmdList.list, state);
    }

}

};
