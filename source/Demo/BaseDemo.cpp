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
	/*float velocity = .5f;
	float rotationVelocity = 5.f;
	graphics::Camera& camera = m_renderContext.GetCamera();
	system::CallbackInfo info;
	info.oneTimeTouch = false;

	info.ptr = [&camera, velocity]() {
		camera.Translate(graphics::MovementDirection::SideDirection, velocity);
	};
	m_inputManager->AddCallback(system::Key::D, info);

	info.ptr = [&camera, velocity]() {
		camera.Translate(graphics::MovementDirection::ViewDirection, velocity);
	};
	m_inputManager->AddCallback(system::Key::W, info);

	info.ptr = [&camera, velocity]() {
		camera.Translate(graphics::MovementDirection::SideDirection, -velocity);
	};
	m_inputManager->AddCallback(system::Key::A, info);

	info.ptr = [&camera, velocity]() {
		camera.Translate(graphics::MovementDirection::ViewDirection, -velocity);
	};
	m_inputManager->AddCallback(system::Key::S, info);
	

	info.ptr = [&camera, rotationVelocity]() {
		camera.Rotate(-rotationVelocity, 0);
	};
	m_inputManager->AddCallback(system::Key::UP, info);

	info.ptr = [&camera, rotationVelocity]() {
		camera.Rotate( rotationVelocity, 0);
	};
	m_inputManager->AddCallback(system::Key::DOWN, info);

	info.ptr = [&camera, rotationVelocity]() {
		camera.Rotate(0, rotationVelocity);
	};
	m_inputManager->AddCallback(system::Key::RIGHT, info);

	info.ptr = [&camera, rotationVelocity]() {
		camera.Rotate(0, -rotationVelocity);
	};
	m_inputManager->AddCallback(system::Key::LEFT, info);

	info.ptr = [&camera]() {
		camera.Reset();
	};
	m_inputManager->AddCallback(system::Key::R, info);*/

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
	m_renderContext.Update();
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
