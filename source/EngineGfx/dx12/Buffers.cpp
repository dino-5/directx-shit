#include "Buffers.h"
#include "Resource.h"
#include "dx12_includes.hpp"
#include "third_party/magic_enum/include/magic_enum.hpp"
using namespace magic_enum::bitwise_operators;

namespace engine::graphics{

std::vector<Buffer> Buffer::s_uploadBuffers;
std::vector<Buffer::UploadBufferInfo> Buffer::s_uploadBufferInfo;

inline void Buffer::copyDataToGPU(
    const void* data,
    Device& device, CommandList& cmdList,
    ResourceState state)
{
    D3D12_SUBRESOURCE_DATA subresData = {};
    subresData.pData = data;
    subresData.RowPitch = m_bufferSize;
    subresData.SlicePitch = 1;

    BufferHandle uploadBufferHandle = GetUploadBufferHandle(
        device, cmdList, m_bufferSize);

    transition(cmdList.list, ResourceState::COPY_DEST);
    UpdateSubresources(cmdList.list,
                       resource(),
                       GetUploadBuffer(uploadBufferHandle).resource(),
                       0, 0, 1, &subresData);
    transition(cmdList.list, state);
}

void Buffer::init(
    Device& device, CommandList& cmdList,
    const BufferDescription& bufferDesc,
    ResourceFlags flags)
{
    m_type = bufferDesc.type;
    m_elementSize = bufferDesc.elementSize;

    m_bufferSize = bufferDesc.elementCount * m_elementSize;
    m_bufferSize = (u32)(m_type & BufferType::CONSTANT)  
                ? util::CalcConstantBufferByteSize(m_bufferSize) 
                : m_bufferSize;

    ResourceDescription desc{
        .format = DXGI_FORMAT_UNKNOWN,
        .width = m_bufferSize,
        .height = 1,
        .depthOrArraySize = 1,
        .dimension = D3D12_RESOURCE_DIMENSION_BUFFER,
        .flags = flags,
        .createState = bufferDesc.state,
        .heapType = bufferDesc.type != BufferType::UPLOAD 
                    && bufferDesc.type != BufferType::CONSTANT 
                    ? D3D12_HEAP_TYPE_DEFAULT : D3D12_HEAP_TYPE_UPLOAD,
        .name = bufferDesc.name.data()
    };

    DescriptorProperties descriptorProps;
    if((u32)(bufferDesc.type & (BufferType::CONSTANT | BufferType::CUSTOM)))
    {
        descriptorProps = {
            .descriptor = bufferDesc.type == BufferType::CUSTOM 
              ? DescriptorFlags::ShaderResource : DescriptorFlags::ConstantBuffer,
            .viewDimension = D3D12_SRV_DIMENSION_BUFFER,
            .bufferStride = m_elementSize,
            .numElements = bufferDesc.elementCount
        };

        if((u32)(flags & ResourceFlags::UNOURDERED))
            descriptorProps.descriptor |= DescriptorFlags::UnorderedAccess;
    }
    Resource::initResource(device, desc, descriptorProps);

    if(bufferDesc.data)
        copyDataToGPU(bufferDesc.data,
                      device, cmdList, bufferDesc.state);
}

};
