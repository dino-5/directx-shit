#pragma once
#include "EngineCommon/math/Vector.h"
#include "EngineCommon/util/ImGuiSettings.h"
#include "EngineCommon/math/Functions.h"	

#include <iostream>

namespace engine::math
{
template<int N>
struct Matrix
{
public:
	Matrix()
	{
		for (int i = 0; i < N; i++)
			m_data[i][i] = 1;
	}

	Matrix(float f)
	{
		for (int i = 0; i < N; i++)
			m_data[i][i] = f;
	}
	Matrix(initializer_list<Vector<N>> list) 
	{
		auto itr = list.begin();
		for (int i = 0; i < list.size() && i < N; i++)
		{
			m_data[i] = *itr;
			itr++;
		}
	}

	Matrix(initializer_list<float> list) 
	{
		auto itr = list.begin();
		for (int i = 0; i < list.size() && i < N*N; i++)
		{
			m_data[i/N][i%N] = *itr;
			itr++;
		}
	}

	Matrix(const Matrix& matrix)
	{
		for (int i = 0; i < N; i++)
		{
			m_data[i] = matrix.m_data[i];
		}
	}
	Matrix Transpose()
	{
		Matrix res;
		for (int i = 0; i < N; i++)
		{
			for (int j = 0; j < N; j++)
			{
				res.m_data[i][j] = m_data[j][i];
			}
		}
		return res;
	}
	Matrix operator*(const Matrix& matrix)
	{
		Matrix result(0);
		for (int i = 0; i < N; i++)
		{
			for (int j = 0; j < N; j++)
			{
				for(int k=0; k<N; k++)
                    result[i][j] += m_data[i][k] * matrix[k][j];
			}
		}
		return result;
	}

	void Print()
	{
		for (int i = 0; i < N; i++)
		{
			for (int j = 0; j < N; j++)
			{
				std::cout << m_data[i][j] << " ";
			}
			std::cout << "\n";
		}
	}

	Matrix(const Quartenion& quart)
	{
        static_assert(N==4, "dimension!=4 to create matrix from quartenion");
		float qx2 = quart[0] * quart[0];
		float qy2 = quart[1] * quart[1];
		float qz2 = quart[2] * quart[2];
		float qxy = quart[0] * quart[1];
		float qxz = quart[0] * quart[2];
		float qxw = quart[0] * quart[3];
		float qyz = quart[1] * quart[2];
		float qyw = quart[1] * quart[3];
		float qzw = quart[2] * quart[3];

		m_data[0][0] = 1-2*(qy2+qz2);
		m_data[0][1] = 2 * (qxy + qzw);
		m_data[0][2] = 2 * (qxz - qyw);
		m_data[0][3] = 0;
		
		m_data[1][0] = 2 * (qxy - qzw);
		m_data[1][1] = 1 - 2 * (qx2 + qz2);
		m_data[1][2] = 2 * (qyz + qxw);
		m_data[1][3] = 0;
		
		m_data[2][0] = 2 * (qxz + qyw);
		m_data[2][1] = 2 * (qyz - qxw);
		m_data[2][2] = 1 - 2 * (qx2 + qy2);
		m_data[2][3] = 0;
		
		m_data[3][0] = 0;
		m_data[3][1] = 0;
		m_data[3][2] = 0;
		m_data[3][3] = 1;
		TransposeSelf();

	}

	void TransposeSelf()
	{
		for (int i = 1; i < N; i++)
		{
			for (int j = 0; j <= (i+1)/2; j++)
			{
				std::swap(m_data[i][j], m_data[j][i]);
			}
		}
	}


	Vector<N>& operator[](int i)
	{
		return m_data[i];
	}

	Vector<N> operator[](int i)const
	{
		return m_data[i];
	}
public:
	Vector<N> m_data[N];

	template<int M>
	friend auto operator*(Matrix<M> m, Vector<M> v);
};

template<int N>
auto operator*(Matrix<N> m, Vector<N> v)
{
	Vector<N> result;
	for (int i = 0; i < N; i++)
		for (int j = 0; j < N; j++)
			result[i] += m[i][j] * v[j];
	return result;
}

using Matrix4 = Matrix<4>;

// view matrices 
Matrix4 RotateX(float degrees);
Matrix4 RotateY(float degrees);
Matrix4 RotateZ(float degrees);
Matrix4 Translate(Vector3 vec);

Matrix4 CreateViewRotationMatrix(const Vector3& viewDirection, Vector3 upDirection, Vector3 rightDirection);

// projection matrices
Matrix4 PerspectiveProjection(float fov/*in degrees*/, float aspectRatio, float nearZ, float farZ);
Matrix4 OrhographicProjection(int l, int r, int b, int t, int n, int f);

}
