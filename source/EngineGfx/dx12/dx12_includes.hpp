#pragma once

extern "C" { _declspec(dllexport) extern const unsigned int D3D12SDKVersion; }
extern "C" { _declspec(dllexport) extern const char* D3D12SDKPath; }

#include <d3d12.h>
#pragma comment(lib, "d3d12.lib")

#if defined(Debug)
#   include <dxgidebug.h>
#   pragma comment(lib, "dxguid.lib")
#endif

#include <d3dcompiler.h>
#pragma comment(lib, "d3dcompiler.lib")

#include <dxgi1_6.h>
#pragma comment(lib, "dxgi.lib")

#include <wrl/client.h>

#include <source_location>
#include <DirectXMath.h>
#include <DirectXCollision.h>

#include "EngineCommon/util/Util.h"
#include "EngineGfx/dx12/d3dx12.h"
#include "EngineCommon/util/Logger.h"

enum class ResourceFlags
{
	NONE          = D3D12_RESOURCE_FLAG_NONE,
	RENDER_TARGET = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET,
	DEPTH_STENCIL = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL,
	UNOURDERED    = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS,
	DENY_SHADER   = D3D12_RESOURCE_FLAG_DENY_SHADER_RESOURCE
};

enum class Format
{
	float3 = DXGI_FORMAT_R32G32B32_FLOAT,
	float2 = DXGI_FORMAT_R32G32_FLOAT,
};

D3D12_RESOURCE_FLAGS CastType(ResourceFlags flag);

inline void ThrowIfFailed(
    HRESULT hr,
    std::source_location location = std::source_location::current()) 
{
    std::string wfn = __FILE__;
    if(FAILED(hr))
    {
        DebugBreak();
        engine::util::printError("{} crashed ",
                                 engine::util::GetFormattedPath(location));
        throw "we Crashed :}";
    }
}

using namespace DirectX;
