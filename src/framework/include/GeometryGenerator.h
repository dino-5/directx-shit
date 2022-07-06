#pragma once

#include <cstdint>
#include <DirectXMath.h>
#include <vector>
#include "MathHelper.h"
#include <array>
#include "Texture.h"

class Geometry
{
public:

	using uint16 = std::uint16_t;
	using uint32 = std::uint32_t;

	struct Vertex
	{
		Vertex() {}
		Vertex(
			const DirectX::XMFLOAT3& p,
			const DirectX::XMFLOAT3& n,
			const DirectX::XMFLOAT2& uv) :
			Position(p),
			Normal(n),
			Tex(uv) {}
		Vertex(
			float px, float py, float pz,
			float nx, float ny, float nz,
			float u, float v) :
			Position(px, py, pz),
			Normal(nx, ny, nz),
			Tex(u, v) {}

		DirectX::XMFLOAT3 Position;
		DirectX::XMFLOAT3 Normal;
		DirectX::XMFLOAT2 Tex;
	};

	struct MeshData
	{
		MeshData(std::vector<Vertex> vertices, std::vector<unsigned int> indices) :
			Vertices(vertices), Indices32(indices)
		{}
		MeshData() = default;
		std::vector<Vertex> Vertices;
		std::vector<uint32> Indices32;

		std::vector<uint16>& GetIndices16()
		{
			if (mIndices16.empty())
			{
				mIndices16.resize(Indices32.size());
				for (size_t i = 0; i < Indices32.size(); ++i)
					mIndices16[i] = static_cast<uint16>(Indices32[i]);
			}

			return mIndices16;
		}

	private:
		std::vector<uint16> mIndices16;
	};

	static MeshData CreateBox(float width=0, float height=0, float depth=0, uint32 numSubdivisions=0);
	MeshData CreateSphere(float radius, uint32 sliceCount, uint32 stackCount);
	MeshData CreateGeosphere(float radius, uint32 numSubdivisions);
	MeshData CreateCylinder(float bottomRadius, float topRadius, float height, uint32 sliceCount, uint32 stackCount);
	MeshData CreateGrid(float width, float depth, uint32 m, uint32 n);
	static MeshData CreateQuad();

private:
	void Subdivide(MeshData& meshData);
	Vertex MidPoint(const Vertex& v0, const Vertex& v1);
	void BuildCylinderTopCap(float bottomRadius, float topRadius, float height, uint32 sliceCount, uint32 stackCount, MeshData& meshData);
	void BuildCylinderBottomCap(float bottomRadius, float topRadius, float height, uint32 sliceCount, uint32 stackCount, MeshData& meshData);
};


