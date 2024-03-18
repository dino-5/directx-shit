#ifndef TEXTURE_H
#define TEXTURE_H

#include "EngineCommon/include/common.h"
#include "EngineCommon/include/types.h"
#include "EngineCommon/system/Filesystem.h"
#include "EngineCommon/util/Logger.h"
#include "EngineGfx/dx12/Resource.h"
#include "EngineGfx/dx12/d3dx12.h"
#include <d3d12.h>
#include <string>
#include <unordered_map>

using namespace engine;

namespace engine::graphics 
{
	class DescriptorHeap;
    struct ImageData
    {
        int width;
        int height;
        int channels;
        u8* data;
		ImageData(const ImageData& imageData) = default;
		ImageData(ImageData& imageData)
		{
			data = imageData.data;
			width = imageData.width;
			height = imageData.height;
			channels = imageData.channels;
			name = imageData.name;
			imageData.data = nullptr;
			needToDestruct = imageData.needToDestruct;
		}
		~ImageData()
		{
			if (needToDestruct && data)
				delete[] data;
		}
		template<typename T>
		void SetData(T* newData)
		{
			data = reinterpret_cast<u8*>(newData);
		}
        ImageData() = default;
        ImageData(engine::system::Filepath path) { Init(path); }
        void Init(engine::system::Filepath path);
		const char* name;
		bool needToDestruct = false;
    };


	class Texture : public Resource
	{
	public:
		Texture() = default;
		Texture (ImageData imData, ID3D12Device* device, ID3D12GraphicsCommandList* cmdList, std::string s = "");
		void Init(ImageData imData, ID3D12Device* device, ID3D12GraphicsCommandList* cmdList, std::string s = "");
		u32 getDescriptorHeapIndex()
		{
			return srv.getDescriptorIndex();
		}

		static Texture* CreateTexture(ImageData imData, ID3D12Device* device, ID3D12GraphicsCommandList* cmdList, std::string s = "");
		static Texture* GetTexture(std::string name){
            if(s_textures.find(name)!=s_textures.end())
                return &s_textures[name];
			util::PrintError("trying to get texture which does not exist {}", name);
			return nullptr;
		}

		static inline std::unordered_map<std::string, Texture> s_textures;
	public:
		Resource textureUploadHeap;
		std::string m_name;
	};

	class TextureHandle
	{
	public:
		TextureHandle() = default;
		TextureHandle(ImageData imData, ID3D12Device* device, ID3D12GraphicsCommandList* cmdList, std::string s = ""){
			Init(imData, device, cmdList, s);
		}
		void Init(ImageData imData, ID3D12Device* device, ID3D12GraphicsCommandList* cmdList, std::string s = "")
		{
			m_ptr = Texture::CreateTexture(imData, device, cmdList, s);
		}
		void Init(std::string name)
		{
			m_ptr = Texture::GetTexture(name);
		}
		Texture* operator->() { return m_ptr; }
	private:
		Texture* m_ptr;
	};

};
#endif