#include "Shaders/common_light.hlsl"
struct DeferredTable
{
    int positionIndex;
    int albedoIndex;
    int normalIndex;
    int lightBufferIndex;
};

Texture2D getTextureByIndex(int index)
{
    return ResourceDescriptorHeap[index];
}

ConstantBuffer<DeferredTable> cbData : register(b0, space10);
ConstantBuffer<LightSettings> cbLightSettings : register(b1, space10);
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

float3 EvaluatePhong(float3 SurfaceColor, float3 SurfaceNormal, float3 SurfaceWorldPos,
                     float SurfaceShininess, float SurfaceSpecularStrength,
                     float3 LightDirection, float3 LightColor, float LightAmbientIntensity)
{
    float3 NegativeLightDir = -LightDirection;
    float AccumIntensity = LightAmbientIntensity;
    
    {
        float DiffuseIntensity = max(0, dot(SurfaceNormal, NegativeLightDir));
        AccumIntensity += DiffuseIntensity;
    }

    float SpecularIntensity = 0;
    {
        float3 CameraDirection = normalize(cbLightSettings.cameraPosition - SurfaceWorldPos);
        float3 HalfVector = normalize(NegativeLightDir + CameraDirection);
        SpecularIntensity = SurfaceSpecularStrength * pow(max(0, dot(HalfVector, SurfaceNormal)), SurfaceShininess);
    }
    
    float3 MixedColor = LightColor * SurfaceColor;
    float3 Result = AccumIntensity * MixedColor + SpecularIntensity * LightColor;

    return Result;
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
    float3 lightVector = normalize(light.position.xyz - position.xyz);
    float diffuse = max(dot(lightVector, normal), 0);

    float3 reflectedLight = reflect(lightVector, normal);
    float3 viewDirection;
    viewDirection = normalize(cbLightSettings.cameraPosition - position.xyz);
    float specular = pow(max(dot(reflectedLight, viewDirection), 0), 32) * 0.5;

    float4 res = albedo * (0.2 + diffuse + specular);
    float3 phong= EvaluatePhong(albedo, normal, position.xyz, 100.0, 1.0, -lightVector, float3(1.f, 1.f, 1.f), 0.5);
    return float4(phong, 1.f);
    return res;
    return float4(normal.xyz, 1.0f);
    return albedo;
}
