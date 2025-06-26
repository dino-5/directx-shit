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
    float3 pad;
};

bool hit_sphere(Sphere sphere, Ray ray,
                float tmin, float tmax,
                out HitRecord record)
{
    // a is always zero
    float3 co = sphere.center - ray.pos;
    float b = dot(ray.dir, co);

    float c = dot(co, co) - sphere.radius * sphere.radius;

    float d = b * b - c ;
    if (d < 0)
        return false;

    d = sqrt(d);
    float t = b - d;
    record.m = sphere.mat;
    if(t <= tmin || t >= tmax)
    {
        t = b + d;
        if(t <= tmin || t >= tmax)
            return false;
    }

    record.p = ray.at(t);
    record.t = t;
    float3 n = (record.p - sphere.center) / sphere.radius;
    record.setNormal(ray, n);

    return true;
}

bool scatter_Lambertian(Ray r, 
                     HitRecord hit,
                     out float4 attenuation,
                     out Ray scattered,
                     inout uint hash)
{
    attenuation = hit.m.color;

    scattered.dir = normalize(hit.n + Random3Unit(hash)); // could be zero vector
    scattered.pos = hit.p;
    
    return true;
}

bool scatter_Metalic(Ray r, 
                     HitRecord hit,
                     out float4 attenuation,
                     out Ray scattered,
                     inout uint hash)
{
    attenuation = hit.m.color;

    scattered.dir = normalize(reflect(r.dir, hit.n));
    scattered.pos = hit.p;
    
    return true;
}

bool scatter_Dielectric(Ray r, 
                     HitRecord hit,
                     out float4 attenuation,
                     out Ray scattered,
                     inout uint hash)
{
    attenuation = hit.m.color;

    scattered.dir = reflect(r.dir, hit.n);//r.dir - 2 * dot(hit.n, r.dir) * hit.n;
    scattered.pos = hit.p;
    
    return true;
}

// NOTE : for some reason direct call to scatter_lambert is not the same as
// call through this function
bool scatter(Ray r, 
             HitRecord hit,
             out float4 attenuation,
             out Ray scattered,
             inout uint hash)
{
    attenuation = hit.m.color;
    scattered.pos = hit.p;
    if(hit.m.isLambertian())
    {
        scattered.dir = hit.n + Random3Unit(hash); // could be zero vector
    }
    else
    {
        scattered.dir = reflect(r.dir, hit.n);//r.dir - 2 * dot(hit.n, r.dir) * hit.n;
    }

    return true;
}

#define MAX_NUMBER_OF_SPHERES 100
struct SphereArray
{
    Sphere spheres[MAX_NUMBER_OF_SPHERES];

    bool hit(Ray ray, float2 interval, out HitRecord record, uint count)
    {
        HitRecord tempRec;
        bool hitAny = false;
        float closestHit = interval.y;

        for(uint i = 0; i < count; i++)
        {
            bool res = hit_sphere(spheres[i],
                                  ray,
                                  interval.x, closestHit,
                                  tempRec);
            if(res) // we shrink interval to the closestHit every time
            {
                hitAny = true;
                record = tempRec;
                closestHit = tempRec.t;
            }
        }

        return hitAny;
    }
};

struct CB_Sphere
{
    SphereArray array;
};


