#pragma once
#include <cstring>
#include <d3d12.h>
#include <string_view>
#include <vector>
#include "EngineCommon/System/config.h"
#include "EngineCommon/include/types.h"
#include "Device.h"
#include "Resource.h"
#include "CommandList.h"
#include "d3dx12.h"
#include "dx12_includes.hpp"

#include "third_party/magic_enum/include/magic_enum.hpp"

namespace engine::graphics {
using namespace magic_enum::bitwise_operators;
using engine::util::CalcConstantBufferByteSize;

enum class BufferType : u16 {
  NONE = 0,
  VERTEX = 1 << 0,
  INDEX = 1 << 1,
  CONSTANT = 1 << 2,
  UPLOAD = 1 << 3,
  CUSTOM = 1 << 4
};

struct BufferDescription {
  template <typename T>
  BufferDescription(T *aData, u32 aElementCount,
                    std::string_view aName,
                    ResourceState aState =
                        ResourceState::GENERIC_READ_STATE,
                    BufferType aType = BufferType::CUSTOM)
      : data((void *)aData), elementCount(aElementCount),
        elementSize(sizeof(T)), name(aName), state(aState), type(aType) {}

  BufferDescription(u32 aSize, std::string_view aName,
                    ResourceState aState =
                        ResourceState::GENERIC_READ_STATE,
                    BufferType aType = BufferType::CUSTOM)
      : data(nullptr), elementCount(1), elementSize(aSize), name(aName),
        state(aState), type(aType) {}

  BufferDescription(u32 aElementCount, u32 aElementSize, std::string_view aName,
                    ResourceState aState = ResourceState::GENERIC_READ_STATE,
                    BufferType aType = BufferType::CUSTOM)
      : data(nullptr), elementCount(aElementCount), elementSize(aElementSize),
        name(aName), state(aState), type(aType) {}
  BufferDescription() = default;

  const void *data = nullptr;
  u32 elementCount{};
  u32 elementSize = 0;
  std::string_view name;
  ResourceState state{};
  BufferType type{};
};

template<typename T>
BufferDescription getIndexBufferDescription(T* data, u32 elementCount)
{
    return BufferDescription(
                        data,
                        elementCount,
                        "IndexBuffer",
                        ResourceState::INDEX_BUFFER,
                        BufferType::INDEX);
}

template<typename T>
BufferDescription getVertexBufferDescription(T* data, u32 elementCount)
{
    return BufferDescription(
                    data,
                    elementCount,
                    "VertexBuffer",
                    ResourceState::VERTEX_CONSTANT_BUFFER,
                    BufferType::VERTEX);
}

template<typename T>
BufferDescription getCustomBufferDescription(T* data, u32 elementCount)
{
    return BufferDescription(
                    data,
                    elementCount,
                    "CustomBuffer",
                    ResourceState::COMMON,
                    BufferType::CUSTOM);
}

inline BufferDescription getConstantBufferDescription(u32 elementCount,
                                               u32 elementSize)
{
    return BufferDescription(elementCount, elementSize,
                                   "ConstantBuffer",
                                   ResourceState::VERTEX_CONSTANT_BUFFER,
                                   BufferType::CONSTANT);
}

inline BufferDescription getUploadBufferDescription(u32 bufferSize)
{
    return BufferDescription(
                bufferSize,
                "UploadBuffer",
                ResourceState::COPY_SOURCE, BufferType::UPLOAD);
}

class Buffer : public Resource
{
public:
  Buffer() = default;

  template <typename T>
  Buffer(Device& device,
         CommandList& cmdList,
         T *data,
         u32 elementCount,
         std::string_view name)
  {
    init(device, cmdList, BufferDescription(data, elementCount, name));
  }

  Buffer(Device& device,
         CommandList& cmdList,
         const BufferDescription &desc) {
    init(device, cmdList, desc);
  }

  void init(Device& device,
            CommandList& cmdList, const BufferDescription &desc);

