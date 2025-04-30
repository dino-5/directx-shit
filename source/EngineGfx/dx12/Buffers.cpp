#include "Buffers.h"
#include "third_party/magic_enum/include/magic_enum.hpp"
using namespace magic_enum::bitwise_operators;

namespace engine::graphics{

std::vector<Buffer> Buffer::s_uploadBuffers;
std::vector<Buffer::UploadBufferInfo> Buffer::s_uploadBufferInfo;

inline void Buffer::copyDataToGPU(const void* data, const GfxContext& context, ResourceState state)
{
    D3D12_SUBRESOURCE_DATA subresData = {};
    subresData.pData = data;
    subresData.RowPitch = m_bufferSize;
    subresData.SlicePitch = 1;

    BufferHandle uploadBufferHandle = GetUploadBufferHandle(context, m_bufferSize);

    transition(context.cmdList, ResourceState::COPY_DEST);
    UpdateSubresources(context.cmdList, resource(), GetUploadBuffer(uploadBufferHandle).resource(), 0, 0, 1, &subresData);
    transition(context.cmdList, state);
}

void Buffer::init(const GfxContext& context, const BufferDescription& bufferDesc)
{
    m_type = bufferDesc.type;
    m_elementSize = (u32)(bufferDesc.type & BufferType::CONSTANT) != 0 ? util::CalcConstantBufferByteSize(bufferDesc.elementSize) : bufferDesc.elementSize;
    m_bufferSize = bufferDesc.elementCount * m_elementSize;
    ResourceDescription desc{
            .format = DXGI_FORMAT_UNKNOWN,
            .width = m_bufferSize,
            .height = 1,
            .depthOrArraySize = 1,
            .dimension = D3D12_RESOURCE_DIMENSION_BUFFER,
            .flags = ResourceFlags::NONE,
            .createState = bufferDesc.state,
            .heapType = bufferDesc.type != BufferType::UPLOAD &&  bufferDesc.type != BufferType::CONSTANT ? D3D12_HEAP_TYPE_DEFAULT : D3D12_HEAP_TYPE_UPLOAD,
            .name = bufferDesc.name.data()
    };

    DescriptorProperties descriptorProps;
    if(bufferDesc.type == BufferType::CONSTANT || bufferDesc.type == BufferType::CUSTOM)
    {
        descriptorProps = {
            .descriptor = bufferDesc.type == BufferType::CUSTOM ? DescriptorFlags::ShaderResource : DescriptorFlags::ConstantBuffer,
            .viewDimension = D3D12_SRV_DIMENSION_BUFFER,
            .bufferStride = m_elementSize,
            .numElements = bufferDesc.elementCount
        };
    }
    Resource::initResource(context.device, desc, descriptorProps);

    if(bufferDesc.data)
        copyDataToGPU(bufferDesc.data, context, bufferDesc.state);
}

};
