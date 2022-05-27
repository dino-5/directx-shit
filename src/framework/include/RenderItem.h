#pragma once
#include "common.h"
#include "MathHelper.h"
#include <DirectXMath.h>
#include <string>

class RenderItem
{
public:
	RenderItem() = default;

	//DirectX::XMFLOAT4X4 m_World = MathHelper::Identity4x4();
	ObjectConstants objectParam;

	void SetTranslation(float, float, float);
	void SetTranslation(XMMATRIX);
	void SetScale(float, float, float);
	void SetScale(XMMATRIX);
	void SetRotation(XMMATRIX);
	void Update();
	void OnImGuiRender();

	int numFramesDirty = NumFrames;
	UINT  m_objCbIndex = -1;

	MeshGeometry* Geo = nullptr;

	D3D12_PRIMITIVE_TOPOLOGY m_primitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

	UINT m_indexCount         = 0;
	UINT m_startIndexLocation = 0;
	UINT m_baseVertexLocation = 0;

	float Translation[3] = { 0,0,0 };
	float Scale[3] = { 1,1,1 };
	float Rotate[3] = { 0,0,0 };

	std::string m_name;

};

