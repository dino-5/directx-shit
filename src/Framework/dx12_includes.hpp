#pragma once

#include <d3d12.h>
#include <Framework/include/d3dx12.h>
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
using Microsoft::WRL::ComPtr;


#include <DirectXMath.h>
#include <DirectXCollision.h>
