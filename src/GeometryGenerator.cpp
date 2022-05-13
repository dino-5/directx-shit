#include "include/GeometryGenerator.h"
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

GeometryGenerator::MeshData GeometryGenerator::CreateBox(float width, float height, float depth, uint32 numSubdivisions)
{
    MeshData mesh;

    const int n  = 8;
    std::vector<XMFLOAT3> vertices =
    {
         XMFLOAT3(-1.0f, -1.0f, -1.0f), // 0
		 XMFLOAT3(+1.0f, -1.0f, -1.0f), // 1
		 XMFLOAT3(+1.0f, -1.0f, +1.0f), // 2 
		 XMFLOAT3(-1.0f, -1.0f, +1.0f), // 3
		 XMFLOAT3(-1.0f, +1.0f, -1.0f), // 4
		 XMFLOAT3(+1.0f, +1.0f, -1.0f), // 5
		 XMFLOAT3(+1.0f, +1.0f, +1.0f), // 6
		 XMFLOAT3(-1.0f, +1.0f, +1.0f), // 7
    };
    XMFLOAT3 tan{ 0,0,0 };
    XMFLOAT2 uv{ 0,0 };

    for (int i = 0; i < 3; i++)
    {
        XMVECTOR vec1 = XMLoadFloat3(&vertices[i]);
        XMVECTOR vec2 = XMLoadFloat3(&vertices[i+4]);
        XMVECTOR v1 = XMVectorSubtract(vec1, vec2);
        vec1 = XMLoadFloat3(&vertices[i]);
        vec2 = XMLoadFloat3(&vertices[i+1]);
        XMVECTOR v2 = XMVectorSubtract(vec1, vec2);
        XMVECTOR norm = XMVector3Cross(v2, v1);
        XMFLOAT3 Norm;
        XMStoreFloat3(&Norm, XMVector3Normalize(norm));

        mesh.Vertices.push_back(Vertex( vertices[i],   Norm, tan, uv));
        mesh.Vertices.push_back(Vertex( vertices[i+4], Norm, tan, uv));
        mesh.Vertices.push_back(Vertex( vertices[i+5], Norm, tan, uv));
        mesh.Vertices.push_back(Vertex( vertices[i+1], Norm, tan, uv));

        mesh.Indices32.push_back(i*4);
        mesh.Indices32.push_back(i*4+1);
        mesh.Indices32.push_back(i*4+2);

        mesh.Indices32.push_back(i*4);
        mesh.Indices32.push_back(i*4+2);
        mesh.Indices32.push_back(i*4+3);
    }

	XMVECTOR vec1 = XMLoadFloat3(&vertices[3]);
	XMVECTOR vec2 = XMLoadFloat3(&vertices[7]);
	XMVECTOR v1 = XMVectorSubtract(vec1, vec2);
	vec1 = XMLoadFloat3(&vertices[3]);
	vec2 = XMLoadFloat3(&vertices[0]);
	XMVECTOR v2 = XMVectorSubtract(vec1, vec2);
	XMVECTOR norm = XMVector3Cross(v1, v2);
	XMFLOAT3 Norm;
	XMStoreFloat3(&Norm, XMVector3Normalize(norm));

	mesh.Vertices.push_back(Vertex( vertices[3], Norm, tan, uv));
	mesh.Vertices.push_back(Vertex( vertices[7], Norm, tan, uv));
	mesh.Vertices.push_back(Vertex( vertices[4], Norm, tan, uv));
	mesh.Vertices.push_back(Vertex( vertices[0], Norm, tan, uv));
	int i = 3;

	mesh.Indices32.push_back(i*4);
	mesh.Indices32.push_back(i*4+1);
	mesh.Indices32.push_back(i*4+2);

	mesh.Indices32.push_back(i*4);
	mesh.Indices32.push_back(i*4+2);
	mesh.Indices32.push_back(i*4+3);
    i++;

	for (int j = 0; j < 2; j++)
	{
        XMVECTOR vec1 = XMLoadFloat3(&vertices[j*4]);
        XMVECTOR vec2 = XMLoadFloat3(&vertices[j*4+3]);
        XMVECTOR v1 = XMVectorSubtract(vec1, vec2);
        vec1 = XMLoadFloat3(&vertices[j*4]);
        vec2 = XMLoadFloat3(&vertices[j*4+1]);
        XMVECTOR v2 = XMVectorSubtract(vec1, vec2);
        XMVECTOR norm = XMVector3Cross(v1, v2);
        XMFLOAT3 Norm;
        XMStoreFloat3(&Norm, XMVector3Normalize(norm));
        if (!j)
            Norm.y *= -1;

        mesh.Vertices.push_back(Vertex( vertices[j*4+0], Norm, tan, uv));
        mesh.Vertices.push_back(Vertex( vertices[j*4+3], Norm, tan, uv));
        mesh.Vertices.push_back(Vertex( vertices[j*4+2], Norm, tan, uv));
        mesh.Vertices.push_back(Vertex( vertices[j*4+1], Norm, tan, uv));

        mesh.Indices32.push_back(i*4);
        mesh.Indices32.push_back(i*4+1+(1-j)*2);
        mesh.Indices32.push_back(i * 4 + 2);// -(1 - j)); // strange calculations == culling of down face

        mesh.Indices32.push_back(i*4);
        mesh.Indices32.push_back(i*4+2);
        mesh.Indices32.push_back(i*4+3-(1-j)*2);
        i++;

	}

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

