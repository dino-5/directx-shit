#pragma once

#include "include/d3dApp.h"
#include <WindowsX.h>
#include "framework/include/ImGuiSettings.h"
#include "framework/dx12/Device.h"
#include "framework/dx12/RenderContext.h"
#include "Framework/include/defines.h"
#include "Framework/System/Window.h"
#include "external/imgui/imgui.h"
#include "external/imgui/backends/imgui_impl_dx12.h"


class BaseDemo : public WindowApp
{
public:
	void Initialize(int width, int height, std::string name);
	SHIT_ENGINE_SINGLETONE(BaseDemo);

	void Run();

private:
    
private:
	RenderContext m_renderContext;
};

