#include "EngineGfx/Model.h"
#include "EngineGfx/Mesh.h"
#include "EngineGfx/Texture.h"
#include "EngineGfx/dx12/Device.h"
#include "EngineCommon/util/Logger.h"
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
        for (tinygltf::Texture & texture: model.textures)
        {
            tinygltf::Image image = model.images[texture.source];
            ImageData data;
            data.SetData(image.image.data());
            data.width = image.width;
            data.height = image.height;
            data.channels = image.component;
            data.name = image.uri.c_str();
            Texture::CreateTexture(data, device, commandList.GetList());
        }
    }

};

