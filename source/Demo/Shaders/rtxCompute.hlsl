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
};

ConstantBuffer<ViewSettings> g_view : register(b0);
ConstantBuffer<RTXData> g_rtxData : register(b1);
ConstantBuffer<IndirectIndices> bindless : register(b2);

[numthreads(8, 8, 1)]
void CSMain(uint3 id : SV_DispatchThreadID)
{
    if(id.x >= g_rtxData.imWidth || id.y >= g_rtxData.imHeight)
        return;
    RWTexture2D<float4> tex = 
      ResourceDescriptorHeap[bindless.outputTextureIndex];
    tex[id.xy] = float4(g_rtxData.color, 1.f);

}
