#include "EngineGfx/Model.h"
#include "EngineGfx/Mesh.h"
#include "EngineGfx/Texture.h"
#include "EngineGfx/dx12/Device.h"
#include "EngineCommon/util/Logger.h"
#include "third_party/tiny_gltf_loader/tiny_gltf.h"


namespace engine::graphics
{
    void Model::Init(system::Filepath path, ComPtr<ID3D12GraphicsCommandList> cmList)
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
        for (tinygltf::Sampler& sampler : model.samplers)
        {
            
        }
    }

};

