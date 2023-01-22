#pragma once
#include "Framework/math/Functions.h"

float Math::ToRadians(float degrees)
{
	return degrees * factorRadians;
}
