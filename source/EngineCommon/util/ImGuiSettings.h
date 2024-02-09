#pragma once
#include <d3d12.h>
#include <dxgi1_4.h>
#include <vector>
#include <string>

namespace engine::util
{
	class ImGuiSettings
	{
	public:
		struct ImGuiElement
		{
			const char* name;
			float** array; // {&di
			int capacity;
			float min;
			float max;

		};
	public:
		ImGuiSettings() = delete;
		static void Init(HWND hwnd, ID3D12Device*, int numFrames);
		static void StartFrame();
		static void EndFrame();
		static void Begin(std::string name);
		static void End();
		static bool SliderFloat(std::string name, float* ptr, float min, float max);
		static bool SliderFloat2(std::string name, float* ptr, float min, float max);
		static bool SliderFloat3(std::string name, float* ptr, float min, float max);
		static bool SliderFloat4(std::string name, float* ptr, float min, float max);

		static ID3D12DescriptorHeap* GetDescriptorHeap() {
			return g_pd3dSrvDescHeap;
		}

	private:
		struct FrameContext
		{
			ID3D12CommandAllocator* CommandAllocator;
			UINT64                  FenceValue;
		};
		static inline float clear_color[] = { 0.45f, 0.55f, 0.60f, 1.00f };
		static inline ID3D12Device* g_pd3dDevice = NULL;
		static inline ID3D12DescriptorHeap* g_pd3dSrvDescHeap = NULL;


	};
};