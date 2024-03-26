#pragma once

#include <cstdint>
#include <DirectXMath.h>
#include <vector>
#include "MathHelper.h"
#include <array>
#include "EngineGfx/Texture.h"

namespace engine::util
{
	class Geometry
	{
	public:

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
			std::vector<u32> Indices32;

			std::vector<u16>& GetIndices16()
			{
				if (mIndices16.empty())
				{
					mIndices16.resize(Indices32.size());
					for (size_t i = 0; i < Indices32.size(); ++i)
						mIndices16[i] = static_cast<u16>(Indices32[i]);
				}

				return mIndices16;
			}

		private:
			std::vector<u16> mIndices16;
		};

		static MeshData CreateBox(float width = 0, float height = 0, float depth = 0, u32 numSubdivisions = 0);
		MeshData CreateSphere(float radius, u32 sliceCount, u32 stackCount);
		MeshData CreateGeosphere(float radius, u32 numSubdivisions);
		MeshData CreateCylinder(float bottomRadius, float topRadius, float height, u32 sliceCount, u32 stackCount);
		MeshData CreateGrid(float width, float depth, u32 m, u32 n);
		static MeshData CreateQuad();
		static MeshData CreateMirror();

	private:
		void Subdivide(MeshData& meshData);
		Vertex MidPoint(const Vertex& v0, const Vertex& v1);
		void BuildCylinderTopCap(float bottomRadius, float topRadius, float height, u32 sliceCount, u32 stackCount, MeshData& meshData);
		void BuildCylinderBottomCap(float bottomRadius, float topRadius, float height, u32 sliceCount, u32 stackCount, MeshData& meshData);
	};

};
