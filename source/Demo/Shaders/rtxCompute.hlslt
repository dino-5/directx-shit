#include "Shaders/Ray.hlsl"

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

struct Camera
{

    void createCamera(float width,
                      float height,
                      float3 cameraPos,
                      float3 cameraV,
                      float3 cameraR,
                      float3 cameraU,
                      float fov,
                      uint2 id)
    {

        float aspectRatio = width / height;
        float3 viewPlaneC = cameraPos + cameraV;
        float cosHalfFov = cos(fov/2);
        float sx = cosHalfFov * aspectRatio;
        float sy = cosHalfFov;

        float3 lb = viewPlaneC - sx * cameraR+ sy * cameraU; // left bottom

        float dx =  2 * sx / width;
        float dy = 2 * sy / height;
        pixel = lb + dx * id.x * cameraR - dy * id.y * cameraU;
    }

    float3 pixel;

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

    Camera camera;
    camera.createCamera(g_rtxData.imWidth,
                        g_rtxData.imHeight,
                        g_view.cameraPos,
                        g_view.cameraViewDir,
                        g_view.cameraRightDir,
                        g_view.cameraUpDir,
                        g_view.fov,
                        id.xy);

    Ray ray;
    ray.pos = g_view.cameraPos;
    ray.dir = normalize(camera.pixel - g_view.cameraPos);

    RWTexture2D<float4> tex = RDH(bindless.outputTextureIndex);
    ConstantBuffer<CB_Sphere> sphereArray = RDH(bindless.sphereBufferIndex);
    
    float4 color = float4(g_rtxData.color, 1.f);

    for(int i = 0; i < g_rtxData.sphereCount; ++i)
    {
        Sphere sph = sphereArray.array.spheres[i];
        HitRecord hit;
        bool result = hit_sphere(sph, ray, 0, 1000, hit);
        if(result)
        {
            color = float4(hit.n, 1.f);
        }

    }

    tex[id.xy] = color; 
}

