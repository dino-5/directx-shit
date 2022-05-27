#ifndef TEXTURE_H
#define TEXTURE_H

#include <d3d12.h>
#include "d3dx12.h"
#include "common.h"

class Texture
{
public:
	Texture(const char* path, ComPtr<ID3D12Device>& device, ComPtr<ID3D12GraphicsCommandList>&, ComPtr<ID3D12DescriptorHeap>&, UINT);

private:
	ComPtr<ID3D12Resource> m_texture;
	int width = 0,
		height = 0,
		nrChannels =0;
    ComPtr<ID3D12Resource> textureUploadHeap;
};

#endif