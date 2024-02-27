struct PS_Input
{
	float4 pos: SV_POSITION;
	float color : COLOR;
};

struct General 
{
    float4x4 perspective;
    float color;
};

struct PassInfo
{
    uint constantBufferIndex;
};

ConstantBuffer<PassInfo> cbIndex : register(b0, space10);


PS_Input VS_Basic(float3 position: POSITION)
{
	PS_Input ret;
    ConstantBuffer<General> buffer = ResourceDescriptorHeap[cbIndex.constantBufferIndex];
    float4 pos = mul(float4(position, 1.0f),buffer.perspective );
    ret.pos = pos;
    ret.color = buffer.color;
    return ret;
}

float4 PS_Basic(PS_Input input): SV_Target
{
    //return float4(1.f, 0.0f, 0.f, 1.0f);
    return float4(input.color, 0.0f, input.color, 1.0f);
}