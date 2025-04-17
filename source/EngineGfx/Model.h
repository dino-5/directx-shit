#pragma once

#include <vector>
#include <string>
#include <iostream>
#include <utility>
#include <memory>
#include <variant>
#include "EngineGfx/Mesh.h"
#include "EngineGfx/Texture.h"
#include "EngineCommon/System/Filesystem.h"
#include "EngineCommon/math/Matrix.h"
#include "third_party/tiny_gltf_loader/tiny_gltf.h"

namespace engine::graphics
{
    struct SubmeshData
    {
        i32 colorTextureIndex;
        i32 normalTextureIndex;
    };

    using GeometryGLTF = Geometry<Vertex>;
    using GeometryDSH = Geometry<math::Vector3>;

	class Model
	{
	public:
		Model() = default;
		void initGLTF(system::Filepath path, GfxContext& context);
        void initDSH(system::Filepath path, GfxContext& context);
        void initOBJ(system::Filepath path, GfxContext& context);
		void drawModel(ID3D12GraphicsCommandList* cmdList);
        void reset()
        {
            m_mesh.reset();
            m_materialBuffer.reset();
            for (auto& texture : m_textures)
                texture.reset();

        }

	private:
		void loadTextures();
		void processNode(uint index);
		void processMesh(uint index);

        struct AccessorData
        {
            tinygltf::Accessor accessor;
            tinygltf::BufferView view;
            tinygltf::Buffer buffer;

            int byteStride() { return accessor.ByteStride(view); }
            unsigned char* getData() { return buffer.data.data() + view.byteOffset + accessor.byteOffset; }
        };

        auto getBufferView(int index) -> auto& {
            return m_model->bufferViews[index];
        };

        auto getBuffer(int index) -> auto& {
            return m_model->buffers[index];
        };
        auto getAccessor(int index) -> auto {
            auto accessor = m_model->accessors[index];
            auto& bufferView = getBufferView(accessor.bufferView);
            auto& buffer = getBuffer(bufferView.buffer);
            return AccessorData{ accessor, bufferView, buffer };
        };

        auto checkAccessor (const AccessorData& obj, int componentType, int type) -> bool {
            return obj.view.byteStride &&
                obj.accessor.componentType == componentType &&
                obj.accessor.type == type;
        };


	public:
		GeometryGLTF m_geometry;
		Mesh m_mesh;
		std::vector<Submesh> m_submeshes;
		std::vector<Texture> m_textures;
        Buffer m_materialBuffer;
        Buffer m_objectBuffer;
        GfxContext* m_context = nullptr;

		system::Filepath m_directory;
        std::unique_ptr<tinygltf::Model> m_model;
	};
};
