#pragma once
#include <string>
#include <d3d12.h>
namespace engine::graphics
{
	class PSO;
	class RootSignature;
	class RenderContext;
};
class Pass
{
public:
	Pass() = default;
	void SetPSO(std::string name);
	void SetRootSignature(std::string name);

	void SetPipeline(ID3D12GraphicsCommandList* cmdList);
	virtual ~Pass() = default;
	virtual void Draw(ID3D12GraphicsCommandList*)=0;

protected:
	engine::graphics::RootSignature* m_rootSignature;
	engine::graphics::PSO* m_pso;
};