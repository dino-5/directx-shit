#pragma once

#include <WindowsX.h>
#include "Framework/graphics/RenderContext.h"
#include "Framework/include/defines.h"
#include "Framework/System/Window.h"
#include "external/imgui/imgui.h"
#include "external/imgui/backends/imgui_impl_dx12.h"

// ?
struct DemoSettings
{
	SwapChainSettings m_settings; 
};


class BaseDemo : public WindowApp
{
public:
	BaseDemo(int width, int height, std::string name);
	bool Initialize()override;
	SHIT_ENGINE_SINGLETONE(BaseDemo);

protected:
	void OnResize()override;
	void Update()override;
	void Draw()override;
	void Destroy()override;
	//LRESULT MsgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) ;

	void OnMouseDown(WPARAM btnState, int x, int y)override;
	void OnMouseUp(WPARAM btnState, int x, int y)override;
	void OnMouseMove(WPARAM btnState, int x, int y)override;
	void OnKeyDown(Key key)override;
	void OnKeyUp(Key key)override;

private:
	SwapChainSettings GetCurrentWindowSettings();

private:
	DemoSettings m_currentSettings;
	engine::graphics::RenderContext m_renderContext;
	ComPtr<ID3D12CommandAllocator> m_allocator = nullptr;
};

