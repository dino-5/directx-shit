#ifndef TEXTURE_H
#define TEXTURE_H

#include "EngineCommon/include/common.h"
#include "EngineCommon/include/types.h"
#include "EngineCommon/system/Filesystem.h"
#include "EngineGfx/dx12/Resource.h"
#include "EngineGfx/dx12/d3dx12.h"
#include <d3d12.h>
#include <string>
#include <unordered_map>
using namespace engine;

namespace engine::graphics 
{

	class DescriptorHeap;
	using TextureHandle = int;

	struct Material
	{
		TextureHandle diffuseTexture = -1;
		TextureHandle specularTexture = -1;
		float shininess = 1;
	};

	class Texture : public Resource
	{
	public:
		Texture() = default;
		Texture(system::Filepath path, ID3D12Device* device, ID3D12GraphicsCommandList*, std::string s = "");
		void Init(system::Filepath path, ID3D12Device* device, ID3D12GraphicsCommandList*, std::string s = "");
		TextureHandle GetHandle()const { return index; }
		u32 getDescriptorHeapIndex()
		{
			return srv.getDescriptorIndex();
		}

	public:
		int width = 0,
			height = 0,
			nrChannels = 0;
		TextureHandle index;
		std::string m_name;

	};


	bool operator==(Material mat1, Material mat2);

	class TextureColection
	{
	public:
		static TextureHandle CreateTexture(system::Filepath path, ID3D12Device* device, ID3D12GraphicsCommandList*, std::string s = "");

		static inline std::unordered_map<std::string, Texture> static_textures;
	};

};
#endif