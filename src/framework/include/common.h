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
	ObjectConstants() = default;
	ObjectConstants(XMFLOAT4X4 world) : World(world) {}
	ObjectConstants(XMMATRIX world) {
		XMStoreFloat4x4(&World, world);
	}
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
