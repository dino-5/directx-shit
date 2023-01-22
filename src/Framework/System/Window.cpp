#include "Window.h"
#include "framework/include/ImGuiSettings.h"
#include "external/imgui/imgui.h"
#include "external/imgui/imgui_impl_dx12.h"
#include "framework/dx12/Device.h"

LRESULT CALLBACK
MainWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    return WindowApp::MsgProc(hwnd, msg, wParam, lParam);
}


LRESULT WindowApp::MsgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	//if (ImGui_ImplWin32_WndProcHandler(hwnd, msg, wParam, lParam))
        return true;
	switch( msg )
	{
	case WM_ACTIVATE:
		if( LOWORD(wParam) == WA_INACTIVE )
		{
			//mAppPaused = true;
			//mTimer.Stop();
		}
		else
		{
			//mAppPaused = false;
			//mTimer.Start();
		}
		return 0;

	// WM_SIZE is sent when the user resizes the window.  
	case WM_SIZE:
		// Save the new client area dimensions.
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

	// Catch this message so to prevent the window from becoming too small.
	case WM_GETMINMAXINFO:
		((MINMAXINFO*)lParam)->ptMinTrackSize.x = 200;
		((MINMAXINFO*)lParam)->ptMinTrackSize.y = 200; 
		return 0;

	case WM_LBUTTONDOWN:
	case WM_MBUTTONDOWN:
	case WM_RBUTTONDOWN:
		//OnMouseDown(wParam, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
		return 0;
	case WM_LBUTTONUP:
	case WM_MBUTTONUP:
	case WM_RBUTTONUP:
		//OnMouseUp(wParam, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
		return 0;
	case WM_MOUSEMOVE:
		//OnMouseMove(wParam, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
		return 0;
    case WM_KEYUP:
        if(wParam == VK_ESCAPE)
        {
            PostQuitMessage(0);
        }
	case WM_KEYDOWN:
		//OutputDebugString((std::to_wstring(wParam)+L"\n").c_str());
		//OnKeyDown(static_cast<Key>(wParam));
		

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
	wc.lpszClassName = name.c_str();

	if( !RegisterClass(&wc) )
	{
		MessageBox(0, "RegisterClass Failed.", 0, 0);
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

	m_windowHandler = CreateWindow(windowName.c_str(), windowName.c_str(),
		WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, w, h, 0, 0, inst, 0); 
	if( m_windowHandler)
	{
		MessageBox(0, "CreateWindow Failed.", 0, 0);
	}
	ShowWindow(m_windowHandler, SW_SHOW);
	UpdateWindow(m_windowHandler);
}


WindowApp::WindowApp(HINSTANCE inst, int width, int height, std::string name) : m_inst(inst), m_window(inst, width, height, name)
{
	app = this;
}