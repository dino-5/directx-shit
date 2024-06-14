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
        float tanf = tan(ToRadians(fov / 2));
        float d = 1 / tanf;
        float RangeZ = farZ - nearZ;
        float a = farZ / RangeZ;
        float b = (-1.f) * nearZ * farZ / RangeZ;


        Matrix4 matrix{ d / aspectRatio, 0,  0, 0,
                        0, d,  0, 0,
                        0, 0,  a, b,
                        0, 0, 1, 0 };
        matrix.TransposeSelf();
        return matrix;
    }

    Matrix4 OrhographicProjection(int l, int r, int b, int t, int n, int f)
    {
        Matrix4 matrix;
        //matrix[0][0] = 2 / (r - l);
        //matrix[1][1] = 2 / (t - b);
        //matrix[2][2] =-2 / (f - n);
        //matrix[0][3] = -(r + l) / (r - l);
        //matrix[1][3] = -(t + b) / (t - b);
        //matrix[2][3] = -(f + n) / (f - n);
        matrix.TransposeSelf();
        return matrix;
    }

    Matrix4 Translate(const Matrix4& mat, Vector4 vec)
    {
        Matrix4 res = mat;
        res[3][0] = -vec[0];
        res[3][1] = -vec[1];
        res[3][2] = -vec[2];
        res[3][3] = -vec[3];
        return res;
        
    }
    Matrix4 Translate(const Matrix4& mat, Vector3 vec)
    {
        Matrix4 res = mat;
        res[3][0] = -vec[0];
        res[3][1] = -vec[1];
        res[3][2] = -vec[2];
        res[3][3] = 1;
        return res;
    }

    void SelfTranslate(Matrix4& mat, Vector4 vec)
    {
        mat[3][0] = -vec[0];
        mat[3][1] = -vec[1];
        mat[3][2] = -vec[2];
        mat[3][3] = -vec[3];
    }
    void SelfTranslate(Matrix4& mat, Vector3 vec)
    {
        mat[3][0] = -vec[0];
        mat[3][1] = -vec[1];
        mat[3][2] = -vec[2];
        mat[3][3] = 1;
    }


};
