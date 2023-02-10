#pragma once

#include "framework/include/Util.h"
#include "framework/dx12/DescriptorHeap.h"
#include "framework/include/common.h"
#include "Framework/dx12/Resource.h"
#include "Framework/System/Window.h"
class D3DApp
{
protected:

	void FlushCommandQueue();

    ComPtr<ID3D12Fence> mFence;
    UINT64 mCurrentFence = 0;
	
    D3D12_VIEWPORT mScreenViewport; 
    D3D12_RECT mScissorRect;
};

