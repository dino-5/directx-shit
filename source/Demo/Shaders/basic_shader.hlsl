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
    float3 pos;
};

struct PassInfo
{
    uint vertexBufferIndex;
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


PS_Input VS_Basic(uint index : SV_VertexID)
{
    PS_Input ret;
    StructuredBuffer<Vertex> vertexBuffer = ResourceDescriptorHeap[cbIndex.vertexBufferIndex];
    ConstantBuffer<General> buffer = ResourceDescriptorHeap[cbIndex.constantBufferIndex];

    Vertex vertex = vertexBuffer.Load(index);
    float4 pos = mul(buffer.perspective,
    mul(buffer.viewMatrix, float4(vertex.pos, 1.0f)));
    ret.pos = pos;

    return ret;
}

float4 PS_Basic(PS_Input input): SV_Target
{
    return float4(1.f, 0.0f, 0.f, 1.0f);
    Texture2D tex = ResourceDescriptorHeap[cbIndex.textureBufferIndex];
    return float4(1.0f, 0.0f, 0.f, 1.f);

}