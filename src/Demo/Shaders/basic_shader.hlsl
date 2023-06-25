struct PS_Input
{
	float4 pos: SV_POSITION;
	float color : COLOR;
};

cbuffer General : register(b0)
{
    float color;
};


PS_Input VS_Basic(float3 position: POSITION)
{
	PS_Input ret;
    float4 pos = float4(position, 1.0f);
    ret.pos = pos;
    ret.color = color;
    return ret;
}

float4 PS_Basic(PS_Input input): SV_Target
{
    return float4(input.color, 0.0f, input.color, 1.0f);
}