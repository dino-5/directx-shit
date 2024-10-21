#include "EngineGfx/Model.h"
#include "EngineGfx/Mesh.h"
#include "EngineGfx/Texture.h"
#include "EngineGfx/dx12/Device.h"
#include "EngineCommon/util/Logger.h"
#include "EngineCommon//math/Vector.h"
#include "EngineCommon//math/Matrix.h"

#include <span>

bool locLoadModel(tinygltf::Model* model, const std::string& path)
{
    tinygltf::TinyGLTF loader;
    std::string err, warn;

    bool ret = loader.LoadASCIIFromFile(model, &err, &warn, path);
    
    if (!warn.empty())
    {
        util::printInfo("during the model loading warning occured : {}", warn);
    }

    if (!err.empty())
    {
        util::printError("during the model loading error occured : {}", err);
    }
    return ret;
}


namespace engine::graphics
{
    void Model::init(system::Filepath path, graphics::RenderContext& context)
    {
        m_directory.init(path.getPath().remove_filename());
        m_renderContext = &context;
        m_model = std::make_unique<tinygltf::Model>();
        bool success= locLoadModel(m_model.get(), path.str());

        if (!success)
        {
            util::printError("failed to load model {}", path.str());
            return;
        }

        loadTextures();
        
        for (auto& scene : m_model->scenes)
        {
            for (auto& nodeIndex : scene.nodes)
            {
                if(nodeIndex!=-1)
                    processNode(nodeIndex);
            }
        }
        m_mesh.init(context, m_geometry);
    }

    void Model::loadTextures()
    {
        for (auto& texture : m_model->textures)
        {
            auto image = m_model->images[texture.source];
            if (!image.uri.length())
            {
                util::printError("no uri is provided for an image");
                return;
            }
            m_textures.push_back(Texture(ImageData(m_directory/image.uri),
                m_renderContext->getDevice().native(), m_renderContext->getList().getList()));
        }

    }

    void Model::processNode(uint index)
    {
        auto& node = m_model->nodes[index];
        if (node.children.size())
            for (auto& nodeIndex : node.children)
            {
                processNode(index);
            }
        if (node.mesh != -1)
            processMesh(node.mesh);

    }

    void Model::processMesh(uint index)
    {
        struct AccessorData
        {
            tinygltf::Accessor accessor;
            tinygltf::BufferView view;
            tinygltf::Buffer buffer;

            int byteStride() { return accessor.ByteStride(view); }
            unsigned char* getData() { return buffer.data.data() + view.byteOffset + accessor.byteOffset; }
        };

        auto getBufferView = [this](int index) -> auto& {
            return this->m_model->bufferViews[index];
        };

        auto getBuffer= [this](int index) -> auto& {
            return this->m_model->buffers[index];
        };
        auto getAccessor = [this, &getBufferView, &getBuffer](int index) -> auto {
            auto accessor = this->m_model->accessors[index];
            auto& bufferView = getBufferView(accessor.bufferView);
            auto& buffer = getBuffer(bufferView.buffer);
            return AccessorData{ accessor, bufferView, buffer };
        };

        auto checkAccessor = [](const AccessorData& obj, int componentType, int type) -> bool {
            return obj.view.byteStride &&
                obj.accessor.componentType == componentType &&
                obj.accessor.type == type;
        };

        auto& mesh = m_model->meshes[index];
        for (auto& primitive : mesh.primitives)
        {
            int positionIndex, texIndex, normalIndex;
            for (auto& [attrName, attrIndex] : primitive.attributes)
            {
                if (attrName == "POSITION")
                {
                    positionIndex = attrIndex;
                }
                else if (attrName == "NORMAL")
                {
                    normalIndex= attrIndex;
                }
                else if (attrName == "TEXCOORD_0")
                {
                    texIndex= attrIndex;
                }
            }

            auto position = getAccessor(positionIndex);
            auto normal = getAccessor(normalIndex);
            auto texture = getAccessor(texIndex);

            assert(checkAccessor(position, TINYGLTF_COMPONENT_TYPE_FLOAT, TINYGLTF_TYPE_VEC3));
            assert(checkAccessor(normal, TINYGLTF_COMPONENT_TYPE_FLOAT, TINYGLTF_TYPE_VEC3));
            assert(checkAccessor(texture, TINYGLTF_COMPONENT_TYPE_FLOAT, TINYGLTF_TYPE_VEC2));

            assert(position.accessor.count == normal.accessor.count && normal.accessor.count == texture.accessor.count);
            u32 count = position.accessor.count;
            auto getSpan3 = [count](AccessorData& accessor) -> auto {
                return std::span<math::Vector3>(reinterpret_cast<math::Vector3*>(accessor.getData()), count); };
            auto getSpan2 = [count](AccessorData& accessor) -> auto {
                return std::span<math::Vector2>(reinterpret_cast<math::Vector2*>(accessor.getData()), count); };

            auto positionSpan = getSpan3(position);
            auto normalSpan = getSpan3(normal);
            auto textureSpan = getSpan2(texture);

            for (int i = 0; i < count; i++)
            {
                m_geometry.vertices.push_back({ positionSpan[i], normalSpan[i], textureSpan[i] });
            }

            auto indicesAccessor = getAccessor(primitive.indices);
            u32 indexCount = indicesAccessor.accessor.count;
            i32 stride = indicesAccessor.byteStride();

            // todo adapt indexData to stride size
            const u16* indexData = reinterpret_cast<u16*>(indicesAccessor.getData());
            m_geometry.indices.reserve(indexCount);
            for (int i = 0; i < indexCount; i+=3)
            {
                m_geometry.indices.push_back(indexData[i+0]);
                m_geometry.indices.push_back(indexData[i+1]);
                m_geometry.indices.push_back(indexData[i+2]);
            }

            auto& material = m_model->materials[primitive.material != -1 ? primitive.material : 0];
            
            m_submeshes.push_back(m_geometry.getSubmesh(
                material.pbrMetallicRoughness.baseColorTexture.index));
        }
    }

    void Model::drawModel(ID3D12GraphicsCommandList* cmdList)
    {
        auto vertexBuffer = GetVertexBufferView(m_mesh.m_vertexBuffer);
        auto indexBuffer = GetIndexBufferView(m_mesh.m_indexBuffer);
        cmdList->IASetVertexBuffers(0, 1, &vertexBuffer);
        cmdList->IASetIndexBuffer(&indexBuffer);

        for (auto& submesh : m_submeshes)
        {
            cmdList->SetGraphicsRootDescriptorTable(1, m_textures[submesh.materialIndex].srv.HandleGPU);
            cmdList->DrawIndexedInstanced(submesh.IndexCount, 1, submesh.StartIndexLocation, submesh.BaseVertexLocation, 0);
        }
    }

};

