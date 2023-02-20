#pragma once
#include<d3d12.h>
#include<DirectXMath.h>
#include"d3dx12.h"
#include "Framework/util/Util.h"
#include <string>
#include <vector>



#ifndef ThrowIfFailed
#define ThrowIfFailed(x)                                              \
{                                                                     \
    HRESULT hr__ = (x);                                               \
    std::wstring wfn = engine::util::to_wstring(__FILE__);                       \
    if(FAILED(hr__)) { throw engine::util::DxException(hr__, L#x, wfn, __LINE__); } \
}
#endif


using namespace DirectX;


struct Vertex
{
    XMFLOAT3 Pos;
    XMFLOAT3 Normal;
    XMFLOAT2 Tex;
};


enum class Key {
	W = 87,
	A = 65,
	S = 83,
	D = 68,
	SWITCH_CAMERA = 192
};

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


template<typename T>
unsigned int GetVectorSize(std::vector<T> vec)
{
	return vec.size() * sizeof(T);
}

D3D12_RESOURCE_FLAGS CastType(ResourceFlags flag);

