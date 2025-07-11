#pragma once
#include "EngineCommon/math/Vector.h"
#include <functional>

enum MaterialType
{
    Lambertian,
    Metal,
    Dielectric,
    MaterialCount
};

static const char* MaterialTypeNames[] = { "Lambert", "Metal", "Dielectric" };

struct Material
{
    engine::math::Vector4 color;
    float metalFuzy=0; // metal
    float refractionIndex=0; // dielectric
    MaterialType type;
};

inline Material MakeLamberian(engine::math::Vector3 color)
{
    Material mat;
    mat.color = engine::math::Vector4(color, 1.f);
    mat.type = Lambertian;
    return mat;
}

inline Material MakeMetal(engine::math::Vector3 color, float fuzzy)
{
    Material mat;
    mat.color = engine::math::Vector4(color, 1.f);
    mat.metalFuzy = fuzzy;
    mat.type = Metal;
    return mat;
}

inline Material MakeDielectric(float refIndex)
{
    Material mat;
    mat.refractionIndex = refIndex;
    mat.type = Dielectric;
    return mat;
}

