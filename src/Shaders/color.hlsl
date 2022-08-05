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
	float4x4 shadowView;

	float4 gEyePosW;
    float4 lightPos;
    float4 lightDir;
    float4 lightColor;

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
    float4 pos    : Pos;
    float4 shadowPos    : ShadowPos;
    float4 normal : Norm;
    float2 Tex    : Texture;
};

VertexOut VS(VertexIn vin)
{
	VertexOut vout;

	float4 posW = mul(float4(vin.PosL, 1.0f), gWorld);
	vout.PosH = mul(posW, gViewProj);

    vout.pos = posW;
    vout.shadowPos = mul(posW, shadowView);
    vout.normal = float4(vin.Norm, 1.0f);
    vout.Tex = vin.Tex;

	return vout;
}

Texture2D diffuse_texture : register(t0);
Texture2D specular_texture: register(t1);

SamplerState g_sampler : register(s0);

float4 PS(VertexOut input) : SV_Target
{
    
    float4 white = float4(1.0f, 1.0f, 1.0f, 1.0f);
    float4 red   = float4(1.0f, .0f, .0f, 1.0f);

    float4 specularPixelColor = specular_texture.Sample(g_sampler, input.Tex);
    float4 pixelColor = diffuse_texture.Sample(g_sampler, input.Tex);
    float ambientStrength = 0.2f;
    float4 ambient = ambientStrength * lightColor * pixelColor;

    float diffuseStrength = 0.8f;

    float4 normal = normalize(input.normal);
    float4 l = normalize(-lightDir);
    float4 diffuse = diffuseStrength * lightColor * max(dot(l, normal),0) * pixelColor;

    float4 eyeVector = normalize(gEyePosW - input.pos);
    float4 reflectedLight = reflect(l, input.normal);
    float4 specular = pow(max(dot(eyeVector, reflectedLight), 0), 16) * specular_texture.Sample(g_sampler, input.Tex);


    float4 result = diffuse+ ambient + specular;
    //result = float4(1.0f, 0.0f, 0.0f, 1.0f) * shadow + ambient + specular * shadow;
    //if(result.a < 0.1)
    //    discard;

    return pixelColor;
    return result;
    return diffuse;
    return normal;
    return specularPixelColor;
}

