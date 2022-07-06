#include "include/RenderItem.h"
#include "include/ImGuiSettings.h"
#include "dx12/Device.h"
#include "include/Model.h"

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
	XMStoreFloat4x4(&properties.World,XMMatrixScaling(Scale, Scale, Scale) * 
		XMMatrixTranslation(Translation[0], Translation[1], Translation[2]));
	m_framesToUpdate = framesToUpdate;
}

void ObjectConstants::OnImGuiRender()
{

	float min_translation = -10;
	float max_translation = 10;

	float min_scale = 1;
	float max_scale = 5;
	if (ImGui::SliderFloat3("pos", Translation, min_translation, max_translation))
	{
		Update();
	}
	
	if (ImGui::SliderFloat("scale", &Scale, min_scale, max_scale))
	{
		Update();
	}

}

RenderItem::RenderItem(std::vector <Geometry::MeshData>& meshes, ComPtr<ID3D12GraphicsCommandList> cmdList,
	std::string name):   m_name(name),
		m_objCbIndex(g_objectCBIndex++)
{
	Geo = Mesh(meshes, cmdList, name);
}

RenderItem::RenderItem(Model model, std::string name) : Geo(model.m_mesh), m_name(name), m_objCbIndex(g_objectCBIndex++)
{}

void RenderItem::SetPrimitiveTopology(ID3D12GraphicsCommandList* cmList)
{
	cmList->IASetPrimitiveTopology(m_primitiveType);
}


void RenderItem::Update()
{
	numFramesDirty = NumFrames;
	m_transformation.Update();
}

void RenderItem::OnImGuiRender()
{
	ImGui::Begin(m_name.c_str());
	{
		m_transformation.OnImGuiRender();
	}
	ImGui::End();
}

