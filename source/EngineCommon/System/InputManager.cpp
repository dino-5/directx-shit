#include "InputManager.h"
#include "EngineCommon/util/ImGuiSettings.h"
#include "EngineCommon/util/Logger.h"
#include "third_party/imgui/imgui.h"
#include "third_party/imgui/backends/imgui_impl_dx12.h"
#include <windowsx.h>

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

namespace engine::system
{

    InputManager& InputManager::GetInputManager()
    {
        static InputManager manager;
        return manager;
    }


    void InputManager::OnMouseDown(WPARAM btnState, int x, int y)
    {

    }
    
    void InputManager::OnMouseUp(WPARAM btnState, int x, int y)
    {

    }

    void InputManager::OnMouseMove(WPARAM btnState, int x, int y)
    {

    }

    void InputManager::OnKeyDown(Key key)
    {
        u32 index = KeyToIndex(key);
        bool isPressed = m_pressed[index];
        for (auto& func : m_callbacks[index])
        {
            if (!isPressed)
                func();
            else if (isPressed && !func.oneTimeTouch)
                func();
        }
        m_pressed[index] = true;
    }

    void InputManager::OnKeyUp(Key key)
    {
        u32 index = KeyToIndex(key);
        m_pressed[index] = false;
    }

    LRESULT InputManager::ProcessInput(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
    {
        if (ImGui_ImplWin32_WndProcHandler(hwnd, msg, wParam, lParam))
            return 0;
        switch( msg )
        {
        case WM_LBUTTONDOWN:
        case WM_MBUTTONDOWN:
        case WM_RBUTTONDOWN:
            OnMouseDown(wParam, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
            return 0;
        case WM_LBUTTONUP:
        case WM_MBUTTONUP:
        case WM_RBUTTONUP:
            OnMouseUp(wParam, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
            return 0;
        case WM_MOUSEMOVE:
            OnMouseMove(wParam, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
            return 0;

        case WM_KEYDOWN:
            if(wParam == VK_ESCAPE)
                PostQuitMessage(0);
            if (!magic_enum::enum_contains(Key(wParam)))
            {
                util::PrintInfo("key is pressed {} ", wParam);
                return 0;
            }
                OnKeyDown(Key(wParam));
            return 0;
        case WM_KEYUP:
            if (!magic_enum::enum_contains(Key(wParam)))
                return 0;
            OnKeyUp(Key(wParam));
            return 0;
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
        default:
            return DefWindowProc(hwnd, msg, wParam, lParam);
        }
    }
};
