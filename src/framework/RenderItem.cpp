#include "include/RenderItem.h"
#include "include/ImGuiSettings.h"
#include "dx12/Device.h"

void ObjectConstants::SetTranslation(float x, float y, float z)
{
	Translation[0] = x;
	Translation[1] = y;
	Translation[2] = z;
}

void ObjectConstants::SetScale(float x, float y, float z)
{
	Scale[0] = x;
	Scale[1] = y;
	Scale[2] = z;
}

void ObjectConstants::SetRotation(XMMATRIX matrix)
{
}

void ObjectConstants::Update()
{
	XMStoreFloat4x4(&properties.World,XMMatrixScaling(Scale[0], Scale[1], Scale[2]) * 
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
	
	if (ImGui::SliderFloat3("scale", Scale, min_scale, max_scale))
	{
		Update();
	}

}

RenderItem::RenderItem(std::vector <Geometry::MeshData>& meshes, ComPtr<ID3D12GraphicsCommandList> cmdList,
	std::string name):   m_name(name)
{
	Geo = Mesh(meshes, cmdList, name);
}

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

