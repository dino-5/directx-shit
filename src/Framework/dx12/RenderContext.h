#pragma once

#include "Framework/dx12/Device.h"
#include "Framework/dx12/SwapChain.h"
#include "Framework/dx12/PSO.h"
#include "Framework/include/defines.h"

class RenderContext
{
public:
	SHIT_ENGINE_SINGLETONE(RenderContext);
	RenderContext(int width, int height, Format format, HWND window);
private:
	Device m_device;
	SwapChain m_swapChain;
	PSO* m_currentPSO;
	
};