#include "include/Texture.h"
#include "../external/stb/stb_image.h"
#include "include/d3dUtil.h"

//std::vector<UINT8> GenerateTextureData()
//{
//    int TextureWidth = 256;
//    int TextureHeight = 256;
//    int TexturePixelSize = 4;
//    const UINT rowPitch = TextureWidth * TexturePixelSize;
//    const UINT cellPitch = rowPitch >> 3;        // The width of a cell in the checkboard texture.
//    const UINT cellHeight = TextureWidth >> 3;    // The height of a cell in the checkerboard texture.
//    const UINT textureSize = rowPitch * TextureHeight;
//
//    std::vector<UINT8> data(textureSize);
//    UINT8* pData = &data[0];
//
//    for (UINT n = 0; n < textureSize; n += TexturePixelSize)
//    {
//        UINT x = n % rowPitch;
//        UINT y = n / rowPitch;
//        UINT i = x / cellPitch;
//        UINT j = y / cellHeight;
//
//        if (i % 2 == j % 2)
//        {
//            pData[n] = 0x00;        // R
//            pData[n + 1] = 0x00;    // G
//            pData[n + 2] = 0x00;    // B
//            pData[n + 3] = 0xff;    // A
//        }
//        else
//        {
//            pData[n] = 0xff;        // R
//            pData[n + 1] = 0xff;    // G
//            pData[n + 2] = 0xff;    // B
//            pData[n + 3] = 0xff;    // A
//        }
//    }
//
//    return data;
//}
//

Texture::Texture(const char* path, ComPtr<ID3D12Device>& device, ComPtr<ID3D12GraphicsCommandList>& commandList,
          ComPtr<ID3D12DescriptorHeap>& srvHeap, UINT index)
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

    ThrowIfFailed(device->CreateCommittedResource(
        &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
        D3D12_HEAP_FLAG_NONE,
        &textureDesc,
        D3D12_RESOURCE_STATE_COPY_DEST,
        nullptr,
        IID_PPV_ARGS(&m_texture)));

    const UINT64 uploadBufferSize = GetRequiredIntermediateSize(m_texture.Get(), 0, 1);

    // Create the GPU upload buffer.
    ThrowIfFailed(device->CreateCommittedResource(
        &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
        D3D12_HEAP_FLAG_NONE,
        &CD3DX12_RESOURCE_DESC::Buffer(uploadBufferSize),
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
    commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_texture.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE));

    // Describe and create a SRV for the texture.
    D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srvDesc.Format = textureDesc.Format;
    srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Texture2D.MipLevels = 1;
    auto handle = CD3DX12_CPU_DESCRIPTOR_HANDLE(srvHeap->GetCPUDescriptorHandleForHeapStart());
    UINT srvDescSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
    handle.Offset(index, srvDescSize);
    device->CreateShaderResourceView(m_texture.Get(), &srvDesc, handle);

}