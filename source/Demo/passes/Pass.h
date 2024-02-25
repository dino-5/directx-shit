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
	void SetPSO(std::wstring name);
	void SetRootSignature(engine::graphics::RootSignatureType type);

	void SetPipeline(ID3D12GraphicsCommandList* cmdList);
	virtual ~Pass() = default;
	virtual void Draw(ID3D12GraphicsCommandList*, unsigned int)=0;

protected:
	engine::graphics::RootSignature* m_rootSignature;
	engine::graphics::PSO* m_pso;
};