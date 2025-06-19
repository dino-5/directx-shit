struct ViewSettings
{
    float4x4 cameraMatrix;
    float4x4 projection;
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

#define RDH(index) ResourceDescriptorHeap[index]

[numthreads(8, 8, 1)]
void CSMain(uint3 id : SV_DispatchThreadID)
{
    if(id.x >= g_rtxData.imWidth || id.y >= g_rtxData.imHeight)
        return;

    RWTexture2D<float4> tex = RDH(bindless.outputTextureIndex);
    float2 uv = float2(id.xy) / float2(g_rtxData.imWidth, g_rtxData.imHeight);
    uv = uv * 2 - 1;

    ConstantBuffer<CB_Sphere> sphereArray = RDH(bindless.sphereBufferIndex);
    float4 color = float4(g_rtxData.color, 1.f);
    for(int i = 0; i < g_rtxData.sphereCount; ++i)
    {
        float radius = sphereArray.spheres[i].radius;
        float len = dot(uv, uv); 
        if(len < radius * radius)
            color = float4(1.0f, 0.f, 1.f, 1.f);
    }

    tex[id.xy] = color; 
}

