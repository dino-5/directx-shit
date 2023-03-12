struct PS_Input
{
	float4 pos: SV_POSITION;
	float3 color : COLOR;
};

PS_Input VS_Basic(float3 position: POSITION)
{
	PS_Input ret;
    ret.pos = float4(position, 1.0f);
    ret.color = normalize(position.xyz);
    return ret;
}

float4 PS_Basic(PS_Input input): SV_Target
{
    return float4(input.color, 1.0f);

}