#include "Shaders/RandomUtilities.hlsl"
#define RED float4(1.0f, 0.f, 0.f, 1.f)

struct Material
{
    float4 color;
    float fuzz;
    float refractionIndex;
    uint materialType;

    bool isLambertian()
    {
        return materialType == 0;
    }

    bool isMetal()
    {
        return materialType == 1;
    }

    bool isDielectric()
    {
        return materialType == 2;
    }
};

Material getMaterial()
{
    Material m;
    m.color = float4(0,0,0,0);
    m.fuzz = 0;
    m.refractionIndex = 0;
    m.materialType = 0;
    return m;
}

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

struct HitRecord 
{
    float3 p;
    float3 n;
    float t;
    Material m;
    bool frontFace;
    bool hit;
    uint index;

    void setNormal(Ray r, float3 normal)
    {
        frontFace = dot(r.dir, normal) < 0;
        n = frontFace ? normal : -normal;
    }
};

HitRecord getHit()
{
    HitRecord rec;
    rec.hit = false;
    rec.n = float3(0,0,0);
    rec.p = float3(0,0,0);
    rec.t = 1000;
    rec.m = getMaterial();
    rec.index = 0;
    return rec;
}

float3 refractRay(float3 r, float3 n, float ri) // refract is HLSL function :)
{
    float cos_theta = min(dot(-r, n), 1);
    float sin_theta = sqrt( 1 - cos_theta*cos_theta);

    if(sin_theta * ri > 1)
        return reflect(r, n);

    // TODO : implement reflectance
    float3 r_out_perp = ri * (r + cos_theta * n);
    float len = length(r_out_perp);
    float3 r_out_parallel = -sqrt(abs(1 - len*len)) * n;
    return r_out_perp + r_out_parallel;
}


// NOTE : for some reason direct call to scatter_lambert is not the same as
// call through this function
bool scatter(Ray r, 
             HitRecord hit,
             inout float4 attenuation,
             inout Ray scattered,
             inout uint hash)
{
    attenuation = hit.m.color;
    scattered.pos = hit.p;
    bool fl = hit.m.isLambertian();
    bool result = true;

    if(fl)
    {
        scattered.dir = hit.n + RandomOnHemisphere(hit.n, hash); // could be zero vector
    }
    else
    {
        scattered.dir = reflect(r.dir, hit.n);
        scattered.dir +=  hit.m.fuzz * RandomOnHemisphere(hit.n, hash);
        result = dot(scattered.dir, hit.n) > 0;
    }
    scattered.dir = normalize(scattered.dir);

    return result;
}



