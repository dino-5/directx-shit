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

        Mesh GetMesh() { return m_mesh; }
        void DrawModel(ID3D12GraphicsCommandList* cmdList);

    private:
        std::vector<Texture> m_textures;
        Mesh m_mesh;
        std::string m_directory;

    private:
        void LoadModel(std::string path, ComPtr<ID3D12GraphicsCommandList> cmList);
        void ProcessNode(aiNode* node, const aiScene* scene, ComPtr<ID3D12GraphicsCommandList> cmList, 
            std::vector<Geometry::MeshData>&);
        Geometry::MeshData ProcessMesh(aiMesh* mesh, const aiScene* scene, ComPtr<ID3D12GraphicsCommandList> cmList);
        std::vector<TextureHandle> LoadMaterialTextures(aiMaterial* mat,
            aiTextureType type, std::string typeName, ComPtr<ID3D12GraphicsCommandList> cmList);

};
