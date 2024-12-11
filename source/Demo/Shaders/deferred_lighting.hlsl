struct DeferredTable
{
    int positionIndex;
    int albedoIndex;
    int normalIndex;
    int lightBufferIndex;
};

struct Light
{
	float3 position;
};

Texture2D getTextureByIndex(int index)
{
    return ResourceDescriptorHeap[index];
}

ConstantBuffer<DeferredTable> cbData : register(b0, space10);
sampler textureSampler;

struct PS_Input
{
    float4 pos : SV_Position;
    float2 uv : UV;
};

PS_Input VSMain(uint id : SV_VertexID)
{
    const float2 vertexPos[] =
    {
        float2(-1.0f, -1.0f),
        float2(-1.0f,  1.0f),
        float2(1.0f, 1.0f),
        float2(-1.0f, -1.0f),
        float2(1.0f, 1.0f),
        float2(1.0f, -1.0f),
    };
    PS_Input result;
    result.pos = float4(vertexPos[id], 0.f, 1.f);
    result.uv = vertexPos[id];
    return result;
}

float4 PSMain(PS_Input input) : SV_Target
{
    float2 uv = (input.uv * 0.5 + 0.5);
    uv.y = 1 - uv.y;
    Texture2D positionT = getTextureByIndex(cbData.positionIndex);
    Texture2D albedoT = getTextureByIndex(cbData.albedoIndex);
    Texture2D normalT = getTextureByIndex(cbData.normalIndex);

    float4 position = positionT.Sample(textureSampler, uv);
    float4 albedo = albedoT.Sample(textureSampler, uv);
    float3 normal = normalT.Sample(textureSampler, uv).xyz;

    StructuredBuffer<Light> lightArray = ResourceDescriptorHeap[cbData.lightBufferIndex];

    Light light = lightArray[0];
    float angle = max(dot(normalize(light.position.xyz - position.xyz), normal), 0);
    float4 res = albedo* (0.2 + angle);
    return res;
    return float4(normal.xyz, 1.0f);
    return albedo;
}