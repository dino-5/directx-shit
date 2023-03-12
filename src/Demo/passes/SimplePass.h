#pragma once
#include "Framework/math/Vector.h"
#include "Demo/passes/Pass.h"

using namespace engine;

struct Vertex
{
    math::Vector3 position;
};

class SimplePass : public Pass
{
public:
	SimplePass() = default;
	SimplePass(ID3D12Device* device, int aspectRatio);
	void Initialize(ID3D12Device* device, int aspectRatio);
	void Draw(ID3D12GraphicsCommandList*)override;
private:
	ComPtr<ID3D12Resource> m_vertexBuffer;
	D3D12_VERTEX_BUFFER_VIEW m_vertexBufferView;

};

