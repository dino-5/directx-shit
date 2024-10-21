struct PS_Input
{
	float4 pos: SV_POSITION;
    float2 uv : UV;
};

struct General 
{
    float4x4 viewMatrix;
    float4x4 perspective;
};

struct Vertex
{
    float3 pos    : POSITION;
    float3 normal : NORMAL;
    float2 uv     : UV;
};

cbuffer cbIndex : register(b0, space0)
{
    General transform;
};

Texture2D texture : register(t0);

SamplerState MeshTextureSampler
{
    Filter = MIN_MAG_MIP_LINEAR;
    AddressU = Wrap;
    AddressV = Wrap;
};


PS_Input VS_Basic(Vertex vertex)
{
    PS_Input ret;

    float4 pos = float4(vertex.pos, 1.0f);;
    pos = mul(transform.viewMatrix, pos);
    pos = mul(transform.perspective, pos);
    ret.pos = pos;
    ret.uv = vertex.uv;

    return ret;
}

float4 PS_Basic(PS_Input input): SV_Target
{
    float4 res = texture.Sample(MeshTextureSampler, input.uv);
    return res;
}