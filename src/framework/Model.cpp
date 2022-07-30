#include "include/Model.h"
#include "include/Mesh.h"
#include "include/Texture.h"
#include "dx12/Device.h"
#include <external/assimp/Importer.hpp>
#include <external/assimp/scene.h>
#include <external/assimp/postprocess.h>
#include <sstream>
#include "dx12/DescriptorHeap.h"

int FindVector(std::vector<std::pair< std::vector<TextureHandle>, std::vector<Geometry::MeshData> >>& meshes, std::vector<TextureHandle>& textures)
{
    for (int i=0;i<meshes.size();i++)
    {
        if (meshes[i].first == textures)
            return i;
    }
    return -1;
}



void Model::LoadModel(std::string path, ComPtr<ID3D12GraphicsCommandList> cmList)
{
    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate);
    if(!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
    {
        std::stringstream stream;
        stream<<"ERROR::ASSIMP::"<<importer.GetErrorString()<<std::endl;
        OutputDebugString(stream.str().c_str());
        return;
    }
    m_directory =  path.substr(0, path.find_last_of('/'))+"/";
    std::vector<std::pair< std::vector<TextureHandle>, std::vector<Geometry::MeshData> >> meshes;
    ProcessNode(scene->mRootNode, scene, cmList, meshes);
    m_mesh = Mesh(meshes, cmList, "model");
}

void Model::ProcessNode(aiNode* node, const aiScene* scene, ComPtr<ID3D12GraphicsCommandList> cmList, 
    std::vector<std::pair< std::vector<TextureHandle>, std::vector<Geometry::MeshData> >>& meshes)
{
    for(unsigned int i=0; i<node->mNumMeshes; i++)
    {
        aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
        auto meshElement = ProcessMesh(mesh, scene, cmList);

        int indx = FindVector(meshes, meshElement.first);
        if (indx==-1)
        {
            meshes.push_back({ meshElement.first, {meshElement.second}  });
        }
        else
        {
            meshes[indx].second.push_back(meshElement.second);
        }
    }

    for(unsigned int i=0;i < node->mNumChildren; i++)
    {
        ProcessNode(node->mChildren[i], scene, cmList, meshes);
    }
}

std::pair<std::vector<TextureHandle> ,Geometry::MeshData> Model::ProcessMesh(aiMesh* mesh, const aiScene* scene, ComPtr<ID3D12GraphicsCommandList> cmList)
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
    return { textures, Geometry::MeshData(vertices, indices) };
}

std::vector<TextureHandle> Model::LoadMaterialTextures(aiMaterial* mat, aiTextureType type, std::string typeName,
    ComPtr<ID3D12GraphicsCommandList> cmList)
{
    std::vector<TextureHandle> textures;
    for(unsigned int i=0; i<mat->GetTextureCount(type);i++)
    {
        aiString str;
        mat->GetTexture(type, i, &str);

		std::string name(str.C_Str());
		textures.push_back(TextureColection::CreateTexture((m_directory+name).c_str(), Device::GetDevice(), cmList, name));
    }
    return textures;
}

void Model::DrawModel(ID3D12GraphicsCommandList* cmdList)
{
    m_mesh.SetIndexBuffer(cmdList);
    m_mesh.SetVertexBuffer(cmdList);
    for (auto& i : m_mesh.DrawArgs)
    {
        BindTextures(i.first, cmdList);
        for(auto& submesh: i.second)
			submesh.Draw(cmdList);
    }
}


