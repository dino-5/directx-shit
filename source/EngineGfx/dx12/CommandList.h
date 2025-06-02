#pragma once

#include <d3d12.h>
#include "EngineGfx/dx12/Device.h"
#include "EngineCommon/include/types.h"
#include "EngineCommon/System/config.h"

namespace engine::graphics
{
using namespace engine;
struct CommandList
{
    CommandList() {}
    CommandList(Device& dev) {
        for (uint i = 0; i < config::NumFrames; i++)
            dev.createCommandAllocator(alloc[i]);
        dev.createCommandList(list, alloc[0]);
    }

    ID3D12GraphicsCommandList** getListAddress() { return &list; }
    ID3D12GraphicsCommandList* operator->() { return list; }
    ID3D12GraphicsCommandList* reset(uint i)
    {
        alloc[i]->Reset();
        list->Reset(alloc[i], nullptr);
        return list;
    }

    ID3D12GraphicsCommandList* list = nullptr;
    ID3D12CommandAllocator* alloc[config::NumFrames];

};
}
