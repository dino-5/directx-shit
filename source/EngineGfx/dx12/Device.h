#ifndef DEVICE_H
#define DEVICE_H
#include <d3d12.h>
#include <windows.h>
#include <dxgi1_6.h>
#include <D3Dcompiler.h>
#include <DirectXMath.h>
#include "EngineCommon/include/defines.h"
#include "EngineCommon/include/types.h"
#include "EngineGfx/dx12/d3dx12.h"
#include "EngineGfx/dx12/dx12_includes.hpp"

class Texture;
class DescriptorHeap;

namespace engine::graphics {

	class Device
	{
	public:
		static inline Device* device = nullptr;
		Device() { initialize(); }
		void initialize();
		SHIT_ENGINE_GET_D3D12COMPONENT(IDXGIFactory4, Factory, m_factory);
		SHIT_ENGINE_GET_D3D12COMPONENT(ID3D12Device, Device, m_device);

		void createCommandList(ID3D12GraphicsCommandList*& list, ID3D12CommandAllocator*& allocator);
		void createCommandAllocator(ID3D12CommandAllocator*&);
		void createFence(ID3D12Fence**);

		void reset() { 
			m_factory->Release(); m_factory = nullptr;
			m_device->Release(); m_device = nullptr;
		}

		ID3D12Device* native() { return m_device; }

	private:

		void getHardwareAdapter(
			IDXGIAdapter1** ppAdapter,
			bool requestHighPerformanceAdapter = true);

	private:
		IDXGIFactory4* m_factory = nullptr;
		ID3D12Device* m_device = nullptr;
	};

	struct GfxContext
	{
		ID3D12Device* device = nullptr;
		ID3D12GraphicsCommandList* cmdList= nullptr;
	};
};

#endif

