//***************************************************************************************
// color.hlsl by Frank Luna (C) 2015 All Rights Reserved.
//
// Transforms and colors geometry.
//***************************************************************************************

cbuffer cbPerObject : register(b0)
{
	float4x4 gWorld; 
};

cbuffer cbPerPass: register(b1)
{
	float4x4 gView;
	float4x4 gInvView;
	float4x4 gProj;
	float4x4 gInvProj;
	float4x4 gViewProj;
	float4x4 gInvViewProj;

	float3 gEyePosW;
    float3 lightPos;
    float3 lightColor;
    float3 boxColor;

};

struct VertexIn
{
	float3 PosL  : POSITION;
    float3 Norm  : NORMAL;
    float2 Tex   : TEXTURE;
};

struct VertexOut
{
	float4 PosH   : SV_POSITION;
    float3 pos    : Pos;
    float3 normal : Norm;
    float2 Tex    : Texture;
};

VertexOut VS(VertexIn vin)
{
	VertexOut vout;

	float4 posW = mul(float4(vin.PosL, 1.0f), gWorld);
	// Transform to homogeneous clip space.
	vout.PosH = mul(posW, gViewProj);

    vout.pos = vin.PosL;
    vout.normal = vin.Norm;
    vout.Tex = vin.Tex;

	return vout;
}

Texture2D g_texture : register(t0);
SamplerState g_sampler : register(s0);

float4 PS(VertexOut pin) : SV_Target
{
    ////float3 boxColor = float3(1.0f, 0.5, 0.0);
    //float4 fragcolor;
    //float ambientStrength = 0.2f;
    //float3 ambient = ambientStrength * lightColor;

    //float3 lightDir = normalize(lightPos - pin.pos);
    //float diff = max(dot(pin.normal, lightDir), 0);
    //float3 diffuse = diff * lightColor;

    //float3 result = (ambient +diffuse) * boxColor;

	return g_texture.Sample(g_sampler, pin.Tex);
}


