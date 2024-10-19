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
        LEFT = 37,
        UP = 38,
        RIGHT = 39,
        DOWN = 40,
        SWITCH_CAMERA = 192,
    };
    constexpr auto KeyCount = magic_enum::enum_count<Key>();
    inline u32 KeyToIndex(Key key) { return magic_enum::enum_index(key).value(); }

    class KeyboardHandler 
    {
    public:
        virtual void onKeyDown(Key key) = 0;
    };

    class MouseHandler
    {
    public:
        virtual void onMouseDown(WPARAM btnState, int x, int y) = 0;
        virtual void onMouseUp(WPARAM btnState, int x, int y) = 0;
        virtual void onMouseMove(WPARAM btnState, int x, int y) = 0;
    };

    struct CallbackInfo
    {
        std::function<void()> ptr;
        bool oneTimeTouch = true; // if key is pressed function will be executed only once till the next press;
        void operator()(){ ptr();}
    };

    class KeyState
    {
    public:
        enum State
        {
            JustPressed,
            AlreadyPressed,
            Released,
            Off
        };
        bool isPressed() const { return m_state == JustPressed || m_state == AlreadyPressed; }
        bool isUp() const { return m_state == Released || m_state == Off;}
        State getState() const { return m_state; }
        void setPressed()
        {
            if (m_state == JustPressed)
                m_state = AlreadyPressed;
            else
                m_state = JustPressed;
        }
        void release()
        {
            m_state = Released;
        }

    private:
        State m_state=Off;
    };

    class InputManager
    {
    public:
        static InputManager& GetInputManager();
        LRESULT processInput(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
        void addCallback(Key key, CallbackInfo func)
        {
            m_callbacks[KeyToIndex(key)].push_back(func);
        }

        auto getKeyState(Key key) const { return m_pressed[KeyToIndex(key)]; }
    private:
        InputManager()=default;

        virtual void onMouseDown(WPARAM btnState, int x, int y) {}
        virtual void onMouseUp(WPARAM btnState, int x, int y){}
        virtual void onMouseMove(WPARAM btnState, int x, int y){}
        virtual void onKeyDown(Key key);
        virtual void onKeyUp(Key key);
        std::array<std::vector<CallbackInfo>, KeyCount> m_callbacks;
        std::array<KeyState, KeyCount> m_pressed;
    };

};
