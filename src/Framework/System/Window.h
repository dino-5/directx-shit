#pragma once
#include "include/common.h"
#include "include/defines.h"
#include <string>
#include "Handlers.h"
#include <memory>


class Window
{
public:
	Window(int width, int height, std::string name);
	void Initialize(int width, int height, std::string name);
	Window() = default;

	static WNDCLASS CreateWindowClass(std::string);

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
	WindowApp(int width, int height, std::string name);
	void Initialize(int width, int height, std::string name);

	SHIT_ENGINE_SINGLETONE(WindowApp);

protected:
	virtual void OnResize() {}
	virtual void Update() {}
	virtual void Draw() {}
	virtual void Destroy() {}

	virtual void OnMouseDown(WPARAM btnState, int x, int y){ }
	virtual void OnMouseUp(WPARAM btnState, int x, int y)  { }
	virtual void OnMouseMove(WPARAM btnState, int x, int y){ }
	virtual void OnKeyDown(Key key) { }
	virtual void Destroy() = 0;
};
