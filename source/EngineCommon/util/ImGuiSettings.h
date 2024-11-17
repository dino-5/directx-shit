#pragma once
#include <d3d12.h>
#include <string_view>

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
		static void EndFrame(ID3D12GraphicsCommandList* cmdList);
		static void Begin(std::string_view name);
		static void End();
		static bool SliderFloat(std::string_view name, float* ptr, float min, float max);
		static bool SliderFloat2(std::string_view name, float* ptr, float min, float max);
		static bool SliderFloat3(std::string_view name, float* ptr, float min, float max);
		static bool SliderFloat4(std::string_view name, float* ptr, float min, float max);
		static bool ColorEdit3(std::string_view label, float* col);
		static bool Button(std::string_view label);

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