#include "Demo/passes/Pass.h"
#include "EngineGfx/dx12/PSO.h"
#include "EngineGfx/dx12/RootSignature.h"
#include "EngineGfx/RenderContext.h"

using namespace engine;

void Pass::SetPSO(std::wstring name)
{
	m_pso = graphics::PSO::GetPSO(name);
}

void Pass::SetRootSignature(graphics::RootSignatureType type)
{
	m_rootSignature = graphics::RootSignature::GetRootSignature(type);
}

void Pass::SetPipeline(ID3D12GraphicsCommandList* cmdList)
{
}
