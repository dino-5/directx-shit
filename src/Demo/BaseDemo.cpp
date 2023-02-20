#include "BaseDemo.h"

using namespace std;
using namespace DirectX;


BaseDemo::BaseDemo(int width, int height, std::string name):
	WindowApp(width, height, name)
{}

SwapChainSettings BaseDemo::GetCurrentWindowSettings()
{
	return { GetWidth(), GetHeight(), DXGI_FORMAT_R8G8B8A8_UNORM, GetWindowHandle()};
}

//LRESULT BaseDemo::MsgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
//{
//	return 0;
//	//return DefWindowProc(hwnd, msg, wParam, lParam);
//}

bool BaseDemo::Initialize()
{
	WindowApp::Initialize();
	m_renderContext.Initialize(m_allocator);
	m_renderContext.ResetSwapChain(GetCurrentWindowSettings());

	return true;
}

void BaseDemo::Draw()
{

}

void BaseDemo::Update()
{

}
void BaseDemo::Destroy()
{
	m_renderContext.Reset();
}
void BaseDemo::OnResize()
{

}

void BaseDemo::OnMouseDown(WPARAM btnState, int x, int y)
{

}

void BaseDemo::OnMouseUp(WPARAM btnState, int x, int y)
{
	
}

void BaseDemo::OnMouseMove(WPARAM btnState, int x, int y)
{

}

void BaseDemo::OnKeyDown(Key key)
{

}

void BaseDemo::OnKeyUp(Key key)
{

}

