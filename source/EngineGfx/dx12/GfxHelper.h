#pragma once

#include "Resource.h"

using namespace engine::graphics;

inline ResourceDescription getDepthStencilDesc(u32 width, u32 height)
{
    return{
            .format = DXGI_FORMAT_D24_UNORM_S8_UINT,
            .width= width,
            .height = height,
            .depthOrArraySize = 1,
            .dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D,
            .flags = ResourceFlags::DEPTH_STENCIL,
            .createState = ResourceState::DEPTH_WRITE ,
            .heapType = D3D12_HEAP_TYPE_DEFAULT,
            .name = "depthStencil"
    };
}

