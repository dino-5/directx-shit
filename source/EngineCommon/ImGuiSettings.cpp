#include "EngineCommon/util/ImGuiSettings.h"
#include "third_party/imgui/imgui.h"
#include "third_party/imgui/backends/imgui_impl_dx12.h"
#include "third_party/imgui/backends/imgui_impl_win32.h"

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

    void ImGuiSettings::EndFrame(ID3D12GraphicsCommandList* cmdList)
    {
        ImGui::Render();
        auto heap = GetDescriptorHeap();
        cmdList->SetDescriptorHeaps(1, &heap);
        ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), cmdList);
    }


    void ImGuiSettings::Begin(std::string_view name)
    {
        ImGui::Begin(name.data());
    }

    void ImGuiSettings::End()
    {
        ImGui::End();
    }

    bool ImGuiSettings::SliderFloat2(std::string_view name, float* ptr, float min, float max)
    {
        return ImGui::SliderFloat2(name.data(), ptr, min, max);
    }

    bool ImGuiSettings::SliderFloat3(std::string_view name, float* ptr, float min, float max)
    {
        return ImGui::SliderFloat3(name.data(), ptr, min, max);
    }

    bool ImGuiSettings::SliderFloat4(std::string_view name, float* ptr, float min, float max)
    {
        return ImGui::SliderFloat4(name.data(), ptr, min, max);
    }

    bool ImGuiSettings::ColorEdit3(std::string_view label, float* col)
    {
        return ImGui::ColorEdit3(label.data(), col);
    }

    bool ImGuiSettings::Button(std::string_view label)
    {
        return ImGui::Button(label.data());
    }

    bool ImGuiSettings::SliderFloat(std::string_view name, float* ptr, float min, float max)
    {
        return ImGui::SliderFloat(name.data(), ptr, min, max);
    }
};
