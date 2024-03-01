#pragma once
#include "EngineCommon/math/Vector.h"
#include "EngineCommon/include/types.h"
#include "Demo/passes/Pass.h"
#include "EngineGfx/dx12/Buffers.h"
#include "EngineGfx/Texture.h"
#include "EngineCommon/math/Matrix.h"

using namespace engine;

struct Vertex
{
    math::Vector3 position;
};

struct ConstandBufferData
{
	math::Matrix4 perspective;
	f32 color;
};

struct BindlessSP
{
	u32 indexConstantBuffer;
	u32 indexTexture;
};

class SimplePass : public Pass
{
public:
	SimplePass() {}
	SimplePass(graphics::RenderContext& context);
	void Initialize(graphics::RenderContext& context);
	void Draw(ID3D12GraphicsCommandList*, u32 nt)override;
private:
	ConstandBufferData m_data;
	BindlessSP m_rootIndexData;
	graphics::Texture m_texture;
	graphics::VertexBuffer m_vertexBuffer;
	graphics::ConstantBuffer m_constantBuffer;
	graphics::ConstantBuffer m_rootStructure;
};

