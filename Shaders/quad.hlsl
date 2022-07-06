
struct VertexIn
{
    float3 PosL  : POSITION;
    float3 Norm  : NORMAL;
    float2 Tex   : TEXTURE;
};

struct VertexOut
{
    float4 PosH   : SV_POSITION;
    float2 Tex    : Texture;
};

VertexOut VS(VertexIn vin)
{
    VertexOut vout;
    vout.pos = float4(vin.PosL, 1.0f);
    vout.Tex = vin.Tex;
    return vout;
}

Texture2D shadow_texture  : register(t0);
SamplerState g_sampler : register(s0);

float4 PS(VertexOut input) : SV_Target
{
    float depthValue = shadow_texture.Sample(g_sampler, input.Tex).r;
	return float4(float4(depthValue), 1.0f);

}

