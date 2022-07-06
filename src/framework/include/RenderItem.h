#pragma once
#include "common.h"
#include "MathHelper.h"
#include <DirectXMath.h>
#include <string>
#include "Mesh.h"


struct ObjectConstants
{
	static inline const int framesToUpdate = 3;
	struct ObjectProperties
	{
		XMFLOAT4X4 World = MathHelper::Identity4x4();
		ObjectProperties() = default;
		ObjectProperties(XMFLOAT4X4 world) : World(world) {}

		ObjectProperties(XMMATRIX world) {
			XMStoreFloat4x4(&World, world);
		}
		
	};
	ObjectConstants() = default;
	ObjectConstants(XMFLOAT4X4 world) : properties(world) {}

	ObjectConstants(XMMATRIX world) {
		XMStoreFloat4x4(&properties.World, world);
	}

	void SetTranslation(float, float, float);
	void SetTranslation(XMMATRIX);
	void SetScale(float);
	void SetScale(XMMATRIX);
	void SetRotation(XMMATRIX);
	void Update();
	void OnImGuiRender();

	float Translation[3] = { 0,0,0 };
	float Scale = 1;
	//float Scale[3] = { 1,1,1 };
	float Rotate[3] = { 0,0,0 };
	int m_framesToUpdate = framesToUpdate;

	ObjectProperties properties;

 };

class Model;

class RenderItem
{
public:
	static inline uint16_t g_objectCBIndex = 0;
	RenderItem() = default;
	RenderItem(Mesh& mesh, std::string name, ObjectConstants obj) :
		Geo(mesh),
		m_transformation(obj), 
		m_name(name),
		m_objCbIndex(g_objectCBIndex++)
	{ }
	RenderItem(std::vector <Geometry::MeshData>& meshes, ComPtr<ID3D12GraphicsCommandList> cmdList, std::string name);
	RenderItem(Model model, std::string name="");

	void DrawIndexedInstanced(ID3D12GraphicsCommandList* cmList, bool fl)
	{
		for (auto& i : Geo.DrawArgs)
		{
			if(fl)
				BindTextures(i.first, cmList);
			for(auto& submesh: i.second)
				submesh.Draw(cmList);
		}
	}

	void Draw(ID3D12GraphicsCommandList* cmdList ,bool fl)
	{
		Geo.SetIndexBuffer(cmdList);
		Geo.SetVertexBuffer(cmdList);
		SetPrimitiveTopology(cmdList);
		for (auto& i : Geo.DrawArgs)
		{
			if(fl)
				BindTextures(i.first, cmdList);
			for(auto& submesh: i.second)
				submesh.Draw(cmdList);
		}
	}

	void Update();
	void OnImGuiRender();
	void SetPrimitiveTopology(ID3D12GraphicsCommandList* cmList);

public:

	int numFramesDirty = NumFrames;
	UINT  m_objCbIndex = 0;
	Mesh Geo;
	ObjectConstants m_transformation;

	D3D12_PRIMITIVE_TOPOLOGY m_primitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

	std::string m_name;
};

