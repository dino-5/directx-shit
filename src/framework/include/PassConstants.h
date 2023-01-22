#pragma once
#include "d3dx12.h"
#include "MathHelper.h"
#include "ImGuiSettings.h"
#include "Framework/math/Vector.h"
#include "Framework/math/Matrix.h"

enum class LightType
{
    Directional,
    Point,
    Spot
};

struct  PointLight
{
    Vector4 Pos   = { 0.0f, 3.0f,-2.0f, 1.0f }; 
    Vector4 Color = { 1.0f, 1.0f, 1.0f, 1.0f };
    float saturation_a = 0;
    float saturation_b = 0;
    float saturation_c = 0;
    void OnImGui()
    {
        ImGuiSettings::Begin("Light");
        Pos.OnImGui("Position");
        Color.OnImGui("Color", 0, 1);
        ImGuiSettings::End();
    }
};

struct  SpotLight
{
    Vector4 Pos   = { 0.0f, 3.0f,-2.0f, 1.0f }; 
    Vector4 Color = { 1.0f, 1.0f, 1.0f, 1.0f };
    Vector4 Dir= { 0.0f, 3.0f,-2.0f, 1.0f }; 
    float angle = 0;
    void OnImGui()
    {
        ImGuiSettings::Begin("Light");
        Pos.OnImGui("Position");
        Color.OnImGui("Color", 0, 1);
        ImGuiSettings::End();
    }
};

class PassConstants
{
public:
    DirectX::XMFLOAT4X4 View = MathHelper::Identity4x4();
    //DirectX::XMFLOAT4X4 Proj = MathHelper::Identity4x4();
    Math::Matrix4 Proj;
    DirectX::XMFLOAT4X4 ViewProj = MathHelper::Identity4x4();

    float EyePosW[4]     = {0.0f, 0.0f, 0.0f, 1.0f};
    // light
};

class ImguiConstants
{
public:
    ImguiConstants(PassConstants& obj) : m_obj(obj)
    {
    }
    void OnImGui()
    {

		//ImGuiSettings::Begin("light");
		{
			float min_translation = -10;
			float max_translation = 10;

			float min_scale = -10;
			float max_scale = 10;
			//if (ImGuiSettings::SliderFloat3("pos", lightPos, min_translation, max_translation))
			{
			}
			
			//if (ImGuiSettings::SliderFloat3("dir", lightDir, min_scale, max_scale))
			//{
   //             m_obj.lightDir.x = lightDir[0];
   //             m_obj.lightDir.y = lightDir[1];
   //             m_obj.lightDir.z = lightDir[2];
			//}
		}
		//ImGuiSettings::End();
    }

    PassConstants& m_obj;
};

