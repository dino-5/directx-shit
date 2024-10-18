#pragma once

#include <vector>
#include <string>
#include <iostream>
#include <utility>
#include <memory>
#include "EngineGfx/Mesh.h"
#include "EngineGfx/Texture.h"
#include "EngineCommon/util/GeometryGenerator.h"
#include "EngineCommon/System/Filesystem.h"
#include "EngineCommon/math/Matrix.h"
#include "third_party/tiny_gltf_loader/tiny_gltf.h"

namespace engine::graphics
{
	class RenderContext;
	class Model
	{
	public:
		Model() = default;
		void Init(system::Filepath path, RenderContext& context);
		//Mesh GetMesh() { return m_mesh; }
		void DrawModel(ID3D12GraphicsCommandList* cmdList);

	private:
		void LoadTextures();
		void ProcessNode(uint index);
		void ProcessMesh(uint index);

	public:
		Geometry<Vertex> m_geometry;
		Mesh m_mesh;
		std::vector<Submesh> m_submeshes;

        ConstantBuffer m_constantBuffer;
		RenderContext* m_renderContext = nullptr;

		Buffer m_vertexBuffer;
		Buffer m_indexBuffer;
		std::vector<Texture> m_textures;

		system::Filepath m_directory;
        std::unique_ptr<tinygltf::Model> m_model;


	};
};
