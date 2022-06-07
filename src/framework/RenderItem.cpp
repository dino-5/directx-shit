#include "include/RenderItem.h"
#include "include/ImGuiSettings.h"

void RenderItem::SetTranslation(float x, float y, float z)
{
	Translation[0] = x;
	Translation[1] = y;
	Translation[2] = z;
	Update();
}

void RenderItem::SetPrimitiveTopology(ID3D12GraphicsCommandList* cmList)
{
	cmList->IASetPrimitiveTopology(m_primitiveType);
}


void RenderItem::SetScale(float x, float y, float z)
{
	Scale[0] = x;
	Scale[1] = y;
	Scale[2] = z;
	Update();
}

void RenderItem::SetRotation(XMMATRIX matrix)
{
	Update();
}


void RenderItem::Update()
{
	XMStoreFloat4x4(&objectParam.World, XMMatrixScaling(Scale[0], Scale[1], Scale[2]) *
										XMMatrixTranslation(Translation[0], Translation[1], Translation[2]));
	numFramesDirty = NumFrames;
}

void RenderItem::OnImGuiRender()
{
	float min_translation = -10;
	float max_translation = 10;

	ImGui::Begin(m_name.c_str());
	{
		if (ImGui::SliderFloat3("pos", Translation, min_translation, max_translation))
		{
			Update();
		}
	}
	ImGui::End();
}
