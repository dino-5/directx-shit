#ifndef TEXTURE_H
#define TEXTURE_H

#include <d3d12.h>
#include "d3dx12.h"
#include <string>
#include "common.h"

class DescriptorHeap;
using TextureHandle = unsigned int;

void BindTextures(std::vector<TextureHandle>& textures, ID3D12GraphicsCommandList* cmdList);

class Texture
{
public:
	Texture() = default;
	Texture(const char* path, ID3D12Device* device, ComPtr<ID3D12GraphicsCommandList>&, std::string s="");
	void Init(const char* path, ID3D12Device* device, ComPtr<ID3D12GraphicsCommandList>&, std::string s="");
	TextureHandle GetHandle()const { return index; }

public:
	ComPtr<ID3D12Resource> m_texture;
	int width = 0,
		height = 0,
		nrChannels =0;
    ComPtr<ID3D12Resource> textureUploadHeap;
	TextureHandle index;
	std::string m_name;

};

class TextureColection
{
public:
	static TextureHandle CreateTexture(const char* path, ID3D12Device* device, ComPtr<ID3D12GraphicsCommandList>&, std::string s="");

	static inline std::unordered_map<std::string, Texture> static_textures;


};


#endif