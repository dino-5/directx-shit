#pragma once
#include<d3d12.h>
#include<DirectXMath.h>
#include"d3dx12.h"
#include"MathHelper.h"
#include<wrl/client.h>
template<typename T>
using ComPtr = Microsoft::WRL::ComPtr <T>;
#include"d3dUtil.h"

static const int NumFrames=3;

using namespace DirectX;

struct Vertex
{
    XMFLOAT3 Pos;
    XMFLOAT3 Normal;
    XMFLOAT2 Tex;
};

struct ObjectConstants
{
    XMFLOAT4X4 World = MathHelper::Identity4x4();
 };

enum class Key {
	W = 65,
	A = 87,
	S = 83,
	D = 68
};
