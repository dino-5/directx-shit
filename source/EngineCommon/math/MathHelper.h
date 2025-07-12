
#pragma once

#include <DirectXMath.h>
#include <cstdint>
#include <random>

namespace engine::util
{
inline float RandF()
{
    return (float)(rand()) / (float)RAND_MAX;
}

// Returns random float in [a, b).
inline float RandF(float a, float b)
{
    return a + RandF() * (b - a);
}

inline int Rand(int a, int b)
{
    return a + rand() % ((b - a) + 1);
}

template<typename T>
inline T Min(const T& a, const T& b)
{
    return a < b ? a : b;
}

template<typename T>
inline T Max(const T& a, const T& b)
{
    return a > b ? a : b;
}

template<typename T>
inline T Lerp(const T& a, const T& b, float t)
{
    return a + (b - a) * t;
}

template<typename T>
inline T Clamp(const T& x, const T& low, const T& high)
{
    return x < low ? low : (x > high ? high : x);
}

// Returns the polar angle of the point (x,y) in [0, 2*PI).

extern const float Infinity;
extern const float Pi;


};

