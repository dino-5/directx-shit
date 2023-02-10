#ifndef DEVICE_H
#define DEVICE_H
#include<d3d12.h>
#include "Framework/include/common.h"
#include "Framework/include/defines.h"
#include "Framework/include/d3dx12.h"
#include <windows.h>
#include <dxgi1_6.h>
#include <D3Dcompiler.h>
#include <DirectXMath.h>

class Texture;
class DescriptorHeap;


class Device
{
public:
	void Initialize();
	SHIT_ENGINE_GET_D3D12COMPONENT(ID3D12Device, Device, m_device);
	SHIT_ENGINE_GET_D3D12COMPONENT(IDXGIFactory4, Factory, m_factory);

	void CreateCommandList(ID3D12GraphicsCommandList*);

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

