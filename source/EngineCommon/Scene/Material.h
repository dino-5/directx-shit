#pragma once
#include "EngineCommon/math/Vector.h"

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
    float metalFuzy; // metal
    float refractionAngle; // dielectric
    MaterialType type;
};

