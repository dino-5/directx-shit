#pragma once
#include "imgui/imgui.h"
#include "imgui/backends/imgui_impl_dx12.h"
#include "imgui/backends/imgui_impl_win32.h"
#include <d3d12.h>
#include <dxgi1_4.h>
#include <vector>

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
	static inline std::vector<ImGuiElement> elements;
public:
	ImGuiSettings() = delete;
	static void Init(HWND hwnd, ID3D12Device*, int numFrames);
	static void Draw();
	static void StartFrame();
	static void EndFrame();
	static ID3D12DescriptorHeap* GetDescriptorHeap() {
		return g_pd3dSrvDescHeap;
	}

	static void AddItem(ImGuiElement);


private:
	struct FrameContext
	{
		ID3D12CommandAllocator* CommandAllocator;
		UINT64                  FenceValue;
	};
    static inline ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
	static inline ID3D12Device*                g_pd3dDevice = NULL;
	static inline ID3D12DescriptorHeap*        g_pd3dSrvDescHeap = NULL;


};