#pragma once
#include "EngineCommon/math/Vector.h"
#include "Demo/passes/Pass.h"
#include "EngineGfx/dx12/Buffers.h"

using namespace engine;

struct Vertex
{
    math::Vector3 position;
};

class SimplePass : public Pass
{
public:
	SimplePass() :m_constantBuffer(sizeof(float)) {}
	SimplePass(ID3D12Device* device, int aspectRatio);
	void Initialize(ID3D12Device* device, int aspectRatio);
	void Draw(ID3D12GraphicsCommandList*, uint)override;
private:
	float m_data = 0.8;
	graphics::VertexBuffer m_vertexBuffer;
	graphics::ConstantBuffer m_constantBuffer;
};

