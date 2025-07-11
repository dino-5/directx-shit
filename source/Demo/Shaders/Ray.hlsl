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

struct HitRecord 
{
    float3 p;
    float3 n;
    float t;
    Material m;
    bool frontFace;
    bool hit;

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
    rec.t = 0;
    return rec;
}


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


struct CB_Sphere
{
    SphereArray array;
};



