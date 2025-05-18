#pragma once
#include "EngineCommon/include/types.h"

#include <algorithm>
#include <initializer_list>
#include <system_error>
#include <type_traits>
#include <math.h>
#include <array>

using std::initializer_list;

namespace engine::math
{

template<int N, typename T = float>
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

    Vector(T value)
    {
        for (int i = 0; i < N; i++)
            m_data[i] = value;
    }

    template<typename M>
    Vector(M value)
    {
        for (int i = 0; i < N; i++)
            m_data[i] = (T)value;
    }

    Vector(initializer_list<T> list) 
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

    template<typename T1>
    Vector(const Vector<N, T1>& vector)
    {
        for (int i = 0; i < N ; i++)
        {
            m_data[i] = (T)vector[i];
        }
    }

    template<int M>
    Vector(const Vector<M, T>& vector, std::initializer_list<T> list = {})
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
    Vector(const Vector<M, T>& vector, Args... args)
    {
        static_assert(M + sizeof...(Args) == N);
        int i = 0;
        for (auto& el : vector)
            m_data[i++] = el;

        for (auto el : std::initializer_list<T>{ args... })
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
    
    template<int M, typename T1>
    friend const Vector<M,T1> operator*(const Vector<M,T1>& v1, const float v);
    template<int M, typename T1>
    friend const Vector<M,T1> operator*(const float v, const Vector<M, T1>& v1);

    template<int M, typename T1>
    friend const Vector<M,T1> operator/(const Vector<M,T1>& v1, const float v);
    template<int M, typename T1>
    friend const Vector<M,T1> operator/(const float v, const Vector<M,T1>& v1);

    template<int M, typename T1>
    friend const Vector<M,T1> CrossProduct(const Vector<M,T1>& v1, const Vector<M, T1>& v2);
    template<int M, typename T1>
    friend float DotProduct(const Vector<M, T1>& v1, const Vector<M,T1>& v2);

    template<int M, typename T1>
    friend const Vector<M,T1> PerElementOperation(const Vector<M,T1>& v1, const Vector<M,T1>& v2, float (*op)(T1, T1));

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

    T& operator[](int i) { return m_data[i]; }
    T operator[](int i) const { return m_data[i]; }
    T* data() { return m_data.data(); }
    uint size() { return N * sizeof(float); }
    
private:
    std::array<T, N> m_data;
};

using Vector2 = Vector<2>;
using Vector3 = Vector<3>;
using Vector4 = Vector<4>;
using Int3 = Vector<3, int>;
using Int4 = Vector<4, int>;

template<int N, typename T=float>
inline Vector<N,T> minVectorCoords(const Vector<N,T>& v1, const Vector<N,T>& v2)
{
    Vector<N,T> result;
    for(int i = 0; i < N; ++i)
    {
        result[i] = (T)min(v1[i], v2[i]);
    }
    return result;
}

template<int N, typename T=float>
inline Vector<N,T> maxVectorCoords(const Vector<N,T>& v1, const Vector<N,T>& v2)
{
    Vector<N,T> result;
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

template<int M, typename T>
const Vector<M,T> operator+(const Vector<M,T>& v1, const Vector<M,T>& v2)
{
    return PerElementOperation(v1, v2, [](T a, T b) {return a + b; });
}

template<int N, typename T=float>
const Vector<N,T> operator-(const Vector<N,T>& v1, const Vector<N,T>& v2)
{
    return PerElementOperation(v1, v2, [](float a, float b) {return a - b; });
}

template<int M, typename T>
const Vector<M,T> operator*(const Vector<M,T>& v1, const Vector<M,T>& v2)
{
    return PerElementOperation(v1, v2, [](T a, T b) {return a * b; });
}

template<int N, typename T=float>
const Vector<N,T> operator/(const Vector<N,T>& v1, const Vector<N,T>& v2)
{
    return PerElementOperation(v1, v2, [](float a, float b) {return a / b; });
}


template<int N, typename T=float>
const Vector<N,T> VectorOpFloat(const Vector<N,T>& v1, const float v, float (*op)(float, float))
{
    Vector<N,T> result;
    for (int i = 0; i < N; i++)
    {
        result[i] = op(v1[i], v);
    }
    return result;
}

template<int N, typename T=float>
const Vector<N,T> operator*(const Vector<N,T>& v1, const float v)
{
    return VectorOpFloat(v1, v, [](float a, float b) { return a * b; });
}

template<int N, typename T=float>
const Vector<N,T> operator*(const float v, const Vector<N,T>& v1)
{
    return v1*v;
}

template<int N, typename T=float>
const Vector<N,T> operator/(const Vector<N,T>& v1, const float v)
{
    return VectorOpFloat(v1, v, [](float a, float b) { return a / b; });
}

template<int N, typename T=float>
const Vector<N,T> operator/(const float v, const Vector<N,T>& v1)
{
    return VectorOpFloat(v1, v, [](float a, float b) { return b / a; });
}


template<int N, typename T=float>
const Vector<N,T> CrossProduct(const Vector<N,T>& v1, const Vector<N,T>& v2)
{
    Vector<N,T> result;
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

template<int N, typename T=float>
float DotProduct(const Vector<N,T>& v1, const Vector<N,T>& v2)
{
    float res = 0.f;
    for (int i = 0; i < N; i++)
    {
        res += v1[i] * v2[i];
    }
    return res;
}

template<typename T>
inline T clamp(T value, T min, T max)
{
    if(value >= min && value <= max) return value;
    if(value < min) return min;
    return max;
}

template<int M,typename T>
inline Vector<M,T> clamp(const Vector<M,T>& value, const Vector<M,T>& min, const Vector<M,T>& max)
{
    Vector<M,T> result;
    for(u32 i = 0; i < M; ++i)
        result[i] = math::clamp(value[i], min[i], max[i]);
    return result;

}

};
