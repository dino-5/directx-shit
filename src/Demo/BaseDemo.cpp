#include "BaseDemo.h"
#include "Framework/util/Logger.h"

using namespace std;
using namespace DirectX;


BaseDemo::BaseDemo(int width, int height, std::string name):
	WindowApp(width, height, name)
{}

engine::graphics::SwapChainSettings BaseDemo::GetCurrentWindowSettings()
{
	return { GetWidth(), GetHeight(), DXGI_FORMAT_R8G8B8A8_UNORM, GetWindowHandle()};
}


bool BaseDemo::Initialize()
{
	LogScope("BaseDemo");
	WindowApp::Initialize();
	m_renderContext.Initialize();
	m_renderContext.ResetSwapChain(GetCurrentWindowSettings());
	m_pass.Initialize(m_renderContext.GetDevice(), m_renderContext.GetAspectRatio());
	return true;
}

void BaseDemo::Draw()
{
	m_renderContext.StartFrame();
	m_pass.Draw(m_renderContext.GetList());
	m_renderContext.EndFrame();
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

