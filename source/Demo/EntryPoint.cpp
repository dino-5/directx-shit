#include "Demo/BaseDemo.h"

//int WINAPI wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE /*hPrevInstance*/, _In_ LPWSTR /*lpCmdLine*/, _In_ int nCmdShow) 
int main()
{
	// Enable run-time memory check for debug builds.
#if defined(DEBUG) | defined(_DEBUG)
	_CrtSetDbgFlag( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
#endif
    uint width = 1200;
    uint height = 800;
    std::string name = "DX12 Demo";

    try
    {
        BaseDemo theApp(width, height, name);
        if(!theApp.Initialize())
            return 0;

        theApp.Run();
    }
    catch(engine::util::DxException& e)
    {
        MessageBox(nullptr, e.ToString().c_str(), "HR Failed", MB_OK);
        return 0;
    }
}
