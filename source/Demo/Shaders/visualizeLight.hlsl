struct VS_Input
{
    float3 pos : POSITION;
};

struct PS_Input
{
    float4 pos : SV_Position;
};

struct passData
{
    float4x4 viewMatrix;
    float4x4 perspective;
};
struct Light
{
	float3 position;
};

ConstantBuffer<passData> PassData : register(b0);
StructuredBuffer<Light> LightData : register(t0);

PS_Input VSMain(VS_Input vertex)
{
    const float size=50;
    PS_Input ps;
    ps.pos = float4((vertex.pos * size) + LightData[0].position, 1.f);
    ps.pos = mul(PassData.viewMatrix, ps.pos);
    ps.pos = mul(PassData.perspective, ps.pos);
    return ps;
}

float4 PSMain(PS_Input pixel) : SV_Target
{
    return float4(1.f, 1.f, 1.f, 1.f);
}
