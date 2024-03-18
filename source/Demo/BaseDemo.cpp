#include "BaseDemo.h"
#include "EngineCommon/util/Logger.h"
#include "EngineCommon/util/CommandLine.h"

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
	cmdLine = &CommandLine::GetCommandLine();
	LogScope("BaseDemo");
	WindowApp::Initialize();
	m_renderContext.Initialize(GetCurrentWindowSettings());
	InitializePasses();
	m_renderContext.FlushCommandQueue();
	return true;
}

void BaseDemo::InitializePasses()
{
	graphics::CommandList& commandList = m_renderContext.GetList();
	commandList.Reset(0);

	m_pass.Initialize(m_renderContext);

    commandList->Close();
    ID3D12CommandList* lists[] = { commandList.GetList() };
	u64 value = m_renderContext.GetFenceValue();
    m_renderContext.GetQueue()->ExecuteCommandLists(1, lists);
	value = m_renderContext.GetFenceValue();
}

void BaseDemo::Draw()
{
	u64 value = m_renderContext.GetFenceValue();
	m_renderContext.StartFrame();
	value = m_renderContext.GetFenceValue();
	m_pass.Draw(m_renderContext.GetList().GetList(), m_currentFrameIndex);
	value = m_renderContext.GetFenceValue();
	m_renderContext.EndFrame();
}

void BaseDemo::Update()
{
	m_currentFrameIndex = (m_currentFrameIndex + 1) % config::NumFrames;
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

