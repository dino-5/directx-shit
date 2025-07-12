#pragma once
#include "EngineCommon/include/defines.h"
#include <string>
#include <memory>
#include <windows.h>
#include <utility>

class Window
{
public:
    Window(int width, int height, std::string_view name);
    Window() = default;

    virtual bool initialize();
    void setWindowHandler(HWND handler) { m_windowHandler = handler; }
    void setWidth(int w) { m_width = w; }
    int  getWidth() const { return m_width; }
    void setHeight(int h) { m_height = h; }
    int  getHeight() const { return m_height; }
    HWND getWindowHandle() const { return m_windowHandler; }

private:
    HWND m_windowHandler = 0;
    int m_width;
    int m_height;
    std::string m_windowName;
};

WNDCLASSEXA CreateWindowClass(const std::string_view );

class WindowApp : public Window {
public:
    static inline WindowApp* App = nullptr;
    LRESULT msgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

    WindowApp(int width, int height, std::string_view name);
    WindowApp() = default;
    virtual ~WindowApp() {}

    SHIT_ENGINE_NON_COPYABLE(WindowApp);

    bool initialize() override;
    void run();

    std::pair<int, int> getWindowSize();

protected:
    virtual void onResize(unsigned int width, unsigned int height);
    virtual void update() = 0;
    virtual void draw() = 0;
    virtual void destroy() = 0;
    virtual LRESULT processInput(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) 
    { return DefWindowProc(hwnd, msg, wParam, lParam); }
}; 
