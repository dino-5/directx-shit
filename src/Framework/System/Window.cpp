#include "Window.h"
#include "framework/include/ImGuiSettings.h"
#include "external/imgui/imgui.h"
#include "external/imgui/backends/imgui_impl_dx12.h"
#include "framework/dx12/Device.h"
#include <string>
#include <windowsx.h>

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

LRESULT CALLBACK
MainWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    return WindowApp::GetWindowApp()->MsgProc(hwnd, msg, wParam, lParam);
}


LRESULT WindowApp::MsgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	if (ImGui_ImplWin32_WndProcHandler(hwnd, msg, wParam, lParam))
        return true;
	switch( msg )
	{
	case WM_SIZE:
		if( Device::GetDevice())
		{
			if( wParam == SIZE_MINIMIZED )
			{
			}
			else if( wParam == SIZE_MAXIMIZED )
			{
				//OnResize();
			}
			else if( wParam == SIZE_RESTORED )
			{
				//OnResize();
			}
		}
		return 0;

	case WM_EXITSIZEMOVE:
		//OnResize();
		return 0;
 
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;

	case WM_GETMINMAXINFO:
		((MINMAXINFO*)lParam)->ptMinTrackSize.x = 200;
		((MINMAXINFO*)lParam)->ptMinTrackSize.y = 200; 
		return 0;

	case WM_LBUTTONDOWN:
	case WM_MBUTTONDOWN:
	case WM_RBUTTONDOWN:
		mouse->OnMouseDown(wParam, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
		return 0;
	case WM_LBUTTONUP:
	case WM_MBUTTONUP:
	case WM_RBUTTONUP:
		mouse->OnMouseUp(wParam, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
		return 0;
	case WM_MOUSEMOVE:
		mouse->OnMouseMove(wParam, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
		return 0;
    case WM_KEYUP:
        if(wParam == VK_ESCAPE)
        {
            PostQuitMessage(0);
        }
	case WM_KEYDOWN:
		keyboard->OnKeyDown(static_cast<Key>(wParam));
        return 0;
	}
	return DefWindowProc(hwnd, msg, wParam, lParam);
}


WNDCLASS Window::CreateWindowClass(HINSTANCE inst, std::string name)
{
	WNDCLASS wc;
	wc.style         = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc   = MainWndProc; 
	wc.cbClsExtra    = 0;
	wc.cbWndExtra    = 0;
	wc.hInstance     = inst;
	wc.hIcon         = LoadIcon(0, IDI_APPLICATION);
	wc.hCursor       = LoadCursor(0, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)GetStockObject(NULL_BRUSH);
	wc.lpszMenuName  = 0;
	wc.lpszClassName = std::wstring(name.begin(), name.end()).c_str();

	if( !RegisterClass(&wc) )
	{
		std::string message = "RegisterClass Failed";
		std::wstring str = std::wstring(message.begin(), message.end()).c_str();
		MessageBox(0, str.c_str(), 0, 0);
	}
	return wc;
}

Window::Window(HINSTANCE inst, int w, int h, std::string name) : width(w), height(h), windowName(name)
{
	auto wc = CreateWindowClass(inst, name);
	RECT R = { 0, 0, width, height};
    AdjustWindowRect(&R, WS_OVERLAPPEDWINDOW, false);
	w = R.right - R.left;
	h = R.bottom - R.top;

	m_windowHandler = CreateWindow(wc.lpszClassName, wc.lpszClassName,
		WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, w, h, 0, 0, inst, 0); 
	if( m_windowHandler)
	{
		std::string message = "CreateWindow Failed.";
		std::wstring str = std::wstring(message.begin(), message.end()).c_str();
		MessageBox(0, str.c_str(), 0, 0);
	}
	ShowWindow(m_windowHandler, SW_SHOW);
	UpdateWindow(m_windowHandler);
}


WindowApp::WindowApp(HINSTANCE inst, int width, int height, std::string name) :
	Window(inst, width, height, name)
{
	WindowApp* app = GetWindowApp();
	app = this;
}