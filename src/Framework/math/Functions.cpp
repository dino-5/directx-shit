#pragma once
#include "Framework/math/Functions.h"

float engine::math::ToRadians(float degrees)
{
	return degrees * factorRadians;
}
