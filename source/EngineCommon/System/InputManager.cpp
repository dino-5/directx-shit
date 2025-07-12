#include "InputManager.h"
#include "EngineCommon/util/ImGuiSettings.h"
#include "EngineCommon/util/Logger.h"
#include "third_party/imgui/imgui.h"
#include "third_party/imgui/backends/imgui_impl_dx12.h"
#include <windowsx.h>

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd,
                                                             UINT msg,
                                                             WPARAM wParam,
                                                             LPARAM lParam);

namespace engine::system
{

    InputManager& InputManager::GetInputManager()
    {
        static InputManager manager;
        return manager;
    }

    void InputManager::onKeyDown(Key key)
    {
        u32 index = KeyToIndex(key);
        m_pressed[index].setPressed();
    }

    void InputManager::onKeyUp(Key key)
    {
        u32 index = KeyToIndex(key);
        m_pressed[index].release();
    }

    LRESULT InputManager::processInput(HWND hwnd, 
                                       UINT msg, 
                                       WPARAM wParam,
                                       LPARAM lParam)
    {
        if (ImGui_ImplWin32_WndProcHandler(hwnd, msg, wParam, lParam))
            return 0;
        switch( msg )
        {
        case WM_LBUTTONDOWN:
        case WM_MBUTTONDOWN:
        case WM_RBUTTONDOWN:
            onMouseDown(wParam, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
            return 0;
        case WM_LBUTTONUP:
        case WM_MBUTTONUP:
        case WM_RBUTTONUP:
            onMouseUp(wParam, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
            return 0;
        case WM_MOUSEMOVE:
            onMouseMove(wParam, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
            return 0;

        case WM_KEYDOWN:
            if(wParam == VK_ESCAPE)
                PostQuitMessage(0);
            if (!magic_enum::enum_contains(Key(wParam)))
            {
                util::printInfo("key is pressed {} ", wParam);
                return 0;
            }
                onKeyDown(Key(wParam));
            return 0;
        case WM_KEYUP:
            if (!magic_enum::enum_contains(Key(wParam)))
                return 0;
            onKeyUp(Key(wParam));
            return 0;
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
        default:
            return DefWindowProc(hwnd, msg, wParam, lParam);
        }
    }
};
