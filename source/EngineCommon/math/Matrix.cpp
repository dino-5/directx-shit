#include "EngineCommon/math/Matrix.h"

namespace engine::math
{

Matrix4 PerspectiveProjection(float fov/*in degrees*/, float aspectRatio, float nearZ, float farZ)
{
    float tanf = tan(ToRadians(fov/2));
    float d = 1 / tanf;
    float RangeZ = nearZ - farZ;

    Matrix4 matrix{ d / aspectRatio, 0,  0, 0,
                    0, d,  0, 0,
                    0, 0, -farZ/RangeZ, farZ*nearZ/RangeZ,
                    0, 0, 1, 0};
    matrix.TransposeSelf();
    return matrix;
}

};
