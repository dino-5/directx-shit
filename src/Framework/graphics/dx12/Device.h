#ifndef DEVICE_H
#define DEVICE_H
#include<d3d12.h>
#include "Framework/include/common.h"
#include "Framework/include/defines.h"
#include "Framework/include/d3dx12.h"
#include "Framework/include/types.h"
#include <windows.h>
#include <dxgi1_6.h>
#include <D3Dcompiler.h>
#include <DirectXMath.h>

class Texture;
class DescriptorHeap;


class Device
{
public:
	static inline Device* device = nullptr;
	void Initialize();
	SHIT_ENGINE_GET_D3D12COMPONENT(ID3D12Device, Device, m_device);
	SHIT_ENGINE_GET_D3D12COMPONENT(IDXGIFactory4, Factory, m_factory);

	void CreateCommandList(ComPtr<ID3D12GraphicsCommandList>&, ComPtr<ID3D12CommandAllocator>&);
	void CreateCommandAllocator(ComPtr<ID3D12CommandAllocator>&);
	void CreateFence(ComPtr<ID3D12Fence>&);

	void Reset() { m_factory.Reset(); m_device.Reset(); }

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

