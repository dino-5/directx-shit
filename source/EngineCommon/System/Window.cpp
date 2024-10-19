#include "Window.h"
#include "EngineCommon/util/ImGuiSettings.h"
#include "EngineCommon/util/Logger.h"
#include "third_party/imgui/imgui.h"
#include "third_party/imgui/backends/imgui_impl_dx12.h"
#include "EngineGfx/dx12/Device.h"
#include <string>
#include <windowsx.h>
#include <chrono>
#include <ctime>

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


WNDCLASSEX Window::CreateWindowClass(const std::string& name)
{
	HINSTANCE inst = GetModuleHandle(nullptr);
	WNDCLASSEX wcex;
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = MainWndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = inst;
	wcex.hIcon = LoadIcon(inst, IDI_APPLICATION);
	wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = nullptr;
	wcex.lpszClassName = name.c_str();
	wcex.hIconSm = LoadIcon(inst, IDI_APPLICATION);
	if (!RegisterClassEx(&wcex))
	{
		MessageBox(0, "RegisterClass Failed.", 0, 0);
	}
	return wcex;
}

Window::Window(int w, int h, std::string name) :
	width(w), height(h), windowName(name)
{}

bool Window::initialize()
{
	auto wc = CreateWindowClass(windowName);
	RECT R = { 0, 0, width, height};
    AdjustWindowRect(&R, WS_OVERLAPPEDWINDOW, false);
	int w = R.right - R.left;
	int h = R.bottom - R.top;
	HINSTANCE inst = GetModuleHandle(nullptr);
	m_windowHandler = CreateWindow(wc.lpszClassName, "DX12 Demo",
		WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, w, h, nullptr, nullptr, inst, nullptr);
	if(!m_windowHandler)
	{
		std::string message = "CreateWindow Failed.";
		MessageBox(0, message.c_str(), 0, 0);
	}
	ShowWindow(m_windowHandler, SW_SHOW);
	UpdateWindow(m_windowHandler);
	return true;
}

WindowApp::WindowApp(int width, int height, std::string name) :
	Window(width, height, name)
{
	App = this;
}

bool WindowApp::initialize()
{
	Window::initialize();
	return true;
}

void WindowApp::run()
{
	// TODO : integrate timer from test project 
	MSG msg = { };
	while (msg.message !=WM_QUIT)
	{
		auto start = std::chrono::system_clock::now();
		if (PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else
		{
			update();
			draw();
		}
		auto end = std::chrono::system_clock::now();
		std::chrono::duration<double> time_elapsed = end - start;
		util::printInfo("frame rate is {}", 1.0 / time_elapsed.count());
	}
	destroy();
}