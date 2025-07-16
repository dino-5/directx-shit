#include "Shaders/Ray.hlsl"

#define PI 3.1415

float degrees_to_radians(float degrees)
{
    return degrees / 180 * PI;
}

struct ViewSettings
{
    float4x4 cameraMatrix;
    float4x4 projection;
    float3 cameraPos;
    float pad0;
    float3 cameraViewDir;
    float pad1;
    float3 cameraRightDir;
    float pad2;
    float3 cameraUpDir;
    float pad3;
    float fov;
};

float4 calculateSky(float blue)
{
    return float4((1 - blue) * float2(1,1) + blue * float2(0.5, 0.7), 
                  1.f, 1.f); 
}


struct Camera
{

    void createCamera(float width,
                      float height,
                      float3 cameraPos,
                      float3 cameraV,
                      float3 cameraR,
                      float3 cameraU,
                      float fov,
                      uint2 id,
                      uint samples,
                      uint depth,
                      float lensR,
                      float focusD)
    {
        lensAngle = lensR;
        focusDist = focusD;
        pos = cameraPos;

        V = cameraV;
        R = cameraR;
        U = cameraU;

        sampleCount = samples;
        maxDepth = depth;

        uint seed = id.x + id.y * (uint)width;
        hash = Hash(seed);

        float aspectRatio = width / height;
        viewPlaneC = pos + cameraV * focusDist;
        float cosHalfFov = cos(fov/2);
        float sy = cosHalfFov * focusDist;
        float sx = sy * aspectRatio; 

        float3 lb = viewPlaneC - sx * cameraR + sy * cameraU; // left bottom

        dx =  2 * sx / width;
        dy = 2 * sy / height;

        pixel = lb + dx * id.x * cameraR - dy * id.y * cameraU;

        float defocusRadius = focusDist * tan(degrees_to_radians(lensAngle / 2));
        defocus_disk_u = U * defocusRadius;
        defocus_disk_r = R * defocusRadius;

    }

    Ray getRay()
    {
        Ray r;
        float3 rayOrigin = pos;
        r.pos = rayOrigin;
        r.dir = normalize(pixel - pos);
        return r;
    }

    Ray getRay(uint i)
    {
        Ray r;

        r.pos = defocus_disk_sample();

        float randomX = dx * (Random1(hash) - 0.5f); 
        float randomY = dy * (Random1(hash) - 0.5f); 

        r.dir = normalize(pixel + R * randomX + U * randomY - r.pos);

        return r;
    }

    float4 pixelColor(SphereArray array, Ray r)
    {
        float4 color = float4(1,1,1,1);

        HitRecord hitRec;
        for(int i = 0; i < maxDepth; i++)
        {
            hitRec = hitArray(array, r, interv, sphCount);
            if(hitRec.hit)
            {
                Ray scattered;
                float4 attenuation;

                scattered.pos = hitRec.p;
                bool result = true;

                if(hitRec.m.isLambertian())
                {
                    attenuation = hitRec.m.color;
                    scattered.dir = hitRec.n + RandomOnHemisphere(hitRec.n, hash); // could be zero vector
                }
                else if (hitRec.m.isMetal())
                {
                    attenuation = hitRec.m.color;
                    scattered.dir = reflect(r.dir, hitRec.n);
                    scattered.dir +=  hitRec.m.fuzz * RandomOnHemisphere(hitRec.n, hash);
                    result = dot(scattered.dir, hitRec.n) > 0;
                }
                else if(hitRec.m.isDielectric())
                {
                    attenuation = float4(1,1,1,1);
                    float rIndex = hitRec.m.refractionIndex;
                    float ri = hitRec.frontFace ? 1 / rIndex : rIndex;
                    scattered.dir = refractRay(r.dir, hitRec.n, ri);
                }
                else
                {
                    break;
                }

                scattered.dir = normalize(scattered.dir);

                if(result)
                {
                    r = scattered;
                    color *= attenuation;
                }
                else
                {
                    color = float4(0,0,0,0);
                    break;

                }
            }
            else 
            {

                r.dir = normalize(r.dir);
                float blue = 0.5 * (r.dir.y + 1);
                color *= calculateSky(blue);
                break;
            }
        }

        return color;
    }

    float3 defocus_disk_sample() {
        // Returns a random point in the camera defocus disk.
        float2 p = random_in_unit_disk(hash);
        return pos + (p.x * defocus_disk_r) + (p.y * defocus_disk_u);
    }

    float4 render(SphereArray array, float2 interval, uint sphereCount)
    {
        Ray r = getRay();
        float4 color = float4(0,0,0,0);

        interv = interval;
        sphCount = sphereCount;

        float scale = 1 / float(sampleCount);
        for(int i = 0; i < sampleCount; i++)
        {
            color += pixelColor(array, getRay(i));
        }

        return color / sampleCount;
    }

    float3 V;
    float3 R;
    float3 U;

    float3 viewPlaneC;

    float3 pixel;
    float3 pos;
    float dx;
    float dy;
    uint hash;
    uint sampleCount;
    uint maxDepth;

    uint sphCount;
    float2 interv;

    float lensAngle;
    float focusDist;
    float3   defocus_disk_u;       // Defocus disk horizontal radius
    float3   defocus_disk_r;       // Defocus disk vertical radius

};

struct RTXData
{
    uint sphereCount;
    uint imWidth;
    uint imHeight;
    float3 color;
};

struct IndirectIndices
{
    uint outputTextureIndex;
    uint sphereBufferIndex;
};

ConstantBuffer<ViewSettings> g_view : register(b0);
ConstantBuffer<RTXData> g_rtxData : register(b1);
ConstantBuffer<IndirectIndices> bindless : register(b2);

#define RDH(index) ResourceDescriptorHeap[index]

[numthreads(8, 8, 1)]
void CSMain(uint3 id : SV_DispatchThreadID)
{
    if(id.x >= g_rtxData.imWidth || id.y >= g_rtxData.imHeight)
        return;

    RWTexture2D<float4> tex = RDH(bindless.outputTextureIndex);
    ConstantBuffer<CB_Sphere> sphereArray = RDH(bindless.sphereBufferIndex);


    Camera camera;
    camera.createCamera(g_rtxData.imWidth,
                        g_rtxData.imHeight,
                        g_view.cameraPos,
                        g_view.cameraViewDir,
                        g_view.cameraRightDir,
                        g_view.cameraUpDir,
                        g_view.fov,
                        id.xy,
                        50,
                        4,
                        0.6,
                        10);

    float4 color = camera.render(sphereArray.array,
                               float2(0.001, 100000),
                               g_rtxData.sphereCount); 
    color.xyz = pow(color.xyz, 1/2.2);
    tex[id.xy] = color;
}

