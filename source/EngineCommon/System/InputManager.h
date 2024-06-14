#pragma once

#include <array>
#include <functional>
#include <vector>
#include <windows.h>
#include <third_party/magic_enum/include/magic_enum.hpp>
#include "EngineCommon/include/types.h"

namespace engine::system
{
    enum class Key {
        Q = 81,
        W = 87,
        A = 65,
        S = 83,
        D = 68,
        E = 69,
        R = 82,
        F = 70,
        N1 = 49,
        N2 = 50,
        N3 = 51,
        N4 = 52,
        SWITCH_CAMERA = 192,
    };
    constexpr auto KeyCount = magic_enum::enum_count<Key>();
    inline u32 KeyToIndex(Key key) { return magic_enum::enum_index(key).value(); }

    class KeyboardHandler 
    {
    public:
        virtual void OnKeyDown(Key key) = 0;
    };

    class MouseHandler
    {
    public:
        virtual void OnMouseDown(WPARAM btnState, int x, int y) = 0;
        virtual void OnMouseUp(WPARAM btnState, int x, int y) = 0;
        virtual void OnMouseMove(WPARAM btnState, int x, int y) = 0;
    };

    struct CallbackInfo
    {
        std::function<void()> ptr;
        bool oneTimeTouch = true; // if key is pressed function will be executed only once till the next press;
        void operator()(){ ptr();}
    };

    class InputManager
    {
    public:
        static InputManager& GetInputManager();
        LRESULT ProcessInput(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
        void AddCallback(Key key, CallbackInfo func)
        {
            m_callbacks[KeyToIndex(key)].push_back(func);
        }
    private:
        InputManager()=default;

        virtual void OnMouseDown(WPARAM btnState, int x, int y);
        virtual void OnMouseUp(WPARAM btnState, int x, int y);
        virtual void OnMouseMove(WPARAM btnState, int x, int y);
        virtual void OnKeyDown(Key key);
        virtual void OnKeyUp(Key key);
        std::array<std::vector<CallbackInfo>, KeyCount> m_callbacks;
        std::array<bool, KeyCount> m_pressed{false};
    };

};
