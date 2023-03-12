#pragma once
#include "Framework/util/ImGuiSettings.h"
#include "Framework/include/types.h"
#include "Framework/include/common.h"
#include <initializer_list>
#include <math.h>

using std::initializer_list;
using namespace engine;
namespace engine::math
{


template<int N>
class Vector
{
public:
	Vector()
	{
		for (int i = 0; i < N; i++)
		{
			m_data[i] = 0;
		}
	}

	Vector(initializer_list<float> list) 
	{
		auto itr = list.begin();
		for (int i = 0; i < list.size() && i < N; i++)
		{
			m_data[i] = *itr;
			itr++;
		}
	}

	Vector(const Vector& vector)
	{
		for (int i = 0; i < N; i++)
		{
			m_data[i] = vector[i];
		}
	}

	template<int M>
	Vector(const Vector<M>& vector)
	{
		for (int i = 0; i < N && i < M; i++)
		{
			m_data[i] = vector[i];
		}
	}

	Vector& operator=(const Vector& v1)
	{
		for (int i = 0; i < N; i++)
		{
			m_data[i] = v1.m_data[i];
		}
		return *this;
	}
	
	friend const Vector operator+(const Vector& v1, const Vector& v2);
	friend const Vector operator-(const Vector& v1, const Vector& v2);
	friend const Vector CrossProduct(const Vector& v1, const Vector& v2);
	friend float DotProduct(const Vector& v1, const Vector& v2);

	float Length()
	{
		float result = 0;
		for (int i = 0; i < N; i++)
		{
			result += m_data[i] * m_data[i];
		}
		return sqrt(result);
	}

	const Vector Normalize()
	{
		Vector result;
		float len = Length();
		for (int i = 0; i < N; i++)
		{
			result[i] = m_data[i] / len;
		}
		return result;
	}

	void NormalizeSelf()
	{
		float len = Length();
		for (int i = 0; i < N; i++)
		{
			m_data[i] /= len;
		}
	}

	bool OnImGui(const char* name, float min=-10.f, float max=10.f)
	{
		if constexpr(N != 2)
			return util::ImGuiSettings::SliderFloat3(name, m_data, min, max);
		else
			return util::ImGuiSettings::SliderFloat2(name, m_data, min, max);
	}
	float& operator[](int i) { return m_data[i]; }
	float* data() { return m_data; }
	uint size() { return N * sizeof(float); }
	
private:
	float m_data[N];
};

template<int N>
const Vector<N> operator+(const Vector<N>& v1, const Vector<N>& v2)
{
	Vector<N> result;
	for (int i = 0; i < N; i++)
	{
		result[i] = v1[i] + v2[i];
	}
	return result;
}

template<int N>
const Vector<N> operator-(const Vector<N>& v1, const Vector<N>& v2)
{
	Vector<N> result;
	for (int i = 0; i < N; i++)
	{
		result[i] = v1[i] - v2[i];
	}
	return result;
}

template<int N>
const Vector<N> CrossProduct(const Vector<N>& v1, const Vector<N>& v2)
{
	Vector<N> result;
	if constexpr(N != 2)
	{
		result[0] = v1[1] * v2[2] - v1[2] * v2[1];
		result[1] = v1[2] * v2[0] - v1[0] * v2[2];
		result[2] = v1[0] * v2[1] - v1[1] * v2[0];
	}
	if constexpr(N == 4)
	{
		result[3] = 1;
	}
	return result;
}

template<int N>
float DotProduct(const Vector<N>& v1, const Vector<N>& v2)
{
	float res = 0.f;
	for (int i = 0; i < N; i++)
	{
		res += v1[i] * v2[i];
	}
	return res;
}

using Vector2 = Vector<2>;
using Vector3 = Vector<3>;
using Vector4 = Vector<4>;

};
