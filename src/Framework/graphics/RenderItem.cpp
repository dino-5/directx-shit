#include "Framework/graphics/RenderItem.h"
#include "Framework/util/ImGuiSettings.h"
#include "Framework/graphics/dx12/Device.h"
#include "Framework/graphics/Model.h"

namespace engine::graphics
{
	void ObjectConstants::SetTranslation(float x, float y, float z)
	{
		Translation[0] = x;
		Translation[1] = y;
		Translation[2] = z;
	}

	void ObjectConstants::SetScale(float x)
	{
		Scale = x;
	}

	void ObjectConstants::SetRotation(XMMATRIX matrix)
	{
	}

	void ObjectConstants::Update()
	{
		if (m_state != ObjectConstantsState::REFLECTED)
			XMStoreFloat4x4(&properties.World, XMMatrixScaling(Scale, Scale, Scale) *
				XMMatrixTranslation(Translation[0], Translation[1], Translation[2]));
		else
		{
			XMVECTOR mirrorPlane = XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f); // xy plane
			XMMATRIX R = XMMatrixReflect(mirrorPlane);
			XMStoreFloat4x4(&properties.World, XMMatrixScaling(Scale, Scale, Scale) *
				XMMatrixTranslation(Translation[0], Translation[1], Translation[2]) * R);
		}
		m_framesToUpdate = framesToUpdate;
	}

	void ObjectConstants::OnImGuiRender()
	{

		float min_translation = -10;
		float max_translation = 10;

		float min_scale = 1;
		float max_scale = 5;
		if (util::ImGuiSettings::SliderFloat3("pos", Translation, min_translation, max_translation))
		{
			Update();
		}

		if (util::ImGuiSettings::SliderFloat("scale", &Scale, min_scale, max_scale))
		{
			Update();
		}

	}

	RenderItem::RenderItem(std::vector <util::Geometry::MeshData>& meshes, ComPtr<ID3D12GraphicsCommandList> cmdList,
		std::string name, RenderItemState state) :
		m_name(name),
		m_objCbIndex(g_objectCBIndex++),
		m_state(state)
	{
		Geo = Mesh(Device::device->GetDevice(), meshes, cmdList, name);

		if (m_state == RenderItemState::REFLECTED)
		{
			m_transformation.m_state = ObjectConstants::ObjectConstantsState::REFLECTED;
		}
	}

	RenderItem::RenderItem(Model model, std::string name, RenderItemState state)
		: Geo(model.m_mesh), m_name(name), m_objCbIndex(g_objectCBIndex++), m_state(state)
	{
		if (m_state == RenderItemState::REFLECTED)
		{
			m_transformation.m_state = ObjectConstants::ObjectConstantsState::REFLECTED;
		}
	}

	RenderItem::RenderItem(Mesh& model, std::string name, RenderItemState state)
		: Geo(model), m_name(name), m_objCbIndex(g_objectCBIndex++), m_state(state)
	{
		if (m_state == RenderItemState::REFLECTED)
		{
			m_transformation.m_state = ObjectConstants::ObjectConstantsState::REFLECTED;
		}
	}

	void RenderItem::SetPrimitiveTopology(ID3D12GraphicsCommandList* cmList)
	{
		cmList->IASetPrimitiveTopology(m_primitiveType);
	}


	void RenderItem::Update()
	{
		numFramesDirty = engine::config::NumFrames;
		m_transformation.Update();
	}

	void RenderItem::OnImGuiRender()
	{
		util::ImGuiSettings::Begin(m_name.c_str());
		{
			m_transformation.OnImGuiRender();
		}
		util::ImGuiSettings::End();
	}
};
