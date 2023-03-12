#pragma once
#include "Framework/math/Vector.h"
#include "Framework/include/common.h"
#include "Framework/util/ImGuiSettings.h"
#include "Framework/math/Functions.h"	

namespace engine::math
{
template<int N>
class Matrix
{
public:
	Matrix()
	{
		for (int i = 0; i < N; i++)
			m_data[i][i] = 1;
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

	void TransposeSelf()
	{
		for (int i = 0; i < N; i++)
		{
			for (int j = 0; j < N; j++)
			{
				std::swap(m_data[i][j], m_data[j][i]);
			}
		}
	}

	static Matrix OrhographicProjection(int l, int r, int b, int t, int n, int f)
	{
		Matrix matrix;
		//matrix[0][0] = 2 / (r - l);
		//matrix[1][1] = 2 / (t - b);
		//matrix[2][2] =-2 / (f - n);
		//matrix[0][3] = -(r + l) / (r - l);
		//matrix[1][3] = -(t + b) / (t - b);
		//matrix[2][3] = -(f + n) / (f - n);
		matrix.TransposeSelf();
		return matrix;
	}

	static Matrix PerspectiveProjection(float fov/*in degrees*/, float aspectRatio, float nearZ, float farZ)
	{
		float tanf = tan(ToRadians(fov/2));
		float d = 1 / tanf;
		float RangeZ = nearZ - farZ;

		Matrix matrix{d/aspectRatio, 0, 0, 0,
						0, d, 0, 0,
						0, 0, (farZ+nearZ)/RangeZ, 2*farZ*nearZ/RangeZ,
						0, 0, -1, 0};
		//matrix.TransposeSelf();
		return matrix;
	}

	Vector<N>& operator[](int i)
	{
		return m_data[i];
	}
private:
	Vector<N> m_data[N];
};

using Matrix4 = Matrix<4>;

}
