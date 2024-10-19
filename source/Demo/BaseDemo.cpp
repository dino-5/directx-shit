#include "BaseDemo.h"
#include "EngineCommon/util/Logger.h"
#include "EngineCommon/util/CommandLine.h"

using namespace std;
using namespace DirectX;


BaseDemo::BaseDemo(int width, int height, std::string name):
	WindowApp(width, height, name)
{
	m_inputManager = &system::InputManager::GetInputManager();
}

engine::graphics::SwapChainSettings BaseDemo::getCurrentWindowSettings()
{
	return { getWidth(), getHeight(), DXGI_FORMAT_R8G8B8A8_UNORM, getWindowHandle()};
}


bool BaseDemo::initialize()
{
	cmdLine = &CommandLine::GetCommandLine();
	LogScope("BaseDemo");
	WindowApp::initialize();
	m_renderContext.initialize(getCurrentWindowSettings());
	initializePasses();
	m_renderContext.flushCommandQueue();
	return true;
}

void BaseDemo::initializePasses()
{
	graphics::CommandList& commandList = m_renderContext.getList();
	commandList.reset(0);

	m_pass.initialize(m_renderContext);

    commandList->Close();
    ID3D12CommandList* lists[] = { commandList.getList() };
	u64 value = m_renderContext.getFenceValue();
    m_renderContext.getQueue()->ExecuteCommandLists(1, lists);
	value = m_renderContext.getFenceValue();
}

void BaseDemo::draw()
{
	m_renderContext.startFrame();
	m_pass.draw(m_renderContext.getList().getList(), m_currentFrameIndex);
	m_renderContext.endFrame();
}

void BaseDemo::update()
{
	m_currentFrameIndex = (m_currentFrameIndex + 1) % config::NumFrames;
	m_renderContext.update();
}
void BaseDemo::destroy()
{
	m_renderContext.reset();
}

void BaseDemo::onResize()
{

}

LRESULT BaseDemo::processInput(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    return m_inputManager->processInput(hwnd, msg, wParam, lParam);
}
