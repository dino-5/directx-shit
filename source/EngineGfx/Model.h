#pragma once

#include <vector>
#include <string>
#include <iostream>
#include <utility>
#include "EngineGfx/Mesh.h"
#include "EngineGfx/Texture.h"
#include "EngineCommon/util/GeometryGenerator.h"
#include "EngineCommon/System/Filesystem.h"

namespace engine::graphics
{
	class Model
	{
	public:
		Model() = default;
		void Init(system::Filepath path, ComPtr<ID3D12GraphicsCommandList> cmList);
		Mesh GetMesh() { return m_mesh; }
		void DrawModel(ID3D12GraphicsCommandList* cmdList);

	public:
		Mesh m_mesh;
		std::string m_directory;

	private:

	};
};
