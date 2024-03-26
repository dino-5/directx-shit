#include "EngineGfx/Model.h"
#include "EngineGfx/Mesh.h"
#include "EngineGfx/Texture.h"
#include "EngineGfx/dx12/Device.h"
#include "EngineCommon/util/Logger.h"
#include "EngineCommon//math/Vector.h"
#include "third_party/tiny_gltf_loader/tiny_gltf.h"


namespace engine::graphics
{
    void Model::Init(system::Filepath path, graphics::RenderContext& context)
    {
        tinygltf::Model model;
        tinygltf::TinyGLTF gltfContext;
        std::string err;
        std::string warn;
        bool ret = gltfContext.LoadASCIIFromFile(&model, &err, &warn, path.str());
        if (!warn.empty()) {
            util::PrintInfo("Warn: {}\n", warn.c_str());
        }

        if (!err.empty()) {
            util::PrintError("Err: {}\n", err.c_str());
        }

        if (!ret) {
            util::PrintError("Failed to parse glTF\n");
        }

        //Accessor
        //Buffer
        //BufferView
        //Material
        //Mesh
        //Node
        //Texture
        //Image
        //Skin
        //Sampler
        //Camera
        //Scene
        //Light

        ID3D12Device* device = context.GetDevice().native();
        graphics::CommandList& commandList = context.GetList();
        std::vector<graphics::TextureHandle> textureHandles(model.textures.size());
        for (u32 index =0; index < model.textures.size() ; index++)
        {
            auto& texture = model.textures[index];
            tinygltf::Image image = model.images[texture.source];
            ImageData data;
            data.SetData(image.image.data());
            data.width = image.width;
            data.height = image.height;
            data.channels = image.component;
            data.name = image.uri.c_str();
            textureHandles[index] = Texture::CreateTexture(data, device, commandList.GetList());
        }
        struct Vertex
        {
            math::Vector3 pos;
            math::Vector2 uv;
            math::Vector3 normal;
        };


        auto getBuffer = [&model](tinygltf::Accessor& accessor) -> const float*
        {
            auto& bufferView = model.bufferViews[accessor.bufferView];
            return reinterpret_cast<const float*>(
                &model.buffers[bufferView.buffer].data[bufferView.byteOffset + accessor.byteOffset]);
        };

        auto getBufferView = [&model](tinygltf::Accessor& accessor) -> tinygltf::BufferView& {
            return model.bufferViews[accessor.bufferView];
        };

        auto getBufferStride = [&model](tinygltf::Accessor& accessor, tinygltf::BufferView& view) -> u32 {
            return accessor.ByteStride(view) ? accessor.ByteStride(view) / sizeof(float) : tinygltf::GetNumComponentsInType(accessor.type);
        };

        struct AccessorParsed
        {
            const float* buffer;
            tinygltf::BufferView view;
            u32 byteStride;
        };

        auto getAccessorParsed = [&model, &getBuffer, &getBufferView, &getBufferStride]
        (tinygltf::Accessor& accesor)->AccessorParsed {
            AccessorParsed ret;
            ret.buffer = getBuffer(accesor);
            ret.view = getBufferView(accesor);
            ret.byteStride = getBufferStride(accesor, ret.view);
            return ret;
        };

        for (tinygltf::Mesh& mesh : model.meshes)
        {
            for (tinygltf::Primitive& primitive : mesh.primitives)
            {
                tinygltf::Accessor& pos= model.accessors[primitive.attributes["POSITION"]];
                tinygltf::Accessor& uv = model.accessors[primitive.attributes["TEXCOORD_0"]];
                tinygltf::Accessor& norm    = model.accessors[primitive.attributes["NORMAL"]];

                auto& positionBufferView = getBufferView(pos);
                auto& texCoordBufferView = getBufferView(uv);
                auto& normalBufferView = getBufferView(norm);

                const float* positionBuffer = getBuffer(pos);
                const float* texCoordBuffer = getBuffer(uv);
                const float* normalBuffer   = getBuffer(norm);

                auto position = getAccessorParsed(pos);
                auto texCoord = getAccessorParsed(uv);
                auto normal   = getAccessorParsed(norm);

                std::vector<Vertex> vertexData(pos.count);
                std::vector<u16> indexData;

                for (i32 index = 0; index < pos.count; index++)
                {
                    Vertex& vertex = vertexData[index];
                    u32 posOffset = position.byteStride * index;
                    u32 normalOffset = normal.byteStride * index;
                    u32 uvOffset = texCoord.byteStride * index;
                    vertex.pos    = {position.buffer[0 + posOffset], position.buffer[1 + posOffset], position.buffer[2+posOffset]};
                    vertex.normal = { normal.buffer[0 + normalOffset], normal.buffer[1 + normalOffset], normal.buffer[2 + normalOffset] };
                    vertex.uv     = { texCoord.buffer[0 + uvOffset], texCoord.buffer[1 + uvOffset], texCoord.buffer[2 + uvOffset] };
                }
                if (primitive.indices != -1)
                {
                    auto& indicesAccessor = model.accessors[primitive.indices];
                    auto indices = getAccessorParsed(indicesAccessor);
                    const u16* buffer = reinterpret_cast<const u16*>(indices.buffer);
                    indexData.resize(indicesAccessor.count);
                    for (u32 index = 0; index < indicesAccessor.count; index++)
                    {
                        indexData[index] = buffer[index];
                    }

                }
                auto& material = primitive.material != -1 ? model.materials[primitive.material] : model.materials[0];
                u32 textureIndex = material.occlusionTexture.index != -1 ? material.occlusionTexture.index : 0;
                TextureHandle occlusion = textureHandles[textureIndex];
                m_mesh.push_back(graphics::Mesh(
                    context, reinterpret_cast<void*>(vertexData.data()), pos.count*sizeof(Vertex), sizeof(Vertex),
                    reinterpret_cast<void*>(indexData.data()), indexData.size(),
                    Material{ occlusion}));
            }
        }
    }

};

