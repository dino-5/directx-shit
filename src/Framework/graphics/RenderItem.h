#pragma once
#include "Framework/include/common.h"
#include "Framework/util/MathHelper.h"
#include <DirectXMath.h>
#include <string>
#include "Mesh.h"
#include "Framework/System/config.h"

#include "Framework/graphics/dx12/PSO.h"
using namespace engine;

struct ObjectConstants
{
public:
	enum class ObjectConstantsState
	{
		NONE,
		REFLECTED
	};
	static inline const int framesToUpdate = 3;
	struct ObjectProperties
	{
		XMFLOAT4X4 World = util::MathHelper::Identity4x4();
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

public:
	float Translation[3] = { 0,0,0 };
	float Scale = 1;
	float Rotate[3] = { 0,0,0 };
	int m_framesToUpdate = framesToUpdate;

	ObjectProperties properties;
	ObjectConstantsState m_state = ObjectConstantsState::NONE;
 };

class Model;
class Resource;


class RenderItem
{
public:
	enum class RenderItemState
	{
		OPAQUE_STATE,
		TRANSPARENT_STATE,
		REFLECTED,
		BOX
	};
	static inline uint16_t g_objectCBIndex = 0;
	RenderItem() = default;
	RenderItem(Mesh& mesh, std::string name, ObjectConstants obj, RenderItemState state = RenderItemState::OPAQUE_STATE) :
		Geo(mesh),
		m_transformation(obj), 
		m_name(name),
		m_objCbIndex(g_objectCBIndex++),
		m_state(state)
	{
		if (m_state == RenderItemState::REFLECTED)
		{
			m_transformation.m_state = ObjectConstants::ObjectConstantsState::REFLECTED;
		}
	}
	RenderItem(std::vector <util::Geometry::MeshData>& meshes, ComPtr<ID3D12GraphicsCommandList> cmdList, std::string name,
	RenderItemState state = RenderItemState::OPAQUE_STATE);
	RenderItem(Model model, std::string name="", RenderItemState state = RenderItemState::OPAQUE_STATE);
	RenderItem(Mesh& model, std::string name="", RenderItemState state = RenderItemState::OPAQUE_STATE);

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

	int numFramesDirty = engine::config::NumFrames;
	UINT  m_objCbIndex = 0;
	Mesh Geo;
	ObjectConstants m_transformation;
	Material* material = nullptr;

	D3D12_PRIMITIVE_TOPOLOGY m_primitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

	std::string m_name;
	RenderItemState m_state;
};

class RenderContext
{
public:
	RenderContext() = default;

private:
	PSO* m_pso;
	std::vector<RenderItem*> m_renderItems;
};

