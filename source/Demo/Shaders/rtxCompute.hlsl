#include "Shaders/Ray.hlsl"

uint Hash(uint x)
{
    x ^= x >> 17;
    x *= 0xed5ad4bbU;
    x ^= x >> 11;
    x *= 0xac4c1b51U;
    x ^= x >> 15;
    x *= 0x31848babU;
    x ^= x >> 14;
    return x;
}

float Random1(inout uint x)
{
    float result =  (float)(Hash(x)) / 4294967296.0; // normalize to [0,1)
    x = Hash(x);
    return result;
}

float2 Random2(inout uint x)
{
    float2 result;
    result.x = Random1(x);
    x = Hash(x);
    result.y = Random1(x);
    x = Hash(x);
    return result;
}

float3 Random3(inout uint x)
{
    float3 result;
    result.x = Random1(x);
    x = Hash(x);
    result.y = Random1(x);
    x = Hash(x);
    result.z = Random1(x);
    x = Hash(x);
    return result;
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
    return float4((1 - blue) * float3(1,1,1) + blue * float3(0.5, 0.7, 1), 1.f); 
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
                      uint samples)
    {
        pos = cameraPos;

        V = cameraV;
        R = cameraR;
        U = cameraU;

        sampleCount = samples;

        uint seed = id.x + id.y * (uint)width;
        hash = Hash(seed);

        float aspectRatio = width / height;
        float3 viewPlaneC = pos + cameraV;
        float cosHalfFov = cos(fov/2);
        float sx = cosHalfFov * aspectRatio;
        float sy = cosHalfFov;

        float3 lb = viewPlaneC - sx * cameraR + sy * cameraU; // left bottom

        dx =  2 * sx / width;
        dy = 2 * sy / height;

        pixel = lb + dx * id.x * cameraR - dy * id.y * cameraU;
    }

    Ray getRay()
    {
        Ray r;
        r.pos = pos;
        r.dir = normalize(pixel - pos);
        return r;
    }

    Ray getRay(uint i)
    {
        Ray r;
        r.pos = pos;

        float randomX = dx * (Random1(hash) - 0.5f); 
        float randomY = dy * (Random1(hash) - 0.5f); 

        r.dir = normalize(pixel + R * randomX + U * randomY - pos);

        return r;
    }

    float4 render(SphereArray array, float2 interval, uint sphereCount)
    {

        Ray r = getRay();
        float blue = 0.5 * (r.dir.y + 1);
        float4 colorBlue = calculateSky(blue);
        float4 color = float4(0,0,0,0);
        HitRecord hit;

        float scale = 1 / float(sampleCount);
        for(int i = 0; i < sampleCount; i++)
        {
            if(array.hit(getRay(i), interval, hit, sphereCount))
            {
                color += float4(hit.n, 1.f);
            }
            else
                color += colorBlue;
        }

        return color / sampleCount;
    }

    float3 V;
    float3 R;
    float3 U;

    float3 pixel;
    float3 pos;
    float dx;
    float dy;
    float hash;
    uint sampleCount;

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
                        100);

    tex[id.xy] = camera.render(sphereArray.array,
                               float2(0, 100000),
                               g_rtxData.sphereCount); 
}

