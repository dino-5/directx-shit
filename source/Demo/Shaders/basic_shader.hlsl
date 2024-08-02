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
    float3 pos : POSITION;
};

struct PassInfo
{
    uint constantBufferIndex;
    uint textureBufferIndex;
};

ConstantBuffer<PassInfo> cbIndex : register(b0, space10);

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

    return ret;
}

float4 PS_Basic(PS_Input input): SV_Target
{
    return float4(input.pos);
    Texture2D tex = ResourceDescriptorHeap[cbIndex.textureBufferIndex];
    return float4(1.0f, 0.0f, 0.f, 1.f);

}