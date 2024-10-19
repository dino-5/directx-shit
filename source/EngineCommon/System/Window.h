#pragma once
#include "EngineCommon/include/defines.h"
#include <string>
#include <memory>
#include <windows.h>

class Window
{
public:
	Window(int width, int height, std::string name);
	Window() = default;

	static WNDCLASSEX CreateWindowClass(const std::string& );

	virtual bool initialize();
	void setWindowHandler(HWND handler) { m_windowHandler = handler; }
	void setWidth(int w) { width = w; }
	int getWidth() { return width; }
	void setHeight(int h) { height = h; }
	int getHeight() { return height; }
	HWND getWindowHandle() const { return m_windowHandler; }
private:
	HWND m_windowHandler;
	int width;
	int height;
	std::string windowName;
};

class WindowApp : public Window
{
public:
	LRESULT msgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
	WindowApp(int width, int height, std::string name);
	WindowApp() = default;
	virtual ~WindowApp() {}
	bool initialize()override;
	SHIT_ENGINE_NON_COPYABLE(WindowApp);

	static inline WindowApp* App = nullptr;

	void run();

protected:
	virtual void onResize() =0;
	virtual void update() = 0;
	virtual void draw() = 0;
	virtual void destroy() = 0;
    virtual LRESULT processInput(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) = 0;
};
