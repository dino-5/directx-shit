#pragma once
#include "Demo/passes/Pass.h"
#include "EngineCommon/math/Vector.h"
#include "EngineCommon/math/Matrix.h"
#include "EngineCommon/include/types.h"
#include "EngineGfx/dx12/Buffers.h"
#include "EngineGfx/Texture.h"
#include "EngineGfx/Model.h"

using namespace engine;

struct Vertex
{
    math::Vector3 position;
};

struct ConstandBufferData
{
	math::Matrix4 view;
	math::Matrix4 perspective;
};

struct BindlessSP
{
	u32 constantBufferIndex;
	u32 textureIndex;
};

class graphics::Mesh;

class SimplePass : public Pass
{
public:
	SimplePass() {}
	SimplePass(graphics::RenderContext& context);
	void initialize(graphics::RenderContext& context);
	void draw(ID3D12GraphicsCommandList*, u32 nt)override;

private:
	BindlessSP m_rootIndexData;
	ConstandBufferData m_data;
	graphics::Texture m_texture;
	graphics::TextureHandle m_textureNew;
	graphics::ConstantBuffer m_constantBuffer;
	graphics::ConstantBuffer m_rootStructure;
	graphics::Model m_model;
	u32 test = 0;
};

