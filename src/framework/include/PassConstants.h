#pragma once
#include"d3dx12.h"
#include"MathHelper.h"
#include "ImGuiSettings.h"

class PassConstants
{
public:
    DirectX::XMFLOAT4X4 View = MathHelper::Identity4x4();
    DirectX::XMFLOAT4X4 InvView = MathHelper::Identity4x4();
    DirectX::XMFLOAT4X4 Proj = MathHelper::Identity4x4();
    DirectX::XMFLOAT4X4 InvProj = MathHelper::Identity4x4();
    DirectX::XMFLOAT4X4 ViewProj = MathHelper::Identity4x4();
    DirectX::XMFLOAT4X4 InvViewProj = MathHelper::Identity4x4();
    DirectX::XMFLOAT4X4 shadowView = MathHelper::Identity4x4();

    DirectX::XMFLOAT4 EyePosW    = { 0.0f, 0.0f, 0.0f, 1.0f};
    DirectX::XMFLOAT4 lightPos   = { 5.0f, 3.0f, 2.0f, 1.0f };
    DirectX::XMFLOAT4 lightDir   = {-5.0f,-3.0f,-2.0f,-1.0f };
    DirectX::XMFLOAT4 lightColor = { 1.0f, 1.0f, 1.0f, 1.0f };
};

class imguiConstants
{
public:
    imguiConstants(PassConstants& obj) : m_obj(obj)
    {
        lightPos[0] = obj.lightPos.x;
        lightPos[1] = obj.lightPos.y;
        lightPos[2] = obj.lightPos.z;

        lightDir[0] = obj.lightDir.x;
        lightDir[1] = obj.lightDir.y;
        lightDir[2] = obj.lightDir.z;
    }
    void OnImGui()
    {

		ImGui::Begin("light");
		{
			float min_translation = -10;
			float max_translation = 10;

			float min_scale = -10;
			float max_scale = 10;
			if (ImGui::SliderFloat3("pos", lightPos, min_translation, max_translation))
			{
                m_obj.lightPos.x = lightPos[0];
                m_obj.lightPos.y = lightPos[1];
                m_obj.lightPos.z = lightPos[2];
			}
			
			if (ImGui::SliderFloat3("dir", lightDir, min_scale, max_scale))
			{
                m_obj.lightDir.x = lightDir[0];
                m_obj.lightDir.y = lightDir[1];
                m_obj.lightDir.z = lightDir[2];
			}
		}
		ImGui::End();
    }

    PassConstants& m_obj;
    float lightPos[3];
    float lightDir[3];
};

