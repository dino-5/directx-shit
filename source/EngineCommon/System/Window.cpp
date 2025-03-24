#include "Window.h"
#include "EngineCommon/util/ImGuiSettings.h"
#include "EngineCommon/util/Logger.h"
#include "EngineCommon/util/Timer.h"
#include "third_party/imgui/imgui.h"
#include "third_party/imgui/backends/imgui_impl_dx12.h"
#include "EngineGfx/dx12/Device.h"
#include <string>
#include <windowsx.h>
#include <chrono>
#include <ctime>
#include <winnt.h>
#include <winuser.h>
#include <windef.h>
#include <dwmapi.h>

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

LRESULT CALLBACK
MainWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    return WindowApp::App->msgProc(hwnd, msg, wParam, lParam);
}


LRESULT WindowApp::msgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	if (ImGui_ImplWin32_WndProcHandler(hwnd, msg, wParam, lParam))
        return 0;
	return processInput(hwnd, msg, wParam, lParam);
}


WNDCLASSEXA Window::CreateWindowClass(const std::string_view name)
{
	HINSTANCE inst = GetModuleHandle(nullptr);
	WNDCLASSEXA wcex;
	wcex.cbSize = sizeof(WNDCLASSEXA);
	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = MainWndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = inst;
	wcex.hIcon = LoadIcon(inst, IDI_APPLICATION);
	wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = nullptr;
	wcex.lpszClassName = name.data();
	wcex.hIconSm = LoadIcon(inst, IDI_APPLICATION);
	if (!RegisterClassExA(&wcex))
	{
		MessageBox(0, "RegisterClass Failed.", 0, 0);
	}
	return wcex;
}

Window::Window(int w, int h, std::string_view name) :
	m_width(w), m_height(h), m_windowName(name)
{}

bool Window::initialize()
{
	auto wc = CreateWindowClass(m_windowName);
	RECT R = { 0, 0, m_width, m_height};
	AdjustWindowRect(&R, WS_OVERLAPPEDWINDOW, false);
	int w = R.right - R.left;
	int h = R.bottom - R.top;
	HINSTANCE inst = GetModuleHandle(nullptr);

	m_windowHandler = CreateWindowExA(
		NULL,
		wc.lpszClassName,
		"DX12 Demo",
		WS_OVERLAPPEDWINDOW, 
		CW_USEDEFAULT, 
		CW_USEDEFAULT,
		w,
		h,
		nullptr,
		nullptr,
		inst,
		nullptr);
	if(!m_windowHandler)
	{
		std::string message = "CreateWindow Failed.";
		MessageBox(0, message.c_str(), 0, 0);
	}

	ShowWindow(m_windowHandler, SW_SHOW);
	UpdateWindow(m_windowHandler);

	return true;
}

WindowApp::WindowApp(int width, int height, std::string_view name) :
	Window(width, height, name)
{
	App = this;
	initialize();
}

void WindowApp::onResize(unsigned int width, unsigned int height)
{
	setWidth(width);
	setHeight(height);
}

bool WindowApp::initialize()
{
	Window::initialize();
	return true;
}

void WindowApp::run()
{
	MSG msg = { };
	while (msg.message !=WM_QUIT)
	{
		if (PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else
		{
			util::Timer timer("main loop", true);
			update();
			draw();
			std::string title = util::to_string(std::format(L"DX12 Demo fps: {}/{}", int(timer.getFps()), timer.getElapsedTime() ));
			bool ret = SetWindowTextA(getWindowHandle(), title.c_str());
			if(!ret)
				util::printError("can't change title name");
		}
		tagRECT rect;
		::GetClientRect(getWindowHandle(),  &rect);
		uint width = abs(rect.right - rect.left);
		uint height = abs(rect.bottom - rect.top);
		if(width != getWidth() || height != getHeight())
			onResize(width, height);
	}
	destroy();
}
