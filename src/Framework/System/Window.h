#pragma once
#include "include/common.h"
#include "include/defines.h"
#include <string>
#include "Handlers.h"
#include <memory>


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

class WindowApp : public Window
{
public:
	LRESULT MsgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
	WindowApp(HINSTANCE inst, int width, int height, std::string name);

	SHIT_ENGINE_SINGLETONE(WindowApp);

	void SetMouseHandler(MouseHandler* handler) { mouse = handler; }
	void SetKeyboardHandler(KeyboardHandler* handler) { keyboard = handler; }

protected:
	MouseHandler* mouse;
	KeyboardHandler* keyboard;
};
