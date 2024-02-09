#include "EngineCommon/util/ImGuiSettings.h"
#include "third_party/imgui/imgui.h"
#include "third_party/imgui/backends/imgui_impl_dx12.h"
#include "third_party/imgui/backends/imgui_impl_win32.h"
#include "third_party/imgui/backends/imgui_impl_win32.h"
#include "third_party/imgui/backends/imgui_impl_dx12.h"

namespace engine::util
{

    void ImGuiSettings::Init(HWND hwnd, ID3D12Device* device, int numFrames)
    {
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO(); (void)io;
        ImGui::StyleColorsDark();
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


    void ImGuiSettings::Begin(std::string name)
    {
        ImGui::Begin(name.c_str());
    }

    void ImGuiSettings::End()
    {
        ImGui::End();
    }

    bool ImGuiSettings::SliderFloat2(std::string name, float* ptr, float min, float max)
    {
        return ImGui::SliderFloat2(name.c_str(), ptr, min, max);
    }

    bool ImGuiSettings::SliderFloat3(std::string name, float* ptr, float min, float max)
    {
        return ImGui::SliderFloat3(name.c_str(), ptr, min, max);
    }

    bool ImGuiSettings::SliderFloat4(std::string name, float* ptr, float min, float max)
    {
        return ImGui::SliderFloat4(name.c_str(), ptr, min, max);
    }

    bool ImGuiSettings::SliderFloat(std::string name, float* ptr, float min, float max)
    {
        return ImGui::SliderFloat(name.c_str(), ptr, min, max);
    }
};
