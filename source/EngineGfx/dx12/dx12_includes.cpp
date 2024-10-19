#include "EngineGfx/dx12/dx12_includes.hpp"

extern "C" { _declspec(dllexport) const unsigned int D3D12SDKVersion = 611; }
extern "C" { _declspec(dllexport) const char* D3D12SDKPath = ".\\"; }

D3D12_RESOURCE_FLAGS CastType(ResourceFlags flag)
{
	return static_cast<D3D12_RESOURCE_FLAGS>(flag);
}
