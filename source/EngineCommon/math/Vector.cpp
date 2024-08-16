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


Quartenion Quartenion::operator*(const Quartenion& q1)
{
    Vector3 v1(m_quarternion), v2(q1.m_quarternion);
    Vector3 res = CrossProduct(v1, v2) + v1 * q1.Real() + v2 * Real();
    return Vector4(res, {-DotProduct(v1, v2)+q1.Real()*Real()});
}
