#include "EngineCommon/math/Matrix.h"

namespace engine::math
{

Matrix4 PerspectiveProjection(float fov/*in degrees*/, float aspectRatio, float nearZ, float farZ)
{
    //positive z direction is assumed
    float tanf = tan(ToRadians(fov/2));
    float d = 1 / tanf;
    float RangeZ = farZ - nearZ;
    float a = farZ  / RangeZ;
    float b = (-1.f) * nearZ * farZ / RangeZ;


    Matrix4 matrix{ d / aspectRatio, 0,  0, 0,
                    0, d,  0, 0,
                    0, 0,  a, b,
                    0, 0, 1, 0};
    matrix.TransposeSelf();
    return matrix;
}

};
