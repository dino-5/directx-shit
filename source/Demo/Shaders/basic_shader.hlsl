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
    float3 uv     : UV;
};

struct PassInfo
{
    uint constantBufferIndex;
    uint textureBufferIndex;
};

ConstantBuffer<PassInfo> cbIndex : register(b0, space10);

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
    ConstantBuffer<General> buffer = ResourceDescriptorHeap[cbIndex.constantBufferIndex];

    float4 pos = float4(vertex.pos, 1.0f);;
    pos = mul(buffer.viewMatrix, pos);
    pos = mul(buffer.perspective, pos);
    ret.pos = pos;
    ret.uv = vertex.uv;

    return ret;
}

float4 PS_Basic(PS_Input input): SV_Target
{
    return texture.Sample(MeshTextureSampler, input.uv);
}