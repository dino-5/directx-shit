#pragma once
#include "include/common.h"
#include <string>
#include "Handlers.h"


class Window
{
public:
	Window(HINSTANCE inst, int width, int height, std::string name);
	Window() = default;

	static WNDCLASS CreateWindowClass(HINSTANCE, std::string);

	void SetWindowHandler(HWND handler) { m_windowHandler = handler; }
	void SetWidth(int w) { width = w; }
	void SetHeight(int h) { height = h; }
	HWND GetWindowHandler() const { return m_windowHandler; }
private:
	HWND m_windowHandler;
	int width;
	int height;
	std::string windowName;
};

class WindowApp
{
public:
	static LRESULT MsgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
	static inline WindowApp* app=nullptr;
	static WindowApp* GetWindowApp()
	{
		return app;
	}

protected:
	WindowApp(HINSTANCE inst, int width, int height, std::string name);
	WindowApp() = default;
	WindowApp operator=(WindowApp&) = delete;
	WindowApp(WindowApp&) = delete;
	Window m_window;
	MouseHandler* mouse;
	KeyboardHandler* keyboard;
	HINSTANCE m_inst;
};
