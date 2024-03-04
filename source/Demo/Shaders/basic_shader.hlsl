struct PS_Input
{
	float4 pos: SV_POSITION;
    float2 uv : UV;
};

struct General 
{
    float4x4 perspective;
    float color;
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
    StructuredBuffer<float3> vertexBuffer = ResourceDescriptorHeap[cbIndex.vertexBufferIndex];
    ConstantBuffer<General> buffer = ResourceDescriptorHeap[cbIndex.constantBufferIndex];
    float4 pos = mul(float4(vertexBuffer[index], 1.0f),buffer.perspective );
    ret.pos = pos;
    ret.uv = pos.xy;
    return ret;
}

float4 PS_Basic(PS_Input input): SV_Target
{
    //return float4(1.f, 0.0f, 0.f, 1.0f);
    Texture2D tex = ResourceDescriptorHeap[cbIndex.textureBufferIndex];
    return tex.Sample(MeshTextureSampler, input.uv);

}