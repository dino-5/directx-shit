#include "BaseDemo.h"

using namespace std;
using namespace DirectX;


void BaseDemo::Initialize(int width, int height, std::string name)
{

	SwapChainSettings settings; 
	settings.width  = width;
	settings.height = height;
	settings.window = Window::GetWindowHandler();
	m_renderContext.Initialize();
	m_renderContext.ResetSwapChain(settings);
	
}


