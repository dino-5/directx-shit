struct Vertex
{
    float3 pos    : POSITION;
    float3 normal : NORMAL;
    float4 tangent: TANGENT;
    float2 uv     : UV;
};

struct PS_Input
{
    float4 pos : SV_POSITION;
    float3 normal : NORMAL;
    float3 tangent: TANGENT;
    float3 bitangent: BITANGENT;
    float2 uv     : UV;
};

struct ViewSettings
{
    float4x4 cameraMatrix;
    float4x4 projection;
};

struct Material
{
    int colorTextureIndex;
    int normalTextureIndex;
};

// indices to resources which are constant along the draw call
struct PassIndices
{
    int materialArrayIndex;
};

// indices for data which may be different for every object
struct ObjectIndices
{
    int materialIndex;
};

ConstantBuffer<ViewSettings> g_view : register(b0);
ConstantBuffer<PassIndices> g_passIndices : register (b0, space10);
ConstantBuffer<ObjectIndices> g_objectIndices : register (b1, space10);

sampler texture_sampler : register(s0);

PS_Input VertexMain(Vertex vertex)
{
    PS_Input result;
    result.pos = mul(g_view.projection,
                     mul(g_view.cameraMatrix, float4(vertex.pos, 1.f) ));
    result.normal = vertex.normal;
    result.tangent = vertex.tangent.xyz;
    result.bitangent = cross(vertex.normal, vertex.tangent.xyz)*vertex.tangent.w;
    result.uv = vertex.uv;

    return result;
}


float4 PixelMain(PS_Input input) : SV_Target 
{
    if(length(input.normal)>0)
        return float4(input.normal, 1.f);
    return float4(input.pos.xyz, 1.f);
    StructuredBuffer<Material> materialArray = ResourceDescriptorHeap[g_passIndices.materialArrayIndex];
    Material objectMaterial = materialArray[g_objectIndices.materialIndex];
    if(objectMaterial.colorTextureIndex != -1)
    {
        Texture2D texture = ResourceDescriptorHeap[objectMaterial.colorTextureIndex];
        return texture.Sample(texture_sampler, input.uv);
    }
}
