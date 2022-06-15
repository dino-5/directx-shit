#pragma once
#include "common.h"
#include "MathHelper.h"
#include <DirectXMath.h>
#include <string>
#include "Mesh.h"

class RenderItem
{
public:
	RenderItem() = default;
	RenderItem(Mesh& mesh, std::string submeshName, ObjectConstants obj) :
		objectParam(obj), 
		Geo(&mesh),
		m_submeshName(submeshName),
		m_name(submeshName)
	{ }

	void DrawIndexedInstanced(ID3D12GraphicsCommandList* cmList)
	{
		Geo->DrawArgs[m_submeshName].Draw(cmList);
	}
	void SetTranslation(float, float, float);
	void SetTranslation(XMMATRIX);
	void SetScale(float, float, float);
	void SetScale(XMMATRIX);
	void SetRotation(XMMATRIX);

	void Update();
	void OnImGuiRender();
	void SetPrimitiveTopology(ID3D12GraphicsCommandList* cmList);

	int numFramesDirty = NumFrames;
	UINT  m_objCbIndex = 0;
	Mesh* Geo = nullptr;
	ObjectConstants objectParam;
	std::string m_submeshName;

	D3D12_PRIMITIVE_TOPOLOGY m_primitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

	float Translation[3] = { 0,0,0 };
	float Scale[3] = { 1,1,1 };
	float Rotate[3] = { 0,0,0 };
	std::string m_name;
};

