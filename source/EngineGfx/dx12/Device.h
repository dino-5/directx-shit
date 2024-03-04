#ifndef DEVICE_H
#define DEVICE_H
#include <d3d12.h>
#include <windows.h>
#include <dxgi1_6.h>
#include <D3Dcompiler.h>
#include <DirectXMath.h>
#include "EngineCommon/include/common.h"
#include "EngineCommon/include/defines.h"
#include "EngineCommon/include/types.h"
#include "EngineGfx/dx12/d3dx12.h"

class Texture;
class DescriptorHeap;

class Device
{
public:
	static inline Device* device = nullptr;
	void Initialize();
	SHIT_ENGINE_GET_D3D12COMPONENT(ID3D12Device, Device, m_device);
	SHIT_ENGINE_GET_D3D12COMPONENT(IDXGIFactory4, Factory, m_factory);

	void CreateCommandList(ID3D12GraphicsCommandList* &list, ID3D12CommandAllocator* &allocator);
	void CreateCommandAllocator(ID3D12CommandAllocator* &);
	void CreateFence(ID3D12Fence**);

	void Reset() { m_factory->Release(); m_device->Release(); }

	ID3D12Device* native() { return m_device; }

private:
		
	void GetHardwareAdapter(
		IDXGIAdapter1** ppAdapter,
		bool requestHighPerformanceAdapter=true);

private:
	IDXGIFactory4* m_factory = nullptr;
	ID3D12Device* m_device   = nullptr;
};

#endif

