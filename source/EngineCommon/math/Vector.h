#pragma once
#include "EngineCommon/include/types.h"

#include <algorithm>
#include <initializer_list>
#include <type_traits>
#include <math.h>
#include <array>

using std::initializer_list;

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

    Vector(Vector&& other) : m_data(std::move(other.m_data)){}

    Vector(float value)
    {
        for (int i = 0; i < N; i++)
            m_data[i] = value;
    }

    Vector(double value)
    {
        for (int i = 0; i < N; i++)
            m_data[i] = (float)value;
    }

    Vector(initializer_list<float> list) 
    {
        auto itr = list.begin();
        for (int i = 0; i < list.size() && i < N; i++)
            m_data[i] = *(itr++);
    }

    Vector(const Vector& vector)
    {
        for (int i = 0; i < N; i++)
        {
            m_data[i] = vector.m_data[i];
        }
    }

    template<int M>
    Vector(const Vector<M>& vector, std::initializer_list<float> list = {})
    {
        for (int i = 0; i < N && i < M; i++)
        {
            m_data[i] = vector[i];
        }
        if constexpr (M < N)
        {
            if (list.size()!=0)
            {
                auto itr = list.begin();
                for (int i = M; i < N; i++)
                {
                    m_data[i] = *itr;
                    itr++;
                }

            }
            else
                for (int i = M; i < N; i++)
                    m_data[i] = 0;
        }
    }

    template<int M, typename... Args>
    Vector(const Vector<M>& vector, Args... args)
    {
        static_assert(M + sizeof...(Args) == N);
        int i = 0;
        for (auto& el : vector)
            m_data[i++] = el;

        for (auto el : std::initializer_list<float>{ args... })
            m_data[i++] = el;
    }

    Vector& operator=(const Vector& v1)
    {
        for (int i = 0; i < N; i++)
        {
            m_data[i] = v1.m_data[i];
        }
        return *this;
    }

    bool operator==(const Vector& v) const 
    {
        for (int i = 0; i < N; i++)
            if(m_data[i] != v.m_data[i])
                return false;
        return true;
    }

    auto begin() { return m_data.begin(); }
    auto begin() const{ return m_data.begin(); }
    auto end() { return m_data.end(); }
    auto end() const { return m_data.end(); }
    
    template<int M>
    friend const Vector<M> operator*(const Vector<M>& v1, const float v);
    template<int M>
    friend const Vector<M> operator*(const float v, const Vector<M>& v1);

    template<int M>
    friend const Vector<M> operator/(const Vector<M>& v1, const float v);
    template<int M>
    friend const Vector<M> operator/(const float v, const Vector<M>& v1);

    template<int M>
    friend const Vector<M> CrossProduct(const Vector<M>& v1, const Vector<M>& v2);
    template<int M>
    friend float DotProduct(const Vector<M>& v1, const Vector<M>& v2);

    template<int M>
    friend const Vector<M> PerElementOperation(const Vector<M>& v1, const Vector<M>& v2, float (*op)(float, float));

    float length() const
    {
        float result = 0;
        for (int i = 0; i < N; i++)
        {
            result += m_data[i] * m_data[i];
        }
        return (float)sqrt(result);
    }

    Vector normalize() const
    {
        Vector result;
        float len = length();
        for (int i = 0; i < N; i++)
        {
            result[i] = m_data[i] / len;
        }
        return result;
    }

    void normalizeSelf()
    {
        float len = length();
        for (int i = 0; i < N; i++)
        {
            m_data[i] /= len;
        }
    }

    float& operator[](int i) { return m_data[i]; }
    float operator[](int i) const { return m_data[i]; }
    float* data() { return m_data.data(); }
    uint size() { return N * sizeof(float); }
    
private:
    std::array<float, N> m_data;
};

using Vector2 = Vector<2>;
using Vector3 = Vector<3>;
using Vector4 = Vector<4>;

template<int N>
inline Vector<N> minVectorCoords(const Vector<N>& v1, const Vector<N>& v2)
{
    Vector<N> result;
    for(int i = 0; i < N; ++i)
    {
        result[i] = (float)fmin(v1[i], v2[i]);
    }
    return result;
}

template<int N>
inline Vector<N> maxVectorCoords(const Vector<N>& v1, const Vector<N>& v2)
{
    Vector<N> result;
    for(int i = 0; i < N; ++i)
    {
        result[i] = (float)fmax(v1[i], v2[i]);
    }
    return result;
}



class Quartenion 
{
public:
    Quartenion(Vector3 direction, float angle /*in degrees*/);
    Quartenion(Vector4 vec) :m_quarternion(vec) {}
    Quartenion conjugated() const { return  Vector4(-1 * Vector3(m_quarternion), { m_quarternion[3] }); }
    float& operator[](i32 index) { return m_quarternion[index]; }
    float operator[](i32 index) const { return m_quarternion[index]; }
    float real()const { return m_quarternion[3]; }

    Quartenion operator*(const Quartenion& q1);
private:
    Vector4 m_quarternion;
};


template<int M>
const Vector<M> PerElementOperation(const Vector<M>& v1, const Vector<M>& v2, float (*op)(float, float))
{
    Vector<M> result;
    for (int i = 0; i < M; i++)
        result[i] = op(v1[i], v2[i]);
    return result;
}

template<int M>
const Vector<M> operator+(const Vector<M>& v1, const Vector<M>& v2)
{
    return PerElementOperation(v1, v2, [](float a, float b) {return a + b; });
}

template<int N>
const Vector<N> operator-(const Vector<N>& v1, const Vector<N>& v2)
{
    return PerElementOperation(v1, v2, [](float a, float b) {return a - b; });
}

template<int M>
const Vector<M> operator*(const Vector<M>& v1, const Vector<M>& v2)
{
    return PerElementOperation(v1, v2, [](float a, float b) {return a * b; });
}

template<int N>
const Vector<N> operator/(const Vector<N>& v1, const Vector<N>& v2)
{
    return PerElementOperation(v1, v2, [](float a, float b) {return a / b; });
}

template<int N>
const Vector<N> VectorOpFloat(const Vector<N>& v1, const float v, float (*op)(float, float))
{
    Vector<N> result;
    for (int i = 0; i < N; i++)
    {
        result[i] = op(v1[i], v);
    }
    return result;
}

template<int N>
const Vector<N> operator*(const Vector<N>& v1, const float v)
{
    return VectorOpFloat(v1, v, [](float a, float b) { return a * b; });
}

template<int N>
const Vector<N> operator*(const float v, const Vector<N>& v1)
{
    return v1*v;
}

template<int N>
const Vector<N> operator/(const Vector<N>& v1, const float v)
{
    return VectorOpFloat(v1, v, [](float a, float b) { return a / b; });
}

template<int N>
const Vector<N> operator/(const float v, const Vector<N>& v1)
{
    return VectorOpFloat(v1, v, [](float a, float b) { return b / a; });
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

};
