#include "include/ImGuiSettings.h"
#include "imgui/backends/imgui_impl_win32.h"
#include "imgui/backends/imgui_impl_dx12.h"

void ImGuiSettings::Init(HWND hwnd, ID3D12Device* device, int numFrames)
{
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImGui::StyleColorsDark();
    //ImGui::StyleColorsClassic();
    g_pd3dDevice = device;

    {
        D3D12_DESCRIPTOR_HEAP_DESC desc = {};
        desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
        desc.NumDescriptors = 1;
        desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
		g_pd3dDevice->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&g_pd3dSrvDescHeap));
    }
    // Setup Platform/Renderer backends
    ImGui_ImplWin32_Init(hwnd);
    ImGui_ImplDX12_Init(g_pd3dDevice, 3,
        DXGI_FORMAT_R8G8B8A8_UNORM, g_pd3dSrvDescHeap,
        g_pd3dSrvDescHeap->GetCPUDescriptorHandleForHeapStart(),
        g_pd3dSrvDescHeap->GetGPUDescriptorHandleForHeapStart());
}

void ImGuiSettings::AddItem(ImGuiElement element)
{
	elements.push_back(element);
}

void ImGuiSettings::Draw()
{
	for (int i = 0; i < elements.size(); i++)
	{
		if (ImGui::Begin(elements[i].name))
		{
			if (elements[i].capacity == 3)
				ImGui::SliderFloat3("name", elements[i].array[0], elements[i].min, elements[i].max);

		}
	}
	ImGui::Render();
}

void ImGuiSettings::StartFrame()
{
	ImGui_ImplDX12_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
}

void ImGuiSettings::EndFrame()
{
	ImGui::Render();
}

