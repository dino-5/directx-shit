struct Vertex
{
    float3 pos    : POSITION;
    float3 normal : NORMAL;
    float4 tangent: TANGENT;
    float2 uv     : UV;
};

struct PS_Input
{
	float4 pos: SV_POSITION;
	float3 worldPos: POSITION;
    float3 normal : NORMAL;
    float4 tangent: TANGENT;
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

    float4 pos = float4(vertex.pos, 1.0f);
    pos = mul(passCB.viewMatrix, pos);
    pos = mul(passCB.perspective, pos);
    ret.pos = pos;
    ret.worldPos = vertex.pos;
    ret.normal = normalize(vertex.normal);
    ret.tangent = normalize(vertex.tangent);
    ret.uv = vertex.uv;

    return ret;
}

struct PS_Result
{
    float4 position : SV_Target0;
    float4 albedo: SV_Target1;
    float4 normal: SV_Target2;
};

PS_Result PS_Basic(PS_Input input): SV_Target
{
    uint objDataIndx = objectTable.materialIndex;

    // get material of an object
    StructuredBuffer<Material> materialArray = ResourceDescriptorHeap[passTable.materialArrayIndex];
    Material material = materialArray[objDataIndx];

    float4 res = 0;
    float3 normal = 0;

    float4 color = 1;
    if(material.colorTexture !=-1)
    {
        Texture2D colorTexture = ResourceDescriptorHeap[material.colorTexture];
        color = colorTexture.Sample(textureSampler, input.uv);
    }

    if(material.normalTexture ==-1)
    {
        normal = input.normal;
    }
    else
    {
        Texture2D normalTexture = ResourceDescriptorHeap[material.normalTexture];
        float3 n = normalTexture.Sample(textureSampler, input.uv);
        float3x3 NormalTransform = transpose(float3x3(input.tangent.xyz, cross(input.normal, input.tangent.xyz)*input.tangent.w, input.normal));
        normal = mul(NormalTransform, n);
    }
    normal = normalize(normal);

    PS_Result result;
    result.position = float4(input.worldPos, 0.f);
    result.albedo = color;
    result.normal = float4(normal, 0.f);
    return result;
}