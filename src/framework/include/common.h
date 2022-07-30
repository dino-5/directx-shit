#pragma once
#include<d3d12.h>
#include<DirectXMath.h>
#include"d3dx12.h"
#include"MathHelper.h"
#include<wrl/client.h>
template<typename T>
using ComPtr = Microsoft::WRL::ComPtr <T>;
using uint = unsigned int;
using uint8 = unsigned char;
#include"d3dUtil.h"

static const int NumFrames=3;

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

template<typename T>
unsigned int GetVectorSize(std::vector<T> vec)
{
	return vec.size() * sizeof(T);
}
