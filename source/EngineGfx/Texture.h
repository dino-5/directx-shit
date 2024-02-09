#ifndef TEXTURE_H
#define TEXTURE_H

#include "EngineCommon/include/common.h"
#include "EngineGfx/dx12/d3dx12.h"
#include "EngineCommon/include/types.h"
#include "EngineGfx/dx12/Resource.h"
#include <d3d12.h>
#include <string>
#include <unordered_map>

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
	void BindTextures(Material& textures, ID3D12GraphicsCommandList* cmdList);


	class Texture
	{
	public:
		Texture() = default;
		Texture(const char* path, ID3D12Device* device, ComPtr<ID3D12GraphicsCommandList>&, std::string s = "");
		void Init(const char* path, ID3D12Device* device, ComPtr<ID3D12GraphicsCommandList>&, std::string s = "");
		TextureHandle GetHandle()const { return index; }

	public:
		Resource m_texture;
		Resource textureUploadHeap;
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
		static TextureHandle CreateTexture(const char* path, ID3D12Device* device, ComPtr<ID3D12GraphicsCommandList>&, std::string s = "");

		static inline std::unordered_map<std::string, Texture> static_textures;
	};

};
#endif