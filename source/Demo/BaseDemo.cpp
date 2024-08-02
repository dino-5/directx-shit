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
	SetupCallbacks();
	return true;
}

void BaseDemo::SetupCallbacks()
{
	float velocity = .5f;
	graphics::Camera& camera = m_renderContext.GetCamera();
	system::CallbackInfo info;
	info.oneTimeTouch = false;

	info.ptr = [&camera, velocity]() {
		camera.Translate(math::Vector3({ velocity, 0.f, 0.f }));
	};
	m_inputManager->AddCallback(system::Key::D, info);

	info.ptr = [&camera, velocity]() {
		camera.Translate(math::Vector3({ 0.f, 0.f, velocity }));
	};
	m_inputManager->AddCallback(system::Key::W, info);

	info.ptr = [&camera, velocity]() {
		camera.Translate(math::Vector3({ -velocity, 0.f, 0.f }));
	};
	m_inputManager->AddCallback(system::Key::A, info);

	info.ptr = [&camera, velocity]() {
		camera.Translate(math::Vector3({ 0.f, 0.f, -velocity }));
	};
	m_inputManager->AddCallback(system::Key::S, info);

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

LRESULT BaseDemo::ProcessInput(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    return m_inputManager->ProcessInput(hwnd, msg, wParam, lParam);
}
