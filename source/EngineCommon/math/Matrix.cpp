#include "EngineCommon/math/Matrix.h"

namespace engine::math
{
    /*
        All multiplications are happened with the next order: M*a
        And M is matrix, a is vector. Also as we see in PerspectiveProjection we need to do transpose for matrix because it's
        row-major
    */
    Matrix4 PerspectiveProjection(float fov/*in degrees*/, float aspectRatio, float nearZ, float farZ)
    {
        //positive z direction is assumed
        float f = ToRadians(fov / 2.f);
        float tanf = tan(f); // 
        float d = 1 / tanf;
        float RangeZ = nearZ - farZ;
        float a =-farZ / RangeZ;
        float b =-nearZ * a;
        float x = d / aspectRatio;

        Matrix4 matrix{ x, 0,  0, 0,
                        0, d,  0, 0,
                        0, 0,  a, 1,
                        0, 0,  b, 0 };
        return matrix;
    }

    Matrix4 OrhographicProjection(float l, float r, float b, float t, float n, float f)
    {
        Matrix4 matrix;
        matrix[0][0] = 2 / (r - l);
        matrix[1][1] = -2 / (t - b);
        matrix[2][2] = 2 / (f - n);
        matrix[0][3] = -(r + l) / (r - l);
        matrix[1][3] = -(t + b) / (t - b);
        matrix[2][3] = -(n) / (f - n);
        matrix.TransposeSelf();
        return matrix;
    }

    Matrix4 Translate(Vector3 vec)
    {
        Matrix4 res;
        res[3][0] = -vec[0];
        res[3][1] = -vec[1];
        res[3][2] = -vec[2];
        res[3][3] = 1;
        return res;
    }

     Matrix4 RotateX(float degrees)
     {
         float radians = ToRadians(degrees);
         float sinValue = sin(radians);
         float cosValue = cos(radians);
         
         Matrix4 result;
         result[1][1] = cosValue;
         result[1][2] = -sinValue;
         result[2][1] = sinValue;
         result[2][2] = cosValue;
         return result;
     }

    Matrix4 RotateY(float degrees)
     {
         float radians = ToRadians(degrees);
         float sinValue = sin(radians);
         float cosValue = cos(radians);
         
         Matrix4 result;
         result[0][0] = cosValue;
         result[0][2] = sinValue;
         result[2][0] = -sinValue;
         result[2][2] = cosValue;
         return result;
     }

    Matrix4 RotateZ(float degrees)
     {
         float radians = ToRadians(degrees);
         float sinValue = sin(radians);
         float cosValue = cos(radians);
         
         Matrix4 result;
         result[0][0] = cosValue;
         result[0][1] = -sinValue;
         result[1][0] = sinValue;
         result[1][2] = cosValue;
         return result;
     }

    Matrix4 CreateViewRotationMatrix(const Vector3& viewDirection, Vector3 upDirection, Vector3 rightDirection)
    {
        Matrix4 res;
        res[0] = rightDirection;
        res[1] = upDirection;
        res[2] = viewDirection;
        res[3][3] = 1;
        return res.Transpose();
    }
};
