#include "Window.h"
#include "Framework/util/ImGuiSettings.h"
#include "external/imgui/imgui.h"
#include "external/imgui/backends/imgui_impl_dx12.h"
#include "Framework/graphics/dx12/Device.h"
#include <string>
#include <windowsx.h>

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

LRESULT CALLBACK
MainWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    return WindowApp::App->MsgProc(hwnd, msg, wParam, lParam);
}


LRESULT WindowApp::MsgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	if (ImGui_ImplWin32_WndProcHandler(hwnd, msg, wParam, lParam))
        return 0;
	switch( msg )
	{
	case WM_LBUTTONDOWN:
	case WM_MBUTTONDOWN:
	case WM_RBUTTONDOWN:
		OnMouseDown(wParam, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
		return 0;
	case WM_LBUTTONUP:
	case WM_MBUTTONUP:
	case WM_RBUTTONUP:
		OnMouseUp(wParam, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
		return 0;
	case WM_MOUSEMOVE:
		OnMouseMove(wParam, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
		return 0;

	case WM_KEYDOWN:
		if(wParam == VK_ESCAPE)
			PostQuitMessage(0);
		return 0;
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	default:
		return DefWindowProc(hwnd, msg, wParam, lParam);
	}
}


WNDCLASSEX Window::CreateWindowClass(std::string name)
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
	wcex.lpszClassName = std::wstring(name.begin(), name.end()).c_str();
	wcex.hIconSm = LoadIcon(inst, IDI_APPLICATION);
	if (!RegisterClassEx(&wcex))
	{
		MessageBox(0, L"RegisterClass Failed.", 0, 0);
	}
	return wcex;
}

Window::Window(int w, int h, std::string name) :
	width(w), height(h), windowName(name)
{}

bool Window::Initialize()
{
	auto wc = CreateWindowClass(windowName);
	RECT R = { 0, 0, width, height};
    AdjustWindowRect(&R, WS_OVERLAPPEDWINDOW, false);
	int w = R.right - R.left;
	int h = R.bottom - R.top;
	HINSTANCE inst = GetModuleHandle(nullptr);
	m_windowHandler = CreateWindow(wc.lpszClassName, wc.lpszClassName,
		WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, w, h, nullptr, nullptr, inst, nullptr);
	if(!m_windowHandler)
	{
		std::string message = "CreateWindow Failed.";
		std::wstring str = std::wstring(message.begin(), message.end()).c_str();
		MessageBox(0, str.c_str(), 0, 0);
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

bool WindowApp::Initialize()
{
	Window::Initialize();
	return true;
}

void WindowApp::Run()
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
			Update();
			Draw();
		}
	}
	Destroy();
}