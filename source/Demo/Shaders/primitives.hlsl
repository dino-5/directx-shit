#include "Shaders/Ray.hlsl"

struct Sphere
{
    float3 center;
    float radius;
    Material mat;
    float pad;
};

#define MAX_NUMBER_OF_SPHERES 100
struct SphereArray
{
    Sphere spheres[MAX_NUMBER_OF_SPHERES];
};

struct CB_Sphere
{
    SphereArray array;
};

HitRecord hit_sphere(Sphere sphere, Ray ray,
                float tmin, float tmax)
{
    HitRecord hit = getHit();
    // a is always zero
    ray.dir = normalize(ray.dir);
    float3 co = sphere.center - ray.pos;
    float b = dot(ray.dir, co);

    float c = dot(co, co) - sphere.radius * sphere.radius;

    float d = b * b - c ;
    if (d < 0)
        return hit;

    d = sqrt(d);
    float t = b - d;
    if(t < tmin || t > tmax)
    {
        t = b + d;
        if(t < tmin || t > tmax)
            return hit;
    }

    hit.p = ray.at(t);
    hit.t = t;
    float3 n = (hit.p - sphere.center) / sphere.radius;
    hit.setNormal(ray, normalize(n));
    hit.m = sphere.mat;
    hit.hit = true;

    return hit;
}

HitRecord hitArray(SphereArray array, 
                  Ray ray, 
                  float2 interval, 
                  uint count)
{
    HitRecord record = getHit(); 
    bool hitAny = false;
    float closestHit = interval.y;

    for(uint i = 0; i < count; i++)
    {
        HitRecord tempRec = hit_sphere(array.spheres[i],
                              ray,
                              interval.x, closestHit);
        if(tempRec.hit) // we shrink interval to the closestHit every time
        {
            record = tempRec;
            record.hit = true;
            closestHit = tempRec.t;
        }
    }

    return record;
}
