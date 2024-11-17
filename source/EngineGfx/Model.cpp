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
    void Model::init(system::Filepath path, graphics::GfxContext& context)
    {
        m_context = &context;
        m_directory.init(path.getPath().remove_filename());
        m_model = std::make_unique<tinygltf::Model>();
        bool success= locLoadModel(m_model.get(), path.str());

        if (!success)
        {
            util::printError("failed to load model {}", path.str());
            return;
        }

        u64 vertexCount = 0;
        u64 indexCount = 0;
        for (auto& mesh : m_model->meshes)
        {
            for (auto& primitive : mesh.primitives)
            {
                indexCount += primitive.indices;
                int positionIndex, texIndex, normalIndex;
                for (auto& [attrName, attrIndex] : primitive.attributes)
                {
                    if (attrName == "POSITION")
                    {
                        auto accessor = getAccessor(attrIndex);
                        vertexCount += accessor.accessor.count;
                        break;
                    }
                    else if (attrName == "NORMAL")
                    {
                        auto accessor = getAccessor(attrIndex);
                        vertexCount += accessor.accessor.count;
                        break;
                    }
                    else if (attrName == "TEXCOORD_0")
                    {
                        auto accessor = getAccessor(attrIndex);
                        vertexCount += accessor.accessor.count;
                        break;
                    }
                }
            }
        }
        m_geometry.vertices.reserve(vertexCount);
        m_geometry.indices.reserve(indexCount);

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

        std::vector<SubmeshData> materialData;
        materialData.reserve(m_textures.size());
        for (auto& material: m_model->materials)
        {
            i32 cIndex = material.pbrMetallicRoughness.baseColorTexture.index;
            i32 nIndex = material.normalTexture.index;
            i32 colorTexture = m_textures[cIndex].getDescriptorHeapIndex();
            i32 normalTexture = nIndex >= 0 ? m_textures[nIndex].getDescriptorHeapIndex() : -1;
            
            materialData.push_back({ colorTexture, normalTexture });
        }

        BufferDescription<SubmeshData> desc;
        desc.data = materialData.data();
        desc.elementCount = materialData.size();
        desc.name = "model submesh data";
        desc.state = ResourceState::PIXEL_SHADER_RESOURCE;
        desc.type = BufferType::CUSTOM;

        m_materialBuffer.init(context, desc);

        m_context = nullptr;
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
            m_textures.push_back(Texture(ImageData(m_directory/image.uri), m_context->device, m_context->cmdList));
        }

    }

    void Model::processNode(uint index)
    {
        auto& node = m_model->nodes[index];
        if (node.children.size())
            for (auto& nodeIndex : node.children)
            {
                processNode(nodeIndex);
            }
        if (node.mesh != -1)
            processMesh(node.mesh);

    }

    void Model::processMesh(uint index)
    {
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

            // todo adapt indexData to stride size
            const u16* indexData = reinterpret_cast<u16*>(indicesAccessor.getData());
            for (int i = 0; i < indexCount; i+=3)
            {
                m_geometry.indices.push_back(indexData[i+0]);
                m_geometry.indices.push_back(indexData[i+1]);
                m_geometry.indices.push_back(indexData[i+2]);
            }

            //auto& material = m_model->materials[primitive.material != -1 ? primitive.material : 0];

            u32 materialIndex = primitive.material != -1 ? primitive.material : 0;
            m_submeshes.push_back(m_geometry.getSubmesh(materialIndex));
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

