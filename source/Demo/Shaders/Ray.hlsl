#include "Shaders/RandomUtilities.hlsl"
#define RED float4(1.0f, 0.f, 0.f, 1.f)

struct Ray
{
    float3 pos;
    float3 dir;

    float3 at(float t)
    {
        return pos + dir * t;
    }
};

Ray createRay(float3 p, float3 d)
{
    Ray r;
    r.pos = p;
    r.dir = d;
    return r;
}

struct Material
{
    float4 color;
    float fuzz;
    uint materialType;

    bool isLambertian()
    {
        return materialType == 0;
    }

    bool isMetal()
    {
        return materialType == 1;
    }
};

struct HitRecord 
{
    float3 p;
    float3 n;
    float t;
    Material m;
    bool frontFace;

    void setNormal(Ray r, float3 normal)
    {
        frontFace = dot(r.dir, normal) < 0;
        n = frontFace ? normal : -normal;
    }
};


struct Sphere
{
    float3 center;
    float radius;
    Material mat;
    float2 pad;
};



