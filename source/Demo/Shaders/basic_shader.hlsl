struct Vertex
{
    float3 pos    : POSITION;
    float3 normal : NORMAL;
    float2 uv     : UV;
};

struct PS_Input
{
	float4 pos: SV_POSITION;
    float3 normal : NORMAL;
    float2 uv : UV;
};

// ==============================
// Pass structures 
struct General 
{
    float4x4 viewMatrix;
    float4x4 perspective;
};

// ==============================
// Object structures 
struct Material
{
    int colorTexture;
    int normalTexture;
};

// ==============================
// Structures for constant buffers
struct PassBindlessResourcess
{
    uint passCBIndex;
    uint materialArrayIndex;
};

struct ObjectBindlessResources
{
    uint materialIndex;
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
    ret.normal = vertex.normal;
    ret.uv = vertex.uv;

    return ret;
}

float4 PS_Basic(PS_Input input): SV_Target
{
    uint objDataIndx = objectTable.materialIndex;

    // get material of an object
    StructuredBuffer<Material> materialArray = ResourceDescriptorHeap[passTable.materialArrayIndex];
    Material material = materialArray[objDataIndx];

    float4 res = 0;
    float3 normal = 0;

    Texture2D colorTexture = ResourceDescriptorHeap[material.colorTexture];
    res = colorTexture.Sample(textureSampler, input.uv);

    if(material.normalTexture ==-1)
    {
        normal = input.normal;
    }
    else
    {
        Texture2D normalTexture = ResourceDescriptorHeap[material.normalTexture];
        normal = normalTexture.Sample(textureSampler, input.uv);
    }

    return res;
}