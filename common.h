#pragma once
#include<d3d12.h>
#include<DirectXMath.h>
#include"../../Common/d3dx12.h"
#include"../../Common/MathHelper.h"
#include<wrl/client.h>
template<typename T>
using ComPtr = Microsoft::WRL::ComPtr <T>;
#include"../../Common/d3dUtil.h"

using namespace DirectX;

struct Vertex
{
    XMFLOAT3 Pos;
    XMFLOAT4 Color;
};

struct ObjectConstants
{
    XMFLOAT4X4 World = MathHelper::Identity4x4();
};
