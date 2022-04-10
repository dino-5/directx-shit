#pragma once
#include<d3d12.h>
#include<DirectXMath.h>
#include"d3dx12.h"
#include"MathHelper.h"
#include<wrl/client.h>
template<typename T>
using ComPtr = Microsoft::WRL::ComPtr <T>;
#include"d3dUtil.h"

using namespace DirectX;

struct Vertex
{
    XMFLOAT3 Pos;
    XMFLOAT3 Normal;
};

struct ObjectConstants
{
    XMFLOAT4X4 World = MathHelper::Identity4x4();
 }    ;

static const int NumFrameResources = 3;
