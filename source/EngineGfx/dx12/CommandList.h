#pragma once

#include <d3d12.h>
#include "EngineGfx/dx12/Device.h"
#include "EngineCommon/include/types.h"
#include "EngineCommon/System/config.h"

namespace engine::graphics
{
using namespace engine;
class CommandList
{
public:
    CommandList() = default;
    CommandList(Device& dev) { initialize(dev); }
    void initialize(Device& dev)
    {
        for (uint i = 0; i < config::NumFrames; i++)
            dev.createCommandAllocator(m_alloc[i]);
        dev.createCommandList(m_list, m_alloc[0]);
    }

    ID3D12GraphicsCommandList* getList() { return m_list; }
    ID3D12GraphicsCommandList** getListAddress() { return &m_list; }
    ID3D12GraphicsCommandList* operator->() { return m_list; }
    ID3D12GraphicsCommandList* reset(uint i)
    {
        m_alloc[i]->Reset();
        m_list->Reset(m_alloc[i], nullptr);
        return m_list;
    }

private:
    ID3D12GraphicsCommandList* m_list = nullptr;
    ID3D12CommandAllocator* m_alloc[config::NumFrames];

};
}
