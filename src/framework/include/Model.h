#pragma once

#include <vector>
#include <string>
#include "Mesh.h"
#include "Texture.h"
#include <iostream>
#include <utility>
#include "GeometryGenerator.h"


class Model
{
    public:
        Model(char* path, ComPtr<ID3D12GraphicsCommandList> cmList)
        {
            Init(path, cmList);
        }
        Model() = default;
        void Init(const char* path, ComPtr<ID3D12GraphicsCommandList> cmList)
        {
            std::cout<<"bye"<<std::endl;
        }

        Mesh GetMesh() { return m_mesh; }
        void DrawModel(ID3D12GraphicsCommandList* cmdList);

    public:
        Mesh m_mesh;
        std::string m_directory;

    private:

};
