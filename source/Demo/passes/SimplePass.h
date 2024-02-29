#pragma once
#include "EngineCommon/math/Vector.h"
#include "EngineCommon/include/types.h"
#include "Demo/passes/Pass.h"
#include "EngineGfx/dx12/Buffers.h"
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
	u32 index;
};

class SimplePass : public Pass
{
public:
	SimplePass() {}
	SimplePass(ID3D12Device* device, i32 aspectRatio);
	void Initialize(ID3D12Device* device, i32 aspectRatio);
	void Draw(ID3D12GraphicsCommandList*, u32 nt)override;
private:
	ConstandBufferData m_data;
	u32 index;
	graphics::VertexBuffer m_vertexBuffer;
	graphics::ConstantBuffer m_constantBuffer;
};

