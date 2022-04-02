#pragma once
#include"common.h"
#include "MathHelper.h"
#include<DirectXMath.h>

class RenderItem
{
public:
	RenderItem() = default;

	DirectX::XMFLOAT4X4 m_World = MathHelper::Identity4x4();

	int numFramesDirty = NumFrameResources;
	UINT  m_objCbIndex = -1;

	MeshGeometry* Geo = nullptr;

	D3D12_PRIMITIVE_TOPOLOGY m_primitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

	UINT m_indexCount = 0;
	UINT m_startIndexLocation = 0;
	UINT m_baseVertexLocation = 0;

};

