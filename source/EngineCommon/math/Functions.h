#pragma once
#include <cmath>

namespace engine::math
{
	constexpr float PI = 3.14159265359;
	constexpr float factorRadians = PI / 180;
	float ToRadians(float degrees);
}