  u32 getDescriptorHeapIndex() { return srv.getDescriptorIndex(); }
  u32 getBufferSize() const { return m_bufferSize; }
  u32 getElementSize() const { return m_elementSize; }

  void copyDataToGPU(const void *data,
                     Device& device,
                     CommandList& cmdList,
                     ResourceState state);

protected:
  struct UploadBufferInfo {
    u32 size;
    bool isUsed;
  };
  using BufferHandle = u32;
  static BufferHandle GetUploadBufferHandle(Device& device, 
                                            CommandList& cmdList,
                                            u32 bufferSize) {
    for (u32 i = 0; i < (u32)s_uploadBufferInfo.size(); ++i) {
      auto &buffer = s_uploadBufferInfo[i];
      if (buffer.size >= bufferSize && !buffer.isUsed) {
        buffer.isUsed = true;
        return i;
      }
    }
    s_uploadBuffers.push_back(Buffer(
            device, cmdList, getUploadBufferDescription(bufferSize)));
    s_uploadBufferInfo.push_back({bufferSize, true});
    return (u32)s_uploadBuffers.size() - 1;
  }
  static void ReleaseUploadBuffer(BufferHandle handle) {
    s_uploadBufferInfo[handle].isUsed = false;
  }
  static Buffer &GetUploadBuffer(BufferHandle handle) {
    return s_uploadBuffers[handle];
  }

  static std::vector<Buffer> s_uploadBuffers;
  static std::vector<UploadBufferInfo> s_uploadBufferInfo;
  u32 m_elementSize;
  u32 m_bufferSize;
  BufferType m_type;
};

class ConstantBuffer : public Buffer {
public:
  template <typename T>
  ConstantBuffer(Device& device, CommandList& cmdList, T *data, u32 elementCount)
      : Buffer(
        device, cmdList,
        getConstantBufferDescription(elementCount, sizeof(T)))
  {
    CD3DX12_RANGE readRange(0, 0);
    ThrowIfFailed(
        resource()->Map(0, &readRange, reinterpret_cast<void **>(&m_buffer)));
    if(data)
        update(data);
  }
  ~ConstantBuffer() {
    CD3DX12_RANGE readRange(0, 0);
    resource()->Unmap(0, &readRange);
  }
  ConstantBuffer() = default;

  template <typename T> void update(T *data, u32 elementNumber = 0) {
    memcpy(&m_buffer[elementNumber * m_elementSize], data, sizeof(T));
  }

private:
  char *m_buffer = nullptr;
};

struct BufferObject {
  template <typename T>
  void create(Device& device,
              CommandList& cmdList,
              const BufferDescription &aDesc) {
    assert(aDesc.type != BufferType::NONE);

    data.resize(aDesc.elementSize * aDesc.elementCount);
    memcpy(aDesc.data, data.data(), aDesc.elementSize * aDesc.elementCount);

    buffer.init(device, cmdList, aDesc);
  }
  void update(Device& device, CommandList& cmdList) {
    buffer.copyDataToGPU(data.data(), device, cmdList, buffer.getCurrentState());
  }

  Buffer buffer;
  std::vector<char *> data;
};

inline D3D12_INDEX_BUFFER_VIEW GetIndexBufferView(Buffer &buffer) {
  D3D12_INDEX_BUFFER_VIEW view;
  view.BufferLocation = buffer.resource()->GetGPUVirtualAddress();
  view.Format = DXGI_FORMAT_R32_UINT;
  view.SizeInBytes = buffer.getBufferSize();
  return view;
}

inline D3D12_VERTEX_BUFFER_VIEW GetVertexBufferView(Buffer &buffer) {
  D3D12_VERTEX_BUFFER_VIEW view;
  view.BufferLocation = buffer.resource()->GetGPUVirtualAddress();
  view.StrideInBytes = buffer.getElementSize();
  view.SizeInBytes = buffer.getBufferSize();
  return view;
}

}; // namespace engine::graphics
