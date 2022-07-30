
struct VertexOut
{
	float4 PosH   : SV_POSITION;
    float4 pos    : Pos;
    float4 shadowPos    : ShadowPos;
    float4 normal : Norm;
    float2 Tex    : Texture;
};

float4 PS(VertexOut input) : SV_Target
{
	return (0.0f, 0.0f, 0.0f, 0.0f);
}

