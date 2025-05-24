#include "EngineGfx/Mesh.h"
#include "EngineGfx/dx12/Device.h"
#include "EngineGfx/dx12/DescriptorHeap.h"

namespace engine::graphics
{
	void Submesh::draw(ID3D12GraphicsCommandList* cmList)const
	{
		if (IndexCount)
		{
			cmList->DrawIndexedInstanced(IndexCount, 1, StartIndexLocation, BaseVertexLocation, 0);
		}
	}
};
