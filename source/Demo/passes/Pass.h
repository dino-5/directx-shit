#pragma once
#include <string>
#include <d3d12.h>
#include "EngineCommon/include/types.h"
namespace engine::graphics
{
	class PSO;
	class RootSignature;
	class RenderContext;
	enum RootSignatureType;
};
class Pass
{
public:
	Pass() = default;
	void setPSO(std::wstring name);
	void setRootSignature(engine::graphics::RootSignatureType type);

	void setPipeline(ID3D12GraphicsCommandList* cmdList);
	virtual ~Pass() = default;
	virtual void draw(ID3D12GraphicsCommandList*, unsigned int)=0;

protected:
	engine::graphics::RootSignature* m_rootSignature;
	engine::graphics::PSO* m_pso;
};