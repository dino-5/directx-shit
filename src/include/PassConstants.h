#pragma once
#include"d3dx12.h"
#include"MathHelper.h"

class PassConstants
{
public:
    DirectX::XMFLOAT4X4 View = MathHelper::Identity4x4();
    DirectX::XMFLOAT4X4 InvView = MathHelper::Identity4x4();
    DirectX::XMFLOAT4X4 Proj = MathHelper::Identity4x4();
    DirectX::XMFLOAT4X4 InvProj = MathHelper::Identity4x4();
    DirectX::XMFLOAT4X4 ViewProj = MathHelper::Identity4x4();
    DirectX::XMFLOAT4X4 InvViewProj = MathHelper::Identity4x4();

    DirectX::XMFLOAT3 EyePosW = { 0.0f, 0.0f, 0.0f };
    DirectX::XMFLOAT3 lightPos   = { 0.0f, 0.0f, 0.0f };
    DirectX::XMFLOAT3 lightColor = { 1.0f, 1.0f, 1.0f };
    DirectX::XMFLOAT3 boxColor   = { 0.5f, 0.1f, 0.0f };
};

