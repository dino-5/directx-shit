struct PS_Input
{
	float4 pos: SV_POSITION;
	float3 color : COLOR;
};

cbuffer General : register(b0)
{
    float4x4 ViewProjection;
};

cbuffer Object : register(b1)
{
    float4x4 World;
};


PS_Input VS_Basic(float3 position: POSITION)
{
	PS_Input ret;
    float4 pos = float4(position, 1.0f);
    ret.pos = pos;
    ret.color = normalize(position.xyz);
    return ret;
}

float4 PS_Basic(PS_Input input): SV_Target
{
    return float4(input.color, 1.0f);

}