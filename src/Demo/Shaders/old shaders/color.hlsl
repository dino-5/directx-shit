#include "lighting.hlsl"
cbuffer cbPerObject : register(b0)
{
	float4x4 g_world; 
};

cbuffer cbPerPass: register(b1)
{
	float4x4 gView;
	float4x4 gProj;
	float4x4 shadowView;

	float4 gEyePosW;

};

cbuffer cbLight : register(b2)
{
    PointLight light;
};


struct VertexIn
{
	float3 pos   : POSITION;
    float3 Norm  : NORMAL;
    float2 Tex   : TEXTURE;
};

struct VertexOut
{
	float4 ScreenPos    : SV_POSITION;
    float4 Pos          : Pos;
    float4 normal       : Norm;
    float2 Tex          : Texture;
};

VertexOut VS(VertexIn vin)
{
	VertexOut vout;

	float4 pos = mul(float4(vin.pos, 1.0f), g_world);
    vout.Pos = pos;
	pos = mul(pos, gView);
	vout.ScreenPos = mul(pos, gProj);
    vout.normal = mul(float4(vin.Norm, 1.0f), g_world);
    vout.Tex = vin.Tex;

	return vout;
}

struct SkyVertexOut
{
	float4 ScreenPos    : SV_POSITION;
    float4 Pos          : Pos;
    float3 Tex          : Texture;
};

SkyVertexOut SkyBoxVS(VertexIn vertex)
{
    SkyVertexOut vertexOut;
    float4 pos=  mul(float4(vertex.pos, 1.0f), g_world);
    pos = float4(pos.xyz+gEyePosW.xyz, 1.0f);
    pos = mul(pos, gView);
    
    vertexOut.Pos = pos;
	vertexOut.ScreenPos = mul(pos, gProj).xyww;
    vertexOut.Tex = vertex.pos;

    return vertexOut;
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
    //Material material = { pixelColor, specularPixelColor, materialShininess};
    float ambientStrength = 0.2f;
    float4 ambient = ambientStrength * light.Color * pixelColor;

    float diffuseStrength = 0.8f;

    float4 normal = normalize(input.normal);
    float4 lightDir = normalize(light.Pos - input.Pos);
    float4 diffuse = diffuseStrength * light.Color * max(dot(lightDir, normal),0) * pixelColor;

    float specularStrength = 0.8;
    float4 eyeVector = normalize(gEyePosW - input.Pos);
    float4 reflectedLight = normalize(reflect(lightDir, normal));
    float4 specular = pow(max(dot(eyeVector, reflectedLight), 0), 4) * specularPixelColor * specularStrength;

    float4 result = diffuse+ ambient + specular;

    return result;
    return diffuse;
    return normal;
    return specular;
    return ambient;
    return pixelColor;
    return specularPixelColor;
}

TextureCube g_cubeMap : register(t2);

float4 SkyBoxPS(SkyVertexOut input) : SV_Target
{
    return g_cubeMap.Sample(g_sampler, input.Tex);
}

float4 WhiteBox(VertexOut input) : SV_Target
{
    return float4(1.0f, 1.0f, 1.0f, 1.0f);
}


