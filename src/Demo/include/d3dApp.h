#pragma once

#include "framework/include/Util.h"
#include "framework/dx12/DescriptorHeap.h"
#include "framework/include/common.h"
#include "Framework/dx12/Resource.h"
#include "Framework/System/Window.h"
class D3DApp
{
	virtual void OnMouseDown(WPARAM btnState, int x, int y){ }
	virtual void OnMouseUp(WPARAM btnState, int x, int y)  { }
	virtual void OnMouseMove(WPARAM btnState, int x, int y){ }
	virtual void OnKeyDown(Key key) { }
	virtual void Destroy() = 0;

protected:

	bool InitDirect3D();
	void CreateCommandObjects();
	void FlushCommandQueue();

    ComPtr<ID3D12Fence> mFence;
    UINT64 mCurrentFence = 0;
	
    ComPtr<ID3D12CommandQueue> mCommandQueue;
    ComPtr<ID3D12CommandAllocator> mDirectCmdListAlloc;
    ComPtr<ID3D12GraphicsCommandList> mCommandList;

	int mCurrBackBuffer = 0;

    D3D12_VIEWPORT mScreenViewport; 
    D3D12_RECT mScissorRect;
};

