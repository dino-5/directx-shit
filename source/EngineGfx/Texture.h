#ifndef TEXTURE_H
#define TEXTURE_H

#include "EngineCommon/include/types.h"
#include "EngineCommon/system/Filesystem.h"
#include "EngineCommon/util/Logger.h"
#include "EngineGfx/dx12/Resource.h"
#include "EngineGfx/dx12/d3dx12.h"
#include "dx12/Device.h"
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
		void setData(T* newData)
		{
			data = reinterpret_cast<u8*>(newData);
		}
        ImageData() = default;
        ImageData(engine::system::Filepath path) { init(path); }
        void init(engine::system::Filepath path);
		const char* name = nullptr;
		bool needToDestruct = false;
    };


	class Texture : public Resource
	{
	public:
		Texture() = default;
		Texture (ImageData imData, const GfxContext& ctx);
		void init(ImageData imData, const GfxContext& ctx);
		u32 getDescriptorHeapIndex()
		{
			return srv.getDescriptorIndex();
		}

	public:
		Resource textureUploadHeap;
	};

};
#endif
