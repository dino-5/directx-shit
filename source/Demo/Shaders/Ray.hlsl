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

struct HitRecord 
{
    float3 p;
    float3 n;
    float t;
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
    float4 color;
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


