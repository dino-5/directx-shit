#define RED float4(1.0f, 0.f, 0.f, 1.f)

struct Ray
{
    float3 pos;
    float3 dir;
};

struct Sphere
{
    float3 center;
    float radius;
    float4 color;
};

#define MAX_NUMBER_OF_SPHERES 100
struct CB_Sphere
{
    Sphere spheres[MAX_NUMBER_OF_SPHERES];
};

bool hit_sphere(Sphere sphere, Ray ray)
{
    float a = dot(ray.dir, ray.dir);

    float3 co = sphere.center - ray.pos;
    float b = dot(ray.dir, co) * -2;

    float c = dot(co, co) - 100;

    float d = b*b - 4 * a * c ;
    if(d<0)
        return false;

    d = sqrt(d);
    float t = (-b - d) / (2 * a);
    return t>0;
}
