#ifndef DEVICE_H
#define DEVICE_H
#include<d3d12.h>
#include "../include/common.h"
#include <windows.h>
#include <dxgi1_6.h>
#include <D3Dcompiler.h>
#include <DirectXMath.h>
#include "../include/d3dx12.h"

class Texture;
class DescriptorHeap;

class Device
{
public:

	void Initialize();
	Device() = delete;
	ID3D12Device* GetDevice();
	IDXGIFactory4* GetFactory();

private:
		
	void GetHardwareAdapter(
		IDXGIFactory1* pFactory,
		IDXGIAdapter1** ppAdapter,
		bool requestHighPerformanceAdapter=true);

private:
	ComPtr<IDXGIFactory4> m_factory = nullptr;
	ComPtr<ID3D12Device> m_device   = nullptr;
};

#endif

