#pragma once

#include <WindowsX.h>
#include "EngineGfx/RenderContext.h"
#include "EngineCommon/include/defines.h"
#include "EngineCommon/System/Window.h"
#include "EngineCommon/System/InputManager.h"
#include "third_party/imgui/imgui.h"
#include "third_party/imgui/backends/imgui_impl_dx12.h"
#include "Demo/passes/SimplePass.h"

class CommandLine;
using namespace engine;

// ?
struct DemoSettings
{
	graphics::SwapChainSettings m_settings; 
};


class BaseDemo : public WindowApp
{
public:
	BaseDemo(int width, int height, std::string name);
	bool Initialize()override;
	void InitializePasses();
	SHIT_ENGINE_SINGLETONE(BaseDemo);

protected:
	void OnResize()override;
	void Update()override;
	void Draw()override;
	void Destroy()override;
    LRESULT ProcessInput(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)override;

private:
	graphics::SwapChainSettings GetCurrentWindowSettings();
	void SetupCallbacks();

private:
	CommandLine* cmdLine;
	SimplePass m_pass;
	DemoSettings m_currentSettings;
	u32 m_currentFrameIndex = 0;
	graphics::RenderContext m_renderContext;
	system::InputManager* m_inputManager;
};

