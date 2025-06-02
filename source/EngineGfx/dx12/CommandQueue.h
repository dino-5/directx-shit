#pragma once
#include <d3d12.h>
#include "EngineCommon/include/defines.h"
#include "Device.h"

namespace engine::graphics
{

struct CommandQueueDesc
{
    D3D12_COMMAND_LIST_TYPE  type = D3D12_COMMAND_LIST_TYPE_DIRECT;
};

class CommandQueue
{
public:
    CommandQueue() = default;
    CommandQueue(
        Device& device,
        CommandQueueDesc desc = CommandQueueDesc())
    {
        D3D12_COMMAND_QUEUE_DESC descQ{
            .Type = desc.type,
            .Priority = 0,
            .Flags = D3D12_COMMAND_QUEUE_FLAG_NONE,
            .NodeMask = 0
        };
        device.device->CreateCommandQueue(&descQ, IID_PPV_ARGS(&queue));
    }

    ID3D12CommandQueue* operator->() { return queue; }
    void reset() { queue->Release(); }

    ID3D12CommandQueue* queue = nullptr;
};
};

