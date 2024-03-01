#include "Demo/BaseDemo.h"

int main()
{

#if defined(DEBUG) | defined(_DEBUG)
	_CrtSetDbgFlag( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
#endif

    // global variable initialization
    {
        g_demoDir = fs::current_path();
        g_homeDir = g_demoDir.parent_path().parent_path();
    }

    //{
        uint width = 1200;
        uint height = 800;
        std::string name = "DX12 Demo";
    //}


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
