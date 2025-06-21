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

    float aspectRatio = float(g_rtxData.imWidth) / float(g_rtxData.imHeight);
    float3 viewPlaneCenter = g_view.cameraPos + g_view.cameraViewDir;
    float cosHalfFov = cos(g_view.fov/2);
    float sx = cosHalfFov * aspectRatio;
    float sy = cosHalfFov;

    float3 lb = viewPlaneCenter - sx * g_view.cameraRightDir - 
        sy * g_view.cameraUpDir; // left bottom

    float dx =  2 * sx / float(g_rtxData.imWidth);
    float dy = 2 * sy / float(g_rtxData.imHeight);
    float3 currentPixel = lb + dx * id.x * g_view.cameraRightDir +
        dy * id.y * g_view.cameraUpDir;

    Ray ray;
    ray.pos = g_view.cameraPos;
    ray.dir = normalize(currentPixel - g_view.cameraPos);

    RWTexture2D<float4> tex = RDH(bindless.outputTextureIndex);
    ConstantBuffer<CB_Sphere> sphereArray = RDH(bindless.sphereBufferIndex);
    
    float4 color = float4(g_rtxData.color, 1.f);

    for(int i = 0; i < g_rtxData.sphereCount; ++i)
    {
        if(hit_sphere(sphereArray.spheres[i], ray))
            color = RED;

    }

    tex[id.xy] = color; 
}

