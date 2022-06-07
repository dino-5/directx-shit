#pragma once

#include <vector>
#include <string>
#include <assimp/material.h>
#include "Mesh.h"
#include "Texture.h"
#include <iostream>
#include "GeometryGenerator.h"

class aiNode;
class aiMesh;
struct aiMaterial;
struct aiScene;

class Model
{
    public:
        Model(char* path, ComPtr<ID3D12GraphicsCommandList> cmList)
        {
            Init(path, cmList);
        }
        Model() = default;
        void Init(char* path, ComPtr<ID3D12GraphicsCommandList> cmList)
        {
            LoadModel(path, cmList);
            std::cout<<"bye"<<std::endl;
        }

        std::vector<Mesh> GetMesh() { return m_meshes; }
        void DrawModel(ID3D12GraphicsCommandList* cmdList);

    private:
        std::vector<Texture> m_textures;
        std::vector<Mesh> m_meshes;
        std::string m_directory;

    private:
        void LoadModel(std::string path, ComPtr<ID3D12GraphicsCommandList> cmList);
        void ProcessNode(aiNode* node, const aiScene* scene, ComPtr<ID3D12GraphicsCommandList> cmList);
        Geometry::MeshData ProcessMesh(aiMesh* mesh, const aiScene* scene, ComPtr<ID3D12GraphicsCommandList> cmList);
        std::vector<TextureHandle> LoadMaterialTextures(aiMaterial* mat,
            aiTextureType type, std::string typeName, ComPtr<ID3D12GraphicsCommandList> cmList);

};
