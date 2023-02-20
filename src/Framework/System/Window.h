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
	Window() = default;

	static WNDCLASSEX CreateWindowClass(std::string );

	virtual bool Initialize();
	void SetWindowHandler(HWND handler) { m_windowHandler = handler; }
	void SetWidth(int w) { width = w; }
	int GetWidth() { return width; }
	void SetHeight(int h) { height = h; }
	int GetHeight() { return height; }
	HWND GetWindowHandle() const { return m_windowHandler; }
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
	WindowApp() = default;
	virtual ~WindowApp() {}
	bool Initialize()override;
	SHIT_ENGINE_NON_COPYABLE(WindowApp);

	static inline WindowApp* App = nullptr;

	void Run();

protected:
	virtual void OnResize() =0;
	virtual void Update() = 0;
	virtual void Draw() = 0;
	virtual void Destroy() = 0;

	virtual void OnMouseDown(WPARAM btnState, int x, int y) = 0;
	virtual void OnMouseUp(WPARAM btnState, int x, int y) = 0;
	virtual void OnMouseMove(WPARAM btnState, int x, int y) = 0;
	virtual void OnKeyDown(Key key) = 0;
	virtual void OnKeyUp(Key key) = 0;
};
