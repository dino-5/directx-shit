struct Vertex
{
    float3 position : POSITION;
};

struct PS_Input
{
    float4 position : SV_POSITION;
};

struct ViewSettings
{
    float4x4 cameraMatrix;
    float4x4 projection;
};

// indices to resources which are constant along the draw call
struct PassIndices
{
    int viewSettingsIndex;
};

ConstantBuffer<PassIndices> g_constantIndices : register(b0, space10);

PS_Input VertexMain(Vertex vertex)
{
    ConstantBuffer<ViewSettings> view = ResourceDescriptorHeap[g_constantIndices.viewSettingsIndex];
    PS_Input result;
    result.position = mul(view.projection, mul(view.cameraMatrix, float4(vertex.position, 1.f)));
    return result;
}

float4 PixelMain(PS_Input input) : SV_Target
{
    return float4(1.f, 0.f, 0.f, 1.f);
}
