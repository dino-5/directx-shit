#include "include/Model.h"
#include "include/Mesh.h"
#include "include/Texture.h"
#include "dx12/Device.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <sstream>
#include "dx12/DescriptorHeap.h"



void Model::LoadModel(std::string path, ComPtr<ID3D12GraphicsCommandList> cmList)
{
    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate);
    if(!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
    {
        std::wstringstream stream;
        stream<<"ERROR::ASSIMP::"<<importer.GetErrorString()<<std::endl;
        OutputDebugString(stream.str().c_str());
        return;
    }
    m_directory =  path.substr(0, path.find_last_of('/'))+"/";
	std::vector<Geometry::MeshData> meshes;
    ProcessNode(scene->mRootNode, scene, cmList, meshes);

    auto submeshes = Submesh::GetSubmeshes(meshes);

    auto sizeOfStruct = sizeof(Geometry::Vertex);
    std::vector<Geometry::Vertex> vertices;
    std::vector<std::uint16_t>     indices;
    for (int i = 0; i < meshes.size(); i++)
    {
        vertices.insert(vertices.end(), meshes[i].Vertices.begin(), meshes[i].Vertices.end());
        indices.insert(indices.end(), meshes[i].GetIndices16().begin(), meshes[i].GetIndices16().end());
    }
    m_mesh.Init(Device::GetDevice(), cmList,
        vertices.data(), GetVectorSize(vertices)  , sizeOfStruct,
        indices.data(), GetVectorSize(indices) , DXGI_FORMAT_R16_UINT);

    for (int i = 0; i < submeshes.size(); i++)
    {
        m_mesh.DrawArgs[std::to_string(i)] = submeshes[i];
    }
    
}

void Model::ProcessNode(aiNode* node, const aiScene* scene, ComPtr<ID3D12GraphicsCommandList> cmList, 
    std::vector<Geometry::MeshData>& meshes)
{
    for(unsigned int i=0; i<node->mNumMeshes; i++)
    {
        aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
        meshes.push_back(ProcessMesh(mesh, scene, cmList));
    }

    for(unsigned int i=0;i < node->mNumChildren; i++)
    {
        ProcessNode(node->mChildren[i], scene, cmList, meshes);
    }
}

Geometry::MeshData Model::ProcessMesh(aiMesh* mesh, const aiScene* scene, ComPtr<ID3D12GraphicsCommandList> cmList)
{
    std::vector<Geometry::Vertex> vertices;
    std::vector<unsigned int> indices;
    std::vector<TextureHandle> textures;

    for(unsigned int i=0; i<mesh->mNumVertices;i++)
    {
        Geometry::Vertex vertex;
        vertex.Position.x = mesh->mVertices[i].x;
        vertex.Position.y = mesh->mVertices[i].y;
        vertex.Position.z = mesh->mVertices[i].z;
         
        vertex.Normal.x = mesh->mNormals[i].x;
        vertex.Normal.y = mesh->mNormals[i].y;
        vertex.Normal.z = mesh->mNormals[i].z;

        if (mesh->mTextureCoords[0])
        {
            vertex.Tex.x = mesh->mTextureCoords[0][i].x;
            vertex.Tex.y = mesh->mTextureCoords[0][i].y;
        }
        else
            vertex.Tex = { 0, 0 };
        
        vertices.push_back(vertex);
    }

    // process indices 
    for(unsigned int i=0;i<mesh->mNumFaces;i++)
    {
        aiFace face = mesh->mFaces[i];
        for(unsigned int j=0; j< face.mNumIndices; j++)
            indices.push_back(face.mIndices[j]);
    }

    // process material
    if(mesh->mMaterialIndex>=0)
    {
        aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
        std::vector<TextureHandle> diffuseMaps = LoadMaterialTextures(material, aiTextureType_DIFFUSE, "texture_diffuse", cmList);
        textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());
        std::vector<TextureHandle> specularMaps = LoadMaterialTextures(material, aiTextureType_SPECULAR, "texture_specular", cmList);
        textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());
    }
    return Geometry::MeshData(vertices, indices);
}

std::vector<TextureHandle> Model::LoadMaterialTextures(aiMaterial* mat, aiTextureType type, std::string typeName,
    ComPtr<ID3D12GraphicsCommandList> cmList)
{
    std::vector<TextureHandle> textures;
    for(unsigned int i=0; i<mat->GetTextureCount(type);i++)
    {
        aiString str;
        mat->GetTexture(type, i, &str);

        bool skip =false;
        for(unsigned int j=0;j<m_textures.size();j++)
        {
            if(std::strcmp(m_textures[j].m_name.data(), str.C_Str())==0)
            {
                textures.push_back(m_textures[j].GetHandle());
                skip = true;
                break;
            }
        }
        if(!skip)
        {
            const char* dir = "";
            std::string name(str.C_Str());
            Texture texture((m_directory+name).c_str(), Device::GetDevice(), cmList);
            texture.m_name = name;
            textures.push_back(texture.GetHandle());
            m_textures.push_back(texture);
        }
    }
    return textures;
}

void Model::DrawModel(ID3D12GraphicsCommandList* cmdList)
{

    auto textureHandle1 = DescriptorHeapManager::CurrentSRVHeap->GetGPUHandle(m_textures[0].GetHandle());
    cmdList->SetGraphicsRootDescriptorTable(2, textureHandle1);

    auto textureHandle2 = DescriptorHeapManager::CurrentSRVHeap->GetGPUHandle(m_textures[1].GetHandle());
    cmdList->SetGraphicsRootDescriptorTable(3, textureHandle1);
    m_mesh.SetIndexBuffer(cmdList);
    m_mesh.SetVertexBuffer(cmdList);
    for(auto& i:m_mesh.DrawArgs)
		i.second.Draw(cmdList);
}


