#pragma once

#include "Framework/dx12/Device.h"
#include "Framework/dx12/SwapChain.h"
#include "Framework/dx12/PSO.h"
#include "Framework/dx12/CommandQueue.h"
#include "Framework/dx12/Resource.h"
#include "Framework/dx12/DescriptorHeap.h"
#include "Framework/include/defines.h"

struct RenderSettings
{
	int width;
	int height;
};

class RenderContext
{
public:
	SHIT_ENGINE_SINGLETONE(RenderContext);
	void Initialize();
	void ResetSwapChain(SwapChainSettings set);
private:
	Device m_device;
	CommandQueue m_graphicsQueue;
	SwapChain m_swapChain;
	Resource m_dsvBuffer;

	DescriptorHeap m_rtvHeap;
	DescriptorHeap m_dsvHeap;

	PSO* m_currentPSO;
	ComPtr<ID3D12GraphicsCommandList> m_commandList;
};