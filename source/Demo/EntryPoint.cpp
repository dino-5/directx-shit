#include "Demo/BaseDemo.h"
#include "EngineCommon/util/DSHLoader.h"
#include "EngineCommon/util/CommandLine.h"
#include "EngineCommon/util/Logger.h"

using namespace engine::config;

int main()
{

#if defined(DEBUG) | defined(_DEBUG)
	_CrtSetDbgFlag( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
#endif

    // global variable initialization
    {
        g_state.demoDir   = fs::current_path();
        g_state.homeDir   = g_state.demoDir.parent().parent();
        g_state.shaderDir = g_state.demoDir / "Shaders";
        g_state.profilingEnabled = CommandLine::GetCommandLine().getValue(CommandLineOption::ENABLE_PROFILER);
        util::g_loggerState.setErrorsEnabled(true);
        util::g_loggerState.setInfoEnabled(true);
    }

    uint width = 1200;
    uint height = 800;
    std::string name = "DX12 Demo";

    BaseDemo theApp(width, height, name);
    if(!theApp.initialize())
        return 0;

    theApp.run();
}
