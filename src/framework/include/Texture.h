#ifndef TEXTURE_H
#define TEXTURE_H

#include <d3d12.h>
#include "d3dx12.h"
#include <string>
#include "common.h"

class DescriptorHeap;
using TextureHandle = unsigned int;

class Texture
{
public:
	Texture() = default;
	Texture(const char* path, ID3D12Device* device, ComPtr<ID3D12GraphicsCommandList>&);
	void Init(const char* path, ID3D12Device* device, ComPtr<ID3D12GraphicsCommandList>&);
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


#endif