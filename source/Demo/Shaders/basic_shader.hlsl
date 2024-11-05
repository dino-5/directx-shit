struct Vertex
{
    float3 pos    : POSITION;
    float3 normal : NORMAL;
    float2 uv     : UV;
};

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

struct PassBindlessResourcess
{
    uint passCBIndex;
};

struct ObjectBindlessResources
{
    uint textureIndex;
};

ConstantBuffer<PassBindlessResourcess> passTable : register(b0, space10);
ConstantBuffer<ObjectBindlessResources> objectTable : register(b1, space10);

sampler textureSampler;

PS_Input VS_Basic(Vertex vertex)
{
    PS_Input ret;

    ConstantBuffer<General> passCB = ResourceDescriptorHeap[passTable.passCBIndex];

    float4 pos = float4(vertex.pos, 1.0f);;
    pos = mul(passCB.viewMatrix, pos);
    pos = mul(passCB.perspective, pos);
    ret.pos = pos;
    ret.uv = vertex.uv;

    return ret;
}

float4 PS_Basic(PS_Input input): SV_Target
{
    Texture2D texture = ResourceDescriptorHeap[objectTable.textureIndex];
    float4 res = texture.Sample(textureSampler, input.uv);
    return res;
}