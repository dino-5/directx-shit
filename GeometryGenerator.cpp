#include "GeometryGenerator.h"
using namespace DirectX;

GeometryGenerator::MeshData GeometryGenerator::CreateCylinder
(
    float bottomRadius, float topRadius, float height, uint32 sliceCount, uint32 stackCount)
{
    MeshData mesh;
    float stackHeight = height / stackCount;
    float radiusStep = (bottomRadius - topRadius) / stackCount;

	float c_dr = bottomRadius - topRadius;
    uint32 ringCount = stackCount + 1;
    for (uint32 i = 0; i < ringCount; i++)
    {
        float y = -0.5f * height + i * stackHeight;
        float r = bottomRadius + i * radiusStep;
        float dTheta = 2.0f * XM_PI / sliceCount;
        for (uint32 j = 0; j < sliceCount; ++j)
        {
            Vertex vertex;
            float c = cosf(j * dTheta);
            float s = sinf(j * dTheta);
            vertex.Position = XMFLOAT3(r * c, y, r * s);
            vertex.TexC.x = (float)j / sliceCount;
            vertex.TexC.y = 1.0f - (float)i / stackCount;
            vertex.TangentU = XMFLOAT3(-s, 0.0f, c);
            XMFLOAT3 bitangent(c_dr * c, -height, c_dr * s);
            XMVECTOR T = XMLoadFloat3(&vertex.TangentU);
            XMVECTOR B = XMLoadFloat3(&bitangent);
            XMVECTOR N = XMVector3Normalize(XMVector3Cross(T, B));
            XMStoreFloat3(&vertex.Normal, N);
            mesh.Vertices.push_back(vertex);
        }
    }
    auto vertexCount = sliceCount + 1;
    for (uint32 i = 0; i < stackCount; i++)
    {
        for (uint32 j = 0; j < sliceCount; j++)
        {
            mesh.Indices32.push_back(i*vertexCount+j);
            mesh.Indices32.push_back((i+1)*vertexCount +j);
            mesh.Indices32.push_back((i+1)*vertexCount +j+1);
            mesh.Indices32.push_back(i*vertexCount+j);
            mesh.Indices32.push_back((i+1)*vertexCount +j+1);
            mesh.Indices32.push_back(i*vertexCount+j+1);
        }
    }
    BuildCylinderBottomCap(bottomRadius, topRadius, height, sliceCount, stackCount, mesh);
    BuildCylinderTopCap(bottomRadius, topRadius, height, sliceCount, stackCount, mesh);

    return mesh;
}

void GeometryGenerator::BuildCylinderTopCap(float bottomRadius, float topRadius, float height, uint32 sliceCount, uint32 stackCount, MeshData& meshData)
{
    uint32 baseIndex = (uint32)meshData.Vertices.size();
    float y = 0.5f * height;
    float dTheta = 2.0f * XM_PI / sliceCount;
    for (uint32 i = 0; i <= sliceCount; i++)
    {
        float x = topRadius * cosf(dTheta * i);
        float z = topRadius * sinf(dTheta * i);

        float u = x / height + 0.5f;
        float v = z / height + 0.5f;
        meshData.Vertices.push_back(
            Vertex(x, y, z, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, u, v)
        );
    }
    
    meshData.Vertices.push_back(
        Vertex(0, y, 0, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.5f, 0.5f)
    );

    uint32 centerIndex = meshData.Vertices.size() - 1;
    for (uint32 i = 0; i < sliceCount; i++)
    {
        meshData.Indices32.push_back(centerIndex);
        meshData.Indices32.push_back(baseIndex+i+1);
        meshData.Indices32.push_back(baseIndex+i);
    }

}

void GeometryGenerator::BuildCylinderBottomCap(float bottomRadius, float topRadius, float height, uint32 sliceCount, uint32 stackCount, MeshData& meshData)
{

    uint32 baseIndex = (uint32)meshData.Vertices.size();
    float y = -0.5f * height;
    float dTheta = 2.0f * XM_PI / sliceCount;
    for (uint32 i = 0; i <= sliceCount; i++)
    {
        float x = bottomRadius* cosf(dTheta * i);
        float z = bottomRadius* sinf(dTheta * i);

        float u = x / height + 0.5f;
        float v = z / height + 0.5f;
        meshData.Vertices.push_back(
            Vertex(x, y, z, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f, 0.0f, u, v)
        );
    }
    
    meshData.Vertices.push_back(
        Vertex(0, y, 0, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.5f, 0.5f)
    );

    uint32 centerIndex = meshData.Vertices.size() - 1;
    for (uint32 i = 0; i < sliceCount; i++)
    {
        meshData.Indices32.push_back(centerIndex);
        meshData.Indices32.push_back(baseIndex+i+1);
        meshData.Indices32.push_back(baseIndex+i);
    }
}

GeometryGenerator::MeshData GeometryGenerator::CreateBox(float width, float height, float depth, uint32_t numSubdivisions)
{
    return MeshData();
}
