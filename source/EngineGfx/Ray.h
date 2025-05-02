#pragma once

#include "EngineCommon/math/Vector.h"

namespace engine::graphic{

struct Ray
{
    using Vec3 = engine::math::Vector3;
    Ray() = default;
    Ray(const Vec3& ori, const Vec3& dir) : m_origin(ori), m_direction(dir) {}

    Vec3 at(double t) const 
    { 
        return m_origin + m_direction * t;
    }

    const Vec3& origin() const { return m_origin; }
    const Vec3& direction() const { return m_direction; }

private:
    Vec3 m_origin;
    Vec3 m_direction;
};

};
