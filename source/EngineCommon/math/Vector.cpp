#include "EngineCommon/math/Vector.h"
#include "EngineCommon/math/Functions.h"

using namespace engine::math;

Quartenion::Quartenion(Vector3 direction, float angle)
{
    float radians = ToRadians(angle/2);
    float sinValue = sin(radians);
    float cosValue = cos(radians);
    direction.NormalizeSelf();

    m_quarternion[0] = sinValue * direction[0];
    m_quarternion[1] = sinValue * direction[1];
    m_quarternion[2] = sinValue * direction[2];
    m_quarternion[3] = cosValue;
}

