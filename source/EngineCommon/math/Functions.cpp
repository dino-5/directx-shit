#pragma once
#include "EngineCommon/math/Functions.h"

float engine::math::ToRadians(float degrees)
{
	return degrees * factorRadians;
}
